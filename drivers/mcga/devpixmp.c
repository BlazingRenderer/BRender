/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: devpixmp.c 1.2 1998/10/21 15:41:12 jon Exp $
 * $Locker: $
 *
 * Device pixelmap methods
 */
#include <stddef.h>
#include <string.h>
#include <sys/nearptr.h>
#include <pc.h>

#include "drv.h"
#include "pm.h"

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

    self->meminfo.address = 0x000A0000;
    self->meminfo.size    = 0x10000;
    self->meminfo.handle  = 0;
    if(__dpmi_physical_address_mapping(&self->meminfo) != 0) {
        if(self->restore_mode) {
            DeviceVGACurrentModeSet(dev, original_mode);
            dev->active = BR_FALSE;
            return NULL;
        }
    }

    self->pm_pixels = (void *)(self->meminfo.address + __djgpp_conventional_base);

    self->pm_type      = OutputFacilityVGAType(facility);
    self->pm_width     = w;
    self->pm_height    = h;
    self->pm_row_bytes = BIOS_STRIDE;
    self->pm_origin_x  = 0;
    self->pm_origin_y  = 0;

    self->pm_flags  = BR_PMF_ROW_WHOLEPIXELS | BR_PMF_LINEAR;
    self->pm_base_x = 0;
    self->pm_base_y = 0;

    self->output_facility = facility;
    self->clut            = DeviceVGAClut(dev);

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

    __dpmi_free_physical_address_mapping(&self->meminfo);

    /*
     * Free up pixelmap structure
     */
    BrResFreeNoCallback(self);
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_mcga, device)(br_object *self)
{
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

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, synchronise)(br_device_pixelmap *self, br_token sync_type, br_boolean block)
{
    if(sync_type != BRT_VERTICAL_BLANK || !block)
        return BRE_UNSUPPORTED;

    /* http://www.techhelpmanual.com/899-enhanced_graphics_adapter_i_o_ports.html */

    /*
     * Finish our current vblank.
     */
    while((inportb(0x03DA) & 0x08) == 0x08)
        ;

    /*
     * Wait for the next to start.
     */
    while((inportb(0x03DA) & 0x08) == 0x00)
        ;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vga, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    br_error r;

    if((r = BR_CMETHOD_REF(br_device_pixelmap_vga, synchronise)(self, BRT_VERTICAL_BLANK, BR_TRUE)) != BRE_OK)
        return r;

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

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_mem, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_mem, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_mem, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_mem, allocateSub),

    ._copyTo       = BR_CMETHOD_REF(br_device_pixelmap_mem, copyTo),
    ._copyTo       = BR_CMETHOD_REF(br_device_pixelmap_mem, copyTo),
    ._copyFrom     = BR_CMETHOD_REF(br_device_pixelmap_mem, copyFrom),
    ._fill         = BR_CMETHOD_REF(br_device_pixelmap_mem, fill),
    ._doubleBuffer = BR_CMETHOD_REF(br_device_pixelmap_vga, doubleBuffer),

    ._copyDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, copyDirty),
    ._copyToDirty       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyToDirty),
    ._copyFromDirty     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFromDirty),
    ._fillDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, fillDirty),
    ._doubleBufferDirty = BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBufferDirty),

    ._rectangle                = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle),
    ._rectangle2               = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle2),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleCopyTo),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleCopyFrom),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_mem, rectangleStretchCopyTo),
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

    ._flush        = BR_CMETHOD_REF(br_device_pixelmap_gen, flush),
    ._synchronise  = BR_CMETHOD_REF(br_device_pixelmap_vga, synchronise),
    ._directLock   = BR_CMETHOD_REF(br_device_pixelmap_gen, directLock),
    ._directUnlock = BR_CMETHOD_REF(br_device_pixelmap_gen, directUnlock),

    ._getControls = BR_CMETHOD_REF(br_device_pixelmap_gen, getControls),
    ._setControls = BR_CMETHOD_REF(br_device_pixelmap_gen, setControls),
};
