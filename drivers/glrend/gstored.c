/*
 * Geometry format for version 1 models
 */
#include <string.h>
#include "drv.h"
#include "state.h"
#include "brassert.h"
#include "formats.h"

/*
 * Default dispatch table for geometry type (defined at and of file)
 */
static const struct br_geometry_stored_dispatch geometryStoredDispatch;

/*
 * Geometry format info. template
 */
#define F(f) offsetof(br_geometry_stored, f)

static br_tv_template_entry templateEntries[] = {
    {BRT(IDENTIFIER_CSTR),   F(identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(GEOMETRY_V1_MODEL), F(gv1model),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(SHARED_B),          F(shared),     BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef F

static GLuint create_vao(const GladGLContext *gl, const br_gl_main_shader *shader, GLuint vbo, GLuint ibo)
{
    GLuint vao;
    gl->GenVertexArrays(1, &vao);
    gl->BindVertexArray(vao);

    gl->BindBuffer(GL_ARRAY_BUFFER, vbo);

    if(shader->attributes.aPosition >= 0) {
        gl->EnableVertexAttribArray(shader->attributes.aPosition);
        gl->VertexAttribPointer(shader->attributes.aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(gl_vertex_f), (void *)offsetof(gl_vertex_f, p));
    }

    if(shader->attributes.aUV >= 0) {
        gl->EnableVertexAttribArray(shader->attributes.aUV);
        gl->VertexAttribPointer(shader->attributes.aUV, 2, GL_FLOAT, GL_FALSE, sizeof(gl_vertex_f), (void *)offsetof(gl_vertex_f, map));
    }

    if(shader->attributes.aNormal >= 0) {
        gl->EnableVertexAttribArray(shader->attributes.aNormal);
        gl->VertexAttribPointer(shader->attributes.aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(gl_vertex_f), (void *)offsetof(gl_vertex_f, n));
    }

    if(shader->attributes.aColour >= 0) {
        gl->EnableVertexAttribArray(shader->attributes.aColour);
        gl->VertexAttribPointer(shader->attributes.aColour, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(gl_vertex_f), (void *)offsetof(gl_vertex_f, c));
    }

    gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl->BindVertexArray(0);
    return vao;
}

static GLuint build_vbo(const GladGLContext *gl, const struct v11model *model, size_t total_vertices)
{
    /* Collate and upload the vertex data. */
    gl_vertex_f *vtx     = (gl_vertex_f *)BrScratchAllocate(total_vertices * sizeof(gl_vertex_f));
    gl_vertex_f *nextVtx = vtx;
    GLuint       buf;

    for(br_uint_16 i = 0; i < model->ngroups; ++i) {
        const struct v11group *gp = model->groups + i;
        for(br_uint_16 v = 0; v < gp->nvertices; ++v, ++nextVtx) {
            nextVtx->p    = *(br_vector3_f *)(gp->position + v);
            nextVtx->map  = *(br_vector2_f *)(gp->map + v);
            nextVtx->n    = *(br_vector3_f *)(gp->normal + v);
            nextVtx->c[0] = BR_RED(gp->vertex_colours[v]);
            nextVtx->c[1] = BR_GRN(gp->vertex_colours[v]);
            nextVtx->c[2] = BR_BLU(gp->vertex_colours[v]);
            nextVtx->c[3] = 255;
        }
    }

    gl->GenBuffers(1, &buf);
    gl->BindBuffer(GL_ARRAY_BUFFER, buf);
    gl->BufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(total_vertices * sizeof(gl_vertex_f)), vtx, GL_STATIC_DRAW);
    BrScratchFree(vtx);
    return buf;
}

static GLuint build_ibo(const GladGLContext *gl, const struct v11model *model, size_t total_faces, gl_groupinfo *groups)
{
    br_uint_16 *idx = (br_uint_16 *)BrScratchAllocate(total_faces * 3 * sizeof(br_uint_16));
    GLuint      buf;

    br_uint_16 *nextIdx     = idx;
    br_uint_16  offset      = 0;
    br_size_t   face_offset = 0;

    for(br_uint_16 i = 0; i < model->ngroups; ++i) {
        const struct v11group *gp = model->groups + i;
        for(br_uint_16 f = 0; f < gp->nfaces; ++f) {
            const br_vector3_u16 *fp = gp->vertex_numbers + f;
            *nextIdx++               = fp->v[0] + offset;
            *nextIdx++               = fp->v[1] + offset;
            *nextIdx++               = fp->v[2] + offset;
        }

        groups[i].count  = (GLsizei)gp->nfaces * 3;
        groups[i].offset = (void *)(br_uintptr_t)face_offset;
        groups[i].group  = model->groups + i;

        face_offset += (br_size_t)gp->nfaces * 3 * sizeof(br_uint_16);

        offset += model->groups[i].nvertices;
    }

    gl->GenBuffers(1, &buf);
    gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
    gl->BufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)face_offset, idx, GL_STATIC_DRAW);
    BrScratchFree(idx);
    return buf;
}

