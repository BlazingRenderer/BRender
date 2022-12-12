/*
 * Renderer methods
 */
#include "drv.h"

/*
 * Default dispatch table for renderer (defined at end of file)
 */
static const struct br_renderer_dispatch rendererDispatch;

#define F(f) offsetof(struct br_renderer, f)

static struct br_tv_template_entry rendererTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR,              NULL, F(identifier),                     BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_FACE_GROUP_COUNT_U32,         NULL, F(stats.face_group_count),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_TRIANGLES_DRAWN_COUNT_U32,    NULL, F(stats.triangles_drawn_count),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_TRIANGLES_RENDERED_COUNT_U32, NULL, F(stats.triangles_rendered_count), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_VERTICES_RENDERED_COUNT_U32,  NULL, F(stats.vertices_rendered_count),  BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
};
#undef F

/*
 * Create a new renderer
 */
br_renderer *RendererGLAllocate(br_device *device, br_renderer_facility *facility, br_device_pixelmap *dest)
{
    br_renderer *self;

    /*
     * Check that destination is valid
     */
    if(dest == NULL || ObjectDevice(dest) != device)
        return NULL;

    if(dest->use_type != BRT_OFFSCREEN)
        return NULL;

    self = BrResAllocate(facility, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch          = &rendererDispatch;
    self->identifier        = facility->identifier;
    self->device            = device;
    self->object_list       = BrObjectListAllocate(self);
    self->pixelmap          = dest;
    self->renderer_facility = facility;
    self->state_pool        = BrPoolAllocate(sizeof(GLSTATE_STACK), 1024, BR_MEMORY_OBJECT_DATA);

    ObjectContainerAddFront(facility, (br_object*)self);

    GLSTATE_Init(&self->state, self->device);

    /*
     * State starts out as default
     */
    RendererStateDefault(self, (br_uint_32)BR_STATE_ALL);

    self->has_begun = 0;
    return (br_renderer *)self;
}

static void BR_CMETHOD_DECL(br_renderer_gl, sceneBegin)(br_renderer *self)
{
    HVIDEO hVideo = &self->device->video;

    self->stats.face_group_count         = 0;
    self->stats.triangles_drawn_count    = 0;
    self->stats.triangles_rendered_count = 0;
    self->stats.vertices_rendered_count  = 0;

    /* First draw call, so do all the per-scene crap */
    while(glGetError() != GL_NO_ERROR);

    GLCACHE_Reset(&self->state.cache);
    GLCACHE_UpdateScene(&self->state.cache, self->state.current);

    glUseProgram(hVideo->brenderProgram.program);
    glBindFramebuffer(GL_FRAMEBUFFER, self->state.cache.fbo);
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingScene, hVideo->brenderProgram.uboScene);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(self->state.cache.scene), &self->state.cache.scene);

    /* Bind the model UBO here, it's faster than doing it for each model group */
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingModel, hVideo->brenderProgram.uboModel);

    if(self->pixelmap->msaa_samples)
        glEnable(GL_MULTISAMPLE);

    self->has_begun = 1;
}

void BR_CMETHOD_DECL(br_renderer_gl, sceneEnd)(br_renderer *self)
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_MULTISAMPLE);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    self->has_begun = 0;
}

