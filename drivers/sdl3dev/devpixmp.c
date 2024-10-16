/*
 * Device pixelmap methods
 *
 * Note for sub-pixelmaps:
 * - pm_pixels is still the top-left of the original image. It is NOT offset.
 * - pm_base_x, and pm_base_y are the top-left coordinates of the rect.
 * - When doing operations, points/rects are clipped against the local coordinates first,
 *   and are then converted to global coordinates by adding pm_base_{x,y}.
 * - As pm_base_{x,y} is 0 on regular pixelmaps, this logic doesn't need to be special-cased.
 * - Double buffering to a sub-pixelmap is explicitly NOT supported.
 */

#include <stddef.h>

#include "drv.h"
#include "pm_ip.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Default dispatch table for device pixelmap (defined at end of file)
 */
static const struct br_device_pixelmap_dispatch devicePixelmapDispatch;

/*
 * Device pixelmap info. template
 */
#define F(f) offsetof(struct br_device_pixelmap, f)

br_boolean DevicePixelmapSDL3IsOurs(const br_pixelmap *pm)
{
    return ((br_device_pixelmap *)pm)->dispatch == &devicePixelmapDispatch;
}

static br_error custom_query(br_value *pvalue, void **extra, br_size_t *pextra_size, void *block,
                             const struct br_tv_template_entry *tep)
{
    const br_device_pixelmap *self = block;

    switch(tep->token) {
        case BRT_INDEXED_B:
            pvalue->b = SDL_ISPIXELFORMAT_INDEXED(self->surface->format);
            break;
        /*
        case BRT_PIXEL_CHANNELS_I32:
            return BRE_UNSUPPORTED;
        case BRT_PIXEL_CHANNELS_TL:
            return BRE_UNSUPPORTED;
        case BRT_ORIGIN_V2_I:
            // This is wrong
            *pvalue->v2_i = (br_vector2_i){
                .v = {self->pm_origin_x, self->pm_origin_y}
            };
            break;
         */
        case BRT_PIXEL_BITS_I32:
            pvalue->i32 = SDL_BITSPERPIXEL(self->surface->format);
            break;
        case BRT_WORD_BYTES_I32:
            pvalue->i32 = SDL_BYTESPERPIXEL(self->surface->format);
            break;
        case BRT_MEMORY_MAPPED_B:
            pvalue->b = self->pm_pixels != NULL;
            break;
        case BRT_SDL_EXT_PROCS_P:
            pvalue->p = (void *)&self->ext_procs;
            break;
        default:
            return BRE_UNKNOWN;
    }

    return BRE_OK;
}

static const br_tv_custom custom = {
    .query      = custom_query,
    .set        = NULL,
    .extra_size = NULL,
};

static struct br_tv_template_entry devicePixelmapTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),    F(pm_identifier),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(WIDTH_I32),          F(pm_width),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(HEIGHT_I32),         F(pm_height),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(PIXEL_TYPE_U8),      F(pm_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8,  0                    },
    {BRT(PIXEL_CHANNELS_I32), 0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(PIXEL_CHANNELS_TL),  0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(INDEXED_B),          0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(ORIGIN_V2_I),        0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(PIXEL_BITS_I32),     0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(WORD_BYTES_I32),     0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(MEMORY_MAPPED_B),    0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(OUTPUT_FACILITY_O),  F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(FACILITY_O),         F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(CLUT_O),             F(clut),            BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(WINDOW_HANDLE_H),    F(window),          BRTV_QUERY,            BRTV_CONV_COPY,    0                    },
    {BRT(SURFACE_HANDLE_H),   F(surface),         BRTV_QUERY,            BRTV_CONV_COPY,    0                    },
    {BRT(SDL_EXT_PROCS_P),    0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {DEV(SDL_SURFACE_H),      F(surface),         BRTV_QUERY,            BRTV_CONV_COPY,    0                    },
};

#undef F

/*
 * Resync after the surface has been changed.
 */
