#version 450 core

/*
 * Fetch the source index and look it up in the palette.
 *
 * An INDEX_8 source is two textures: the index plane, which the format table gives the
 * unsigned-integer format R8_UINT, and the palette. The index is therefore fetched through
 * a usampler2D and recovered from .r - the source view's VkComponentMapping places the one
 * byte there - and the palette is addressed the way BrPixelmapPixelGet(map, 0, index)
 * addresses it: column 0, raw row `index`. Sampling the palette normalises its channel
 * values through the same swizzle the plain rect draw uses, so an indexed source needs no
 * per-palette-format specialisation either.
 */

layout(push_constant) uniform RectData {
    vec4 src_rect;
} rect;

layout(location = 0) in vec2 uv;

layout(set = 0, binding = 0) uniform usampler2D uIndex;
layout(set = 0, binding = 1) uniform sampler2D uClut;

layout(location = 0) out vec4 colour;

void main()
{
    ivec2 size  = textureSize(uIndex, 0);
    ivec2 texel = ivec2(rect.src_rect.xy) + ivec2(floor(uv * rect.src_rect.zw));
    uint  index = texelFetch(uIndex, clamp(texel, ivec2(0), size - 1), 0).r;

    /*
     * An index is a byte and the palette is conventionally 256 entries, but a palette with
     * fewer rows has no entry for the larger indices - clamp, rather than fetch past it.
     */
    ivec2 clut_size = textureSize(uClut, 0);

    colour = texelFetch(uClut, ivec2(0, min(int(index), clut_size.y - 1)), 0);
}
