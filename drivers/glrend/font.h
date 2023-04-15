#ifndef _FONT_H_
#define _FONT_H_

typedef struct br_font_gl {
    GLuint   tex;
    br_font *font;
    struct {
        float x0;
        float x1;
        float y0;
        float y1;
    } glyph[256];
} br_font_gl;

#endif /* _FONT_H_ */
