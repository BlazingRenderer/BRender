#include "drv.h"
#include "brassert.h"

#define GLYPH_COUNT 256

#pragma pack(push, 16)
typedef struct font_data {
    /*
     * NB: This is a vec4[GLYPH_COUNT/4] on the GLSL side because
     * std140's array packing rules are shit.
     */
    alignas(16) br_float widths[GLYPH_COUNT];
} font_data;
#pragma pack(pop)

br_error FontGLBuildArray(br_font_gl *gl_font, br_font *font)
{
    GLuint                    tex;
    const br_pixelmap_gl_fmt *fmt;
    br_pixelmap              *pm;
    br_int_32                 max_width  = font->glyph_x;
    br_int_32                 max_height = font->glyph_y;

    font_data fd = {};

    if((fmt = DeviceGLGetFormatDetails(BR_PMT_RGBA_8888)) == NULL)
        return BRE_FAIL;

    /*
     * Find the max width, and calculate the UV coords.
     */
    for(size_t i = 0; i < GLYPH_COUNT; ++i) {
        br_int_32 width = (font->flags & BR_FONTF_PROPORTIONAL) ? font->width[i] : font->glyph_x;

        fd.widths[i] = (float)width;

        if(width > max_width)
            max_width = width;
    }

    for(size_t i = 0; i < GLYPH_COUNT; ++i) {
        fd.widths[i] /= (float)max_width;
    }

    /*
     * Upload the font data.
     */
    glGenBuffers(1, &gl_font->font_data);
    glBindBuffer(GL_UNIFORM_BUFFER, gl_font->font_data);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(font_data), &fd, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    /*
     * Allocate a temporary pixelmap to draw text into.
     */
    if((pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, max_width, max_height, NULL, BR_PMAF_NORMAL)) == NULL)
        return BRE_FAIL;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, fmt->internal_format, max_width, max_height, GLYPH_COUNT, 0, fmt->format, fmt->type, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    for(GLsizei i = 0; i < GLYPH_COUNT; ++i) {
        char c[2] = {(char)i, '\0'};

        BrPixelmapFill(pm, BR_COLOUR_RGBA(0, 0, 0, 0));
        BrPixelmapText(pm, -pm->origin_x, -pm->origin_y, BR_COLOUR_RGBA(255, 255, 255, 255), font, c);

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, max_width, max_height, 1, fmt->format, fmt->type, pm->pixels);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    BrPixelmapFree(pm);

    gl_font->tex  = tex;
    gl_font->font = font;
    return BRE_OK;
}
