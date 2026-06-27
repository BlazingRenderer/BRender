#define BR_DEVICE_PIXELMAP_PRIVATE
#include "pm.h"
#include "pmmem.h"
#include "pmclone.h"

/*
 * BR_PMT_INDEX_8
 */

static br_colour index_8_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 index;
    br_colour colour;
    br_uint_8 r, g, b;

    (void)opts;

    /*
     * get colour index
     */
    index = *pixels;

    /*
     * get rgba colour
     */
    colour = BrPixelmapPixelGet(pm->map, 0, index - pm->map->origin_y);

    /*
     * get colour components
     */
    r = BR_RED(colour);
    g = BR_GRN(colour);
    b = BR_BLU(colour);

    /*
     * 0 is the indexed transparency colour
     */
    if(index == 0)
        return BR_COLOUR_RGBA(r, g, b, 0);

    return BR_COLOUR_RGBA(r, g, b, 255);
}

static void index_8_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    br_uint_8 rgb[3] = {
        [0] = BR_RED(colour),
        [1] = BR_GRN(colour),
        [2] = BR_BLU(colour),
    };

    if(BR_ALPHA(colour) <= opts->index_alpha_threshold)
        *pixels = 0;
    else
        BrQuantMapColours(opts->target_clut ? 0 : 1, rgb, pixels, 1);
}

/*
 * BR_PMT_RGB_555
 */

static br_colour rgb_555_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b;

    (void)pm;
    (void)opts;

    r = BR_RED_555(*((br_uint_16 *)pixels));
    g = BR_GRN_555(*((br_uint_16 *)pixels));
    b = BR_BLU_555(*((br_uint_16 *)pixels));

    return BR_COLOUR_RGBA(r, g, b, 255);
}

static void rgb_555_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b;

    (void)opts;

    r = BR_RED(colour);
    g = BR_GRN(colour);
    b = BR_BLU(colour);

    *((br_uint_16 *)pixels) = BR_COLOUR_RGB_555(r, g, b);
}

/*
 * BR_PMT_RGB_565
 */

static br_colour rgb_565_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b;

    (void)pm;
    (void)opts;

    r = BR_RED_565(*((br_uint_16 *)pixels));
    g = BR_GRN_565(*((br_uint_16 *)pixels));
    b = BR_BLU_565(*((br_uint_16 *)pixels));

    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    return BR_COLOUR_RGBA(r, g, b, 255);
}

static void rgb_565_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b;

    (void)opts;

    r = BR_RED(colour);
    g = BR_GRN(colour);
    b = BR_BLU(colour);

    *((br_uint_16 *)pixels) = BR_COLOUR_RGB_565(r, g, b);
}

/*
 * BR_PMT_BGR_565
 */

static br_colour bgr_565_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b;

    (void)pm;
    (void)opts;

    r = BR_BLU_565(*((br_uint_16 *)pixels));
    g = BR_GRN_565(*((br_uint_16 *)pixels));
    b = BR_RED_565(*((br_uint_16 *)pixels));

    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    return BR_COLOUR_RGBA(r, g, b, 255);
}

static void bgr_565_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b;

    (void)opts;

    r = BR_RED(colour);
    g = BR_GRN(colour);
    b = BR_BLU(colour);

    *((br_uint_16 *)pixels) = BR_COLOUR_RGB_565(b, g, r);
}

/*
 * BR_PMT_RGB_888
 */

static br_colour rgb_888_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b;

    (void)pm;
    (void)opts;

    r = pixels[2];
    g = pixels[1];
    b = pixels[0];

    return BR_COLOUR_RGBA(r, g, b, 255);
}

static void rgb_888_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    (void)opts;

    pixels[2] = BR_RED(colour);
    pixels[1] = BR_GRN(colour);
    pixels[0] = BR_BLU(colour);
}

/*
 * BR_PMT_RGBX_888
 */

static br_colour rgbx_888_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    (void)pm;
    (void)opts;

    return *((br_colour *)pixels);
}

static void rgbx_888_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    (void)opts;

    *((br_colour *)pixels) = colour;
}

/*
 * BR_PMT_RGBA_8888
 */

static br_colour rgba_8888_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    (void)pm;
    (void)opts;

    return *((br_colour *)pixels);
}

static void rgba_8888_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    *((br_colour *)pixels) = colour;
}

