/*
* Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
*
* $Id: gv1model.c 1.1 1997/07/11 16:14:09 jon Exp JON $
* $Locker: JON $
*
* Geometry format for version 1 models
*/
#include <log.h>
#include <stddef.h>
#include <string.h>
#include <log.h>
#include "drv_ip.hpp"
#include "brassert.h"
#include "formats.h"

/*
* Geometry format info. template
*/
#define F(f)	offsetof(struct br_geometry_v1_model_gl, f)

static const struct br_tv_template_entry geometryV1ModelTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	0,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_FACILITY_O,0,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_FACILITY_O,			0,	F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY,},
};
#undef F


static const char * br_geometry_v1_model_gl_identifier(br_object *_self)
{
	br_geometry_v1_model_gl *self = reinterpret_cast<br_geometry_v1_model_gl*>(_self);
	return self->identifier;
}

static br_device * br_geometry_v1_model_gl_device(br_object *_self)
{
	br_geometry_v1_model_gl *self = reinterpret_cast<br_geometry_v1_model_gl*>(_self);
	return reinterpret_cast<br_device*>(self->device);
}

static void br_geometry_v1_model_gl_free(br_object *_self)
{
	br_geometry_v1_model_gl *self = reinterpret_cast<br_geometry_v1_model_gl*>(_self);
	ObjectContainerRemove(self->renderer_facility, (br_object *)self);

	BrResFreeNoCallback(self);
}

static br_token br_geometry_v1_model_gl_type(br_object *self)
{
	return BRT_GEOMETRY_V1_MODEL;
}

static br_boolean br_geometry_v1_model_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_GEOMETRY_V1_MODEL) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_int_32 br_geometry_v1_model_gl_space(br_object *self)
{
	return sizeof(br_geometry_v1_model);
}

static struct br_tv_template * br_geometry_v1_model_gl_templateQuery(br_object *_self)
{
	br_geometry_v1_model_gl *self = reinterpret_cast<br_geometry_v1_model_gl*>(_self);
	return self->device->templates.geometryV1ModelTemplate;
}

static br_error br_geometry_v1_model_gl_storedAvail(
	struct br_geometry_v1_model *self,
	br_int_32 *psize,
	br_token_value *tv)
{
	log_trace("GL1MDL", __FUNCTION__);
	return BRE_FAIL;
}

static br_error br_geometry_v1_model_gl_storedNewFixedToFloat
(struct br_geometry_v1_model *self, struct br_renderer *renderer,
 struct br_geometry_stored **psg, struct v1model_x *model, br_token type, br_token_value *tv)
{
	log_trace("GL1MDL", __FUNCTION__);
	return BRE_FAIL;
}

static br_error br_geometry_v1_model_gl_storedNew(struct br_geometry_v1_model *_self, struct br_renderer *_renderer, struct br_geometry_stored **psg, struct v1model_f *_model, br_token type, br_token_value *tv)
{
	br_geometry_v1_model_gl *self = reinterpret_cast<br_geometry_v1_model_gl*>(_self);
	br_renderer_gl *renderer = reinterpret_cast<br_renderer_gl*>(_renderer);
	struct v11model_f *model = reinterpret_cast<struct v11model_f*>(_model);

	ASSERT(type == BRT_TRIANGLE);
	if((*psg = (br_geometry_stored*)GeometryStoredGLAllocate(self, "MODEL", model)) == nullptr)
		return BRE_FAIL;

	return BRE_OK;
}

static br_error br_geometry_v1_model_gl_renderFixedToFloat(struct br_geometry_v1_model *self, struct br_renderer *renderer, struct v1model_x *model, br_renderer_state_stored *default_state, br_token type)
{
	log_trace("GL1MDL", __FUNCTION__);
	return BRE_FAIL;
}

static br_error br_geometry_v1_model_gl_renderOnScreenFixedToFloat(struct br_geometry_v1_model *self, struct br_renderer *renderer, struct v1model_x *model, br_renderer_state_stored *default_state, br_token type)
{
	log_trace("GL1MDL", __FUNCTION__);
	return BRE_FAIL;
}

static_assert(sizeof(glm::mat4x3) == sizeof(br_matrix34_f), "sizeof(glm::mat4x3) != sizeof(br_matrix34_f)");
static_assert(sizeof(glm::mat4) == sizeof(br_matrix4), "sizeof(glm::mat4) != sizeof(br_matrix4)");


br_error br_geometry_v1_model_gl_render(struct br_geometry_v1_model *_self, struct br_renderer *_renderer, struct v1model_f *_model, struct br_renderer_state_stored *default_state, br_token type)
{
	BR_FAILURE("Immediate rendering is not supported");
	return BRE_FAIL;
}

br_error br_geometry_v1_model_gl_renderOnScreen(struct br_geometry_v1_model *self, struct br_renderer *renderer, struct v1model_f *model, struct br_renderer_state_stored *default_state, br_token type)
{
	BR_FAILURE("Immediate rendering is not supported");
	return BRE_FAIL;
}



/*
* Default dispatch table for renderer type
*/
static const struct br_geometry_v1_model_dispatch geometryV1ModelDispatch = {
	NULL,
	NULL,
	NULL,
	NULL,
	br_geometry_v1_model_gl_free,
	br_geometry_v1_model_gl_identifier,
	br_geometry_v1_model_gl_type,
	br_geometry_v1_model_gl_isType,
	br_geometry_v1_model_gl_device,
	br_geometry_v1_model_gl_space,

	br_geometry_v1_model_gl_templateQuery,
	BR_CMETHOD(br_object,					query),
	BR_CMETHOD(br_object,					queryBuffer),
	BR_CMETHOD(br_object,					queryMany),
	BR_CMETHOD(br_object,					queryManySize),
	BR_CMETHOD(br_object,					queryAll),
	BR_CMETHOD(br_object,					queryAllSize),

	br_geometry_v1_model_gl_render,
	br_geometry_v1_model_gl_renderFixedToFloat,
	br_geometry_v1_model_gl_renderOnScreen,
	br_geometry_v1_model_gl_renderOnScreenFixedToFloat,
	br_geometry_v1_model_gl_storedNew,
	br_geometry_v1_model_gl_storedNewFixedToFloat,

	br_geometry_v1_model_gl_storedAvail,
};


/*
* Allocate a geometry format
*/
br_geometry_v1_model_gl * GeometryV1ModelGLAllocate(br_renderer_facility_gl *type, const char *id)
{
	br_geometry_v1_model_gl *self;

	self = (br_geometry_v1_model_gl*)BrResAllocate(type->object_list, sizeof(*self), BR_MEMORY_OBJECT);

	if(self == NULL)
		return NULL;

	self->dispatch = (struct br_geometry_v1_model_dispatch *)&geometryV1ModelDispatch;
	self->identifier = id;
	self->device = type->output_facility->device;
	self->renderer_facility = type;

	self->device->templates.geometryV1ModelTemplate = BrTVTemplateAllocate(self->device, geometryV1ModelTemplateEntries, BR_ASIZE(geometryV1ModelTemplateEntries));

	ObjectContainerAddFront(type, (br_object *)self);
	return self;
}
