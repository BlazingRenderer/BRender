/*
 * Pixelmap utility functions.
 */
#include "pm.h"
#include "brassert.h"

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapDeCLUT(br_pixelmap *src)
{
    br_pixelmap *newpm, *map;
    br_colour   key;

    // TODO: Support more than just BR_PMT_INDEX_8
    if(src == NULL || src->type != BR_PMT_INDEX_8 || (src->flags & BR_PMF_NO_ACCESS))
        return NULL;

    map = src->map;

    if(map == NULL || map->type != BR_PMT_RGBX_888 || map->width != 1 || (map->flags & BR_PMF_NO_ACCESS))
        return NULL;

    newpm = BrPixelmapAllocate(
        (src->flags & BR_PMF_KEYED_TRANSPARENCY) ? BR_PMT_RGBA_8888 : BR_PMT_RGBX_888,
        src->width, src->height, NULL, BR_PMAF_NORMAL
    );
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

            col  = BrPixelmapPixelGet(map, 0, index - map->origin_y);
            col |= BR_COLOUR_RGBA(0x00, 0x00, 0x00, 0xFF);

            /* Apply colour keying */
            if(src->flags & BR_PMF_KEYED_TRANSPARENCY) {
                br_uint_8 r = BR_RED(col);
                br_uint_8 g = BR_GRN(col);
                br_uint_8 b = BR_BLU(col);
                col = BR_COLOUR_RGBA(r, g, b, BR_COLOUR_RGB(r, g, b) == key ? 0x00 : 0xFF);
            }

            BrPixelmapPixelSet(newpm, i, j, col);
        }
    }

    return newpm;
}
