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

    /**
     * \brief The pixelmap is used as a depth buffer with 24-bit precision.
     *
     * \note 32-bit encoding:
     *       \code dddddddddddddddddddddddd00000000 \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code Undefined \endcode
     */
    BR_PMT_DEPTH_24,

    /**
     * \brief The pixelmap is used as a depth buffer with 32-bit floating-point precision.
     *
     * \note 32-bit encoding:
     *       \code dddddddddddddddddddddddddddddddd \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code Undefined \endcode
     */
    BR_PMT_DEPTH_FP32,

    /**
     * \brief 32-bit 'true colour' RGBA, 8 bits each colour with an 8-bit alpha channel.
     *
     * \note This is what BR_PMT_RGBA_8888 should've been.
     *
     * \note 32-bit encoding:
     *       \code rrrrrrrrggggggggbbbbbbbbaaaaaaaa \endcode
     *
     * \note First Four Bytes of Left Hand Pixel:
     *       \code aaaaaaaa bbbbbbbb gggggggg rrrrrrrr \endcode
     *
     * \remark The left hand byte is the byte at br_pixelmap::pixels.
     */
    BR_PMT_R8G8B8A8,

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

    BR_PMF_RESERVED_0 = 0x20,

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
// clang-format off
#define BR_PIXELMAP_MEMBERS_PREFIXED(prefix)                                                              \
    /**                                                                                                   \
    \brief When pixel data is directly accessible (see flags), this member points to an area of           \
           memory containing the raw pixel data.                                                          \
    */                                                                                                    \
    /**                                                                                                   \
    It either points to the start of the memory occupied by the pixel map or the last row_bytes of        \
    it. However, it always points to the byte of the left hand pixel of the 'first' row. For              \
    instance, in monochrome pixel maps it will point to the byte whose most significant bit               \
    represents the left hand pixel of the first row. In true colour pixel maps it will point to the       \
    least significant byte of the colour of the left hand pixel of the first row, which will be the       \
    blue component in BR_PMT_RGB_888 pixel maps and the alpha component in BR_PMT_RGBA_8888 pixel         \
    maps.                                                                                                 \
    */                                                                                                    \
    void *prefix##pixels;                                                                                 \
                                                                                                          \
    /**                                                                                                   \
    \brief For indexed pixel maps (of type BR_PMT_INDEX_?), this member points to a colour map.           \
    */                                                                                                    \
    /**                                                                                                   \
    This is used to obtain the 'true colour' corresponding to a particular index.                         \
    */                                                                                                    \
    struct br_pixelmap *prefix##map;                                                                      \
                                                                                                          \
    /**                                                                                                   \
    \brief This member defines the physical row length of the pixel map in terms of the byte              \
           difference between pixels in the same column of adjacent rows.                                 \
    */                                                                                                    \
    /**                                                                                                   \
    It will be negative if the pixel map memory is inverted.                                              \
    */                                                                                                    \
    br_int_16 prefix##row_bytes;                                                                          \
                                                                                                          \
    /*                                                                                                    \
     * if ! 0, offset (in top level rows) from top map start to top-1 map                                 \
     */                                                                                                   \
    br_int_16 prefix##mip_offset;                                                                         \
                                                                                                          \
    /**                                                                                                   \
    \brief This member defines the type of data stored for each pixel in the pixel map.                   \
    */                                                                                                    \
    /**                                                                                                   \
    The various types have values defined by the following symbols; each symbol's entry on the            \
    enumeration gives its 32 bit pixel value encoding and the first four bytes of the left hand           \
    pixel. Values are written with the most significant bit to the left, and the encoding is the          \
    32 bit value to be supplied as colour to functions such as BrPixelmapPixelSet(). The dots             \
    represent further pixels.                                                                             \
    */                                                                                                    \
    /**                                                                                                   \
    <br> <br>                                                                                             \
    */                                                                                                    \
    /**                                                                                                   \
    The ordering of bytes pixel maps is independent of word byte order, except in the case of             \
    depth buffers, in which pixels are read and written as 32 bit values. This means that in a            \
    16 bit depth buffer the least significant 16 bits are lost. Note, with respect to pixel maps          \
    used as textures, that zero pixels (irrespective of any palette information) are not rendered,        \
    and so have the effect of transparency. This only applies to textures and not to pixel map            \
    operations such as BrPixelmapCopy().                                                                  \
    */                                                                                                    \
    /**                                                                                                   \
    \li \ref BR_PMT_INDEX_1 — 1 bit index into a colour map (2 colours)                                   \
    \li \ref BR_PMT_INDEX_2 — 2 bit index into a colour map (4 colours)                                   \
    \li \ref BR_PMT_INDEX_4 — 4 bit index into a colour map (16 colours)                                  \
    \li \ref BR_PMT_INDEX_8 — 8 bit index into a colour map (256 colours)                                 \
    \li \ref BR_PMT_RGB_555 — 16 bit 'true colour' RGB, 5 bits each colour                                \
    \li \ref BR_PMT_RGB_565 — 16 bit 'true colour' RGB, 5 bits red and blue, 6 bits green                 \
    \li \ref BR_PMT_RGB_888 — 24 bit 'true colour' RGB, 8 bits each colour                                \
    \li \ref BR_PMT_RGBX_888 — 32 bit 'true colour' RGB, 8 bits each colour, 8 bits unused                \
    \li \ref BR_PMT_RGBA_8888 — 32 bit 'true colour' RGB, 8 bits each colour with an 8 bit alpha channel  \
    \li \ref BR_PMT_DEPTH_16 — The pixel map is used as a depth buffer with 16 bit precision              \
    \li \ref BR_PMT_DEPTH_32 — The pixel map is used as a depth buffer with 32 bit precision              \
    */                                                                                                    \
    br_uint_8 prefix##type;                                                                               \
                                                                                                          \
    /**                                                                                                   \
    \brief This is a read-only member, set upon allocation, that contains various flag values.            \
    */                                                                                                    \
    /**                                                                                                   \
    One of the flags that may be useful is BR_PMF_NO_ACCESS which will be set if the pixel data is        \
    stored at pixels. If not set then pixels is invalid and there is no direct access to pixel data.      \
    */                                                                                                    \
    br_uint_16 prefix##flags;                                                                             \
                                                                                                          \
    /**                                                                                                   \
    \brief These members define the top left of the start of pixel map data in terms of base_y as a       \
           number of row_bytes, and base_x as a smaller offset from this.                                 \
    */                                                                                                    \
    br_uint_16 prefix##base_x;                                                                            \
    br_uint_16 prefix##base_y;                                                                            \
                                                                                                          \
    /**                                                                                                   \
    \brief These members contain the dimensions of the visible region of the pixel map.                   \
    */                                                                                                    \
    br_uint_16 prefix##width;                                                                             \
    br_uint_16 prefix##height;                                                                            \
                                                                                                          \
    /**                                                                                                   \
    \brief These members define the position of the co-ordinate origin of the pixel map relative to       \
           the base origin (given by base_x, base_y).                                                     \
    */                                                                                                    \
    /**                                                                                                   \
    Thus a point plotted at (0,0) will be plotted at column origin_x from base_x. The co-ordinate         \
    origin also effectively defines the centre of projection when used as a rendering destination.        \
    */                                                                                                    \
    br_int_16 prefix##origin_x;                                                                           \
    br_int_16 prefix##origin_y;                                                                           \
                                                                                                          \
    /**                                                                                                   \
    \brief This member may be used by the application for its own purposes.                               \
    */                                                                                                    \
    /**                                                                                                   \
    It is initialised to NULL upon allocation (if allocated by BRender), and not accessed by BRender      \
    thereafter.                                                                                           \
    */                                                                                                    \
    void *prefix##user;                                                                                   \
    void *prefix##stored;