static br_error resync(br_device_pixelmap *self)
{
    SDL_Renderer *renderer;

    /*
     * It is an error to call this on a subsurface.
     */
    if(!self->owned)
        return BRE_UNSUPPORTED;

    self->pm_width  = self->surface->w;
    self->pm_height = self->surface->h;

    self->pm_pixels = self->surface->pixels;
    if(self->pm_pixels == NULL)
        self->pm_flags |= BR_PMF_NO_ACCESS;
    else {
        self->pm_flags &= ~BR_PMF_NO_ACCESS;
    }

    self->pm_flags |= BR_PMF_LINEAR;

    self->pm_row_bytes = (br_int_16)self->surface->pitch;
    if(((self->pm_row_bytes * 8) % SDL_BITSPERPIXEL(self->surface->format)) == 0)
        self->pm_flags |= BR_PMF_ROW_WHOLEPIXELS;

    if(self->renderer != NULL)
        SDL_DestroyRenderer(self->renderer);

    if((renderer = SDL_CreateSoftwareRenderer(self->surface)) == NULL) {
        BrLogError("SDL3", "Unable to create software renderer: %s", SDL_GetError());
        return BRE_FAIL;
    }

    self->renderer = renderer;
    return BRE_OK;
}

br_device_pixelmap *DevicePixelmapSDL3Allocate(br_device *dev, br_output_facility *outfcty, SDL_Window *window,
                                               SDL_Surface *surface, br_boolean owned)
{
    br_device_pixelmap *self;
    int                 bpp;
    br_uint_8           type;

    UASSERT(dev != NULL);
    UASSERT(outfcty == NULL || ObjectDevice(outfcty) == dev);

    if(SDLToBRenderPixelFormat(surface->format, &bpp, &type) != BRE_OK)
        return NULL;

    self = BrResAllocate(dev->res, sizeof(br_device_pixelmap), BR_MEMORY_OBJECT);
    if(self == NULL)
        return NULL;

    self->dispatch = &devicePixelmapDispatch;
    self->pm_identifier = BrResSprintf(self, "SDL3:%s:%dx%d", window != NULL ? "Window" : "Surface", surface->w, surface->h);
    self->device          = dev;
    self->output_facility = outfcty;

    self->pm_flags         = 0;
    self->pm_copy_function = BR_PMCOPY_NORMAL;
    self->pm_base_x        = 0;
    self->pm_base_y        = 0;
    self->pm_origin_x      = 0;
    self->pm_origin_y      = 0;
    self->pm_type          = type;

    self->owned   = owned;
    self->surface = surface;
    self->window  = window;

    if(resync(self) != BRE_OK) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    if(outfcty != NULL) {
        ObjectContainerAddFront(outfcty, (br_object *)self);
    }

    if(self->pm_type == BR_PMT_INDEX_8) {
        if((self->clut = DeviceClutSDL3Allocate(self, "CLUT")) == NULL) {
            BrResFreeNoCallback(self);
            return NULL;
        }
    }

    return self;
}

