
#include "brender.h"
#include "fmt.h"

#define DR_PCX_IMPLEMENTATION
#define DR_PCX_NO_STDIO
#include "dr_pcx.h"

/*
 * callback function so we don't use stdio
 */
size_t br_drpcx_read_cbfn(void *file, void *out, size_t size)
{
    return BrFileRead(out, 1, size, file);
}

/*
 * use drpcx to load PCX to pixelmap
 */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtPCXLoad(const char *name)
{
    br_pixelmap *pm;
    drpcx_uint8 *pixels;
    void        *file;
    int          w, h, c;
    int          type;
    br_uint_8   *pixel;
    br_colour    colour;

    /*
     * open file
     */
    file = BrFileOpenRead(name, 0, NULL, NULL);
    if(file == NULL) {
        BrLogError("FMT", "Failed to open \"%s\".", name);
        return NULL;
    }

    /*
     * parse file
     */
    pixels = drpcx_load(br_drpcx_read_cbfn, file, DRPCX_FALSE, &w, &h, &c, 0);
    if(pixels == NULL || w < 1 || h < 1 || c < 3 || c > 4) {
        BrLogError("FMT", "Failed to parse \"%s\".", name);
        return NULL;
    }

    /*
     * determine type
     */
    switch(c) {
        case 3:
            type = BR_PMT_RGB_888;
            break;

        case 4:
            type = BR_PMT_RGBA_8888;
            break;

        default:
            BrLogError("FMT", "Unsupported numer of pixel components %d in file \"%s\".", c, name);
            return NULL;
    }

    /*
     * allocate pm
     */
    pm = BrPixelmapAllocate(type, w, h, NULL, BR_PMAF_NORMAL);
    if(pm == NULL) {
        BrLogError("FMT", "Failed to allocate pixelmap.");
        return NULL;
    }

    /*
     * copy name
     */
    pm->identifier = BrResStrDup(pm, name);

    /*
     * copy pixels
     */
    pixel = (br_uint_8 *)pixels;
    for(br_int_32 y = 0; y < h; ++y) {
        for(br_int_32 x = 0; x < w; ++x) {

            /*
             * create brender colour
             */
            if(c == 4) {
                colour = BR_COLOUR_RGBA(pixel[0], pixel[1], pixel[2], pixel[3]);
            } else {
                colour = BR_COLOUR_RGB(pixel[0], pixel[1], pixel[2]);
            }

            /*
             * set destination colour
             */
            BrPixelmapPixelSet(pm, -pm->origin_x + x, -pm->origin_y + y, colour);

            /*
             * move pixel ptr
             */
            pixel += c;
        }
    }

    /*
     * free memory
     */
    drpcx_free(pixels);
    BrFileClose(file);

    return pm;
}
