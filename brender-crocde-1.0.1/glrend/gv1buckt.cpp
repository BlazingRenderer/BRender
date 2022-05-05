/*
* Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
*
* $Id: gv1buckt.c 1.2 1997/08/11 14:25:42 jon Exp JON $
* $Locker: JON $
*
* Geometry format for rendering the order table buckets spat out by a prior hidden surface render
*/
#include <stddef.h>
#include <string.h>
#include <log.h>
#include "drv_ip.hpp"

/*
* Geometry format info. template
*/
#define F(f)	offsetof(struct br_geometry_v1_buckets_gl, f)

static const struct br_tv_template_entry geometryV1BucketsTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,		0,	F(identifier),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_FACILITY_O,	0,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_FACILITY_O,			0,	F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY,},
};
#undef F


static void br_geometry_v1_buckets_gl_free(br_object *_self)
{
	br_geometry_v1_buckets_gl *self = reinterpret_cast<br_geometry_v1_buckets_gl*>(_self);

	ObjectContainerRemove(self->renderer_facility, (br_object *)self);

	BrResFreeNoCallback(self);
}

static const char * br_geometry_v1_buckets_gl_identifier(br_object *_self)
{
	br_geometry_v1_buckets_gl *self = reinterpret_cast<br_geometry_v1_buckets_gl*>(_self);
	return self->identifier;
}

static br_device * br_geometry_v1_buckets_gl_device(br_object *_self)
{
	br_geometry_v1_buckets_gl *self = reinterpret_cast<br_geometry_v1_buckets_gl*>(_self);
	return reinterpret_cast<br_device*>(self->device);
}

static br_token br_geometry_v1_buckets_gl_type(br_object *self)
{
	return BRT_GEOMETRY_V1_BUCKETS;
}

static br_boolean br_geometry_v1_buckets_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_GEOMETRY_PRIMITIVES) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_int_32 br_geometry_v1_buckets_gl_space(br_object *self)
{
	return sizeof(br_geometry_v1_buckets);
}

static struct br_tv_template *br_geometry_v1_buckets_gl_templateQuery(br_object *_self)
{
	br_geometry_v1_buckets_gl *self = reinterpret_cast<br_geometry_v1_buckets_gl*>(_self);
	return self->device->templates.geometryV1BucketsTemplate;
}

static void renderByMode(br_renderer_gl *self, render_mode mode, bool depthOverride, bool reverseOrder)
{
	auto proc = [self, mode, depthOverride, reverseOrder](const br_renderer_gl_deferred& r) {
		*self->state.current = r.snapshot;
		StoredGLRenderGroups(r.stored, self, r.flags[static_cast<int>(mode)], depthOverride);
	};

	if(reverseOrder)
		std::for_each(self->geom.crbegin(), self->geom.crend(), proc);
	else
		std::for_each(self->geom.cbegin(), self->geom.cend(), proc);
}

br_error br_geometry_v1_buckets_gl_render(struct br_geometry_v1_buckets *_self, struct br_renderer *_renderer, struct br_primitive **buckets, br_int_32 nbuckets)
{
	br_geometry_v1_buckets_gl *self = reinterpret_cast<br_geometry_v1_buckets_gl*>(_self);
	br_renderer_gl *renderer = reinterpret_cast<br_renderer_gl*>(_renderer);

	renderByMode(renderer, render_mode::force_back, true, true);
	renderByMode(renderer, render_mode::opaque, false, true);
	renderByMode(renderer, render_mode::transparent, false, false);
	renderByMode(renderer, render_mode::force_front, true, false);

	renderer->geom.clear();

	return BRE_OK;
}

/*
* Default dispatch table for renderer type
*/
static const struct br_geometry_v1_buckets_dispatch geometryV1BucketsDispatch = {
	NULL,
	NULL,
	NULL,
	NULL,
	br_geometry_v1_buckets_gl_free,
	br_geometry_v1_buckets_gl_identifier,
	br_geometry_v1_buckets_gl_type,
	br_geometry_v1_buckets_gl_isType,
	br_geometry_v1_buckets_gl_device,
	br_geometry_v1_buckets_gl_space,

	br_geometry_v1_buckets_gl_templateQuery,
	BR_CMETHOD(br_object,					query),
	BR_CMETHOD(br_object,					queryBuffer),
	BR_CMETHOD(br_object,					queryMany),
	BR_CMETHOD(br_object,					queryManySize),
	BR_CMETHOD(br_object,					queryAll),
	BR_CMETHOD(br_object,					queryAllSize),

	br_geometry_v1_buckets_gl_render,
	br_geometry_v1_buckets_gl_render,
};


/*
* Allocate a geometry format
*/
br_geometry_v1_buckets_gl *GeometryV1BucketsGLAllocate(br_renderer_facility_gl *type, const char *id)
{
	br_geometry_v1_buckets_gl *self;

	self = (br_geometry_v1_buckets_gl*)BrResAllocate(type->object_list, sizeof(*self), BR_MEMORY_OBJECT);

	if(self == NULL)
		return NULL;

	self->dispatch = (struct br_geometry_v1_buckets_dispatch *)&geometryV1BucketsDispatch;
	self->identifier = id;
	self->device = type->output_facility->device;
	self->renderer_facility = type;
	self->device->templates.geometryV1BucketsTemplate = BrTVTemplateAllocate(self->device, geometryV1BucketsTemplateEntries, BR_ASIZE(geometryV1BucketsTemplateEntries));

	ObjectContainerAddFront(type, (br_object *)self);
	return self;
}