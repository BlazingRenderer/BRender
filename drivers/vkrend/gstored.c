/*
 * Geometry format for a version 1 model's stored GPU buffers.
 *
 * storedNew() collates the prepared model's groups into one vertex buffer (position,
 * normal, map, colour) and one index buffer (u16, one shared pool with a per-group offset),
 * device-scoped and owned by this object. render() follows glrend: for a
 * bucket-sorted hidden surface it defers a (geometry_stored, cached state, group) triple
 * into the primitive heap and inserts it into the order table, leaving the actual draw to
 * the bucket geometry at scene end. When the hidden surface is a Z-sort (no depth buffer)
 * or the surface is transparent it splits the group into one BRT_TRIANGLE per face, each
 * carrying its centroid's view distance as its depth, so the order table can order the
 * faces back-to-front - the same split glrend's gstored.c makes.
 */
#include "drv.h"
#include "state.h"
#include "brassert.h"
#include "formats.h"

#include <math.h>

/*
 * Default dispatch table for geometry type (defined at end of file)
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

/*
 * A buffer created at build time and never rewritten, so host-visible device memory is
 * enough: the mapped write plus flush makes it available to the device before the frame
 * that reads it is submitted.
 */
static VkBuffer BuildBuffer(br_device *dev, const void *data, br_size_t size, VkBufferUsageFlags usage, VmaAllocation *out_allocation)
{
    VkBuffer      buffer;
    VmaAllocation allocation;
    VkResult      res;

    if(size == 0) {
        *out_allocation = NULL;
        return VK_NULL_HANDLE;
    }

    const VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = size,
        .usage       = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo alloc_info = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    };

    if((res = vmaCreateBuffer(dev->vma_allocator, &buffer_info, &alloc_info, &buffer, &allocation, NULL)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create a geometry buffer of %zu bytes: VkResult = %d", (size_t)size, res);
        *out_allocation = NULL;
        return VK_NULL_HANDLE;
    }

    VmaAllocationInfo alloc_info_out;
    vmaGetAllocationInfo(dev->vma_allocator, allocation, &alloc_info_out);
    BrMemCpy(alloc_info_out.pMappedData, data, size);
    vmaFlushAllocation(dev->vma_allocator, allocation, 0, size);

    *out_allocation = allocation;
    return buffer;
}

static VkBuffer BuildVertexBuffer(br_device *dev, const struct v11model *model, br_size_t total_vertices, br_geometry_stored_group *groups,
                                  VmaAllocation *out_allocation)
{
    br_geometry_stored_vertex *vtx      = BrScratchAllocate(total_vertices * sizeof(*vtx));
    br_geometry_stored_vertex *next_vtx = vtx;

    (void)groups;

    for(br_uint_16 i = 0; i < model->ngroups; ++i) {
        const struct v11group *gp = model->groups + i;

        for(br_uint_16 v = 0; v < gp->nvertices; ++v, ++next_vtx) {
            next_vtx->position = *(br_vector3_f *)(gp->position + v);
            next_vtx->normal   = *(br_vector3_f *)(gp->normal + v);
            next_vtx->map      = *(br_vector2_f *)(gp->map + v);

            next_vtx->colour.v[0] = BR_RED(gp->vertex_colours[v]) / 255.0f;
            next_vtx->colour.v[1] = BR_GRN(gp->vertex_colours[v]) / 255.0f;
            next_vtx->colour.v[2] = BR_BLU(gp->vertex_colours[v]) / 255.0f;
            next_vtx->colour.v[3] = BR_ALPHA(gp->vertex_colours[v]) / 255.0f;
        }
    }

    VkBuffer buffer = BuildBuffer(dev, vtx, total_vertices * sizeof(*vtx), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, out_allocation);

    BrScratchFree(vtx);
    return buffer;
}

