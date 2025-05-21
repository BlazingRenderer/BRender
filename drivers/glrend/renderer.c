/*
 * Renderer methods
 */
#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for renderer (defined at end of file)
 */
static const struct br_renderer_dispatch rendererDispatch;

#define F(f) offsetof(struct br_renderer, f)

static struct br_tv_template_entry rendererTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),              F(identifier),                     BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(FACE_GROUP_COUNT_U32),         F(stats.face_group_count),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(TRIANGLES_DRAWN_COUNT_U32),    F(stats.triangles_drawn_count),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(TRIANGLES_RENDERED_COUNT_U32), F(stats.triangles_rendered_count), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(VERTICES_RENDERED_COUNT_U32),  F(stats.vertices_rendered_count),  BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {DEV(OPENGL_OPAQUE_DRAW_COUNT_U32),      F(stats.opaque_draw_count),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {DEV(OPENGL_TRANSPARENT_DRAW_COUNT_U32), F(stats.transparent_draw_count),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef F

static void RendererGLInitImm(br_renderer *self, HVIDEO hVideo)
{
    glGenVertexArrays(1, &self->trans.vao);
    glBindVertexArray(self->trans.vao);

    glGenBuffers(1, &self->trans.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->trans.vbo);

    if(hVideo->brenderProgram.attributes.aUV >= 0) {
        glEnableVertexAttribArray(hVideo->brenderProgram.attributes.aPosition);
        glVertexAttribPointer(hVideo->brenderProgram.attributes.aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(br_immvert_gl),
                              (void *)offsetof(br_immvert_gl, position));
    }

    if(hVideo->brenderProgram.attributes.aUV >= 0) {
        glEnableVertexAttribArray(hVideo->brenderProgram.attributes.aUV);
        glVertexAttribPointer(hVideo->brenderProgram.attributes.aUV, 2, GL_FLOAT, GL_FALSE, sizeof(br_immvert_gl),
                              (void *)offsetof(br_immvert_gl, map));
    }

    if(hVideo->brenderProgram.attributes.aNormal >= 0) {
        glEnableVertexAttribArray(hVideo->brenderProgram.attributes.aNormal);
        glVertexAttribPointer(hVideo->brenderProgram.attributes.aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(br_immvert_gl),
                              (void *)offsetof(br_immvert_gl, normal));
    }

    if(hVideo->brenderProgram.attributes.aColour >= 0) {
        glEnableVertexAttribArray(hVideo->brenderProgram.attributes.aColour);
        glVertexAttribPointer(hVideo->brenderProgram.attributes.aColour, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                              sizeof(br_immvert_gl), (void *)offsetof(br_immvert_gl, colour));
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void RendererGLFreeImm(br_renderer *self)
{
    glDeleteVertexArrays(1, &self->trans.vao);
    glDeleteBuffers(1, &self->trans.vbo);
}

static br_hash SamplerInfoGLHash(const void *p)
{
    return BrHash(p, sizeof(br_sampler_info_gl));
}

static br_boolean SamplerInfoGLCompare(const void *a, const void *b)
{
    return BrMemCmp(a, b, sizeof(br_sampler_info_gl)) == 0;
}

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

    self                    = BrResAllocate(facility, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch          = &rendererDispatch;
    self->identifier        = facility->identifier;
    self->device            = device;
    self->object_list       = BrObjectListAllocate(self);
    self->pixelmap          = dest;
    self->renderer_facility = facility;
    self->state_pool        = BrPoolAllocate(sizeof(state_stack), 1024, BR_MEMORY_OBJECT_DATA);

    RendererGLInitImm(self, &dest->screen->asFront.video);

    self->sampler_pool = BrHashMapAllocate(self, SamplerInfoGLHash, SamplerInfoGLCompare);

    ObjectContainerAddFront(facility, (br_object *)self);

    StateGLInit(&self->state, self->device);

    /*
     * State starts out as default
     */
    RendererStateDefault(self, (br_uint_32)BR_STATE_ALL);

    GLint alignment = 256;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);

    self->uniform_buffer_offset_alignment = alignment;
    BufferRingGLInit(&self->model_ring, "model", alignment, BR_GLREND_MAX_DRAWS_IN_FLIGHT,
                     dest->screen->asFront.video.brenderProgram.blockBindingModel, sizeof(shader_data_model),
                     GL_UNIFORM_BUFFER, dest->screen->asFront.quirks.orphan_model_buffers ? BUFFER_RING_GL_FLAG_ORPHAN : 0);

    self->has_begun = 0;
    return (br_renderer *)self;
}

static void BR_CMETHOD_DECL(br_renderer_gl, sceneBegin)(br_renderer *self)
{
    HVIDEO              hVideo        = &self->pixelmap->screen->asFront.video;
    br_device_pixelmap *colour_target = NULL, *depth_target = NULL;

    self->stats.face_group_count         = 0;
    self->stats.triangles_drawn_count    = 0;
    self->stats.triangles_rendered_count = 0;
    self->stats.vertices_rendered_count  = 0;
    self->stats.opaque_draw_count        = 0;
    self->stats.transparent_draw_count   = 0;

    self->trans.next = 0;

    /* First draw call, so do all the per-scene crap */
    DeviceGLCheckErrors();

    if(self->state.current->valid & BR_STATE_OUTPUT) {
        colour_target = self->state.current->output.colour;
        depth_target  = self->state.current->output.depth;
    }

    if(colour_target != NULL && ObjectDevice(colour_target) != self->device) {
        BR_ERROR0("Can't render to a non-device colour pixelmap");
    }

    if(depth_target != NULL && ObjectDevice(depth_target) != self->device) {
        BR_ERROR0("Can't render to a non-device depth pixelmap");
    }

    /*
     * TODO: Consider if we want to handle depth-only renders.
     */
    if(colour_target == NULL) {
        BR_ERROR("Can't render without a destination");
    }

    /*
     * Cache some data before we blat our cache.
     */
    br_uint_32 last_num_clip_planes = self->state.cache.scene.num_clip_planes;

    StateGLReset(&self->state.cache);
    StateGLUpdateScene(&self->state.cache, self->state.current);

    glDepthRange(1.0f, 0.0f);

    glUseProgram(hVideo->brenderProgram.program);
    glUniform1i(hVideo->brenderProgram.uniforms.main_texture, hVideo->brenderProgram.mainTextureBinding);
    glUniform1i(hVideo->brenderProgram.uniforms.index_texture, hVideo->brenderProgram.indexTextureBinding);

    glBindFramebuffer(GL_FRAMEBUFFER, self->state.cache.fbo);
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingScene, hVideo->brenderProgram.uboScene);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(self->state.cache.scene), &self->state.cache.scene);

