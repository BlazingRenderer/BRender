/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: pmmem.c 1.5 1998/08/10 16:12:27 johng Exp $
 * $Locker: $
 *
 * Memory pixelmap implementation
 */
#include <stddef.h>

#define BR_DEVICE_PIXELMAP_PRIVATE

#include "pm.h"
#include "pmmem.h"
#include "brassert.h"

/*
 * Useful info about each pixelmap type
 */
// clang-format off
struct pm_type_info pmTypeInfo[] = {
    [BR_PMT_INDEX_1]   = {.bits =  1, .file_size = 1, .align = 32, .channels = BR_PMCHAN_INDEX                  },
    [BR_PMT_INDEX_2]   = {.bits =  2, .file_size = 1, .align = 16, .channels = BR_PMCHAN_INDEX                  },
    [BR_PMT_INDEX_4]   = {.bits =  4, .file_size = 1, .align =  8, .channels = BR_PMCHAN_INDEX                  },
    [BR_PMT_INDEX_8]   = {.bits =  8, .file_size = 1, .align =  4, .channels = BR_PMCHAN_INDEX                  },
    [BR_PMT_RGB_555]   = {.bits = 16, .file_size = 2, .align =  2, .channels = BR_PMCHAN_RGB                    },
    [BR_PMT_RGB_565]   = {.bits = 16, .file_size = 2, .align =  2, .channels = BR_PMCHAN_RGB                    },
    [BR_PMT_RGB_888]   = {.bits = 24, .file_size = 3, .align =  4, .channels = BR_PMCHAN_RGB                    },
    [BR_PMT_RGBX_888]  = {.bits = 32, .file_size = 4, .align =  1, .channels = BR_PMCHAN_RGB                    },
    [BR_PMT_RGBA_8888] = {.bits = 32, .file_size = 4, .align =  1, .channels = BR_PMCHAN_RGB | BR_PMCHAN_ALPHA  },
    [BR_PMT_YUYV_8888] = {.bits = 16, .file_size = 1, .align =  2, .channels = BR_PMCHAN_YUV                    },
    [BR_PMT_YUV_888]   = {.bits = 32, .file_size = 1, .align =  1, .channels = BR_PMCHAN_YUV                    },
    [BR_PMT_DEPTH_16]  = {.bits = 16, .file_size = 2, .align =  4, .channels = BR_PMCHAN_DEPTH                  },
    [BR_PMT_DEPTH_32]  = {.bits = 32, .file_size = 4, .align =  4, .channels = BR_PMCHAN_DEPTH                  },
    [BR_PMT_ALPHA_8]   = {.bits =  8, .file_size = 1, .align =  4, .channels = BR_PMCHAN_ALPHA                  },

    /*****These may be wrong and need updating. *****/
    [BR_PMT_AINDEX_88]      = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    [BR_PMT_NORMAL_INDEX_8] = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    [BR_PMT_NORMAL_XYZ]     = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    [BR_PMT_BGR_555]        = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    [BR_PMT_RGBA_4444]      = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    [BR_PMT_RBG_bab]        = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    [BR_PMT_RBG_1aba]       = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    /********************************************/

    /* Accelerator extensions. */
    [BR_PMT_RGB_332]    = {.bits =  8, .file_size = 1, .align = 4,  .channels = BR_PMCHAN_RGB                    },
    [BR_PMT_DEPTH_8]    = {.bits =  8, .file_size = 1, .align = 4,  .channels = BR_PMCHAN_DEPTH                  },
    [BR_PMT_ARGB_8888]  = {.bits = 32, .file_size = 4, .align = 1,  .channels = BR_PMCHAN_RGB | BR_PMCHAN_ALPHA  },
    [BR_PMT_ALPHA_4]    = {.bits =  4, .file_size = 1, .align = 8,  .channels = BR_PMCHAN_ALPHA                  },
    [BR_PMT_INDEXA_44]  = {.bits =  8, .file_size = 1, .align = 4,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    [BR_PMT_DEPTH_15]   = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_DEPTH                  },
    [BR_PMT_DEPTH_31]   = {.bits = 32, .file_size = 4, .align = 1,  .channels = BR_PMCHAN_DEPTH                  },
    [BR_PMT_DEPTH_FP16] = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_DEPTH                  },
    [BR_PMT_DEPTH_FP15] = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_DEPTH                  },

    [BR_PMT_RGBA_5551] = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    [BR_PMT_ARGB_1555] = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},
    [BR_PMT_ARGB_4444] = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_INDEX | BR_PMCHAN_ALPHA},

    [BR_PMT_RGBA_8888_ARR] = {.bits = 32, .file_size = 4, .align = 1,  .channels = BR_PMCHAN_RGB | BR_PMCHAN_ALPHA  },
    [BR_PMT_BGR_565]       = {.bits = 16, .file_size = 2, .align = 2,  .channels = BR_PMCHAN_RGB                    },
    [BR_PMT_DEPTH_24]      = {.bits = 24, .file_size = 3, .align = 4,  .channels = BR_PMCHAN_DEPTH                  },
    [BR_PMT_DEPTH_FP32]    = {.bits = 32, .file_size = 4, .align = 4,  .channels = BR_PMCHAN_DEPTH                  },

    [BR_PMT_R8G8B8A8]      = {.bits = 32, .file_size = 4, .align = 4,  .channels = BR_PMCHAN_RGB | BR_PMCHAN_ALPHA},
};
// clang-format on

BR_STATIC_ASSERT(sizeof(pmTypeInfo) / sizeof(pmTypeInfo[0]) == BR_PMT_MAX, "Pixel type array size mismatch");

/*
 * Default dispatch table for device pixelmap (defined at end of file)
 */
static struct br_device_pixelmap_dispatch devicePixelmapDispatch;

/*
 * Device pixelmap info. template
 */
#define F(f) offsetof(struct br_device_pixelmap, f)

// clang-format off
static struct br_tv_template_entry devicePixelmapTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR,    NULL, F(pm_identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT_WIDTH_I32,          NULL, F(pm_width),      BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT_HEIGHT_I32,         NULL, F(pm_height),     BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT_PIXEL_TYPE_U8,      NULL, F(pm_type),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8 },
    // {BRT_PIXEL_CHANNELS_I32, NULL, 0,                BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM },
    // {BRT_PIXEL_CHANNELS_TL,  NULL, 0,                BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM },
    // {BRT_INDEXED_B,          NULL, F(pm_indexed),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM },
    // {BRT_ORIGIN_V2_I,        NULL, F(pm_indexed),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM },
    // {BRT_PIXEL_BITS_I32,     NULL, F(pm_indexed),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM },
    // {BRT_WORD_BYTES_I32,     NULL, F(pm_indexed),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM },
    // {BRT_MEMORY_MAPPED,      NULL, F(pm_indexed),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM },
};
// clang-format on
#undef F

