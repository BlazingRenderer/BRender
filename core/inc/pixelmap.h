/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: pixelmap.h 1.7 1998/09/09 13:28:45 johng Exp $
 * $Locker: $
 *
 * A stopgap 2D pixelmap structure for brender. This should really be the
 * pixelmap data type from the underlying 2D system (whatever that will
 * be)
 *
 * Used for input (maps) and output (render buffer)
 */
#ifndef _PIXELMAP_H_
#define _PIXELMAP_H_

/*
 * Various types of pixel
 */
enum {
    /**
     * \brief 1-bit index into a colour map (2 colours).
     *
     * \note 32-bit encoding:
     *       \code 0000000000000000000000000000000i \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code i....... ........ ........ ........ \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_INDEX_1,

    /**
     * \brief 2-bit index into a colour map (4 colours).
     *
     * \note 32-bit encoding:
     *       \code 000000000000000000000000000000ii \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code ii...... ........ ........ ........ \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_INDEX_2,

    /**
     * \brief 4-bit index into a colour map (16 colours).
     *
     * \note 32-bit encoding:
     *       \code 0000000000000000000000000000iiii \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code iiii.... ........ ........ ........ \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_INDEX_4,

    /**
     * \brief 8-bit index into a colour map (256 colours).
     *
     * \note 32-bit encoding:
     *       \code 000000000000000000000000iiiiiiii \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code iiiiiiii ........ ........ ........ \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_INDEX_8,

    /**
     * \brief 16-bit 'true colour' RGB, 5-bits each colour.
     *
     * \note 32-bit encoding:
     *       \code 00000000000000000rrrrrgggggbbbbb \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code gggbbbbb 0rrrrrgg ........ ........ \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_RGB_555,

    /**
     * \brief 16-bit 'true colour' RGB, 5 bits red and blue, 6 bits green.
     *
     * \note 32-bit encoding:
     *       \code 0000000000000000rrrrrggggggbbbbb \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code gggbbbbb rrrrrggg ........ ........ \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_RGB_565,

    /**
     * \brief 24-bit 'true colour' RGB, 8 bits each colour.
     *
     * \note 32-bit encoding:
     *       \code 00000000rrrrrrrrggggggggbbbbbbbb \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code bbbbbbbb gggggggg rrrrrrrr ........ \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_RGB_888,

    /**
     * \brief 32-bit 'true colour' RGB, 8 bits each colour, 8 bits unused.
     *
     * \note Actually XRGB.
     *
     * \note 32-bit encoding:
     *       \code 00000000rrrrrrrrggggggggbbbbbbbb \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code bbbbbbbb gggggggg rrrrrrrr xxxxxxxx \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_RGBX_888,

    /**
     * \brief 32-bit 'true colour' RGB, 8 bits each colour with an 8-bit alpha channel.
     *
     * \note Actually ARGB.
     *
     * \note 32-bit encoding:
     *       \code aaaaaaaarrrrrrrrggggggggbbbbbbbb \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code bbbbbbbb gggggggg rrrrrrrr aaaaaaaa \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_RGBA_8888,

    /*
     * YUV
     */
    BR_PMT_YUYV_8888, /* YU YV YU YV ... */
    BR_PMT_YUV_888,

    /*
     * Depth
     */

    /**
     * \brief The pixelmap is used as a depth buffer with 16-bit precision.
     *
     * \note 32-bit encoding:
     *       \code dddddddddddddddd0000000000000000 \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code Undefined \endcode
     */
    BR_PMT_DEPTH_16,

    /**
     * @brief The pixelmap is used as a depth buffer with 32-bit precision.
     *
     * \note 32-bit encoding:
     *       \code dddddddddddddddddddddddddddddddd \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code Undefined \endcode
     */
    BR_PMT_DEPTH_32,

    /*
     * Opacity
     */
    BR_PMT_ALPHA_8,

    /*
     * Opacity + Index.
     */
    BR_PMT_INDEXA_88,

    /*
     * Bump maps
     */
    BR_PMT_NORMAL_INDEX_8,
    BR_PMT_NORMAL_XYZ,

    /*
     * Wrong way around 15 bit true colour
     */
    BR_PMT_BGR_555,

    /*
     * 16 bit r,g,b & alpha
     */
    BR_PMT_RGBA_4444,

