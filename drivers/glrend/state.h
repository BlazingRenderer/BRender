#ifndef STATE_H_
#define STATE_H_

#include "drv.h"

#define MAX_STATE_STACK       64
#define MAX_STATE_LIGHTS      BR_MAX_LIGHTS
#define MAX_STATE_CLIP_PLANES BR_MAX_CLIP_PLANES

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
};

/*
 * Bits used for template masks to indicate actions when a template entry is triggered
 */
#define TM_PART           0x00001 /* set timestamp for whole part								*/
#define TM_INDEX          0x00002 /* set timestamp for indexed part							*/
#define TM_V2S_HINT       0x00004 /* set timestamp for matrix.view_to_screen_hint				*/
#define TM_M2V            0x00008 /* set timestamp for matrix.model_to_view					*/
#define TM_V2S            0x00010 /* set timestamp for matrix.view_to_screen					*/
#define TM_COPY_CACHE     0x00020 /* set timestamp for anything that affects the copied cache	*/
#define TM_CACHE          0x00040 /* set timestamp for anything that affects the static cache	*/

#define TM_CLEAR_M2V_HINT 0x00080 /* set M2V_HINT to NONE										*/
#define TM_CLEAR_V2S_HINT 0x00100 /* set V2S_HINT to NONE										*/

#define TM_INVALID_PS     0x01000 /* set Update Per Scene										*/
#define TM_INVALID_PM     0x02000 /* set Update Per Model										*/
#define TM_INVALID_V2M    0x04000 /* set Update View To Model									*/
#define TM_INVALID_M2S    0x08000 /* set Update Model To Screen								*/
#define TM_INVALID_CC     0x10000 /* set Update Copied Cache									*/

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
    br_token space;
} state_cull;

typedef struct state_surface {
    br_colour colour;

    br_scalar opacity;
    br_scalar ka;
    br_scalar kd;
    br_scalar ks;
    br_scalar power;

    br_boolean lighting;
    br_boolean force_front;
    br_boolean force_back;
    br_scalar  depth_bias;

    br_token colour_source;
    br_token mapping_source;

    br_matrix23 map_transform;
} state_surface;

typedef struct state_light {
    br_token type;
    br_token lighting_space;

    br_vector3_f position;
    br_vector3_f direction;

    br_colour colour;

    br_scalar spot_outer;
    br_scalar spot_inner;

    br_scalar attenuation_l;
    br_scalar attenuation_c;
    br_scalar attenuation_q;
} state_light;

typedef void insert_cbfn(br_primitive *primitive, void *arg1, void *arg2, void *arg3, br_order_table *order_table,
                         br_scalar *z);

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
    PRIMF_COLOUR_WRITE_BIT,

    PRIMF_BLEND_BIT,
    PRIMF_MODULATE_BIT,
    PRIMF_COLOUR_KEY_BIT,
};

enum {
    PRIMF_DEPTH_WRITE  = (1 << PRIMF_DEPTH_WRITE_BIT),
    PRIMF_COLOUR_WRITE = (1 << PRIMF_COLOUR_WRITE_BIT),
    PRIMF_BLEND        = (1 << PRIMF_BLEND_BIT),
    PRIMF_MODULATE     = (1 << PRIMF_MODULATE_BIT),
    PRIMF_COLOUR_KEY   = (1 << PRIMF_COLOUR_KEY_BIT),
};

struct br_buffer_stored;

typedef struct state_primitive {
    /*
     * flags
     */
    br_uint_32 flags;

    /*
     * User selected ramp
     */
    br_int_32 index_base;
    br_int_32 index_range;

    /*
     * Type of colour
     */
    br_token colour_type;

    br_token depth_test;

    br_token blend_mode;

    /*
     * Type of perspective correct rendering
     */
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

    /* Texture filtering. BRT_NONE or BRT_LINEAR */
    br_token filter;

    /* Mipmap filtering. BRT_NONE or BRT_LINEAR */
    br_token mip_filter;
} state_primitive;

typedef struct state_output {
    struct br_device_pixelmap *colour;
    struct br_device_pixelmap *depth;
} state_output;

typedef struct state_stack {
    state_matrix matrix;

    /* Used for texture/materials */
    state_clip      clip[MAX_STATE_CLIP_PLANES];
    state_cull      cull;
    state_surface   surface;
    state_primitive prim;
    state_output    output;
    state_light     light[MAX_STATE_LIGHTS];
    state_hidden    hidden;

    br_uint_32 valid;
} state_stack;

typedef struct {
    GLuint fbo;

    alignas(16) shader_data_scene scene;

    struct {
        br_matrix4 p;
        br_matrix4 mv;
        br_matrix4 mvp;
        br_matrix4 normal;
        br_matrix4 environment;

        br_matrix4 view_to_model; /* Inverse of mv, needed for eye_model calc. */
        br_vector4 eye_m;
    } model;
} state_cache;

typedef struct state_all {
    state_stack  default_;
    state_stack *current;
    state_stack  stack[MAX_STATE_STACK];
    int          top;

    state_cache cache;

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

#endif /* STATE_H_ */