    br_rectangle viewport = DevicePixelmapGLGetViewport(colour_target);
    glViewport(viewport.x, viewport.y, viewport.w, viewport.h);

    if(self->pixelmap->msaa_samples)
        glEnable(GL_MULTISAMPLE);

    /*
     * Now enable our clip planes.
     */
    br_uint_32 current_num_clip_planes = self->state.cache.scene.num_clip_planes;

    if(last_num_clip_planes > current_num_clip_planes) {
        for(br_uint_32 i = current_num_clip_planes; i < last_num_clip_planes; ++i) {
            glDisable(GL_CLIP_DISTANCE0 + i);
        }
    }

    for(br_uint_32 i = last_num_clip_planes; i < current_num_clip_planes; ++i) {
        glEnable(GL_CLIP_DISTANCE0 + i);
    }

    BufferRingGLBegin(&self->model_ring);

    self->has_begun = 1;
}

void BR_CMETHOD_DECL(br_renderer_gl, sceneEnd)(br_renderer *self)
{
    BufferRingGLEnd(&self->model_ring);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_MULTISAMPLE);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    self->has_begun = 0;

    BrPoolEmpty(self->state_pool);

    self->trans.next = 0;
}

static int RendererGLDeleteSampler(const void *key, void *value, br_hash hash, void *user)
{
    GLuint sampler = (GLuint)(br_uintptr_t)value;
    glDeleteSamplers(1, &sampler);
    return 0;
}

static void BR_CMETHOD_DECL(br_renderer_gl, free)(br_object *_self)
{
    br_renderer *self = (br_renderer *)_self;

    RendererGLFreeImm(self);

    BrHashMapEnumerate(self->sampler_pool, RendererGLDeleteSampler, NULL);

    BrPoolFree(self->state_pool);

    ObjectContainerRemove(self->renderer_facility, (br_object *)self);

    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BufferRingGLFini(&self->model_ring);

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
        self->device->templates.rendererTemplate = BrTVTemplateAllocate(self->device, rendererTemplateEntries,
                                                                        BR_ASIZE(rendererTemplateEntries));
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

static br_error BR_CMETHOD_DECL(br_renderer_gl, bufferStoredNew)(br_renderer *self, br_buffer_stored **psm, br_token use,
                                                                 br_device_pixelmap *pm, br_token_value *tv)
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
    (void)self;
    (void)space;
    (void)use;
    (void)tv;
    return BRE_FAIL;
}

