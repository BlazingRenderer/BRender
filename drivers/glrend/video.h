/*
 * VIDEO structures
 */
#ifndef VIDEO_H_
#define VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct br_pixelmap_gl_fmt {
    br_uint_8  pm_type;
    GLint      internal_format;
    GLenum     format;
    GLenum     type;
    GLsizeiptr bytes;
    br_boolean blended;
    br_boolean indexed;
    GLint      swizzle_r;
    GLint      swizzle_g;
    GLint      swizzle_b;
    GLint      swizzle_a;
} br_pixelmap_gl_fmt;

typedef struct _VIDEO {
    GLint   maxUniformBlockSize;
    GLint   maxUniformBufferBindings;
    GLint   maxVertexUniformBlocks;
    GLint   maxFragmentUniformBlocks;
    GLint   maxSamples;
    GLfloat maxAnisotropy;

    struct {
        GLuint program;
        GLint  uSampler;      /* Sampler, sampler2D */
        GLint  uIndexTex;     /* Sampler, usampler2D */

        GLuint block_index_rect_data;
        GLuint block_binding_rect_data;

        GLuint vao;
        GLuint ubo;
    } rectProgram;

    struct {
        GLuint program;
        GLint  uSampler;  /* Sampler, sampler2D */

        GLuint block_index_font_data;
        GLuint block_binding_font_data;

        GLuint vao_glyphs;
        GLuint ubo_glyphs;
        GLuint block_index_glyphs;
        GLuint block_binding_glyphs;
    } textProgram;

    struct {
        GLuint program;
        GLuint block_index_line_data;
        GLuint block_binding_line_data;

        GLuint vao;
        GLuint ubo;
    } lineProgram;

    struct {
        GLuint program;

        struct {
            GLint aPosition; /* Vectex Position, vec3 */
            GLint aUV;       /* UV, vec2 */
            GLint aNormal;   /* Vertex Normal, vec3 */
            GLint aColour;   /* Vertex colour, vec4 */
        } attributes;

        struct {
            GLint main_texture; /* sampler2D */
            GLint index_texture; /* usampler2D */
        } uniforms;

        GLuint uboScene;
        GLuint blockIndexScene;
        GLuint blockBindingScene;

        GLuint blockIndexModel;
        GLuint blockBindingModel;

        GLint mainTextureBinding;
        GLint indexTextureBinding;
    } brenderProgram;
} VIDEO, *HVIDEO;

#pragma pack(push, 16)
/* std140-compatible light structure */
typedef struct shader_data_light {
    /* (X, Y, Z, T), if T == 0, direct, otherwise point/spot */
    alignas(16) br_vector4 position;
    /* (X, Y, Z, 0), normalised */
    alignas(16) br_vector4 direction;
    /* (X, Y, Z, 0), normalised */
    alignas(16) br_vector4 half;
    /* (R, G, B, 0) */
    alignas(16) br_vector4 colour;
    alignas(4) float intensity;
    alignas(4) float attenuation_c;
    alignas(4) float attenuation_l;
    alignas(4) float attenuation_q;
    alignas(4) float spot_inner_cos;
    alignas(4) float spot_outer_cos;
    alignas(4) uint32_t light_type;
    alignas(4) uint32_t attenuation_type;
    alignas(4) float radius_inner;
    alignas(4) float radius_outer;
    alignas(4) float _pad0;
    alignas(4) float _pad1;
} shader_data_light;
BR_STATIC_ASSERT(sizeof(shader_data_light) % 16 == 0, "shader_data_light is not aligned");

typedef struct shader_data_scene {
    alignas(16) br_vector4 eye_view;
    alignas(16) shader_data_light lights[BR_MAX_LIGHTS];
    alignas(16) br_vector4 clip_planes[BR_MAX_CLIP_PLANES];
    alignas(16) br_vector4 ambient_colour;
    alignas(16) br_vector4_i light_start;
    alignas(16) br_vector4_i light_end;
    alignas(4) uint32_t num_clip_planes;
    alignas(4) uint32_t use_ambient_colour;
} shader_data_scene;
BR_STATIC_ASSERT(sizeof(((shader_data_scene *)NULL)->lights) == sizeof(shader_data_light) * BR_MAX_LIGHTS,
                 "std::array<shader_data_light> fucked up");

typedef struct shader_data_model {
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
    alignas(4) uint32_t is_indexed;
    alignas(4) uint32_t is_filtered;
    alignas(4) uint32_t enable_fog;
    alignas(4) br_scalar fog_scale;
} shader_data_model;
#pragma pack(pop)

#ifdef __cplusplus
};
#endif
#endif /* VIDEO_H_ */