br_device_pixelmap *DevicePixelmapMemAllocate(br_uint_8 type, br_uint_16 w, br_uint_16 h, void *pixels, int flags)
{
    br_device_pixelmap        *pm;
    const struct pm_type_info *tip = pmTypeInfo + type;

    pm = BrResAllocate(_pixelmap.res, sizeof(*pm), BR_MEMORY_PIXELMAP);

    UASSERT(type < BR_ASIZE(pmTypeInfo));

    /*
     * Fill in base structure
     */
    pm->dispatch = &devicePixelmapDispatch;

    pm->pm_identifier    = NULL;
    pm->pm_type          = type;
    pm->pm_map           = NULL;
    pm->pm_flags         = 0;
    pm->pm_copy_function = BR_PMCOPY_NORMAL;
    pm->pm_base_x        = 0;
    pm->pm_base_y        = 0;

    pm->pm_width  = w;
    pm->pm_height = h;

    pm->pm_origin_x = 0;
    pm->pm_origin_y = 0;

    /*
     * Work out size of a row
     */
    pm->pm_row_bytes = tip->bits * tip->align * ((w + tip->align - 1) / tip->align) / 8;

    if(((pm->pm_row_bytes * 8) % tip->bits) == 0)
        pm->pm_flags |= BR_PMF_ROW_WHOLEPIXELS;

    /*
     * Set linear flag if there's no alignment padding.
     */
    if((pm->pm_width * tip->bits) / 8 == pm->pm_row_bytes)
        pm->pm_flags |= BR_PMF_LINEAR;

    /*
     * Allocate pixels
     */
    if(!(flags & BR_PMAF_NO_PIXELS)) {
        if(pixels)
            pm->pm_pixels = pixels;
        else
            pm->pm_pixels = BrResAllocate(pm, pm->pm_row_bytes * pm->pm_height, BR_MEMORY_PIXELS);
    }

    /*
     * Make it a bottom up bitmap if required
     */
    if(flags & BR_PMAF_INVERTED) {
        pm->pm_pixels    = (char *)pm->pm_pixels + pm->pm_row_bytes * (pm->pm_height - 1);
        pm->pm_row_bytes = -pm->pm_row_bytes;
    }

    return (br_device_pixelmap *)pm;
}

/*
 * Comatibility hack to catch any pixelmaps with NULL dispatch pointers and
 * redirect them to the memory dispatch table
 */