/*
 * BR_PMT_BGR_555
 */

static br_colour bgr_555_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b;

    (void)pm;
    (void)opts;

    r = BR_BLU_555(*((br_uint_16 *)pixels));
    g = BR_GRN_555(*((br_uint_16 *)pixels));
    b = BR_RED_555(*((br_uint_16 *)pixels));

    return BR_COLOUR_RGBA(r, g, b, 255);
}

static void bgr_555_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b;

    (void)opts;

    r = BR_RED(colour);
    g = BR_GRN(colour);
    b = BR_BLU(colour);

    *((br_uint_16 *)pixels) = BR_COLOUR_BGR_555(r, g, b);
}

/*
 * BR_PMT_RGBA_4444
 */

static br_colour rgba_4444_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b, a;

    (void)pm;
    (void)opts;

    r = BR_RED_4444(*((br_uint_16 *)pixels));
    g = BR_GRN_4444(*((br_uint_16 *)pixels));
    b = BR_BLU_4444(*((br_uint_16 *)pixels));
    a = BR_ALPHA_4444(*((br_uint_16 *)pixels));

    r = (r >> 4) | r;
    g = (g >> 4) | g;
    b = (b >> 4) | b;
    a = (a >> 4) | a;

    return BR_COLOUR_RGBA(r, g, b, a);
}

static void rgba_4444_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b, a;

    (void)opts;

    r = BR_RED(colour);
    g = BR_GRN(colour);
    b = BR_BLU(colour);
    a = BR_ALPHA(colour);

    *((br_uint_16 *)pixels) = BR_COLOUR_RGBA_4444(r, g, b, a);
}

/*
 * BR_PMT_ARGB_4444
 */

static br_colour argb_4444_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b, a;

    (void)pm;
    (void)opts;

    r = BR_RED_4444(*((br_uint_16 *)pixels));
    g = BR_GRN_4444(*((br_uint_16 *)pixels));
    b = BR_BLU_4444(*((br_uint_16 *)pixels));
    a = BR_ALPHA_4444(*((br_uint_16 *)pixels));

    r = (r >> 4) | r;
    g = (g >> 4) | g;
    b = (b >> 4) | b;
    a = (a >> 4) | a;

    return BR_COLOUR_RGBA(r, g, b, a);
}

static void argb_4444_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b, a;

    (void)opts;

    r = BR_RED(colour);
    g = BR_GRN(colour);
    b = BR_BLU(colour);
    a = BR_ALPHA(colour);

    *((br_uint_16 *)pixels) = BR_COLOUR_ARGB_4444(r, g, b, a);
}

/*
 * BR_PMT_RGBA_8888_ARR
 */

static br_colour rgba_8888_arr_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b, a;

    (void)pm;
    (void)opts;

    r = pixels[0];
    g = pixels[1];
    b = pixels[2];
    a = pixels[3];

    return BR_COLOUR_RGBA(r, g, b, a);
}

static void rgba_8888_arr_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    (void)opts;

    pixels[0] = BR_RED(colour);
    pixels[1] = BR_GRN(colour);
    pixels[2] = BR_BLU(colour);
    pixels[3] = BR_ALPHA(colour);
}

static br_colour r8b8g8a8_read(const br_uint_8 *pixels, const br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b, a;

    br_uint_32 pixel = *(br_uint_32 *)pixels;

    (void)pm;
    (void)opts;

    r = (pixel & 0xFF000000) >> 24;
    g = (pixel & 0x00FF0000) >> 16;
    b = (pixel & 0x0000FF00) >> 8;
    a = (pixel & 0x000000FF) >> 0;

    return BR_COLOUR_ARGB(a, r, g, b);
}

static void r8g8b8a8_write(br_uint_8 *pixels, br_colour colour, const br_pixelmap_convert_options *opts)
{
    br_uint_8 r, g, b, a;

    (void)opts;

    r = BR_RED(colour);
    g = BR_GRN(colour);
    b = BR_BLU(colour);
    a = BR_ALPHA(colour);

    *(br_uint_32 *)pixels = (r << 24) | (g << 16) | (b << 8) | (a << 0);
}

/*
 * converters
 */

// clang-format off
/*
 * read function, write function, pixel type, pixel type string
 */
