/*
 * Renderer methods.
 *
 * The renderer is mostly state storage, ported from glrend's renderer.c with the GL
 * specifics dropped: every partSet/partQuery, the matrix helpers and the state stack only
 * touch the state_all hanging off the renderer. Only sceneBegin() and sceneEnd() do device
 * work, opening and closing a dynamic rendering scope on the offscreen named by the OUTPUT
 * state. Everything recorded between them (the bucket geometry's draws) goes into the front
 * screen's current frame command buffer and is submitted by the readback's flush, not here.
 */
#include "drv.h"
#include "brassert.h"
#include "shortcut.h"

/*
 * Default dispatch table for renderer (defined at end of file)
 */
static const struct br_renderer_dispatch rendererDispatch;

#define F(f) offsetof(struct br_renderer, f)

static br_tv_template_entry rendererTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),              F(identifier),                     BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(FACE_GROUP_COUNT_U32),         F(stats.face_group_count),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(TRIANGLES_DRAWN_COUNT_U32),    F(stats.triangles_drawn_count),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(TRIANGLES_RENDERED_COUNT_U32), F(stats.triangles_rendered_count), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(VERTICES_RENDERED_COUNT_U32),  F(stats.vertices_rendered_count),  BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(OPAQUE_DRAW_COUNT_U32),        F(stats.opaque_draw_count),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(TRANSPARENT_DRAW_COUNT_U32),   F(stats.transparent_draw_count),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef F

/*
 * Check a bounding box against the view volume, from glrend's onscreen.c. Returns
 * BRT_ACCEPT (completely on screen), BRT_PARTIAL or BRT_REJECT.
 *
 * It transforms the six planes bounding the view volume into model space through the
 * model-to-screen transform; because the view plane equations are only zeros and ones,
 * each transformed equation is an add/sub of rows of the transform. Testing the box only
 * needs the two vertices on the diagonal the plane's normal picks out.
 */
#define MAKE_EQN(sign, row)                                                     \
    {                                                                           \
        eqn.v[X] = model_to_screen->m[0][3] sign model_to_screen->m[0][row];    \
        eqn.v[Y] = model_to_screen->m[1][3] sign model_to_screen->m[1][row];    \
        eqn.v[Z] = model_to_screen->m[2][3] sign model_to_screen->m[2][row];    \
        eqn.v[W] = -(model_to_screen->m[3][3] sign model_to_screen->m[3][row]); \
    }

#define TEST_NOT_IN                                                                          \
    (BR_MAC3(eqn.v[X], ((eqn.v[X] > 0) ? (bounds->min.v[X]) : (bounds->max.v[X])), eqn.v[Y], \
             ((eqn.v[Y] > 0) ? (bounds->min.v[Y]) : (bounds->max.v[Y])), eqn.v[Z],           \
             ((eqn.v[Z] > 0) ? (bounds->min.v[Z]) : (bounds->max.v[Z]))) < eqn.v[W])

#define TEST_OUT                                                                             \
    (BR_MAC3(eqn.v[X], ((eqn.v[X] > 0) ? (bounds->max.v[X]) : (bounds->min.v[X])), eqn.v[Y], \
             ((eqn.v[Y] > 0) ? (bounds->max.v[Y]) : (bounds->min.v[Y])), eqn.v[Z],           \
             ((eqn.v[Z] > 0) ? (bounds->max.v[Z]) : (bounds->min.v[Z]))) < eqn.v[W])

static br_token VkOnScreenCheck(const br_matrix4 *model_to_screen, const br_bounds3 *bounds)
{
    int        accept = 1;
    br_vector4 eqn;

    MAKE_EQN(+, 0);
    if(TEST_OUT)
        return BRT_REJECT;
    if(TEST_NOT_IN)
        accept = 0;

    MAKE_EQN(-, 0);
    if(TEST_OUT)
        return BRT_REJECT;
    if(accept && TEST_NOT_IN)
        accept = 0;

    MAKE_EQN(+, 1);
    if(TEST_OUT)
        return BRT_REJECT;
    if(accept && TEST_NOT_IN)
        accept = 0;

    MAKE_EQN(-, 1);
    if(TEST_OUT)
        return BRT_REJECT;
    if(accept && TEST_NOT_IN)
        accept = 0;

    MAKE_EQN(+, 2);
    if(TEST_OUT)
        return BRT_REJECT;
    if(accept && TEST_NOT_IN)
        accept = 0;

    MAKE_EQN(-, 2);
    if(TEST_OUT)
        return BRT_REJECT;
    if(accept && TEST_NOT_IN)
        accept = 0;

    return accept ? BRT_ACCEPT : BRT_PARTIAL;
}

/*
 * Create a new renderer
 */
