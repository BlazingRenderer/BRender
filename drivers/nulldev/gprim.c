/*
 * Geometry format for primitives
 */
#include <stddef.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Default dispatch table for renderer type (defined at and of file)
 */
static const struct br_geometry_primitives_dispatch geometryPrimitivesDispatch;

/*
 * Geometry format info. template
 */
#define F(f) offsetof(struct br_geometry_primitives, f)

static struct br_tv_template_entry geometryPrimitivesTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR,     NULL, F(identifier),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT_RENDERER_FACILITY_O, NULL, F(renderer_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT_FACILITY_O,          NULL, F(renderer_facility), BRTV_QUERY,            BRTV_CONV_COPY},
};
#undef F

/*
 * Allocate a geometry format
 */
br_geometry_primitives *GeometryPrimitivesNullAllocate(br_renderer_facility *type, const char *id)
{
    br_geometry_primitives *self;

    if((self = BrResAllocate(type, sizeof(*self), BR_MEMORY_OBJECT)) == NULL)
        return NULL;

    self->dispatch          = &geometryPrimitivesDispatch;
    self->identifier        = id;
    self->renderer_facility = type;
    self->device            = ObjectDevice(type);
    self->templates         = BrTVTemplateAllocate(self, (br_tv_template_entry *)geometryPrimitivesTemplateEntries,
                                                   BR_ASIZE(geometryPrimitivesTemplateEntries));

    if(self->templates == NULL) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    ObjectContainerAddFront(type, (br_object *)self);
    return self;
}

static void BR_CMETHOD_DECL(br_geometry_primitives_null, free)(br_object *self)
{
    ObjectContainerRemove(((br_geometry_primitives *)self)->renderer_facility, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_geometry_primitives_null, identifier)(br_object *self)
{
    return ((br_geometry_primitives *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_geometry_primitives_null, type)(br_object *self)
{
    (void)self;
    return BRT_GEOMETRY_PRIMITIVES;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_primitives_null, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_GEOMETRY_PRIMITIVES) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_geometry_primitives_null, device)(br_object *self)
{
    return ((br_geometry_primitives *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_geometry_primitives_null, space)(br_object *self)
{
    (void)self;
    return sizeof(br_geometry_primitives);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_geometry_primitives_null, templateQuery)(br_object *self)
{
    return ((br_geometry_primitives *)self)->templates;
}

br_error BR_CMETHOD_DECL(br_geometry_primitives_null, render)(struct br_geometry_primitives *self,
                                                              struct br_renderer *renderer, struct fmt_vertex *vertices,
                                                              int nvertices, br_token type)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_geometry_primitives_null,
                         renderOnScreen)(struct br_geometry_primitives *self, struct br_renderer *renderer,
                                         struct fmt_vertex *vertices, int nvertices, br_token type)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_geometry_primitives_null,
                         storedNew)(struct br_geometry_primitives *self, struct br_renderer *renderer,
                                    struct br_geometry_stored **psg, struct fmt_vertex *vertices, int nvertices,
                                    br_token type, br_token_value *tv)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_geometry_primitives_null, storedAvail)(struct br_geometry_primitives *self,
                                                                          br_int_32 *psize, br_token_value *tv)
{
    return BRE_FAIL;
}

/*
 * Default dispatch table for renderer type
 */
static const struct br_geometry_primitives_dispatch geometryPrimitivesDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_geometry_primitives_null, free),
    ._identifier = BR_CMETHOD_REF(br_geometry_primitives_null, identifier),
    ._type       = BR_CMETHOD_REF(br_geometry_primitives_null, type),
    ._isType     = BR_CMETHOD_REF(br_geometry_primitives_null, isType),
    ._device     = BR_CMETHOD_REF(br_geometry_primitives_null, device),
    ._space      = BR_CMETHOD_REF(br_geometry_primitives_null, space),

    ._templateQuery = BR_CMETHOD_REF(br_geometry_primitives_null, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._render         = BR_CMETHOD_REF(br_geometry_primitives_null, render),
    ._renderOnScreen = BR_CMETHOD_REF(br_geometry_primitives_null, renderOnScreen),
    ._storedNew      = BR_CMETHOD_REF(br_geometry_primitives_null, storedNew),

    ._storedAvail = BR_CMETHOD_REF(br_geometry_primitives_null, storedAvail),
};
