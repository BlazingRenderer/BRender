#include <brender.h>

static void make_board(br_pixelmap *pm, br_uint_32 black, br_uint_32 magenta)
{
    for(br_int_32 j = 0; j < pm->height; ++j) {
        for(br_int_32 i = 0; i < pm->width; ++i) {
            if(j < pm->height / 2) {
                if(i < pm->width / 2)
                    BrPixelmapPixelSet(pm, i, j, black);
                else
                    BrPixelmapPixelSet(pm, i, j, magenta);
            }
            else {
                if(i < pm->width / 2)
                    BrPixelmapPixelSet(pm, i, j, magenta);
                else
                    BrPixelmapPixelSet(pm, i, j, black);
            }
        }
    }
}

br_pixelmap *mkres_make_checkerboard_pixelmap(const char *name)
{
    br_pixelmap *pm;

    if((pm = BrPixelmapAllocate(BR_PMT_RGB_565, 64, 64, NULL, 0)) == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, name);

    make_board(pm, BR_COLOUR_565(0, 0, 0), BR_COLOUR_565(0xFF, 0x00, 0xFF));

    BrMapAdd(pm);
    return pm;
}

br_pixelmap *mkres_make_checkerboard8_pixelmap(const char *name)
{
    br_pixelmap *pal, *pm;
    br_colour *col;
    const int BLACK   = 1;
    const int MAGENTA = 2;

    /* Build the palette */
    if((pal = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, 0)) == NULL)
        return NULL;

    BrSprintf(BrScratchString(), "%s (palette)", name);
    pal->identifier = BrResStrDup(pal, BrScratchString());

    /* NB: anything at index 0 is transparent. */
    col = pal->pixels;
    col[BLACK]   = BR_COLOUR_RGB(0x00, 0x00, 0x00);
    col[MAGENTA] = BR_COLOUR_RGB(0xFF, 0x00, 0xFF);

    /* Build the map. */
    if((pm = BrPixelmapAllocate(BR_PMT_INDEX_8, 64, 64, NULL, 0)) == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, name);
    pm->map = pal;

    make_board(pm, BLACK, MAGENTA);

    BrMapAdd(pm);
    return pm;
}

br_material *mkres_make_checkerboard_material(const char *name, br_pixelmap *pm)
{
    br_material *mat;

    if((mat = BrMaterialAllocate(name)) == NULL)
        return NULL;

    mat->flags |= BR_MATF_DISABLE_COLOUR_KEY;
    mat->colour_map = pm;

    BrMaterialAdd(mat);
    return mat;
}
