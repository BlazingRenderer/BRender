/*
 * Renderer state: the stack, the default state and the per-part templates.
 *
 * Ported from glrend's state.h/state.c and state_*.c, with the GL cache dropped. The
 * state is pure storage - a deferred draw snapshots it, the bucket geometry restores it -
 * so nothing here touches the device. The template tables are what resolve a
 * (part, index, token) into a state_stack field; they are identical to glrend's, which is
 * why the layouts must stay the same.
 */
#include "drv.h"
#include "brassert.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(state_stack, f)
#define P(f) ((br_uintptr_t)(&(f)))

/*
 * Matrix
 */
static br_tv_template_entry matrix_template_entries[] = {
    // clang-format off
    {BRT(MODEL_TO_VIEW_M34_X),        F(matrix.model_to_view),            Q | S | AX, BRTV_CONV_M34_FIXED_SCALAR, 0, TM_PART | TM_M2V | TM_CLEAR_M2V_HINT | TM_INVALID_PM | TM_INVALID_V2M | TM_INVALID_M2S},
    {BRT(MODEL_TO_VIEW_M34_F),        F(matrix.model_to_view),            Q | S | AF, BRTV_CONV_M34_FLOAT_SCALAR, 0, TM_PART | TM_M2V | TM_CLEAR_M2V_HINT | TM_INVALID_PM | TM_INVALID_V2M | TM_INVALID_M2S},
    {BRT(VIEW_TO_ENVIRONMENT_M34_X),  F(matrix.view_to_environment),      Q | S | AX, BRTV_CONV_M34_FIXED_SCALAR, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(VIEW_TO_ENVIRONMENT_M34_F),  F(matrix.view_to_environment),      Q | S | AF, BRTV_CONV_M34_FLOAT_SCALAR, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(VIEW_TO_SCREEN_M4_X),        F(matrix.view_to_screen),           Q | S | AX, BRTV_CONV_M4_FIXED_SCALAR,  0, TM_PART | TM_V2S | TM_CLEAR_V2S_HINT | TM_V2S_HINT | TM_INVALID_PS | TM_INVALID_PM | TM_INVALID_M2S},
    {BRT(VIEW_TO_SCREEN_M4_F),        F(matrix.view_to_screen),           Q | S | AF, BRTV_CONV_M4_FLOAT_SCALAR,  0, TM_PART | TM_V2S | TM_CLEAR_V2S_HINT | TM_V2S_HINT | TM_INVALID_PS | TM_INVALID_PM | TM_INVALID_M2S},
    {BRT(MODEL_TO_VIEW_HINT_T),       F(matrix.model_to_view_hint),       Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_PM | TM_INVALID_V2M},
    {BRT(VIEW_TO_SCREEN_HINT_T),      F(matrix.view_to_screen_hint),      Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_V2S_HINT | TM_INVALID_PS | TM_INVALID_PM | TM_INVALID_M2S},
    {BRT(VIEW_TO_ENVIRONMENT_HINT_T), F(matrix.view_to_environment_hint), Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(HITHER_Z_X),                 F(matrix.hither_z),                 Q | S | A,  BRTV_CONV_FIXED_SCALAR,     0, TM_PART},
    {BRT(HITHER_Z_F),                 F(matrix.hither_z),                 Q | S | A,  BRTV_CONV_FLOAT_SCALAR,     0, TM_PART},
    {BRT(YON_Z_X),                    F(matrix.yon_z),                    Q | S | A,  BRTV_CONV_FIXED_SCALAR,     0, TM_PART},
    {BRT(YON_Z_F),                    F(matrix.yon_z),                    Q | S | A,  BRTV_CONV_FLOAT_SCALAR,     0, TM_PART},
    // clang-format on
};

