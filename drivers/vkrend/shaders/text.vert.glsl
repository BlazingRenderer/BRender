#version 450 core

/*
 * Instanced glyph-quad vertex shader for the pixelmap -> device text draw.
 *
 * One instance is one character: the per-instance vertex data carries the glyph
 * rectangle in destination pixels (rect) and the glyph index together with the extent
 * of the glyph within its atlas layer (glyph.x is the glyph's width over the atlas
 * layer's width, so sampling the layer's left `glyph.x` of the [0,1] u range touches
 * only the glyph; glyph.y is the character code, i.e. the atlas array layer).
 *
 * The text's colour and the destination size are push constants. The destination size
 * turns a destination pixel into NDC; the viewport is the whole destination image, so
 * there is no other transform. As in the line and rect draws, destination row 0 is the
 * top of the image, so NDC +y is down and a smaller destination y is a smaller NDC y.
 *
 * The quad is drawn from the glyph rectangle's edges, exactly as the rect draw maps its
 * quad to a rectangle, so a glyph lands on the pixel grid the CPU renderer would use.
 * Vulkan texture row 0 is the top row of the image, which is the glyph's top row, so
 * the quad's top edge takes v = 0 and its bottom edge v = 1.
 */

#define GLYPH_COUNT 256

layout(push_constant) uniform TextData {
    vec4 colour;  /* r, g, b, a in [0,1]. */
    vec2 size;    /* destination width, height in pixels. */
    vec2 padding; /* rounds the block up to its 16-byte alignment. */
} text_data;

layout(location = 0) in vec4 rect;  /* x, y, w, h of the glyph rectangle, in destination pixels. */
layout(location = 1) in vec2 glyph; /* x = u extent (width / atlas layer width), y = glyph index. */

layout(location = 0) out vec2 uv;
layout(location = 1) flat out float character;

const int indices[6] = int[6](0, 1, 2, 2, 1, 3);

void main()
{
    /* 0 top-left, 1 bottom-left, 2 top-right, 3 bottom-right. */
    vec2 corners[4] = vec2[4](
        vec2(rect.x, rect.y),
        vec2(rect.x, rect.y + rect.w),
        vec2(rect.x + rect.z, rect.y),
        vec2(rect.x + rect.z, rect.y + rect.w)
    );

    /* v is 0 at the top row and 1 at the bottom, matching the image's row order. */
    vec2 uvs[4] = vec2[4](
        vec2(0.0, 0.0),
        vec2(0.0, 1.0),
        vec2(glyph.x, 0.0),
        vec2(glyph.x, 1.0)
    );

    int idx = indices[gl_VertexIndex];

    gl_Position = vec4(2.0 * corners[idx] / text_data.size - 1.0, 0.0, 1.0);
    uv          = uvs[idx];
    character   = glyph.y;
}
