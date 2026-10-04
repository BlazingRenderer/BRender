
#include "brender.h"
#include "fmt.h"
#include "brstb.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/*
 * Binary PPM (P6). PPM carries no alpha, so take three of every four bytes of
 * the RGBA the caller clones for us. Rows are written top-down as stored.
 *
 * stb_image_write has no PNM writer, which is why this one is hand-rolled;
 * stb_image (the loader) does read PNM, so this closes the round-trip.
 */
static int save_ppm(void *file, br_pixelmap *pm)
{
    char       header[64];
    br_int_32  header_len;
    br_uint_8 *row;

    header_len = BrSprintf(header, "P6\n%d %d\n255\n", (int)pm->width, (int)pm->height);

    if(BrFileWrite(header, (br_size_t)header_len, 1, file) != 1)
        return 0;

    if((row = BrMemAllocate((br_size_t)pm->width * 3, BR_MEMORY_SCRATCH)) == NULL)
        return 0;

    for(int y = 0; y < pm->height; ++y) {
        const br_uint_8 *src = (const br_uint_8 *)pm->pixels + (br_size_t)y * pm->row_bytes;

        for(int x = 0; x < pm->width; ++x) {
            row[x * 3 + 0] = src[x * 4 + 0];
            row[x * 3 + 1] = src[x * 4 + 1];
            row[x * 3 + 2] = src[x * 4 + 2];
        }

        if(BrFileWrite(row, (br_size_t)pm->width * 3, 1, file) != 1) {
            BrMemFree(row);
            return 0;
        }
    }

    BrMemFree(row);
    return 1;
}

/*
 * save pixelmap as image, enumerated by type BR_FMT_IMAGE_*
 */
br_uint_32 BR_PUBLIC_ENTRY BrFmtImageSave(const char *name, br_pixelmap *pm, br_uint_8 type)
{
    void        *file;
    int          ret;
    br_pixelmap *dst;

    /*
     * check type
     */
    if(type > BR_FMT_IMAGE_PPM || type < BR_FMT_IMAGE_PNG) {
        BrLogError("FMT", "Invalid image save type %u", type);
        return 0;
    }

    /*
     * open file for writing
     */
    file = BrFileOpenWrite(name, BR_FS_MODE_BINARY);
    if(file == NULL) {
        BrLogError("FMT", "Failed to open \"%s\" for writing.", name);
        return 0;
    }

    /*
     * check if we need to clone
     */
    if(pm->type == BR_PMT_RGBA_8888_ARR) {
        dst = pm;
    } else {
        dst = BrPixelmapCloneTyped(pm, BR_PMT_RGBA_8888_ARR);
        if(dst == NULL) {
            BrLogError("FMT", "Failed to clone \"%s\".", pm->identifier);
            return 0;
        }
    }

    /*
     * write the pixelmap
     */
    switch(type) {
        case BR_FMT_IMAGE_PNG:
            ret = stbi_write_png_to_func(FmtSTBFileWrite, file, dst->width, dst->height, 4, dst->pixels, dst->row_bytes);
            break;

        case BR_FMT_IMAGE_JPG:
            ret = stbi_write_jpg_to_func(FmtSTBFileWrite, file, dst->width, dst->height, 4, dst->pixels, 100);
            break;

        case BR_FMT_IMAGE_BMP:
            ret = stbi_write_bmp_to_func(FmtSTBFileWrite, file, dst->width, dst->height, 4, dst->pixels);
            break;

        case BR_FMT_IMAGE_TGA:
            ret = stbi_write_tga_to_func(FmtSTBFileWrite, file, dst->width, dst->height, 4, dst->pixels);
            break;

        case BR_FMT_IMAGE_PPM:
            ret = save_ppm(file, dst);
            break;
    }

    /*
     * check for error
     */
    if(!ret) {
        BrLogError("FMT", "Failed to write \"%s\".", name);
    }

    /*
     * free our data
     */
    BrFileClose(file);
    if(pm->type != BR_PMT_RGBA_8888_ARR) {
        BrPixelmapFree(dst);
    }

    return ret;
}
