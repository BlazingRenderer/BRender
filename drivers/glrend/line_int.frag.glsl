#version 430 core

layout(std140, binding=0) uniform LineData {
    mat4 mvp;
    vec2 start;
    vec2 end;
    vec4 colour;
} line_data;

layout(location=0) out uint main_colour;

void main()
{
    /*
     * An indexed destination is a GL_R8UI integer texture, so the fragment
     * output must be an integer and holds the CLUT index. line_data.colour.x
     * carries the index itself for this program, not a normalised colour as in
     * the float variant.
     */
    main_colour = uint(line_data.colour.x + 0.5);
}