static void BR_CMETHOD_DECL(br_device_pixelmap_sdl3, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->output_facility != NULL)
        ObjectContainerRemove(self->output_facility, (br_object *)self);

    if(self->owned) {
        if(self->renderer != NULL)
            SDL_DestroyRenderer(self->renderer);

        if(self->window != NULL) {
            SDL_DestroyWindow(self->window);
        } else {
            SDL_DestroySurface(self->surface);
        }
    }

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_pixelmap_sdl3, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_sdl3, device)(br_object *self)
{
    return ((br_device_pixelmap *)self)->device;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_sdl3, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_sdl3, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_sdl3, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device_pixelmap);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_sdl3, queryTemplate)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapTemplate == NULL)
        self->device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(self->device, devicePixelmapTemplateEntries,
                                                                              BR_ASIZE(devicePixelmapTemplateEntries));

    return self->device->templates.devicePixelmapTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    SDL_Surface *surface;

    /*
     * Catch this special case.
     */
    if(self->pm_width == width && self->pm_height == height)
        return BRE_OK;

    /*
     * Can't resize a non-owned surface.
     */
    if(!self->owned)
        return BRE_FAIL;

    /*
     * Can always resize a window.
     */
    if(self->window != NULL) {
        SDL_SetWindowSize(self->window, width, height);

        if((self->surface = SDL_GetWindowSurface(self->window)) == NULL) {
            BrLogError("SDL3", "Unable to get window surface: %s", SDL_GetError());
            return BRE_FAIL;
        }
    } else {
        ASSERT(self->surface != NULL);

        surface = DevicePixelmapSDL3CreateSurface(width, height, self->surface->format);
        if(surface == NULL) {
            BrLogError("SDL3", "Error creating surface: %s", SDL_GetError());
            return BRE_FAIL;
        }

        /*
         * If we're indexed and there's a CLUT attached, copy it over.
         */
        if(SDL_ISPIXELFORMAT_INDEXED(surface->format) && self->clut != NULL) {
            SDL_Palette *palette = SDL_GetSurfacePalette(surface);
            if(DeviceSDL3SetPaletteFromCLUT(palette, self->clut) != BRE_OK) {
                SDL_DestroySurface(surface);
                BrLogError("SDL3", "Unable to restore palette");
                return BRE_FAIL;
            }

            self->clut->pal = palette;
        }

        SDL_DestroySurface(self->surface);
        self->surface = surface;
    }

    return resync(self);
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, validSource)(br_device_pixelmap *self, br_boolean *bp, br_object *h)
{
    /*
     * Nothing uses this, not sure what the expected behaviour is.
     */
    (void)self;
    (void)h;

    *bp = BR_FALSE;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, allocateSub)(br_device_pixelmap  *self,
                                                                      br_device_pixelmap **newpm, br_rectangle *rect)
{
    br_rectangle        out;
    br_device_pixelmap *pm;

    /*
     * Don't allow nested sub pixelmaps, it's too annoying to deal with.
     */
    if(self->owned != BR_TRUE)
        return BRE_FAIL;

    /*
     * Create sub-window (clipped against original)
     */
    if(PixelmapRectangleClip(&out, rect, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    /*
     * Create the new structure and copy
     */
    pm = BrResAllocate(self->device->res, sizeof(*pm), BR_MEMORY_OBJECT);

    *pm = *self;

    pm->pm_base_x += (br_uint_16)out.x;
    pm->pm_base_y += (br_uint_16)out.y;
    pm->pm_width  = (br_uint_16)out.w;
    pm->pm_height = (br_uint_16)out.h;

    pm->pm_origin_x = 0;
    pm->pm_origin_y = 0;

    pm->pm_stored = NULL;

    /*
     * Pixel rows may not be contiguous
     */
    if(self->pm_width != pm->pm_width)
        pm->pm_flags &= ~BR_PMF_LINEAR;

    pm->owned    = BR_FALSE;
    pm->surface  = self->surface;
    pm->renderer = self->renderer;

    *newpm = pm;

    return BRE_OK;
}

/*
 * Device->device same-size, non-stretch copy.
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, copy)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    SDL_Rect     drect;
    br_rectangle r = {
        .x = -self->pm_origin_x,
        .y = -self->pm_origin_y,
        .w = src->pm_width,
        .h = src->pm_height,
    };

    if(DevicePixelmapSDL3RectangleClip(&drect, &r, (const br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    drect.x += self->pm_base_x;
    drect.y += self->pm_base_y;

    if(!SDL_BlitSurface(src->surface, NULL, self->surface, &drect))
        return BRE_FAIL;

    return BRE_OK;
}

/*
 * Memory->device non-stretch copy
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, rectangleCopyTo)(br_device_pixelmap *self, br_point *p,
                                                                          br_device_pixelmap *src, br_rectangle *r)
{
    SDL_Rect  srect, drect;
    SDL_Point dpoint;

    if(DevicePixelmapSDL3RectangleClipTwo(&srect, &dpoint, r, p, (br_pixelmap *)self, (br_pixelmap *)src) == BR_CLIP_REJECT)
        return BRE_OK;

    drect = (SDL_Rect){
        .x = dpoint.x + self->pm_base_x,
        .y = dpoint.y + self->pm_base_y,
        .w = srect.w,
        .h = srect.h,
    };

    if(DevicePixelmapSDL3BlitSurface((br_pixelmap *)src, &srect, (br_pixelmap *)self, &drect, SDL_BlitSurface) == BRE_OK)
        return BRE_OK;

    return BRE_FAIL;
}

/*
 * Device->memory copy, device non-addressable.
 * Otherwise this would be going via br_device_pixelmap_mem::rectangleCopyTo().
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p,
                                                                            br_device_pixelmap *dest, br_rectangle *r)
{

    SDL_Rect  srect, drect;
    SDL_Point dpoint;

    if(DevicePixelmapSDL3RectangleClipTwo(&srect, &dpoint, r, p, (br_pixelmap *)dest, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    drect = (SDL_Rect){
        .x = dpoint.x + self->pm_base_x,
        .y = dpoint.y + self->pm_base_y,
        .w = srect.w,
        .h = srect.h,
    };

    if(DevicePixelmapSDL3BlitSurface((br_pixelmap *)self, &srect, (br_pixelmap *)dest, &drect, SDL_BlitSurface) == BRE_OK)
        return BRE_OK;

    return BRE_FAIL;
}

/*
 * Memory -> device stretch copy
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *d,
                                                                                 br_device_pixelmap *src, br_rectangle *s)
{
    SDL_Rect srect, drect;

    if(DevicePixelmapSDL3RectangleClip(&srect, s, (const br_pixelmap *)src) == BR_CLIP_REJECT)
        return BRE_OK;

    if(DevicePixelmapSDL3RectangleClip(&drect, d, (const br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    drect.x += self->pm_base_x;
    drect.y += self->pm_base_y;

    if(DevicePixelmapSDL3BlitSurface((br_pixelmap *)src, &srect, (br_pixelmap *)self, &drect, DevicePixelmapSDL3BlitScaled) == BRE_OK)
        return BRE_OK;

    return BRE_FAIL;
}

/*
 * Device->memory stretch copy, device non-addressable.
 * Otherwise this would be going via br_device_pixelmap_mem::rectangleStretchCopyTo().
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, rectangleStretchCopyFrom)(br_device_pixelmap *self, br_rectangle *s,
                                                                                   br_device_pixelmap *dest, br_rectangle *d)
{
    SDL_Rect srect, drect;

    if(DevicePixelmapSDL3RectangleClip(&srect, s, (const br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if(DevicePixelmapSDL3RectangleClip(&drect, d, (const br_pixelmap *)dest) == BR_CLIP_REJECT)
        return BRE_OK;

    drect.x += self->pm_base_x;
    drect.y += self->pm_base_y;

    if(DevicePixelmapSDL3BlitSurface((br_pixelmap *)self, &srect, (br_pixelmap *)dest, &drect, DevicePixelmapSDL3BlitScaled) == BRE_OK)
        return BRE_OK;

    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    SDL_Point point;
    SDL_Color col;
    const SDL_PixelFormatDetails *pfd;

    if(DevicePixelmapSDL3PointClip(&point, p, (const br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    point.x += self->pm_base_x;
    point.y += self->pm_base_y;

    pfd = SDL_GetPixelFormatDetails(self->surface->format);

    // TODO: test with indexed
    SDL_GetRGBA(colour, pfd, SDL_GetSurfacePalette(self->surface), &col.r, &col.g, &col.b, &col.a);

    if(!SDL_SetRenderDrawColor(self->renderer, col.r, col.g, col.b, col.a))
        return BRE_FAIL;

    if(!SDL_RenderPoint(self->renderer, (float)point.x, (float)point.y))
        return BRE_FAIL;

    SDL_FlushRenderer(self->renderer);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, line)(br_device_pixelmap *self, br_point *s, br_point *e,
                                                               br_uint_32 colour)
{
    SDL_Point spoint, epoint;
    SDL_Color col;

    SDL_Palette                  *pal = SDL_GetSurfacePalette(self->surface);
    const SDL_PixelFormatDetails *pfd = SDL_GetPixelFormatDetails(self->surface->format);

    if(DevicePixelmapSDL3LineClip(&spoint, &epoint, s, e, (const br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    spoint.x += self->pm_base_x;
    spoint.y += self->pm_base_y;

    epoint.x += self->pm_base_x;
    epoint.y += self->pm_base_y;

    SDL_GetRGBA(colour, pfd, pal, &col.r, &col.g, &col.b, &col.a);

    if(!SDL_SetRenderDrawColor(self->renderer, col.r, col.g, col.b, col.a))
        return BRE_FAIL;

    if(!SDL_RenderLine(self->renderer, (float)spoint.x, (float)spoint.y, (float)epoint.x, (float)epoint.y))
        return BRE_FAIL;

    SDL_FlushRenderer(self->renderer);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, rectangleFill)(br_device_pixelmap *self, br_rectangle *r,
                                                                        br_uint_32 colour)
{
    SDL_Rect rect;

    if(DevicePixelmapSDL3RectangleClip(&rect, r, (const br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    rect.x += self->pm_base_x;
    rect.y += self->pm_base_y;

    if(!SDL_FillSurfaceRect(self->surface, &rect, colour))
        return BRE_FAIL;

    return BRE_OK;
}

void *DevicePixelmapSDLMemAddress(br_device_pixelmap *self, br_int_32 x, br_int_32 y)
{
    return (void *)((br_uintptr_t)self->pm_pixels + ((self->pm_base_y + y) * self->surface->pitch) +
                    ((self->pm_base_x + x) * SDL_BYTESPERPIXEL(self->surface->format)));
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, pixelQuery)(br_device_pixelmap *self, br_uint_32 *pcolour, br_point *p)
{
    br_error  err;
    SDL_Point ap;
    br_colour col;

    switch(SDL_BYTESPERPIXEL(self->surface->format)) {
        case 1:
        case 2:
        case 3:
        case 4:
            break;
        default:
            return BRE_FAIL;
    }

    if(DevicePixelmapSDL3PointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    if((err = DevicePixelmapDirectLock(self, BR_FALSE)) != BRE_OK)
        return err;

    col = _MemPixelGet(DevicePixelmapSDLMemAddress(self, ap.x, ap.y), SDL_BYTESPERPIXEL(self->surface->format));

    if((err = DevicePixelmapDirectUnlock(self)) != BRE_OK)
        return err;

    *pcolour = col;
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, pixelAddressQuery)(br_device_pixelmap *self, void **pptr,
                                                                     br_uint_32 *pqual, br_point *p)
{
    SDL_Point ap;

    (void)pqual;

    if(DevicePixelmapSDL3PointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    if(pptr != NULL)
        *pptr = DevicePixelmapSDLMemAddress(self, ap.x, ap.y);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, directLock)(br_device_pixelmap *self, br_boolean block)
{
    (void)block; /* We're single-threaded CPU surface. We can't block. */

    if(SDL_MUSTLOCK(self->surface) == 0)
        return BRE_OK;

    if(self->surface->flags & SDL_SURFACE_LOCKED)
        return BRE_OK;

    if(!SDL_LockSurface(self->surface))
        return BRE_FAIL;

    self->pm_pixels    = self->surface->pixels;
    self->pm_row_bytes = (br_int_16)self->surface->pitch;
    self->pm_flags &= ~BR_PMF_NO_ACCESS;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, directUnlock)(br_device_pixelmap *self)
{
    if(SDL_MUSTLOCK(self->surface) == 0)
        return BRE_OK;

    if(self->pm_pixels == NULL)
        return BRE_OK;

    SDL_UnlockSurface(self->surface);

    self->pm_pixels    = NULL;
    self->pm_row_bytes = 0;
    self->pm_flags |= BR_PMF_NO_ACCESS;

    return BRE_OK;
}

static void DevicePixelmapSDLExtPreSwap(br_device_pixelmap *self)
{
    if(self->ext_procs.preswap != NULL)
        self->ext_procs.preswap((br_pixelmap *)self, self->ext_procs.user);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    br_error err;

    /*
     * Go via the generic double-buffer, which will punt us off the appropriate copy().
     */
    if((err = BR_CMETHOD(br_device_pixelmap_gen, doubleBuffer)(self, src)) != BRE_OK)
        return err;

    DevicePixelmapSDLExtPreSwap(self);

    if(self->window != NULL) {
        if(!SDL_UpdateWindowSurface(self->window)) {
            BrLogTrace("SDL3", "Unable to update window surface: %s", SDL_GetError());
            return BRE_FAIL;
        }
    }

    return BRE_OK;
}

/*
 * Structures used to translate tokens and values
 */
struct match_off_tokens {
    br_token  use;
    br_int_32 pixel_bits;
    br_int_32 width;
    br_int_32 height;
    br_uint_8 type;
};

#define F(f) offsetof(struct match_off_tokens, f)

static struct br_tv_template_entry matchOffTemplateEntries[] = {
    {BRT_USE_T,          0, F(use),        BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_BITS_I32, 0, F(pixel_bits), BRTV_SET, BRTV_CONV_COPY},
    {BRT_WIDTH_I32,      0, F(width),      BRTV_SET, BRTV_CONV_COPY},
    {BRT_HEIGHT_I32,     0, F(height),     BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_TYPE_U8,  0, F(type),       BRTV_SET, BRTV_CONV_COPY},
};
#undef F

br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, match)(br_device_pixelmap *self, br_device_pixelmap **newpm,
                                                         br_token_value *tv)
{
    br_int_32           count;
    br_device_pixelmap *pm;
    SDL_Surface        *surface;
    Uint32              format;
    int                 bpp;

    struct match_off_tokens mt = {
        .use        = BRT_NONE,
        .pixel_bits = 0,
        .width      = 0,
        .height     = 0,
        .type       = BR_PMT_MAX,
    };

    if(self->device->templates.matchOffTemplate == NULL) {
        self->device->templates.matchOffTemplate = BrTVTemplateAllocate(self->device->res, matchOffTemplateEntries,
                                                                        BR_ASIZE(matchOffTemplateEntries));
    }

    mt.width  = self->pm_width;
    mt.height = self->pm_height;
    mt.type   = self->pm_type;

    BrTokenValueSetMany(&mt, &count, NULL, tv, self->device->templates.matchOffTemplate);

    if(mt.use == BRT_CLONE) {
        mt.type       = self->pm_type;
        mt.pixel_bits = SDL_BITSPERPIXEL(self->surface->format);
    }

    /*
     * SDL has no depth or single-channel formats, so we can't handle depth
     * textures - fall back to memory.
     *
     * NB: pixel_bits is only used for depth formats
     */
    if(mt.use == BRT_DEPTH) {
        return BR_CMETHOD(br_device_pixelmap_mem, match)(self, newpm, tv);
    }

    if(BRenderToSDLPixelFormat(mt.type, &format, &bpp) != BRE_OK) {
        /*
         * Unsupported pixel format, fall back to memory.
         */
        return BR_CMETHOD(br_device_pixelmap_mem, match)(self, newpm, tv);
    }

    surface = DevicePixelmapSDL3CreateSurface(mt.width, mt.height, format);
    if(surface == NULL) {
        BrLogError("SDL3", "Error creating surface: %s", SDL_GetError());
        return BRE_FAIL;
    }

    if((pm = DevicePixelmapSDL3Allocate(self->device, NULL, NULL, surface, BR_TRUE)) == NULL)
        return BRE_FAIL;

    /*
     * Copy origin over
     */
    pm->pm_origin_x = self->pm_origin_x;
    pm->pm_origin_y = self->pm_origin_y;

    /*
     * If both pixelmaps have a CLUT, copy it over.
     */
    if(self->clut != NULL && pm->clut != NULL) {
        DeviceSDL3SetPaletteFromCLUT(pm->clut->pal, self->clut);
    }

    *newpm = pm;

    return BRE_OK;
}

