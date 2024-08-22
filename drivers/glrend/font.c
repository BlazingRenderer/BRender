#include "drv.h"
#include "brassert.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

/*
 * Amount of padding in pixels around each glyph. Keep POT.
 */
#define GLYPH_PAD_PX 4
#define GLYPH_COUNT  256

typedef struct br_font_atlas_state_gl {
    stbrp_rect rects[GLYPH_COUNT];
    stbrp_node nodes[GLYPH_COUNT];
} br_font_atlas_state_gl;

br_error FontGLBuildAtlas(br_font_gl *gl_font, br_font *font, br_int_32 width, br_int_32 height)
{
    br_pixelmap            *pm;
    char                    c[2];
    GLuint                  tex;
    br_font_atlas_state_gl *state;

    gl_font->font = font;

    state = BrScratchAllocate(sizeof(br_font_atlas_state_gl));

    for(int i = 0; i < BR_ASIZE(state->rects); ++i) {
        stbrp_rect *rect = state->rects + i;
        rect->id         = i;
        rect->w          = ((font->flags & BR_FONTF_PROPORTIONAL) ? font->width[i] : font->glyph_x) + GLYPH_PAD_PX;
        rect->h          = font->glyph_y + GLYPH_PAD_PX;
    }

    stbrp_context ctx = {};
    stbrp_init_target(&ctx, width, height, state->nodes, BR_ASIZE(state->nodes));

    /* If this fails, that's programmer error. */
    (void)stbrp_pack_rects(&ctx, state->rects, BR_ASIZE(state->rects));

    if((pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, width, height, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrScratchFree(state);
        return 0;
    }

    pm->origin_x = pm->origin_y = 0;
    BrPixelmapFill(pm, 0x00000000);

    c[1] = '\0';
    for(size_t i = 0; i < BR_ASIZE(state->rects); ++i) {
        const stbrp_rect *rect = state->rects + i;

        br_rectangle r = {
            .w = rect->w - GLYPH_PAD_PX,
            .h = rect->h - GLYPH_PAD_PX,
            .x = rect->x + (GLYPH_PAD_PX >> 1),
            .y = rect->y + (GLYPH_PAD_PX >> 1),
        };

        c[0] = (char)i;

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
    }

    tex = VIDEO_BrPixelmapToGLTexture(pm);
    BrPixelmapFree(pm);

    BrScratchFree(state);

    gl_font->tex = tex;
    if(tex == 0)
        return BRE_FAIL;

    return BRE_OK;
}
