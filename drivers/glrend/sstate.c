/*
 * Stored renderer state
 */
#include "drv.h"

/*
 * Default dispatch table for renderer type (defined at and of file)
 */
static const struct br_renderer_state_stored_dispatch rendererStateStoredDispatch;

/*
 * Geometry format info. template
 */
#define F(f) offsetof(struct br_renderer_state_stored, f)

static struct br_tv_template_entry rendererStateStoredTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR), F(identifier),  BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(RENDERER_O),      F(renderer),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(PARTS_U32),       F(state.valid), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef F

/*
 * Allocate a stored state
 */
br_renderer_state_stored *RendererStateStoredGLAllocate(br_renderer *renderer, state_stack *base_state, br_uint_32 m,
                                                        br_token_value *tv)
{
    br_renderer_state_stored *self;

    self             = BrResAllocate(renderer, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch   = &rendererStateStoredDispatch;
    self->identifier = "Renderer-State-Stored";
    self->device     = ObjectDevice(renderer);
    self->renderer   = renderer;

    StateGLCopy(&self->state, base_state, m);
    ObjectContainerAddFront(renderer, (br_object *)self);
    return self;
}

static void BR_CMETHOD_DECL(br_renderer_state_stored_gl, free)(br_object *_self)
{
    br_renderer_state_stored *self = (br_renderer_state_stored *)_self;

    ObjectContainerRemove(self->renderer, (br_object *)self);

    /*
     * Any associated primitive state will have been attached as a resource
     */
    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_renderer_state_stored_gl, identifier)(br_object *self)
{
    return ((br_renderer_state_stored *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_renderer_state_stored_gl, type)(br_object *self)
{
    return BRT_RENDERER_STATE_STORED;
}

static br_boolean BR_CMETHOD_DECL(br_renderer_state_stored_gl, isType)(br_object *self, br_token t)
{
    return (t == BRT_RENDERER_STATE_STORED) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_renderer_state_stored_gl, device)(br_object *self)
{
    return ((br_renderer_state_stored *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_renderer_state_stored_gl, space)(br_object *self)
{
    return sizeof(br_renderer_state_stored);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_renderer_state_stored_gl, templateQuery)(br_object *_self)
{
    br_renderer_state_stored *self = (br_renderer_state_stored *)_self;

    if(self->device->templates.rendererStateStoredTemplate == NULL) {
        self->device->templates.rendererStateStoredTemplate = BrTVTemplateAllocate(
            self->device, (br_tv_template_entry *)rendererStateStoredTemplateEntries,
            BR_ASIZE(rendererStateStoredTemplateEntries));
    }

    return self->device->templates.rendererStateStoredTemplate;
}

/*
 * Default dispatch table for renderer type (defined at and of file)
 */
static const struct br_renderer_state_stored_dispatch rendererStateStoredDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_renderer_state_stored_gl, free),
    ._identifier = BR_CMETHOD_REF(br_renderer_state_stored_gl, identifier),
    ._type       = BR_CMETHOD_REF(br_renderer_state_stored_gl, type),
    ._isType     = BR_CMETHOD_REF(br_renderer_state_stored_gl, isType),
    ._device     = BR_CMETHOD_REF(br_renderer_state_stored_gl, device),
    ._space      = BR_CMETHOD_REF(br_renderer_state_stored_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_renderer_state_stored_gl, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),
};