static void BR_CMETHOD_DECL(br_renderer_gl, free)(br_object *_self)
{
    br_renderer *self = (br_renderer *)_self;

    BrPoolFree(self->state_pool);

    ObjectContainerRemove(self->renderer_facility, (br_object *)self);

    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_renderer_gl, identifier)(br_object *self)
{
    return ((br_renderer *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_renderer_gl, type)(br_object *self)
{
    return BRT_RENDERER;
}

static br_boolean BR_CMETHOD_DECL(br_renderer_gl, isType)(br_object *self, br_token t)
{
    return (t == BRT_RENDERER) || (t == BRT_OBJECT);
}

static struct br_device *BR_CMETHOD_DECL(br_renderer_gl, device)(br_object *self)
{
    return ((br_renderer *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_renderer_gl, space)(br_object *self)
{
    return sizeof(br_renderer);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_renderer_gl, templateQuery)(br_object *_self)
{
    br_renderer *self = (br_renderer *)_self;

    if(self->device->templates.rendererTemplate == NULL) {
        self->device->templates.rendererTemplate = BrTVTemplateAllocate(
            self->device,
            rendererTemplateEntries,
            BR_ASIZE(rendererTemplateEntries)
        );
    }

    return self->device->templates.rendererTemplate;
}

static void *BR_CMETHOD_DECL(br_renderer_gl, listQuery)(br_object_container *self)
{
    return ((br_renderer *)self)->object_list;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, validDestination)(br_renderer *self, br_boolean *bp, br_object *h)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateStoredNew)(br_renderer *self, br_renderer_state_stored **pss,
                                                                br_uint_32 mask, br_token_value *tv)
{
    br_renderer_state_stored *ss;

    if((ss = RendererStateStoredGLAllocate(self, self->state.current, mask, tv)) == NULL)
        return BRE_FAIL;

    *pss = ss;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateStoredAvail)(br_renderer *self, br_int_32 *psize, br_uint_32 mask,
                                                                  br_token_value *tv)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, bufferStoredNew)(br_renderer *self, br_buffer_stored **psm,
                                                                 br_token use, br_device_pixelmap *pm,
                                                                 br_token_value *tv)
{
    br_buffer_stored *sm;

    if((sm = BufferStoredGLAllocate(self, use, pm, tv)) == NULL)
        return BRE_FAIL;

    *psm = sm;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, bufferStoredAvail)(br_renderer *self, br_int_32 *space, br_token use,
                                                                   br_token_value *tv)
{
    /*
     * Should return free VRAM
     */
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}


/*
 * Setting current state
 */
br_error BR_CMETHOD_DECL(br_renderer_gl, partSet)(br_renderer *self, br_token part, br_int_32 index, br_token t,
                                                  br_value value)
{
    br_error              r;
    br_uint_32            m;
    struct br_tv_template *tp;
    br_value_passthrough  pthru;

    if((tp = GLSTATE_GetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    BrMemCpy(&pthru, &value, sizeof(value));

    m = 0;
    r = BrTokenValueSet(self->state.current, &m, t, pthru, tp);
    if(m)
        GLSTATE_TemplateActions(&self->state, m);

    return r;
}

static br_error
BR_CMETHOD_DECL(br_renderer_gl, partSetMany)(br_renderer *self, br_token part, br_int_32 index, br_token_value *tv,
                                             br_int_32 *pcount)
{
    br_error              r;
    br_uint_32            m;
    struct br_tv_template *tp;

    if((tp = GLSTATE_GetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    m = 0;
    r = BrTokenValueSetMany(self->state.current, pcount, &m, tv, tp);
    if(m)
        GLSTATE_TemplateActions(&self->state, m);

    return r;
}

/*
 * Reading current state
 */
static br_error BR_CMETHOD_DECL(br_renderer_gl, partQuery)(br_renderer *self, br_token part, br_int_32 index,
                                                           void *pvalue, br_token t)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryBuffer)(br_renderer *self, br_token part, br_int_32 index,
                                                                 void *pvalue, void *buffer,
                                                                 br_size_t buffer_size, br_token t)
{
    struct br_tv_template *tp;

    if((tp = GLSTATE_GetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQuery(pvalue, buffer, buffer_size, t, self->state.current, tp);
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryMany)(br_renderer *self, br_token part, br_int_32 index,
                                                               br_token_value *tv, void *extra, br_size_t extra_size,
                                                               br_int_32 *pcount)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryManySize)(br_renderer *self, br_token part, br_int_32 index,
                                                                   br_size_t *pextra_size, br_token_value *tv)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryAll)(br_renderer *self, br_token part, br_int_32 index,
                                                              br_token_value *buffer, br_size_t buffer_size)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryAllSize)(br_renderer *self, br_token part, br_int_32 index,
                                                                  br_size_t *psize)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partIndexQuery)(br_renderer *self, br_token part, br_int_32 *pnindex)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModeSet)(br_renderer *self, br_token mode)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModeQuery)(br_renderer *self, br_token *mode)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModeDefault)(br_renderer *self)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModePush)(br_renderer *self)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModePop)(br_renderer *self)
{
    return BRE_FAIL;
}


