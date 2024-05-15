/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: gv1model.c 1.1 1997/12/10 16:52:04 jon Exp $
 * $Locker: $
 *
 * Geometry format for version 1 models
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"


/*
 * Default dispatch table for renderer type (defined at and of file)
 */
static const struct br_geometry_v1_model_dispatch geometryV1ModelDispatch;

/*
 * Geometry format info. template
 */
#define F(f)	offsetof(struct br_geometry_v1_model, f)

static const struct br_tv_template_entry geometryV1ModelTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	0,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
	{BRT_RENDERER_FACILITY_O,0,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
	{BRT_FACILITY_O,			0,	F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY, },
};
#undef F

/*
 * Allocate a geometry format
 */
br_geometry_v1_model * GeometryV1ModelAllocate(br_renderer_facility *type, char *id)
{
	br_geometry_v1_model * self;

	self = BrResAllocate(type->device, sizeof(*self), BR_MEMORY_OBJECT_DATA);

	if(self == NULL)
		return NULL;

	self->dispatch = (struct br_geometry_v1_model_dispatch *)&geometryV1ModelDispatch;
 	self->identifier = id;
    self->device = type->device;
 	self->renderer_facility = type;

	ObjectContainerAddFront(type, (br_object *)self);
	return self;
}

static void BR_CMETHOD_DECL(br_geometry_v1_model_soft, free)(br_object *_self)
{
	br_geometry_v1_model *self = (br_geometry_v1_model*)_self;

	ObjectContainerRemove(self->renderer_facility, (br_object *)self);

	BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_geometry_v1_model_soft, type)(br_object *self)
{
	return BRT_GEOMETRY_V1_MODEL;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_v1_model_soft, isType)(br_object *self, br_token t)
{
	return (t == BRT_GEOMETRY_V1_MODEL) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_geometry_v1_model_soft, space)(br_object *self)
{
	return sizeof(br_geometry_v1_model);
}

static struct br_tv_template * BR_CMETHOD_DECL(br_geometry_v1_model_soft, templateQuery)(br_object *_self)
{
    br_geometry_v1_model *self = (br_geometry_v1_model*)_self;

    if(self->device->templates.geometryV1ModelTemplate == NULL)
        self->device->templates.geometryV1ModelTemplate = BrTVTemplateAllocate(self->device,
            (br_tv_template_entry *)geometryV1ModelTemplateEntries,
			BR_ASIZE(geometryV1ModelTemplateEntries));

    return self->device->templates.geometryV1ModelTemplate;
}

static br_error BR_CMETHOD_DECL(br_geometry_v1_model_soft, storedAvail)(
		struct br_geometry_v1_model *self,
		br_int_32 *psize,
		br_token_value *tv)
{
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_geometry_v1_model_soft, storedNew)
		(struct br_geometry_v1_model *self, struct br_renderer *renderer,
		struct br_geometry_stored **psg, struct v11model_f *model, br_token type, br_token_value *tv)
{
	return BRE_FAIL;
}

/*
 * Default dispatch table for renderer type
 */
static const struct br_geometry_v1_model_dispatch geometryV1ModelDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_geometry_v1_model_soft, free),
    ._identifier = BR_CMETHOD_REF(br_object_soft, identifier),
    ._type       = BR_CMETHOD_REF(br_geometry_v1_model_soft, type),
    ._isType     = BR_CMETHOD_REF(br_geometry_v1_model_soft, isType),
    ._device     = BR_CMETHOD_REF(br_object_soft, device),
    ._space      = BR_CMETHOD_REF(br_geometry_v1_model_soft, space),

    ._templateQuery = BR_CMETHOD_REF(br_geometry_v1_model_soft, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._render         = BR_CMETHOD_REF(br_geometry_v1_model_soft, render),
    ._renderOnScreen = BR_CMETHOD_REF(br_geometry_v1_model_soft, renderOnScreen),
    ._storedNew      = BR_CMETHOD_REF(br_geometry_v1_model_soft, storedNew),

    ._storedAvail    = BR_CMETHOD_REF(br_geometry_v1_model_soft, storedAvail),
};

