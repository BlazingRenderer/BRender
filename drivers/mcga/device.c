/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: device.c 1.1 1997/12/10 16:45:34 jon Exp $
 * $Locker: $
 *
 * Device methods
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"

#define DEVICE_VERSION BR_VERSION(1, 0, 0)

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_dispatch deviceDispatch;

/*
 * Device info. template
 */
static const char deviceTitle[] = "ATL MCGA v1.0";

static const char deviceCreator[] = "Argonaut Technologies Ltd.";
static const char deviceProduct[] = "VGA MCGA";

#define _F(f) offsetof(br_device, f)
#define _A(f) ((br_int_32)(f))
static br_tv_template_entry deviceTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),     _F(identifier),    BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0              },
    {BRT(CLUT_O),              _F(clut),          BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0              },
    {BRT(VERSION_U32),         0,                 BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, DEVICE_VERSION },
    {BRT(BRENDER_VERSION_U32), 0,                 BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, __BRENDER__    },
    {BRT(DDI_VERSION_U32),     0,                 BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, __BRENDER_DDI__},
    {BRT(CREATOR_CSTR),        _A(deviceCreator), BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
    {BRT(TITLE_CSTR),          _A(deviceTitle),   BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
    {BRT(PRODUCT_CSTR),        _A(deviceProduct), BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
};
#undef _A
#undef _F

/*
 * Set up a static device object
 */
br_device *DeviceVGAAllocate(const char *identifier)
{
    br_device *self;

    /*
     * Set up device block and resource anchor
     */
    self      = BrResAllocate(NULL, sizeof(*self), BR_MEMORY_OBJECT_DATA);
    self->res = BrResAllocate(self, 0, BR_MEMORY_DRIVER);

    self->identifier = BrResStrDup(self, identifier);
    self->dispatch   = &deviceDispatch;
    self->device     = self;

    /*
     * _DEV_ Remember initial VGA mode
     */
    self->original_mode = BIOSVideoGetMode();
    self->current_mode  = self->original_mode;

    self->object_list = BrObjectListAllocate(self->res);

    /*
     * Build CLUT object
     */
    self->clut = DeviceClutVGAAllocate(self, "Hardware-CLUT");

    return self;
}

static void BR_CMETHOD_DECL(br_device_vga, free)(br_object *self)
{
    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    /*
     * Throw away attached resources
     */
    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_vga, identifier)(br_object *_self)
{
    br_device *self = (br_device *)_self;
    return self->identifier;
}

static br_device *BR_CMETHOD_DECL(br_device_vga, device)(br_object *_self)
{
    br_device *self = (br_device *)_self;
    return self->device;
}

static br_token BR_CMETHOD_DECL(br_device_vga, type)(br_object *self)
{
    return BRT_DEVICE;
}

static br_boolean BR_CMETHOD_DECL(br_device_vga, isType)(br_object *self, br_token t)
{
    return (t == BRT_DEVICE) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_device_vga, space)(br_object *self)
{
    return sizeof(br_device);
}

static br_tv_template *BR_CMETHOD_DECL(br_device_vga, templateQuery)(br_object *_self)
{
    br_device *self = (br_device *)_self;

    if(self->templates.deviceTemplate == NULL)
        self->templates.deviceTemplate = BrTVTemplateAllocate(self, deviceTemplateEntries, BR_ASIZE(deviceTemplateEntries));

    return self->templates.deviceTemplate;
}

static void *BR_CMETHOD_DECL(br_device_vga, listQuery)(br_object_container *_self)
{
    br_device *self = (br_device *)_self;
    return self->object_list;
}

/*
 * Default dispatch table for device
 */
static const struct br_device_dispatch deviceDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_vga, free),
    ._identifier = BR_CMETHOD_REF(br_device_vga, identifier),
    ._type       = BR_CMETHOD_REF(br_device_vga, type),
    ._isType     = BR_CMETHOD_REF(br_device_vga, isType),
    ._device     = BR_CMETHOD_REF(br_device_vga, device),
    ._space      = BR_CMETHOD_REF(br_device_vga, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_vga, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery            = BR_CMETHOD_REF(br_device_vga, listQuery),
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