br_geometry_stored *GeometryStoredGLAllocate(br_geometry_v1_model *gv1model, const char *id, br_renderer *r, struct v11model *model)
{
    size_t               total_vertices, total_faces;
    br_geometry_stored  *self;
    const GladGLContext *gl  = r->gl;
    br_gl_context_state *ctx = GLContextState(gl);

    self             = BrResAllocate(gv1model->renderer_facility->object_list, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch   = &geometryStoredDispatch;
    self->identifier = BrResSprintf(self, BR_GLREND_DEBUG_USER_PREFIX "%s", id);
    self->device     = gv1model->device;
    self->gv1model   = gv1model;
    self->gl         = gl;

    ObjectContainerAddFront(gv1model->renderer_facility, (br_object *)self);

    self->model  = model;
    self->shared = BR_TRUE;

    self->groups = BrResAllocate(gv1model, sizeof(gl_groupinfo) * model->ngroups, BR_MEMORY_OBJECT_DATA);

    total_vertices = 0;
    total_faces    = 0;
    for(br_uint_16 i = 0; i < model->ngroups; ++i) {
        total_vertices += model->groups[i].nvertices;
        total_faces += model->groups[i].nfaces;
    }

    gl->BindVertexArray(0);

    self->gl_vbo = build_vbo(gl, model, total_vertices);
    self->gl_ibo = build_ibo(gl, model, total_faces, self->groups);
    self->gl_vao = create_vao(gl, &ctx->main_shader, self->gl_vbo, self->gl_ibo);

    DeviceGLObjectLabelF(gl, GL_BUFFER, self->gl_vbo, "%s:vbo", self->identifier);
    DeviceGLObjectLabelF(gl, GL_BUFFER, self->gl_ibo, "%s:ibo", self->identifier);
    DeviceGLObjectLabelF(gl, GL_VERTEX_ARRAY, self->gl_vao, "%s:vao", self->identifier);

    gl->BindVertexArray(0);
    gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    gl->BindBuffer(GL_ARRAY_BUFFER, 0);
    return (br_geometry_stored *)self;
}

static void BR_CMETHOD(br_geometry_stored_gl, free)(br_object *_self)
{
    br_geometry_stored  *self = (br_geometry_stored *)_self;
    const GladGLContext *gl   = self->gl;

    ObjectContainerRemove(self->gv1model->renderer_facility, (br_object *)self);

    gl->DeleteVertexArrays(1, &self->gl_vao);
    gl->DeleteBuffers(1, &self->gl_vbo);
    gl->DeleteBuffers(1, &self->gl_ibo);
    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD(br_geometry_stored_gl, identifier)(br_object *self)
{
    return ((br_geometry_stored *)self)->identifier;
}

static br_device *BR_CMETHOD(br_geometry_stored_gl, device)(br_object *self)
{
    return ((br_geometry_stored *)self)->device;
}

static br_token BR_CMETHOD(br_geometry_stored_gl, type)(br_object *self)
{
    return BRT_GEOMETRY_STORED;
}

static br_boolean BR_CMETHOD(br_geometry_stored_gl, isType)(br_object *self, br_token t)
{
    return (t == BRT_GEOMETRY_STORED) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD(br_geometry_stored_gl, space)(br_object *self)
{
    return sizeof(br_geometry_stored);
}

static br_tv_template *BR_CMETHOD(br_geometry_stored_gl, templateQuery)(br_object *_self)
{
    br_geometry_stored *self = (br_geometry_stored *)_self;

    if(self->device->templates.geometryStoredTemplate == NULL) {
        self->device->templates.geometryStoredTemplate = BrTVTemplateAllocate(self->device, templateEntries, BR_ASIZE(templateEntries));
    }

    return self->device->templates.geometryStoredTemplate;
}

static br_primitive *heapPrimitiveAdd(br_primitive_heap *heap, br_token type)
{
    br_primitive *p = (br_primitive *)heap->current;

    ASSERT_MESSAGE("Out of heap space", p != NULL);

    p->type = type;

    heap->current += sizeof(br_primitive);
    return p;
}

enum {
    RM_FORCE_BACK  = 0,
    RM_OPAQUE      = 1,
    RM_TRANS       = 2,
    RM_FORCE_FRONT = 3,
    RM_MAX         = 4,
};

static int get_render_mode(const state_stack *state)
{
    if(state->valid & MASK_STATE_SURFACE) {
        if(state->surface.force_back)
            return RM_FORCE_BACK;

        if(state->surface.force_front)
            return RM_FORCE_FRONT;

        /* Transparent? Defer. */
        if(state->surface.opacity < BR_SCALAR(1.0f))
            return RM_TRANS;
    }

    if(state->valid & MASK_STATE_PRIMITIVE) {
        /* Blend flags set? Defer.*/
        if(state->prim.flags & PRIMF_BLEND)
            return RM_TRANS;

        /* Has a blend table? Defer. */
        if(state->prim.index_blend != NULL)
            return RM_TRANS;

        if(state->prim.colour_map && state->prim.colour_map->source) {
            /* Keyed transparency? Defer. */
            if(state->prim.colour_map->source->flags & BR_PMF_KEYED_TRANSPARENCY)
                return RM_TRANS;
        }
    }

    return RM_OPAQUE;
}

/*
 * Determine which bucket to dump things into. At least four are needed
 * for the various render types.
 *
 * | Type        | Ratio | DT | Order |
 * |-------------|-------|----|-------|
 * | Force back  |  5%   |    | BtF   |
 * | Opaque      | 80%   | X  | FtB   |
 * | Transparent | 10%   | X  | BtF   |
 * | Force front |  5%   |    | BtF   |
 *
 * DT  = Depth Tested
 * BtF = Back-to-front
 * TtB = Front-to-back
 */
static br_uint_16 calculate_bucket(const br_order_table *ot, const state_stack *state, br_scalar *depth)
{
    const br_scalar ratio_force_frontback = BR_SCALAR(0.05);
    const br_scalar ratio_transparent     = BR_SCALAR(0.10);
    br_uint_16      count_forced, count_opaque, count_trans;
    int             render_mode;
    br_scalar       ot_size;
    br_uint_16      base, count;
    br_scalar       tmp_depth;
    br_boolean      force_btf = (state->valid & MASK_STATE_OUTPUT) && state->output.depth == NULL;

    ASSERT(BR_ADD(BR_MUL(ratio_force_frontback, BR_SCALAR(2)), ratio_transparent) < BR_SCALAR(1.0));

    render_mode = get_render_mode(state);

    /*
     * Case 1 - A single bucket.
     * Force back/front geometry to the back/front and pray
     * for the best. ¯\_(ツ)_/¯
     */
    if(ot->size == 1) {
        if(render_mode == RM_FORCE_BACK)
            *depth = BR_SCALAR_MAX;
        else if(render_mode == RM_FORCE_FRONT)
            *depth = BR_SCALAR(0.0);
        return 0;
    }

    /*
     * Case 2 - 2 buckets.
     */
    if(ot->size == 2) {
        switch(render_mode) {
            case RM_FORCE_BACK:
                *depth = BR_SCALAR_MAX;
            case RM_OPAQUE:
            default:
                return 1;
            case RM_FORCE_FRONT:
                *depth = BR_SCALAR(0.0);
            case RM_TRANS:
                return 0;
        }
    }

    /*
     * Case 3 - 3 buckets.
     */
    if(ot->size == 3) {
        switch(render_mode) {
            case RM_FORCE_BACK:
                *depth = BR_SCALAR_MAX;
            case RM_OPAQUE:
            default:
                return 2;

            case RM_TRANS:
                return 1;
            case RM_FORCE_FRONT:
                return 0;
        }
    }

    /*
     * Case 4 - 4 buckets.
     */
    if(ot->size == 4) {
        switch(render_mode) {
            case RM_FORCE_BACK:
                return 3;
            case RM_OPAQUE:
            default:
                return 2;
            case RM_TRANS:
                return 1;
            case RM_FORCE_FRONT:
                return 0;
        }
    }

    /*
     * Case 5 - >4 buckets
     */
    ot_size      = BR_SCALAR(ot->size);
    count_forced = (br_uint_16)ceilf(ot_size * ratio_force_frontback);
    count_trans  = (br_uint_16)((float)(ot->size - (count_forced << 1)) - ceilf(ot_size * ratio_transparent));
    count_opaque = ot->size - count_trans - (count_forced << 1);

    ASSERT(count_forced + count_opaque + count_trans + count_forced == ot->size);

    tmp_depth = *depth;
    switch(render_mode) {
        case RM_FORCE_FRONT:
            base  = 0;
            count = count_forced;
            break;
        case RM_TRANS:
            base  = count_forced;
            count = count_opaque;
            break;
        case RM_OPAQUE:
        default:
            base  = count_forced + count_trans;
            count = count_opaque;

            if(!force_btf)
                tmp_depth = -tmp_depth;
            break;
        case RM_FORCE_BACK:
            base  = count_forced + count_trans + count_opaque;
            count = count_forced;
            break;
    }

    return base + BrZsPrimitiveBucketSelect(&tmp_depth, BR_PRIMITIVE_POINT, ot->min_z, ot->max_z, count, ot->type);
}

static br_boolean want_defer(const state_hidden *hidden)
{
    if(hidden->type != BRT_BUCKET_SORT)
        return BR_FALSE;

    if(hidden->divert == BRT_NONE)
        return BR_FALSE;

    if(hidden->divert == BRT_ALL)
        return BR_TRUE;

    UASSERT(hidden->divert == BRT_BLENDED);

    return hidden->order_table != NULL && hidden->heap != NULL;
}

static br_error V1Model_RenderStored(br_geometry_stored *self, br_renderer *renderer, br_boolean on_screen)
{
    br_primitive *prim;
    br_vector3    pos;
    br_boolean    defer;
    br_scalar     distance_from_zero;
    br_boolean    z_sorting;

    pos.v[0] = renderer->state.current->matrix.model_to_view.m[3][0];
    pos.v[1] = renderer->state.current->matrix.model_to_view.m[3][1];
    pos.v[2] = renderer->state.current->matrix.model_to_view.m[3][2];

    distance_from_zero = BrVector3Length(&pos);

    defer = want_defer(&renderer->state.current->hidden);

    z_sorting = (renderer->state.current->valid & MASK_STATE_OUTPUT) && renderer->state.current->output.depth == NULL;

    for(int i = 0; i < self->model->ngroups; ++i) {
        struct v11group          *group     = self->model->groups + i;
        gl_groupinfo             *groupinfo = self->groups + i;
        br_renderer_state_stored *stored    = group->stored;
        int                       render_mode;
        br_uint_16                bucket;
        state_stack               state;

        /*
         * If there's a stored state (i.e. a material), apply it to our current state.
         */
        state = *renderer->state.current;
        if(stored != NULL) {
            StateGLCopy(&state, &stored->state, MASK_STATE_STORED);
        }

        render_mode = get_render_mode(&state);

        if(!defer) {
            RendererGLRenderGroup(renderer, self, groupinfo);
            continue;
        }

        bucket = calculate_bucket(state.hidden.order_table, &state, &distance_from_zero);

        if(z_sorting || render_mode == RM_TRANS) {
            /*
             * If transparent or z-sorting, send things triangle-by-triangle.
             */
            state_stack *tmpstate = RendererGLAllocState(renderer, &state, group->nfaces);
            for(int f = 0; f < group->nfaces; ++f) {
                br_vector3 centroid, centroid_view;
                br_int_32  base;

                if((base = RendererGLNextImmTri(renderer, group, group->vertex_numbers[f])) < 0) {
                    /*
                     * Out of immediate primitives.
                     */
                    BrLogWarn("GLREND", "Out of transparent primitives.");
                    RendererGLUnrefState(renderer, tmpstate);
                    continue;
                }

                prim         = heapPrimitiveAdd(state.hidden.heap, BRT_TRIANGLE);
                prim->stored = stored;
                prim->v[0]   = (void *)(br_uintptr_t)base;
                prim->v[1]   = tmpstate;
                prim->v[2]   = groupinfo;

                centroid = DeviceGLTriangleCentroid(&renderer->trans.pool[base + 0].position, &renderer->trans.pool[base + 1].position,
                                                    &renderer->trans.pool[base + 2].position);
                BrMatrix34ApplyP(&centroid_view, &centroid, &state.matrix.model_to_view);

                prim->depth = -BrVector3Length(&centroid_view);

                /*
                 * If the user set a function defer to them.
                 */
                if(state.hidden.insert_fn != NULL) {
                    state.hidden.insert_fn(prim, state.hidden.insert_arg1, state.hidden.insert_arg2, state.hidden.insert_arg3,
                                           state.hidden.order_table, &prim->depth);
                    continue;
                }

                BrZsOrderTablePrimitiveInsert(state.hidden.order_table, prim, bucket);
            }
        } else {
            /*
             * Render everything else by group.
             */
            prim         = heapPrimitiveAdd(state.hidden.heap, BRT_GEOMETRY_STORED);
            prim->stored = stored;
            prim->v[0]   = self;
            prim->v[1]   = RendererGLAllocState(renderer, &state, 1);
            prim->v[2]   = groupinfo;
            prim->depth  = distance_from_zero;

            /*
             * If the user set a function defer to them.
             */
            if(state.hidden.insert_fn != NULL) {
                state.hidden.insert_fn(prim, state.hidden.insert_arg1, state.hidden.insert_arg2, state.hidden.insert_arg3,
                                       state.hidden.order_table, &prim->depth);
                continue;
            }

            BrZsOrderTablePrimitiveInsert(state.hidden.order_table, prim, bucket);
        }
    }
    return BRE_OK;
}

static br_error BR_CMETHOD(br_geometry_stored_gl, render)(br_geometry_stored *self, br_renderer *renderer)
{
    return V1Model_RenderStored(self, renderer, BR_FALSE);
}

static br_error BR_CMETHOD(br_geometry_stored_gl, renderOnScreen)(br_geometry_stored *self, br_renderer *renderer)
{
    return V1Model_RenderStored(self, renderer, BR_TRUE);
}

static const struct br_geometry_stored_dispatch geometryStoredDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD(br_geometry_stored_gl, free),
    ._identifier = BR_CMETHOD(br_geometry_stored_gl, identifier),
    ._type       = BR_CMETHOD(br_geometry_stored_gl, type),
    ._isType     = BR_CMETHOD(br_geometry_stored_gl, isType),
    ._device     = BR_CMETHOD(br_geometry_stored_gl, device),
    ._space      = BR_CMETHOD(br_geometry_stored_gl, space),

    ._templateQuery = BR_CMETHOD(br_geometry_stored_gl, templateQuery),
    ._query         = BR_CMETHOD(br_object, query),
    ._queryBuffer   = BR_CMETHOD(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD(br_object, queryMany),
    ._queryManySize = BR_CMETHOD(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD(br_object, queryAllSize),

    ._render         = BR_CMETHOD(br_geometry_stored_gl, render),
    ._renderOnScreen = BR_CMETHOD(br_geometry_stored_gl, renderOnScreen),
};
