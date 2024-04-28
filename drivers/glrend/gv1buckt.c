/*
 * Geometry format for rendering the order table buckets spat out by a prior hidden surface render
 */
#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for renderer type (defined at end of file)
 */
static const struct br_geometry_v1_buckets_dispatch geometryV1BucketsDispatch;

/*
 * Geometry format info. template
 */
#define F(f) offsetof(struct br_geometry_v1_buckets, f)

static struct br_tv_template_entry geometryV1BucketsTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),     F(identifier),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(RENDERER_FACILITY_O), F(renderer_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(FACILITY_O),          F(renderer_facility), BRTV_QUERY,            BRTV_CONV_COPY},
};
#undef F

/*
 * Allocate a geometry format
 */
br_geometry_v1_buckets *GeometryV1BucketsGLAllocate(br_renderer_facility *type, const char *id)
{
    br_geometry_v1_buckets *self;

    self                    = BrResAllocate(type, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch          = &geometryV1BucketsDispatch;
    self->identifier        = id;
    self->device            = type->device;
    self->renderer_facility = type;

    ObjectContainerAddFront(type, (br_object *)self);
    return self;
}

static void BR_CMETHOD_DECL(br_geometry_v1_buckets_gl, free)(br_object *_self)
{
    br_geometry_v1_buckets *self = (br_geometry_v1_buckets *)_self;

    ObjectContainerRemove(self->renderer_facility, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD(br_geometry_v1_buckets_gl, identifier)(br_object *self)
{
    return ((br_geometry_v1_buckets *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_geometry_v1_buckets_gl, type)(br_object *self)
{
    return BRT_GEOMETRY_V1_BUCKETS;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_v1_buckets_gl, isType)(br_object *self, br_token t)
{
    return (t == BRT_GEOMETRY_PRIMITIVES) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD(br_geometry_v1_buckets_gl, device)(br_object *self)
{
    return ((br_geometry_v1_buckets *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_geometry_v1_buckets_gl, space)(br_object *self)
{
    return sizeof(br_geometry_v1_buckets);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_geometry_v1_buckets_gl, templateQuery)(br_object *_self)
{
    br_geometry_v1_buckets *self = (br_geometry_v1_buckets *)_self;

    if(self->device->templates.geometryV1BucketsTemplate == NULL) {
        self->device->templates.geometryV1BucketsTemplate = BrTVTemplateAllocate(
            self->device, (br_tv_template_entry *)geometryV1BucketsTemplateEntries, BR_ASIZE(geometryV1BucketsTemplateEntries));
    }

    return self->device->templates.geometryV1BucketsTemplate;
}

br_error BR_CMETHOD_DECL(br_geometry_v1_buckets_gl, render)(br_geometry_v1_buckets *self, br_renderer *renderer,
                                                            br_primitive **buckets, br_int_32 nbuckets)
{
    br_primitive             *p;
    br_renderer_state_stored *stored;

    if(nbuckets <= 0)
        return BRE_OK;

    size_t count = 0;
    /*
     * Render bucket table from last to first
     */
    for(buckets += nbuckets - 1; nbuckets--; buckets--) {
        for(p = *buckets; p; p = p->next) {
            ++count;
            stored = (br_renderer_state_stored *)p->stored;
            ASSERT(stored == NULL || ObjectIsType((br_object *)stored, BRT_RENDERER_STATE_STORED));

            /*
             * See gstored.c
             */
            if(p->type == BRT_GEOMETRY_STORED) {
                br_geometry_stored *geom      = (br_geometry_stored *)p->v[0];
                state_stack        *state     = (state_stack*)p->v[1];
                const gl_groupinfo *groupinfo = (gl_groupinfo *)p->v[2];

                ASSERT(ObjectIsType((br_object *)geom, BRT_GEOMETRY_STORED));

                *renderer->state.current = *state;
                BrPoolBlockFree(renderer->state_pool, p->v[1]);

                StoredGLRenderGroup(geom, (br_renderer *)renderer, groupinfo);
            } else {
                /*
                 * We can't handle other things yet.
                 */
                continue;
            }
        }
    }

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
    ._free       = BR_CMETHOD_REF(br_geometry_v1_buckets_gl, free),
    ._identifier = BR_CMETHOD_REF(br_geometry_v1_buckets_gl, identifier),
    ._type       = BR_CMETHOD_REF(br_geometry_v1_buckets_gl, type),
    ._isType     = BR_CMETHOD_REF(br_geometry_v1_buckets_gl, isType),
    ._device     = BR_CMETHOD_REF(br_geometry_v1_buckets_gl, device),
    ._space      = BR_CMETHOD_REF(br_geometry_v1_buckets_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_geometry_v1_buckets_gl, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._render         = BR_CMETHOD_REF(br_geometry_v1_buckets_gl, render),
    ._renderOnScreen = BR_CMETHOD_REF(br_geometry_v1_buckets_gl, render),
};
