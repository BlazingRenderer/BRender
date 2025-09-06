#ifndef SHADER_RECT_H_
#define SHADER_RECT_H_

#pragma pack(push, 16)
typedef struct br_gl_rect_data {
    alignas(16) br_matrix4 mvp;
    alignas(16) br_vector4 src_rect;
    alignas(16) br_vector4 dst_rect;
    alignas(4) float vertical_flip;
    alignas(4) float indexed;
} br_gl_rect_data;
#pragma pack(pop)

typedef struct br_gl_rect_shader {
    GLuint program;
    GLint  uSampler;  /* Sampler, sampler2D */
    GLint  uIndexTex; /* Sampler, usampler2D */

    GLuint block_index_rect_data;
    GLuint block_binding_rect_data;

    GLuint vao;
    GLuint ubo;
} br_gl_rect_shader;

#endif /* SHADER_RECT_H_ */