static br_error BR_CMETHOD_DECL(br_renderer_gl, modelMul)(br_renderer *self, br_matrix34_f *m)
{
    br_matrix34 om = self->state.current->matrix.model_to_view;

    BrMatrix34Mul(&self->state.current->matrix.model_to_view, (br_matrix34 *)m, &om);

    self->state.current->matrix.model_to_view_hint = BRT_NONE;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, modelPopPushMul)(br_renderer *self, br_matrix34_f *m)
{
    if(self->state.top == 0)
        return BRE_UNDERFLOW;

    BrMatrix34Mul(
        &self->state.current->matrix.model_to_view,
        (br_matrix34 *)m,
        &self->state.stack[0].matrix.model_to_view
    );

    self->state.current->matrix.model_to_view_hint = BRT_NONE;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, modelInvert)(br_renderer *self)
{
    br_matrix34 old;

    BrMatrix34Copy(&old, &self->state.current->matrix.model_to_view);

    if(self->state.current->matrix.model_to_view_hint == BRT_LENGTH_PRESERVING)
        BrMatrix34LPInverse(&self->state.current->matrix.model_to_view, &old);
    else
        BrMatrix34Inverse(&self->state.current->matrix.model_to_view, &old);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, statePush)(br_renderer *self, br_uint_32 mask)
{
    return GLSTATE_Push(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, statePop)(br_renderer *self, br_uint_32 mask)
{
    return GLSTATE_Pop(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

static br_error
BR_CMETHOD_DECL(br_renderer_gl, stateSave)(br_renderer *self, br_renderer_state_stored *save, br_uint_32 mask)
{
    GLSTATE_Copy(&save->state, self->state.current, mask);
    return BRE_OK;
}

static br_error
BR_CMETHOD_DECL(br_renderer_gl, stateRestore)(br_renderer *self, br_renderer_state_stored *save, br_uint_32 mask)
{
    GLSTATE_Copy(self->state.current, &save->state, mask);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateDefault)(br_renderer *self, br_uint_32 mask)
{
    GLSTATE_Default(&self->state, mask);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateMask)(br_renderer *self, br_uint_32 *mask,
                                                           br_token *parts, int n_parts)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, boundsTest)(br_renderer *self, br_token *r, br_bounds3_f *bounds)
{
    // FIXME: Should probably cache this.
    br_matrix4 m2s;
    BrMatrix4Mul34(&m2s, &self->state.current->matrix.model_to_view, &self->state.current->matrix.view_to_screen);
    *r = GLOnScreenCheck(&m2s, bounds);
    return BRE_OK;
}


static br_error BR_CMETHOD_DECL(br_renderer_gl, coverageTest)(br_renderer *self, br_float *r, br_bounds3_f *bounds)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, viewDistance)(br_renderer *self, br_float *r)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, flush)(br_renderer *self, br_boolean wait)
{
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, synchronise)(br_renderer *self, br_token sync_type, br_boolean block)
{
    return BRE_UNSUPPORTED;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, testRender)(br_renderer *self, br_token type, union brp_vertex *v0,
                                                            union brp_vertex *v1, union brp_vertex *v2)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryCapability)(br_renderer *self, br_token part, br_int_32 index,
                                                                     br_token_value *buffer, br_size_t buffer_size)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}


static br_error BR_CMETHOD_DECL(br_renderer_gl, stateQueryPerformance)(br_renderer *self, br_fixed_lu *speed)
{
    BrLogTrace("GLREND", "%s", __FUNCTION__);
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, frameBegin)(br_renderer *self)
{
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, frameEnd)(br_renderer *self)
{
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, focusLossBegin)(br_renderer *self)
{
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, focusLossEnd)(br_renderer *self)
{
    return BRE_OK;
}


/*
 * Default dispatch table for renderer
 */
