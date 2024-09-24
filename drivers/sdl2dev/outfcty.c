/*
 * Output type methods
 */
#include <stddef.h>
#include <SDL.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_output_facility_dispatch outputFacilityDispatch;

/*
 * Output Type info. template
 */
#define F(f) offsetof(struct br_output_facility, f)

static struct br_tv_template_entry outputFacilityTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR,    NULL, F(identifier),          BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},

    {BRT_WIDTH_I32,          NULL, F(width),               BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_WIDTH_MIN_I32,      NULL, F(width),               BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_WIDTH_MAX_I32,      NULL, F(width),               BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_HEIGHT_I32,         NULL, F(height),              BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_HEIGHT_MIN_I32,     NULL, F(height),              BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_HEIGHT_MAX_I32,     NULL, F(height),              BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},

    {BRT_PIXEL_TYPE_U8,      NULL, F(colour_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_PIXEL_BITS_I32,     NULL, F(colour_bits),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},

    {BRT_WINDOW_MONITOR_I32, NULL, F(monitor),             BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},

 /* Unused, just required for matching. */
    {BRT_WINDOW_HANDLE_H,    NULL, (br_uintptr_t)NULL,     BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_WINDOW_NAME_CSTR,   NULL, (br_uintptr_t)NULL,     BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_USE_T,              NULL, (br_uintptr_t)BRT_NONE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_HIDPI_B,            NULL, (br_uintptr_t)BR_FALSE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_RESIZABLE_B,        NULL, (br_uintptr_t)BR_FALSE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_OPENGL_B,           NULL, (br_uintptr_t)BR_FALSE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_WINDOW_FULLSCREEN_B,NULL, (br_uintptr_t)BR_FALSE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_SDL_EXT_PROCS_P,    NULL, 0,                      BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
};

#undef F

#define F(f) offsetof(struct pixelmap_new_tokens, f)
static struct br_tv_template_entry pixelmapNewTemplateEntries[] = {
    {BRT(WINDOW_NAME_CSTR), F(title),      BRTV_SET, BRTV_CONV_COPY, 0                       },
    {BRT(WIDTH_I32),        F(width),      BRTV_SET, BRTV_CONV_COPY, 0                       },
    {BRT(HEIGHT_I32),       F(height),     BRTV_SET, BRTV_CONV_COPY, 0                       },
    {BRT(PIXEL_BITS_I32),   F(pixel_bits), BRTV_SET, BRTV_CONV_COPY, 0                       },
    {BRT(PIXEL_TYPE_U8),    F(pixel_type), BRTV_SET, BRTV_CONV_COPY, 0                       },
    {BRT(WINDOW_HANDLE_H),  F(window),     BRTV_SET, BRTV_CONV_COPY, 0                       },
    {BRT(SURFACE_HANDLE_H), F(surface),    BRTV_SET, BRTV_CONV_COPY, 0                       },
    {BRT(USE_T),            F(use_type),   BRTV_SET, BRTV_CONV_COPY, 0                       },
    {BRT(HIDPI_B),          F(flags),      BRTV_SET, BRTV_CONV_BIT,  SDL_WINDOW_ALLOW_HIGHDPI},
    {BRT(RESIZABLE_B),      F(flags),      BRTV_SET, BRTV_CONV_BIT,  SDL_WINDOW_RESIZABLE    },
    {BRT(OPENGL_B),         F(flags),      BRTV_SET, BRTV_CONV_BIT,  SDL_WINDOW_OPENGL       },
    {BRT(WINDOW_FULLSCREEN_B), F(flags),   BRTV_SET, BRTV_CONV_COPY, SDL_WINDOW_FULLSCREEN   },
    {BRT(SDL_EXT_PROCS_P),  F(ext_procs),  BRTV_SET, BRTV_CONV_COPY, 0                       },
};
#undef F

const static struct {
    Uint32    format;
    br_int_32 bpp;
    br_uint_8 type;
} pixel_formats[] = {
  // {.format = SDL_PIXELFORMAT_INDEX4MSB, .bpp = 4,  .type = BR_PMT_INDEX_4  }, /* Untested */
    {.format = SDL_PIXELFORMAT_INDEX8,   .bpp = 8,  .type = BR_PMT_INDEX_8  },
    {.format = SDL_PIXELFORMAT_RGB332,   .bpp = 8,  .type = BR_PMT_RGB_332  },
    {.format = SDL_PIXELFORMAT_RGB555,   .bpp = 15, .type = BR_PMT_RGB_555  },
    {.format = SDL_PIXELFORMAT_BGR555,   .bpp = 15, .type = BR_PMT_BGR_555  },
    {.format = SDL_PIXELFORMAT_RGB565,   .bpp = 16, .type = BR_PMT_RGB_565  },
    {.format = SDL_PIXELFORMAT_BGR565,   .bpp = 16, .type = BR_PMT_BGR_565  },
#if BR_ENDIAN_LITTLE
    {.format = SDL_PIXELFORMAT_BGR24,    .bpp = 24, .type = BR_PMT_RGB_888  },
#else
    {.format = SDL_PIXELFORMAT_RGB24, .bpp = 24, .type = BR_PMT_RGB_888},
#endif
    {.format = SDL_PIXELFORMAT_XRGB8888, .bpp = 24, .type = BR_PMT_RGBX_888 },
    {.format = SDL_PIXELFORMAT_ARGB8888, .bpp = 32, .type = BR_PMT_RGBA_8888},
    {.format = SDL_PIXELFORMAT_ARGB4444, .bpp = 16, .type = BR_PMT_ARGB_4444},
};

br_error SDLToBRenderPixelFormat(Uint32 format, br_int_32 *bpp, br_uint_8 *type)
{
    for(size_t i = 0; i < BR_ASIZE(pixel_formats); ++i) {
        if(format != pixel_formats[i].format)
            continue;

        if(bpp != NULL)
            *bpp = pixel_formats[i].bpp;

        if(type != NULL)
            *type = pixel_formats[i].type;
        return BRE_OK;
    }

    BrLogTrace("SDL2", "Skipping unsupported pixel format %s", SDL_GetPixelFormatName(format));
    return BRE_FAIL;
}

br_error BRenderToSDLPixelFormat(br_uint_8 type, Uint32 *format, br_int_32 *bpp)
{
    for(size_t i = 0; i < BR_ASIZE(pixel_formats); ++i) {
        if(type != pixel_formats[i].type)
            continue;

        if(bpp != NULL)
            *bpp = pixel_formats[i].bpp;

        if(format != NULL)
            *format = pixel_formats[i].format;
        return BRE_OK;
    }

    return BRE_FAIL;
}

br_output_facility *OutputFacilitySDL2CreateFromMode(br_device *dev, SDL_DisplayMode *mode, int monitor)
{
    br_output_facility *self;
    br_int_32           bpp;
    br_uint_8           type;

    if(SDLToBRenderPixelFormat(mode->format, &bpp, &type) != BRE_OK)
        return NULL;

    if((self = BrResAllocate(dev->res, sizeof(br_output_facility), BR_MEMORY_OBJECT)) == NULL)
        return NULL;

    self->dispatch    = &outputFacilityDispatch;
    self->identifier  = BrResSprintf(self, "%d:%dx%dx%d", monitor, mode->w, mode->h, bpp);
    self->device      = dev;
    self->object_list = BrObjectListAllocate(self);
    ObjectContainerAddFront(dev, (br_object *)self);

    self->width       = mode->w;
    self->height      = mode->h;
    self->colour_bits = bpp;
    self->colour_type = type;
    self->monitor     = monitor;

    BrLogTrace("SDL2", "Created output facility: %s", self->identifier);
    return self;
}

int OutputFacilitySDL2EnumerateModes(br_device *dev)
{
    int ndisplays = SDL_GetNumVideoDisplays();
    int nadded    = 0;

    for(int d = 0; d < ndisplays; ++d) {
        int nmodes = SDL_GetNumDisplayModes(d);

        for(int m = 0; m < nmodes; ++m) {
            SDL_DisplayMode mode;

            if(SDL_GetDisplayMode(d, m, &mode) < 0) {
                BrLogWarn("SDL2", "Unable to get display mode (display=%d,mode=%d)", d, m);
                continue;
            }

            if(OutputFacilitySDL2CreateFromMode(dev, &mode, d) != NULL)
                ++nadded;
        }
    }

    return nadded;
}

br_output_facility *OutputFacilitySDL2CreateGeneric(br_device *dev)
{
    br_output_facility *self;

    if((self = BrResAllocate(dev->res, sizeof(br_output_facility), BR_MEMORY_OBJECT)) == NULL)
        return NULL;

    self->dispatch    = &outputFacilityDispatch;
    self->identifier  = BrResStrDup(self, "SDL2");
    self->device      = dev;
    self->object_list = BrObjectListAllocate(self);
    ObjectContainerAddFront(dev, (br_object *)self);

    self->width       = -1;
    self->height      = -1;
    self->colour_bits = -1;
    self->colour_type = BR_PMT_MAX;
    self->monitor     = -1;

    BrLogTrace("SDL2", "Created output facility: %s", self->identifier);
    return self;
}

/*
 * Common object methods
 */
static void BR_CMETHOD_DECL(br_output_facility_sdl2, free)(br_object *self)
{
    ObjectContainerRemove(ObjectDevice(self), (br_object *)self);

    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_output_facility_sdl2, identifier)(br_object *self)
{
    return ((br_output_facility *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_output_facility_sdl2, type)(br_object *self)
{
    return BRT_OUTPUT_FACILITY;
}

static br_boolean BR_CMETHOD_DECL(br_output_facility_sdl2, isType)(br_object *self, br_token t)
{
    return (t == BRT_OUTPUT_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_output_facility_sdl2, device)(br_object *self)
{
    return ((br_output_facility *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_output_facility_sdl2, space)(br_object *self)
{
    return sizeof(br_output_facility);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_output_facility_sdl2, queryTemplate)(br_object *_self)
{
    br_output_facility *self = (br_output_facility *)_self;

    if(self->device->templates.outputFacilityTemplate == NULL)
        self->device->templates.outputFacilityTemplate = BrTVTemplateAllocate(self->device, outputFacilityTemplateEntries,
                                                                              BR_ASIZE(outputFacilityTemplateEntries));

    return self->device->templates.outputFacilityTemplate;
}

static br_error BR_CMETHOD_DECL(br_output_facility_sdl2, validSource)(br_output_facility *self, br_boolean *bp, br_object *h)
{
    return BRE_OK;
}

#define DEFAULT_WINDOW_TITLE "BRender Application"

static br_uint_8 FigureOutFormat(br_uint_8 type, br_int_32 bits)
{
    if(type != BR_PMT_MAX)
        return type;

    switch(bits) {
        case 8:
            return BR_PMT_INDEX_8;
        case 15:
            return BR_PMT_RGB_555;
        case 16:
            return BR_PMT_RGB_565;
        case 24:
            return BR_PMT_RGB_888;
        case 32:
            return BR_PMT_RGBX_888;
        default:
            break;
    }

    return BR_PMT_MAX;
}

/*
 * Instantiate an output pixelmap from the output type
 *
 */
static br_error BR_CMETHOD_DECL(br_output_facility_sdl2, pixelmapNew)(br_output_facility  *self,
                                                                      br_device_pixelmap **ppmap, br_token_value *tv)
{
    br_device_pixelmap *pm;
    br_int_32           count;
    SDL_Window         *window  = NULL;
    SDL_Surface        *surface = NULL;
    Uint32              format  = SDL_PIXELFORMAT_UNKNOWN;
    int                 bpp     = -1;

    struct pixelmap_new_tokens pt = {
        .title      = NULL,
        .width      = -1,
        .height     = -1,
        .pixel_bits = -1,
        .pixel_type = BR_PMT_MAX,
        .window     = NULL,
        .use_type   = BRT_NONE,
        .flags      = 0,
    };

    if(self->device->templates.pixelmapNewTemplate == NULL) {
        self->device->templates.pixelmapNewTemplate = BrTVTemplateAllocate(self->device, pixelmapNewTemplateEntries,
                                                                           BR_ASIZE(pixelmapNewTemplateEntries));
    }

    BrTokenValueSetMany(&pt, &count, NULL, tv, self->device->templates.pixelmapNewTemplate);

    pt.pixel_type = FigureOutFormat(pt.pixel_type, pt.pixel_bits);

    /*
     * Four cases:
     *   1. New window, owned.
     *   2. New surface, owned.
     *   3. Existing window, unowned.
     *   4. Existing surface, unowned.
     */

    br_boolean is_window  = pt.surface == NULL && pt.use_type == BRT_NONE;
    br_boolean is_surface = pt.window == NULL && ((pt.surface == NULL && pt.use_type == BRT_OFFSCREEN) ||
                                                  (pt.surface != NULL && pt.use_type == BRT_NONE));
    br_boolean is_new     = (is_window && pt.window == NULL) || (pt.surface == NULL && pt.use_type == BRT_OFFSCREEN);

    if(is_window && is_surface) {
        BrLogError("SDL2", "Invalid parameter combination, refusing to create pixelmap");
        return BRE_FAIL;
    }

    /*
     * Always validate the pixel format, if given.
     */
    if(pt.pixel_type != BR_PMT_MAX) {
        if(BRenderToSDLPixelFormat(pt.pixel_type, &format, &bpp) != BRE_OK)
            return BRE_FAIL;
    }

    if(is_new) {
        if(pt.width <= 0 || pt.height <= 0)
            return BRE_FAIL;

        if(pt.title == NULL)
            pt.title = DEFAULT_WINDOW_TITLE;
    }

    /*
     * If wanting OpenGL, forward things on.
     */
    if(pt.flags & SDL_WINDOW_OPENGL)
        return DevicePixelmapSDL2CreateGL(&pt, ppmap);

    if(is_window) {
        if(is_new) {
            window = SDL_CreateWindow(pt.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pt.width, pt.height, pt.flags);
            if(window == NULL) {
                BrLogError("SDL2", "Error creating window: %s", SDL_GetError());
                return BRE_FAIL;
            }
        } else {
            window = pt.window;

            /* Not valid to set the pixel format on an existing window. */
            if(pt.pixel_type != BR_PMT_MAX)
                return BRE_FAIL;

            /* Update the size, if specified. */
            if(pt.width > 0 && pt.height > 0)
                SDL_SetWindowSize(pt.window, pt.width, pt.height);

            /* Update the title, if specified. */
            if(pt.title != NULL)
                SDL_SetWindowTitle(pt.window, pt.title);
        }

        /* Get the window surface. */
        if((surface = SDL_GetWindowSurface(window)) == NULL) {
            BrLogError("SDL2", "Unable to get window surface: %s", SDL_GetError());
            return BRE_FAIL;
        }

        /*
         * NB: Windows can't have their pixel format specified.
         * If the user has explicitly requested one, we check it after the pixelmap has been
         * created, and bail if it's wrong.
         */
    }

    if(is_surface) {
        if(is_new) {
            if(pt.pixel_type == BR_PMT_MAX)
                return BRE_FAIL;

            surface = SDL_CreateRGBSurfaceWithFormat(0, pt.width, pt.height, bpp, format);
            if(surface == NULL) {
                BrLogError("SDL2", "Error creating surface: %s", SDL_GetError());
                return BRE_FAIL;
            }
        } else {
            surface = pt.surface;
        }
    }

    if((pm = DevicePixelmapSDL2Allocate(self->device, self, window, surface, is_new)) == NULL) {
        /* NB: This will window/surface cleanup. */
        return BRE_FAIL;
    }

    /*
     * If the the user's requested a specific pixel format, and the window or surface doesn't have it, die.
     */
    if(pt.pixel_type != BR_PMT_MAX && pm->pm_type != pt.pixel_type) {
        ObjectFree(pm);
        return BRE_FAIL;
    }

    *ppmap = pm;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_output_facility_sdl2, clutNew)(br_output_facility     *self,
                                                                  struct br_device_clut **pclut, br_token_value *tv)
{
    (void)self;
    (void)pclut;
    (void)tv;
    return BRE_FAIL;
}

/*
 * No querying ability yet
 */
static br_error BR_CMETHOD_DECL(br_output_facility_sdl2, queryCapability)(br_output_facility *self, br_token_value *buffer_in,
                                                                          br_token_value *buffer_out, br_size_t size_buffer_out)
{
    (void)self;
    (void)buffer_in;
    (void)buffer_out;
    (void)size_buffer_out;
    return BRE_FAIL;
}

static void *BR_CMETHOD_DECL(br_output_facility_sdl2, listQuery)(br_object_container *self)
{
    return ((br_output_facility *)self)->object_list;
}

/*
 * Default dispatch table for device
 */
static const struct br_output_facility_dispatch outputFacilityDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_output_facility_sdl2, free),
    ._identifier = BR_CMETHOD_REF(br_output_facility_sdl2, identifier),
    ._type       = BR_CMETHOD_REF(br_output_facility_sdl2, type),
    ._isType     = BR_CMETHOD_REF(br_output_facility_sdl2, isType),
    ._device     = BR_CMETHOD_REF(br_output_facility_sdl2, device),
    ._space      = BR_CMETHOD_REF(br_output_facility_sdl2, space),

    ._templateQuery = BR_CMETHOD_REF(br_output_facility_sdl2, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery        = BR_CMETHOD_REF(br_output_facility_sdl2, listQuery),
    ._tokensMatchBegin = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch      = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd   = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._addFront         = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront      = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove           = BR_CMETHOD_REF(br_object_container, remove),
    ._find             = BR_CMETHOD_REF(br_object_container, find),
    ._findMany         = BR_CMETHOD_REF(br_object_container, findMany),
    ._count            = BR_CMETHOD_REF(br_object_container, count),

    ._validSource     = BR_CMETHOD_REF(br_output_facility_sdl2, validSource),
    ._pixelmapNew     = BR_CMETHOD_REF(br_output_facility_sdl2, pixelmapNew),
    ._clutNew         = BR_CMETHOD_REF(br_output_facility_sdl2, clutNew),
    ._queryCapability = BR_CMETHOD_REF(br_output_facility_sdl2, queryCapability),
};
