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

static struct br_tv_template_entry templateEntries[] = {
    {BRT(IDENTIFIER_CSTR),   F(identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(GEOMETRY_V1_MODEL), F(gv1model),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(SHARED_B),          F(shared),     BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef F

static GLuint create_vao(HVIDEO hVideo, GLuint vbo_posn, GLuint vbo, GLuint ibo)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* Separate buffer for positions. Makes it easier on tiling (i.e. mobile) GPUs. */
    glBindBuffer(GL_ARRAY_BUFFER, vbo_posn);
    glEnableVertexAttribArray(hVideo->brenderProgram.attributes.aPosition);
    glVertexAttribPointer(hVideo->brenderProgram.attributes.aPosition, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    if(hVideo->brenderProgram.attributes.aUV >= 0) {
        glEnableVertexAttribArray(hVideo->brenderProgram.attributes.aUV);
        glVertexAttribPointer(hVideo->brenderProgram.attributes.aUV, 2, GL_FLOAT, GL_FALSE, sizeof(gl_vertex_f),
                              (void *)offsetof(gl_vertex_f, map));
    }

    if(hVideo->brenderProgram.attributes.aNormal >= 0) {
        glEnableVertexAttribArray(hVideo->brenderProgram.attributes.aNormal);
        glVertexAttribPointer(hVideo->brenderProgram.attributes.aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(gl_vertex_f),
                              (void *)offsetof(gl_vertex_f, n));
    }

    if(hVideo->brenderProgram.attributes.aColour >= 0) {
        glVertexAttribPointer(hVideo->brenderProgram.attributes.aColour, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                              sizeof(gl_vertex_f), (void *)offsetof(gl_vertex_f, c));
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindVertexArray(0);
    return vao;
}

static GLuint build_vbo_posn(const struct v11model *model, size_t total_vertices)
{
    br_vector3_f *vtx  = BrScratchAllocate(total_vertices * sizeof(br_vector3_f));
    br_vector3_f *next = vtx;
    GLuint        buf;

    for(br_uint_16 i = 0; i < model->ngroups; ++i) {
        const struct v11group *gp = model->groups + i;
        memcpy(next, gp->position, gp->nvertices * sizeof(br_vector3_f));
        next += gp->nvertices;
    }

    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(total_vertices * sizeof(br_vector3_f)), vtx, GL_STATIC_DRAW);
    BrScratchFree(vtx);
    return buf;
}

static GLuint build_vbo(const struct v11model *model, size_t total_vertices)
{
    /* Collate and upload the vertex data. */
    gl_vertex_f *vtx     = (gl_vertex_f *)BrScratchAllocate(total_vertices * sizeof(gl_vertex_f));
    gl_vertex_f *nextVtx = vtx;
    GLuint       buf;

    for(br_uint_16 i = 0; i < model->ngroups; ++i) {
        const struct v11group *gp = model->groups + i;
        for(br_uint_16 v = 0; v < gp->nvertices; ++v, ++nextVtx) {
            nextVtx->map = *(br_vector2_f *)(gp->map + v);
            nextVtx->n   = *(br_vector3_f *)(gp->normal + v);
            nextVtx->c   = gp->vertex_colours[v];
        }
    }

    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(total_vertices * sizeof(gl_vertex_f)), vtx, GL_STATIC_DRAW);
    BrScratchFree(vtx);
    return buf;
}

static GLuint build_ibo(const struct v11model *model, size_t total_faces, gl_groupinfo *groups)
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

    glGenBuffers(1, &buf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)face_offset, idx, GL_STATIC_DRAW);
    BrScratchFree(idx);
    return buf;
}

