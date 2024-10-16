#include "drv.h"
#include "pm.h"
#include <brassert.h>

br_clip_result DevicePixelmapSDL3PointClip(SDL_Point *out, const br_point *in, const br_pixelmap *pm)
{
    br_clip_result r;
    br_point       tmp;

    if((r = PixelmapPointClip(&tmp, in, pm)) == BR_CLIP_REJECT)
        return BR_CLIP_REJECT;

    *out = (SDL_Point){
        .x = tmp.x,
        .y = tmp.y,
    };

    return r;
}

br_clip_result DevicePixelmapSDL3LineClip(SDL_Point *s_out, SDL_Point *e_out, const br_point *s_in,
                                          const br_point *e_in, const br_pixelmap *pm)
{
    br_clip_result r;
    br_point       stmp, etmp;

    if((r = PixelmapLineClip(&stmp, &etmp, s_in, e_in, pm)) == BR_CLIP_REJECT)
        return BR_CLIP_REJECT;

    *s_out = (SDL_Point){
        .x = stmp.x,
        .y = stmp.y,
    };

    *e_out = (SDL_Point){
        .x = etmp.x,
        .y = etmp.y,
    };
    return r;
}

br_clip_result DevicePixelmapSDL3RectangleClip(SDL_Rect *out, const br_rectangle *in, const br_pixelmap *pm)
{
    br_clip_result r;
    br_rectangle   tmp;

    if((r = PixelmapRectangleClip(&tmp, in, pm)) == BR_CLIP_REJECT)
        return BR_CLIP_REJECT;

    *out = (SDL_Rect){
        .x = tmp.x,
        .y = tmp.y,
        .w = tmp.w,
        .h = tmp.h,
    };

    return r;
}

br_clip_result DevicePixelmapSDL3RectangleClipTwo(SDL_Rect *r_out, SDL_Point *p_out, const br_rectangle *r_in,
                                                  const br_point *p_in, const br_pixelmap *pm_dst, const br_pixelmap *pm_src)
{
    br_clip_result r;
    br_rectangle   rect;
    br_point       point;

    if((r = PixelmapRectangleClipTwo(&rect, &point, r_in, p_in, pm_dst, pm_src)) == BR_CLIP_REJECT)
        return BR_CLIP_REJECT;

    *r_out = (SDL_Rect){
        .x = rect.x,
        .y = rect.y,
        .w = rect.w,
        .h = rect.h,
    };

    *p_out = (SDL_Point){
        .x = point.x,
        .y = point.y,
    };

    return r;
}

bool DevicePixelmapSDL3BlitScaled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect)
{
    return SDL_BlitSurfaceScaled(src, srcrect, dst, dstrect, SDL_SCALEMODE_NEAREST);
}

SDL_Surface *DevicePixelmapSDL3GetSurface(br_pixelmap *pm, br_boolean ignore_locked)
{
    br_device_pixelmap *self = (br_device_pixelmap *)pm;
    SDL_Surface        *surf = NULL;

    /*
     * If we're one of ours, get it directly, otherwise query the token.
     */
    if(DevicePixelmapSDL3IsOurs(pm))
        surf = self->surface;
    else if(ObjectQuery(pm, &surf, brt_sdl_surface_h) != BRE_OK)
        return NULL;

    if((surf->flags & SDL_SURFACE_LOCKED) && ignore_locked != BR_TRUE)
        return NULL;

    return surf;
}

SDL_Surface *DevicePixelmapSDL3CreateSurface(int width, int height, SDL_PixelFormat format)
{
    SDL_Surface *surf;

    if((surf = SDL_CreateSurface(width, height, format)) == NULL)
        return NULL;

    if(SDL_ISPIXELFORMAT_INDEXED(format) && SDL_CreateSurfacePalette(surf) == NULL) {
        SDL_DestroySurface(surf);
        return NULL;
    }

    return surf;
}

br_device_clut *DevicePixelmapSDL3GetCLUT(br_pixelmap *pm)
{
    br_device_clut *clut = NULL;

    if(DevicePixelmapSDL3IsOurs(pm))
        return ((br_device_pixelmap *)pm)->clut;

    if(ObjectQuery(pm, &clut, BRT_CLUT_O) != BRE_OK)
        return NULL;

    return clut;
}

