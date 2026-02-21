/*
 * Geometry format for version 1 models.
 *
 * vkrend only handles the stored path: a model's prepared groups are uploaded to a GPU
 * vertex/index buffer pair when BrModelUpdate() calls storedNew(), and drawing goes through
 * the bucket order table. Immediate BRT_TRIANGLE rendering is not implemented.
 */
#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for geometry type (defined at end of file)
 */
static const struct br_geometry_v1_model_dispatch geometryV1ModelDispatch;

/*
 * Geometry format info. template
 */
#define F(f) offsetof(struct br_geometry_v1_model, f)
#define A(a) ((br_uintptr_t)(a))

static br_tv_template_entry geometryV1ModelTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),     F(identifier),        BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY},
    {BRT(RENDERER_FACILITY_O), F(renderer_facility), BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY},
    {BRT(FACILITY_O),          F(renderer_facility), BRTV_QUERY,                       BRTV_CONV_COPY},
    {BRT(STORED_ONLY_B),       A(BR_TRUE),           BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY},
};
#undef F
#undef A

/*
 * Allocate a geometry format
 */
br_geometry_v1_model *GeometryV1ModelVkAllocate(br_renderer_facility *type, const char *id)
{
    br_geometry_v1_model *self;

    self                    = BrResAllocate(type, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch          = &geometryV1ModelDispatch;
    self->identifier        = id;
    self->device            = type->device;
    self->renderer_facility = type;

    ObjectContainerAddFront(type, (br_object *)self);
    return self;
}

static void BR_CMETHOD_DECL(br_geometry_v1_model_vk, free)(br_object *_self)
{
    br_geometry_v1_model *self = (br_geometry_v1_model *)_self;

    ObjectContainerRemove(self->renderer_facility, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_geometry_v1_model_vk, identifier)(br_object *self)
{
    return ((br_geometry_v1_model *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_geometry_v1_model_vk, type)(br_object *self)
{
    (void)self;
    return BRT_GEOMETRY_V1_MODEL;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_v1_model_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_GEOMETRY_V1_MODEL) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_geometry_v1_model_vk, device)(br_object *self)
{
    return ((br_geometry_v1_model *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_geometry_v1_model_vk, space)(br_object *self)
{
    (void)self;
    return sizeof(br_geometry_v1_model);
}

static br_tv_template *BR_CMETHOD_DECL(br_geometry_v1_model_vk, templateQuery)(br_object *_self)
{
    br_geometry_v1_model *self = (br_geometry_v1_model *)_self;

    if(self->device->templates.geometryV1ModelTemplate == NULL) {
        self->device->templates.geometryV1ModelTemplate = BrTVTemplateAllocate(
            self->device, (br_tv_template_entry *)geometryV1ModelTemplateEntries, BR_ASIZE(geometryV1ModelTemplateEntries));
    }

    return self->device->templates.geometryV1ModelTemplate;
}

static br_error BR_CMETHOD_DECL(br_geometry_v1_model_vk, storedAvail)(struct br_geometry_v1_model *self, br_int_32 *psize, br_token_value *tv)
{
    (void)self;
    (void)psize;
    (void)tv;
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_geometry_v1_model_vk, storedNew)(struct br_geometry_v1_model *self, struct br_renderer *renderer,
                                                                    struct br_geometry_stored **psg, struct v11model *model, br_token type,
                                                                    br_token_value *tv)
{
    (void)tv;

    if(type != BRT_TRIANGLE)
        return BRE_FAIL;

    if((*psg = GeometryStoredVkAllocate(self, "model", renderer, model)) == NULL)
        return BRE_FAIL;

    return BRE_OK;
}

/*
 * Immediate-mode rendering needs a vertex pool and per-triangle submission that this
 * renderer does not have. Like glrend, warn once and fail; the stored path is the one the
 * core uses because the format advertises BRT_STORED_ONLY_B.
 */
static br_error BR_CMETHOD_DECL(br_geometry_v1_model_vk, render)(struct br_geometry_v1_model *self, struct br_renderer *renderer,
                                                                 struct v11model *model, struct br_renderer_state_stored *default_state,
                                                                 br_token type)
{
    static br_boolean warned_imm = BR_FALSE;

    (void)self;
    (void)renderer;
    (void)model;
    (void)default_state;
    (void)type;

    if(warned_imm != BR_TRUE) {
        BrLogWarn("VKREND", "Immediate-mode rendering is not supported.");
        BrLogWarn("VKREND", "This message will not be shown again.");
        warned_imm = BR_TRUE;
    }

    return BRE_FAIL;
}

/*
 * Default dispatch table for geometry type
 */
static const struct br_geometry_v1_model_dispatch geometryV1ModelDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_geometry_v1_model_vk, free),
    ._identifier = BR_CMETHOD_REF(br_geometry_v1_model_vk, identifier),
    ._type       = BR_CMETHOD_REF(br_geometry_v1_model_vk, type),
    ._isType     = BR_CMETHOD_REF(br_geometry_v1_model_vk, isType),
    ._device     = BR_CMETHOD_REF(br_geometry_v1_model_vk, device),
    ._space      = BR_CMETHOD_REF(br_geometry_v1_model_vk, space),

    ._templateQuery = BR_CMETHOD_REF(br_geometry_v1_model_vk, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._render         = BR_CMETHOD_REF(br_geometry_v1_model_vk, render),
    ._renderOnScreen = BR_CMETHOD_REF(br_geometry_v1_model_vk, render),
    ._storedNew      = BR_CMETHOD_REF(br_geometry_v1_model_vk, storedNew),

    ._storedAvail = BR_CMETHOD_REF(br_geometry_v1_model_vk, storedAvail),
};
