#include <brender.h>
#include <brassert.h>

#define BR_DEVICE_PIXELMAP_PRIVATE

#include "pm.h"
#include "pmmem.h"
#include "pmclone.h"

static br_uint_32 mip_level_count(br_int_32 width, br_int_32 height)
{
    br_uint_32 levels  = 1;
    br_int_32  longest = (width > height) ? width : height;

    while(longest > 1) {
        longest >>= 1;
        ++levels;
    }

    return levels;
}

br_error BR_RESIDENT_ENTRY PixelmapMipLevelGeometry(br_pixelmap *pm, br_uint_32 level, br_pixelmap_mip_level *out)
{
    const br_uint_16 bytes = pmTypeInfo[pm->type].bits >> 3;
    br_size_t        at    = 0;
    br_int_32        w     = pm->width;
    br_int_32        h     = pm->height;

    if(out == NULL || bytes == 0 || pm->row_bytes == 0 || level >= BrPixelmapMipLevelCount(pm))
        return BRE_FAIL;

    /*
     * A bottom-up map has a negative stride, which no packed-against-level-0 chain can
     * describe - so it is a single level or nothing. The count above has already answered 1
     * for a map with no chain; a map that claims one is refused.
     */
    if(pm->row_bytes < 0 && pm->mip_offset != 0)
        return BRE_FAIL;

    for(br_uint_32 i = 1; i <= level; ++i) {
        if(i == 1)
            at = (br_size_t)pm->mip_offset * pm->row_bytes;
        else
            at += (br_size_t)w * h * bytes;

        w = (w > 1) ? (w >> 1) : 1;
        h = (h > 1) ? (h >> 1) : 1;
    }

    out->width     = w;
    out->height    = h;
    out->row_bytes = (level == 0) ? pm->row_bytes : w * bytes;
    out->offset    = at;

    return BRE_OK;
}

br_size_t BR_RESIDENT_ENTRY PixelmapMipMemorySize(br_int_32 width, br_int_32 height, br_uint_8 type)
{
    const br_uint_16 bytes = pmTypeInfo[type].bits >> 3;
    br_size_t        total = 0;

    if(width < 1 || height < 1)
        return 0;

    for(;;) {
        total += (br_size_t)width * height * bytes;

        if(width == 1 && height == 1)
            break;

        width  = (width > 1) ? (width >> 1) : 1;
        height = (height > 1) ? (height >> 1) : 1;
    }

    return total;
}