static br_error maybe_allocate_surface(SDL_Surface **out_surf, br_boolean *changed, br_pixelmap *pm, br_device_clut *dst_clut)
{
    br_error        result;
    Uint32          format;
    SDL_Surface    *surf;
    int             bpp;
    br_device_clut *clut;

    if(*out_surf != NULL) {
        *changed = BR_FALSE;
        return BRE_OK;
    }

    if(pm->pixels == NULL || (pm->flags & BR_PMF_NO_ACCESS))
        return BRE_UNSUPPORTED;

    if((result = BRenderToSDLPixelFormat(pm->type, &format, &bpp)) != BRE_OK)
        return result;

    /*
     * If we're blitting from an indexed format, there's a few things to keep in mind:
     *   1. BRender pixelmaps may not have a map or a CLUT attached, in which case it's expected
     *      that the destination's CLUT is used.
     *   2. The destination may not have a CLUT (i.e. it won't if it's not indexed).
     *  In this case, just fail the blit.
     */
    if(SDL_ISPIXELFORMAT_INDEXED(format)) {
        clut = DevicePixelmapSDL3GetCLUT(pm);
        if(pm->map == NULL && clut == NULL && dst_clut == NULL)
            return BRE_UNSUPPORTED;
    }

    if((surf = SDL_CreateSurfaceFrom(pm->width, pm->height, format, pm->pixels, pm->row_bytes)) == NULL)
        return BRE_FAIL;

    /*
     * Now actually handle all the CLUT stuff mentioned above.
     */
    if(SDL_ISPIXELFORMAT_INDEXED(format)) {
        UASSERT(!(pm->map == NULL && clut == NULL && dst_clut == NULL));

        /*
         * The new surface won't have a CLUT. Add one.
         */
        if(SDL_CreateSurfacePalette(surf) == NULL) {
            SDL_DestroySurface(surf);
            return result;
        }

        if((result = DeviceClutSDL3CopyToSurface(surf, pm, dst_clut)) != BRE_OK) {
            SDL_DestroySurface(surf);
            return result;
        }
    }

    *changed  = BR_TRUE;
    *out_surf = surf;
    return BRE_OK;
}

br_error DeviceSDL3SetPalette(SDL_Palette *pal, br_int_32 index, br_int_32 count, const br_colour *entries, br_boolean has_alpha)
{
    SDL_Color cols[MAX_CLUT_ENTRIES];

    if(count > MAX_CLUT_ENTRIES)
        return BRE_OVERFLOW;

    if(index < 0 || index >= pal->ncolors)
        return BRE_OVERFLOW;

    if(index + count > pal->ncolors)
        return BRE_OVERFLOW;

    for(br_int_32 i = 0; i < count; ++i) {
        cols[i] = (SDL_Color){
            .r = BR_RED(entries[i]),
            .g = BR_GRN(entries[i]),
            .b = BR_BLU(entries[i]),
            .a = has_alpha ? BR_ALPHA(entries[i]) : 0xFF,
        };
    }

    if(!SDL_SetPaletteColors(pal, cols, index, count))
        return BRE_FAIL;

    return BRE_OK;
}

br_error DeviceSDL3SetPaletteFromCLUT(SDL_Palette *pal, br_device_clut *clut)
{
    br_error  r;
    br_colour colours[MAX_CLUT_ENTRIES];

    UASSERT(pal != NULL);
    UASSERT(clut != NULL);

    /*
     * If the CLUT is one of ours, do it directly.
     */
    if(DeviceClutSDL3IsOurs(clut)) {
        if(pal == clut->pal)
            return BRE_OK;

        if(pal->ncolors != clut->pal->ncolors)
            return BRE_UNSUPPORTED;

        if(!SDL_SetPaletteColors(pal, clut->pal->colors, 0, clut->pal->ncolors))
            return BRE_FAIL;

        return BRE_OK;
    }

    /*
     * Otherwise, go via dispatch.
     */
    if((r = DeviceClutEntryQueryMany(clut, colours, 0, pal->ncolors)) != BRE_OK)
        return r;

    return DeviceSDL3SetPalette(pal, 0, pal->ncolors, colours, BR_FALSE);
}

br_error DeviceSDL3SetPaletteFromMap(SDL_Palette *pal, const br_pixelmap *map)
{
    br_boolean has_alpha;

    if(map->width != 1 || map->height != pal->ncolors)
        return BRE_FAIL;

    if(map->pixels == NULL || (map->flags & BR_PMF_NO_ACCESS) != 0)
        return BRE_FAIL;

    switch(map->type) {
        case BR_PMT_RGBX_888:
            has_alpha = BR_FALSE;
            break;
        case BR_PMT_RGBA_8888:
            has_alpha = BR_TRUE;
            break;
        default:
            /* Have never seen CLUTs that aren't these formats. */
            return BRE_FAIL;
    }

    return DeviceSDL3SetPalette(pal, 0, map->height, map->pixels, has_alpha);
}

