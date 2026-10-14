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

layout(location=0) out uint main_colour;

void main()
{
    /*
     * An indexed destination is a GL_R8UI integer texture, so the fragment
     * output is the glyph's index where it covers a pixel and discarded
     * elsewhere. text_data.colour.x carries the index itself for this program,
     * not a normalised colour as in the float variant.
     */
    if(texture(uSampler, vec3(uv, character)).r < 0.5)
        discard;

    main_colour = uint(text_data.colour.x + 0.5);
}
