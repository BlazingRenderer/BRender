/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: pmfile.c 1.1 1997/12/10 16:41:26 jon Exp $
 * $Locker: $
 *
 * Pixelmap per-chunk file operations
 *
 */
#include <stddef.h>

#define BR_DEVICE_PIXELMAP_PRIVATE

#include "pm.h"
#include "pmmem.h"
#include "brassert.h"
#include "datafile.h"

#include "pmmem.h"

/**
 ** Pixelmap
 **/
// clang-format off
static br_file_enum_member pixelmap_type_FM[] = {
    _ENUM_MEMBER(BR_PMT_INDEX_1),
    _ENUM_MEMBER(BR_PMT_INDEX_2),
    _ENUM_MEMBER(BR_PMT_INDEX_4),
    _ENUM_MEMBER(BR_PMT_INDEX_8),
    _ENUM_MEMBER(BR_PMT_RGB_555),
    _ENUM_MEMBER(BR_PMT_RGB_565),
    _ENUM_MEMBER(BR_PMT_RGB_888),
    _ENUM_MEMBER(BR_PMT_RGBX_888),
    _ENUM_MEMBER(BR_PMT_RGBA_8888),
    _ENUM_MEMBER(BR_PMT_YUYV_8888),
    _ENUM_MEMBER(BR_PMT_YUV_888),
    _ENUM_MEMBER(BR_PMT_DEPTH_16),
    _ENUM_MEMBER(BR_PMT_DEPTH_32),
    _ENUM_MEMBER(BR_PMT_ALPHA_8),
    _ENUM_MEMBER(BR_PMT_INDEXA_88),
    _ENUM_MEMBER(BR_PMT_NORMAL_INDEX_8),
    _ENUM_MEMBER(BR_PMT_NORMAL_XYZ),
    _ENUM_MEMBER(BR_PMT_BGR_555),
    _ENUM_MEMBER(BR_PMT_RGBA_4444),
    _ENUM_MEMBER(BR_PMT_RBG_bab),
    _ENUM_MEMBER(BR_PMT_RBG_1aba),
    _ENUM_MEMBER(BR_PMT_RGB_332),
    _ENUM_MEMBER(BR_PMT_DEPTH_8),
    _ENUM_MEMBER(BR_PMT_ARGB_8888),
    _ENUM_MEMBER(BR_PMT_ALPHA_4),
    _ENUM_MEMBER(BR_PMT_INDEXA_44),
    _ENUM_MEMBER(BR_PMT_DEPTH_15),
    _ENUM_MEMBER(BR_PMT_DEPTH_31),
    _ENUM_MEMBER(BR_PMT_DEPTH_FP16),
    _ENUM_MEMBER(BR_PMT_DEPTH_FP15),
    _ENUM_MEMBER(BR_PMT_RGBA_5551),
    _ENUM_MEMBER(BR_PMT_ARGB_1555),
    _ENUM_MEMBER(BR_PMT_ARGB_4444),
    _ENUM_MEMBER(BR_PMT_RGBA_8888_ARR),
    _ENUM_MEMBER(BR_PMT_BGR_565),
    _ENUM_MEMBER(BR_PMT_DEPTH_24),
    _ENUM_MEMBER(BR_PMT_DEPTH_FP32),
    _ENUM_MEMBER(BR_PMT_R8G8B8A8),
};
// clang-format on

static _FILE_ENUM(pixelmap_type);

#define _STRUCT_NAME struct br_pixelmap

// clang-format off
static br_file_struct_member br_old_pixelmap_FM[] = {
    _ENUM_8(type, pixelmap_type_F),
    _UINT_16(row_bytes),
    _UINT_16(width),
    _UINT_16(height),
    _UINT_16(origin_x),
    _UINT_16(origin_y),
    _ASCIZ(identifier),
};

static br_file_struct_member br_pixelmap_FM[] = {
    _ENUM_8(type, pixelmap_type_F),
    _UINT_16(row_bytes),
    _UINT_16(width),
    _UINT_16(height),
    _UINT_16(origin_x),
    _UINT_16(origin_y),
    _UINT_16(mip_offset),
    _ASCIZ(identifier),
};
// clang-format on

static _FILE_STRUCT(br_pixelmap);
static _FILE_STRUCT(br_old_pixelmap);

#undef _STRUCT_NAME

static int FopWrite_PIXELMAP(br_datafile *df, br_pixelmap *pixelmap)
{
    br_pixelmap pmap = *pixelmap;

    pmap.row_bytes = pmap.width * DevicePixelmapPixelBytes(&pmap);

    df->prims->chunk_write(df, FID_PIXELMAP, df->prims->struct_size(df, &br_pixelmap_F, &pmap));
    df->prims->struct_write(df, &br_pixelmap_F, &pmap);

    return 0;
}