    /*
     * Handy types for converting to 15/16 bit
     */
    BR_PMT_RBG_bab,
    BR_PMT_RBG_1aba,

    /*
     * Pixelmap extensions.
     */
    BR_PMT_RGB_332,
    BR_PMT_DEPTH_8,

    BR_PMT_ARGB_8888,
    BR_PMT_ALPHA_4,
    BR_PMT_INDEXA_44,
    BR_PMT_DEPTH_15,
    BR_PMT_DEPTH_31,
    BR_PMT_DEPTH_FP16,
    BR_PMT_DEPTH_FP15,

    BR_PMT_RGBA_5551,
    BR_PMT_ARGB_1555,
    BR_PMT_ARGB_4444,

    /*
     * An RGBA8888 "byte array".
     * Mainly used for exporting to PNG.
     */
    BR_PMT_RGBA_8888_ARR,

    /**
     * \brief 16-bit 'true colour' BGR, 5 bits red and blue, 6 bits green.
     *
     * \note 32-bit encoding:
     *       \code 0000000000000000bbbbbggggggrrrrr \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code gggrrrrr bbbbbggg ........ ........ \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_BGR_565,

    BR_PMT_MAX,

    BR_PMT_AINDEX_44 = BR_PMT_INDEXA_44,
    BR_PMT_AINDEX_88 = BR_PMT_INDEXA_88,
};

/*
 * pixelmap flags
 */
enum {
    /*
     * No direct access to pixels
     */
    BR_PMF_NO_ACCESS = 0x01,

    BR_PMF_LINEAR          = 0x02,
    BR_PMF_ROW_WHOLEPIXELS = 0x04,

    BR_PMF_PIXELS_NEAR       = 0x08,
    BR_PMF_PIXELS_NEAR_ALIAS = 0x10,

    BR_PMF_KEYED_TRANSPARENCY = 0x20,

    BR_PMF_KEEP_ORIGINAL = 0x40,
    /*
     * Experimental - if set then pixelmap is tiled - low bits of Y are moved to least significant part of pixel address
     */
    BR_PMF_ENABLE_TEXTURE_COMPRESSION = 0x80,
    //      BR_PMF_TILED = 0x80,
};

#define BR_PM_TILE_SIZE 2

/*
 * A macro that declares the pixelmap member entries - this is so that
 * various compatible structures can be created
 */
#define BR_PIXELMAP_MEMBERS_PREFIXED(prefix)                              \
    /*                                                                    \
     * pointer to raw pixel data                                          \
     */                                                                   \
    void *prefix##pixels;                                                 \
                                                                          \
    /*                                                                    \
     * Optional pixel map when pixels are indexed.                        \
     */                                                                   \
    struct br_pixelmap *prefix##map;                                      \
                                                                          \
    /*                                                                    \
     * Key colour ranges                                                  \
     */                                                                   \
    br_colour_range prefix##src_key;                                      \
    br_colour_range prefix##dst_key;                                      \
                                                                          \
    /*                                                                    \
     * Key colour                                                         \
     */                                                                   \
    br_uint_32 prefix##key;                                               \
                                                                          \
    /*                                                                    \
     * Byte difference between pixels at same column of adjacent rows     \
     */                                                                   \
    br_int_16 prefix##row_bytes;                                          \
                                                                          \
    /*                                                                    \
     * if ! 0, offset (in top level rows) from top map start to top-1 map \
     */                                                                   \
    br_int_16 prefix##mip_offset;                                         \
                                                                          \
    /*                                                                    \
     * Type of pixels                                                     \
     */                                                                   \
    br_uint_8 prefix##type;                                               \
                                                                          \
    /*                                                                    \
     * Flags                                                              \
     */                                                                   \
    br_uint_16 prefix##flags;                                             \
                                                                          \
    /*                                                                    \
     * Copy function                                                      \
     */                                                                   \
    br_uint_16 prefix##copy_function;                                     \
                                                                          \
    /*                                                                    \
     * top left visible region in pixels from pixel at 'pixel' pointer    \
     */                                                                   \
    br_uint_16 prefix##base_x;                                            \
    br_uint_16 prefix##base_y;                                            \
                                                                          \
    /*                                                                    \
     * Width and height of bitmap in pixels                               \
     */                                                                   \
    br_uint_16 prefix##width;                                             \
    br_uint_16 prefix##height;                                            \
                                                                          \
    /*                                                                    \
     * Local origin for any graphics system rendering into map, relative  \
     * to 'base'                                                          \
     */                                                                   \
    br_int_16 prefix##origin_x;                                           \
    br_int_16 prefix##origin_y;                                           \
                                                                          \
    /*                                                                    \
     * Workspace fields for user and database                             \
     */                                                                   \
    void *prefix##user;                                                   \
    void *prefix##stored;

#define BR_PIXELMAP_MEMBERS BR_PIXELMAP_MEMBERS_PREFIXED(pm_)

/*
 * Public structure that is comaptible with a device_pixelmap
 */
typedef struct br_pixelmap {
    br_uintptr_t _reserved;

    /*
     * Optional identifier (when maps used as textures/tables etc.)
     */
    char *identifier;

    BR_PIXELMAP_MEMBERS_PREFIXED()

} br_pixelmap;

/*
 * Flags to BrPixelMapAllocate
 */
enum br_pixelmap_allocate_flags {
    BR_PMAF_NORMAL    = 0x0000, /* Setup pixelmap so that 0th scanline is at low memory		*/
    BR_PMAF_INVERTED  = 0x0001, /* Setup pixelmap so that 0th scanline is at high memory	*/
    BR_PMAF_NO_PIXELS = 0x0002, /* Don't allocate any pixel data							*/
};

/*
 * Channel flags
 */
enum br_pixelmap_channel_mask {
    BR_PMCHAN_INDEX  = 0x0001,
    BR_PMCHAN_RGB    = 0x0002,
    BR_PMCHAN_DEPTH  = 0x0004,
    BR_PMCHAN_ALPHA  = 0x0008,
    BR_PMCHAN_YUV    = 0x0010,
    BR_PMCHAN_VECTOR = 0x0020
};

/*
 * Matching pixelmap types
 */
enum br_pmmatch_type {
    BR_PMMATCH_OFFSCREEN,
    BR_PMMATCH_DEPTH_16,
    BR_PMMATCH_DEPTH,
    BR_PMMATCH_HIDDEN,
    BR_PMMATCH_HIDDEN_BUFFER,
    BR_PMMATCH_NO_RENDER,
    BR_PMMATCH_DEPTH_8,
    BR_PMMATCH_DEPTH_32,
    BR_PMMATCH_DEPTH_15,
    BR_PMMATCH_DEPTH_31,
    BR_PMMATCH_DEPTH_FP15,
    BR_PMMATCH_DEPTH_FP16,

