/*
 * Fetch the palette entry an INDEX_8 colour map names.
 *
 * An indexed colour map is two textures: the index plane, which the format table gives the
 * unsigned-integer format R8_UINT, and the palette. The index is fetched through a
 * usampler2D (the sampler must be NEAREST - integer textures cannot be filtered) and looked
 * up the way BrPixelmapPixelGet(map, 0, index) addresses the palette: column 0, raw row
 * `index`. texelFetch keeps the row exact and clamps it to the palette's last row, so a
 * palette with fewer than 256 rows resolves an out-of-range index to that last row. Sampling
 * the palette applies the source view's swizzle, so a palette in any format needs no
 * per-format shader.
 *
 * The raw index is returned so the colour key can be tested on it before the lookup: an
 * INDEX_8 map keys palette index 0, an unindexed one an all-zero texel
 * (resources/aidocs/pixelmap-colour-keying.md).
 */
layout(set = 0, binding = 1) uniform usampler2D colour_index;
layout(set = 0, binding = 2) uniform sampler2D colour_clut;

vec4 indexedColourMap(vec2 uv, out uint index)
{
    index = texture(colour_index, uv).r;

    ivec2 clut_size = textureSize(colour_clut, 0);

    return texelFetch(colour_clut, ivec2(0, clamp(int(index), 0, clut_size.y - 1)), 0);
}
