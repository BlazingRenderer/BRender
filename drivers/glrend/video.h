/*
 * VIDEO structures
 */
#ifndef VIDEO_H_
#define VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VIDEO {
    GLint   maxUniformBlockSize;
    GLint   maxUniformBufferBindings;
    GLint   maxVertexUniformBlocks;
    GLint   maxFragmentUniformBlocks;
    GLint   maxSamples;
    GLfloat maxAnisotropy;

    struct {
        GLuint program;
        GLint  aPosition;     /* Position, vec3 */
        GLint  aColour;       /* Colour, vec3 */
        GLint  aUV;           /* UV, vec2 */
        GLint  uSampler;      /* Sampler, sampler2D */
        GLint  uMVP;          /* Model-View-Projection Matrix, mat4 */
        GLint  uVerticalFlip; /* Boolean (actually a float). Should the image be vertically flipped. */
    } defaultProgram;

    struct {
        GLuint program;
        GLint  aPosition; /* Position, vec3 */
        GLint  aUV;       /* UV, vec2 */
        GLint  uSampler;  /* Sampler, sampler2D */
        GLint  uMVP;      /* Model-View-Projection Matrix, mat4 */
        GLint  uColour;   /* Text colour, vec3 */
    } textProgram;

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
        } uniforms;

        GLuint uboScene;
        GLuint blockIndexScene;
        GLuint blockBindingScene;

        GLuint uboModel;
        GLuint blockIndexModel;
        GLuint blockBindingModel;

        GLint mainTextureBinding;
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
    /* (intensity, constant, linear, attenutation) */
    alignas(16) br_vector4 iclq;
    /* (inner, outer), if (0.0, 0.0), then this is a point light. */
    alignas(16) br_vector2 spot_angles;

    /* Pad out the structure to maintain alignment. */
    alignas(4) float _pad0, _pad1;
} shader_data_light;
BR_STATIC_ASSERT(sizeof(shader_data_light) % 16 == 0, "shader_data_light is not aligned");

typedef struct shader_data_scene {
    alignas(16) br_vector4 eye_view;
    alignas(16) shader_data_light lights[BR_MAX_LIGHTS];
    alignas(4) uint32_t num_lights;
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
    alignas(16) br_vector4 clear_colour;
    alignas(16) br_vector4 eye_m;
    alignas(4) float ka;
    alignas(4) float ks;
    alignas(4) float kd;
    alignas(4) float power;
    alignas(4) uint32_t unlit;
    alignas(4) uint32_t uv_source;
    alignas(4) uint32_t disable_colour_key;
} shader_data_model;
#pragma pack(pop)

#ifdef __cplusplus
};
#endif
#endif /* VIDEO_H_ */
