#version 450 core

/*
 * Sample the glyph's atlas layer and tint it with the text colour.
 *
 * The atlas stores each glyph as white with the glyph's coverage in alpha, so the
 * tinted colour is colour.rgb * glyph.rgb. Following glrend's text shader, the alpha is
 * the sampled coverage alone, not colour.a * coverage: the pipeline's SRC_ALPHA /
 * ONE_MINUS_SRC_ALPHA blend then reproduces GL's blend exactly.
 */

layout(push_constant) uniform TextData {
    vec4 colour;
    vec2 size;
    vec2 padding;
} text_data;

layout(location = 0) in vec2 uv;
layout(location = 1) flat in float character;

layout(set = 0, binding = 0) uniform sampler2DArray uAtlas;

layout(location = 0) out vec4 out_colour;

void main()
{
    vec4 glyph = texture(uAtlas, vec3(uv, character));

    out_colour = vec4(text_data.colour.rgb * glyph.rgb, glyph.a);
}