static VkBuffer BuildIndexBuffer(br_device *dev, const struct v11model *model, br_size_t total_faces, br_geometry_stored_group *groups,
                                 VmaAllocation *out_allocation)
{
    br_uint_16 *idx         = BrScratchAllocate(total_faces * 3 * sizeof(*idx));
    br_uint_16 *next_idx    = idx;
    br_uint_16  offset      = 0;
    br_size_t   face_offset = 0;

    for(br_uint_16 i = 0; i < model->ngroups; ++i) {
        const struct v11group *gp = model->groups + i;

        for(br_uint_16 f = 0; f < gp->nfaces; ++f) {
            const br_vector3_u16 *fp = gp->vertex_numbers + f;
            *next_idx++              = fp->v[0] + offset;
            *next_idx++              = fp->v[1] + offset;
            *next_idx++              = fp->v[2] + offset;
        }

        groups[i].count  = (br_uint_32)gp->nfaces * 3;
        groups[i].offset = (br_uint_32)face_offset;

        face_offset += (br_size_t)gp->nfaces * 3 * sizeof(br_uint_16);
        offset += model->groups[i].nvertices;
    }

    VkBuffer buffer = BuildBuffer(dev, idx, (br_size_t)total_faces * 3 * sizeof(*idx), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, out_allocation);

    BrScratchFree(idx);
    return buffer;
}

br_geometry_stored *GeometryStoredVkAllocate(br_geometry_v1_model *gv1model, const char *id, br_renderer *r, struct v11model *model)
{
    br_geometry_stored *self;
    br_size_t           total_vertices, total_faces;

    (void)r;

    self             = BrResAllocate(gv1model->renderer_facility, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch   = &geometryStoredDispatch;
    self->identifier = BrResSprintf(self, "VKREND:%s", id);
    self->device     = gv1model->device;
    self->gv1model   = gv1model;

    self->model  = model;
    self->shared = BR_TRUE;

    self->groups = BrResAllocate(self, sizeof(br_geometry_stored_group) * model->ngroups, BR_MEMORY_OBJECT_DATA);

    total_vertices = 0;
    total_faces    = 0;
    for(br_uint_16 i = 0; i < model->ngroups; ++i) {
        total_vertices += model->groups[i].nvertices;
        total_faces += model->groups[i].nfaces;
    }

    self->vertex_buffer = BuildVertexBuffer(self->device, model, total_vertices, self->groups, &self->vertex_allocation);
    self->index_buffer  = BuildIndexBuffer(self->device, model, total_faces, self->groups, &self->index_allocation);

    ObjectContainerAddFront(gv1model->renderer_facility, (br_object *)self);
    return self;
}

static void BR_CMETHOD_DECL(br_geometry_stored_vk, free)(br_object *_self)
{
    br_geometry_stored *self   = (br_geometry_stored *)_self;
    br_device_pixelmap *screen = self->device->screen;

    ObjectContainerRemove(self->gv1model->renderer_facility, (br_object *)self);

    /*
     * The model's buffers may still be referenced by the front screen's current command
     * buffer, recorded but not yet submitted: a mid-frame BrModelClear()/BrModelUpdate() or
     * a renderer swap frees this object, and gv1buckt.c binds the buffers with
     * vkCmdBindVertexBuffers()/vkCmdBindIndexBuffer(). Retire them against the frame that
     * would read them instead of destroying them in place. With no front screen (teardown,
     * after the queue has been idled) direct destruction is safe.
     */
    if(screen != NULL) {
        br_device_pixelmap_vk_frame *frame = DevicePixelmapVkGetCurrentFrame(screen);

        if(self->vertex_buffer != VK_NULL_HANDLE)
            DevicePixelmapVkRetireBuffer(frame, self->vertex_buffer, self->vertex_allocation);

        if(self->index_buffer != VK_NULL_HANDLE)
            DevicePixelmapVkRetireBuffer(frame, self->index_buffer, self->index_allocation);
    } else {
        if(self->vertex_buffer != VK_NULL_HANDLE)
            vmaDestroyBuffer(self->device->vma_allocator, self->vertex_buffer, self->vertex_allocation);

        if(self->index_buffer != VK_NULL_HANDLE)
            vmaDestroyBuffer(self->device->vma_allocator, self->index_buffer, self->index_allocation);
    }

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_geometry_stored_vk, identifier)(br_object *self)
{
    return ((br_geometry_stored *)self)->identifier;
}

static br_device *BR_CMETHOD_DECL(br_geometry_stored_vk, device)(br_object *self)
{
    return ((br_geometry_stored *)self)->device;
}

static br_token BR_CMETHOD_DECL(br_geometry_stored_vk, type)(br_object *self)
{
    (void)self;
    return BRT_GEOMETRY_STORED;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_stored_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_GEOMETRY_STORED) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_geometry_stored_vk, space)(br_object *self)
{
    (void)self;
    return sizeof(br_geometry_stored);
}

