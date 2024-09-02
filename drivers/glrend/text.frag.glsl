#version 150

#define GLYPH_COUNT 256
#define CHUNK_SIZE  64

layout(std140) uniform FontData {
    // 0 = [0][0], 1 = [0][1], 2 = [0][2], 3 = [0][3]
    // 4 = [1][0], 5 = [1][1], 6 = [1][2], 7 = [1][3]
    // and so on.
    vec4 widths[GLYPH_COUNT/4];
} font_data;

layout(std140) uniform TextData {
    mat4  mvp;
    vec4  colour;
    vec4  rects[CHUNK_SIZE];
    uvec4 chars[CHUNK_SIZE/4];
} text_data;

in vec2 uv;
flat in float character;
uniform sampler2DArray uSampler;

out vec4 main_colour;

void main()
{
    main_colour = texture(uSampler, vec3(uv, character)) * vec4(text_data.colour.rgb, 1.0);
}
