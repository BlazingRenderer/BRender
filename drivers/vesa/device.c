/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: device.c 1.1 1997/12/10 16:53:30 jon Exp $
 * $Locker: $
 *
 * Device methods
 */

#include <stddef.h>
#include <string.h>
#include <go32.h>

#include "drv.h"
#include "host.h"
#include "brassert.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static struct br_device_dispatch deviceDispatch;

/*
 * Device info. template, two versions - one for VESA 1.x and one for VESA 2.x or greater
 */
#define F(f) offsetof(struct br_device, f)

static br_tv_template_entry deviceTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),       F(identifier),           BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {DEV(VERSION_U16),           F(vbe_version),          BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {DEV(OEM_STRING_CSTR),       F(vbe_oem_string),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {DEV(OEM_PRODUCT_NAME_CSTR), F(vbe_oem_product_name), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {DEV(OEM_PRODUCT_REV_CSTR),  F(vbe_oem_product_rev),  BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {DEV(OEM_VENDOR_NAME_CSTR),  F(vbe_oem_vendor_name),  BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT(CLUT_O),                F(clut),                 BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
};

#undef F

/*
 * Set up a static device object
 */
br_device *DeviceVESAAllocate(const char *identifier, const char *arguments)
{
    br_device *self;

    self             = BrResAllocate(NULL, sizeof(*self), BR_MEMORY_OBJECT);
    self->identifier = BrResStrDup(self, identifier);
    self->dispatch   = &deviceDispatch;
    self->device     = self;

    /*
     * Find out if VESA driver is around and
     * fetch the VBE information buffer
     */
    self->info = BrResAllocate(NULL, sizeof(*self->info), BR_MEMORY_DRIVER);

    if(VESAInfo(self->info) != BRE_OK) {
        BrLogError("VESA", "Failed to retrieve VBE block.");
        BrResFreeNoCallback(self);
        return NULL;
    }

    if(self->info->vbe_version < 0x200) {
        BrLogError("VESA", "Unsupported VESA VBE version %d.%d, want >= 2.0.", self->vbe_version >> 8, self->vbe_version & 0xF);
        BrResFreeNoCallback(self);
        return NULL;
    }

    self->vbe_version          = self->info->vbe_version;
    self->vbe_oem_string       = (const char *)self->info->oem_string_ptr;
    self->vbe_oem_product_name = (const char *)self->info->oem_product_name_ptr;
    self->vbe_oem_product_rev  = (const char *)self->info->oem_product_rev_ptr;
    self->vbe_oem_vendor_name  = (const char *)self->info->oem_vendor_name_ptr;

    BrLogTrace("VESA", "VBE Version          = %d.%d", self->vbe_version >> 8, self->vbe_version & 0xF);
    BrLogTrace("VESA", "VBE OEM String       = %s", self->vbe_oem_string);
    BrLogTrace("VESA", "VBE OEM Vendor       = %s", self->vbe_oem_vendor_name);
    BrLogTrace("VESA", "VBE OEM Product Name = %s", self->vbe_oem_product_name);
    BrLogTrace("VESA", "VBE OEM Product Rev  = %s", self->vbe_oem_product_rev);

    /*
     * Create Anchor resource
     */
    self->res = BrResAllocate(NULL, 0, BR_MEMORY_DRIVER);

    /*
     * Create all the driver specific tokens
     */
    BrTokenCreate("VESA_MODE_U16", BRT_UINT_16);
    BrTokenCreate("VERSION_U16", BRT_UINT_16);
    BrTokenCreate("OEM_STRING_CSTR", BRT_CONSTANT_STRING);
    BrTokenCreate("OEM_VENDOR_NAME_CSTR", BRT_CONSTANT_STRING);
    BrTokenCreate("OEM_PRODUCT_NAME_CSTR", BRT_CONSTANT_STRING);
    BrTokenCreate("OEM_PRODUCT_REV_CSTR", BRT_CONSTANT_STRING);
    BrTokenCreate("USE_PMI_B", BRT_BOOLEAN);
    BrTokenCreate("USE_LINEAR_B", BRT_BOOLEAN);
    BrTokenCreate("SET_STRIDE_B", BRT_BOOLEAN);

    /*
     * _DEV_ Remember initial VESA mode
     */
    // VESAModeGet(&self->original_mode);
    self->original_mode = BIOSVideoGetMode();

    self->current_mode = self->original_mode;

    /*
     * Set up object list
     */
    self->object_list = BrObjectListAllocate(self->res);

    /*
     * Build CLUT object
     */
    self->clut = DeviceClutVESAAllocate(self, "Hardware-CLUT");

    self->screen_active = BR_FALSE;
    return self;
}

/*
 * Calculate a bunch of per-mode constants for use by the low level
 * loops whilst in linear mode
 */
br_error DeviceVESALinearInitialise(br_device *self, struct vesa_modeinfo *mi)
{
    struct vesa_work *vw = &self->work;
    __dpmi_meminfo    mapping;
    int               selector;

    ASSERT(vw->access_linear);

    if(vw->mapping.address != 0) {
        __dpmi_free_physical_address_mapping(&vw->mapping);
        vw->mapping.address = 0;
    }

    if(vw->selector >= 0) {
        __dpmi_free_ldt_descriptor(vw->selector);
        vw->selector = -1;
    }

    /*
     * Map buffer in if not already available
     */
    mapping = (__dpmi_meminfo){
        .address = mi->phys_base_ptr,
        .size    = self->info->memory * (1 << 16),
    };
    if(__dpmi_physical_address_mapping(&mapping) != 0) {
        return BRE_FAIL;
    }

    if((selector = __dpmi_allocate_ldt_descriptors(1)) < 0) {
        __dpmi_free_physical_address_mapping(&mapping);
        return BRE_FAIL;
    }

    __dpmi_set_segment_base_address(selector, mapping.address);
    __dpmi_set_segment_limit(selector, mapping.size);

    vw->selector       = selector;
    vw->mapping        = mapping;
    vw->stride         = mi->bytes_per_scanline;
    vw->bits_per_pixel = mi->bits_per_pixel;

    return BRE_OK;
}

/*
 * Return true if device can support linear mapping in the given mode
 */
br_boolean DeviceVESALinearAvailable(br_device *self, struct vesa_modeinfo *mi)
{
    host_info hi;

    /*
     * VBE earlien that 2.0 does not support linear buffers
     */
    if(self->info->vbe_version < 0x200)
        return BR_FALSE;

    /*
     * Check mode info for physical buffer
     */
    if(!(mi->mode_attributes & VESA_MA_LINEAR))
        return BR_FALSE;

    if(mi->phys_base_ptr == 0)
        return BR_FALSE;

    /*
     * Can host do linear mapping?
     */
    HostInfo(&hi, sizeof(hi));
    if(!(hi.capabilities & HOST_CAPS_PHYSICAL_MAP))
        return BR_FALSE;

    /*
     * Everything looks good for linear mapping...
     */
    return BR_TRUE;
}

static void BR_CMETHOD_DECL(br_device_vesa, free)(br_object *_self)
{
    br_device *self = (br_device *)_self;

    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    /*
     * Throw away attached resources
     */
    BrResFreeNoCallback(self->res);
    self->res = NULL;
}

static const char *BR_CMETHOD_DECL(br_device_vesa, identifier)(br_object *_self)
{
    br_device *self = (br_device *)_self;
    return self->identifier;
}

static br_token BR_CMETHOD_DECL(br_device_vesa, type)(br_object *self)
{
    return BRT_DEVICE;
}

static br_boolean BR_CMETHOD_DECL(br_device_vesa, isType)(br_object *self, br_token t)
{
    return (t == BRT_DEVICE) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_vesa, device)(br_object *_self)
{
    br_device *self = (br_device *)_self;
    return self->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_vesa, space)(br_object *self)
{
    return sizeof(br_device);
}

static br_tv_template *BR_CMETHOD_DECL(br_device_vesa, queryTemplate)(br_object *_self)
{
    br_device *self = (br_device *)_self;

    if(self->templates.deviceTemplate == NULL)
        self->templates.deviceTemplate = BrTVTemplateAllocate(self, deviceTemplateEntries, BR_ASIZE(deviceTemplateEntries));

    return self->templates.deviceTemplate;
}

static void *BR_CMETHOD_DECL(br_device_vesa, listQuery)(br_object_container *_self)
{
    br_device *self = (br_device *)_self;

    return self->object_list;
}

/*
 * Default dispatch table for device
 */
static struct br_device_dispatch deviceDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_vesa, free),
    ._identifier = BR_CMETHOD_REF(br_device_vesa, identifier),
    ._type       = BR_CMETHOD_REF(br_device_vesa, type),
    ._isType     = BR_CMETHOD_REF(br_device_vesa, isType),
    ._device     = BR_CMETHOD_REF(br_device_vesa, device),
    ._space      = BR_CMETHOD_REF(br_device_vesa, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_vesa, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery            = BR_CMETHOD_REF(br_device_vesa, listQuery),
    ._tokensMatchBegin     = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch          = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd       = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._tokensMatchInfoQuery = BR_CMETHOD_REF(br_object_container, tokensMatchInfoQuery),
    ._addFront             = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront          = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove               = BR_CMETHOD_REF(br_object_container, remove),
    ._find                 = BR_CMETHOD_REF(br_object_container, find),
    ._findMany             = BR_CMETHOD_REF(br_object_container, findMany),
    ._count                = BR_CMETHOD_REF(br_object_container, count),
};