static int FopRead_OLD_PIXELMAP(br_datafile *df, br_uint_32 id, br_uint_32 length, br_uint_32 count)
{
    br_pixelmap *pp;

    /*
     * Allocate and read in pixelmap structure
     */
    pp = (br_pixelmap *)DevicePixelmapMemAllocate(BR_PMT_INDEX_8, 0, 0, NULL, BR_PMAF_NO_PIXELS);

    df->res = pp;
    df->prims->struct_read(df, &br_old_pixelmap_F, pp);
    df->res = NULL;

    pp->row_bytes = pp->width * DevicePixelmapPixelBytes(pp);

    /*
     * Leave material on stack
     */
    DfPush(DFST_PIXELMAP, pp, 1);

    return 0;
}

static int FopRead_PIXELMAP(br_datafile *df, br_uint_32 id, br_uint_32 length, br_uint_32 count)
{
    br_pixelmap *pp;

    /*
     * Allocate and read in pixelmap structure
     */
    pp = (br_pixelmap *)DevicePixelmapMemAllocate(BR_PMT_INDEX_8, 0, 0, NULL, BR_PMAF_NO_PIXELS);

    df->res = pp;
    df->prims->struct_read(df, &br_pixelmap_F, pp);
    df->res = NULL;

    /*
     * The type is a byte out of the file, and everything below indexes pmTypeInfo[] with it.
     */
    if(pp->type >= BR_PMT_MAX) {
        BrLogWarn("PM", "pixelmap type %u is not one of the %d types", (unsigned)pp->type, (int)BR_PMT_MAX);
        BrPixelmapFree(pp);
        return -1;
    }

    pp->row_bytes = pp->width * DevicePixelmapPixelBytes(pp);

    /*
     * Leave material on stack
     */
    DfPush(DFST_PIXELMAP, pp, 1);

    return 0;
}

/**
 ** Pixel bytes
 **/
static int FopWrite_PIXELS(br_datafile *df, br_pixelmap *pixelmap)
{
    int   size        = DevicePixelmapFileBytes(pixelmap);
    int   bytes       = DevicePixelmapPixelBytes(pixelmap);
    int   block_count = (pixelmap->width * bytes) / size;
    char *pixels;

    pixels = (char *)(pixelmap->pixels) + pixelmap->base_y * pixelmap->row_bytes + (pixelmap->base_x * bytes);

    if(pixelmap->mip_offset) {
        int mipSize = 0;
        int currentMipSize;
        UASSERT_MESSAGE("File system currently only supports contiguos mipmaps with no padding that are powers of two "
                        "in size.",
                        (pixelmap->width == pixelmap->height) && (pixelmap->row_bytes == pixelmap->width * bytes) &&
                            (!(pixelmap->width & (pixelmap->width - 1))));

        for(currentMipSize = pixelmap->width; currentMipSize; currentMipSize >>= 1)
            mipSize += currentMipSize * currentMipSize * bytes;

        df->prims->chunk_write(df, FID_PIXELS, df->prims->block_size(df, pixels, mipSize / size, mipSize, 1, size));

        df->prims->block_write(df, pixels, mipSize, mipSize, 1, size);

    } else {

        df->prims->chunk_write(df, FID_PIXELS, df->prims->block_size(df, pixels, block_count, pixelmap->row_bytes, pixelmap->height, size));

        df->prims->block_write(df, pixels, block_count, pixelmap->row_bytes, pixelmap->height, size);
    }

    return 0;
}

static int FopRead_PIXELS(br_datafile *df, br_uint_32 id, br_uint_32 length, br_uint_32 count)
{
    int          icount;
    br_pixelmap *pp;
    int          size;
    br_int_32    file_bytes;
    br_size_t    need;

    pp         = DfTop(DFST_PIXELMAP, 0);
    size       = DevicePixelmapFileBytes(pp);
    file_bytes = pp->row_bytes < 0 ? -(br_int_32)pp->row_bytes : pp->row_bytes;

    /*
     * The chunk has to hold the map's samples: a short one leaves a map with a stride
     * and nothing behind it. A bottom-up map's row_bytes is negative and its samples are
     * still row_bytes * height of them. A mipmapped map's block covers every level, so it
     * is left alone.
     */
    need = pp->mip_offset != 0 ? 0 : (br_size_t)file_bytes * (br_size_t)pp->height;

    if(length < (br_uint_32)need) {
        BrLogWarn("PM", "pixel chunk is %u bytes, %d x %d at %d bytes per row needs %lu", (unsigned)length, pp->width, pp->height,
                  pp->row_bytes, (unsigned long)need);
        return -1;
    }

    df->res    = pp;
    pp->pixels = df->prims->block_read(df, NULL, &icount, size, BR_MEMORY_PIXELS);
    df->res    = NULL;

    /*
     * The block carries its own length, and that is the one the reader allocates from - a
     * chunk header that agrees with the map proves nothing. The allocation is the only
     * quantity that settles it, so it is checked once it exists.
     */
    if(pp->pixels != NULL && need != 0 && BrResSize(pp->pixels) < need) {
        BrLogWarn("PM", "pixel block is %lu bytes, %d x %d at %d bytes per row needs %lu", (unsigned long)BrResSize(pp->pixels), pp->width,
                  pp->height, pp->row_bytes, (unsigned long)need);
        BrResFree(pp->pixels);
        pp->pixels = NULL;
        return -1;
    }

    pp->flags |= BR_PMF_LINEAR;

    return 0;
}

