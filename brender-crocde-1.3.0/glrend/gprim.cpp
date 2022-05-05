/*
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

static const struct br_tv_template_entry templateEntries[] = {
	{BRT_IDENTIFIER_CSTR,		F(identifier),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_FACILITY_O,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_FACILITY_O,			F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY,},
};
#undef F

static void BR_CMETHOD_DECL(br_geometry_primitives_gl, free)(br_object *_self)
{
	br_geometry_primitives_gl *self = reinterpret_cast<br_geometry_primitives_gl*>(_self);
	ObjectContainerRemove(self->renderer_facility, self);
	BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_geometry_primitives_gl, identifier)(br_object *_self)
{
	return reinterpret_cast<br_geometry_primitives_gl*>(_self)->identifier;
}

static br_device *BR_CMETHOD_DECL(br_geometry_primitives_gl, device)(br_object *_self)
{
	br_geometry_primitives_gl *self = reinterpret_cast<br_geometry_primitives_gl*>(_self);
	return reinterpret_cast<br_device*>(self->device);
}


static br_token BR_CMETHOD_DECL(br_geometry_primitives_gl, type)(br_object *self)
{
	return BRT_GEOMETRY_PRIMITIVES;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_primitives_gl, isType)(br_object *self, br_token t)
{
	return (t == BRT_GEOMETRY_PRIMITIVES) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_geometry_primitives_gl, space)(br_object *self)
{
	return sizeof(br_geometry_primitives);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_geometry_primitives_gl, templateQuery)(br_object *_self)
{
	br_geometry_primitives_gl *self = reinterpret_cast<br_geometry_primitives_gl*>(_self);

	if(self->device->templates.geometryPrimitives == nullptr)
		self->device->templates.geometryPrimitives = BrTVTemplateAllocate(self->device, templateEntries, BR_ASIZE(templateEntries));

	return self->device->templates.geometryPrimitives;
}

static br_error BR_CMETHOD_DECL(br_geometry_primitives_gl, storedAvail)(
	struct br_geometry_primitives *self, br_int_32 *psize, br_token_value *tv)
{
	log_trace("GLPRIM", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_geometry_primitives_gl, render)
(struct br_geometry_primitives *self, struct br_renderer *renderer, struct fmt_vertex *vertices, int nvertices, br_token type)
{
	log_trace("GLPRIM", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_geometry_primitives_gl, renderOnScreen)
(struct br_geometry_primitives *self, struct br_renderer *renderer, struct fmt_vertex *vertices, int nvertices, br_token type)
{
	log_trace("GLPRIM", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_geometry_primitives_gl, storedNew)
(struct br_geometry_primitives *self, struct br_renderer *renderer,
 struct br_geometry_stored **psg, struct fmt_vertex *vertices, int nvertices, br_token type, br_token_value *tv)
{
	log_trace("GLPRIM", "%s", __FUNCTION__);
	return BRE_FAIL;
}

/*
* Default dispatch table for renderer type
*/
static const struct br_geometry_primitives_dispatch geometryPrimitivesDispatch = {
	.__reserved0		= nullptr,
	.__reserved1		= nullptr,
	.__reserved2		= nullptr,
	.__reserved3		= nullptr,
	._free				= BR_CMETHOD(br_geometry_primitives_gl,	free),
	._identifier		= BR_CMETHOD(br_geometry_primitives_gl,	identifier),
	._type				= BR_CMETHOD(br_geometry_primitives_gl,	type),
	._isType			= BR_CMETHOD(br_geometry_primitives_gl,	isType),
	._device			= BR_CMETHOD(br_geometry_primitives_gl,	device),
	._space				= BR_CMETHOD(br_geometry_primitives_gl,	space),

	._templateQuery		= BR_CMETHOD(br_geometry_primitives_gl,	templateQuery),
	._query				= BR_CMETHOD(br_object,					query),
	._queryBuffer		= BR_CMETHOD(br_object,					queryBuffer),
	._queryMany			= BR_CMETHOD(br_object,					queryMany),
	._queryManySize		= BR_CMETHOD(br_object,					queryManySize),
	._queryAll			= BR_CMETHOD(br_object,					queryAll),
	._queryAllSize		= BR_CMETHOD(br_object,					queryAllSize),

	._render			= BR_CMETHOD(br_geometry_primitives_gl,	render),
	._renderOnScreen	= BR_CMETHOD(br_geometry_primitives_gl,	renderOnScreen),
	._storedNew			= BR_CMETHOD(br_geometry_primitives_gl,	storedNew),

	._storedAvail		= BR_CMETHOD(br_geometry_primitives_gl,	storedAvail),
};


/*
* Allocate a geometry format
*/
br_geometry_primitives_gl *GeometryPrimitivesGLAllocate(br_renderer_facility_gl *type, const char *id)
{
	br_geometry_primitives_gl *self = BrResAllocate<br_geometry_primitives_gl>(type->object_list, BR_MEMORY_OBJECT);

	self->dispatch = &geometryPrimitivesDispatch;
	self->identifier = id;
	self->renderer_facility = type;
	self->device = type->output_facility->device;

	ObjectContainerAddFront(type, self);
	return self;
}

