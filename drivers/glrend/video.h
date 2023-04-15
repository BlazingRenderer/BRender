/*
 * VIDEO structures
 */
#ifndef _VIDEO_H_
#define _VIDEO_H_

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
typedef struct _GLSTD140_LIGHT {
    /* (X, Y, Z, T), if T == 0, direct, otherwise point/spot */
    alignas(16) br_vector4_f position;
    /* (X, Y, Z, 0), normalised */
    alignas(16) br_vector4_f direction;
    /* (X, Y, Z, 0), normalised */
    alignas(16) br_vector4_f half;
    /* (R, G, B, 0) */
    alignas(16) br_vector4_f colour;
    /* (intensity, constant, linear, attenutation) */
    alignas(16) br_vector4_f iclq;
    /* (inner, outer), if (0.0, 0.0), then this is a point light. */
    alignas(16) br_vector2_f spot_angles;

    /* Pad out the structure to maintain alignment. */
    alignas(4) float _pad0, _pad1;
} GLSTD140_LIGHT, *HGLSTD140_LIGHT;
BR_STATIC_ASSERT(sizeof(GLSTD140_LIGHT) % 16 == 0, "GLSTD140_LIGHT is not aligned");

typedef struct _GLSTD140_SCENE_DATA {
    alignas(16) br_vector4_f eye_view;
    alignas(16) GLSTD140_LIGHT lights[BR_MAX_LIGHTS];
    alignas(4) uint32_t num_lights;
} GLSTD140_SCENE_DATA, *HGLSTD140_SCENE_DATA;
BR_STATIC_ASSERT(sizeof(((GLSTD140_SCENE_DATA *)NULL)->lights) == sizeof(GLSTD140_LIGHT) * BR_MAX_LIGHTS,
                 "std::array<GLSTD140_LIGHT> fucked up");

typedef struct _GLSTD140_MODEL_DATA {
    alignas(16) br_matrix4_f model_view;
    alignas(16) br_matrix4_f projection;
    alignas(16) br_matrix4_f mvp;
    alignas(16) br_matrix4_f normal_matrix;
    alignas(16) br_vector4_f surface_colour;
    alignas(16) br_vector4_f clear_colour;
    alignas(4) float ka;
    alignas(4) float ks;
    alignas(4) float kd;
    alignas(4) float power;
    alignas(4) uint32_t unlit;
    alignas(4) uint32_t disable_colour_key;
} GLSTD140_MODEL_DATA, *HGLSTD140_MODEL_DATA;
#pragma pack(pop)

#ifdef __cplusplus
};
#endif
#endif /* _VIDEO_H_ */