static br_tv_template *BR_CMETHOD_DECL(br_geometry_stored_vk, templateQuery)(br_object *_self)
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
    }

    return RM_OPAQUE;
}

/*
 * Determine which bucket to dump things into, from glrend's gstored.c. At least four are
 * needed for the various render types. Bucket 0 is the force-front one and the last is
 * force-back; render() walks the table from the last bucket to the first.
 *
 * | Type        | Ratio | DT | Order |
 * |-------------|-------|----|-------|
 * | Force front |  5%   |    | BtF   |
 * | Transparent | ~80%  | X  | BtF   |
 * | Opaque      | 10%   | X  | FtB   |
 * | Force back  |  5%   |    | BtF   |
 *
 * The ratios are the ones calculate_bucket() allocates below: each forced bucket takes
 * ceil(5%) of the table, opaque takes ceil(10%) and the transparent bucket takes whatever
 * is left, which is ~80% at any size.
 *
 * DT  = Depth Tested
 * BtF = Back-to-front
 * FtB = Front-to-back
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

    if(ot->size == 1) {
        if(render_mode == RM_FORCE_BACK)
            *depth = BR_SCALAR_MAX;
        else if(render_mode == RM_FORCE_FRONT)
            *depth = BR_SCALAR(0.0);
        return 0;
    }

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

static br_error V1Model_RenderStored(br_geometry_stored *self, br_renderer *renderer, br_token type, br_boolean on_screen)
{
    br_primitive *prim;
    br_vector3    pos;
    br_boolean    defer;
    br_boolean    z_sorting;
    br_scalar     distance_from_zero;

    (void)on_screen;

    if(type != BRT_TRIANGLE && type != BRT_POINT)
        return BRE_FAIL;

    pos.v[0] = renderer->state.current->matrix.model_to_view.m[3][0];
    pos.v[1] = renderer->state.current->matrix.model_to_view.m[3][1];
    pos.v[2] = renderer->state.current->matrix.model_to_view.m[3][2];

    distance_from_zero = BrVector3Length(&pos);

    defer = want_defer(&renderer->state.current->hidden);

    /*
     * glrend's z_sorting: with no depth buffer the order table, not the Z-buffer, resolves
     * occlusion. It is only consulted once deferral has begun (below), so an immediate draw
     * is unaffected.
     */
    z_sorting = (renderer->state.current->valid & MASK_STATE_OUTPUT) && renderer->state.current->output.depth == NULL;

    for(int i = 0; i < self->model->ngroups; ++i) {
        struct v11group          *group     = self->model->groups + i;
        br_geometry_stored_group *groupinfo = self->groups + i;
        br_renderer_state_stored *stored    = group->stored;
        int                       render_mode;
        br_uint_16                bucket;
        state_stack               state;

        /*
         * If there's a stored state (i.e. a material), apply it to our current state when
         * computing the bucket. BrDbModelRender() has already restored the material into
         * the current state, so the immediate path draws with that directly.
         */
        renderer->state.current->render_type = type;
        state                                = *renderer->state.current;
        if(stored != NULL)
            StateVkCopy(&state, &stored->state, MASK_STATE_STORED);

        if(!defer) {
            /*
             * No hidden-surface deferral: draw the group immediately, with the current
             * state. Zs always defers, so this is only reached by a direct renderer call.
             */
            VkV1ModelRenderGroup(self, renderer, groupinfo);
            continue;
        }

        render_mode = get_render_mode(&state);
        bucket      = calculate_bucket(state.hidden.order_table, &state, &distance_from_zero);

        if((z_sorting || render_mode == RM_TRANS) && group->nfaces > 0) {
            /*
             * Z-sorting or transparent: one primitive per face, as glrend does. The faces
             * of one group share a state snapshot; the last one drawn releases it. v[2]
             * carries the face's first index in the shared IBO, from which the bucket walk
             * draws its three vertices.
             */
            state_stack *faces_state = RendererVkAllocState(renderer, &state, group->nfaces);

            for(br_uint_16 f = 0; f < group->nfaces; ++f) {
                const br_vector3_u16 *fp = group->vertex_numbers + f;
                br_vector3            centroid, centroid_view;

                centroid.v[0] = BR_DIV(group->position[fp->v[0]].v[0] + group->position[fp->v[1]].v[0] + group->position[fp->v[2]].v[0],
                                       BR_SCALAR(3.0));
                centroid.v[1] = BR_DIV(group->position[fp->v[0]].v[1] + group->position[fp->v[1]].v[1] + group->position[fp->v[2]].v[1],
                                       BR_SCALAR(3.0));
                centroid.v[2] = BR_DIV(group->position[fp->v[0]].v[2] + group->position[fp->v[1]].v[2] + group->position[fp->v[2]].v[2],
                                       BR_SCALAR(3.0));

                BrMatrix34ApplyP(&centroid_view, &centroid, &state.matrix.model_to_view);

                prim         = heapPrimitiveAdd(state.hidden.heap, BRT_TRIANGLE);
                prim->stored = stored;
                prim->v[0]   = self;
                prim->v[1]   = faces_state;
                prim->v[2]   = (void *)(br_uintptr_t)(groupinfo->offset / sizeof(br_uint_16) + (br_uint_32)f * 3);
                prim->depth  = -BrVector3Length(&centroid_view);

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
            continue;
        }

        /*
         * Everything else by group.
         */
        prim         = heapPrimitiveAdd(state.hidden.heap, BRT_GEOMETRY_STORED);
        prim->stored = stored;
        prim->v[0]   = self;
        prim->v[1]   = RendererVkAllocState(renderer, &state, 1);
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
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_geometry_stored_vk, render)(br_geometry_stored *self, br_renderer *renderer, br_token type)
{
    return V1Model_RenderStored(self, renderer, type, BR_FALSE);
}

static br_error BR_CMETHOD_DECL(br_geometry_stored_vk, renderOnScreen)(br_geometry_stored *self, br_renderer *renderer, br_token type)
{
    return V1Model_RenderStored(self, renderer, type, BR_TRUE);
}

static const struct br_geometry_stored_dispatch geometryStoredDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD(br_geometry_stored_vk, free),
    ._identifier = BR_CMETHOD(br_geometry_stored_vk, identifier),
    ._type       = BR_CMETHOD(br_geometry_stored_vk, type),
    ._isType     = BR_CMETHOD(br_geometry_stored_vk, isType),
    ._device     = BR_CMETHOD(br_geometry_stored_vk, device),
    ._space      = BR_CMETHOD(br_geometry_stored_vk, space),

    ._templateQuery = BR_CMETHOD(br_geometry_stored_vk, templateQuery),
    ._query         = BR_CMETHOD(br_object, query),
    ._queryBuffer   = BR_CMETHOD(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD(br_object, queryMany),
    ._queryManySize = BR_CMETHOD(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD(br_object, queryAllSize),

    ._render         = BR_CMETHOD(br_geometry_stored_vk, render),
    ._renderOnScreen = BR_CMETHOD(br_geometry_stored_vk, renderOnScreen),
};