static const state_matrix matrix_default_state = {
    .model_to_view       = {{
        BR_VECTOR3(1, 0, 0),
        BR_VECTOR3(0, 1, 0),
        BR_VECTOR3(0, 0, 1),
        BR_VECTOR3(0, 0, 0),
    }},
    .view_to_screen      = {{
        BR_VECTOR4(1, 0, 0, 0),
        BR_VECTOR4(0, 1, 0, 0),
        BR_VECTOR4(0, 0, 1, 0),
        BR_VECTOR4(0, 0, 0, 1),
    }},
    .view_to_environment = {{
        BR_VECTOR3(1, 0, 0),
        BR_VECTOR3(0, 1, 0),
        BR_VECTOR3(0, 0, 1),
        BR_VECTOR3(0, 0, 0),
    }},
    .model_to_view_hint  = BRT_LENGTH_PRESERVING,
    .view_to_screen_hint = BRT_PARALLEL,
};

/*
 * Cull
 */
static br_tv_template_entry cull_template_entries[] = {
    {BRT(TYPE_T), F(cull.type), Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_CC},
};

static const state_cull cull_default_state = {
    .type = BRT_NONE,
};

/*
 * Clip planes
 */
static br_tv_template_entry clip_template_entries[] = {
    {BRT(TYPE_T),     F(clip[0].type),  Q | S | A,  BRTV_CONV_COPY,            0, TM_PART | TM_INVALID_PS},
    {BRT(PLANE_V4_X), F(clip[0].plane), Q | S | AX, BRTV_CONV_V4_FIXED_SCALAR, 0, TM_PART | TM_INVALID_PS},
    {BRT(PLANE_V4_F), F(clip[0].plane), Q | S | AF, BRTV_CONV_V4_FLOAT_SCALAR, 0, TM_PART | TM_INVALID_PS},
};

static const state_clip clip_default_state = {
    .type  = BRT_NONE,
    .plane = BR_VECTOR4(0, 0, -1, 0),
};

/*
 * Surface
 */
static br_tv_template_entry surface_template_entries[] = {
    // clang-format off
    {BRT(COLOUR_RGB),       F(surface.colour),         Q | S | AX, BRTV_CONV_COPY,             0, TM_PART                },

    {BRT(OPACITY_X),        F(surface.opacity),        Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT(OPACITY_F),        F(surface.opacity),        Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },
    {BRT(AMBIENT_X),        F(surface.ka),             Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT(AMBIENT_F),        F(surface.ka),             Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },
    {BRT(DIFFUSE_X),        F(surface.kd),             Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT(DIFFUSE_F),        F(surface.kd),             Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },
    {BRT(SPECULAR_X),       F(surface.ks),             Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT(SPECULAR_F),       F(surface.ks),             Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },
    {BRT(SPECULAR_POWER_X), F(surface.power),          Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT(SPECULAR_POWER_F), F(surface.power),          Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },

    {BRT(LIGHTING_B),       F(surface.lighting),       Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},
    {BRT(PRELIGHTING_B),    F(surface.prelighting),    Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},
    {BRT(FORCE_FRONT_B),    F(surface.force_front),    Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},
    {BRT(FORCE_BACK_B),     F(surface.force_back),     Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},
    {BRT(DEPTH_BIAS_X),     F(surface.depth_bias),     Q | S | A,  BRTV_CONV_FIXED_SCALAR,     0, TM_PART | TM_INVALID_CC},
    {BRT(DEPTH_BIAS_F),     F(surface.depth_bias),     Q | S | A,  BRTV_CONV_FLOAT_SCALAR,     0, TM_PART | TM_INVALID_CC},

    {BRT(COLOUR_SOURCE_T),  F(surface.colour_source),  Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},
    {BRT(OPACITY_SOURCE_T), F(surface.opacity_source), Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},
    {BRT(MAPPING_SOURCE_T), F(surface.mapping_source), Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},

    {BRT(MAP_MATRIX_M23_X), F(surface.map_transform),  Q | S | AX, BRTV_CONV_M23_FIXED_SCALAR, 0, TM_PART | TM_INVALID_CC},
    {BRT(MAP_MATRIX_M23_F), F(surface.map_transform),  Q | S | AF, BRTV_CONV_M23_FLOAT_SCALAR, 0, TM_PART | TM_INVALID_CC},
    // clang-format on
};

