/*
* Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
*
* $Id: gprim.c 1.1 1997/07/11 16:14:07 jon Exp JON $
* $Locker: JON $
*
* Geometry format for primitives
*/
#include <stddef.h>
#include <string.h>
#include <log.h>

#include "drv_ip.hpp"

/*
* Geometry format info. template
*/
#define F(f)	offsetof(struct br_geometry_primitives_gl, f)

static const struct br_tv_template_entry geometryPrimitivesTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,		0,	F(identifier),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_FACILITY_O,	0,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_FACILITY_O,			0,	F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY,},
};
#undef F

static void br_geometry_primitives_gl_free(br_object *_self)
{
	br_geometry_primitives_gl *self = reinterpret_cast<br_geometry_primitives_gl*>(_self);
	ObjectContainerRemove(self->renderer_facility, (br_object *)self);

	BrResFreeNoCallback(self);
}

static const char * br_geometry_primitives_gl_identifier(br_object *_self)
{
	br_geometry_primitives_gl *self = reinterpret_cast<br_geometry_primitives_gl*>(_self);
	return self->identifier;
}

static br_device * br_geometry_primitives_gl_device(br_object *_self)
{
	br_geometry_primitives_gl *self = reinterpret_cast<br_geometry_primitives_gl*>(_self);
	return reinterpret_cast<br_device*>(self->device);
}


static br_token br_geometry_primitives_gl_type(br_object *self)
{
	return BRT_GEOMETRY_PRIMITIVES;
}

static br_boolean br_geometry_primitives_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_GEOMETRY_PRIMITIVES) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_int_32 br_geometry_primitives_gl_space(br_object *self)
{
	return sizeof(br_geometry_primitives);
}

static struct br_tv_template * br_geometry_primitives_gl_templateQuery(br_object *_self)
{
	br_geometry_primitives_gl *self = reinterpret_cast<br_geometry_primitives_gl*>(_self);
	return self->device->templates.geometryPrimitivesTemplate;
}

static br_error br_geometry_primitives_gl_storedAvail(
	struct br_geometry_primitives *self,
	br_int_32 *psize,
	br_token_value *tv)
{
	log_trace("GLPRIM", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_geometry_primitives_gl_render
(struct br_geometry_primitives *self, struct br_renderer *renderer, struct fmt_vertex *vertices, int nvertices, br_token type)
{
	log_trace("GLPRIM", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_geometry_primitives_gl_renderOnScreen
(struct br_geometry_primitives *self, struct br_renderer *renderer, struct fmt_vertex *vertices, int nvertices, br_token type)
{
	log_trace("GLPRIM", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_geometry_primitives_gl_storedNew
(struct br_geometry_primitives *self, struct br_renderer *renderer,
 struct br_geometry_stored **psg, struct fmt_vertex *vertices, int nvertices, br_token type, br_token_value *tv)
{
	log_trace("GLPRIM", __FUNCTION__);
	return BRE_FAIL;
}
br_error br_geometry_primitives_gl_renderFixedToFloat
(struct br_geometry_primitives *self, struct br_renderer *renderer, struct fmt_vertex *vertices, int nvertices, br_token type)
{
	log_trace("GLPRIM", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_geometry_primitives_gl_renderOnScreenFixedToFloat
(struct br_geometry_primitives *self, struct br_renderer *renderer, struct fmt_vertex *vertices, int nvertices, br_token type)
{
	log_trace("GLPRIM", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_geometry_primitives_gl_storedNewFixedToFloat
(struct br_geometry_primitives *self, struct br_renderer *renderer,
 struct br_geometry_stored **psg, struct fmt_vertex *vertices, int nvertices, br_token type, br_token_value *tv)
{
	log_trace("GLPRIM", __FUNCTION__);
	return BRE_FAIL;
}

/*
* Default dispatch table for renderer type
*/
static const struct br_geometry_primitives_dispatch geometryPrimitivesDispatch = {
	NULL,
	NULL,
	NULL,
	NULL,
	br_geometry_primitives_gl_free,
	br_geometry_primitives_gl_identifier,
	br_geometry_primitives_gl_type,
	br_geometry_primitives_gl_isType,
	br_geometry_primitives_gl_device,
	br_geometry_primitives_gl_space,

	br_geometry_primitives_gl_templateQuery,
	BR_CMETHOD(br_object,					query),
	BR_CMETHOD(br_object,					queryBuffer),
	BR_CMETHOD(br_object,					queryMany),
	BR_CMETHOD(br_object,					queryManySize),
	BR_CMETHOD(br_object,					queryAll),
	BR_CMETHOD(br_object,					queryAllSize),

	br_geometry_primitives_gl_render,
	br_geometry_primitives_gl_renderFixedToFloat,
	br_geometry_primitives_gl_renderOnScreen,
	br_geometry_primitives_gl_renderOnScreenFixedToFloat,
	br_geometry_primitives_gl_storedNew,
	br_geometry_primitives_gl_storedNewFixedToFloat,

	br_geometry_primitives_gl_storedAvail,
};


/*
* Allocate a geometry format
*/
br_geometry_primitives_gl *GeometryPrimitivesGLAllocate(br_renderer_facility_gl *type, const char *id)
{
	br_geometry_primitives_gl * self;

	self = (br_geometry_primitives_gl*)BrResAllocate(type->object_list, sizeof(*self), BR_MEMORY_OBJECT);

	if(self == NULL)
		return NULL;

	self->dispatch = (struct br_geometry_primitives_dispatch *)&geometryPrimitivesDispatch;
	self->identifier = id;
	self->renderer_facility = type;
	self->device = type->output_facility->device;

	self->device->templates.geometryPrimitivesTemplate = BrTVTemplateAllocate(self->device, geometryPrimitivesTemplateEntries, BR_ASIZE(geometryPrimitivesTemplateEntries));

	ObjectContainerAddFront(type, (br_object *)self);
	return self;
}

