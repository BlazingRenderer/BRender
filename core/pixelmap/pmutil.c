/*
 * Pixelmap utility functions.
 */
#include "pm.h"
#include "brassert.h"

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapDeCLUT(br_pixelmap *src)
{
    if(src == NULL || src->type > BR_PMT_INDEX_8 || (src->flags & BR_PMF_NO_ACCESS))
        return NULL;

    return BrPixelmapCloneTyped(src, BR_PMT_RGBA_8888);
}

br_error BrPixelmapResizeBuffersTV(br_pixelmap *screen, br_pixelmap **colour, br_pixelmap **depth, const br_token_value *tv)
{
    br_pixelmap   *tmp;
    br_int_32      msaa_samples = -1;
    br_int_32      target_width;
    br_int_32      target_height;
    br_uint_8      target_type;
    br_int_32      target_depth_bits;
    size_t         tvidx, tvbase;
    br_token_value tva[] = {
        {.t = BR_NULL_TOKEN, .v = {}},
        {.t = BR_NULL_TOKEN, .v = {}},
        {.t = BR_NULL_TOKEN, .v = {}},
        {.t = BR_NULL_TOKEN, .v = {}},
        {.t = BR_NULL_TOKEN, .v = {}},
        {.t = BR_NULL_TOKEN, .v = {}},
    };

    if(screen == NULL || colour == NULL || depth == NULL)
        return BRE_FAIL;

    /*
     * Default to the screen's width/height/type.
     */
    target_width      = screen->width;
    target_height     = screen->height;
    target_type       = screen->type;
    target_depth_bits = 32;

    for(const br_token_value *t = tv; t->t != BR_NULL_TOKEN; ++t) {
        if(t->t == BRT_MSAA_SAMPLES_I32) {
            msaa_samples = t->v.i32;
        } else if(t->t == BRT_WIDTH_I32) {
            target_width = t->v.i32;
        } else if(t->t == BRT_HEIGHT_I32) {
            target_height = t->v.i32;
        } else if(t->t == BRT_PIXEL_TYPE_U8) {
            target_type = t->v.u8;
        } else if(t->t == BRT_PIXEL_BITS_I32) {
            target_depth_bits = t->v.i32;
        }
    }

    if(target_width <= 0 || target_height <= 0)
        return BRE_FAIL;

    if(target_depth_bits < 16)
        target_depth_bits = 16;

    /*
     * Try to resize the framebuffer directly. Fall back to recreation if we can't.
     */
    if(*colour != NULL && *depth != NULL) {
        tmp = *colour;

        if(tmp->type != target_type)
            goto full_cleanup;

        if(tmp->width != target_width || tmp->height != target_height) {
            if((tmp = BrPixelmapResize(tmp, target_width, target_height)) == NULL)
                goto full_cleanup;

            *colour = tmp;
        }

        tmp->origin_x = (br_int_16)(tmp->width >> 1);
        tmp->origin_y = (br_int_16)(tmp->height >> 1);

        tmp = *depth;
        if(tmp->width != target_width || tmp->height != target_height) {
            if((tmp = BrPixelmapResize(*depth, target_width, target_height)) == NULL)
                goto full_cleanup;

            *depth = tmp;
        }

        tmp->origin_x = (br_int_16)(tmp->width >> 1);
        tmp->origin_y = (br_int_16)(tmp->height >> 1);
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
     * Prepare common tokens.
     */
    tvbase            = 0;
    tva[tvbase].t     = BRT_WIDTH_I32;
    tva[tvbase].v.i32 = target_width;
    ++tvbase;

    tva[tvbase].t     = BRT_HEIGHT_I32;
    tva[tvbase].v.i32 = target_height;
    ++tvbase;

    if(msaa_samples > 0) {
        tva[tvbase].t     = BRT_MSAA_SAMPLES_I32;
        tva[tvbase].v.i32 = msaa_samples;
        ++tvbase;
    }

    /*
     * Recreate the colour buffer.
     */
    tvidx = tvbase;

    tva[tvidx].t   = BRT_USE_T;
    tva[tvidx].v.t = BRT_OFFSCREEN;
    ++tvidx;

    tva[tvidx].t    = BRT_PIXEL_TYPE_U8;
    tva[tvidx].v.u8 = target_type;
    ++tvidx;

    tva[tvidx].t = BR_NULL_TOKEN;

    if((tmp = BrPixelmapMatchTV(screen, tva)) == NULL)
        return BRE_FAIL;

    *colour       = tmp;
    tmp->origin_x = (br_int_16)(tmp->width >> 1);
    tmp->origin_y = (br_int_16)(tmp->height >> 1);

    /*
     * Recreate the depth buffer.
     */
    tvidx = tvbase;

    tva[tvidx].t   = BRT_USE_T;
    tva[tvidx].v.t = BRT_DEPTH;
    ++tvidx;

    tva[tvidx].t     = BRT_PIXEL_BITS_I32;
    tva[tvidx].v.i32 = target_depth_bits;
    ++tvidx;

    tva[tvidx].t = BR_NULL_TOKEN;

    if((tmp = BrPixelmapMatchTV(*colour, tva)) == NULL) {
        BrPixelmapFree(*colour);
        *colour = NULL;
        return BRE_FAIL;
    }

    *depth        = tmp;
    tmp->origin_x = (br_int_16)(tmp->width >> 1);
    tmp->origin_y = (br_int_16)(tmp->height >> 1);

    return BRE_OK;
}

br_error BrPixelmapResizeBuffers(br_pixelmap *screen, br_pixelmap **colour, br_pixelmap **depth)
{
    br_token_value tv = {.t = BR_NULL_TOKEN, .v = {}};
    return BrPixelmapResizeBuffersTV(screen, colour, depth, &tv);
}
