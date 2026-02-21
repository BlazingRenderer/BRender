#ifndef VKREND_STATE_H_
#define VKREND_STATE_H_

#include "drv.h"

#define MAX_STATE_STACK       64
#define MAX_STATE_LIGHTS      BR_MAX_LIGHTS
#define MAX_STATE_CLIP_PLANES BR_MAX_CLIP_PLANES

/*
 * Caps on a light's cutoff volume. As in glrend, a volume over either is dropped whole
 * rather than clipped - a partial volume would light the wrong region. The scene-wide
 * tables the shader reads are larger; see BR_VKREND_MAX_LIGHT_VOLUME_* in renderer.h.
 */
#define MAX_STATE_LIGHT_VOLUME_REGIONS 2
#define MAX_STATE_LIGHT_VOLUME_PLANES  8

/*
 * State masks - these should match the BR_STATE_* enums.
 */
enum {
    MASK_STATE_SURFACE   = BR_STATE_SURFACE,
    MASK_STATE_MATRIX    = BR_STATE_MATRIX,
    MASK_STATE_ENABLE    = BR_STATE_ENABLE,
    MASK_STATE_LIGHT     = BR_STATE_LIGHT,
    MASK_STATE_CLIP      = BR_STATE_CLIP,
    MASK_STATE_BOUNDS    = BR_STATE_BOUNDS,
    MASK_STATE_CULL      = BR_STATE_CULL,
    MASK_STATE_OUTPUT    = BR_STATE_OUTPUT,
    MASK_STATE_PRIMITIVE = BR_STATE_PRIMITIVE,

    /*
     * Parts of state that are stored.
     * This is also PARTS_MASK in prepmatl.c
     */
    MASK_STATE_STORED = (MASK_STATE_CULL | MASK_STATE_SURFACE | MASK_STATE_PRIMITIVE),
};

/*
 * Bits used for template masks to indicate actions when a template entry is triggered.
 * Kept from glrend so the template tables are recognisably the same; StateVkTemplateActions()
 * is a no-op, as glrend's is.
 */
#define TM_PART           0x00001
#define TM_INDEX          0x00002
#define TM_V2S_HINT       0x00004
#define TM_M2V            0x00008
#define TM_V2S            0x00010
#define TM_COPY_CACHE     0x00020
#define TM_CACHE          0x00040

#define TM_CLEAR_M2V_HINT 0x00080
#define TM_CLEAR_V2S_HINT 0x00100

#define TM_INVALID_PS     0x01000
#define TM_INVALID_PM     0x02000
#define TM_INVALID_V2M    0x04000
#define TM_INVALID_M2S    0x08000
#define TM_INVALID_CC     0x10000

typedef struct state_clip {
    br_token   type;
    br_vector4 plane;
} state_clip;

typedef struct state_matrix {
    br_matrix34 model_to_view;
    br_matrix4  view_to_screen;
    br_matrix34 view_to_environment;

    br_token model_to_view_hint;
    br_token view_to_screen_hint;
    br_token view_to_environment_hint;

    br_scalar hither_z;
    br_scalar yon_z;
} state_matrix;

typedef struct state_cull {
    br_token type;
} state_cull;

typedef struct state_surface {
    br_colour colour;

    br_scalar opacity;
    br_scalar ka;
    br_scalar kd;
    br_scalar ks;
    br_scalar power;

    br_boolean lighting;
    br_boolean prelighting;
    br_boolean force_front;
    br_boolean force_back;
    br_scalar  depth_bias;

    br_token colour_source;
    br_token opacity_source;
    br_token mapping_source;

    br_matrix23 map_transform;
} state_surface;

typedef struct state_light_volume_region {
    br_uint_32 plane_offset; /* Into state_light_volume::planes */
    br_uint_32 plane_count;
} state_light_volume_region;

/*
 * A light's cutoff volume, stored by value.
 *
 * This lives in state_all, not state_light: state_light sits in the state stack, which
 * StateVkPush() copies whole, so the region and plane arrays are kept outside it. The
 * descriptor below rides in state_light and names the light's slot; LightingVolumeSet()
 * copies the payload in here and VkRenderPackSceneLights() packs it into the scene block.
 */
typedef struct state_light_volume {
    br_scalar                 falloff_distance;
    br_uint_32                region_count; /* 0 means no volume */
    state_light_volume_region regions[MAX_STATE_LIGHT_VOLUME_REGIONS];
    br_vector4                planes[MAX_STATE_LIGHT_VOLUME_PLANES];
} state_light_volume;

/*
 * The part of a light's cutoff volume that rides in state_light: enough for
 * VkRenderPackSceneLights() to find the payload in state_all::light_volume and to know
 * whether the light has one at all. region_count == 0 means no volume.
 */
typedef struct state_light_volume_desc {
    br_scalar  falloff_distance;
    br_uint_32 region_count;
} state_light_volume_desc;

