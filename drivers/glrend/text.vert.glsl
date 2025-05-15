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

out vec2 uv;
flat out float character;

const int INDICES[6] = int[6](0, 1, 2, 2, 1, 3);

void main()
{
    float x     = text_data.rects[gl_InstanceID].x;
    float y     = text_data.rects[gl_InstanceID].y;
    float w     = text_data.rects[gl_InstanceID].z;
    float h     = text_data.rects[gl_InstanceID].w;
    uint ch     = text_data.chars[gl_InstanceID / 4][gl_InstanceID % 4];
    float width = font_data.widths[ch / 4u][ch % 4u];

    vec2 verts[4] = vec2[4](
        vec2(x,     y + h),
        vec2(x,     y    ),
        vec2(x + w, y + h),
        vec2(x + w, y    )
    );

    vec2 uvs[4] = vec2[4](
        vec2(0,     0),
        vec2(0,     1),
        vec2(width, 0),
        vec2(width, 1)
    );

    gl_Position = text_data.mvp * vec4(verts[INDICES[gl_VertexID]], 0, 1);
    uv = uvs[INDICES[gl_VertexID]];
    character = ch;
}
