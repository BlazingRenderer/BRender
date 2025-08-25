/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 * Copyright (c) 2005 Zane van Iperen
 *
 * $Id: devpixmp.c 1.2 1998/10/21 15:41:12 jon Exp $
 * $Locker: $
 *
 * Device pixelmap methods for VGA mode 13h (MCGA).
 *
 * These routines are not particular well-optimised, but that doesn't matter
 * because you should be double-buffering anyway :)
 */
#include <stddef.h>
#include <string.h>
#include <sys/segments.h>
#include <sys/farptr.h>
#include <pc.h>

#include "drv.h"
#include "pm.h"
#include "brassert.h"

/*
 * Display mode and stride are fixed for MCGA
 */
#define BIOS_MODE   0x13
#define BIOS_STRIDE 320

/*
 * Default dispatch table for device pixelmap (defined at end of file)
 */
static const struct br_device_pixelmap_dispatch devicePixelmapDispatch;

/*
 * Device pixelmap info. template
 */
#define F(f) offsetof(struct br_device_pixelmap, f)

static br_tv_template_entry devicePixelmapTemplateEntries[] = {
    {BRT_WIDTH_I32,         0, F(pm_width),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT_HEIGHT_I32,        0, F(pm_height),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT_PIXEL_TYPE_U8,     0, F(pm_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8 },
    //	{BRT_PIXEL_CHANNELS_TL,	0,	0,					BRTV_QUERY | BRTV_ALL,	BRTV_CONV_CUSTOM },
    //	{BRT_INDEXED_B,			0,	F(indexed),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY },
    {BRT_OUTPUT_FACILITY_O, 0, F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT_FACILITY_O,        0, F(output_facility), BRTV_QUERY,            BRTV_CONV_COPY   },
    {BRT_IDENTIFIER_CSTR,   0, F(pm_identifier),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT_CLUT_O,            0, F(clut),            BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
};
#undef F

/*
 * Create a new device pixelmap and set a display mode
 */
br_device_pixelmap *DevicePixelmapMCGAAllocateMode(br_device *dev, br_output_facility *facility, br_uint_16 w, br_uint_16 h)
{
    br_device_pixelmap *self;
    br_uint_16          original_mode;

    if(dev->active)
        return NULL;

    /*
     * _DEV_ Setup mode
     */
    original_mode = BIOSVideoGetMode();
    if(BIOSVideoSetMode(BIOS_MODE) != BRE_OK)
        return NULL;

    DeviceVGACurrentModeSet(dev, BIOS_MODE);
    dev->active = BR_TRUE;

    self = BrResAllocate(DeviceVGAResource(dev), sizeof(*self), BR_MEMORY_OBJECT_DATA);

    self->dispatch      = &devicePixelmapDispatch;
    self->pm_identifier = ObjectIdentifier(facility);
    self->device        = dev;
    self->restore_mode  = BR_TRUE;
    self->original_mode = original_mode;

    self->vga_sel = __dpmi_segment_to_descriptor(0xA000);
    self->my_sel  = _my_ds();

    self->pm_type      = OutputFacilityVGAType(facility);
    self->pm_width     = w;
    self->pm_height    = h;
    self->pm_row_bytes = BIOS_STRIDE;
    self->pm_origin_x  = 0;
    self->pm_origin_y  = 0;

    self->pm_pixels = NULL;
    self->pm_flags  = BR_PMF_NO_ACCESS;
    self->pm_base_x = 0;
    self->pm_base_y = 0;

    self->output_facility = facility;
    self->clut            = DeviceVGAClut(dev);

    self->tmp_row = BrResAllocate(self, self->pm_row_bytes, BR_MEMORY_SCRATCH);

    self->output_facility->num_instances++;
    ObjectContainerAddFront(facility, (br_object *)self);

    return self;
}

static void BR_CMETHOD_DECL(br_device_pixelmap_mcga, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    ObjectContainerRemove(self->output_facility, (br_object *)self);
    self->output_facility->num_instances--;

    /*
     * Restore video mode
     */
    if(self->restore_mode) {
        BIOSVideoSetMode(self->original_mode);
        DeviceVGACurrentModeSet(ObjectDevice(self), self->original_mode);
        ObjectDevice(self)->active = BR_FALSE;
    }

    /*
     * Free up pixelmap structure
     */
    BrResFreeNoCallback(self);
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_mcga, device)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;
    return self->device;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_mcga, type)(br_object *self)
{
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_mcga, isType)(br_object *self, br_token t)
{
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_mcga, space)(br_object *self)
{
    return sizeof(br_device_pixelmap);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_vga, queryTemplate)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapTemplate == NULL)
        self->device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(self->device, devicePixelmapTemplateEntries,
                                                                              BR_ASIZE(devicePixelmapTemplateEntries));

    return self->device->templates.devicePixelmapTemplate;
}