br_renderer *RendererVkAllocate(br_device *device, br_renderer_facility *facility, br_device_pixelmap *dest)
{
    br_renderer *self;

    ASSERT(dest != NULL && ObjectDevice(dest) == device);

    /*
     * The destination must resolve to a vkrend offscreen. A sub-pixelmap qualifies: its
     * as_sub member aliases as_back, so the image belongs to its owner, and every site that
     * needs it resolves the owner through DevicePixelmapVkResolve() (sceneBegin() and
     * RendererVkEndScope()).
     */
    if(!DevicePixelmapVkIsOffscreen(dest))
        return NULL;

    self                    = BrResAllocate(facility, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch          = &rendererDispatch;
    self->identifier        = facility->identifier;
    self->device            = device;
    self->object_list       = BrObjectListAllocate(self);
    self->pixelmap          = dest;
    self->renderer_facility = facility;
    self->state_pool        = BrPoolAllocate(sizeof(state_stack), 1024, BR_MEMORY_OBJECT_DATA);
    self->has_begun         = BR_FALSE;
    self->scene_dst         = NULL;
    self->scene_depth       = NULL;
    self->scene_buffer      = VK_NULL_HANDLE;
    self->scene_light_count = 0;

    self->material_buffer     = VK_NULL_HANDLE;
    self->material_allocation = VK_NULL_HANDLE;
    self->material_mapped     = NULL;
    self->material_count      = 0;
    self->material_capacity   = 0;

    ObjectContainerAddFront(facility, (br_object *)self);

    StateVkInit(&self->state, self->device);

    /*
     * State starts out as default
     */
    RendererStateDefault(self, BR_STATE_ALL);

    return self;
}

/*
 * Snapshot the current state for a deferred primitive. The pool is emptied at scene end,
 * after the bucket geometry has consumed every snapshot.
 */
state_stack *RendererVkAllocState(br_renderer *self, const state_stack *tpl, br_uint_32 refs)
{
    state_stack *state = BrPoolBlockAllocate(self->state_pool);
    *state             = *tpl;
    state->num_refs    = refs;
    return state;
}

void RendererVkUnrefState(br_renderer *self, state_stack *state)
{
    --state->num_refs;

    if(state->num_refs == 0)
        BrPoolBlockFree(self->state_pool, state);
}

/*
 * Gather a light's cutoff volume into the scene's shared region and plane tables, and
 * record where it landed on the light's own record, as glrend's PackLightVolume()
 * (drivers/glrend/cache.c) does.
 *
 * The location rides in the light record rather than being derived from the light's index:
 * VkRenderPackSceneLights() packs the lights in part order but skips the non-radial ambient
 * ones, so a light's index in the packed array is not its index in the model block.
 *
 * out_region/out_plane are the running fill positions in the two shared tables. A volume
 * that does not fit them is dropped whole - a partial volume would light the wrong region.
 */
static void VkRenderPackLightVolume(br_vk_scene_state *scene, const state_light_volume_desc *desc, const state_light_volume *volume,
                                    br_vk_light_state *out, br_uint_32 *out_region, br_uint_32 *out_plane)
{
    static br_boolean warned_overflow = BR_FALSE;
    br_uint_32        num_planes      = 0;

    out->volume[0] = 0.0f;
    out->volume[1] = 0.0f;
    out->volume[2] = 0.0f;
    out->volume[3] = 0.0f;

    if(desc->region_count == 0)
        return;

    for(br_uint_32 i = 0; i < desc->region_count; ++i)
        num_planes += volume->regions[i].plane_count;

    if(*out_region + desc->region_count > BR_VKREND_MAX_LIGHT_VOLUME_REGIONS || *out_plane + num_planes > BR_VKREND_MAX_LIGHT_VOLUME_PLANES) {

        if(!warned_overflow) {
            BrLogWarn("VKREND", "Scene has more light-volume regions or planes than the %d/%d the shader supports; dropping the rest.",
                      BR_VKREND_MAX_LIGHT_VOLUME_REGIONS, BR_VKREND_MAX_LIGHT_VOLUME_PLANES);
            BrLogWarn("VKREND", "This message will not be shown again.");
            warned_overflow = BR_TRUE;
        }

        return;
    }

    out->volume[0] = (float)*out_region;
    out->volume[1] = (float)desc->region_count;
    out->volume[2] = (float)desc->falloff_distance;
    out->volume[3] = 0.0f;

    for(br_uint_32 i = 0; i < desc->region_count; ++i) {
        const state_light_volume_region *region  = volume->regions + i;
        float                           *out_reg = scene->light_volume_regions[*out_region + i];

        out_reg[0] = (float)*out_plane;
        out_reg[1] = (float)region->plane_count;
        out_reg[2] = 0.0f;
        out_reg[3] = 0.0f;

        for(br_uint_32 j = 0; j < region->plane_count; ++j) {
            const br_vector4 *plane = volume->planes + region->plane_offset + j;

            scene->light_volume_planes[*out_plane + j][0] = (float)plane->v[0];
            scene->light_volume_planes[*out_plane + j][1] = (float)plane->v[1];
            scene->light_volume_planes[*out_plane + j][2] = (float)plane->v[2];
            scene->light_volume_planes[*out_plane + j][3] = (float)plane->v[3];
        }

        *out_plane += region->plane_count;
    }

    *out_region += desc->region_count;
}

/*
 * Pack the frame's lights into the scene block the lit shader reads, mirroring glrend's
 * ProcessSceneLights() (drivers/glrend/cache.c) so the two shade the same. A non-radial
 * ambient light contributes a flat colour to every surface, so it is accumulated into
 * ambient_colour rather than packed; the rest keep their part order, which is what lets a
 * draw map a packed light back to its per-model cull flag.
 */
static void VkRenderPackSceneLights(br_renderer *self, br_vk_scene_state *scene)
{
    const state_stack *state              = self->state.current;
    br_boolean         use_ambient_colour = BR_FALSE;
    br_uint_32         num_lights         = 0;
    br_uint_32         volume_region      = 0;
    br_uint_32         volume_plane       = 0;

    scene->ambient_colour[0] = 0.0f;
    scene->ambient_colour[1] = 0.0f;
    scene->ambient_colour[2] = 0.0f;
    scene->ambient_colour[3] = 1.0f;

    scene->eye_view[0] = 0.0f;
    scene->eye_view[1] = 0.0f;
    scene->eye_view[2] = 1.0f;
    scene->eye_view[3] = 0.0f;

    for(int r = 0; r < 4; ++r)
        for(int c = 0; c < 4; ++c)
            scene->projection[r * 4 + c] = (float)state->matrix.view_to_screen.m[r][c];

    for(br_uint_32 i = 0; i < MAX_STATE_LIGHTS; ++i) {
        const state_light *light = &state->light[i];
        br_vk_light_state *out;
        br_vector3         half;
        float              intensity;
        float              red, green, blue;

        if(light->type == BRT_NONE)
            continue;

        intensity = (light->attenuation_c != BR_SCALAR(0.0)) ? BR_RCP(light->attenuation_c) : 16384.0f;
        red       = BR_RED(light->colour) / 255.0f;
        green     = BR_GRN(light->colour) / 255.0f;
        blue      = BR_BLU(light->colour) / 255.0f;

        if(light->type == BRT_AMBIENT && light->attenuation_type != BRT_RADII) {
            scene->ambient_colour[0] += red * intensity;
            scene->ambient_colour[1] += green * intensity;
            scene->ambient_colour[2] += blue * intensity;
            use_ambient_colour = BR_TRUE;
            continue;
        }

        ASSERT(num_lights < BR_MAX_LIGHTS);
        out = &scene->lights[num_lights];

        switch(light->type) {
            case BRT_AMBIENT:
                out->info[0] = 0;
                break;
            case BRT_DIRECT:
                out->info[0] = 1;
                break;
            case BRT_POINT:
                out->info[0] = 2;
                break;
            case BRT_SPOT:
                out->info[0] = 3;
                break;
            default:
                continue;
        }

        out->info[1] = (light->attenuation_type == BRT_RADII) ? 1u : 0u;
        out->info[2] = 0;
        out->info[3] = 0;

        out->position[0] = (float)light->position.v[0];
        out->position[1] = (float)light->position.v[1];
        out->position[2] = (float)light->position.v[2];
        out->position[3] = (light->type == BRT_DIRECT) ? 0.0f : 1.0f;

        out->direction[0] = (float)light->direction.v[0];
        out->direction[1] = (float)light->direction.v[1];
        out->direction[2] = (float)light->direction.v[2];
        out->direction[3] = 0.0f;

        out->half_vector[0] = 0.0f;
        out->half_vector[1] = 0.0f;
        out->half_vector[2] = 0.0f;
        out->half_vector[3] = 0.0f;

        if(light->type == BRT_DIRECT) {
            /* Halfway between the light direction and the view-space eye (0, 0, 1). */
            BrVector3Set(&half, light->direction.v[0], light->direction.v[1], light->direction.v[2] + BR_SCALAR(1.0));
            BrVector3Normalise(&half, &half);

            out->half_vector[0] = (float)half.v[0];
            out->half_vector[1] = (float)half.v[1];
            out->half_vector[2] = (float)half.v[2];

            /* The direction carries the intensity so the shader's diffuse term scales. */
            out->direction[0] *= intensity;
            out->direction[1] *= intensity;
            out->direction[2] *= intensity;
        }

        out->colour[0] = red;
        out->colour[1] = green;
        out->colour[2] = blue;
        out->colour[3] = 0.0f;

        out->iclq[0] = intensity;
        out->iclq[1] = (float)light->attenuation_c;
        out->iclq[2] = (float)light->attenuation_l;
        out->iclq[3] = (float)light->attenuation_q;

        out->radii[0] = (light->type == BRT_SPOT) ? (float)light->spot_inner : 0.0f;
        out->radii[1] = (light->type == BRT_SPOT) ? (float)light->spot_outer : 0.0f;
        out->radii[2] = 0.0f;
        out->radii[3] = 0.0f;

        if(light->attenuation_type == BRT_RADII) {
            out->radii[2] = (float)light->radius_inner;
            out->radii[3] = (float)light->radius_outer;

            if(out->radii[2] == out->radii[3])
                out->radii[2] = 0.0f;
        }

        VkRenderPackLightVolume(scene, &light->volume, &self->state.light_volume[i], out, &volume_region, &volume_plane);

        self->scene_light_part[num_lights] = i;
        ++num_lights;
    }

    for(int c = 0; c < 3; ++c) {
        if(scene->ambient_colour[c] > 1.0f)
            scene->ambient_colour[c] = 1.0f;
        if(scene->ambient_colour[c] < 0.0f)
            scene->ambient_colour[c] = 0.0f;
    }

    scene->counts[0] = num_lights;
    scene->counts[1] = use_ambient_colour ? 1u : 0u;
    scene->counts[2] = 0;
    scene->counts[3] = 0;

    self->scene_light_count = num_lights;
}

/*
 * Pack the enabled clip planes into the scene block, as glrend's StateGLUpdateScene()
 * does: for each state clip plane the core set to BRT_PLANE (core/v1db/enables.c
 * BrSetupClipPlanes()) apply view_to_screen, undoing the screen-space push that left the
 * plane in state, and store the result as a view-space half-space the vertex stage dots
 * with the view position. Planes keep their state index. The mask names the enabled ones
 * because Vulkan, unlike GL, has no glEnable: the shader declares the whole array and must
 * write every element, so a plane the core did not enable has to be marked disabled rather
 * than left unset.
 */
static void VkRenderPackSceneClipPlanes(br_renderer *self, br_vk_scene_state *scene)
{
    const state_stack *state = self->state.current;

    scene->clip_mask[0] = 0;
    scene->clip_mask[1] = 0;
    scene->clip_mask[2] = 0;
    scene->clip_mask[3] = 0;

    for(int i = 0; i < MAX_STATE_CLIP_PLANES; ++i) {
        br_vector4 plane;

        scene->clip_planes[i][0] = 0.0f;
        scene->clip_planes[i][1] = 0.0f;
        scene->clip_planes[i][2] = 0.0f;
        scene->clip_planes[i][3] = 0.0f;

        if(!(state->valid & MASK_STATE_CLIP) || state->clip[i].type != BRT_PLANE)
            continue;

        BrMatrix4TApply(&plane, &state->clip[i].plane, &state->matrix.view_to_screen);

        scene->clip_planes[i][0] = (float)plane.v[0];
        scene->clip_planes[i][1] = (float)plane.v[1];
        scene->clip_planes[i][2] = (float)plane.v[2];
        scene->clip_planes[i][3] = (float)plane.v[3];
        scene->clip_mask[0] |= 1u << i;
    }
}

/*
 * Pack the environment frame the fragment stage rotates a reflected vector into, mirroring
 * glrend's UpdateMatrices() (drivers/glrend/cache.c) and softrend's StaticCacheUpdate_PerModel()
 * (drivers/softrend/setup.c). BrSetupEnvironment() (core/v1db/enables.c) sets view_to_environment
 * and its hint once per scene, so the matrix belongs in the scene block rather than the
 * per-draw push constants: BRT_DONT_CARE means no environment is enabled, and the reference
 * renderers leave the frame at identity then, while BRT_NONE (or any other hint) means the
 * core set the matrix. The shader only uses the linear part, so the last row is the identity
 * row BrMatrix4Copy34() writes.
 */
static void VkRenderPackSceneEnvironment(br_renderer *self, br_vk_scene_state *scene)
{
    const state_stack *state = self->state.current;

    for(int r = 0; r < 4; ++r) {
        for(int c = 0; c < 4; ++c)
            scene->environment[r * 4 + c] = (r == c) ? 1.0f : 0.0f;
    }

    if(state->matrix.view_to_environment_hint == BRT_DONT_CARE)
        return;

    /*
     * A matrix34 has three columns, not four: the fourth is the identity
     * column BrMatrix4Copy34() writes, which the loop above has already put
     * there. Reading m[r][3] runs off the end of the row into the next one.
     */
    for(int r = 0; r < 3; ++r) {
        for(int c = 0; c < 3; ++c)
            scene->environment[r * 4 + c] = (float)state->matrix.view_to_environment.m[r][c];
    }
}

/*
 * Upload the scene block into a host-visible device buffer the draw binds with a push
 * descriptor. The buffer is freshly created per scene and retired against the frame that
 * reads it, because every work item recorded between two submissions shares the block.
 */
static VkBuffer VkRenderSceneBufferCreate(br_renderer *self, const br_vk_scene_state *scene, VmaAllocation *out_allocation)
{
    br_device    *dev = self->device;
    VkBuffer      buffer;
    VmaAllocation allocation;
    VkResult      res;

    const VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = sizeof(*scene),
        .usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo alloc_info = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    };

    if((res = vmaCreateBuffer(dev->vma_allocator, &buffer_info, &alloc_info, &buffer, &allocation, NULL)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the scene block buffer: VkResult = %d", res);
        return VK_NULL_HANDLE;
    }

    VmaAllocationInfo alloc_info_out;
    vmaGetAllocationInfo(dev->vma_allocator, allocation, &alloc_info_out);
    BrMemCpy(alloc_info_out.pMappedData, scene, sizeof(*scene));
    vmaFlushAllocation(dev->vma_allocator, allocation, 0, sizeof(*scene));

    *out_allocation = allocation;
    return buffer;
}

/*
 * The initial number of per-draw material blocks a scene's material buffer holds before it
 * grows. Small, because most scenes have few draws that change material; growth is a
 * reallocation, not a failure.
 */
#define BR_VKREND_MATERIAL_INITIAL_BLOCKS 16

/*
 * Create a host-visible device buffer for `capacity` per-draw material blocks and map it.
 * The caller retires it against the frame that reads it. Returns BR_FALSE on failure, with
 * the outputs unmodified.
 */
static br_boolean VkRenderMaterialBufferCreate(br_renderer *self, br_uint_32 capacity, VkBuffer *out_buffer, VmaAllocation *out_allocation,
                                               void **out_mapped)
{
    br_device *dev = self->device;
    VkBuffer   buffer;
    VkResult   res;

    const VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = (VkDeviceSize)capacity * sizeof(br_vk_material_state),
        .usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo alloc_info = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    };

    VmaAllocation     allocation;
    VmaAllocationInfo alloc_info_out;

    if((res = vmaCreateBuffer(dev->vma_allocator, &buffer_info, &alloc_info, &buffer, &allocation, &alloc_info_out)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the material block buffer: VkResult = %d", res);
        return BR_FALSE;
    }

    *out_buffer     = buffer;
    *out_allocation = allocation;
    *out_mapped     = alloc_info_out.pMappedData;
    return BR_TRUE;
}

/*
 * Reserve the next per-draw material block from the scene's material buffer, growing the
 * buffer if it is full. Returns the block for the caller to fill, or NULL if it could not
 * be allocated - which declines the draw, because a draw that needed fog but had no block
 * would silently render unfogged. *out_offset is the block's byte offset in
 * renderer->material_buffer; the draw pushes it as the block's descriptor range.
 *
 * Every buffer this creates is retired against the frame at creation, exactly like the
 * scene block: sceneBegin() retires the initial one and each replacement retires itself, so
 * the descriptors recorded against a buffer stay readable until the frame's fence signals
 * and no buffer is destroyed twice or leaked. (The buffer being replaced was already
 * retired when it was created - retiring it again here would enqueue a second destroy.)
 */
br_vk_material_state *RendererVkMaterialReserve(br_renderer *self, VkDeviceSize *out_offset)
{
    br_vk_material_state *block;
    VkDeviceSize          offset;

    if(self->material_buffer == VK_NULL_HANDLE)
        return NULL;

    if(self->material_count == self->material_capacity) {
        br_uint_32    capacity = self->material_capacity ? self->material_capacity * 2 : BR_VKREND_MATERIAL_INITIAL_BLOCKS;
        VkBuffer      buffer;
        VmaAllocation allocation;
        void         *mapped;

        if(!VkRenderMaterialBufferCreate(self, capacity, &buffer, &allocation, &mapped))
            return NULL;

        BrMemCpy(mapped, self->material_mapped, self->material_count * sizeof(br_vk_material_state));

        DevicePixelmapVkRetireBuffer(DevicePixelmapVkGetCurrentFrame(self->scene_dst->screen), buffer, allocation);

        self->material_buffer     = buffer;
        self->material_allocation = allocation;
        self->material_mapped     = mapped;
        self->material_capacity   = capacity;
    }

    offset = (VkDeviceSize)self->material_count * sizeof(br_vk_material_state);
    block  = (br_vk_material_state *)((char *)self->material_mapped + offset);
    self->material_count++;

    *out_offset = offset;
    return block;
}

/*
 * Push a just-filled material block to the device. CPU_TO_GPU memory is not guaranteed
 * coherent, so a draw flushes its block after writing it and before recording the
 * descriptor that reads it.
 */
void RendererVkMaterialFlush(br_renderer *self, VkDeviceSize offset)
{
    if(self->material_buffer == VK_NULL_HANDLE)
        return;

    vmaFlushAllocation(self->device->vma_allocator, self->material_allocation, offset, sizeof(br_vk_material_state));
}

/*
 * Close the dynamic rendering scope sceneBegin() opened, if one is open, and put the
 * destination back in GENERAL. Shared by sceneEnd() and sceneBegin(): the core re-enters a
 * scene through BrZsSceneRenderContinue(), which calls BrDbSceneRenderBegin() - and so
 * sceneBegin() - again with the previous scope still open, and a second vkCmdBeginRendering()
 * with no intervening end is invalid.
 */
static void RendererVkEndScope(br_renderer *self)
{
    br_device_pixelmap          *dst;
    br_device_pixelmap          *dst_owner;
    br_device_pixelmap_vk_frame *frame;
    const GladVulkanContext     *vk;
    VkCommandBuffer              cmd;

    if(!self->has_begun)
        return;

    dst       = self->scene_dst;
    dst_owner = DevicePixelmapVkResolve(dst);
    frame     = DevicePixelmapVkGetCurrentFrame(dst->screen);
    vk        = &self->device->context;
    cmd       = frame->command_buffer;

    vk->CmdEndRendering(cmd);

    const VkImageSubresourceRange range = {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    };
    const VkImageMemoryBarrier to_general = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
        .oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
        .image            = dst_owner->as_back.image,
        .subresourceRange = range,
    };
    vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1,
                           &to_general);

    /*
     * The layout is tracked on the owner: the image, and so the layout its barrier names,
     * belongs to the owner even when dst is a sub of it.
     */
    dst_owner->as_back.layout = VK_IMAGE_LAYOUT_GENERAL;

    /*
     * A Zb scene attached a depth buffer; put it back in GENERAL too, so its layout record
     * matches the image for whatever command follows the scene.
     */
    if(self->scene_depth != NULL) {
        const VkImageSubresourceRange depth_range = {
            .aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        };
        const VkImageMemoryBarrier depth_to_general = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
            .image            = self->scene_depth->as_back.image,
            .subresourceRange = depth_range,
        };
        vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                               VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &depth_to_general);

        self->scene_depth->as_back.layout = VK_IMAGE_LAYOUT_GENERAL;
        self->scene_depth                 = NULL;
    }

    self->has_begun = BR_FALSE;
    self->scene_dst = NULL;
}

