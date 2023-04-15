/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: gprim.c 1.1 1997/12/10 16:51:59 jon Exp $
 * $Locker: $
 *
 * Geometry format for primitives
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

BR_RCS_ID("$Id: gprim.c 1.1 1997/12/10 16:51:59 jon Exp $");

/*
 * Default dispatch table for renderer type (defined at and of file)
 */								 
static const struct br_geometry_primitives_dispatch geometryPrimitivesDispatch;

/*
 * Geometry format info. template
 */
#define F(f)	offsetof(struct br_geometry_primitives, f)

static const struct br_tv_template_entry geometryPrimitivesTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,		0,	F(identifier),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
	{BRT_RENDERER_FACILITY_O,	0,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
	{BRT_FACILITY_O,			0,	F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY, },
};
#undef F

/*
 * Allocate a geometry format
 */
br_geometry_primitives * GeometryPrimitivesAllocate(br_renderer_facility *type, char *id)
{
	br_geometry_primitives * self;

	self = BrResAllocate(type->device, sizeof(*self), BR_MEMORY_OBJECT_DATA);

	if(self == NULL)
		return NULL;

	self->dispatch = (struct br_geometry_primitives_dispatch *)&geometryPrimitivesDispatch;
 	self->identifier = id;
 	self->renderer_facility = type;
    self->device = type->device;

	ObjectContainerAddFront(type, (br_object *)self);
	return self;
}

static void BR_CMETHOD_DECL(br_geometry_primitives_soft, free)(br_object *_self)
{
	br_geometry_primitives *self = (br_geometry_primitives*)_self;
	ObjectContainerRemove(self->renderer_facility, (br_object *)self);

	BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_geometry_primitives_soft, type)(br_object *self)
{
	return BRT_GEOMETRY_PRIMITIVES;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_primitives_soft, isType)(br_object *self, br_token t)
{
	return (t == BRT_GEOMETRY_PRIMITIVES) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_geometry_primitives_soft, space)(br_object *self)
{
	return sizeof(br_geometry_primitives);
}

static struct br_tv_template * BR_CMETHOD_DECL(br_geometry_primitives_soft, templateQuery)(br_object *_self)
{
    br_geometry_primitives *self = (br_geometry_primitives*)_self;

    if(self->device->templates.geometryPrimitivesTemplate == NULL)
       self->device->templates.geometryPrimitivesTemplate = BrTVTemplateAllocate(self->device,
        (br_tv_template_entry *)geometryPrimitivesTemplateEntries,
		BR_ASIZE(geometryPrimitivesTemplateEntries));

    return self->device->templates.geometryPrimitivesTemplate;
}

static br_error BR_CMETHOD_DECL(br_geometry_primitives_soft, storedAvail)(
		struct br_geometry_primitives *self,
		br_int_32 *psize,
		br_token_value *tv)
{
	return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_geometry_primitives_soft, render)
		(struct br_geometry_primitives *self, struct br_renderer *renderer, struct fmt_vertex *vertices, int nvertices, br_token type)
{
	return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_geometry_primitives_soft, renderOnScreen)
		(struct br_geometry_primitives *self, struct br_renderer *renderer, struct fmt_vertex *vertices, int nvertices, br_token type)
{
	return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_geometry_primitives_soft, storedNew)
		(struct br_geometry_primitives *self, struct br_renderer *renderer,
		struct br_geometry_stored **psg, struct fmt_vertex *vertices, int nvertices, br_token type, br_token_value *tv)
{
	return BRE_FAIL;
}

/*
 * Default dispatch table for renderer type
 */
static const struct br_geometry_primitives_dispatch geometryPrimitivesDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_geometry_primitives_soft, free),
    ._identifier = BR_CMETHOD_REF(br_object_soft, identifier),
    ._type       = BR_CMETHOD_REF(br_geometry_primitives_soft, type),
    ._isType     = BR_CMETHOD_REF(br_geometry_primitives_soft, isType),
    ._device     = BR_CMETHOD_REF(br_object_soft, device),
    ._space      = BR_CMETHOD_REF(br_geometry_primitives_soft, space),

    ._templateQuery = BR_CMETHOD_REF(br_geometry_primitives_soft, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._render         = BR_CMETHOD_REF(br_geometry_primitives_soft, render),
    ._renderOnScreen = BR_CMETHOD_REF(br_geometry_primitives_soft, renderOnScreen),
    ._storedNew      = BR_CMETHOD_REF(br_geometry_primitives_soft, storedNew),

    ._storedAvail = BR_CMETHOD_REF(br_geometry_primitives_soft, storedAvail),
};