static br_uintptr_t DevicePixelmapMCGAMemOffset(br_device_pixelmap *self, br_int_32 x, br_int_32 y)
{
    return (br_uintptr_t)self->pm_pixels + ((self->pm_base_y + y) * BIOS_STRIDE) + ((self->pm_base_x + x) * 1);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, validSource)(br_device_pixelmap *self, br_boolean *bp, br_object *h)
{
    (void)self;
    (void)h;
    *bp = BR_FALSE;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    if(width == self->pm_width && height == self->pm_height)
        return BRE_OK;

    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, rectangleCopyTo)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                         br_rectangle *r)
{
    br_rectangle ar;
    br_point     ap;
    br_boolean   is_contig;

    UASSERT(self->pm_type == src->pm_type);

    if(PixelmapRectangleClipTwo(&ar, &ap, r, p, (br_pixelmap *)self, (br_pixelmap *)src) == BR_CLIP_REJECT)
        return BRE_OK;

    is_contig = (src->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS);

    if(ar.w == self->pm_width && ar.h == self->pm_height && self->pm_row_bytes == src->pm_row_bytes && is_contig) {
        void        *pp;
        br_uint_32   pqual;
        br_uintptr_t vga_off;
        br_point     dp = {
                .x = -src->pm_origin_x + ap.x,
                .y = -src->pm_origin_y + ap.y,
        };
        if(DevicePixelmapPixelAddressQuery(src, &pp, &pqual, &dp) != BRE_OK)
            return BRE_FAIL;

        vga_off = DevicePixelmapMCGAMemOffset(self, ar.x, ar.y);
        movedata(self->my_sel, (unsigned)pp, self->vga_sel, vga_off, ar.w * ar.h);
    } else {
        for(br_int_32 y = 0; y < ar.h; y++) {
            void        *pp;
            br_uint_32   pqual;
            br_uintptr_t vga_off;
            br_point     dp = {
                    .x = -src->pm_origin_x + ap.x,
                    .y = -src->pm_origin_y + ap.y + y,
            };
            if(DevicePixelmapPixelAddressQuery(src, &pp, &pqual, &dp) != BRE_OK)
                return BRE_FAIL;

            vga_off = DevicePixelmapMCGAMemOffset(self, ar.x, ar.y + y);
            movedata(self->my_sel, (br_uintptr_t)pp, self->vga_sel, vga_off, ar.w);
        }
    }

    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, rectangleCopy)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                       br_rectangle *r)
{
    UASSERT(self->pm_type == src->pm_type);

    /*
     * This will be the case if we're from a "match()"'d pixelmap. Same device, but using
     * memory dispatch. Punt us off to our memory copy.
     */
    if(src->dispatch != self->dispatch)
        return BR_CMETHOD_REF(br_device_pixelmap_vga, rectangleCopyTo)(self, p, src, r);

    /*
     * This should never be hit as there'll never be front screens.
     */
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *dest,
                                                                           br_rectangle *r)
{
    /*
     * Why are you copying _from_ the screen? No. Implement this yourself.
     */
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    br_rectangle arect;

    if(PixelmapRectangleClip(&arect, rect, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    /*
     * Fill our temporary row.
     */
    BrMemSet(self->tmp_row, (int)colour, arect.w);

    /*
     * Now copy it, row-by-row.
     */
    for(br_int_32 y = arect.y; y < arect.y + arect.h; ++y) {
        const br_uintptr_t base = DevicePixelmapMCGAMemOffset(self, 0, y);
        movedata(self->my_sel, (unsigned)self->tmp_row, self->vga_sel, base, arect.w);
    }

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    br_point ap;

    if(PixelmapPointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    _farpokeb(self->vga_sel, DevicePixelmapMCGAMemOffset(self, p->x, p->y), (char)colour);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, pixelQuery)(br_device_pixelmap *self, br_uint_32 *pcolour, br_point *p)
{
    br_point ap;

    if(PixelmapPointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    *pcolour = _farpeekb(self->vga_sel, DevicePixelmapMCGAMemOffset(self, p->x, p->y));
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, synchronise)(br_device_pixelmap *self, br_token sync_type, br_boolean block)
{
    unsigned long spins;

    if(sync_type != BRT_VERTICAL_BLANK || !block)
        return BRE_UNSUPPORTED;

    /* http://www.techhelpmanual.com/899-enhanced_graphics_adapter_i_o_ports.html */

    /*
     * Finish our current vblank.
     */
    spins = 200000UL;
    while(((inportb(0x03DA) & 0x08) == 0x08) && spins--)
        ;

    if(spins == 0)
        return BRE_FAIL;

    /*
     * Wait for the next to start.
     */
    spins = 200000UL;
    while(((inportb(0x03DA) & 0x08) == 0x00) && spins--)
        ;

    if(spins == 0)
        return BRE_FAIL;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    br_error r;

    if((r = BR_CMETHOD_REF(br_device_pixelmap_vga, synchronise)(self, BRT_VERTICAL_BLANK, BR_TRUE)) != BRE_OK)
        return r;

    /*
     * "Fast"-path. If we're the same size/type, and they have linear+whole rows, just do a copy.
     */
    if(src->pm_width == self->pm_width && src->pm_height == self->pm_height && src->pm_type == self->pm_type &&
       ((src->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))) {

        movedata(self->my_sel, (br_uintptr_t)src->pm_pixels, self->vga_sel, 0, self->pm_width * self->pm_height);
        return BRE_OK;
    }

    return BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBuffer)(self, src);
}

/*
 * Default dispatch table for device pixelmap
 */
static const struct br_device_pixelmap_dispatch devicePixelmapDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_mcga, free),
    ._identifier = BR_CMETHOD_REF(br_object_vga, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_mcga, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_mcga, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_mcga, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_mcga, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_vga, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_vga, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_vga, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_mem, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_fail, allocateSub),

    ._copy         = BR_CMETHOD_REF(br_device_pixelmap_gen, copy),
    ._copyTo       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyTo),
    ._copyFrom     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFrom),
    ._fill         = BR_CMETHOD_REF(br_device_pixelmap_gen, fill),
    ._doubleBuffer = BR_CMETHOD_REF(br_device_pixelmap_vga, doubleBuffer),

    ._copyDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, copyDirty),
    ._copyToDirty       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyToDirty),
    ._copyFromDirty     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFromDirty),
    ._fillDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, fillDirty),
    ._doubleBufferDirty = BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBufferDirty),

    ._rectangle                = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle),
    ._rectangle2               = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle2),
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_vga, rectangleCopy),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_vga, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_vga, rectangleCopyFrom),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyTo),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_vga, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_vga, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_gen, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_gen, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_gen, text),
    ._textBounds = BR_CMETHOD_REF(br_device_pixelmap_gen, textBounds),

    ._rowSize  = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSize),
    ._rowQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, rowQuery),
    ._rowSet   = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSet),

    ._pixelQuery        = BR_CMETHOD_REF(br_device_pixelmap_vga, pixelQuery),
    ._pixelAddressQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressQuery),

    ._pixelAddressSet = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressSet),
    ._originSet       = BR_CMETHOD_REF(br_device_pixelmap_gen, originSet),

    ._flush        = BR_CMETHOD_REF(br_device_pixelmap_gen, flush),
    ._synchronise  = BR_CMETHOD_REF(br_device_pixelmap_vga, synchronise),
    ._directLock   = BR_CMETHOD_REF(br_device_pixelmap_fail, directLock),
    ._directUnlock = BR_CMETHOD_REF(br_device_pixelmap_fail, directUnlock),

    ._getControls = BR_CMETHOD_REF(br_device_pixelmap_gen, getControls),
    ._setControls = BR_CMETHOD_REF(br_device_pixelmap_gen, setControls),
};
