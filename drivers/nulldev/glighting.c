/*
 * Geometry format for rendering the order table buckets spat out by a prior hidden surface render
 */
#include <stddef.h>

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
#define F(f) offsetof(struct br_geometry_lighting, f)

static struct br_tv_template_entry geometryLightingTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR,     NULL, F(identifier),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT_RENDERER_FACILITY_O, NULL, F(renderer_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT_FACILITY_O,          NULL, F(renderer_facility), BRTV_QUERY,            BRTV_CONV_COPY},
};
#undef F

/*
 * Allocate a geometry format
 */
br_geometry_lighting *GeometryLightingNullAllocate(br_renderer_facility *type, const char *id)
{
    br_geometry_lighting *self;
    struct br_device     *device = ObjectDevice(type);

    if((self = BrResAllocate(device, sizeof(*self), BR_MEMORY_OBJECT)) == NULL)
        return NULL;

    self->dispatch          = &geometryLightingDispatch;
    self->identifier        = id;
    self->device            = device;
    self->renderer_facility = type;
    self->templates         = BrTVTemplateAllocate(self, geometryLightingTemplateEntries,
                                                   BR_ASIZE(geometryLightingTemplateEntries));

    if(self->templates == NULL) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    ObjectContainerAddFront(type, (struct br_object *)self);
    return self;
}

static void BR_CMETHOD_DECL(br_geometry_lighting_null, free)(struct br_object *self)
{
    ObjectContainerRemove(((br_geometry_lighting *)self)->renderer_facility, (struct br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_geometry_lighting_null, identifier)(struct br_object *self)
{
    return ((br_geometry_lighting *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_geometry_lighting_null, type)(struct br_object *self)
{
    (void)self;
    return BRT_GEOMETRY_LIGHTING;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_lighting_null, isType)(struct br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_GEOMETRY_LIGHTING) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_geometry_lighting_null, device)(struct br_object *self)
{
    return ((br_geometry_lighting *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_geometry_lighting_null, space)(struct br_object *self)
{
    (void)self;
    return sizeof(br_geometry_lighting);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_geometry_lighting_null, templateQuery)(struct br_object *self)
{
    return ((br_geometry_lighting *)self)->templates;
}

/*
 * Lighting function calling facility
 */
br_error BR_CMETHOD_DECL(br_geometry_lighting_null,
                         render)(struct br_geometry_lighting *self, struct br_renderer *renderer, br_vector3_f *points,
                                 br_vector3_f *normals, br_colour *colour_in, br_colour *colour_out, br_uint_16 *redirect,
                                 int pstride, int nstride, int cinstride, int coutstride, int nvertices)
{
    return BRE_FAIL;
}

/*
 * Default dispatch table for renderer type
 */
static const struct br_geometry_lighting_dispatch geometryLightingDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_geometry_lighting_null, free),
    ._identifier = BR_CMETHOD_REF(br_geometry_lighting_null, identifier),
    ._type       = BR_CMETHOD_REF(br_geometry_lighting_null, type),
    ._isType     = BR_CMETHOD_REF(br_geometry_lighting_null, isType),
    ._device     = BR_CMETHOD_REF(br_geometry_lighting_null, device),
    ._space      = BR_CMETHOD_REF(br_geometry_lighting_null, space),

    ._templateQuery = BR_CMETHOD_REF(br_geometry_lighting_null, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._render = BR_CMETHOD_REF(br_geometry_lighting_null, render),
};