/*
 * Resync after an external change, such as a screen resize.
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_sdl3, handleWindowEvent)(br_device_pixelmap *self, void *arg)
{
    const SDL_WindowEvent *evt = arg;

    if(self->window == NULL)
        return BRE_OK;

    /*
     * We can only handle window size changes for now.
     */
    if(evt->type != SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
        return BRE_OK;

    /* NB: this will invalidate all subsurfaces. */
    if((self->surface = SDL_GetWindowSurface(self->window)) == NULL) {
        BrLogError("SDL3", "Unable to get window surface: %s", SDL_GetError());
        return BRE_FAIL;
    }

    return resync(self);
}

/*
 * Default dispatch table for device pixelmap
 */
static const struct br_device_pixelmap_dispatch devicePixelmapDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_sdl3, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_sdl3, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_sdl3, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_sdl3, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_sdl3, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_sdl3, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_sdl3, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_sdl3, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_sdl3, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_sdl3, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_sdl3, allocateSub),

    ._copy         = BR_CMETHOD_REF(br_device_pixelmap_sdl3, copy),
    ._copyTo       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyTo),
    ._copyFrom     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFrom),
    ._fill         = BR_CMETHOD_REF(br_device_pixelmap_gen, fill),
    ._doubleBuffer = BR_CMETHOD_REF(br_device_pixelmap_sdl3, doubleBuffer),

    ._copyDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, copyDirty),
    ._copyToDirty       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyToDirty),
    ._copyFromDirty     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFromDirty),
    ._fillDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, fillDirty),
    ._doubleBufferDirty = BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBufferDirty),

    ._rectangle                = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle),
    ._rectangle2               = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle2),
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_sdl3, rectangleCopyTo),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_sdl3, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_sdl3, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_sdl3, rectangleStretchCopyTo),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_sdl3, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_sdl3, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_sdl3, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_sdl3, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_sdl3, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_gen, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_gen, text),
    ._textBounds = BR_CMETHOD_REF(br_device_pixelmap_gen, textBounds),

    /* FIXME: Implement these once something actually uses us. */
    ._rowSize  = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSize),
    ._rowQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, rowQuery),
    ._rowSet   = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSet),

    ._pixelQuery        = BR_CMETHOD_REF(br_device_pixelmap_sdl3, pixelQuery),
    ._pixelAddressQuery = BR_CMETHOD_REF(br_device_pixelmap_sdl3, pixelAddressQuery),

    ._pixelAddressSet = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressSet),
    ._originSet       = BR_CMETHOD_REF(br_device_pixelmap_gen, originSet),

    ._flush        = BR_CMETHOD_REF(br_device_pixelmap_gen, flush),
    ._synchronise  = BR_CMETHOD_REF(br_device_pixelmap_gen, synchronise),
    ._directLock   = BR_CMETHOD_REF(br_device_pixelmap_sdl3, directLock),
    ._directUnlock = BR_CMETHOD_REF(br_device_pixelmap_sdl3, directUnlock),

    ._getControls = BR_CMETHOD_REF(br_device_pixelmap_gen, getControls),
    ._setControls = BR_CMETHOD_REF(br_device_pixelmap_gen, setControls),

    ._handleWindowEvent = BR_CMETHOD_REF(br_device_pixelmap_sdl3, handleWindowEvent),
};
