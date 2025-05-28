
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
    br_pixelmap *pixelmap, *tmp;
    int          x, y, c;
    stbi_uc     *pixels;
    void        *file;

    /* open file */
    file = BrFileOpenRead(name, 0, NULL, NULL);
    if(file == NULL) {
        BrLogError("FMT", "Failed to open \"%s\"", name);
        return NULL;
    }

    /*
     * Get pixels, always request 4 components.
     * Makes our lives easier.
     */
    pixels = stbi_load_from_callbacks(&br_stb_file_cbfns, file, &x, &y, &c, 4);
    if(pixels == NULL) {
        BrLogError("FMT", "Failed to process \"%s\". Reason: %s", name, stbi_failure_reason());
        return NULL;
    }

    /* close file */
    BrFileClose(file);

    /*
     * pixels isn't a resource so we can't use it directly.
     * Instead, shove it in a temporary pixelmap, then convert it to BR_PMT_RGBA_8888.
     */
    tmp             = BrPixelmapAllocate(BR_PMT_RGBA_8888_ARR, x, y, pixels, BR_PMAF_NORMAL);
    tmp->identifier = (char *)name; /* NB: This is safe, the clone below will copy it. */

    pixelmap = BrPixelmapCloneTyped(tmp, BR_PMT_RGBA_8888);

    BrPixelmapFree(tmp);
    BrMemFree(pixels);

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