static const struct br_renderer_dispatch rendererDispatch = {
    .__reserved0            = NULL,
    .__reserved1            = NULL,
    .__reserved2            = NULL,
    .__reserved3            = NULL,
    ._free                  = BR_CMETHOD(br_renderer_gl, free),
    ._identifier            = BR_CMETHOD(br_renderer_gl, identifier),
    ._type                  = BR_CMETHOD(br_renderer_gl, type),
    ._isType                = BR_CMETHOD(br_renderer_gl, isType),
    ._device                = BR_CMETHOD(br_renderer_gl, device),
    ._space                 = BR_CMETHOD(br_renderer_gl, space),
    ._templateQuery         = BR_CMETHOD(br_renderer_gl, templateQuery),
    ._query                 = BR_CMETHOD(br_object, query),
    ._queryBuffer           = BR_CMETHOD(br_object, queryBuffer),
    ._queryMany             = BR_CMETHOD(br_object, queryMany),
    ._queryManySize         = BR_CMETHOD(br_object, queryManySize),
    ._queryAll              = BR_CMETHOD(br_object, queryAll),
    ._queryAllSize          = BR_CMETHOD(br_object, queryAllSize),
    ._listQuery             = BR_CMETHOD(br_renderer_gl, listQuery),
    ._tokensMatchBegin      = BR_CMETHOD(br_object_container, tokensMatchBegin),
    ._tokensMatch           = BR_CMETHOD(br_object_container, tokensMatch),
    ._tokensMatchEnd        = BR_CMETHOD(br_object_container, tokensMatchEnd),
    ._addFront              = BR_CMETHOD(br_object_container, addFront),
    ._removeFront           = BR_CMETHOD(br_object_container, removeFront),
    ._remove                = BR_CMETHOD(br_object_container, remove),
    ._find                  = BR_CMETHOD(br_object_container, find),
    ._findMany              = BR_CMETHOD(br_object_container, findMany),
    ._count                 = BR_CMETHOD(br_object_container, count),

    ._validDestination      = BR_CMETHOD(br_renderer_gl, validDestination),
    ._stateStoredNew        = BR_CMETHOD(br_renderer_gl, stateStoredNew),
    ._stateStoredAvail      = BR_CMETHOD(br_renderer_gl, stateStoredAvail),
    ._bufferStoredNew       = BR_CMETHOD(br_renderer_gl, bufferStoredNew),
    ._bufferStoredAvail     = BR_CMETHOD(br_renderer_gl, bufferStoredAvail),
    ._partSet               = BR_CMETHOD(br_renderer_gl, partSet),
    ._partSetMany           = BR_CMETHOD(br_renderer_gl, partSetMany),
    ._partQuery             = BR_CMETHOD(br_renderer_gl, partQuery),
    ._partQueryBuffer       = BR_CMETHOD(br_renderer_gl, partQueryBuffer),
    ._partQueryMany         = BR_CMETHOD(br_renderer_gl, partQueryMany),
    ._partQueryManySize     = BR_CMETHOD(br_renderer_gl, partQueryManySize),
    ._partQueryAll          = BR_CMETHOD(br_renderer_gl, partQueryAll),
    ._partQueryAllSize      = BR_CMETHOD(br_renderer_gl, partQueryAllSize),
    ._partIndexQuery        = BR_CMETHOD(br_renderer_gl, partIndexQuery),
    ._modelMul              = BR_CMETHOD(br_renderer_gl, modelMul),
    ._modelPopPushMul       = BR_CMETHOD(br_renderer_gl, modelPopPushMul),
    ._modelInvert           = BR_CMETHOD(br_renderer_gl, modelInvert),
    ._statePush             = BR_CMETHOD(br_renderer_gl, statePush),
    ._statePop              = BR_CMETHOD(br_renderer_gl, statePop),
    ._stateSave             = BR_CMETHOD(br_renderer_gl, stateSave),
    ._stateRestore          = BR_CMETHOD(br_renderer_gl, stateRestore),
    ._stateMask             = BR_CMETHOD(br_renderer_gl, stateMask),
    ._stateDefault          = BR_CMETHOD(br_renderer_gl, stateDefault),
    ._boundsTest            = BR_CMETHOD(br_renderer_gl, boundsTest),
    ._coverageTest          = BR_CMETHOD(br_renderer_gl, coverageTest),
    ._viewDistance          = BR_CMETHOD(br_renderer_gl, viewDistance),
    ._commandModeSet        = BR_CMETHOD(br_renderer_gl, commandModeSet),
    ._commandModeQuery      = BR_CMETHOD(br_renderer_gl, commandModeQuery),
    ._commandModeDefault    = BR_CMETHOD(br_renderer_gl, commandModeDefault),
    ._commandModePush       = BR_CMETHOD(br_renderer_gl, commandModePush),
    ._commandModePop        = BR_CMETHOD(br_renderer_gl, commandModePop),
    ._flush                 = BR_CMETHOD(br_renderer_gl, flush),
    ._synchronise           = BR_CMETHOD(br_renderer_gl, synchronise),
    ._testRender            = BR_CMETHOD(br_renderer_gl, testRender),
    ._partQueryCapability   = BR_CMETHOD(br_renderer_gl, partQueryCapability),
    ._stateQueryPerformance = BR_CMETHOD(br_renderer_gl, stateQueryPerformance),
    ._frameBegin            = BR_CMETHOD(br_renderer_gl, frameBegin),
    ._frameEnd              = BR_CMETHOD(br_renderer_gl, frameEnd),
    ._focusLossBegin        = BR_CMETHOD(br_renderer_gl, focusLossBegin),
    ._focusLossEnd          = BR_CMETHOD(br_renderer_gl, focusLossEnd),
    ._sceneBegin            = BR_CMETHOD(br_renderer_gl, sceneBegin),
    ._sceneEnd              = BR_CMETHOD(br_renderer_gl, sceneEnd),
};
