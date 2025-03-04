/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: lighting.c 1.1 1997/12/10 16:52:12 jon Exp $
 * $Locker: $
 *
 * Geometry format for rendering the order table buckets spat out by a prior hidden surface render
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"


/*
 * Default dispatch table for renderer type (defined at end of file)
 */								 
static const struct br_geometry_lighting_dispatch geometryLightingDispatch;

/*
 * Geometry format info. template
 */
#define F(f)    offsetof(struct br_geometry_lighting, f)

static const struct br_tv_template_entry geometryLightingTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	0,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
	{BRT_RENDERER_FACILITY_O,0,	F(renderer_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
	{BRT_FACILITY_O,			0,	F(renderer_facility),	BRTV_QUERY,				BRTV_CONV_COPY, },
};
#undef F

/*
 * Allocate a geometry format
 */
br_geometry_lighting * GeometryLightingAllocate(br_renderer_facility *type, char *id)
{
	br_geometry_lighting * self;

	self = BrResAllocate(type->device, sizeof(*self), BR_MEMORY_OBJECT_DATA);

	if(self == NULL)
		return NULL;

	self->dispatch = (struct br_geometry_lighting_dispatch *)&geometryLightingDispatch;
 	self->identifier = id;
        self->device = type->device;
 	self->renderer_facility = type;

	ObjectContainerAddFront(type, (br_object *)self);
	return self;
}

static void BR_CMETHOD_DECL(br_geometry_lighting_soft, free)(br_object *_self)
{
	br_geometry_lighting *self = (br_geometry_lighting*)_self;

	ObjectContainerRemove(self->renderer_facility, (br_object *)self);

	BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_geometry_lighting_soft, type)(br_object  *self)
{
        return BRT_GEOMETRY_LIGHTING;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_lighting_soft, isType)(br_object *self, br_token t)
{
        return (t == BRT_GEOMETRY_LIGHTING) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_geometry_lighting_soft, space)(br_object *self)
{
        return sizeof(br_geometry_lighting);
}

static struct br_tv_template * BR_CMETHOD_DECL(br_geometry_lighting_soft, templateQuery)(br_object *_self)
{
    br_geometry_lighting *self = (br_geometry_lighting*)_self;

    if(self->device->templates.geometryLightingTemplate == NULL)
        self->device->templates.geometryLightingTemplate = BrTVTemplateAllocate(self->device,
            (struct br_tv_template_entry *)geometryLightingTemplateEntries,
			BR_ASIZE(geometryLightingTemplateEntries));

    return self->device->templates.geometryLightingTemplate;
}

/*
 * Lighting function calling facility
 */

br_error BR_CMETHOD_DECL(br_geometry_lighting_soft, render)
		(struct br_geometry_lighting *self, struct br_renderer *renderer,
                 br_vector3_f *points, br_vector3_f *normals,
                 br_colour *colour_in, br_colour *colour_out,
                 br_uint_16 *redirect, int pstride, int nstride,
                 int cinstride, int coutstride, int nvertices)
{
        int i, j;
        br_vector2 map = {BR_SCALAR(0.0), BR_SCALAR(0.0)};
        br_scalar comp[NUM_COMPONENTS];
        br_error r;

        if(renderer->state.pstate == NULL) {
            r = PrimitiveLibraryStateNew(renderer->plib, &renderer->state.pstate);
            if(r != BRE_OK)
                return r;
        }

        if(!scache.valid_per_scene){
            StaticCacheUpdate_PerScene(renderer);
            scache.valid_per_scene = BR_TRUE;
        }

        if(!scache.valid_per_model){
            StaticCacheUpdate_PerModel(renderer);
            scache.valid_per_model = BR_TRUE;
        }

		r = PrimitiveStateRenderBegin(renderer->state.pstate,
            &rend.block, &rend.block_changed, &rend.range_changed, 0,
            BRT_TRIANGLE);

        if(r != BRE_OK)
            return r;

        CacheUpdate(renderer);

        renderer->state.cache.comp_offsets[C_I] = BR_SCALAR(0.0);
        renderer->state.cache.comp_offsets[C_R] = BR_SCALAR(0.0);
        renderer->state.cache.comp_offsets[C_G] = BR_SCALAR(0.0);
        renderer->state.cache.comp_offsets[C_B] = BR_SCALAR(0.0);
        renderer->state.cache.comp_scales[C_I] = BR_SCALAR(256);
        renderer->state.cache.comp_scales[C_R] = BR_SCALAR(256);
        renderer->state.cache.comp_scales[C_G] = BR_SCALAR(256);
        renderer->state.cache.comp_scales[C_B] = BR_SCALAR(256);

        if(redirect)
                colour_out = (br_colour *)(((char *)colour_out) + coutstride*(*redirect));

        for(j = 0; j < nvertices; j++){
                for(i = 0; i < renderer->state.cache.nvertex_fns; i++)
                        renderer->state.cache.vertex_fns[i](renderer, (br_vector3*)points,
                                &map, (br_vector3*)normals, *colour_in, comp);
                for(i = 0; i < renderer->state.cache.nconstant_fns; i++)
                    renderer->state.cache.constant_fns[i](renderer, (br_vector3*)points,
                        &map, (br_vector3*)normals, *colour_in, comp);

                *((char *)colour_out) = BrScalarToInt(comp[C_I]);
                *(((char *)colour_out)+1) = BrScalarToInt(comp[C_R]);
                *(((char *)colour_out)+2) = BrScalarToInt(comp[C_G]);
                *(((char *)colour_out)+3) = BrScalarToInt(comp[C_B]);

                points = (br_vector3_f *)((br_uintptr_t)points + pstride);
                normals = (br_vector3_f *)((br_uintptr_t)normals + nstride);
                colour_in = (br_colour *)((char *)colour_in + cinstride);
                if(redirect){
                        colour_out = (br_colour *)(((char *)colour_out) + coutstride*((*(redirect+1))-(*redirect)));
                        redirect++;
                }
                else
                        colour_out = (br_colour *)(((char *)colour_out) + coutstride);
        }

		PrimitiveStateRenderEnd(renderer->state.pstate,rend.block);

        return BRE_OK;
}

/*
 * Default dispatch table for renderer type
 */
static const struct br_geometry_lighting_dispatch geometryLightingDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_geometry_lighting_soft, free),
    ._identifier = BR_CMETHOD_REF(br_object_soft, identifier),
    ._type       = BR_CMETHOD_REF(br_geometry_lighting_soft, type),
    ._isType     = BR_CMETHOD_REF(br_geometry_lighting_soft, isType),
    ._device     = BR_CMETHOD_REF(br_object_soft, device),
    ._space      = BR_CMETHOD_REF(br_geometry_lighting_soft, space),

    ._templateQuery = BR_CMETHOD_REF(br_geometry_lighting_soft, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._render        = BR_CMETHOD_REF(br_geometry_lighting_soft, render),
};

