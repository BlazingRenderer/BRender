#ifndef _FONT_H_
#define _FONT_H_

typedef struct br_font_gl {
    GLuint   tex;
    GLuint   font_data;
    br_font *font;
} br_font_gl;

/*
 * How many characters to draw at once. Update the shaders if you change this.
 */
#define BR_TEXT_GL_CHUNK_SIZE 64

#pragma pack(push, 16)
typedef struct br_text_gl {
    alignas(16) br_matrix4 mvp;
    alignas(16) br_vector4 colour;
    alignas(16) br_vector4_f rects[BR_TEXT_GL_CHUNK_SIZE];
    alignas(16) br_uint_32 chars[BR_TEXT_GL_CHUNK_SIZE];
} br_text_gl;
#pragma pack(pop)

#endif /* _FONT_H_ */
