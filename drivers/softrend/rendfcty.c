/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: rendfcty.c 1.1 1997/12/10 16:52:31 jon Exp $
 * $Locker: $
 *
 * Renderer type methods
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Default dispatch table for renderer type (defined at and of file)
 */
static const struct br_renderer_facility_dispatch rendererFacilityDispatch;

/*
 * Renderer Type info. template
 */
#define F(f) offsetof(struct br_renderer_facility, f)

static br_tv_template_entry rendererFacilityTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),  0,                                  F(identifier),                    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT(RENDERER_MAX_I32), 0,                                  0,                                BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 1},
    {BRT(PARTS_TL),         (br_uintptr_t)&RendererPartsTokens, BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_LIST,        0,                0},
};
#undef F

/*
 * Set up a static renderer type
 */
br_renderer_facility *RendererFacilitySoftAllocate(br_device *dev, char *identifier)
{
    br_renderer_facility *self;

    self = BrResAllocate(NULL, sizeof(*self), BR_MEMORY_OBJECT_DATA);

    self->dispatch      = (struct br_renderer_facility_dispatch *)&rendererFacilityDispatch;
    self->identifier    = identifier;
    self->device        = dev;
    self->num_instances = 0;
    self->object_list   = BrObjectListAllocate(dev);

    /*
     * Create default state
     */
    StateInitialise(&self->default_state);

    /*
     * Create geometry objects
     */
    GeometryV1BucketsAllocate(self, "V1Buckets");
    GeometryPrimitivesAllocate(self, "Primitives");
    GeometryLightingAllocate(self, "Lighting");
    GeometryV1ModelAllocate(self, "V1Model");

    ObjectContainerAddFront(dev, (br_object *)self);

    return self;
}

static void BR_CMETHOD_DECL(br_renderer_facility_soft, free)(br_object *_self)
{
    br_renderer_facility *self = (br_renderer_facility *)_self;

    /*
     * Detach rendererer from device
     */
    ObjectContainerRemove(ObjectDevice(self), (br_object *)self);

    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_renderer_facility_soft, type)(br_object *self)
{
    return BRT_RENDERER_FACILITY;
}

static br_boolean BR_CMETHOD_DECL(br_renderer_facility_soft, isType)(br_object *self, br_token t)
{
    return (t == BRT_RENDERER_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_renderer_facility_soft, space)(br_object *self)
{
    return sizeof(br_renderer_facility);
}

static br_tv_template *BR_CMETHOD_DECL(br_renderer_facility_soft, templateQuery)(br_object *_self)
{
    br_renderer_facility *self = (br_renderer_facility *)_self;

    if(self->device->templates.rendererFacilityTemplate == NULL)
        self->device->templates.rendererFacilityTemplate = BrTVTemplateAllocate(self->device, rendererFacilityTemplateEntries,
                                                                                BR_ASIZE(rendererFacilityTemplateEntries));

    return self->device->templates.rendererFacilityTemplate;
}

static br_error BR_CMETHOD_DECL(br_renderer_facility_soft, validDestination)(br_renderer_facility *self, br_boolean *bp, br_object *h)
{
    return BRE_OK;
}

/*
 * Structure used to unpack the rendererNew arguments
 */
struct newRendererTokens {
    br_object            *dest;
    br_primitive_library *prims;
};

#define F(f) offsetof(struct newRendererTokens, f)

static br_tv_template_entry rendererNewTemplateEntries[] = {
    {BRT(DESTINATION_O),       F(dest),  BRTV_SET, BRTV_CONV_COPY},
    {BRT(OUTPUT_FACILITY_O),   F(dest),  BRTV_SET, BRTV_CONV_COPY},
    {BRT(PRIMITIVE_LIBRARY_O), F(prims), BRTV_SET, BRTV_CONV_COPY},
};
#undef F

/*
 * type for entry point into primtiives module
 */
typedef struct br_primtive_library *BR_EXPORT prim_begin_fn(char *arguments);

/*
 * Create a new renderer
 */
static br_error BR_CMETHOD_DECL(br_renderer_facility_soft, rendererNew)(br_renderer_facility *self, br_renderer **prenderer, br_token_value *tv)
{
    struct newRendererTokens rt = {NULL, NULL};
    br_error                 r;
    br_int_32                count;
    br_renderer             *renderer;

    /*
     * Process any options
     */

    if(self->device->templates.rendererNewTemplate == NULL)
        self->device->templates.rendererNewTemplate = BrTVTemplateAllocate(self->device, rendererNewTemplateEntries,
                                                                           BR_ASIZE(rendererNewTemplateEntries));

    BrTokenValueSetMany(&rt, &count, NULL, tv, self->device->templates.rendererNewTemplate);

    /*
     * If there is already an instance of this type, then we can't create another
     */
    //	if(self->num_instances >= 1)
    //		return BRE_FAIL;

    self->num_instances++;

    /*
     * If no primitive base has been specified then try and find one by other means
     */
    if(rt.prims == NULL) {
        r = BrPrimitiveLibraryFind(&rt.prims, (br_device_pixelmap *)rt.dest, BR_SCALAR_TOKEN);
        if(r != BRE_OK)
            return r;
    }

    /*
     * Fail if we could not hook up with any primitives
     *
     * XX how about allowing a special case (nullprim.dll ?) for
     * instantiating a pipeline with no actual renderers
     */
    if(rt.prims == NULL)
        return BRE_FAIL;

    /*
     * Create Renderer
     */
    renderer = RendererSoftAllocate(ObjectDevice(self), self, rt.prims);

    if(renderer == NULL)
        return BRE_FAIL;

    *prenderer = renderer;
    return BRE_OK;
}

static void *BR_CMETHOD_DECL(br_renderer_facility_soft, listQuery)(br_object_container *_self)
{
    return ((br_renderer_facility *)_self)->object_list;
}

/*
 * Default dispatch table for renderer type (defined at and of file)
 */
static const struct br_renderer_facility_dispatch rendererFacilityDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_renderer_facility_soft, free),
    ._identifier = BR_CMETHOD_REF(br_object_soft, identifier),
    ._type       = BR_CMETHOD_REF(br_renderer_facility_soft, type),
    ._isType     = BR_CMETHOD_REF(br_renderer_facility_soft, isType),
    ._device     = BR_CMETHOD_REF(br_object_soft, device),
    ._space      = BR_CMETHOD_REF(br_renderer_facility_soft, space),

    ._templateQuery = BR_CMETHOD_REF(br_renderer_facility_soft, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery        = BR_CMETHOD_REF(br_renderer_facility_soft, listQuery),
    ._tokensMatchBegin = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch      = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd   = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._addFront         = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront      = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove           = BR_CMETHOD_REF(br_object_container, remove),
    ._find             = BR_CMETHOD_REF(br_object_container, find),
    ._findMany         = BR_CMETHOD_REF(br_object_container, findMany),
    ._count            = BR_CMETHOD_REF(br_object_container, count),

    ._validDestination = BR_CMETHOD_REF(br_renderer_facility_soft, validDestination),

    ._rendererNew = BR_CMETHOD_REF(br_renderer_facility_soft, rendererNew),
};