/**
 ** Connecting a map to an indexed pixelmap
 **/
static int FopWrite_ADD_MAP(br_datafile *df)
{
    df->prims->chunk_write(df, FID_ADD_MAP, 0);

    return 0;
}

static int FopRead_ADD_MAP(br_datafile *df, br_uint_32 id, br_uint_32 length, br_uint_32 count)
{
    br_pixelmap *pp, *map;

    map = DfPop(DFST_PIXELMAP, 0);
    pp  = DfTop(DFST_PIXELMAP, 0);

    pp->map = map;

    BrResAdd(pp, map);

    return 0;
}

/**
 **	Public file operations
 **/

/*
 * Load a group of pixelmaps from a file
 */
static br_chunks_table_entry PixelmapLoadEntries[] = {
    {FID_END,          0, FopRead_END         },
    {FID_PIXELMAP,     0, FopRead_PIXELMAP    },
    {FID_PIXELS,       0, FopRead_PIXELS      },
    {FID_ADD_MAP,      0, FopRead_ADD_MAP     },
    {FID_OLD_PIXELMAP, 0, FopRead_OLD_PIXELMAP},
};

static br_chunks_table PixelmapLoadTable = {
    BR_ASIZE(PixelmapLoadEntries),
    PixelmapLoadEntries,
};

br_uint_32 BR_PUBLIC_ENTRY BrPixelmapLoadMany(const char *filename, br_pixelmap **pixelmaps, br_uint_16 num)
{
    br_datafile *df;
    int          count;
    int          r;

    df = DfOpen(filename, 0, BR_SCALAR_TOKEN);

    if(df == NULL)
        return 0;

    for(count = 0; count < num;) {
        r = DfChunksInterpret(df, &PixelmapLoadTable);

        if(DfTopType() == DFST_PIXELMAP)
            pixelmaps[count++] = DfPop(DFST_PIXELMAP, 0);

        if(r == 0)
            break;
    }

    DfClose(df);

    return count;
}

/*
 * Save a group of pixelmaps to a file
 */
static int WritePixelmap(br_pixelmap *pp, br_datafile *df)
{
    // Lock the pixelmap because we want to access it's pixels

    BrPixelmapDirectLock(pp, BR_TRUE);

    ASSERT(pp->pixels);

    /*
     * Write base pixelmap structure
     */
    FopWrite_PIXELMAP(df, pp);

    /*
     * Write any palette
     */
    if(pp->map) {
        WritePixelmap(pp->map, df);
        FopWrite_ADD_MAP(df);
    }

    /*
     * Write pixel data
     */
    FopWrite_PIXELS(df, pp);

    // We're finished with the pixelmap, so unlock it.

    BrPixelmapDirectUnlock(pp);

    return 0;
}

br_uint_32 BR_PUBLIC_ENTRY BrPixelmapSaveMany(const char *filename, br_pixelmap **pixelmaps, br_uint_16 num)
{
    br_datafile *df;
    int          i;

    ASSERT(filename != NULL);
    ASSERT(pixelmaps != NULL);

    /*
     * Open file and write header
     */
    df = DfOpen(filename, 1, BR_SCALAR_TOKEN);

    if(df == NULL)
        return 0;

    FopWrite_FILE_INFO(df, FILE_TYPE_PIXELMAP);

    for(i = 0; i < num; i++) {
        WritePixelmap(pixelmaps[i], df);
        FopWrite_END(df);
    }

    DfClose(df);

    return num;
}

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapLoad(const char *filename)
{
    br_pixelmap *ptr;

    return (BrPixelmapLoadMany(filename, &ptr, 1) != 1) ? NULL : ptr;
}

br_uint_32 BR_PUBLIC_ENTRY BrPixelmapSave(const char *filename, br_pixelmap *ptr)
{
    return BrPixelmapSaveMany(filename, &ptr, 1);
}
