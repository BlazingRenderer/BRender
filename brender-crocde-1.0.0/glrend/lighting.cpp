#include <stddef.h>
#include <string.h>

#include "drv_ip.hpp"

/*
* Geometry format info. template
*/
#define F(f)    offsetof(struct br_geometry_lighting_gl, f)

static const struct br_tv_template_entry geometryLightingTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	0,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_FACILITY_O,0,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_FACILITY_O,			0,	F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY,},
};
#undef F

static const char * br_geometry_lighting_gl_identifier(br_object *_self)
{
	br_geometry_lighting_gl *self = reinterpret_cast<br_geometry_lighting_gl*>(_self);
	return self->identifier;
}

static br_device * br_geometry_lighting_gl_device(br_object *_self)
{
	br_geometry_lighting_gl *self = reinterpret_cast<br_geometry_lighting_gl*>(_self);
	return reinterpret_cast<br_device*>(self->device);
}

static void br_geometry_lighting_gl_free(br_object *_self)
{
	br_geometry_lighting_gl *self = reinterpret_cast<br_geometry_lighting_gl*>(_self);
	ObjectContainerRemove(self->renderer_facility, (br_object *)self);

	BrResFreeNoCallback(self);
}

static br_token br_geometry_lighting_gl_type(br_object *self)
{
	return BRT_GEOMETRY_LIGHTING;
}

static br_boolean br_geometry_lighting_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_GEOMETRY_LIGHTING) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_int_32 br_geometry_lighting_gl_space(br_object *self)
{
	return sizeof(br_geometry_lighting);
}

static struct br_tv_template * br_geometry_lighting_gl_templateQuery(br_object *_self)
{
	br_geometry_lighting_gl *self = reinterpret_cast<br_geometry_lighting_gl*>(_self);
	return self->device->templates.geometryLightingTemplate;
}

br_error br_geometry_lighting_gl_render
(struct br_geometry_lighting *self, struct br_renderer *renderer,
 br_vector3_f *points, br_vector3_f *normals,
 br_colour *colour_in, br_colour *colour_out,
 br_uint_16 *redirect, int pstride, int nstride,
 int cinstride, int coutstride, int nvertices)
{
	return BRE_FAIL;
}

br_error br_geometry_lighting_gl_renderFixedToFloat
(struct br_geometry_lighting *self, struct br_renderer *renderer,
 br_vector3_x *points, br_vector3_x *normals,
 br_colour *colour_in, br_colour *colour_out,
 br_uint_16 *redirect, int pstride, int nstride,
 int cinstride, int coutstride, int nvertices)
{
	return BRE_FAIL;
}

/*
* Default dispatch table for renderer type
*/
static const struct br_geometry_lighting_dispatch geometryLightingDispatch = {
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	br_geometry_lighting_gl_free,
	br_geometry_lighting_gl_identifier,
	br_geometry_lighting_gl_type,
	br_geometry_lighting_gl_isType,
	br_geometry_lighting_gl_device,
	br_geometry_lighting_gl_space,

	br_geometry_lighting_gl_templateQuery,
	BR_CMETHOD(br_object,					query),
	BR_CMETHOD(br_object,					queryBuffer),
	BR_CMETHOD(br_object,					queryMany),
	BR_CMETHOD(br_object,					queryManySize),
	BR_CMETHOD(br_object,					queryAll),
	BR_CMETHOD(br_object,					queryAllSize),

	br_geometry_lighting_gl_render,
	br_geometry_lighting_gl_renderFixedToFloat,
};


/*
* Allocate a geometry format
*/
br_geometry_lighting_gl *GeometryLightingGLAllocate(br_renderer_facility_gl *type, const char *id)
{
	br_geometry_lighting_gl *self;

	self = (br_geometry_lighting_gl*)BrResAllocate(type->object_list, sizeof(*self), BR_MEMORY_OBJECT);

	if(self == NULL)
		return NULL;

	self->dispatch = (struct br_geometry_lighting_dispatch *)&geometryLightingDispatch;
	self->identifier = id;
	self->device = type->output_facility->device;
	self->renderer_facility = type;

	if(self->device->templates.geometryLightingTemplate == nullptr)
		self->device->templates.geometryLightingTemplate = BrTVTemplateAllocate(self->device, geometryLightingTemplateEntries, BR_ASIZE(geometryLightingTemplateEntries));

	ObjectContainerAddFront(type, (br_object *)self);
	return self;
}
