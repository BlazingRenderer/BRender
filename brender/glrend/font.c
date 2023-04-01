#include "drv.h"
#include "brassert.h"

br_error FontGLBuildAtlas(br_font_gl *gl_font, br_font *font, br_int_32 width, br_int_32 height)
{
    br_pixelmap *pm;
    br_rectangle r = {.x = 0, .y = 0, .w = 0, .h = font->glyph_y};
    char         c[2];
    GLuint       tex;

    gl_font->font = font;

    if((pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, width, height, NULL, BR_PMAF_NORMAL)) == NULL)
        return 0;

    pm->origin_x = pm->origin_y = 0;
    BrPixelmapFill(pm, 0x00000000);

    c[1] = '\0';
    for(size_t i = 0; i < 256; ++i) {
        c[0] = (char)i;

        r.w = ((font->flags & BR_FONTF_PROPORTIONAL) ? font->width[i] : font->glyph_x);
        if((r.x + r.w) > pm->width) {
            --i;
            r.x = 0;
            r.y += r.h;
            UASSERT(r.y <= pm->height);
            continue;
        }

        BrPixelmapText(pm, r.x, r.y, 0xFFFFFFFF, font, c);

        /*
         * Calculate the UV coordinates.
         * * Remember that the image will be upside down when converted to a
         *     OpenGL texture. The UV for each glyph will match this.
         * * The origin of the atlas is being set to top-left@(0,0), so
         *     just swap y0/y1.
         */
        VIDEOI_BrRectToUVs(pm, &r, &gl_font->glyph[i].x0, &gl_font->glyph[i].y1, &gl_font->glyph[i].x1,
                           &gl_font->glyph[i].y0);

        r.x += r.w;
    }

    tex = VIDEO_BrPixelmapToGLTexture(pm);
    BrPixelmapFree(pm);

    gl_font->tex = tex;
    if(tex == 0)
        return BRE_FAIL;

    return BRE_OK;
}