    BR_PMMATCH_MAX
};

/*
 * Copy functions
 */
enum br_pixelmap_copy_function {
    BR_PMCOPY_NORMAL    = 0x0000,
    BR_PMCOPY_SRC_KEYED = 0x0001,
    BR_PMCOPY_DST_KEYED = 0x0002
};

/*
 * General 2D point and rectangle
 */
typedef struct br_point {
    br_int_32 x;
    br_int_32 y;
} br_point;

typedef struct br_rectangle {
    br_int_32 x;
    br_int_32 y;
    br_int_32 w;
    br_int_32 h;
} br_rectangle;

/*
 * General result for a clipping operation
 */
typedef enum br_clip_result {
    BR_CLIP_REJECT,
    BR_CLIP_PARTIAL,
    BR_CLIP_ACCEPT
} br_clip_result;

/*
 * Quantization methods used in mip-level generation for BR_PMT_INDEX_8
 */
enum {
    BR_QUANTIZE_RGB,
    BR_QUANTIZE_YIQ,
    BR_QUANTIZE_MAX
};

enum {
    BR_CONTROL_GAMMA_RGB  = 0x00000001,
    BR_CONTROL_GAMMA      = 0x00000002,
    BR_CONTROL_BRIGHTNESS = 0x00000004,
    BR_CONTROL_CONTRAST   = 0x00000008,
    BR_CONTROL_HUE        = 0x00000010,
    BR_CONTROL_SATURATION = 0x00000020,
    BR_CONTROL_SHARPNESS  = 0x00000040,
};

typedef struct {
    br_uint_32 flags;
    br_uint_16 gamma_red[256];
    br_uint_16 gamma_green[256];
    br_uint_16 gamma_blue[256];
    br_int_32  gamma;
    br_int_32  brightness;
    br_int_32  contrast;
    br_int_32  hue;
    br_int_32  saturation;
    br_int_32  sharpness;
} br_display_controls;

#endif
