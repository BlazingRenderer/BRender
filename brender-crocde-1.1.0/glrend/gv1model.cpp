/*
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
#define F(f) offsetof(struct br_geometry_v1_model_gl, f)

static const struct br_tv_template_entry templateEntries[] = {
	{BRT_IDENTIFIER_CSTR,		F(identifier),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_FACILITY_O,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_FACILITY_O,			F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY,},
};
#undef F


static const char *BR_CMETHOD_DECL(br_geometry_v1_model_gl, identifier)(br_object *_self)
{
	return reinterpret_cast<br_geometry_v1_model_gl*>(_self)->identifier;
}

static br_device *BR_CMETHOD_DECL(br_geometry_v1_model_gl, device)(br_object *_self)
{
	br_geometry_v1_model_gl *self = reinterpret_cast<br_geometry_v1_model_gl*>(_self);
	return reinterpret_cast<br_device*>(self->device);
}

static void BR_CMETHOD_DECL(br_geometry_v1_model_gl, free)(br_object *_self)
{
	br_geometry_v1_model_gl *self = reinterpret_cast<br_geometry_v1_model_gl*>(_self);
	ObjectContainerRemove(self->renderer_facility, self);
	BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_geometry_v1_model_gl, type)(br_object *self)
{
	return BRT_GEOMETRY_V1_MODEL;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_v1_model_gl, isType)(br_object *self, br_token t)
{
	return (t == BRT_GEOMETRY_V1_MODEL) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_geometry_v1_model_gl, space)(br_object *self)
{
	return sizeof(br_geometry_v1_model);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_geometry_v1_model_gl, templateQuery)(br_object *_self)
{
	br_geometry_v1_model_gl *self = reinterpret_cast<br_geometry_v1_model_gl*>(_self);

	if(self->device->templates.geometryV1Model == nullptr)
		self->device->templates.geometryV1Model = BrTVTemplateAllocate(self->device, templateEntries, BR_ASIZE(templateEntries));

	return self->device->templates.geometryV1Model;
}

static br_error BR_CMETHOD_DECL(br_geometry_v1_model_gl, storedAvail)(
	struct br_geometry_v1_model *self,
	br_int_32 *psize,
	br_token_value *tv)
{
	log_trace("GL1MDL", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_geometry_v1_model_gl, storedNew)(struct br_geometry_v1_model *_self, struct br_renderer *_renderer, struct br_geometry_stored **psg, struct v11model_f *model, br_token type, br_token_value *tv)
{
	br_geometry_v1_model_gl *self = reinterpret_cast<br_geometry_v1_model_gl*>(_self);
	br_renderer_gl *renderer = reinterpret_cast<br_renderer_gl*>(_renderer);

	ASSERT(type == BRT_TRIANGLE);
	if((*psg = (br_geometry_stored*)GeometryStoredGLAllocate(self, "MODEL", model)) == nullptr)
		return BRE_FAIL;

	return BRE_OK;
}

static_assert(sizeof(glm::mat4x3) == sizeof(br_matrix34_f), "sizeof(glm::mat4x3) != sizeof(br_matrix34_f)");
static_assert(sizeof(glm::mat4) == sizeof(br_matrix4), "sizeof(glm::mat4) != sizeof(br_matrix4)");


br_error BR_CMETHOD_DECL(br_geometry_v1_model_gl, render)(struct br_geometry_v1_model *_self, struct br_renderer *_renderer, struct v11model_f *_model, struct br_renderer_state_stored *default_state, br_token type)
{
	BR_FAILURE("Immediate rendering is not supported");
	return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_geometry_v1_model_gl, renderOnScreen)(struct br_geometry_v1_model *self, struct br_renderer *renderer, struct v11model_f *model, struct br_renderer_state_stored *default_state, br_token type)
{
	BR_FAILURE("Immediate rendering is not supported");
	return BRE_FAIL;
}



/*
* Default dispatch table for renderer type
*/
static const struct br_geometry_v1_model_dispatch geometryV1ModelDispatch = {
	.__reserved0		= nullptr,
	.__reserved1		= nullptr,
	.__reserved2		= nullptr,
	.__reserved3		= nullptr,
	._free				= BR_CMETHOD(br_geometry_v1_model_gl,	free),
	._identifier		= BR_CMETHOD(br_geometry_v1_model_gl,	identifier),
	._type				= BR_CMETHOD(br_geometry_v1_model_gl,	type),
	._isType			= BR_CMETHOD(br_geometry_v1_model_gl,	isType),
	._device			= BR_CMETHOD(br_geometry_v1_model_gl,	device),
	._space				= BR_CMETHOD(br_geometry_v1_model_gl,	space),

	._templateQuery		= BR_CMETHOD(br_geometry_v1_model_gl,	templateQuery),
	._query				= BR_CMETHOD(br_object,					query),
	._queryBuffer		= BR_CMETHOD(br_object,					queryBuffer),
	._queryMany			= BR_CMETHOD(br_object,					queryMany),
	._queryManySize		= BR_CMETHOD(br_object,					queryManySize),
	._queryAll			= BR_CMETHOD(br_object,					queryAll),
	._queryAllSize		= BR_CMETHOD(br_object,					queryAllSize),

	._render			= BR_CMETHOD(br_geometry_v1_model_gl,	render),
	._renderOnScreen	= BR_CMETHOD(br_geometry_v1_model_gl,	renderOnScreen),
	._storedNew			= BR_CMETHOD(br_geometry_v1_model_gl,	storedNew),

	._storedAvail		= BR_CMETHOD(br_geometry_v1_model_gl,	storedAvail),
};


/*
* Allocate a geometry format
*/
br_geometry_v1_model_gl *GeometryV1ModelGLAllocate(br_renderer_facility_gl *type, const char *id)
{
	br_geometry_v1_model_gl *self = BrResAllocate<br_geometry_v1_model_gl>(type->object_list, BR_MEMORY_OBJECT);

	self->dispatch = &geometryV1ModelDispatch;
	self->identifier = id;
	self->device = type->output_facility->device;
	self->renderer_facility = type;

	ObjectContainerAddFront(type, self);
	return self;
}
