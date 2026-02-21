/*
 * Private renderer structure
 */
#ifndef VKREND_RENDERER_H_
#define VKREND_RENDERER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_RENDERER_PRIVATE

/*
 * One packed light, mirroring the br_light struct in shaders/render.common.glsl. The field
 * set is the one glrend's shader consumes, so the two renderers shade the same.
 */
typedef struct br_vk_light_state {
    float    position[4];    /* (X, Y, Z, w); w is 0 for a directional light, 1 otherwise. */
    float    direction[4];   /* (X, Y, Z, 0), view space; scaled by intensity when directional. */
    float    half_vector[4]; /* (X, Y, Z, 0), the directional light's halfway vector. */
    float    colour[4];      /* (R, G, B, 0). */
    float    iclq[4];        /* (intensity, constant, linear, quadratic) attenuation. */
    float    radii[4];       /* (cos(inner), cos(outer), radius_inner, radius_outer). */
    uint32_t info[4];        /* (type, attenuation_type, 0, 0). */
    float    volume[4];      /* (region_offset, region_count, falloff_distance, 0). */
} br_vk_light_state;

/*
 * Caps on the scene-wide light-volume tables the shader reads, as glrend's
 * BR_GL_MAX_LIGHT_VOLUME_*: the per-light caps in state.h bound one light, these bound the
 * sum across the frame. A volume that does not fit is dropped whole; see
 * VkRenderPackLightVolume().
 */
#define BR_VKREND_MAX_LIGHT_VOLUME_REGIONS 16
#define BR_VKREND_MAX_LIGHT_VOLUME_PLANES  64

/*
 * The per-scene block the scene draw reads, mirroring SceneData in shaders/render.common.glsl.
 * It is a uniform buffer rather than push constants because a full light array cannot fit in
 * the 128 bytes Vulkan guarantees.
 */
typedef struct br_vk_scene_state {
    float             projection[16];                     /* view_to_screen, transposed. */
    float             environment[16];                    /* view_to_environment, transposed; identity when unset. */
    float             eye_view[4];                        /* (0, 0, 1, 0): the view-space eye direction. */
    float             ambient_colour[4];                  /* Sum of the non-radial ambient lights, clamped. */
    uint32_t          counts[4];                          /* [0] num_lights, [1] use_ambient_colour. */
    float             clip_planes[BR_MAX_CLIP_PLANES][4]; /* View-space half-spaces, one per clip plane. */
    uint32_t          clip_mask[4];                       /* [0] bit i => clip plane i was enabled in the state. */
    br_vk_light_state lights[BR_MAX_LIGHTS];

    /*
     * Every light's cutoff volume, gathered into one table per scene, as glrend's
     * br_gl_main_data_scene. Each region is (plane_offset, plane_count, 0, 0) into the plane
     * table, and a light names its first region in br_vk_light_state::volume.
     */
    float light_volume_regions[BR_VKREND_MAX_LIGHT_VOLUME_REGIONS][4];
    float light_volume_planes[BR_VKREND_MAX_LIGHT_VOLUME_PLANES][4];
} br_vk_scene_state;

/*
 * One draw's material block, mirroring MaterialData in shaders/render.common.glsl. The
 * push constant block is exactly the 128 bytes Vulkan guarantees, so the per-draw material
 * state the fragment stage needs beyond it lives here and is bound as a push descriptor at
 * set 0, binding 3. The trailing reserved word pads the block to a 16-byte multiple so
 * more material state can join the fog fields without moving them.
 */
typedef struct br_vk_material_state {
    float    fog_colour[4]; /* (R, G, B, 1): glrend's fog_colour with alpha 1. */
    float    fog_range[2];  /* (fog_min, fog_max), in view-space units.      */
    uint32_t fog_enabled;   /* 1 when the primitive's fog_type is not BRT_NONE. */
    uint32_t reserved;      /* Padding for material state to join later.     */
} br_vk_material_state;

typedef struct br_renderer {
    /*
     * Dispatch table
     */
    const struct br_renderer_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * List of objects associated with this renderer
     */
    void *object_list;

    /*
     * Destination pixelmap this renderer was created for. The colour target actually
     * rendered to comes from the OUTPUT state, which is this pixelmap in the common case.
     */
    br_device_pixelmap *pixelmap;

    br_renderer_facility *renderer_facility;

    /*
     * Pool of state_stack snapshots, one per deferred primitive. Emptied at scene end;
     * the primitive heap and order tables that point into it are reset alongside.
     */
    br_pool *state_pool;

    state_all state;

    /*
     * Set by sceneBegin() when a dynamic rendering scope was opened on the destination,
     * cleared by sceneEnd(). No draw may be recorded while it is clear.
     */
    br_boolean has_begun;

    /*
     * The destination sceneBegin() opened the rendering scope on, and the command buffer
     * it recorded into. Kept so sceneEnd() ends the scope on the same image and updates
     * the same layout. It may be a sub-pixelmap, whose own as_back aliases as_sub, so
     * RendererVkEndScope() resolves the owner through DevicePixelmapVkResolve() before
     * touching the image or the tracked layout. scene_depth is the depth buffer attached
     * for a Zb scene, or NULL for a Zs scene (the bucket-sorted path with no depth
     * attachment).
     */
    br_device_pixelmap *scene_dst;
    br_device_pixelmap *scene_depth;

    /*
     * The scene block this scene's draws read: the projection and the packed lights. It is a
     * fresh device buffer per sceneBegin, retired against the frame that references it, so a
     * scene recorded but not yet submitted cannot be handed a later scene's lights.
     */
    VkBuffer   scene_buffer;
    br_uint_32 scene_light_count;

    /*
     * The per-draw material block buffer. The push constant block is full, so each draw
     * bump-allocates one br_vk_material_state from this buffer and pushes its offset as a
     * descriptor. The buffer is created per sceneBegin and retired against the frame that
     * reads it - and replaced on growth, with the old one retired too - so no recorded draw
     * reads a block a later draw's write overwrote.
     */
    VkBuffer      material_buffer;
    VmaAllocation material_allocation;
    void         *material_mapped;
    br_uint_32    material_count;
    br_uint_32    material_capacity;

    /*
     * The light part each packed light came from, so a draw can find that light's
     * per-model cull flag.
     */
    br_uint_32 scene_light_part[MAX_STATE_LIGHTS];

    struct {
        br_uint_32 face_group_count;
        br_uint_32 triangles_drawn_count;
        br_uint_32 triangles_rendered_count;
        br_uint_32 vertices_rendered_count;
        br_uint_32 opaque_draw_count;
        br_uint_32 transparent_draw_count;
    } stats;
} br_renderer;

#endif

#ifdef __cplusplus
};
#endif
#endif /* VKREND_RENDERER_H_ */