static const state_surface surface_default_state = {
    .colour  = BR_COLOUR_RGB(255, 255, 255),
    .opacity = BR_SCALAR(1.0),
    .ka      = BR_SCALAR(0.0),
    .kd      = BR_SCALAR(0.75),
    .ks      = BR_SCALAR(0.0),
    .power   = BR_SCALAR(20.0),

    .lighting       = BR_FALSE,
    .prelighting    = BR_FALSE,
    .force_front    = BR_FALSE,
    .force_back     = BR_FALSE,
    .depth_bias     = BR_SCALAR(0.0),
    .colour_source  = BRT_SURFACE,
    .opacity_source = BRT_SURFACE,
    .mapping_source = BRT_GEOMETRY_MAP,

    .map_transform = {{
        BR_VECTOR2(1, 0),
        BR_VECTOR2(0, 1),
        BR_VECTOR2(0, 0),
    }},
};

/*
 * Primitive
 */
static br_tv_template_entry primitive_template_entries[] = {
    // clang-format off
    {BRT(DEPTH_WRITE_B),           F(prim.flags),               Q | S | A, BRTV_CONV_BIT,          PRIMF_DEPTH_WRITE, 1},
    {BRT(BLEND_B),                 F(prim.flags),               Q | S | A, BRTV_CONV_BIT,          PRIMF_BLEND,       1},
    {BRT(MODULATE_B),              F(prim.flags),               Q | S | A, BRTV_CONV_BIT,          PRIMF_MODULATE,    1},
    {BRT(COLOUR_KEY_B),            F(prim.flags),               Q | S | A, BRTV_CONV_BIT,          PRIMF_COLOUR_KEY,  1},

    {BRT(COLOUR_T),                F(prim.colour_type),         Q | S | A, BRTV_CONV_COPY,         0,                 1},

    {BRT(DEPTH_TEST_T),            F(prim.depth_test),          Q | S | A, BRTV_CONV_COPY,         0,                 1},
    {BRT(BLEND_MODE_T),            F(prim.blend_mode),          Q | S | A, BRTV_CONV_COPY,         0,                 1},
    {BRT(MAP_WIDTH_LIMIT_T),       F(prim.map_width_limit),     Q | S | A, BRTV_CONV_COPY,         0,                 1},
    {BRT(MAP_HEIGHT_LIMIT_T),      F(prim.map_height_limit),    Q | S | A, BRTV_CONV_COPY,         0,                 1},
    {BRT(SHADING_MODE_T),          F(prim.shading_mode),        Q | S | A, BRTV_CONV_COPY,         0,                 1},

    {BRT(INDEX_BASE_I32),          F(prim.index_base),          Q | S | A, BRTV_CONV_COPY,         0,                 1},
    {BRT(INDEX_RANGE_I32),         F(prim.index_range),         Q | S | A, BRTV_CONV_COPY,         0,                 1},

    {BRT(PERSPECTIVE_T),           F(prim.perspective_type),    Q | S | A, BRTV_CONV_COPY,         0,                 1},
    {BRT(PERSPECTIVE_B),           F(prim.perspective_type),    S,         BRTV_CONV_BOOL_TOKEN,   BRT_DEFAULT,       1},

    {BRT(SUBDIVIDE_TOLERANCE_I32), F(prim.subdivide_tolerance), Q | S | A, BRTV_CONV_COPY,         0,                 1},

    {BRT(COLOUR_MAP_O),            F(prim.colour_map),          Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(TEXTURE_O),               F(prim.colour_map),          Q | S,     BRTV_CONV_COPY,         0,                 0},
    {BRT(INDEX_SHADE_O),           F(prim.index_shade),         Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(INDEX_BLEND_O),           F(prim.index_blend),         Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(SCREEN_DOOR_O),           F(prim.screendoor),          Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(INDEX_LIGHT_O),           F(prim.lighting),            Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(BUMP_O),                  F(prim.bump),                Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(INDEX_FOG_O),             F(prim.index_fog),           Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(MAP_INTERPOLATION_T),     F(prim.filter),              Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(MIP_INTERPOLATION_T),     F(prim.mip_filter),          Q | S | A, BRTV_CONV_COPY,         0,                 0},

    {BRT(FOG_T),                   F(prim.fog_type),            Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(FOG_MIN_X),               F(prim.fog_min),             Q | S | A, BRTV_CONV_FIXED_SCALAR, 0,                 0},
    {BRT(FOG_MIN_F),               F(prim.fog_min),             Q | S | A, BRTV_CONV_FLOAT_SCALAR, 0,                 0},
    {BRT(FOG_MAX_X),               F(prim.fog_max),             Q | S | A, BRTV_CONV_FIXED_SCALAR, 0,                 0},
    {BRT(FOG_MAX_F),               F(prim.fog_max),             Q | S | A, BRTV_CONV_FLOAT_SCALAR, 0,                 0},
    {BRT(FOG_RGB),                 F(prim.fog_colour),          Q | S | A, BRTV_CONV_COPY,         0,                 0},
    {BRT(FOG_SCALE_U8),            F(prim.fog_scale),           Q | S | A, BRTV_CONV_COPY,         0,                 0},
    // clang-format on
};

