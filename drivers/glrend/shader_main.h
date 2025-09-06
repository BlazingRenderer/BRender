#ifndef SHADER_MAIN_H_
#define SHADER_MAIN_H_

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

    GLuint ubo_scene;
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

typedef struct br_gl_main_data_scene {
    alignas(16) br_vector4 eye_view;
    alignas(16) br_gl_main_data_light_info light_info[BR_MAX_LIGHTS];   /* (type, atten_type, 0, 0) */
    alignas(16) br_vector4 light_positions[BR_MAX_LIGHTS];              /* (X, Y, Z, 0) */
    alignas(16) br_vector4 light_directions[BR_MAX_LIGHTS];             /* (X, Y, Z, 0), normalised */
    alignas(16) br_vector4 light_halfs[BR_MAX_LIGHTS];                  /* (X, Y, Z, 0), normalised */
    alignas(16) br_vector4 light_colours[BR_MAX_LIGHTS];                /* (R, G, B, 0)   */
    alignas(16) br_gl_main_data_light_atten light_atten[BR_MAX_LIGHTS]; /* (1/C, C, L, Q) */
    alignas(16) br_gl_main_data_light_radii light_radii[BR_MAX_LIGHTS]; /* (cos(inner), cos(outer), radius_inner, radius_outer) */
    alignas(16) br_vector4 clip_planes[BR_MAX_CLIP_PLANES];
    alignas(16) br_vector4 ambient_colour;
    alignas(16) br_vector4_i light_start;
    alignas(16) br_vector4_i light_end;
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
    alignas(4) uint32_t unlit;
    alignas(4) uint32_t uv_source;
    alignas(4) uint32_t disable_colour_key;
    alignas(4) uint32_t texture_mode;
    alignas(4) uint32_t enable_fog;
    alignas(4) br_scalar fog_scale;
} br_gl_main_data_model;
#pragma pack(pop)

#endif /* SHADER_MAIN_H_ */
