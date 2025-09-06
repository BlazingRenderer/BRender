#ifndef SHADER_TEXT_H_
#define SHADER_TEXT_H_

/*
 * How many characters to draw at once. Update the shaders if you change this.
 */
#define BR_GL_TEXT_CHUNK_SIZE 64

/*
 * Max no. glyphs in a font.
 */
#define BR_GL_TEXT_FONT_GLYPH_COUNT 256

#pragma pack(push, 16)
typedef struct br_gl_text_data {
    alignas(16) br_matrix4 mvp;
    alignas(16) br_vector4 colour;
    alignas(16) br_vector4_f rects[BR_GL_TEXT_CHUNK_SIZE];
    alignas(16) br_uint_32 chars[BR_GL_TEXT_CHUNK_SIZE];
} br_gl_text_data;

typedef struct br_gl_text_font_data {
    /*
     * NB: This is a vec4[GLYPH_COUNT/4] on the GLSL side because
     * std140's array packing rules are shit.
     */
    alignas(16) br_float widths[BR_GL_TEXT_FONT_GLYPH_COUNT];
} br_gl_text_font_data;
#pragma pack(pop)

typedef struct br_gl_text_font {
    GLuint   tex;
    GLuint   font_data;
    br_font *font;
} br_gl_text_font;

typedef struct br_gl_text_shader {
    GLuint program;
    GLint  uSampler; /* Sampler, sampler2D */

    GLuint block_index_font_data;
    GLuint block_binding_font_data;

    GLuint vao_glyphs;
    GLuint ubo_glyphs;
    GLuint block_index_glyphs;
    GLuint block_binding_glyphs;
} br_gl_text_shader;

#endif /* SHADER_TEXT_H_ */