static const state_primitive primitive_default_state = {
    .flags               = PRIMF_DEPTH_WRITE,
    .index_base          = 0,
    .index_range         = 0,
    .colour_type         = BRT_DEFAULT,
    .depth_test          = BRT_LESS,
    .blend_mode          = BRT_NONE,
    .map_width_limit     = BRT_WRAP,
    .map_height_limit    = BRT_WRAP,
    .shading_mode        = BRT_FLAT,
    .perspective_type    = BRT_NONE,
    .subdivide_tolerance = 0,
    .colour_map          = NULL,
    .index_shade         = NULL,
    .index_blend         = NULL,
    .index_fog           = NULL,
    .screendoor          = NULL,
    .lighting            = NULL,
    .bump                = NULL,
    .filter              = BRT_NONE,
    .mip_filter          = BRT_NONE,
    .fog_type            = BRT_NONE,
    .fog_min             = BR_SCALAR(256),
    .fog_max             = BR_SCALAR(512),
    .fog_colour          = BR_COLOUR_RGBA(255, 255, 255, 0),
    .fog_scale           = 255,
};

/*
 * Output
 */
static br_tv_template_entry output_template_entries[] = {
    {BRT(COLOUR_BUFFER_O), F(output.colour), Q | S | A, BRTV_CONV_COPY},
    {BRT(DEPTH_BUFFER_O),  F(output.depth),  Q | S | A, BRTV_CONV_COPY},
};

static const state_output output_default_state = {
    .colour = NULL,
    .depth  = NULL,
};

/*
 * Hidden surface
 */