/*
 * Setting current state
 */
br_error BR_CMETHOD_DECL(br_renderer_gl, partSet)(br_renderer *self, br_token part, br_int_32 index, br_token t, br_value value)
{
    br_error               r;
    br_uint_32             m;
    struct br_tv_template *tp;

    if((tp = StateGLGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    m = 0;
    r = BrTokenValueSet(self->state.current, &m, t, value, tp);
    if(m)
        StateGLTemplateActions(&self->state, m);

    return r;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partSetMany)(br_renderer *self, br_token part, br_int_32 index,
                                                             br_token_value *tv, br_int_32 *pcount)
{
    br_error               r;
    br_uint_32             m;
    struct br_tv_template *tp;

    if((tp = StateGLGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    m = 0;
    r = BrTokenValueSetMany(self->state.current, pcount, &m, tv, tp);
    if(m)
        StateGLTemplateActions(&self->state, m);

    return r;
}

/*
 * Reading current state
 */
static br_error BR_CMETHOD_DECL(br_renderer_gl, partQuery)(br_renderer *self, br_token part, br_int_32 index,
                                                           void *pvalue, br_token t)
{
    struct br_tv_template *tp;

    if((tp = StateGLGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQuery(pvalue, NULL, 0, t, self->state.current, tp);
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryBuffer)(br_renderer *self, br_token part, br_int_32 index,
                                                                 void *pvalue, void *buffer, br_size_t buffer_size, br_token t)
{
    struct br_tv_template *tp;

    if((tp = StateGLGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQuery(pvalue, buffer, buffer_size, t, self->state.current, tp);
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryMany)(br_renderer *self, br_token part, br_int_32 index,
                                                               br_token_value *tv, void *extra, br_size_t extra_size,
                                                               br_int_32 *pcount)
{
    struct br_tv_template *tp;

    if((tp = StateGLGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryMany(tv, extra, extra_size, pcount, self->state.current, tp);
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryManySize)(br_renderer *self, br_token part, br_int_32 index,
                                                                   br_size_t *pextra_size, br_token_value *tv)
{
    struct br_tv_template *tp;

    if((tp = StateGLGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryManySize(pextra_size, tv, self->state.current, tp);
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryAll)(br_renderer *self, br_token part, br_int_32 index,
                                                              br_token_value *buffer, br_size_t buffer_size)
{
    struct br_tv_template *tp;

    if((tp = StateGLGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryAll(buffer, buffer_size, self->state.current, tp);
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryAllSize)(br_renderer *self, br_token part, br_int_32 index,
                                                                  br_size_t *psize)
{
    struct br_tv_template *tp;

    if((tp = StateGLGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryAllSize(psize, self->state.current, tp);
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partIndexQuery)(br_renderer *self, br_token part, br_int_32 *pnindex)
{
    (void)self;

    if(pnindex == NULL)
        return BRE_FAIL;

    switch(part) {
        /* Renderer states. */
        case BRT_CULL:
        case BRT_SURFACE:
        case BRT_MATRIX:
        case BRT_ENABLE:
        case BRT_BOUNDS:
        case BRT_HIDDEN_SURFACE:
            *pnindex = 1;
            return BRE_OK;

        case BRT_LIGHT:
            *pnindex = MAX_STATE_LIGHTS;
            return BRE_OK;

        case BRT_CLIP:
            *pnindex = MAX_STATE_CLIP_PLANES;
            return BRE_OK;

        /* Primitive states. */
        case BRT_OUTPUT:
        case BRT_PRIMITIVE:
            *pnindex = 1;
            return BRE_OK;

        default:
            break;
    }

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

    BrMatrix34Mul(&self->state.current->matrix.model_to_view, (br_matrix34 *)m, &self->state.stack[0].matrix.model_to_view);

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
    return StateGLPush(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, statePop)(br_renderer *self, br_uint_32 mask)
{
    return StateGLPop(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateSave)(br_renderer *self, br_renderer_state_stored *save, br_uint_32 mask)
{
    StateGLCopy(&save->state, self->state.current, mask);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateRestore)(br_renderer *self, br_renderer_state_stored *save, br_uint_32 mask)
{
    StateGLCopy(self->state.current, &save->state, mask);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateDefault)(br_renderer *self, br_uint_32 mask)
{
    StateGLDefault(&self->state, mask);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateMask)(br_renderer *self, br_uint_32 *mask, br_token *parts, int n_parts)
{
    br_uint_32 m;

    (void)self;

    if(mask == NULL)
        return BRE_FAIL;

    m = 0;
    for(int i = 0; i < n_parts; i++) {
        switch(parts[i]) {
            case BRT_SURFACE:
                m |= MASK_STATE_SURFACE;
                break;

            case BRT_MATRIX:
                m |= MASK_STATE_MATRIX;
                break;

            case BRT_ENABLE:
                m |= MASK_STATE_ENABLE;
                break;

            case BRT_LIGHT:
                m |= MASK_STATE_LIGHT;
                break;

            case BRT_CLIP:
                m |= MASK_STATE_CLIP;
                break;

            case BRT_BOUNDS:
                m |= MASK_STATE_BOUNDS;
                break;

            case BRT_CULL:
                m |= MASK_STATE_CULL;
                break;

            case BRT_OUTPUT:
                m |= MASK_STATE_OUTPUT;
                break;

            case BRT_PRIMITIVE:
                m |= MASK_STATE_PRIMITIVE;
                break;

            default:
                break;
        }
    }

    *mask = m;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, boundsTest)(br_renderer *self, br_token *r, br_bounds3_f *bounds)
{
    // FIXME: Should probably cache this.
    br_matrix4 m2s;
    BrMatrix4Mul34(&m2s, &self->state.current->matrix.model_to_view, &self->state.current->matrix.view_to_screen);
    *r = GLOnScreenCheck(&m2s, bounds);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, flush)(br_renderer *self, br_boolean wait)
{
    (void)self;
    glFlush();

    if(wait)
        glFinish();

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, synchronise)(br_renderer *self, br_token sync_type, br_boolean block)
{
    return BRE_UNSUPPORTED;
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
    .__reserved0           = NULL,
    .__reserved1           = NULL,
    .__reserved2           = NULL,
    .__reserved3           = NULL,
    ._free                 = BR_CMETHOD(br_renderer_gl, free),
    ._identifier           = BR_CMETHOD(br_renderer_gl, identifier),
    ._type                 = BR_CMETHOD(br_renderer_gl, type),
    ._isType               = BR_CMETHOD(br_renderer_gl, isType),
    ._device               = BR_CMETHOD(br_renderer_gl, device),
    ._space                = BR_CMETHOD(br_renderer_gl, space),
    ._templateQuery        = BR_CMETHOD(br_renderer_gl, templateQuery),
    ._query                = BR_CMETHOD(br_object, query),
    ._queryBuffer          = BR_CMETHOD(br_object, queryBuffer),
    ._queryMany            = BR_CMETHOD(br_object, queryMany),
    ._queryManySize        = BR_CMETHOD(br_object, queryManySize),
    ._queryAll             = BR_CMETHOD(br_object, queryAll),
    ._queryAllSize         = BR_CMETHOD(br_object, queryAllSize),
    ._listQuery            = BR_CMETHOD(br_renderer_gl, listQuery),
    ._tokensMatchBegin     = BR_CMETHOD(br_object_container, tokensMatchBegin),
    ._tokensMatch          = BR_CMETHOD(br_object_container, tokensMatch),
    ._tokensMatchEnd       = BR_CMETHOD(br_object_container, tokensMatchEnd),
    ._tokensMatchInfoQuery = BR_CMETHOD_REF(br_object_container, tokensMatchInfoQuery),
    ._addFront             = BR_CMETHOD(br_object_container, addFront),
    ._removeFront          = BR_CMETHOD(br_object_container, removeFront),
    ._remove               = BR_CMETHOD(br_object_container, remove),
    ._find                 = BR_CMETHOD(br_object_container, find),
    ._findMany             = BR_CMETHOD(br_object_container, findMany),
    ._count                = BR_CMETHOD(br_object_container, count),

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
    ._commandModeSet        = BR_CMETHOD(br_renderer_gl, commandModeSet),
    ._commandModeQuery      = BR_CMETHOD(br_renderer_gl, commandModeQuery),
    ._commandModeDefault    = BR_CMETHOD(br_renderer_gl, commandModeDefault),
    ._commandModePush       = BR_CMETHOD(br_renderer_gl, commandModePush),
    ._commandModePop        = BR_CMETHOD(br_renderer_gl, commandModePop),
    ._flush                 = BR_CMETHOD(br_renderer_gl, flush),
    ._synchronise           = BR_CMETHOD(br_renderer_gl, synchronise),
    ._partQueryCapability   = BR_CMETHOD(br_renderer_gl, partQueryCapability),
    ._stateQueryPerformance = BR_CMETHOD(br_renderer_gl, stateQueryPerformance),
    ._frameBegin            = BR_CMETHOD(br_renderer_gl, frameBegin),
    ._frameEnd              = BR_CMETHOD(br_renderer_gl, frameEnd),
    ._focusLossBegin        = BR_CMETHOD(br_renderer_gl, focusLossBegin),
    ._focusLossEnd          = BR_CMETHOD(br_renderer_gl, focusLossEnd),
    ._sceneBegin            = BR_CMETHOD(br_renderer_gl, sceneBegin),
    ._sceneEnd              = BR_CMETHOD(br_renderer_gl, sceneEnd),
};

state_stack *RendererGLAllocState(br_renderer *self, const state_stack *tpl, br_uint_32 refs)
{
    state_stack *state = BrPoolBlockAllocate(self->state_pool);
    *state             = *tpl;
    state->num_refs    = refs;
    return state;
}

void RendererGLUnrefState(br_renderer *self, state_stack *state)
{
    --state->num_refs;

    if(state->num_refs == 0)
        BrPoolBlockFree(self->state_pool, state);
}

br_int_32 RendererGLNextImmTri(br_renderer *self, struct v11group *group, br_vector3_u16 fp)
{
    br_size_t base = self->trans.next;

    if(base + 3 >= BR_ASIZE(self->trans.pool))
        return -1;

    br_immvert_gl *v0 = self->trans.pool + self->trans.next++;
    br_immvert_gl *v1 = self->trans.pool + self->trans.next++;
    br_immvert_gl *v2 = self->trans.pool + self->trans.next++;

    *v0 = (br_immvert_gl){
        .position = group->position[fp.v[0]],
        .normal   = group->normal[fp.v[0]],
        .map      = group->map[fp.v[0]],
        .colour   = group->vertex_colours[fp.v[0]],
    };

    *v1 = (br_immvert_gl){
        .position = group->position[fp.v[1]],
        .normal   = group->normal[fp.v[1]],
        .map      = group->map[fp.v[1]],
        .colour   = group->vertex_colours[fp.v[1]],
    };

    *v2 = (br_immvert_gl){
        .position = group->position[fp.v[2]],
        .normal   = group->normal[fp.v[2]],
        .map      = group->map[fp.v[2]],
        .colour   = group->vertex_colours[fp.v[2]],
    };

    return (br_int_32)base;
}

GLuint RendererGLGetSampler(br_renderer *self, const br_sampler_info_gl *info)
{
    const br_quirks_gl *quirks = &self->pixelmap->screen->asFront.quirks;
    void               *raw_sampler;
    br_sampler_info_gl  *key;
    GLuint sampler;

    raw_sampler = BrHashMapFindByHash(self->sampler_pool, SamplerInfoGLHash(info));
    if(raw_sampler != NULL)
        return (GLuint)(br_uintptr_t)raw_sampler;

    glGenSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, info->wrap_s);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, info->wrap_t);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, info->filter_min);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, info->filter_mag);

    if(!quirks->disable_anisotropic_filtering) {
        if(GLAD_GL_ARB_texture_filter_anisotropic && info->filter_min != GL_NEAREST && info->filter_mag != GL_NEAREST) {
            glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, self->pixelmap->screen->asFront.video.maxAnisotropy);
        }
    }


    key = BrResAllocate(self->sampler_pool, sizeof(br_sampler_info_gl), BR_MEMORY_DRIVER);
    *key = *info;

    BrHashMapInsert(self->sampler_pool, key, (void*)(br_uintptr_t)sampler);

    return sampler;
}
