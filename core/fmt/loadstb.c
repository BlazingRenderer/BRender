
/* brender headers */
#include "brender.h"
#include "fmt.h"
#include "brstb.h"

/* stb implementation */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* load with stb_image (private) */
static br_pixelmap *BrFmtSTBLoad(const char *name)
{
    /* variables */
    br_pixelmap *pixelmap;
    int          x, y, c, i;
    br_uint_8    type;
    stbi_uc     *pixels;
    void        *file;

    /* open file */
    file = BrFileOpenRead(name, 0, NULL, NULL);
    if(file == NULL) {
        BrLogError("FMT", "Failed to open \"%s\"", name);
        return NULL;
    }

    /* get pixels */
    pixels = stbi_load_from_callbacks(&br_stb_file_cbfns, file, &x, &y, &c, 0);
    if(pixels == NULL) {
        BrLogError("FMT", "Failed to process \"%s\". Reason: %s", name, stbi_failure_reason());
        return NULL;
    }

    /* close file */
    BrFileClose(file);

    /* decide depth and swap based on endianness */
    switch(c) {
        /* RGB */
        case 3: {
            type = BR_PMT_RGB_888;
#if BR_ENDIAN_LITTLE
            i = 0;
            while(i < x * y * 3) {
                /* variables */
                br_uint_8 *pixel;
                br_uint_8  r, g, b;

                /* get pixel pointer */
                pixel = &(((br_uint_8 *)pixels)[i]);

                /* extract components */
                r = pixel[0];
                g = pixel[1];
                b = pixel[2];

                /* reassign */
                pixel[0] = b;
                pixel[1] = g;
                pixel[2] = r;

                /* advance 3 bytes */
                i += 3;
            }
#endif
            break;
        }

        /* RGBA */
        case 4: {
            type = BR_PMT_RGBA_8888;
            for(i = 0; i < x * y; i++) {
                /* variables */
                br_uint_32 *ptr;
                br_uint_8  *pixel;
                br_uint_8   r, g, b, a;

                /* get pixel pointer */
                ptr   = &(((br_uint_32 *)pixels)[i]);
                pixel = (br_uint_8 *)ptr;

                /* extract components */
                r = pixel[0];
                g = pixel[1];
                b = pixel[2];
                a = pixel[3];

                /* reassign */
                *ptr = BR_COLOUR_RGBA(r, g, b, a);
            }
            break;
        }

        /* unsupported */
        default:
            BrLogError("FMT", "Unhandled number of components for pixelmap: %d", c);
            return NULL;
    }

    /* allocate pixmap */
    pixelmap = BrPixelmapAllocate(type, x, y, pixels, 0);
    if(pixelmap == NULL) {
        BrLogError("FMT", "Failed to allocate pixelmap");
        return NULL;
    }

    /* add pixels to pixelmap */
    BrResAdd(pixelmap, pixels);

    /* return ptr */
    return pixelmap;
}

/* load png */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtPNGLoad(const char *name, br_uint_32 flags)
{
    return BrFmtSTBLoad(name);
}

/* load jpg */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtJPGLoad(const char *name, br_uint_32 flags)
{
    return BrFmtSTBLoad(name);
}
