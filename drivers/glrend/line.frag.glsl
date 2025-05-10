#version 430 core

layout(std140, binding=0) uniform LineData {
    mat4 mvp;
    vec2 start;
    vec2 end;
    vec4 colour;
} line_data;

layout(location=0) out vec4 main_colour;

void main()
{
    main_colour = line_data.colour;
}
