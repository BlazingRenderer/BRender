/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: devpixmp.c 1.1 1997/12/10 16:53:37 jon Exp $
 * $Locker: $
 *
g */
#include <stddef.h>
#include <stdlib.h>
#include <go32.h>
#include <pc.h>
#include <sys/farptr.h>

#include "drv.h"
#include "pm.h"
#include "brassert.h"

/*
 * Device pixelmap info. template
 */
#define F(f) offsetof(struct br_device_pixelmap, f)

static struct br_tv_template_entry devicePixelmapTemplateEntries[] = {
    {BRT(WIDTH_I32),         F(pm_width),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT(HEIGHT_I32),        F(pm_height),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT(PIXEL_TYPE_U8),     F(pm_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8 },
    //	{BRT(PIXEL_CHANNELS_TL),0,					BRTV_QUERY | BRTV_ALL,	BRTV_CONV_CUSTOM },
    //	{BRT(INDEXED_B,)		F(indexed),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY },
    {BRT(OUTPUT_FACILITY_O), F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT(FACILITY_O),        F(output_facility), BRTV_QUERY,            BRTV_CONV_COPY   },
    {BRT(IDENTIFIER_CSTR),   F(pm_identifier),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT(CLUT_O),            F(clut),            BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
};
#undef F

static int pixelBytes(br_device_pixelmap *pm)
{
    switch(pm->pm_type) {
        case BR_PMT_RGB_555:
        case BR_PMT_RGB_565:
        case BR_PMT_DEPTH_16:
            return 2;

        case BR_PMT_RGB_888:
            return 3;

        case BR_PMT_RGBX_888:
        case BR_PMT_RGBA_8888:
            return 4;

        default:
            return 1;
    }
}

/*
 * Create a new device pixelmap and set a display mode
 */
br_device_pixelmap *DevicePixelmapVESAAllocateMode(br_device *dev, br_output_facility *facility, br_uint_16 w, br_uint_16 h)
{
    br_device_pixelmap *self;
    br_uint_16          original_mode;
    struct vesa_work   *vw;

    if(dev->screen_active)
        return NULL;

    vw = DeviceVESAWork(dev);

    /*
     * _DEV_ Setup mode
     */
    VESAModeGet(&original_mode);

    /*
     * See if linear access will work
     */
    if(!DeviceVESALinearAvailable(dev, OutputFacilityVESAModeInfo(facility))) {
        return NULL;
    }

    /*
     * Put the device into the desired mode
     */
    if(VESAModeSet(OutputFacilityVESAMode(facility) | VESA_MODE_LINEAR) != BRE_OK)
        return NULL;

    DeviceVESACurrentModeSet(dev, OutputFacilityVESAMode(facility) | VESA_MODE_LINEAR);

    if(DeviceVESALinearInitialise(dev, OutputFacilityVESAModeInfo(facility)) != BRE_OK) {
        BIOSVideoSetMode(original_mode);
        DeviceVESACurrentModeSet(dev, original_mode);
        return NULL;
    }

    dev->screen_active = BR_TRUE;

    /*
     * Build self
     */
    self = BrResAllocate(DeviceVESAResource(dev), sizeof(*self), BR_MEMORY_OBJECT_DATA);

    self->dispatch = &devicePixelmapDispatch_l;
    self->device   = dev;

    self->pm_flags |= BR_PMF_NO_ACCESS;
    self->pm_pixels = NULL;

    self->restore_mode  = BR_TRUE;
    self->original_mode = original_mode;

    self->pm_type      = OutputFacilityVESAType(facility);
    self->pm_width     = w;
    self->pm_height    = h;
    self->pm_origin_x  = 0;
    self->pm_origin_y  = 0;
    self->pm_row_bytes = vw->stride;

    self->pm_base_x = 0;
    self->pm_base_y = 0;

    self->output_facility = facility;

    if(self->pm_width * pixelBytes(self) == self->pm_row_bytes)
        self->pm_flags |= BR_PMF_LINEAR;
    else
        self->pm_flags &= ~BR_PMF_LINEAR;

    if(self->pm_type == BR_PMT_INDEX_8)
        self->clut = DeviceVESAClut(dev);

    // FIXME:
    self->pm_flags |= BR_PMF_ROW_WHOLEPIXELS;

    /*
     * Build a descriptive identifier string
     */
    self->pm_identifier = BrResSprintf(self, "%dx%dx%d Linear Protected", self->pm_width, self->pm_height, OutputFacilityVESABits(facility));

    self->output_facility->num_instances++;
    ObjectContainerAddFront(facility, (br_object *)self);

    return self;
}

static void BR_CMETHOD_DECL(br_device_pixelmap_vesa, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    ObjectContainerRemove(self->output_facility, (br_object *)self);
    self->output_facility->num_instances--;

    /*
     * Restore video mode
     */
    if(self->restore_mode) {
        // VESAModeSet(self->original_mode);
        BIOSVideoSetMode(self->original_mode);
        DeviceVESACurrentModeSet(ObjectDevice(self), self->original_mode);
        ObjectDevice(self)->screen_active = BR_FALSE;
    }

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_pixelmap_vesa, identifier)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;
    return self->pm_identifier;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_vesa, type)(br_object *self)
{
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_vesa, isType)(br_object *self, br_token t)
{
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_vesa, device)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;
    return self->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_vesa, space)(br_object *self)
{
    return sizeof(br_device_pixelmap);
}

