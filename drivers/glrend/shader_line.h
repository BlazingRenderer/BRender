#ifndef SHADER_LINE_H_
#define SHADER_LINE_H_

#pragma pack(push, 16)
typedef struct br_gl_line_data {
    alignas(16) br_matrix4 mvp;
    alignas(8) br_vector2 start;
    alignas(8) br_vector2 end;
    alignas(16) br_vector4 colour;
} br_gl_line_data;
#pragma pack(pop)

typedef struct br_gl_line_shader {
    GLuint program;
    GLuint block_index_line_data;
    GLuint block_binding_line_data;

    GLuint vao;
    GLuint ubo;
} br_gl_line_shader;

#endif /* SHADER_LINE_H_ */