static br_tv_template_entry hidden_template_entries[] = {
    // clang-format off
    {BRT(TYPE_T),              F(hidden.type),        Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(DIVERT_T),            F(hidden.divert),      Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(V1ORDER_TABLE_P),     F(hidden.order_table), Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
    {BRT(V1PRIMITIVE_HEAP_P),  F(hidden.heap),        Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(V1INSERT_FUNCTION_P), F(hidden.insert_fn),   Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
    {BRT(V1INSERT_ARG1_P),     F(hidden.insert_arg1), Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
    {BRT(V1INSERT_ARG2_P),     F(hidden.insert_arg2), Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
    {BRT(V1INSERT_ARG3_P),     F(hidden.insert_arg3), Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
    // clang-format on
};

static const state_hidden hidden_default_state = {
    .type        = BRT_NONE,
    .divert      = BRT_NONE,
    .order_table = NULL,
    .heap        = NULL,
    .insert_fn   = NULL,
    .insert_arg1 = NULL,
    .insert_arg2 = NULL,
    .insert_arg3 = NULL,
};

/*
 * Lights
 */
static br_error BR_CALLBACK LightingVolumeSet(void *block, const br_value *pvalue, const struct br_tv_template_entry *tep, void *context);

static const struct br_tv_custom lighting_volume_conv = {
    .query      = NULL,
    .set        = LightingVolumeSet,
    .extra_size = NULL,
};

static br_tv_template_entry light_template_entries[] = {
    // clang-format off
    {BRT(TYPE_T),          F(light[0].type),           Q | S | A,  BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(POSITION_V3_X),   F(light[0].position),       Q | S | AX, BRTV_CONV_V3_FIXED_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(POSITION_V3_F),   F(light[0].position),       Q | S | AF, BRTV_CONV_V3_FLOAT_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(DIRECTION_V3_X),  F(light[0].direction),      Q | S | AX, BRTV_CONV_V3_FIXED_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(DIRECTION_V3_F),  F(light[0].direction),      Q | S | AF, BRTV_CONV_V3_FLOAT_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(SPOT_INNER_X),    F(light[0].spot_inner),     Q | S | AX, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(SPOT_INNER_F),    F(light[0].spot_inner),     Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(SPOT_OUTER_X),    F(light[0].spot_outer),     Q | S | AX, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(SPOT_OUTER_F),    F(light[0].spot_outer),     Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ANGLE_OUTER_A),   F(light[0].angle_outer),    Q | S | AF, BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(RADIUS_INNER_X),  F(light[0].radius_inner),   Q | S | AX, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(RADIUS_INNER_F),  F(light[0].radius_inner),   Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(RADIUS_OUTER_X),  F(light[0].radius_outer),   Q | S | AX, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(RADIUS_OUTER_F),  F(light[0].radius_outer),   Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(COLOUR_RGB),      F(light[0].colour),         Q | S | AF, BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(ATTENUATION_C_F), F(light[0].attenuation_c),  Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_C_X), F(light[0].attenuation_c),  Q | S | AF, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_L_F), F(light[0].attenuation_l),  Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_L_X), F(light[0].attenuation_l),  Q | S | AF, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_Q_F), F(light[0].attenuation_q),  Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_Q_X), F(light[0].attenuation_q),  Q | S | AF, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(ATTENUATION_TYPE_T), F(light[0].attenuation_type), Q | S | AF, BRTV_CONV_COPY,       0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_HINT_T), F(light[0].attenuation_hint), Q | S | AF, BRTV_CONV_COPY,       0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(RADIUS_CULL_B),   F(light[0].radius_cull),    Q | S | AF, BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ANGLE_CULL_B),    F(light[0].angle_cull),     Q | S | AF, BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(LIGHTING_VOLUME_P), F(light[0].volume),           S,      BRTV_CONV_CUSTOM,          P(lighting_volume_conv), TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(CULLED_B),        F(light[0].culled),         Q | S | AF, BRTV_CONV_COPY,            0, TM_INVALID_PM},
    // clang-format on
};

static const state_light light_default_state = {
    .type             = BRT_NONE,
    .position         = BR_VECTOR3(0, 0, 0),
    .direction        = BR_VECTOR3(0, 0, 0),
    .colour           = BR_COLOUR_RGB(255, 255, 255),
    .spot_outer       = BR_SCALAR(0.866025403),
    .spot_inner       = BR_SCALAR(0.93969262),
    .radius_outer     = BR_SCALAR(0.0),
    .radius_inner     = BR_SCALAR(0.0),
    .angle_outer      = BR_ANGLE_DEG(30.0),
    .attenuation_l    = BR_SCALAR(1.0),
    .attenuation_c    = BR_SCALAR(0.0),
    .attenuation_q    = BR_SCALAR(0.0),
    .attenuation_type = BRT_QUADRATIC,
    .attenuation_hint = BRT_CONSTANT,
    .radius_cull      = BR_FALSE,
    .angle_cull       = BR_FALSE,
    .volume           = {.falloff_distance = BR_SCALAR(0.0)},
    .culled           = BR_FALSE,
};

/*
 * The per-plane and per-light templates need one copy per index with the offsets folded
 * in, so they cannot be built straight from a single table the way the others can.
 */
static br_tv_template_entry clip_states[MAX_STATE_CLIP_PLANES][BR_ASIZE(clip_template_entries)];
static br_tv_template_entry light_states[MAX_STATE_LIGHTS][BR_ASIZE(light_template_entries)];

static void StateVkInitMatrix(state_all *state)
{
    state->templates.matrix = BrTVTemplateAllocate(state->res, matrix_template_entries, BR_ASIZE(matrix_template_entries));

    state->default_.matrix = matrix_default_state;
    state->default_.valid |= MASK_STATE_MATRIX;
}

