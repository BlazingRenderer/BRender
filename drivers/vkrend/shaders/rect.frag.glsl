#version 450 core

/*
 * Fetch the source texel for the interpolated quad position.
 *
 * `uv` is [0,1] within the source region and `rect.src_rect` is that region in source
 * pixels, so the texel address is an integer one. texelFetch keeps it exact - a
 * filtering fetch on a straight copy is only ever half a texel away from being wrong.
 * Sampling normalises the source's channel values; the destination's colour-attachment
 * write converts them back, so the source format needs no shader specialisation.
 */

layout(push_constant) uniform RectData {
    vec4 src_rect;
} rect;

layout(location = 0) in vec2 uv;

layout(set = 0, binding = 0) uniform sampler2D uSource;

layout(location = 0) out vec4 colour;

void main()
{
    ivec2 size  = textureSize(uSource, 0);
    ivec2 texel = ivec2(rect.src_rect.xy) + ivec2(floor(uv * rect.src_rect.zw));

    colour = texelFetch(uSource, clamp(texel, ivec2(0), size - 1), 0);
}
