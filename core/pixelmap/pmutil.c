/*
 * Pixelmap utility functions.
 */
#include "pm.h"
#include "brassert.h"

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapDeCLUT(br_pixelmap *src)
{
    br_pixelmap *newpm, *map;
    br_colour    key;

    // TODO: Support more than just BR_PMT_INDEX_8
    if(src == NULL || src->type != BR_PMT_INDEX_8 || (src->flags & BR_PMF_NO_ACCESS))
        return NULL;

    map = src->map;

    if(map == NULL || map->type != BR_PMT_RGBX_888 || map->width != 1 || (map->flags & BR_PMF_NO_ACCESS))
        return NULL;

    newpm = BrPixelmapAllocate((src->flags & BR_PMF_KEYED_TRANSPARENCY) ? BR_PMT_RGBA_8888 : BR_PMT_RGBX_888,
                               src->width, src->height, NULL, BR_PMAF_NORMAL);
    if(newpm == NULL)
        return NULL;

    if(src->identifier != NULL)
        newpm->identifier = BrResStrDup(newpm, src->identifier);

    newpm->origin_x = src->origin_x;
    newpm->origin_y = src->origin_y;

    /* Remove any alpha garbage */
    key = BR_COLOUR_RGB(BR_RED(map->key), BR_GRN(map->key), BR_BLU(map->key));

    for(br_int_32 j = -src->origin_y; j < src->origin_y; ++j) {
        for(br_int_32 i = -src->origin_x; i < src->origin_x; ++i) {
            br_int_32 index;
            br_colour col;

            index = (br_int_32)BrPixelmapPixelGet(src, i, j);
            UASSERT(index <= 0xFF);

            col = BrPixelmapPixelGet(map, 0, index - map->origin_y);
            col |= BR_COLOUR_RGBA(0x00, 0x00, 0x00, 0xFF);

            /* Apply colour keying */
            if(src->flags & BR_PMF_KEYED_TRANSPARENCY) {
                br_uint_8 r = BR_RED(col);
                br_uint_8 g = BR_GRN(col);
                br_uint_8 b = BR_BLU(col);
                col         = BR_COLOUR_RGBA(r, g, b, BR_COLOUR_RGB(r, g, b) == key ? 0x00 : 0xFF);
            }

            BrPixelmapPixelSet(newpm, i, j, col);
        }
    }

    return newpm;
}

br_error BrPixelmapResizeBuffersTV(br_pixelmap *screen, br_pixelmap **colour, br_pixelmap **depth, const br_token_value *tv)
{
    br_pixelmap   *tmp;
    br_int_32      msaa_samples = -1;
    size_t         tvidx        = 0;
    br_token_value tva[]        = {
        {.t = BR_NULL_TOKEN, .v = {}},
        {.t = BR_NULL_TOKEN, .v = {}},
        {.t = BR_NULL_TOKEN, .v = {}},
        {.t = BR_NULL_TOKEN, .v = {}},
    };

    if(screen == NULL || colour == NULL || depth == NULL)
        return BRE_FAIL;

    for(const br_token_value *t = tv; t->t != BR_NULL_TOKEN; ++t) {
        if(t->t == BRT_MSAA_SAMPLES_I32) {
            msaa_samples = t->v.i32;
            break;
        }
    }

    /*
     * Try to resize the framebuffer directly. Fall back to recreation if we can't.
     */
    if(*colour != NULL && *depth != NULL) {
        tmp = *colour;
        if(tmp->width != screen->width || tmp->height != screen->height) {
            if((tmp = BrPixelmapResize(tmp, screen->width, screen->height)) == NULL)
                goto full_cleanup;

            *colour       = tmp;
            tmp->origin_x = screen->origin_x;
            tmp->origin_y = screen->origin_y;
        }

        tmp = *depth;
        if(tmp->width != screen->width || tmp->height != screen->height) {
            if((tmp = BrPixelmapResize(*depth, screen->width, screen->height)) == NULL)
                goto full_cleanup;

            *depth        = tmp;
            tmp->origin_x = screen->origin_x;
            tmp->origin_y = screen->origin_y;
        }
        return BRE_OK;
    }

full_cleanup:

    /*
     * Clear the screen, just in case.
     */
    if(*colour != NULL) {
        BrPixelmapFill(*colour, 0);
        BrPixelmapDoubleBuffer(screen, *colour);
    }

    /*
     * Delete everything.
     */
    if(*depth != NULL) {
        BrPixelmapFree(*depth);
        *depth = NULL;
    }

    if(*colour != NULL) {
        BrPixelmapFree(*colour);
        *colour = NULL;
    }

    /*
     * Recreate the colour buffer.
     */
    tvidx = 0;

    tva[tvidx].t   = BRT_USE_T;
    tva[tvidx].v.t = BRT_OFFSCREEN;
    ++tvidx;

    tva[tvidx].t    = BRT_PIXEL_TYPE_U8;
    tva[tvidx].v.u8 = BR_PMT_INDEX_8;//screen->type;
    ++tvidx;

    if(msaa_samples > 0) {
        tva[tvidx].t     = BRT_MSAA_SAMPLES_I32;
        tva[tvidx].v.i32 = msaa_samples;
        ++tvidx;
    }

    tva[tvidx].t = BR_NULL_TOKEN;

    if((tmp = BrPixelmapMatchTV(screen, tva)) == NULL)
        return BRE_FAIL;

    *colour       = tmp;
    tmp->origin_x = screen->origin_x;
    tmp->origin_y = screen->origin_y;

    /*
     * Recreate the depth buffer.
     */
    tvidx = 0;

    tva[tvidx].t   = BRT_USE_T;
    tva[tvidx].v.t = BRT_DEPTH;
    ++tvidx;

    tva[tvidx].t     = BRT_PIXEL_BITS_I32;
    tva[tvidx].v.i32 = 16;
    ++tvidx;

    if(msaa_samples > 0) {
        tva[tvidx].t     = BRT_MSAA_SAMPLES_I32;
        tva[tvidx].v.i32 = msaa_samples;
        ++tvidx;
    }

    tva[tvidx].t = BR_NULL_TOKEN;

    if((tmp = BrPixelmapMatchTV(*colour, tva)) == NULL) {
        BrPixelmapFree(*colour);
        *colour = NULL;
        return BRE_FAIL;
    }

    *depth        = tmp;
    tmp->origin_x = screen->origin_x;
    tmp->origin_y = screen->origin_y;

    return BRE_OK;
}

br_error BrPixelmapResizeBuffers(br_pixelmap *screen, br_pixelmap **colour, br_pixelmap **depth)
{
    br_token_value tv = {.t = BR_NULL_TOKEN, .v = {}};
    return BrPixelmapResizeBuffersTV(screen, colour, depth, &tv);
}
