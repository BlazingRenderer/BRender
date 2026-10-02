#ifndef SHADER_MAIN_H_
#define SHADER_MAIN_H_

/*
 * Both uniform blocks are bound to fixed binding points; the buffers behind them
 * are supplied per frame by the renderer's buffer rings.
 */
typedef struct br_gl_main_shader {
    GLuint program;

    struct {
        GLint aPosition; /* Vectex Position, vec3 */
        GLint aUV;       /* UV, vec2 */
        GLint aNormal;   /* Vertex Normal, vec3 */
        GLint aColour;   /* Vertex colour, vec4 */
    } attributes;

    struct {
        GLint main_texture;  /* sampler2D */
        GLint index_texture; /* usampler2D */
    } uniforms;

    GLuint block_index_scene;
    GLuint block_binding_scene;

    GLuint block_index_model;
    GLuint block_binding_model;

    GLint main_texture_binding;
    GLint index_texture_binding;
} br_gl_main_shader;

#pragma pack(push, 16)
typedef struct br_gl_main_data_light_info {
    alignas(4) uint32_t type;
    alignas(4) uint32_t attenuation_type;
    alignas(4) uint32_t _pad0;
    alignas(4) uint32_t _pad1;
} br_gl_main_data_light_info;
BR_STATIC_ASSERT(sizeof(br_gl_main_data_light_info) == sizeof(br_vector4), "sizeof(br_gl_main_data_light_info) != sizeof(br_vector4)");

typedef struct br_gl_main_data_light_atten {
    alignas(4) br_float intensity;
    alignas(4) br_float attenuation_c;
    alignas(4) br_float attenuation_l;
    alignas(4) br_float attenuation_q;
} br_gl_main_data_light_atten;
BR_STATIC_ASSERT(sizeof(br_gl_main_data_light_atten) == sizeof(br_vector4), "sizeof(br_gl_main_data_light_atten) != sizeof(br_vector4)");

typedef struct br_gl_main_data_light_radii {
    alignas(4) br_float spot_cos_inner;
    alignas(4) br_float spot_cos_outer;
    alignas(4) br_float radius_inner;
    alignas(4) br_float radius_outer;
} br_gl_main_data_light_radii;
BR_STATIC_ASSERT(sizeof(br_gl_main_data_light_radii) == sizeof(br_vector4), "sizeof(br_gl_main_data_light_radii) != sizeof(br_vector4)");

/*
 * One light, as the shader sees it.
 *
 * Kept as an array of these rather than seven parallel arrays: a draw only
 * carries the lights it can see, and with the fields interleaved the used part
 * of the block is a contiguous prefix, so the per-draw upload can stop early.
 * With parallel arrays, eight surviving lights still touch the whole block.
 */
typedef struct br_gl_main_data_light {
    alignas(16) br_gl_main_data_light_info info;   /* (type, atten_type, 0, 0) */
    alignas(16) br_vector4 position;               /* (X, Y, Z, w) */
    alignas(16) br_vector4 direction;              /* (X, Y, Z, 0), normalised */
    alignas(16) br_vector4 halfway;                /* (X, Y, Z, 0), normalised */
    alignas(16) br_vector4 colour;                 /* (R, G, B, 0) */
    alignas(16) br_gl_main_data_light_atten atten; /* (intensity, C, L, Q) */
    alignas(16) br_gl_main_data_light_radii radii; /* (cos(inner), cos(outer), radius_inner, radius_outer) */
} br_gl_main_data_light;

/*
 * The lights a single draw can see, packed so that each type occupies a
 * contiguous range and the shader's per-type loops stay affine. Rebuilt for
 * every draw - see StateGLBuildLightLists().
 */
typedef struct br_gl_main_data_lights {
    br_gl_main_data_light light[BR_MAX_LIGHTS];
} br_gl_main_data_lights;

typedef struct br_gl_main_data_scene {
    alignas(16) br_vector4 eye_view;
    alignas(16) br_vector4 clip_planes[BR_MAX_CLIP_PLANES];
    alignas(16) br_vector4 ambient_colour;
    alignas(4) uint32_t num_clip_planes;
    alignas(4) uint32_t use_ambient_colour;
} br_gl_main_data_scene;

typedef struct br_gl_main_data_model {
    alignas(16) br_matrix4 model_view;
    alignas(16) br_matrix4 projection;
    alignas(16) br_matrix4 mvp;
    alignas(16) br_matrix4 normal_matrix;
    alignas(16) br_matrix4 environment_matrix;
    alignas(16) br_matrix4 map_transform;
    alignas(16) br_vector4 surface_colour;
    alignas(16) br_vector4 eye_m;
    alignas(16) br_vector4 fog_colour;
    alignas(8) br_vector2 fog_range;
    alignas(4) float ka;
    alignas(4) float ks;
    alignas(4) float kd;
    alignas(4) float power;
    alignas(4) uint32_t lighting;
    alignas(4) uint32_t prelighting;
    alignas(4) uint32_t colour_source;
    alignas(4) uint32_t uv_source;
    alignas(4) uint32_t disable_colour_key;
    alignas(4) uint32_t texture_mode;
    alignas(4) uint32_t enable_fog;
    alignas(4) br_scalar fog_scale;
    alignas(4) uint32_t shading_mode;

    /*
     * The lights for this draw. The shader's loops are identical to a per-frame
     * light set; light_start/light_end just describe a smaller, repacked one.
     */
    alignas(16) br_vector4_i light_start;
    alignas(16) br_vector4_i light_end;
    br_gl_main_data_lights lights;
} br_gl_main_data_model;
#pragma pack(pop)

#endif /* SHADER_MAIN_H_ */
