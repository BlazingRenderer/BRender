#version 450 core

/*
 * Vertex-less quad for the pixelmap -> device rect draw.
 *
 * The destination rectangle is expressed with the Vulkan viewport, so the quad is the
 * whole NDC square and `uv` is the [0,1] position within that rectangle. Row 0 is the
 * top of a BRender pixelmap and the top of a Vulkan image, so `v.y = 0` maps to the top
 * of the viewport: Vulkan's NDC +y is down, so the top is `ndc.y = -1` and the mapping is
 * `2 * v.y - 1`. The staging copy and vkCmdBlitImage keep the origin at the top-left and
 * this matches them; mapping `v.y = 0` to NDC +1 would put source row 0 at the bottom.
 */

layout(push_constant) uniform RectData {
    vec4 src_rect; /* x, y, w, h of the source region, in source pixels. */
} rect;

layout(location = 0) out vec2 uv;

const vec2 verts[4]   = vec2[4](vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0));
const int  indices[6] = int[6](0, 1, 2, 2, 1, 3);

void main()
{
    vec2 v = verts[indices[gl_VertexIndex]];

    gl_Position = vec4(v.x * 2.0 - 1.0, 2.0 * v.y - 1.0, 0.0, 1.0);
    uv          = v;
}