typedef struct state_light {
    br_token type;

    br_vector3_f position;
    br_vector3_f direction;

    br_colour colour;

    br_scalar spot_outer;
    br_scalar spot_inner;

    br_scalar radius_outer;
    br_scalar radius_inner;

    br_angle angle_outer;

    br_scalar attenuation_l;
    br_scalar attenuation_c;
    br_scalar attenuation_q;

    br_token attenuation_type;
    br_token attenuation_hint;

    br_boolean radius_cull;
    br_boolean angle_cull;

    state_light_volume_desc volume;

    br_boolean culled;
} state_light;

typedef void insert_cbfn(br_primitive *primitive, void *arg1, void *arg2, void *arg3, br_order_table *order_table, br_scalar *z);

typedef struct state_hidden {
    br_token           type;
    br_token           divert;
    br_order_table    *order_table;
    br_primitive_heap *heap;
    insert_cbfn       *insert_fn;
    void              *insert_arg1;
    void              *insert_arg2;
    void              *insert_arg3;
} state_hidden;

/*
 * state.prim.flags
 */
enum {
    /*
     * The public flags
     */
    PRIMF_DEPTH_WRITE_BIT,

    PRIMF_BLEND_BIT,
    PRIMF_MODULATE_BIT,
    PRIMF_COLOUR_KEY_BIT,
};

enum {
    PRIMF_DEPTH_WRITE = (1 << PRIMF_DEPTH_WRITE_BIT),
    PRIMF_BLEND       = (1 << PRIMF_BLEND_BIT),
    PRIMF_MODULATE    = (1 << PRIMF_MODULATE_BIT),
    PRIMF_COLOUR_KEY  = (1 << PRIMF_COLOUR_KEY_BIT),
};

struct br_buffer_stored;

typedef struct state_primitive {
    br_uint_32 flags;

    br_int_32 index_base;
    br_int_32 index_range;

    br_token colour_type;

    br_token depth_test;

    br_token blend_mode;

    br_token map_width_limit;

    br_token map_height_limit;

    br_token shading_mode;

    br_token  perspective_type;
    br_int_32 subdivide_tolerance;

    /*
     * Current input buffers
     */
    struct br_buffer_stored *colour_map;
    struct br_buffer_stored *index_shade;
    struct br_buffer_stored *index_blend;
    struct br_buffer_stored *index_fog;
    struct br_buffer_stored *screendoor;
    struct br_buffer_stored *lighting;
    struct br_buffer_stored *bump;

    br_token filter;

    br_token mip_filter;

    br_token  fog_type;
    br_scalar fog_min;
    br_scalar fog_max;
    br_colour fog_colour;
    br_uint_8 fog_scale;
} state_primitive;

typedef struct state_output {
    struct br_device_pixelmap *colour;
    struct br_device_pixelmap *depth;
} state_output;

typedef struct state_stack {
    state_matrix matrix;

    state_clip      clip[MAX_STATE_CLIP_PLANES];
    state_cull      cull;
    state_surface   surface;
    state_primitive prim;
    state_output    output;
    state_light     light[MAX_STATE_LIGHTS];
    state_hidden    hidden;

    br_uint_32 valid;

    /*
     * Render type (BRT_POINT, BRT_LINE, BRT_TRIANGLE).
     */
    br_token render_type;

    /*
     * Only for use when in the pool.
     */
    br_uint_32 num_refs;
} state_stack;

typedef struct state_all {
    state_stack  default_;
    state_stack *current;
    state_stack  stack[MAX_STATE_STACK];
    int          top;

    /*
     * Every light's cutoff volume payload, indexed by light part. state_all is outside the
     * state stack, so this is not copied by StateVkPush()/StateVkPop(); state_light::volume
     * names the slot.
     */
    state_light_volume light_volume[MAX_STATE_LIGHTS];

    struct {
        struct br_tv_template *clip[MAX_STATE_CLIP_PLANES];
        struct br_tv_template *matrix;
        struct br_tv_template *cull;
        struct br_tv_template *surface;
        struct br_tv_template *prim;
        struct br_tv_template *output;
        struct br_tv_template *light[MAX_STATE_LIGHTS];
        struct br_tv_template *hidden;
    } templates;

    void *res;

} state_all;

#ifdef BR_RENDERER_STATE_STORED_PRIVATE

/*
 * A snapshot of renderer state, taken by RendererStateStoredNew()/stateSave() and applied
 * by RendererStateRestore(). A material's stored state is one of these.
 */
typedef struct br_renderer_state_stored {
    const struct br_renderer_state_stored_dispatch *dispatch;

    const char *identifier;

    struct br_device *device;

    /*
     * Renderer this state is associated with
     */
    struct br_renderer *renderer;

    state_stack state;
} br_renderer_state_stored;

#endif

#endif /* VKREND_STATE_H_ */
