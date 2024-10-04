#version 150

layout(std140) uniform RectData {
    mat4  mvp;
    vec4  src_rect;
    vec4  dst_rect;
    float vertical_flip;
    float indexed;
} rect_data;

out vec2 uv;

const int INDICES[6] = int[6](0, 1, 2, 2, 1, 3);

void main()
{
    float sx = rect_data.src_rect.x;
    float sy = rect_data.src_rect.y;
    float sw = rect_data.src_rect.z;
    float sh = rect_data.src_rect.w;

    float dx = rect_data.dst_rect.x;
    float dy = rect_data.dst_rect.y;
    float dw = rect_data.dst_rect.z;
    float dh = rect_data.dst_rect.w;

    vec2 verts[4] = vec2[4](
        vec2(dx,      dy + dh),
        vec2(dx,      dy     ),
        vec2(dx + dw, dy + dh),
        vec2(dx + dw, dy     )
    );

    vec2 uvs[4] = vec2[4](
        vec2(sx,      sy + sh),
        vec2(sx,      sy     ),
        vec2(sx + sw, sy + sh),
        vec2(sx + sw, sy     )
    );

    gl_Position = rect_data.mvp * vec4(verts[INDICES[gl_VertexID]], 0, 1);
    uv = uvs[INDICES[gl_VertexID]];
}