static void BR_CMETHOD_DECL(br_renderer_vk, sceneBegin)(br_renderer *self)
{
    const GladVulkanContext     *vk;
    br_device_pixelmap          *colour_target;
    br_device_pixelmap          *colour_owner;
    br_device_pixelmap          *depth_target = NULL;
    const br_pixelmap_vk_fmt    *dst_fmt;
    const br_pixelmap_vk_fmt    *depth_fmt = NULL;
    br_device_pixelmap_vk_frame *frame;
    VkCommandBuffer              cmd;
    VkImageSubresourceRange      range;

    self->stats.face_group_count         = 0;
    self->stats.triangles_drawn_count    = 0;
    self->stats.triangles_rendered_count = 0;
    self->stats.vertices_rendered_count  = 0;
    self->stats.opaque_draw_count        = 0;
    self->stats.transparent_draw_count   = 0;

    /*
     * A continued scene (BrZsSceneRenderContinue()) re-enters here with the previous scope
     * still open; close it before opening the next so the command buffer never sees two
     * vkCmdBeginRendering() calls without an intervening end.
     */
    RendererVkEndScope(self);

    self->scene_buffer      = VK_NULL_HANDLE;
    self->scene_light_count = 0;

    self->material_buffer     = VK_NULL_HANDLE;
    self->material_allocation = VK_NULL_HANDLE;
    self->material_mapped     = NULL;
    self->material_count      = 0;
    self->material_capacity   = 0;

    colour_target = (self->state.current->valid & MASK_STATE_OUTPUT) ? self->state.current->output.colour : self->pixelmap;

    if(colour_target == NULL) {
        BrLogError("VKREND", "Cannot render without a destination.");
        return;
    }

    if(ObjectDevice(colour_target) != self->device || !DevicePixelmapVkIsOffscreen(colour_target)) {
        BrLogError("VKREND", "Renderer destination is not a vkrend offscreen.");
        return;
    }

    /*
     * The destination may be a sub-pixelmap, whose as_sub aliases as_back. The image, the
     * view and the tracked layout all belong to its owner, so every barrier, attachment and
     * layout write below goes through colour_owner; only the viewport and scissor are placed
     * from colour_target's rect within that owner.
     */
    colour_owner = DevicePixelmapVkResolve(colour_target);

    if((dst_fmt = DeviceVkGetFormatDetails(colour_target->pm_type)) == NULL || dst_fmt->format == VK_FORMAT_UNDEFINED) {
        BrLogError("VKREND", "Destination pixelmap type %d cannot be a colour attachment.", colour_target->pm_type);
        return;
    }

    vk = &self->device->context;

    /*
     * The destination image only carries VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT when the
     * device reports the format feature (DevicePixelmapVkCreateImage()); a format the table
     * knows but the device cannot attach must fail here, cleanly, rather than open a scope
     * on an image with no colour-attachment usage. Every other dynamic-rendering path
     * checks the same bit before opening a scope.
     */
    {
        VkFormatProperties dst_props;

        vk->GetPhysicalDeviceFormatProperties(self->device->physical_device->dev, dst_fmt->format, &dst_props);
        if(!(dst_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
            BrLogError("VKREND", "Destination pixelmap type %d cannot be a colour attachment.", colour_target->pm_type);
            return;
        }
    }

    /*
     * A Zb scene names a depth buffer in the OUTPUT state; a Zs scene leaves it NULL. A
     * depth target must be this device's depth pixelmap, of a format the table knows as a
     * depth type and the device can attach, for the same reasons the colour target must be.
     */
    depth_target = (self->state.current->valid & MASK_STATE_OUTPUT) ? self->state.current->output.depth : NULL;

    if(depth_target != NULL) {
        if(ObjectDevice(depth_target) != self->device || !DevicePixelmapVkIsDepth(depth_target)) {
            BrLogError("VKREND", "Renderer depth destination is not a vkrend depth pixelmap.");
            return;
        }

        if((depth_fmt = DeviceVkGetFormatDetails(depth_target->pm_type)) == NULL || !depth_fmt->depth || depth_fmt->format == VK_FORMAT_UNDEFINED) {
            BrLogError("VKREND", "Destination pixelmap type %d cannot be a depth attachment.", depth_target->pm_type);
            return;
        }

        VkFormatProperties depth_props;

        vk->GetPhysicalDeviceFormatProperties(self->device->physical_device->dev, depth_fmt->format, &depth_props);
        if(!(depth_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            BrLogError("VKREND", "Destination pixelmap type %d cannot be a depth attachment.", depth_target->pm_type);
            return;
        }
    }

    /*
     * The scene pipeline is destination-format specific, and with a depth buffer also
     * depth-format specific. If one cannot be built there is no point opening a rendering
     * scope, because every draw would be skipped anyway.
     */
    if(DeviceVkRenderEnsureInit(self->device) != BR_VK_RENDER_READY)
        return;

    if(DeviceVkRenderGetPipeline(self->device, dst_fmt->format, VK_SAMPLE_COUNT_1_BIT, BR_FALSE, BR_FALSE, BR_FALSE, BRT_FLAT, VK_CULL_MODE_NONE,
                                 VK_FRONT_FACE_COUNTER_CLOCKWISE, BR_FALSE, BRT_NONE, depth_target != NULL, depth_target != NULL,
                                 VK_COMPARE_OP_LESS, (depth_target != NULL) ? depth_fmt->format : VK_FORMAT_UNDEFINED) == VK_NULL_HANDLE)
        return;

    frame = DevicePixelmapVkGetCurrentFrame(colour_target->screen);
    cmd   = frame->command_buffer;

    /*
     * Pack the frame's lights, and the projection the vertex stage shares with the lit
     * fragment stage, into the scene block. It is a fresh buffer retired against this frame,
     * so a scene not yet submitted cannot be handed a later scene's lights.
     */
    br_vk_scene_state *scene_state = BrScratchAllocate(sizeof(*scene_state));
    if(scene_state == NULL) {
        BrLogError("VKREND", "Could not allocate the scene block.");
        return;
    }

    VkRenderPackSceneLights(self, scene_state);
    VkRenderPackSceneClipPlanes(self, scene_state);
    VkRenderPackSceneEnvironment(self, scene_state);

    VmaAllocation scene_allocation;
    VkBuffer      scene_buffer = VkRenderSceneBufferCreate(self, scene_state, &scene_allocation);

    BrScratchFree(scene_state);

    if(scene_buffer == VK_NULL_HANDLE)
        return;

    DevicePixelmapVkRetireBuffer(frame, scene_buffer, scene_allocation);
    self->scene_buffer = scene_buffer;

    /*
     * The per-draw material buffer, created fresh per scene and retired against this frame
     * like the scene block: every draw in the scene bump-allocates from it, and no draw may
     * read a block a later draw overwrites. The rendering scope has not been opened yet, so
     * a failure here returns with has_begun clear and no draw is recorded.
     */
    {
        VkBuffer      material_buffer;
        VmaAllocation material_allocation;
        void         *material_mapped;

        if(!VkRenderMaterialBufferCreate(self, BR_VKREND_MATERIAL_INITIAL_BLOCKS, &material_buffer, &material_allocation, &material_mapped))
            return;

        DevicePixelmapVkRetireBuffer(frame, material_buffer, material_allocation);
        self->material_buffer     = material_buffer;
        self->material_allocation = material_allocation;
        self->material_mapped     = material_mapped;
        self->material_count      = 0;
        self->material_capacity   = BR_VKREND_MATERIAL_INITIAL_BLOCKS;
    }

    range = (VkImageSubresourceRange){
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    };

    /*
     * Destination: GENERAL -> colour attachment for the render scope. GENERAL is the
     * driver's resting layout (the rect draw, the staging copy and the blit all assume it),
     * and loadOp LOAD keeps everything the scene does not cover. Record the new layout on the
     * owner: a command recorded between sceneBegin and sceneEnd - a fill, copy or readback -
     * uses the owner's as_back.layout as the oldLayout of its own barrier, so leaving it
     * claiming GENERAL would make that barrier name a layout the image is not in. sceneEnd
     * puts it back to GENERAL.
     */
    const VkImageMemoryBarrier to_colour = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask    = VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout        = colour_owner->as_back.layout,
        .newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image            = colour_owner->as_back.image,
        .subresourceRange = range,
    };
    vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1,
                           &to_colour);

    colour_owner->as_back.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    /*
     * A Zb scene also attaches the depth buffer: GENERAL -> depth-stencil attachment, cleared
     * to the far plane so every draw starts with a full depth range. As with the colour
     * image, the new layout is recorded in the pixelmap so a command recorded inside the
     * scope names the layout the image is really in; sceneEnd puts it back to GENERAL.
     */
    if(depth_target != NULL) {
        const VkImageSubresourceRange depth_range = {
            .aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        };
        const VkImageMemoryBarrier to_depth = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout        = depth_target->as_back.layout,
            .newLayout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .image            = depth_target->as_back.image,
            .subresourceRange = depth_range,
        };
        vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, 0, NULL, 0, NULL,
                               1, &to_depth);

        depth_target->as_back.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    /*
     * The viewport and scissor cover the destination's rect within the owner's image. A plain
     * offscreen is its own owner with a zero base, so this is the whole image; a sub draws
     * only into its rect. The origin is deliberately not added: pm_base_x/pm_base_y are the
     * rect's top-left in the owner's image coordinates, and the core's view_to_screen already
     * accounts for the sub's origin. The image's row 0 is the top - the scene vertex shader
     * emits Vulkan clip space - so unlike glrend there is no bottom-up flip of y.
     */
    const VkViewport viewport = {
        .x      = (float)colour_target->pm_base_x,
        .y      = (float)colour_target->pm_base_y,
        .width  = (float)colour_target->pm_width,
        .height = (float)colour_target->pm_height,
        /*
         * Reverse the depth range: BRender's near plane is NDC z = +1 and its view_to_screen
         * emits OpenGL clip space, so after the shader's [-w, w] -> [0, w] remap near is the
         * larger depth. Mapping [0, 1] to [1, 0] makes near 0, which is what a LESS depth test
         * wants. This is the VkViewport equivalent of glrend's glDepthRange(1, 0).
         */
        .minDepth = 1.0f,
        .maxDepth = 0.0f,
    };
    const VkRect2D area = {
        .offset = {colour_target->pm_base_x, colour_target->pm_base_y},
        .extent = {colour_target->pm_width,  colour_target->pm_height},
    };
    vk->CmdSetViewport(cmd, 0, 1, &viewport);
    vk->CmdSetScissor(cmd, 0, 1, &area);

    const VkRenderingAttachmentInfo colour_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = colour_owner->as_back.view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
    };

    /*
     * The depth attachment exists only for the Zb path (output.depth != NULL). loadOp CLEAR
     * resets it to 1.0 - the far plane in the shader's remapped [0, w] clip range - because
     * the buffer outlives the scene and must not carry a previous scene's depths into this
     * one. For Zs the field stays NULL and the rendering scope is byte-for-byte the one
     * opened before the depth path existed.
     */
    const VkRenderingAttachmentInfo depth_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = (depth_target != NULL) ? depth_target->as_back.view : VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = {.depthStencil = {1.0f, 0}},
    };
    const VkRenderingInfo rendering = {
        .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea           = area,
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &colour_attachment,
        .pDepthAttachment     = (depth_target != NULL) ? &depth_attachment : NULL,
    };
    vk->CmdBeginRendering(cmd, &rendering);

    self->scene_dst   = colour_target;
    self->scene_depth = depth_target;
    self->has_begun   = BR_TRUE;
}