static void StateVkInitCull(state_all *state)
{
    state->templates.cull = BrTVTemplateAllocate(state->res, cull_template_entries, BR_ASIZE(cull_template_entries));

    state->default_.cull = cull_default_state;
    state->default_.valid |= MASK_STATE_CULL;
}

static void StateVkInitClip(state_all *state)
{
    for(int i = 0; i < MAX_STATE_CLIP_PLANES; ++i) {
        BrMemCpy(&clip_states[i], clip_template_entries, sizeof(clip_template_entries));

        for(int j = 0; j < BR_ASIZE(clip_template_entries); ++j)
            clip_states[i][j].offset += sizeof(state_clip) * i;

        state->templates.clip[i] = BrTVTemplateAllocate(state->res, clip_states[i], BR_ASIZE(clip_states[i]));
        state->default_.clip[i]  = clip_default_state;
    }

    state->default_.valid |= MASK_STATE_CLIP;
}

static void StateVkInitSurface(state_all *state)
{
    state->templates.surface = BrTVTemplateAllocate(state->res, surface_template_entries, BR_ASIZE(surface_template_entries));

    state->default_.surface = surface_default_state;
    state->default_.valid |= MASK_STATE_SURFACE;
}

static void StateVkInitPrimitive(state_all *state)
{
    state->templates.prim = BrTVTemplateAllocate(state->res, primitive_template_entries, BR_ASIZE(primitive_template_entries));

    state->default_.prim = primitive_default_state;
    state->default_.valid |= MASK_STATE_PRIMITIVE;
}

static void StateVkInitOutput(state_all *state)
{
    state->templates.output = BrTVTemplateAllocate(state->res, output_template_entries, BR_ASIZE(output_template_entries));

    state->default_.output = output_default_state;
    state->default_.valid |= MASK_STATE_OUTPUT;
}

static void StateVkInitHidden(state_all *state)
{
    state->templates.hidden = BrTVTemplateAllocate(state->res, hidden_template_entries, BR_ASIZE(hidden_template_entries));

    state->default_.hidden = hidden_default_state;
}

static void StateVkInitLight(state_all *state)
{
    for(int i = 0; i < MAX_STATE_LIGHTS; ++i) {
        BrMemCpy(&light_states[i], light_template_entries, sizeof(light_template_entries));

        for(int j = 0; j < BR_ASIZE(light_template_entries); ++j)
            light_states[i][j].offset += sizeof(state_light) * i;

        state->templates.light[i] = BrTVTemplateAllocate(state->res, light_states[i], BR_ASIZE(light_states[i]));
        state->default_.light[i]  = light_default_state;
    }

    state->default_.valid |= MASK_STATE_LIGHT;
}

void StateVkInit(state_all *state, void *res)
{
    state->res = res;

    StateVkInitMatrix(state);
    StateVkInitClip(state);
    StateVkInitCull(state);
    StateVkInitSurface(state);
    StateVkInitPrimitive(state);
    StateVkInitOutput(state);
    StateVkInitHidden(state);
    StateVkInitLight(state);

    state->current = state->stack;
    StateVkCopy(state->current, &state->default_, ~0u);
}

void StateVkCopy(state_stack *dst, const state_stack *src, uint32_t mask)
{
    /* Restrict the copy mask to the valid parts. */
    mask &= src->valid;

    /* Merge the valid mask into the destination. */
    dst->valid |= mask;

    if(mask & MASK_STATE_MATRIX)
        dst->matrix = src->matrix;

    if(mask & MASK_STATE_CLIP) {
        for(int i = 0; i < MAX_STATE_CLIP_PLANES; ++i)
            dst->clip[i] = src->clip[i];
    }

    if(mask & MASK_STATE_CULL)
        dst->cull = src->cull;

    if(mask & MASK_STATE_SURFACE)
        dst->surface = src->surface;

    if(mask & MASK_STATE_PRIMITIVE)
        dst->prim = src->prim;

    if(mask & MASK_STATE_OUTPUT)
        dst->output = src->output;

    if(mask & MASK_STATE_LIGHT) {
        for(int i = 0; i < MAX_STATE_LIGHTS; ++i)
            dst->light[i] = src->light[i];
    }
}