br_uint_32 BR_PUBLIC_ENTRY BrPixelmapMipLevelCount(br_pixelmap *pm)
{
    if(pm == NULL || pm->mip_offset <= 0)
        return 1;

    return mip_level_count(pm->width, pm->height);
}

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapMipLevel(br_pixelmap *pm, br_uint_32 level)
{
    br_pixelmap_mip_level geometry;
    br_pixelmap          *view;

    if(pm == NULL || PixelmapMipLevelGeometry(pm, level, &geometry) != BRE_OK)
        return NULL;

    if((view = BrPixelmapAllocate(pm->type, geometry.width, geometry.height, (br_uint_8 *)pm->pixels + geometry.offset, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    /*
     * A level below 0 is tightly packed, which is not what BrPixelmapAllocate() computes for
     * a type whose rows are aligned, so the flags that describe the stride are set here.
     */
    view->row_bytes = geometry.row_bytes;
    view->flags     = (level == 0) ? pm->flags : ((pm->flags & BR_PMF_NO_ACCESS) | BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS);

    view->mip_offset = 0;
    view->map        = pm->map;

    if(pm->identifier != NULL)
        view->identifier = BrResStrDup(view, pm->identifier);

    return view;
}
#define SQ(a) ((a) * (a))

/* The palette's entries in the caller's metric, and the nearest-entry search over them. */
typedef struct mip_index {
    br_uint_32 base;
    br_uint_32 range;
    br_uint_32 method;
    double     r[256];
    double     g[256];
    double     b[256];
} mip_index;

static void mip_index_prepare(mip_index *ix, br_pixelmap *palette, br_uint_32 base, br_uint_32 range, br_uint_32 method)
{
    ix->base   = base;
    ix->range  = range;
    ix->method = method;

    for(br_uint_32 entry = 0; entry < 256; ++entry) {
        const br_colour colour = BrPixelmapPixelGet(palette, 0, (br_int_32)entry);
        const double    red    = BR_RED(colour);
        const double    green  = BR_GRN(colour);
        const double    blue   = BR_BLU(colour);

        if(method == BR_QUANTIZE_YIQ) {
            ix->r[entry] = 0.299 * red + 0.587 * green + 0.114 * blue;
            ix->g[entry] = 0.596 * red - 0.275 * green - 0.321 * blue;
            ix->b[entry] = 0.212 * red - 0.523 * green + 0.311 * blue;
        } else {
            ix->r[entry] = red;
            ix->g[entry] = green;
            ix->b[entry] = blue;
        }
    }
}

static br_uint_8 mip_index_nearest(const mip_index *ix, br_colour colour)
{
    const double red     = BR_RED(colour);
    const double green   = BR_GRN(colour);
    const double blue    = BR_BLU(colour);
    double       r       = red;
    double       g       = green;
    double       b       = blue;
    double       nearest = 0.0;
    br_uint_8    entry   = (br_uint_8)ix->base;

    if(ix->method == BR_QUANTIZE_YIQ) {
        r = 0.299 * red + 0.587 * green + 0.114 * blue;
        g = 0.596 * red - 0.275 * green - 0.321 * blue;
        b = 0.212 * red - 0.523 * green + 0.311 * blue;
    }

    for(br_uint_32 i = 0; i < ix->range; ++i) {
        const br_uint_32 candidate = ix->base + i;
        const double     distance  = SQ(r - ix->r[candidate]) + SQ(g - ix->g[candidate]) + SQ(b - ix->b[candidate]);

        if(i == 0 || distance < nearest) {
            nearest = distance;
            entry   = (br_uint_8)candidate;
        }
    }

    return entry;
}

/* The converter table's reader decodes a type; the memory accessor hands back the raw bytes. */
static br_colour mip_source_colour(const br_pixelmap_converter *converter, br_pixelmap *source, br_int_32 x, br_int_32 y)
{
    static const br_pixelmap_convert_options opts = {
        .index_alpha_threshold = 127,
    };

    const br_uint_8 *pixel = (const br_uint_8 *)DevicePixelmapMemAddress(source, x, y, pmTypeInfo[source->type].bits >> 3);

    return converter->read(pixel, source, &opts);
}

static void mip_write(const br_pixelmap_converter *converter, br_uint_8 *pixels, br_colour colour, const mip_index *ix)
{
    static const br_pixelmap_convert_options opts = {
        .index_alpha_threshold = 127,
    };

    if(ix != NULL)
        *pixels = mip_index_nearest(ix, colour);
    else
        converter->write(pixels, colour, &opts);
}

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapMakeMipMap(br_pixelmap *source, br_uint_32 destinationType, br_pixelmap *palette, br_uint_32 base,
                                                  br_uint_32 range, br_uint_32 quantizationMethod)
{
    const br_pixelmap_converter *converter, *src_converter;
    const mip_index             *ix = NULL;
    mip_index                    index;
    br_pixelmap                 *destination;
    br_uint_8                   *pixels, *level;
    br_int_32                   *aggregate;
    br_int_32                    width, height, area, row, col, channel, sx, sy;
    br_uint_32                   dst_bytes;
    br_size_t                    size;

    if(source == NULL || source->pixels == NULL) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): the source has no pixel buffer.");
        return NULL;
    }

    if(source->type >= BR_PMT_MAX || destinationType >= BR_PMT_MAX || quantizationMethod >= BR_QUANTIZE_MAX) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): invalid pixelmap type or quantization method.");
        return NULL;
    }

    if(base > 256 || range > 256 || base + range > 256) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): base %u and range %u run past the palette.", base, range);
        return NULL;
    }

    if(destinationType == BR_PMT_INDEX_8 && palette == NULL) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): an indexed destination needs a palette.");
        return NULL;
    }

    if(source->width < 1 || source->height < 1) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): \"%s\" is %dx%d.", source->identifier != NULL ? source->identifier : "pixelmap",
                   source->width, source->height);
        return NULL;
    }

    if((br_int_64)source->width * (br_int_64)source->height * 255 > (br_int_64)0x7fffffff) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): a %dx%d source overflows the 32-bit sums the filter accumulates.", source->width, source->height);
        return NULL;
    }

    converter     = br_pixelmap_converters + destinationType;
    src_converter = br_pixelmap_converters + source->type;
    dst_bytes     = pmTypeInfo[destinationType].bits >> 3;

    /*
     * Level 0's stride and the chain's offset are 16-bit fields, so a destination whose row does
     * not fit in one - or a source taller than one - cannot be described at all. Truncating them
     * puts the level below level 0 before the allocation and the write runs off the end of it.
     */
    if((br_int_64)source->width * dst_bytes > 0x7fff || source->height > 0x7fff) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): a %dx%d source cannot be described by a chain of this type.", source->width, source->height);
        return NULL;
    }

    if(src_converter->read == NULL) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): \"%s\" is not a supported mipmap source.", src_converter->name);
        return NULL;
    }

    if(converter->write == NULL || dst_bytes == 0) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): \"%s\" is not a supported mipmap destination.", converter->name);
        return NULL;
    }

    if((size = PixelmapMipMemorySize(source->width, source->height, (br_uint_8)destinationType)) == 0) {
        BrLogError("PM", "BrPixelmapMakeMipMap(): the chain's size overflows.");
        return NULL;
    }

    if((pixels = BrResAllocate(NULL, size, BR_MEMORY_PIXELS)) == NULL)
        return NULL;

    if((destination = BrPixelmapAllocate((br_uint_8)destinationType, source->width, source->height, pixels, BR_PMAF_NORMAL)) == NULL) {
        BrResFree(pixels);
        return NULL;
    }

    BrResAdd(destination, pixels);

    /* A chain level's row is tight, not what BrPixelmapAllocate() gives a type with aligned rows. */
    destination->row_bytes  = (br_int_16)(destination->width * dst_bytes);
    destination->mip_offset = (br_int_16)source->height;
    destination->origin_x   = source->origin_x;
    destination->origin_y   = source->origin_y;
    destination->map        = palette;
    destination->flags |= BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS;

    if(source->identifier != NULL)
        destination->identifier = BrResStrDup(destination, source->identifier);

    if(destinationType == BR_PMT_INDEX_8) {
        mip_index_prepare(&index, palette, base, range, quantizationMethod);
        ix = &index;
    }

    if(source->type == destinationType) {
        BrPixelmapCopy(destination, source);
    } else {
        for(row = 0; row < source->height; ++row) {
            br_uint_8 *pixel = (br_uint_8 *)destination->pixels + (br_size_t)row * destination->row_bytes;

            for(col = 0; col < source->width; ++col, pixel += dst_bytes)
                mip_write(converter, pixel, mip_source_colour(src_converter, source, col, row), ix);
        }
    }

    /*
     * Levels accumulate exact sums of the source's 8-bit channels and divide once per level, so
     * a level is round(sum / count) rather than a re-average of the quantised level above it.
     */
    if((aggregate = BrMemAllocate((br_size_t)source->width * source->height * 4 * sizeof(*aggregate), BR_MEMORY_SCRATCH)) == NULL) {
        BrPixelmapFree(destination);
        return NULL;
    }

    for(row = 0; row < source->height; ++row) {
        for(col = 0; col < source->width; ++col) {
            const br_colour colour = mip_source_colour(src_converter, source, col, row);
            br_int_32      *sample = aggregate + ((br_size_t)row * source->width + col) * 4;

            sample[0] = BR_RED(colour);
            sample[1] = BR_GRN(colour);
            sample[2] = BR_BLU(colour);
            sample[3] = BR_ALPHA(colour);
        }
    }

    level  = (br_uint_8 *)destination->pixels + (br_size_t)destination->mip_offset * destination->row_bytes;
    width  = source->width;
    height = source->height;
    area   = 1;

    while(width > 1 || height > 1) {
        const br_int_32 block_w = (width > 1) ? 2 : 1;
        const br_int_32 block_h = (height > 1) ? 2 : 1;
        const br_int_32 next_w  = (width > 1) ? (width >> 1) : 1;
        const br_int_32 next_h  = (height > 1) ? (height >> 1) : 1;
        br_uint_8      *pixel   = level;
        br_int_32      *dst;

        area *= block_w * block_h;

        for(row = 0; row < next_h; ++row) {
            for(col = 0; col < next_w; ++col, pixel += dst_bytes) {
                const br_int_32 *src    = aggregate + (row * block_h * width + col * block_w) * 4;
                br_int_32        sum[4] = {0, 0, 0, 0};

                /*
                 * Read the sums before writing the destination: the halving is in place, so the
                 * first output pixel's destination is the first input pixel itself.
                 */
                for(sy = 0; sy < block_h; ++sy)
                    for(sx = 0; sx < block_w; ++sx)
                        for(channel = 0; channel < 4; ++channel)
                            sum[channel] += src[sy * 4 * width + sx * 4 + channel];

                dst    = aggregate + (row * next_w + col) * 4;
                dst[0] = sum[0];
                dst[1] = sum[1];
                dst[2] = sum[2];
                dst[3] = sum[3];

                mip_write(converter, pixel,
                          BR_COLOUR_RGBA(dst[0] / (double)area + 0.5, dst[1] / (double)area + 0.5, dst[2] / (double)area + 0.5,
                                         dst[3] / (double)area + 0.5),
                          ix);
            }
        }

        level += (br_size_t)next_w * next_h * dst_bytes;
        width  = next_w;
        height = next_h;
    }

    BrMemFree(aggregate);

    return destination;
}
