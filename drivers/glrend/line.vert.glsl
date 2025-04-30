#version 150

layout(std140) uniform LineData {
    mat4 mvp;
    vec2 start;
    vec2 end;
    vec4 colour;
} line_data;

void main()
{
    vec2 verts[2] = vec2[2](
        line_data.start,
        line_data.end
    );

    gl_Position = line_data.mvp * vec4(verts[gl_VertexID], 0, 1);
}
