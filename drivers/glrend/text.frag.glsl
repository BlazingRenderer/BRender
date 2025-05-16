#version 430 core

#define GLYPH_COUNT 256
#define CHUNK_SIZE  64

layout(std140, binding=0) uniform FontData {
    // 0 = [0][0], 1 = [0][1], 2 = [0][2], 3 = [0][3]
    // 4 = [1][0], 5 = [1][1], 6 = [1][2], 7 = [1][3]
    // and so on.
    vec4 widths[GLYPH_COUNT/4];
} font_data;

layout(std140, binding=1) uniform TextData {
    mat4  mvp;
    vec4  colour;
    vec4  rects[CHUNK_SIZE];
    uvec4 chars[CHUNK_SIZE/4];
} text_data;

layout(location=0) in vec2 uv;
layout(location=1) flat in float character;
uniform sampler2DArray uSampler;

layout(location=0) out vec4 main_colour;

void main()
{
    // NB: We explicitly ignore the alpha here because everything that uses text assumes so.
    main_colour = texture(uSampler, vec3(uv, character)) * vec4(text_data.colour.rgb, 1.0);
}