br_error DeviceClutSDL3CopyToSurface(SDL_Surface *surf, br_pixelmap *pm, br_device_clut *fallback)
{
    br_device_clut *cluts[2];
    SDL_Palette    *pal;
    UASSERT(SDL_ISPIXELFORMAT_INDEXED(surf->format));

    pal = SDL_GetSurfacePalette(surf);

    /*
     * If a map is explicitly set, use it.
     */
    if(pm->map != NULL)
        return DeviceSDL3SetPaletteFromMap(pal, pm->map);

    /*
     * Otherwise, fall back to the pixelmap's CLUT (if any), and our fallback CLUT.
     */
    cluts[0] = DevicePixelmapSDL3GetCLUT(pm);
    cluts[1] = fallback;

    for(size_t i = 0; i < BR_ASIZE(cluts); ++i) {
        if(cluts[i] == NULL)
            continue;

        return DeviceSDL3SetPaletteFromCLUT(pal, cluts[i]);
    }

    return BRE_FAIL;
}

static bool xblit(SDL_Surface *src, const SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr, br_sdl_blit_cbfn blit)
{
    /*
     * Handle two special cases that SDL doesn't support.
     *   1. Regular blitting from INDEX* -> XRGB8888
     *   2. Scaled  blitting from INDEX* -> anything
     *
     * Go via an intermediate surface (possibly again). This is slow, and indexed pixelmaps
     * should be de-indexed at load-time. See BrPixelmapDeCLUT().
     */
    if(SDL_ISPIXELFORMAT_INDEXED(src->format) &&
       (dst->format == SDL_PIXELFORMAT_XRGB8888 || blit == DevicePixelmapSDL3BlitScaled)) {
        int          r;
        SDL_Surface *newsrc;

        if((newsrc = SDL_ConvertSurface(src, SDL_PIXELFORMAT_XRGB8888)) == NULL)
            return false;

        r = blit(newsrc, sr, dst, dr);
        SDL_DestroySurface(newsrc);
        return r;
    }

    return blit(src, sr, dst, dr);
}

br_error DevicePixelmapSDL3BlitSurface(br_pixelmap *src, SDL_Rect *sr, br_pixelmap *dst, SDL_Rect *dr, br_sdl_blit_cbfn blit)
{
    br_error     result;
    SDL_Surface *src_surf, *dst_surf;
    br_boolean   free_src = BR_FALSE, free_dst = BR_FALSE;

    // TODO: handle colour keyed copy

    dst_surf = DevicePixelmapSDL3GetSurface(dst, BR_FALSE);
    src_surf = DevicePixelmapSDL3GetSurface(src, BR_FALSE);

    /*
     * If both surfaces expose an SDL surface - we can blit directly.
     */
    if(dst_surf != NULL && src_surf != NULL) {
        if(xblit(src_surf, sr, dst_surf, dr, blit))
            return BRE_OK;

        BrLogTrace("SDL3", "Blit from %s->%s failed: %s", src->identifier, dst->identifier, SDL_GetError());
    }

    /*
     * Otherwise, fall back to creating temporary surfaces.
     * Prefer doing this over using BRender's in-memory implementation, as it can't handle conversions between
     * the various pixel formats. All at the cost of potentially allocating two SDL_Surface's.
     */
    result = BRE_FAIL;

    if((result = maybe_allocate_surface(&src_surf, &free_src, src, DevicePixelmapSDL3GetCLUT(dst))) != BRE_OK)
        goto cleanup;

    if((result = maybe_allocate_surface(&dst_surf, &free_dst, dst, NULL)) != BRE_OK)
        goto cleanup;

    if(xblit(src_surf, sr, dst_surf, dr, blit))
        result = BRE_OK;
    else
        BrLogTrace("SDL3", "Blit from %s->%s failed: %s", src->identifier, dst->identifier, SDL_GetError());

cleanup:

    if(free_dst != BR_FALSE)
        SDL_DestroySurface(dst_surf);

    if(free_src != BR_FALSE)
        SDL_DestroySurface(src_surf);

    return result;
}

SDL_Color DevicePixelmapSDL3GetSurfaceColour(SDL_Surface *surf, br_colour colour)
{
    const SDL_PixelFormatDetails *pfd = SDL_GetPixelFormatDetails(surf->format);
    SDL_Color                     col;

    SDL_GetRGBA(colour, pfd, SDL_GetSurfacePalette(surf), &col.r, &col.g, &col.b, &col.a);
    return col;
}