static void BR_CMETHOD_DECL(br_renderer_vk, sceneEnd)(br_renderer *self)
{
    RendererVkEndScope(self);

    /*
     * The scene block was retired against the frame when it was created; drop the handle so
     * a draw between scenes cannot read a block the frame no longer references.
     */
    self->scene_buffer      = VK_NULL_HANDLE;
    self->scene_light_count = 0;

    self->material_buffer     = VK_NULL_HANDLE;
    self->material_allocation = VK_NULL_HANDLE;
    self->material_mapped     = NULL;
    self->material_count      = 0;
    self->material_capacity   = 0;

    BrPoolEmpty(self->state_pool);
}

static void BR_CMETHOD_DECL(br_renderer_vk, free)(br_object *_self)
{
    br_renderer *self = (br_renderer *)_self;

    BrPoolFree(self->state_pool);

    ObjectContainerRemove(self->renderer_facility, (br_object *)self);

    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_renderer_vk, identifier)(br_object *self)
{
    return ((br_renderer *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_renderer_vk, type)(br_object *self)
{
    (void)self;
    return BRT_RENDERER;
}

static br_boolean BR_CMETHOD_DECL(br_renderer_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_RENDERER) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static struct br_device *BR_CMETHOD_DECL(br_renderer_vk, device)(br_object *self)
{
    return ((br_renderer *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_renderer_vk, space)(br_object *self)
{
    (void)self;
    return sizeof(br_renderer);
}

static br_tv_template *BR_CMETHOD_DECL(br_renderer_vk, templateQuery)(br_object *_self)
{
    br_renderer *self = (br_renderer *)_self;

    if(self->device->templates.rendererTemplate == NULL) {
        self->device->templates.rendererTemplate = BrTVTemplateAllocate(self->device, rendererTemplateEntries, BR_ASIZE(rendererTemplateEntries));
    }

    return self->device->templates.rendererTemplate;
}

static void *BR_CMETHOD_DECL(br_renderer_vk, listQuery)(br_object_container *self)
{
    return ((br_renderer *)self)->object_list;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, validDestination)(br_renderer *self, br_boolean *bp, br_object *h)
{
    (void)self;
    (void)bp;
    (void)h;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, stateStoredNew)(br_renderer *self, br_renderer_state_stored **pss, br_uint_32 mask,
                                                                br_token_value *tv)
{
    br_renderer_state_stored *ss;

    if((ss = RendererStateStoredVkAllocate(self, self->state.current, mask, tv)) == NULL)
        return BRE_FAIL;

    *pss = ss;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, stateStoredAvail)(br_renderer *self, br_int_32 *psize, br_uint_32 mask, br_token_value *tv)
{
    (void)self;
    (void)psize;
    (void)mask;
    (void)tv;
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, bufferStoredNew)(br_renderer *self, struct br_buffer_stored **psm, br_token use,
                                                                 br_device_pixelmap *pm, br_token_value *tv)
{
    struct br_buffer_stored *stored;

    if(psm == NULL || pm == NULL)
        return BRE_FAIL;

    if((stored = BufferStoredVkAllocate(self, use, pm, tv)) == NULL)
        return BRE_FAIL;

    *psm = stored;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, bufferStoredAvail)(br_renderer *self, br_int_32 *space, br_token use, br_token_value *tv)
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
static br_error BR_CMETHOD_DECL(br_renderer_vk, partSet)(br_renderer *self, br_token part, br_int_32 index, br_token t, br_value value)
{
    br_error               r;
    br_uint_32             m;
    struct br_tv_template *tp;

    if((tp = StateVkGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    m = 0;
    r = BrTokenValueSet(self->state.current, &m, t, value, tp, self);
    if(m)
        StateVkTemplateActions(&self->state, m);

    return r;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, partSetMany)(br_renderer *self, br_token part, br_int_32 index, br_token_value *tv, br_int_32 *pcount)
{
    br_error        r;
    br_uint_32      m;
    br_tv_template *tp;

    if((tp = StateVkGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    m = 0;
    r = BrTokenValueSetMany(self->state.current, pcount, &m, tv, tp, self);
    if(m)
        StateVkTemplateActions(&self->state, m);

    return r;
}

/*
 * Reading current state
 */
static br_error BR_CMETHOD_DECL(br_renderer_vk, partQuery)(br_renderer *self, br_token part, br_int_32 index, void *pvalue, br_token t)
{
    br_tv_template *tp;

    if((tp = StateVkGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQuery(pvalue, NULL, 0, t, self->state.current, tp, self);
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, partQueryBuffer)(br_renderer *self, br_token part, br_int_32 index, void *pvalue,
                                                                 void *buffer, br_size_t buffer_size, br_token t)
{
    br_tv_template *tp;

    if((tp = StateVkGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQuery(pvalue, buffer, buffer_size, t, self->state.current, tp, self);
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, partQueryMany)(br_renderer *self, br_token part, br_int_32 index, br_token_value *tv,
                                                               void *extra, br_size_t extra_size, br_int_32 *pcount)
{
    br_tv_template *tp;

    if((tp = StateVkGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryMany(tv, extra, extra_size, pcount, self->state.current, tp, self);
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, partQueryManySize)(br_renderer *self, br_token part, br_int_32 index,
                                                                   br_size_t *pextra_size, br_token_value *tv)
{
    br_tv_template *tp;

    if((tp = StateVkGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryManySize(pextra_size, tv, self->state.current, tp, self);
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, partQueryAll)(br_renderer *self, br_token part, br_int_32 index, br_token_value *buffer,
                                                              br_size_t buffer_size)
{
    br_tv_template *tp;

    if((tp = StateVkGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryAll(buffer, buffer_size, self->state.current, tp, self);
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, partQueryAllSize)(br_renderer *self, br_token part, br_int_32 index, br_size_t *psize)
{
    br_tv_template *tp;

    if((tp = StateVkGetStateTemplate(&self->state, part, index)) == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryAllSize(psize, self->state.current, tp, self);
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, partIndexQuery)(br_renderer *self, br_token part, br_int_32 *pnindex)
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

static br_error BR_CMETHOD_DECL(br_renderer_vk, commandModeSet)(br_renderer *self, br_token mode)
{
    (void)self;
    (void)mode;
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, commandModeQuery)(br_renderer *self, br_token *mode)
{
    (void)self;
    (void)mode;
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, commandModeDefault)(br_renderer *self)
{
    (void)self;
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, commandModePush)(br_renderer *self)
{
    (void)self;
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, commandModePop)(br_renderer *self)
{
    (void)self;
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, modelMul)(br_renderer *self, br_matrix34 *m)
{
    br_matrix34 om = self->state.current->matrix.model_to_view;

    BrMatrix34Mul(&self->state.current->matrix.model_to_view, m, &om);

    self->state.current->matrix.model_to_view_hint = BRT_NONE;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, modelPopPushMul)(br_renderer *self, br_matrix34 *m)
{
    if(self->state.top == 0)
        return BRE_UNDERFLOW;

    BrMatrix34Mul(&self->state.current->matrix.model_to_view, m, &self->state.stack[0].matrix.model_to_view);

    self->state.current->matrix.model_to_view_hint = BRT_NONE;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, modelInvert)(br_renderer *self)
{
    br_matrix34 old;

    BrMatrix34Copy(&old, &self->state.current->matrix.model_to_view);

    if(self->state.current->matrix.model_to_view_hint == BRT_LENGTH_PRESERVING)
        BrMatrix34LPInverse(&self->state.current->matrix.model_to_view, &old);
    else
        BrMatrix34Inverse(&self->state.current->matrix.model_to_view, &old);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, statePush)(br_renderer *self, br_uint_32 mask)
{
    return StateVkPush(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, statePop)(br_renderer *self, br_uint_32 mask)
{
    return StateVkPop(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, stateSave)(br_renderer *self, br_renderer_state_stored *save, br_uint_32 mask)
{
    StateVkCopy(&save->state, self->state.current, mask);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, stateRestore)(br_renderer *self, br_renderer_state_stored *save, br_uint_32 mask)
{
    StateVkCopy(self->state.current, &save->state, mask);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, stateDefault)(br_renderer *self, br_uint_32 mask)
{
    StateVkDefault(&self->state, mask);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, stateMask)(br_renderer *self, br_uint_32 *mask, const br_token *parts, br_size_t n_parts)
{
    br_uint_32 m;

    (void)self;

    if(mask == NULL)
        return BRE_FAIL;

    m = 0;
    for(br_size_t i = 0; i < n_parts; i++) {
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

static br_error BR_CMETHOD_DECL(br_renderer_vk, boundsTest)(br_renderer *self, br_token *r, br_bounds3 *bounds)
{
    br_matrix4 m2s;

    BrMatrix4Mul34(&m2s, &self->state.current->matrix.model_to_view, &self->state.current->matrix.view_to_screen);
    *r = VkOnScreenCheck(&m2s, bounds);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, flush)(br_renderer *self, br_boolean wait)
{
    /*
     * Drawing is recorded synchronously into the front screen's current frame command
     * buffer; the readback's flush submits it. There is nothing to flush here.
     */
    (void)self;
    (void)wait;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, synchronise)(br_renderer *self, br_token sync_type, br_boolean block)
{
    (void)self;
    (void)sync_type;
    (void)block;
    return BRE_UNSUPPORTED;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, frameBegin)(br_renderer *self)
{
    (void)self;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, frameEnd)(br_renderer *self)
{
    (void)self;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, focusLossBegin)(br_renderer *self)
{
    (void)self;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_vk, focusLossEnd)(br_renderer *self)
{
    (void)self;
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
    ._free                 = BR_CMETHOD(br_renderer_vk, free),
    ._identifier           = BR_CMETHOD(br_renderer_vk, identifier),
    ._type                 = BR_CMETHOD(br_renderer_vk, type),
    ._isType               = BR_CMETHOD(br_renderer_vk, isType),
    ._device               = BR_CMETHOD(br_renderer_vk, device),
    ._space                = BR_CMETHOD(br_renderer_vk, space),
    ._templateQuery        = BR_CMETHOD(br_renderer_vk, templateQuery),
    ._query                = BR_CMETHOD(br_object, query),
    ._queryBuffer          = BR_CMETHOD(br_object, queryBuffer),
    ._queryMany            = BR_CMETHOD(br_object, queryMany),
    ._queryManySize        = BR_CMETHOD(br_object, queryManySize),
    ._queryAll             = BR_CMETHOD(br_object, queryAll),
    ._queryAllSize         = BR_CMETHOD(br_object, queryAllSize),
    ._listQuery            = BR_CMETHOD(br_renderer_vk, listQuery),
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

    ._validDestination   = BR_CMETHOD(br_renderer_vk, validDestination),
    ._stateStoredNew     = BR_CMETHOD(br_renderer_vk, stateStoredNew),
    ._stateStoredAvail   = BR_CMETHOD(br_renderer_vk, stateStoredAvail),
    ._bufferStoredNew    = BR_CMETHOD(br_renderer_vk, bufferStoredNew),
    ._bufferStoredAvail  = BR_CMETHOD(br_renderer_vk, bufferStoredAvail),
    ._partSet            = BR_CMETHOD(br_renderer_vk, partSet),
    ._partSetMany        = BR_CMETHOD(br_renderer_vk, partSetMany),
    ._partQuery          = BR_CMETHOD(br_renderer_vk, partQuery),
    ._partQueryBuffer    = BR_CMETHOD(br_renderer_vk, partQueryBuffer),
    ._partQueryMany      = BR_CMETHOD(br_renderer_vk, partQueryMany),
    ._partQueryManySize  = BR_CMETHOD(br_renderer_vk, partQueryManySize),
    ._partQueryAll       = BR_CMETHOD(br_renderer_vk, partQueryAll),
    ._partQueryAllSize   = BR_CMETHOD(br_renderer_vk, partQueryAllSize),
    ._partIndexQuery     = BR_CMETHOD(br_renderer_vk, partIndexQuery),
    ._modelMul           = BR_CMETHOD(br_renderer_vk, modelMul),
    ._modelPopPushMul    = BR_CMETHOD(br_renderer_vk, modelPopPushMul),
    ._modelInvert        = BR_CMETHOD(br_renderer_vk, modelInvert),
    ._statePush          = BR_CMETHOD(br_renderer_vk, statePush),
    ._statePop           = BR_CMETHOD(br_renderer_vk, statePop),
    ._stateSave          = BR_CMETHOD(br_renderer_vk, stateSave),
    ._stateRestore       = BR_CMETHOD(br_renderer_vk, stateRestore),
    ._stateMask          = BR_CMETHOD(br_renderer_vk, stateMask),
    ._stateDefault       = BR_CMETHOD(br_renderer_vk, stateDefault),
    ._boundsTest         = BR_CMETHOD(br_renderer_vk, boundsTest),
    ._commandModeSet     = BR_CMETHOD(br_renderer_vk, commandModeSet),
    ._commandModeQuery   = BR_CMETHOD(br_renderer_vk, commandModeQuery),
    ._commandModeDefault = BR_CMETHOD(br_renderer_vk, commandModeDefault),
    ._commandModePush    = BR_CMETHOD(br_renderer_vk, commandModePush),
    ._commandModePop     = BR_CMETHOD(br_renderer_vk, commandModePop),
    ._flush              = BR_CMETHOD(br_renderer_vk, flush),
    ._synchronise        = BR_CMETHOD(br_renderer_vk, synchronise),
    ._frameBegin         = BR_CMETHOD(br_renderer_vk, frameBegin),
    ._frameEnd           = BR_CMETHOD(br_renderer_vk, frameEnd),
    ._focusLossBegin     = BR_CMETHOD(br_renderer_vk, focusLossBegin),
    ._focusLossEnd       = BR_CMETHOD(br_renderer_vk, focusLossEnd),
    ._sceneBegin         = BR_CMETHOD(br_renderer_vk, sceneBegin),
    ._sceneEnd           = BR_CMETHOD(br_renderer_vk, sceneEnd),
};
