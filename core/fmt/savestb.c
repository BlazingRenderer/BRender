
#include "brender.h"
#include "fmt.h"
#include "brstb.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
    if(type > BR_FMT_IMAGE_TGA || type < BR_FMT_IMAGE_PNG) {
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
            ret = stbi_write_png_to_func(FmtSTBFileRead, file, dst->width, dst->height, 4, dst->pixels, dst->row_bytes);
            break;

        case BR_FMT_IMAGE_JPG:
            ret = stbi_write_jpg_to_func(FmtSTBFileRead, file, dst->width, dst->height, 4, dst->pixels, 100);
            break;

        case BR_FMT_IMAGE_BMP:
            ret = stbi_write_bmp_to_func(FmtSTBFileRead, file, dst->width, dst->height, 4, dst->pixels);
            break;

        case BR_FMT_IMAGE_TGA:
            ret = stbi_write_tga_to_func(FmtSTBFileRead, file, dst->width, dst->height, 4, dst->pixels);
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