br_geometry_stored *GeometryStoredGLAllocate(br_geometry_v1_model *gv1model, const char *id, br_renderer *r, struct v11model *model)
{
    size_t              total_vertices, total_faces;
    br_geometry_stored *self;

    self             = BrResAllocate(gv1model->renderer_facility->object_list, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch   = &geometryStoredDispatch;
    self->identifier = BrResSprintf(self, BR_GLREND_DEBUG_USER_PREFIX "%s", id);
    self->device     = gv1model->device;
    self->gv1model   = gv1model;

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

    glBindVertexArray(0);

    self->gl_vbo_posn = build_vbo_posn(model, total_vertices);
    self->gl_vbo      = build_vbo(model, total_vertices);
    self->gl_ibo      = build_ibo(model, total_faces, self->groups);
    self->gl_vao      = create_vao(&r->pixelmap->screen->asFront.video, self->gl_vbo_posn, self->gl_vbo, self->gl_ibo);

    DeviceGLObjectLabelF(GL_BUFFER, self->gl_vbo_posn, "%s:vbo:posn", self->identifier);
    DeviceGLObjectLabelF(GL_BUFFER, self->gl_vbo, "%s:vbo", self->identifier);
    DeviceGLObjectLabelF(GL_BUFFER, self->gl_ibo, "%s:ibo", self->identifier);
    DeviceGLObjectLabelF(GL_VERTEX_ARRAY, self->gl_vao, "%s:vao", self->identifier);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return (br_geometry_stored *)self;
}

static void BR_CMETHOD(br_geometry_stored_gl, free)(br_object *_self)
{
    br_geometry_stored *self = (br_geometry_stored *)_self;

    ObjectContainerRemove(self->gv1model->renderer_facility, (br_object *)self);

    glDeleteVertexArrays(1, &self->gl_vao);
    glDeleteBuffers(1, &self->gl_vbo_posn);
    glDeleteBuffers(1, &self->gl_vbo);
    glDeleteBuffers(1, &self->gl_ibo);
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

static struct br_tv_template *BR_CMETHOD(br_geometry_stored_gl, templateQuery)(br_object *_self)
{
    br_geometry_stored *self = (br_geometry_stored *)_self;

    if(self->device->templates.geometryStoredTemplate == NULL) {
        self->device->templates.geometryStoredTemplate = BrTVTemplateAllocate(self->device, templateEntries,
                                                                              BR_ASIZE(templateEntries));
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

static br_error V1Model_RenderStored(struct br_geometry_stored *self, br_renderer *renderer, br_boolean on_screen)
{
    state_stack *state;
    br_primitive  *prim;
    br_vector3     pos;
    br_boolean     defer;
    br_scalar      distance_from_zero;

    state = renderer->state.current;

    BrVector3Set(&pos, state->matrix.model_to_view.m[3][0], state->matrix.model_to_view.m[3][1],
                 state->matrix.model_to_view.m[3][2]);
    distance_from_zero = BrVector3Length(&pos);

    defer = want_defer(&state->hidden);

    for(int i = 0; i < self->model->ngroups; ++i) {
        struct v11group          *group     = self->model->groups + i;
        gl_groupinfo             *groupinfo = self->groups + i;
        br_renderer_state_stored *stored    = (br_renderer_state_stored *)group->stored;

        /*
         * TODO: I *think* the group's material may change in between here and build_ibo()
         *  so always set this here.
         */
        groupinfo->stored = stored;

        if(!defer) {
            StoredGLRenderGroup(self, renderer, groupinfo);
            continue;
        }

        br_order_table *ot = state->hidden.order_table;
        br_uint_16      bucket;
        state_stack    *tmpstate;

        tmpstate = BrPoolBlockAllocate(renderer->state_pool);

        prim         = heapPrimitiveAdd(state->hidden.heap, BRT_GEOMETRY_STORED);
        prim->stored = stored;
        prim->v[0]   = self;
        prim->v[1]   = tmpstate;
        prim->v[2]   = groupinfo;

        *tmpstate = *state;

        /*
         * If the user set a function defer to them.
         */
        if(state->hidden.insert_fn != NULL) {
            state->hidden.insert_fn(prim, state->hidden.insert_arg1, state->hidden.insert_arg2,
                                    state->hidden.insert_arg3, ot, &prim->depth);
            continue;
        }

        bucket      = calculate_bucket(ot, stored ? &stored->state : state, &distance_from_zero);
        prim->depth = distance_from_zero;
        BrZsOrderTablePrimitiveInsert(ot, prim, bucket);
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