#define CONVERTER(r, w, t, n) [t] = {.read = r, .write = w, .type = t, .name = n}
br_pixelmap_converter br_pixelmap_converters[] = {
    CONVERTER(NULL,               NULL,                BR_PMT_INDEX_1,        "BR_PMT_INDEX_1"),
    CONVERTER(NULL,               NULL,                BR_PMT_INDEX_2,        "BR_PMT_INDEX_2"),
    CONVERTER(NULL,               NULL,                BR_PMT_INDEX_4,        "BR_PMT_INDEX_4"),
    CONVERTER(index_8_read,       index_8_write,       BR_PMT_INDEX_8,        "BR_PMT_INDEX_8"),
    CONVERTER(rgb_555_read,       rgb_555_write,       BR_PMT_RGB_555,        "BR_PMT_RGB_555"),
    CONVERTER(rgb_565_read,       rgb_565_write,       BR_PMT_RGB_565,        "BR_PMT_RGB_565"),
    CONVERTER(rgb_888_read,       rgb_888_write,       BR_PMT_RGB_888,        "BR_PMT_RGB_888"),
    CONVERTER(rgbx_888_read,      rgbx_888_write,      BR_PMT_RGBX_888,       "BR_PMT_RGBX_888"),
    CONVERTER(rgba_8888_read,     rgba_8888_write,     BR_PMT_RGBA_8888,      "BR_PMT_RGBA_8888"),
    CONVERTER(NULL,               NULL,                BR_PMT_YUYV_8888,      "BR_PMT_YUYV_8888"),
    CONVERTER(NULL,               NULL,                BR_PMT_YUV_888,        "BR_PMT_YUV_888"),
    CONVERTER(NULL,               NULL,                BR_PMT_DEPTH_16,       "BR_PMT_DEPTH_16"),
    CONVERTER(NULL,               NULL,                BR_PMT_DEPTH_32,       "BR_PMT_DEPTH_32"),
    CONVERTER(NULL,               NULL,                BR_PMT_ALPHA_8,        "BR_PMT_ALPHA_8"),
    CONVERTER(NULL,               NULL,                BR_PMT_INDEXA_88,      "BR_PMT_INDEXA_88"),
    CONVERTER(NULL,               NULL,                BR_PMT_NORMAL_INDEX_8, "BR_PMT_NORMAL_INDEX_8"),
    CONVERTER(NULL,               NULL,                BR_PMT_NORMAL_XYZ,     "BR_PMT_NORMAL_XYZ"),
    CONVERTER(bgr_555_read,       bgr_555_write,       BR_PMT_BGR_555,        "BR_PMT_BGR_555"),
    CONVERTER(rgba_4444_read,     rgba_4444_write,     BR_PMT_RGBA_4444,      "BR_PMT_RGBA_4444"),
    CONVERTER(NULL,               NULL,                BR_PMT_RBG_bab,        "BR_PMT_RBG_bab"),
    CONVERTER(NULL,               NULL,                BR_PMT_RBG_1aba,       "BR_PMT_RBG_1aba"),
    CONVERTER(NULL,               NULL,                BR_PMT_RGB_332,        "BR_PMT_RGB_332"),
    CONVERTER(NULL,               NULL,                BR_PMT_DEPTH_8,        "BR_PMT_DEPTH_8"),
    CONVERTER(NULL,               NULL,                BR_PMT_ARGB_8888,      "BR_PMT_ARGB_8888"),
    CONVERTER(NULL,               NULL,                BR_PMT_ALPHA_4,        "BR_PMT_ALPHA_4"),
    CONVERTER(NULL,               NULL,                BR_PMT_INDEXA_44,      "BR_PMT_INDEXA_44"),
    CONVERTER(NULL,               NULL,                BR_PMT_DEPTH_15,       "BR_PMT_DEPTH_15"),
    CONVERTER(NULL,               NULL,                BR_PMT_DEPTH_31,       "BR_PMT_DEPTH_31"),
    CONVERTER(NULL,               NULL,                BR_PMT_DEPTH_FP16,     "BR_PMT_DEPTH_FP16"),
    CONVERTER(NULL,               NULL,                BR_PMT_DEPTH_FP15,     "BR_PMT_DEPTH_FP15"),
    CONVERTER(NULL,               NULL,                BR_PMT_RGBA_5551,      "BR_PMT_RGBA_5551"),
    CONVERTER(NULL,               NULL,                BR_PMT_ARGB_1555,      "BR_PMT_ARGB_1555"),
    CONVERTER(argb_4444_read,     argb_4444_write,     BR_PMT_ARGB_4444,      "BR_PMT_ARGB_4444"),
    CONVERTER(rgba_8888_arr_read, rgba_8888_arr_write, BR_PMT_RGBA_8888_ARR,  "BR_PMT_RGBA_8888_ARR"),
    CONVERTER(bgr_565_read,       bgr_565_write,       BR_PMT_BGR_565,        "BR_PMT_BGR_565"),
    CONVERTER(NULL,               NULL,                BR_PMT_DEPTH_24,       "BR_PMT_DEPTH_24"),
    CONVERTER(NULL,               NULL,                BR_PMT_DEPTH_FP32,     "BR_PMT_DEPTH_FP32"),
    CONVERTER(r8b8g8a8_read,      r8g8b8a8_write,      BR_PMT_R8G8B8A8,       "BR_PMT_R8G8B8A8"),
};
#undef CONVERTER
// clang-format on