void _CheckDispatch(br_device_pixelmap *pm)
{
    if(pm->dispatch == NULL)
        pm->dispatch = &devicePixelmapDispatch;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_rectangle *rect)
{
    br_device_pixelmap *pm;
    br_rectangle        out;

    /*
     * Create sub-window (clipped against original)
     */
    if(PixelmapRectangleClip(&out, rect, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    /*
     * Create the new structure and copy
     */
    pm = BrResAllocate(_pixelmap.res, sizeof(*pm), BR_MEMORY_PIXELMAP);

    *pm = *self;

    pm->pm_base_x += (br_uint_16)out.x;
    pm->pm_base_y += (br_uint_16)out.y;
    pm->pm_width  = (br_uint_16)out.w;
    pm->pm_height = (br_uint_16)out.h;

    pm->pm_origin_x = 0;
    pm->pm_origin_y = 0;

    pm->pm_stored = NULL;

    /*
     * Set dispatch table
     */
    pm->dispatch = &devicePixelmapDispatch;

    /*
     * Pixel rows may not be contiguous
     */
    if(self->pm_width != pm->pm_width)
        pm->pm_flags &= ~BR_PMF_LINEAR;

    *newpm = (br_device_pixelmap *)pm;

    return BRE_OK;
}

static void BR_CMETHOD_DECL(br_device_pixelmap_mem, free)(br_object *self)
{
    BrResFree(self);
}

static const char *BR_CMETHOD_DECL(br_device_pixelmap_mem, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_mem, type)(br_object *self)
{
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_mem, isType)(br_object *self, br_token t)
{
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static struct br_device *BR_CMETHOD_DECL(br_device_pixelmap_mem, device)(br_object *self)
{
    /*
     * Memory pixelmaps are not associated with a device
     */
    return NULL;
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_mem, space)(br_object *self)
{
    return BrResSizeTotal(self);
}

/*
 * Token/Value support methods
 */
static struct br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_mem, queryTemplate)(br_object *self)
{
    if(_pixelmap.device_pixelmap_template == NULL)
        _pixelmap.device_pixelmap_template = BrTVTemplateAllocate(_pixelmap.res, devicePixelmapTemplateEntries,
                                                                  BR_ASIZE(devicePixelmapTemplateEntries));

    return _pixelmap.device_pixelmap_template;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, validSource)(br_device_pixelmap *self, br_boolean *bp, br_object *h)
{
    /* ... */
    *bp = BR_FALSE;
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    char                *pixels;
    struct pm_type_info *tip = pmTypeInfo + self->pm_type;
    br_int_16            old_row_bytes;

    /*
     * Free up old pixels, if any
     */
    pixels = self->pm_pixels;
    if(self->pm_row_bytes < 0)
        pixels += self->pm_row_bytes * (self->pm_height - 1);
    if(BrResIsChild(self, pixels))
        BrResFree(pixels);

    self->pm_width  = (br_uint_16)width;
    self->pm_height = (br_uint_16)height;

    /*
     * Work out new size of a row
     */
    old_row_bytes      = self->pm_row_bytes;
    self->pm_row_bytes = tip->bits * tip->align * ((width + tip->align - 1) / tip->align) / 8;

    if(((self->pm_row_bytes * 8) % tip->bits) == 0)
        self->pm_flags |= BR_PMF_ROW_WHOLEPIXELS;

    /*
     * Allocate pixels
     */
    self->pm_pixels = BrResAllocate(self, self->pm_row_bytes * self->pm_height, BR_MEMORY_PIXELS);

    /*
     * Make it a bottom up bitmap if required
     */
    if(old_row_bytes < 0) {
        self->pm_pixels    = (char *)self->pm_pixels + self->pm_row_bytes * (self->pm_height - 1);
        self->pm_row_bytes = -self->pm_row_bytes;
    }

    return BRE_OK;
}

/*
 * Structure used to unpack the 'match' tokens/values
 */
struct match_tokens {
    br_token   use;
    br_uint_8  pixel_type;
    br_int_32  pixel_bits;
    br_int_32  width;
    br_int_32  height;
    br_object *renderer;
};

#define F(f) offsetof(struct match_tokens, f)

static struct br_tv_template_entry matchTemplateEntries[] = {
    {BRT_USE_T,          NULL, F(use),        BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_TYPE_U8,  NULL, F(pixel_type), BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_BITS_I32, NULL, F(pixel_bits), BRTV_SET, BRTV_CONV_COPY},
    {BRT_RENDERER_O,     NULL, F(renderer),   BRTV_SET, BRTV_CONV_COPY},
    {BRT_WIDTH_I32,      NULL, F(width),      BRTV_SET, BRTV_CONV_COPY},
    {BRT_HEIGHT_I32,     NULL, F(height),     BRTV_SET, BRTV_CONV_COPY},
};
#undef F

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv)
{
    struct match_tokens mt = {BRT_NONE, 0};
    br_int_32           count;
    br_device_pixelmap *pm;
    br_int_32           bytes, r;

    if(_pixelmap.pixelmap_match_template == NULL) {
        _pixelmap.pixelmap_match_template = BrTVTemplateAllocate(_pixelmap.res, matchTemplateEntries, BR_ASIZE(matchTemplateEntries));

        if(_pixelmap.pixelmap_match_template == NULL)
            return BRE_FAIL;
    }

    /*
     * Default type, width and height
     */
    mt.pixel_type = self->pm_type;
    mt.width      = self->pm_width;
    mt.height     = self->pm_height;

    BrTokenValueSetMany(&mt, &count, NULL, tv, _pixelmap.pixelmap_match_template);

    switch(mt.use) {
        case BRT_DEPTH:
            switch(mt.pixel_bits) {
                case 0:
                case 8:
                    mt.pixel_type = BR_PMT_DEPTH_8;
                    break;

                case 16:
                    mt.pixel_type = BR_PMT_DEPTH_16;
                    break;

                case 32:
                    mt.pixel_type = BR_PMT_DEPTH_32;
                    break;

                default:
                    return BRE_FAIL;
            }
            break;

        case BRT_CLONE:
            return DevicePixelmapMemCloneTyped(self, mt.pixel_type, newpm);

        case BRT_OFFSCREEN:
        case BRT_HIDDEN:
        case BRT_HIDDEN_BUFFER:
        case BRT_NO_RENDER:
            break;

        default:
            return BRE_FAIL;
    }

    /*
     * XXX - Make strides match, query renderer for it's requirements
     */
    if(mt.use == BRT_DEPTH) {
        /*
         * Allocate basic structure, no pixels
         */
        pm    = DevicePixelmapMemAllocate(mt.pixel_type, (br_uint_16)mt.width, (br_uint_16)mt.height, NULL,
                                          ((self->pm_row_bytes < 0) ? BR_PMAF_INVERTED : 0) | BR_PMAF_NO_PIXELS);
        bytes = DevicePixelmapPixelBytes(self);

        /*
         * Work out a matching stride for depth map
         */
        r = self->pm_row_bytes;
        if(r < 0)
            r = -r;

        r                = (r + bytes - 1) / bytes;
        pm->pm_row_bytes = r * DevicePixelmapPixelBytes(pm);

        pm->pm_pixels = BrResAllocate(pm, pm->pm_row_bytes * pm->pm_height, BR_MEMORY_PIXELS);

        /*
         * Make sure 'linear' bit is set correctly
         */
        if(pm->pm_width * DevicePixelmapPixelBytes(pm) == pm->pm_row_bytes)
            pm->pm_flags |= BR_PMF_LINEAR;
        else
            pm->pm_flags &= ~BR_PMF_LINEAR;

        if(self->pm_row_bytes < 0) {
            pm->pm_pixels    = (char *)pm->pm_pixels + pm->pm_row_bytes * (pm->pm_height - 1);
            pm->pm_row_bytes = -pm->pm_row_bytes;
        }
    } else {
        pm = DevicePixelmapMemAllocate(mt.pixel_type, (br_uint_16)mt.width, (br_uint_16)mt.height, NULL,
                                       (self->pm_row_bytes < 0) ? BR_PMAF_INVERTED : 0);
    }

    /*
     * Copy origin over
     */
    pm->pm_origin_x = self->pm_origin_x;
    pm->pm_origin_y = self->pm_origin_y;

    *newpm = pm;

    return BRE_OK;
}

/*
 * Operations on whole pixemap (with versions that include a hint dirty rectangle)
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyTo)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    br_int_8 bytes;

    UASSERT((self->pm_type == src->pm_type) && (self->pm_width == src->pm_width) && (self->pm_height == src->pm_height));

    bytes = DevicePixelmapPixelBytes(self);

    /*
     * Do a colour keyed copy if necessary
     */
    if(src->pm_copy_function & BR_PMCOPY_SRC_KEYED) {
#if 0
		/*
		 * Don't do generic colour keying at the moment
		 */
		if (self->pm_copy_function & BR_PMCOPY_DST_KEYED ||
			src->pm_src_key.low != 0 || src->pm_src_key.high != 0)

			return BRE_UNSUPPORTED;

		/*
		 * Do block copy when both pixelmaps have same stride and have
		 * adjoining rows
		 */
		if (self->pm_row_bytes == src->pm_row_bytes &&
			(self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) ==
			(BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) &&
			(src->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) ==
			(BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

			if(self->pm_row_bytes > 0)

				_MemCopySourceColourKey0_A(
					DevicePixelmapMemAddress(self,0,0,bytes),self->pm_pixels_qualifier,
					DevicePixelmapMemAddress(src,0,0,bytes),src->pm_pixels_qualifier,
					self->pm_width * self->pm_height, bytes);
			else
				_MemCopySourceColourKey0_A(
					DevicePixelmapMemAddress(self,0,self->pm_height-1,bytes),self->pm_pixels_qualifier,
					DevicePixelmapMemAddress(src,0,src->pm_height-1,bytes),src->pm_pixels_qualifier,
					self->pm_width * self->pm_height, bytes);

		else

			_MemRectCopySourceColourKey0_A(
				DevicePixelmapMemAddress(self,0,0,bytes),self->pm_pixels_qualifier,
				DevicePixelmapMemAddress(src,0,0,bytes),src->pm_pixels_qualifier,
				self->pm_width,self->pm_height,
				self->pm_row_bytes,src->pm_row_bytes,bytes);

		return BRE_OK;
#endif
        return BRE_UNSUPPORTED;
    }

    /*
     * Do block copy when both pixelmaps have same stride and have
     * adjoining rows
     */
    if(self->pm_row_bytes == src->pm_row_bytes &&
       (self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) &&
       (src->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

        if(self->pm_row_bytes > 0)

            _MemCopy_A(DevicePixelmapMemAddress(self, 0, 0, bytes), DevicePixelmapMemAddress(src, 0, 0, bytes),
                       self->pm_width * self->pm_height, bytes);
        else
            _MemCopy_A(DevicePixelmapMemAddress(self, 0, self->pm_height - 1, bytes),
                       DevicePixelmapMemAddress(src, 0, src->pm_height - 1, bytes), self->pm_width * self->pm_height, bytes);

    else if((self->pm_row_bytes & 7) == 0)

        _MemRectCopy_A(self->pm_pixels, src->pm_pixels, self->pm_width, self->pm_height, self->pm_row_bytes, src->pm_row_bytes, bytes);

    else {

        char      *s, *d;
        br_uint_32 y;

        s = DevicePixelmapMemAddress(src, 0, 0, bytes);
        d = DevicePixelmapMemAddress(self, 0, 0, bytes);

        for(y = 0; y < self->pm_height; y++) {

            _MemCopy_A(d, s, self->pm_width, bytes);

            s += src->pm_row_bytes;
            d += self->pm_row_bytes;
        }
    }

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyFrom)(br_device_pixelmap *self, br_device_pixelmap *dest)
{
    br_int_8 bytes;

    UASSERT((self->pm_type == dest->pm_type) && (self->pm_width == dest->pm_width) && (self->pm_height == dest->pm_height));

    bytes = DevicePixelmapPixelBytes(self);

    /*
     * Do a colour keyed copy if necessary
     */
    if(self->pm_copy_function & BR_PMCOPY_SRC_KEYED) {
#if 0
		/*
		 * Don't do generic colour keying at the moment
		 */
		if (dest->pm_copy_function & BR_PMCOPY_DST_KEYED ||
			self->pm_src_key.low != 0 || self->pm_src_key.high != 0)

			return BRE_UNSUPPORTED;

		/*
		 * Do block copy when both pixelmaps have same stride and have
		 * adjoining rows
		 */
		if (self->pm_row_bytes == dest->pm_row_bytes &&
			(self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) ==
			(BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) &&
			(dest->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) ==
			(BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

			if(self->pm_row_bytes > 0)

				_MemCopySourceColourKey0_A(
					DevicePixelmapMemAddress(dest,0,0,bytes),dest->pm_pixels_qualifier,
					DevicePixelmapMemAddress(self,0,0,bytes),self->pm_pixels_qualifier,
					self->pm_width * self->pm_height, bytes);
			else
				_MemCopySourceColourKey0_A(
					DevicePixelmapMemAddress(dest,0,0,bytes),dest->pm_pixels_qualifier,
					DevicePixelmapMemAddress(self,0,self->pm_height-1,bytes),self->pm_pixels_qualifier,
					self->pm_width * self->pm_height, bytes);

		else

			_MemRectCopySourceColourKey0_A(
				DevicePixelmapMemAddress(dest,0,0,bytes),dest->pm_pixels_qualifier,
				DevicePixelmapMemAddress(self,0,0,bytes),self->pm_pixels_qualifier,
				self->pm_width,self->pm_height,
				dest->pm_row_bytes,self->pm_row_bytes,bytes);

		return BRE_OK;
#endif
        return BRE_UNSUPPORTED;
    }

    /*
     * Do block copy when both pixelmaps have same stride and have
     * adjoining rows
     */
    if(self->pm_row_bytes == dest->pm_row_bytes &&
       (self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) &&
       (dest->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

        if(self->pm_row_bytes > 0)

            _MemCopy_A(DevicePixelmapMemAddress(dest, 0, 0, bytes), DevicePixelmapMemAddress(self, 0, 0, bytes),
                       self->pm_width * self->pm_height, bytes);
        else
            _MemCopy_A(DevicePixelmapMemAddress(dest, 0, dest->pm_height - 1, bytes),
                       DevicePixelmapMemAddress(self, 0, self->pm_height - 1, bytes), self->pm_width * self->pm_height, bytes);

    else if((dest->pm_row_bytes & 7) == 0)

        _MemRectCopy_A(dest->pm_pixels, self->pm_pixels, self->pm_width, self->pm_height, dest->pm_row_bytes, self->pm_row_bytes, bytes);

    else {

        char      *s, *d;
        br_uint_32 y;

        s = DevicePixelmapMemAddress(self, 0, 0, bytes);
        d = DevicePixelmapMemAddress(dest, 0, 0, bytes);

        for(y = 0; y < self->pm_height; y++) {

            _MemCopy_A(d, s, self->pm_width, bytes);

            s += self->pm_row_bytes;
            d += dest->pm_row_bytes;
        }
    }

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, fill)(br_device_pixelmap *self, br_uint_32 colour)
{
    br_int_8 bytes;

    bytes = DevicePixelmapPixelBytes(self);

    /*
     * Do block fill when pixelmap has adjoining rows
     */
    if((self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

        if(self->pm_row_bytes > 0)

            _MemFill_A(DevicePixelmapMemAddress(self, 0, 0, bytes), self->pm_width * self->pm_height, bytes, colour);
        else
            _MemFill_A(DevicePixelmapMemAddress(self, 0, self->pm_height - 1, bytes), self->pm_width * self->pm_height, bytes, colour);

    else {

        char      *d;
        br_uint_32 y;

        d = self->pm_pixels;

        for(y = 0; y < self->pm_height; y++) {

            _MemFill_A(d, self->pm_width, bytes, colour);

            d += self->pm_row_bytes;
        }
    }

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    return BRE_UNSUPPORTED;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyDirty)(br_device_pixelmap *self, br_device_pixelmap *src, br_rectangle *dirty, br_int_32 num_rects)
{
    return BRE_UNSUPPORTED;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyToDirty)(br_device_pixelmap *self, br_device_pixelmap *src, br_rectangle *dirty,
                                                              br_int_32 num_rects)
{
    return BRE_UNSUPPORTED;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyFromDirty)(br_device_pixelmap *self, br_device_pixelmap *src, br_rectangle *dirty,
                                                                br_int_32 num_rects)
{
    return BRE_UNSUPPORTED;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, fillDirty)(br_device_pixelmap *self, br_uint_32 colour, br_rectangle *dirty, br_int_32 num_rects)
{
    return BRE_UNSUPPORTED;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, doubleBufferDirty)(br_device_pixelmap *self, br_device_pixelmap *src, br_rectangle *dirty,
                                                                    br_int_32 num_rects)
{
    return BRE_UNSUPPORTED;
}

/*
 * Direct 2D rendering operations
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleCopyTo)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src, br_rectangle *r)
{
    int          bytes;
    br_rectangle ar;
    br_point     ap;

    UASSERT(self->pm_type == src->pm_type);

    if(PixelmapRectangleClipTwo(&ar, &ap, r, p, (br_pixelmap *)self, (br_pixelmap *)src) == BR_CLIP_REJECT)
        return BRE_OK;

    bytes = DevicePixelmapPixelBytes(self);
    /*
     * Do a colour keyed copy if necessary
     */
    if(src->pm_copy_function & BR_PMCOPY_SRC_KEYED) {
#if 0
		/*
		 * Don't do generic colour keying at the moment
		 */
		if (self->pm_copy_function & BR_PMCOPY_DST_KEYED ||
			src->pm_src_key.low != 0 || src->pm_src_key.high != 0)

			return BRE_UNSUPPORTED;

		/*
		 * Try to do full width copies with block copy
		 */
		if (ar.w == self->pm_width && self->pm_row_bytes == src->pm_row_bytes &&
			(self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) ==
			(BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) &&
			(src->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) ==
			(BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

			if(self->pm_row_bytes > 0)

				_MemCopySourceColourKey0_A(
					DevicePixelmapMemAddress(self,ap.x,ap.y,bytes),self->pm_pixels_qualifier,
					DevicePixelmapMemAddress(src,ar.x,ar.y,bytes),src->pm_pixels_qualifier,
					ar.w * ar.h,bytes);
			else
				_MemCopySourceColourKey0_A(
					DevicePixelmapMemAddress(self,ap.x,ap.y+ar.h-1,bytes),self->pm_pixels_qualifier,
					DevicePixelmapMemAddress(src,ar.x,ar.y+ar.h-1,bytes),src->pm_pixels_qualifier,
					ar.w * ar.h,bytes);

		else

			_MemRectCopySourceColourKey0_A(
				DevicePixelmapMemAddress(self,ap.x,ap.y,bytes),self->pm_pixels_qualifier,
				DevicePixelmapMemAddress(src,ar.x,ar.y,bytes),src->pm_pixels_qualifier,
				ar.w,ar.h,self->pm_row_bytes,src->pm_row_bytes,bytes);

		return BRE_OK;
#endif
        return BRE_UNSUPPORTED;
    }

    /*
     * Try to do full width copies with block copy
     */
    if(ar.w == self->pm_width && self->pm_row_bytes == src->pm_row_bytes &&
       (self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) &&
       (src->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

        if(self->pm_row_bytes > 0)

            _MemCopy_A(DevicePixelmapMemAddress(self, ap.x, ap.y, bytes), DevicePixelmapMemAddress(src, ar.x, ar.y, bytes), ar.w * ar.h, bytes);
        else
            _MemCopy_A(DevicePixelmapMemAddress(self, ap.x, ap.y + ar.h - 1, bytes),
                       DevicePixelmapMemAddress(src, ar.x, ar.y + ar.h - 1, bytes), ar.w * ar.h, bytes);

    /*
     * Rectangle copy only works when qword alignment is the same from row
     * to row
     */
    else if((self->pm_row_bytes & 7) == 0)

        _MemRectCopy_A(DevicePixelmapMemAddress(self, ap.x, ap.y, bytes), DevicePixelmapMemAddress(src, ar.x, ar.y, bytes), ar.w, ar.h,
                       self->pm_row_bytes, src->pm_row_bytes, bytes);

    else {

        char     *s, *d;
        br_int_32 y;

        s = DevicePixelmapMemAddress(src, ar.x, ar.y, bytes);
        d = DevicePixelmapMemAddress(self, ap.x, ap.y, bytes);

        for(y = 0; y < ar.h; y++) {

            _MemCopy_A(d, s, ar.w, bytes);

            s += src->pm_row_bytes;
            d += self->pm_row_bytes;
        }
    }

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *dest,
                                                                    br_rectangle *r)
{
    int          bytes;
    br_rectangle ar;
    br_point     ap;

    UASSERT(self->pm_type == dest->pm_type);

    if(PixelmapRectangleClipTwo(&ar, &ap, r, p, (br_pixelmap *)dest, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    bytes = DevicePixelmapPixelBytes(self);

    /*
     * Do a colour keyed copy if necessary
     */
    if(self->pm_copy_function & BR_PMCOPY_SRC_KEYED) {
#if 0
		/*
		 * Don't do generic colour keying at the moment
		 */
		if (dest->pm_copy_function & BR_PMCOPY_DST_KEYED ||
			self->pm_src_key.low != 0 || self->pm_src_key.high != 0)

			return BRE_UNSUPPORTED;

		/*
		 * Try to do full width copies with block copy
		 */
		if (ar.w == self->pm_width && self->pm_row_bytes == dest->pm_row_bytes &&
			(self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) ==
			(BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) &&
			(dest->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) ==
			(BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

			if(self->pm_row_bytes > 0)

				_MemCopySourceColourKey0_A(
					DevicePixelmapMemAddress(dest,ap.x,ap.y,bytes),dest->pm_pixels_qualifier,
					DevicePixelmapMemAddress(self,ar.x,ar.y,bytes),self->pm_pixels_qualifier,
					ar.w * ar.h,bytes);
			else
				_MemCopySourceColourKey0_A(
					DevicePixelmapMemAddress(dest,ap.x,ap.y+ar.h-1,bytes),dest->pm_pixels_qualifier,
					DevicePixelmapMemAddress(self,ar.x,ar.y+ar.h-1,bytes),self->pm_pixels_qualifier,
					ar.w * ar.h,bytes);

		else

			_MemRectCopySourceColourKey0_A(
				DevicePixelmapMemAddress(dest,ap.x,ap.y,bytes),dest->pm_pixels_qualifier,
				DevicePixelmapMemAddress(self,ar.x,ar.y,bytes),self->pm_pixels_qualifier,
				ar.w,ar.h,dest->pm_row_bytes,self->pm_row_bytes,bytes);

		return BRE_OK;
#endif
        return BRE_UNSUPPORTED;
    }

    /*
     * Try to do full width copies with block copy
     */
    if(ar.w == self->pm_width && self->pm_row_bytes == dest->pm_row_bytes &&
       (self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) &&
       (dest->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

        if(self->pm_row_bytes > 0)

            _MemCopy_A(DevicePixelmapMemAddress(dest, ap.x, ap.y, bytes), DevicePixelmapMemAddress(self, ar.x, ar.y, bytes), ar.w * ar.h, bytes);
        else
            _MemCopy_A(DevicePixelmapMemAddress(dest, ap.x, ap.y + ar.h - 1, bytes),
                       DevicePixelmapMemAddress(self, ar.x, ar.y + ar.h - 1, bytes), ar.w * ar.h, bytes);

    /*
     * Rectangle copy only works when qword alignment is the same from row
     * to row
     */
    else if((dest->pm_row_bytes & 7) == 0)

        _MemRectCopy_A(DevicePixelmapMemAddress(dest, ap.x, ap.y, bytes), DevicePixelmapMemAddress(self, ar.x, ar.y, bytes), ar.w, ar.h,
                       dest->pm_row_bytes, self->pm_row_bytes, bytes);

    else {

        char     *s, *d;
        br_int_32 y;

        s = DevicePixelmapMemAddress(self, ar.x, ar.y, bytes);
        d = DevicePixelmapMemAddress(dest, ap.x, ap.y, bytes);

        for(y = 0; y < ar.h; y++) {

            _MemCopy_A(d, s, ar.w, bytes);

            s += self->pm_row_bytes;
            d += dest->pm_row_bytes;
        }
    }

    return BRE_OK;
}

// absoloute value for an integer
#define I_ABS(x) ((x) >= 0 ? (x) : -(x))
// positive part of an integer
#define I_POS(x) ((x) >= 0 ? (x) : 0)
// negative part of an integer
#define I_NEG(x) ((x) <= 0 ? (x) : 0)
// absoloute value of negative part of an integer
#define I_ABS_NEG(x) ((x) <= 0 ? (-x) : 0)

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *destinationRectangle,
                                                                         br_device_pixelmap *src, br_rectangle *sourceRectangle)
{
#if 0
	int bytes;
	int width,height;
	int sourceX,sourceY,destinationX,destinationY;
	int sourceClipLeft,sourceClipRight,sourceClipTop,sourceClipBottom;
	int destinationClipLeft,destinationClipRight,destinationClipTop,destinationClipBottom;  

	br_pixelmap *source,*destination;
	float widthRatio,heightRatio;
	
	destination=(br_pixelmap *)self;
	source=(br_pixelmap *)src;

	ASSERT(destination->type==source->type);

	widthRatio=(float)sourceRectangle->w/(float)destinationRectangle->w;
	heightRatio=(float)sourceRectangle->h/(float)destinationRectangle->h;

//positive values encroach onto image, negative define a border around it.

	sourceClipLeft=sourceRectangle->x+source->origin_x;
	sourceClipTop=sourceRectangle->y+source->origin_y;
	sourceClipRight=source->width-(sourceRectangle->x+sourceRectangle->w+source->origin_x);
	sourceClipBottom=source->height-(sourceRectangle->y+sourceRectangle->h+source->origin_y);

	destinationClipLeft=destinationRectangle->x+destination->origin_x;
	destinationClipTop=destinationRectangle->y+destination->origin_y;
	destinationClipRight=destination->width-(destinationRectangle->x+destinationRectangle->w+destination->origin_x);
	destinationClipBottom=destination->height-(destinationRectangle->y+destinationRectangle->h+destination->origin_y);

	width=destination->width-I_POS(destinationClipLeft)-I_POS(destinationClipRight);
	height=destination->height-I_POS(destinationClipTop)-I_POS(destinationClipBottom);

	sourceX=I_POS(sourceClipLeft);
	sourceY=I_POS(sourceClipTop);
	destinationX=I_POS(destinationClipLeft);
	destinationY=I_POS(destinationClipTop);
	
	bytes = DevicePixelmapPixelBytes(self);

	switch(destination->type){
		case BR_PMT_INDEX_8:

			stretchBlit8(
				DevicePixelmapMemAddress(self,destinationX,destinationY,bytes),
				destination->row_bytes,
				DevicePixelmapMemAddress(src,sourceX,sourceY,bytes),
				source->row_bytes,
				width,
				height,
				widthRatio,
				heightRatio
			);

			return BRE_OK;

		default:
			return BRE_FAIL;
	}
#endif
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleStretchCopyFrom)(br_device_pixelmap *self, br_rectangle *d,
                                                                           br_device_pixelmap *src, br_rectangle *s)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    br_rectangle arect;
    br_int_8     bytes;

    if(PixelmapRectangleClip(&arect, rect, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    bytes = DevicePixelmapPixelBytes(self);

    /*
     * Try to do full width fills with block fill
     */
    if(arect.w == self->pm_width && (self->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))

        if(self->pm_row_bytes > 0)

            _MemFill_A(DevicePixelmapMemAddress(self, arect.x, arect.y, bytes), arect.w * arect.h, bytes, colour);
        else
            _MemFill_A(DevicePixelmapMemAddress(self, arect.x, arect.y + arect.h - 1, bytes), arect.w * arect.h, bytes, colour);

    else {

        br_int_32 y;

        for(y = arect.y; y < arect.y + arect.h; y++) {

            _MemFill_A(DevicePixelmapMemAddress(self, arect.x, y, bytes), arect.w, bytes, colour);
        }
    }

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    br_point ap;
    br_int_8 bytes;

    if(PixelmapPointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    bytes = DevicePixelmapPixelBytes(self);

    _MemPixelSet(DevicePixelmapMemAddress(self, ap.x, ap.y, bytes), bytes, colour);

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, line)(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
    int      dx, dy, incr1, incr2, D, x, y, xend, c, pixels_left;
    int      x1, y1;
    int      sign_x, sign_y, step, reverse, i;
    char    *dest;
    int      bytes;
    br_point as, ae;

    if(PixelmapLineClip(&as, &ae, s, e, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    /*
     * Symmetric Double Step Line Algorithm by Brian Wyvill from "Graphics Gems",
     * Academic Press, 1990
     */

#define LINEPLOT(x, y, flag)                                    \
    do {                                                        \
        if(flag) {                                              \
            dest = DevicePixelmapMemAddress(self, y, x, bytes); \
        } else {                                                \
            dest = DevicePixelmapMemAddress(self, x, y, bytes); \
        }                                                       \
        _MemPixelSet(dest, bytes, colour);                      \
    } while(0)

#define swap(a, b) \
    {              \
        a ^= b;    \
        b ^= a;    \
        a ^= b;    \
    }
#define absolute(i, j, k) ((i - j) * (k = ((i - j) < 0 ? -1 : 1)))

    bytes = DevicePixelmapPixelBytes(self);

    dx = absolute(ae.x, as.x, sign_x);
    dy = absolute(ae.y, as.y, sign_y);

    if(dx == 0 && dy == 0) {
        dest = DevicePixelmapMemAddress(self, as.x, as.y, bytes);
        _MemPixelSet(dest, bytes, colour);
        return BRE_OK;
    };

    /* decide increment sign by the slope sign */
    if(sign_x == sign_y)
        step = 1;
    else
        step = -1;

    if(dy > dx) { /* chooses axis of greatest movement (make dx) */
        swap(as.x, as.y);
        swap(ae.x, ae.y);
        swap(dx, dy);
        reverse = 1;
    } else
        reverse = 0;

    /* note error check for dx==0 should be included here */
    if(as.x > ae.x) { /* start from the smaller coordinate */
        x  = ae.x;
        y  = ae.y;
        x1 = as.x;
        y1 = as.y;
    } else {
        x  = as.x;
        y  = as.y;
        x1 = ae.x;
        y1 = ae.y;
    }

    /* Note dx=n implies 0 - n or (dx+1) pixels to be set */
    /* Go round loop dx/4 times then plot last 0,1,2 or 3 pixels */
    /* In fact (dx-1)/4 as 2 pixels are already plottted */

    xend        = (dx - 1) / 4;
    pixels_left = (dx - 1) % 4; /* number of pixels left over at the end */
    LINEPLOT(x, y, reverse);
    LINEPLOT(x1, y1, reverse); /* plot first two points */
    incr2 = 4 * dy - 2 * dx;
    if(incr2 < 0) { /* slope less than 1/2 */
        c     = 2 * dy;
        incr1 = 2 * c;
        D     = incr1 - dx;

        for(i = 0; i < xend; i++) { /* plotting loop */
            ++x;
            --x1;
            if(D < 0) {
                /* pattern 1 forwards */
                LINEPLOT(x, y, reverse);
                LINEPLOT(++x, y, reverse);
                /* pattern 1 backwards */
                LINEPLOT(x1, y1, reverse);
                LINEPLOT(--x1, y1, reverse);
                D += incr1;
            } else {
                if(D < c) {
                    /* pattern 2 forwards */
                    LINEPLOT(x, y, reverse);
                    LINEPLOT(++x, y += step, reverse);
                    /* pattern 2 backwards */
                    LINEPLOT(x1, y1, reverse);
                    LINEPLOT(--x1, y1 -= step, reverse);
                } else {
                    /* pattern 3 forwards */
                    LINEPLOT(x, y += step, reverse);
                    LINEPLOT(++x, y, reverse);
                    /* pattern 3 backwards */
                    LINEPLOT(x1, y1 -= step, reverse);
                    LINEPLOT(--x1, y1, reverse);
                }
                D += incr2;
            }
        } /* end for */

        /* plot last pattern */
        if(pixels_left) {
            if(D < 0) {
                LINEPLOT(++x, y, reverse); /* pattern 1 */
                if(pixels_left > 1)
                    LINEPLOT(++x, y, reverse);
                if(pixels_left > 2)
                    LINEPLOT(--x1, y1, reverse);
            } else {
                if(D < c) {
                    LINEPLOT(++x, y, reverse); /* pattern 2  */
                    if(pixels_left > 1)
                        LINEPLOT(++x, y += step, reverse);
                    if(pixels_left > 2)
                        LINEPLOT(--x1, y1, reverse);
                } else {
                    /* pattern 3 */
                    LINEPLOT(++x, y += step, reverse);
                    if(pixels_left > 1)
                        LINEPLOT(++x, y, reverse);
                    if(pixels_left > 2)
                        LINEPLOT(--x1, y1 -= step, reverse);
                }
            }
        } /* end if pixels_left */
    }
    /* end slope < 1/2 */
    else { /* slope greater than 1/2 */
        c     = 2 * (dy - dx);
        incr1 = 2 * c;
        D     = incr1 + dx;
        for(i = 0; i < xend; i++) {
            ++x;
            --x1;
            if(D > 0) {
                /* pattern 4 forwards */
                LINEPLOT(x, y += step, reverse);
                LINEPLOT(++x, y += step, reverse);
                /* pattern 4 backwards */
                LINEPLOT(x1, y1 -= step, reverse);
                LINEPLOT(--x1, y1 -= step, reverse);
                D += incr1;
            } else {
                if(D < c) {
                    /* pattern 2 forwards */
                    LINEPLOT(x, y, reverse);
                    LINEPLOT(++x, y += step, reverse);

                    /* pattern 2 backwards */
                    LINEPLOT(x1, y1, reverse);
                    LINEPLOT(--x1, y1 -= step, reverse);
                } else {
                    /* pattern 3 forwards */
                    LINEPLOT(x, y += step, reverse);
                    LINEPLOT(++x, y, reverse);
                    /* pattern 3 backwards */
                    LINEPLOT(x1, y1 -= step, reverse);
                    LINEPLOT(--x1, y1, reverse);
                }
                D += incr2;
            }
        } /* end for */
        /* plot last pattern */
        if(pixels_left) {
            if(D > 0) {
                LINEPLOT(++x, y += step, reverse); /* pattern 4 */
                if(pixels_left > 1)
                    LINEPLOT(++x, y += step, reverse);
                if(pixels_left > 2)
                    LINEPLOT(--x1, y1 -= step, reverse);
            } else {
                if(D < c) {
                    LINEPLOT(++x, y, reverse); /* pattern 2  */
                    if(pixels_left > 1)
                        LINEPLOT(++x, y += step, reverse);
                    if(pixels_left > 2)
                        LINEPLOT(--x1, y1, reverse);
                } else {
                    /* pattern 3 */
                    LINEPLOT(++x, y += step, reverse);
                    if(pixels_left > 1)
                        LINEPLOT(++x, y, reverse);
                    if(pixels_left > 2) {
                        if(D > c) /* step 3 */
                            LINEPLOT(--x1, y1 -= step, reverse);
                        else /* step 2 */
                            LINEPLOT(--x1, y1, reverse);
                    }
                }
            }
        }
    }

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyBits)(br_device_pixelmap *self, br_point *point, const br_uint_8 *src,
                                                           br_uint_16 s_stride, br_rectangle *bit_rect, br_uint_32 colour)
{
    int          bytes, bit;
    br_rectangle ar;
    br_point     ap;

    if(PixelmapCopyBitsClip(&ar, &ap, bit_rect, point, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    bytes = DevicePixelmapPixelBytes(self);
    bit   = (ar.x & 7);

    _MemCopyBits_A(DevicePixelmapMemAddress(self, ap.x + (ar.x & ~7) - bit, ap.y, bytes), self->pm_row_bytes, src + ar.y * s_stride,
                   s_stride, bit, bit + ar.w, ar.h, bytes, colour);

    return BRE_OK;
}

/*
 * Reading back
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, pixelQuery)(br_device_pixelmap *self, br_uint_32 *pcolour, br_point *p)
{
    br_point ap;
    br_int_8 bytes;

    if(PixelmapPointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    bytes = DevicePixelmapPixelBytes(self);

    *pcolour = _MemPixelGet(DevicePixelmapMemAddress(self, ap.x, ap.y, bytes), bytes);

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, pixelAddressQuery)(br_device_pixelmap *self, void **pptr, br_uint_32 *pqual, br_point *p)
{
    br_int_8 bytes;
    br_point ap;

    if(PixelmapPointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    bytes = DevicePixelmapPixelBytes(self);

    if(pptr)
        *pptr = DevicePixelmapMemAddress(self, ap.x, ap.y, bytes);

    return BRE_OK;
}

/*
 * Updating the pixelmap
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, pixelAddressSet)(br_device_pixelmap *self, void *ptr, br_uint_32 *qual)
{
    if(ptr != NULL)
        self->pm_pixels = ptr;

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, originSet)(br_device_pixelmap *self, br_point *p)
{
    self->pm_origin_x = (br_uint_16)p->x;
    self->pm_origin_y = (br_uint_16)p->y;

    return BRE_OK;
}

/*
 * Row read/write
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rowSize)(br_device_pixelmap *self, br_size_t *sizep)
{
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rowQuery)(br_device_pixelmap *self, void *buffer, br_size_t buffer_size, br_uint_32 row)
{
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rowSet)(br_device_pixelmap *self, void *buffer, br_size_t buffer_size, br_uint_32 row)
{
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, flush)(struct br_device_pixelmap *self)
{
    return BRE_OK;
}
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, synchronise)(struct br_device_pixelmap *self, br_token sync_type, br_boolean block)
{
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, directLock)(struct br_device_pixelmap *self, br_boolean block)
{
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, directUnlock)(struct br_device_pixelmap *self)
{
    return BRE_OK;
}

/*
 * Default dispatch table for device pixelmap
 */
static struct br_device_pixelmap_dispatch devicePixelmapDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_mem, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_mem, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_mem, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_mem, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_mem, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_mem, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_mem, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_mem, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_mem, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_mem, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_mem, allocateSub),

    ._copy              = BR_CMETHOD_REF(br_device_pixelmap_mem, copyTo),
    ._copyTo            = BR_CMETHOD_REF(br_device_pixelmap_mem, copyTo),
    ._copyFrom          = BR_CMETHOD_REF(br_device_pixelmap_mem, copyFrom),
    ._fill              = BR_CMETHOD_REF(br_device_pixelmap_mem, fill),
    ._doubleBuffer      = BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBuffer),
    ._copyDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, copyDirty),
    ._copyToDirty       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyToDirty),
    ._copyFromDirty     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFromDirty),
    ._fillDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, fillDirty),
    ._doubleBufferDirty = BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBufferDirty),

    ._rectangle                = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle),
    ._rectangle2               = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle2),
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleCopyTo),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleStretchCopyTo),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_mem, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_mem, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_mem, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_gen, text),
    ._textBounds = BR_CMETHOD_REF(br_device_pixelmap_gen, textBounds),

    ._rowSize  = BR_CMETHOD_REF(br_device_pixelmap_mem, rowSize),
    ._rowQuery = BR_CMETHOD_REF(br_device_pixelmap_mem, rowQuery),
    ._rowSet   = BR_CMETHOD_REF(br_device_pixelmap_mem, rowSet),

    ._pixelQuery        = BR_CMETHOD_REF(br_device_pixelmap_mem, pixelQuery),
    ._pixelAddressQuery = BR_CMETHOD_REF(br_device_pixelmap_mem, pixelAddressQuery),

    ._pixelAddressSet = BR_CMETHOD_REF(br_device_pixelmap_mem, pixelAddressSet),
    ._originSet       = BR_CMETHOD_REF(br_device_pixelmap_mem, originSet),

    ._flush        = BR_CMETHOD_REF(br_device_pixelmap_mem, flush),
    ._synchronise  = BR_CMETHOD_REF(br_device_pixelmap_mem, synchronise),
    ._directLock   = BR_CMETHOD_REF(br_device_pixelmap_mem, directLock),
    ._directUnlock = BR_CMETHOD_REF(br_device_pixelmap_mem, directUnlock),

    ._getControls = BR_CMETHOD_REF(br_device_pixelmap_gen, getControls),
    ._setControls = BR_CMETHOD_REF(br_device_pixelmap_gen, setControls),

    ._handleWindowEvent = BR_CMETHOD_REF(br_device_pixelmap_gen, handleWindowEvent),
};
