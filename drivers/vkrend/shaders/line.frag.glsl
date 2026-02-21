#version 450 core

/*
 * Flat colour for the GPU line and point draw.
 *
 * The colour is premultiplied here so the pipeline's ONE / ONE_MINUS_SRC_ALPHA colour
 * blend reproduces GL's SRC_ALPHA / ONE_MINUS_SRC_ALPHA exactly - see DeviceVkLineGetPipeline().
 */

layout(push_constant) uniform LineData {
    vec4 colour;
    vec2 size;
    vec2 start;
    vec2 end;
} line;

layout(location = 0) out vec4 out_colour;

void main()
{
    out_colour = vec4(line.colour.rgb * line.colour.a, line.colour.a);
}