static const br_pixelmap_convert_options default_opts = {
    .index_alpha_threshold = 127,
    .enable_colour_key     = BR_FALSE,
    .colour_key            = BR_COLOUR_RGB(0, 0, 0),
};

br_error BR_RESIDENT_ENTRY BrColourUnpack(br_colour pixel, br_uint_8 type, br_uint_8 *r, br_uint_8 *g, br_uint_8 *b, br_uint_8 *a)
{
    br_uint_8 buf[sizeof(br_colour)];

    const br_pixelmap_converter *cvt = br_pixelmap_converters + type;
    if(type <= BR_PMT_INDEX_8 || cvt == NULL || cvt->read == NULL)
        return BRE_FAIL;

    _MemPixelSet(buf, pmTypeInfo[type].bits >> 3, pixel);
    pixel = cvt->read(buf, NULL, &default_opts);

    if(r != NULL)
        *r = BR_RED(pixel);

    if(g != NULL)
        *g = BR_GRN(pixel);

    if(b != NULL)
        *b = BR_BLU(pixel);

    if(a != NULL)
        *a = BR_ALPHA(pixel);

    return BRE_OK;
}

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapConvert(br_pixelmap *src, br_uint_8 type, const br_pixelmap_convert_options *opts)
{
    br_device_pixelmap         *dst;
    br_pixelmap_converter      *src_converter;
    br_pixelmap_converter      *dst_converter;
    br_uint_16                  dst_bytes;
    br_uint_16                  src_bytes;
    br_pixelmap_convert_options safeopts = default_opts;

    if(opts != NULL) {
        safeopts            = *opts;
        safeopts.colour_key = BR_COLOUR_RGB(BR_RED(safeopts.colour_key), BR_GRN(safeopts.colour_key), BR_BLU(safeopts.colour_key));
    }

    /*
     * check for accessibility.
     */
    if(src->flags & BR_PMF_NO_ACCESS) {
        BrLogError("PM", "Source pixelmap \"%s\" has BR_PMF_NO_ACCESS flag, cannot clone.", src->identifier);
        return NULL;
    }

    /*
     * check array bounds
     */
    if(type >= BR_PMT_MAX) {
        BrLogError("PM", "Invalid pixelmap type %d.", type);
        return NULL;
    }

    /*
     * get converter facilities
     */
    src_converter = &br_pixelmap_converters[src->type];
    dst_converter = &br_pixelmap_converters[type];

    /*
     * check if we've implemented the source conversion facilities
     */
    if(src_converter->read == NULL || src_converter->write == NULL) {
        BrLogError("PM", "\"%s\" is not supported as a pixelmap cloning source.", src_converter->name);
        return NULL;
    }

    /*
     * check if we've implemented the destination conversion facilities
     */
    if(dst_converter->read == NULL || dst_converter->write == NULL) {
        BrLogError("PM", "\"%s\" is not supported as a pixelmap cloning destination.", dst_converter->name);
        return NULL;
    }

    /*
     * check for palette if necessary
     */
    if(src->type == BR_PMT_INDEX_8) {
        if(src->map == NULL) {
            BrLogError("PM", "Source pixelmap \"%s\" is BR_PMT_INDEX_8, but has no palette.", src->identifier);
            return NULL;
        }

        if(safeopts.target_clut != NULL) {
            if(safeopts.target_clut->type != BR_PMT_RGBX_888) {
                BrLogError("PM", "Target CLUT provided, but is the wrong format.");
                return NULL;
            }

            if(safeopts.target_clut->width != 1 && safeopts.target_clut->height != 256) {
                BrLogError("PM", "Target CLUT provided, but is the wrong dimensions.");
                return NULL;
            }
        }
    }

    /*
     * get bytes per pixel
     */
    dst_bytes = pmTypeInfo[type].bits >> 3;
    src_bytes = pmTypeInfo[src->type].bits >> 3;

    /*
     * allocate destination pixelmap
     */

    dst = DevicePixelmapMemAllocate(type, src->width, src->height, NULL, BR_PMAF_NORMAL);
    if(dst == NULL) {
        BrLogError("PM", "Failed to allocate pixelmap.");
        return NULL;
    }

    /*
     * duplicate identifier
     */
    if(src->identifier != NULL)
        dst->pm_identifier = BrResStrDup(dst, src->identifier);

    /*
     * duplicate values
     */
    dst->pm_origin_x = src->origin_x;
    dst->pm_origin_y = src->origin_y;

    /*
     * check if we need to quantize
     */
    if(type == BR_PMT_INDEX_8) {

        /*
         * begin quantization
         */
        BrQuantBegin();

        if(safeopts.target_clut == NULL) {
            /*
             * create palette pixelmap
             */
            dst->pm_map = (br_pixelmap *)DevicePixelmapMemAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL);
            BrResAdd(dst, dst->pm_map);

            /*
             * add rgb values to quantizer
             */
            for(int y = 0; y < src->height; y++) {
                for(int x = 0; x < src->width; x++) {
                    br_uint_8 *pixel  = (br_uint_8 *)DevicePixelmapMemAddress(src, x, y, src_bytes);
                    br_colour  colour = src_converter->read(pixel, src, &safeopts);

                    /*
                     * Skip transparent pixels
                     */
                    if(BR_ALPHA(colour) <= safeopts.index_alpha_threshold)
                        continue;

                    colour = BR_COLOUR_RGB(BR_RED(colour), BR_GRN(colour), BR_BLU(colour));

                    if(safeopts.enable_colour_key && colour == safeopts.colour_key)
                        continue;

                    br_uint_8 rgb[3] = {
                        [0] = BR_RED(colour),
                        [1] = BR_GRN(colour),
                        [2] = BR_BLU(colour),
                    };

                    BrQuantAddColours(rgb, 1);
                }
            }

            /*
             * create quantised palette, first entry always transparent.
             */
            BrQuantMakePalette(1, 255, dst->pm_map);
            BrPixelmapPixelSet(dst->pm_map, 0, 0, 0);
            BrQuantPrepareMapping(1, 255, dst->pm_map);
        } else {
            dst->pm_map = safeopts.target_clut;
            BrQuantPrepareMapping(0, 256, dst->pm_map);
        }
    }

    /*
     * pixel conversion loop
     */
    for(int y = 0; y < src->height; y++) {
        for(int x = 0; x < src->width; x++) {
            br_uint_8 *dst_pixel  = (br_uint_8 *)DevicePixelmapMemAddress(dst, x, y, dst_bytes);
            br_uint_8 *src_pixel  = (br_uint_8 *)DevicePixelmapMemAddress(src, x, y, src_bytes);
            br_colour  dst_colour = src_converter->read(src_pixel, src, &safeopts);

            /*
             * If we're colour-keying and it matches, zero the alpha.
             */
            const br_uint_8 r = BR_RED(dst_colour), g = BR_GRN(dst_colour), b = BR_BLU(dst_colour);
            if(safeopts.enable_colour_key && BR_COLOUR_RGB(r, g, b) == safeopts.colour_key)
                dst_colour = BR_COLOUR_RGBA(r, g, b, 0);

            dst_converter->write(dst_pixel, dst_colour, &safeopts);
        }
    }

    /*
     * end quantization
     */
    if(type == BR_PMT_INDEX_8) {
        BrQuantEnd();
    }

    return (br_pixelmap *)dst;
}
