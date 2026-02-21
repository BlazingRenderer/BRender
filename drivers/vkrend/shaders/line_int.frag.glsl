#version 450 core

/*
 * CLUT index for the GPU line and point draw on an indexed destination.
 *
 * An INDEX_8 image is a VK_FORMAT_R8_UINT integer attachment, so the fragment output
 * must be an integer and holds the CLUT index itself. The caller places that index in
 * the push block's colour.x (as the indexed fill does); no blending is possible against
 * an integer attachment, so the indexed pipeline leaves it disabled.
 */

layout(push_constant) uniform LineData {
    vec4 colour; /* the CLUT index in .r for this program, not a normalised colour. */
    vec2 size;
    vec2 start;
    vec2 end;
} line;

layout(location = 0) out uint out_index;

void main()
{
    out_index = uint(line.colour.r + 0.5);
}