// clang-format on

#define BR_PIXELMAP_MEMBERS BR_PIXELMAP_MEMBERS_PREFIXED(pm_)

/**
 * \brief BRender's pixel map structure, used for texture maps, shade tables, blend tables, colour
 *        buffers and Z-buffers.
 *
 * See Image Support. Texture maps, shade tables, and blend tables being required for rendering
 * materials should be maintained within the registry as necessary.
 */
typedef struct br_pixelmap {
    br_uintptr_t _reserved;

    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * If the pixel map is loaded or imported, the identifier will have been set using BrResStrDup().
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

typedef struct br_pixelmap_convert_options {
    /**
     * \brief The alpha transparency threshold for indexed pixelmaps.
     *
     * Any alpha <= this will be considered transparent, if treating index-0 as transparent.
     *
     * \remark This is only meaningful when writing an indexed pixelmap, and if \p index_0_transparent is set.
     */
    br_uint_8 index_alpha_threshold;

    /**
     * \brief The target CLUT.
     *
     * If writing to an indexed pixelmap, use this CLUT instead of generating one.
     */
    br_pixelmap *target_clut;

    /**
     * \brief Should colour keying be enabled?
     *
     * If set, any pixels where the RGB matches \p colour_key will have their alpha set to 0 (fully transparent).
     */
    br_uint_8 enable_colour_key;

    /**
     * \brief The colour key. Only the RGB fields are meaningful.
     */
    br_colour colour_key;
} br_pixelmap_convert_options;

#endif