br_boolean StateVkPush(state_all *state, uint32_t mask)
{
    (void)mask;

    if(state->top >= MAX_STATE_STACK)
        return BR_FALSE;

    state_stack *old = state->current;
    ++state->top;
    ++state->current;

    /*
     * The whole frame is copied; a part-by-part copy would leave parts the mask does not
     * name stale, and the copy is cheap enough that the bookkeeping is not worth it.
     */
    *state->current = *old;
    return BR_TRUE;
}

br_boolean StateVkPop(state_all *state, uint32_t mask)
{
    (void)mask;

    if(state->top <= 0)
        return BR_FALSE;

    --state->top;
    --state->current;
    return BR_TRUE;
}

void StateVkDefault(state_all *state, uint32_t mask)
{
    StateVkCopy(state->current, &state->default_, mask);
}

void StateVkTemplateActions(state_all *state, uint32_t mask)
{
    (void)state;
    (void)mask;
}

struct br_tv_template *StateVkGetStateTemplate(state_all *state, br_token part, br_int_32 index)
{
    switch(part) {
        case BRT_MATRIX:
            return state->templates.matrix;

        case BRT_CLIP:
            if(index >= MAX_STATE_CLIP_PLANES)
                return NULL;

            return state->templates.clip[index];

        case BRT_CULL:
            return state->templates.cull;

        case BRT_SURFACE:
            return state->templates.surface;

        case BRT_PRIMITIVE:
            return state->templates.prim;

        case BRT_HIDDEN_SURFACE:
            return state->templates.hidden;

        case BRT_LIGHT:
            if(index >= MAX_STATE_LIGHTS)
                return NULL;

            return state->templates.light[index];

        case BRT_OUTPUT:
            return state->templates.output;

        case BRT_ENABLE:
        case BRT_BOUNDS:
        default:
            break;
    }

    return NULL;
}

/*
 * Copy a light's cutoff volume into the renderer's per-light slot, as glrend's
 * customLightingVolumeSet() does. The core transforms the volume into view space and frees
 * it as soon as the set returns, so the regions and planes are copied here, not kept. They
 * cannot ride in state_light: the state stack is copied whole by StateVkPush(), and the
 * payload would be copied with every push. The descriptor in state_light names the slot.
 *
 * The template entry targets light[i].volume and its offset already has the light's part
 * folded in (StateVkInitLight()), so the part falls out of the field's address.
 */
static br_error BR_CALLBACK LightingVolumeSet(void *block, const br_value *pvalue, const struct br_tv_template_entry *tep, void *context)
{
    const br_light_volume   *src             = (const br_light_volume *)pvalue->p;
    state_stack             *stack           = (state_stack *)block;
    br_renderer             *renderer        = context;
    state_light_volume_desc *desc            = (state_light_volume_desc *)((char *)block + tep->offset);
    static br_boolean        warned_overflow = BR_FALSE;
    state_light_volume      *dst;
    br_uint_32               num_planes = 0;
    br_uint_32               index;

    index = (br_uint_32)((state_light *)((char *)desc - offsetof(state_light, volume)) - stack->light);

    ASSERT(renderer != NULL);
    ASSERT(index < (br_uint_32)BR_ASIZE(stack->light));

    dst = &renderer->state.light_volume[index];

    /*
     * No volume, or one with no regions, is the same as not having one. Both the descriptor
     * and the slot are cleared, so a light that lost its volume cannot leave stale data
     * behind.
     */
    desc->falloff_distance = BR_SCALAR(0.0);
    desc->region_count     = 0;
    dst->falloff_distance  = BR_SCALAR(0.0);
    dst->region_count      = 0;

    if(src == NULL || src->regions == NULL || src->nregions == 0)
        return BRE_OK;

    for(br_uint_32 i = 0; i < src->nregions; ++i)
        num_planes += src->regions[i].nplanes;

    if(src->nregions > MAX_STATE_LIGHT_VOLUME_REGIONS || num_planes > MAX_STATE_LIGHT_VOLUME_PLANES) {

        if(!warned_overflow) {
            BrLogWarn("VKREND", "Light volume has %u regions and %u planes, more than the %d/%d vkrend supports; dropping it.",
                      (unsigned)src->nregions, (unsigned)num_planes, MAX_STATE_LIGHT_VOLUME_REGIONS, MAX_STATE_LIGHT_VOLUME_PLANES);
            BrLogWarn("VKREND", "This message will not be shown again.");
            warned_overflow = BR_TRUE;
        }

        return BRE_OK;
    }

    desc->falloff_distance = src->falloff_distance;
    desc->region_count     = src->nregions;
    dst->falloff_distance  = src->falloff_distance;
    dst->region_count      = src->nregions;

    br_uint_32 offset = 0;
    for(br_uint_32 i = 0; i < src->nregions; ++i) {
        const br_convex_region *region = src->regions + i;

        dst->regions[i].plane_offset = offset;
        dst->regions[i].plane_count  = region->nplanes;

        for(br_uint_32 j = 0; j < region->nplanes; ++j)
            dst->planes[offset + j] = region->planes[j];

        offset += region->nplanes;
    }

    return BRE_OK;
}

