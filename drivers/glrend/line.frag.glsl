#version 150

layout(std140) uniform LineData {
    mat4 mvp;
    vec2 start;
    vec2 end;
    vec4 colour;
} line_data;

out vec4 main_colour;

void main()
{
    main_colour = line_data.colour;
}
