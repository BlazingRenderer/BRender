/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: gv1buckt.c 1.1 1997/12/10 16:52:02 jon Exp $
 * $Locker: $
 *
 * Geometry format for rendering the order table buckets spat out by a prior hidden surface render
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

BR_RCS_ID("$Id: gv1buckt.c 1.1 1997/12/10 16:52:02 jon Exp $");

/*
 * Default dispatch table for renderer type (defined at end of file)
 */								 
static const struct br_geometry_v1_buckets_dispatch geometryV1BucketsDispatch;

/*
 * Geometry format info. template
 */
#define F(f)	offsetof(struct br_geometry_v1_buckets, f)

static const struct br_tv_template_entry geometryV1BucketsTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	0,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
	{BRT_RENDERER_FACILITY_O,0,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
	{BRT_FACILITY_O,			0,	F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY, },
};
#undef F

/*
 * Allocate a geometry format
 */
br_geometry_v1_buckets * GeometryV1BucketsAllocate(br_renderer_facility *type, char *id)
{
	br_geometry_v1_buckets * self;

	self = BrResAllocate(type->device, sizeof(*self), BR_MEMORY_OBJECT_DATA);

	if(self == NULL)
		return NULL;

	self->dispatch = (struct br_geometry_v1_buckets_dispatch *)&geometryV1BucketsDispatch;
 	self->identifier = id;
    self->device = type->device;
 	self->renderer_facility = type;

	ObjectContainerAddFront(type, (br_object *)self);
	return self;
}

static void BR_CMETHOD_DECL(br_geometry_v1_buckets_soft, free)(br_object *_self)
{
	br_geometry_v1_buckets *self = (br_geometry_v1_buckets*)_self;

	ObjectContainerRemove(self->renderer_facility, (br_object *)self);

	BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_geometry_v1_buckets_soft, type)(br_object *self)
{
	return BRT_GEOMETRY_V1_BUCKETS;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_v1_buckets_soft, isType)(br_object *self, br_token t)
{
	return (t == BRT_GEOMETRY_PRIMITIVES) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_geometry_v1_buckets_soft, space)(br_object *self)
{
	return sizeof(br_geometry_v1_buckets);
}

static struct br_tv_template * BR_CMETHOD_DECL(br_geometry_v1_buckets_soft, templateQuery)(br_object *_self)
{
	br_geometry_v1_buckets *self = (br_geometry_v1_buckets*)_self;

    if(self->device->templates.geometryV1BucketsTemplate == NULL)
        self->device->templates.geometryV1BucketsTemplate = BrTVTemplateAllocate(self->device,
            (br_tv_template_entry *)geometryV1BucketsTemplateEntries,
			BR_ASIZE(geometryV1BucketsTemplateEntries));

    return self->device->templates.geometryV1BucketsTemplate;
}

br_error BR_CMETHOD_DECL(br_geometry_v1_buckets_soft, render)
		(struct br_geometry_v1_buckets *self, struct br_renderer *renderer,
			struct br_primitive **buckets, br_int_32 nbuckets)
{
	struct br_primitive *p;
	struct br_renderer_state_stored *last_state;
	br_error r;
	br_token last_type;

	if(nbuckets <= 0)
		return BRE_OK;

	/*
	 * Use 'self' as an object handle that will never
	 * be a stored object
	 */
	last_state = (br_renderer_state_stored *)self;
	last_type = BR_NULL_TOKEN;

	rend.block = NULL;

	/*
	 * Render bucket table from last to first
	 */
	for(buckets += nbuckets-1; nbuckets--; buckets--) {
		for(p = *buckets; p ; p = p->next) {
			/*
			 * Reload state for primitive, if it has changed
			 */
			if(p->stored != last_state || p->type != (br_uint_32)last_type) {

				if(rend.block)
					PrimitiveStateRenderEnd(renderer->state.pstate, rend.block);
	
				if(p->stored)
					RendererStateRestore(renderer, p->stored, (br_uint_32)(BR_STATE_PRIMITIVE | BR_STATE_CACHE));

				/*
				 * Reference the primtive block
				 */
				r = PrimitiveStateRenderBegin(renderer->state.pstate,
					&rend.block, &rend.block_changed, &rend.range_changed, BR_FALSE, p->type);

				last_type = p->type;
				last_state = p->stored;

				if(r != BRE_OK)
					continue;
			}

			/*
			 * Render the primitive
			 */
			rend.block->render(rend.block,p->v[0],p->v[1],p->v[2]);
		}
	}

	if(rend.block)
		PrimitiveStateRenderEnd(renderer->state.pstate, rend.block);
	
	return BRE_OK;
}

/*
 * Default dispatch table for renderer type
 */
static const struct br_geometry_v1_buckets_dispatch geometryV1BucketsDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_geometry_v1_buckets_soft, free),
    ._identifier = BR_CMETHOD_REF(br_object_soft, identifier),
    ._type       = BR_CMETHOD_REF(br_geometry_v1_buckets_soft, type),
    ._isType     = BR_CMETHOD_REF(br_geometry_v1_buckets_soft, isType),
    ._device     = BR_CMETHOD_REF(br_object_soft, device),
    ._space      = BR_CMETHOD_REF(br_geometry_v1_buckets_soft, space),

    ._templateQuery = BR_CMETHOD_REF(br_geometry_v1_buckets_soft, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._render         = BR_CMETHOD_REF(br_geometry_v1_buckets_soft, render),
    ._renderOnScreen = BR_CMETHOD_REF(br_geometry_v1_buckets_soft, render),
};