/*
 * Stored renderer state
 */
static const struct br_renderer_state_stored_dispatch rendererStateStoredDispatch;

#define SS_F(f) offsetof(struct br_renderer_state_stored, f)

static br_tv_template_entry rendererStateStoredTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR), SS_F(identifier),  BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(RENDERER_O),      SS_F(renderer),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(PARTS_U32),       SS_F(state.valid), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef SS_F

br_renderer_state_stored *RendererStateStoredVkAllocate(br_renderer *renderer, state_stack *base_state, br_uint_32 m, br_token_value *tv)
{
    br_renderer_state_stored *self;

    (void)tv;

    self             = BrResAllocate(renderer, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch   = &rendererStateStoredDispatch;
    self->identifier = "Renderer-State-Stored";
    self->device     = ObjectDevice(renderer);
    self->renderer   = renderer;

    StateVkCopy(&self->state, base_state, m);
    ObjectContainerAddFront(renderer, (br_object *)self);
    return self;
}

static void BR_CMETHOD_DECL(br_renderer_state_stored_vk, free)(br_object *_self)
{
    br_renderer_state_stored *self = (br_renderer_state_stored *)_self;

    ObjectContainerRemove(self->renderer, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_renderer_state_stored_vk, identifier)(br_object *self)
{
    return ((br_renderer_state_stored *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_renderer_state_stored_vk, type)(br_object *self)
{
    (void)self;
    return BRT_RENDERER_STATE_STORED;
}

static br_boolean BR_CMETHOD_DECL(br_renderer_state_stored_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_RENDERER_STATE_STORED) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_renderer_state_stored_vk, device)(br_object *self)
{
    return ((br_renderer_state_stored *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_renderer_state_stored_vk, space)(br_object *self)
{
    (void)self;
    return sizeof(br_renderer_state_stored);
}

static br_tv_template *BR_CMETHOD_DECL(br_renderer_state_stored_vk, templateQuery)(br_object *_self)
{
    br_renderer_state_stored *self = (br_renderer_state_stored *)_self;

    if(self->device->templates.rendererStateStoredTemplate == NULL) {
        self->device->templates.rendererStateStoredTemplate = BrTVTemplateAllocate(self->device, rendererStateStoredTemplateEntries,
                                                                                   BR_ASIZE(rendererStateStoredTemplateEntries));
    }

    return self->device->templates.rendererStateStoredTemplate;
}

static const struct br_renderer_state_stored_dispatch rendererStateStoredDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_renderer_state_stored_vk, free),
    ._identifier = BR_CMETHOD_REF(br_renderer_state_stored_vk, identifier),
    ._type       = BR_CMETHOD_REF(br_renderer_state_stored_vk, type),
    ._isType     = BR_CMETHOD_REF(br_renderer_state_stored_vk, isType),
    ._device     = BR_CMETHOD_REF(br_renderer_state_stored_vk, device),
    ._space      = BR_CMETHOD_REF(br_renderer_state_stored_vk, space),

    ._templateQuery = BR_CMETHOD_REF(br_renderer_state_stored_vk, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),
};