static br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_vesa, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapTemplate == NULL)
        self->device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(self->device, devicePixelmapTemplateEntries,
                                                                              BR_ASIZE(devicePixelmapTemplateEntries));

    return self->device->templates.devicePixelmapTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, validSource)(br_device_pixelmap *self, br_boolean *bp, br_object *h)
{
    (void)self;
    (void)h;
    *bp = BR_FALSE;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    if(width == self->pm_width && height == self->pm_height)
        return BRE_OK;

    return BRE_FAIL;
}

static br_uintptr_t DevicePixelmapVESAMemOffset(br_device_pixelmap *self, br_int_32 x, br_int_32 y)
{
    const int bytes_per_pixel = self->device->work.bits_per_pixel >> 3;
    return (br_uintptr_t)self->pm_pixels + ((self->pm_base_y + y) * self->pm_row_bytes) + ((self->pm_base_x + x) * bytes_per_pixel);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, rectangleCopyTo)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
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

        vga_off = DevicePixelmapVESAMemOffset(self, ar.x, ar.y);
        movedata(_my_ds(), (unsigned)pp, self->device->work.selector, vga_off, self->pm_row_bytes * self->pm_height);
    } else {
        int bytes_per_pixel = self->device->work.bits_per_pixel >> 3;
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

            vga_off = DevicePixelmapVESAMemOffset(self, ar.x, ar.y + y);
            movedata(_my_ds(), (br_uintptr_t)pp, self->device->work.selector, vga_off, ar.w * bytes_per_pixel);
        }
    }

    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, rectangleCopy)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                        br_rectangle *r)
{
    UASSERT(self->pm_type == src->pm_type);

    /*
     * This will be the case if we're from a "match()"'d pixelmap. Same device, but using
     * memory dispatch. Punt us off to our memory copy.
     */
    if(src->dispatch != self->dispatch)
        return BR_CMETHOD_REF(br_device_pixelmap_vesa, rectangleCopyTo)(self, p, src, r);

    /*
     * This should never be hit as there'll never be front screens.
     */
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *dest,
                                                                            br_rectangle *r)
{
    /*
     * Why are you copying _from_ the screen? No. Implement this yourself.
     */
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    br_point     ap;
    int          bytes_per_pixel;
    br_uintptr_t offset;

    if(PixelmapPointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    bytes_per_pixel = self->device->work.bits_per_pixel >> 3;
    offset          = DevicePixelmapVESAMemOffset(self, p->x, p->y);

    if(PixelmapPointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    switch(bytes_per_pixel) {
        case 8:
            _farpokeb(self->device->work.selector, offset, colour);
            break;
        case 16:
            _farpokew(self->device->work.selector, offset, colour);
            break;
        case 32:
            _farpokel(self->device->work.selector, offset, colour);
            break;
        default:
            return BRE_FAIL;
    }

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, pixelQuery)(br_device_pixelmap *self, br_uint_32 *pcolour, br_point *p)
{
    br_point     ap;
    int          bytes_per_pixel;
    br_uintptr_t offset;

    if(PixelmapPointClip(&ap, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    bytes_per_pixel = self->device->work.bits_per_pixel >> 3;
    offset          = DevicePixelmapVESAMemOffset(self, p->x, p->y);

    switch(bytes_per_pixel) {
        case 8:
            *pcolour = _farpeekb(self->device->work.selector, offset);
            break;
        case 16:
            *pcolour = _farpeekw(self->device->work.selector, offset);
            break;
        case 32:
            *pcolour = _farpeekl(self->device->work.selector, offset);
            break;
        default:
            return BRE_FAIL;
    }

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, synchronise)(br_device_pixelmap *self, br_token sync_type, br_boolean block)
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

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vesa, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    br_error r;

    if((r = BR_CMETHOD_REF(br_device_pixelmap_vesa, synchronise)(self, BRT_VERTICAL_BLANK, BR_TRUE)) != BRE_OK)
        return r;

    /*
     * "Fast"-path. If we're the same size/type, and they have linear+whole rows, just do a copy.
     */
    if(src->pm_width == self->pm_width && src->pm_height == self->pm_height && src->pm_type == self->pm_type &&
       ((src->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS))) {

        movedata(_my_ds(), (br_uintptr_t)src->pm_pixels, self->device->work.selector, 0, self->pm_width * self->pm_height * 2);
        return BRE_OK;
    }

    return BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBuffer)(self, src);
}

/*
 * Default dispatch table for device pixelmap - linear
 */
struct br_device_pixelmap_dispatch devicePixelmapDispatch_l = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_vesa, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_vesa, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_vesa, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_vesa, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_vesa, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_vesa, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_vesa, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_vesa, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_vesa, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_mem, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_fail, allocateSub),

    ._copy         = BR_CMETHOD_REF(br_device_pixelmap_gen, copyTo),
    ._copyTo       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyTo),
    ._copyFrom     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFrom),
    ._fill         = BR_CMETHOD_REF(br_device_pixelmap_gen, fill),
    ._doubleBuffer = BR_CMETHOD_REF(br_device_pixelmap_vesa, doubleBuffer),

    ._copyDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, copyDirty),
    ._copyToDirty       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyToDirty),
    ._copyFromDirty     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFromDirty),
    ._fillDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, fillDirty),
    ._doubleBufferDirty = BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBufferDirty),

    ._rectangle                = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle),
    ._rectangle2               = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle2),
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_vesa, rectangleCopyTo),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_vesa, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_vesa, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyTo),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_vesa, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_vesa, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_gen, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_gen, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_gen, text),
    ._textBounds = BR_CMETHOD_REF(br_device_pixelmap_gen, textBounds),

    ._rowSize  = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSize),
    ._rowQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, rowQuery),
    ._rowSet   = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSet),

    ._pixelQuery        = BR_CMETHOD_REF(br_device_pixelmap_vesa, pixelQuery),
    ._pixelAddressQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressQuery),

    ._pixelAddressSet = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressSet),
    ._originSet       = BR_CMETHOD_REF(br_device_pixelmap_gen, originSet),

    ._flush        = BR_CMETHOD_REF(br_device_pixelmap_gen, flush),
    ._synchronise  = BR_CMETHOD_REF(br_device_pixelmap_vesa, synchronise),
    ._directLock   = BR_CMETHOD_REF(br_device_pixelmap_fail, directLock),
    ._directUnlock = BR_CMETHOD_REF(br_device_pixelmap_fail, directUnlock),

    ._getControls = BR_CMETHOD_REF(br_device_pixelmap_gen, getControls),
    ._setControls = BR_CMETHOD_REF(br_device_pixelmap_gen, setControls),

    ._handleWindowEvent = BR_CMETHOD_REF(br_device_pixelmap_gen, handleWindowEvent),
};
