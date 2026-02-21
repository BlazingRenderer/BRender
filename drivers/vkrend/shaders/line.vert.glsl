#version 450 core

/*
 * Vertex shader for the GPU line and point draw.
 *
 * One vertex is emitted per endpoint: gl_VertexIndex 0 is the start (or the only
 * point, for a point draw) and 1 the end. The endpoints arrive in destination pixels
 * and are placed at the pixel centre, (p + 0.5): that is the window position a Vulkan
 * line's diamond-exit rule and a point's centre rule cover a fragment for, and it puts
 * destination row 0 at the top, which matches the rect draw and the CPU paths. The
 * destination size turns that straight into NDC; the viewport is the whole destination
 * image, so there is no other transform.
 *
 * The end is pushed half a pixel further along the segment, which the start is not. A
 * Vulkan line's diamond-exit rule is half-open: the diamond of the start point always
 * produces a fragment, while the diamond of the end point produces one only if the line
 * leaves it. Both endpoints here sit on pixel centres, so the end point is a diamond's
 * centre and the rule would drop that last pixel - the pixel the CPU paths (pmmem's line,
 * softprim's SoftPrimLine) do plot, since they include both endpoints. Half a pixel is as
 * far as a diamond reaches from its centre, so this always leaves the end diamond, and it
 * is nowhere near enough to leave the diamond after it, so no pixel past the end is added
 * and nothing is plotted twice. (A driver whose rule already includes the end point is
 * unaffected: the extension only adds that same diamond.) A zero-length line is extended
 * by nothing - it has no direction to extend along, and the start point's diamond is
 * already covered.
 *
 * gl_PointSize is written so a point draw does not depend on the maintenance5 default
 * of 1.0, which this device does not enable.
 */

layout(push_constant) uniform LineData {
    vec4 colour;  /* r, g, b, a in [0,1]; the fragment stage premultiplies. */
    vec2 size;    /* destination width, height in pixels. */
    vec2 start;   /* start endpoint, in destination pixels. */
    vec2 end;     /* end endpoint, in destination pixels. */
} line;

void main()
{
    vec2 p = line.start;

    if(gl_VertexIndex != 0) {
        vec2  from_start = line.end - line.start;
        float length2    = dot(from_start, from_start);

        p = line.end;
        if(length2 > 0.0)
            p += 0.5 * (from_start / sqrt(length2));
    }

    vec2 ndc = (2.0 * (p + 0.5) / line.size) - 1.0;

    gl_Position  = vec4(ndc, 0.0, 1.0);
    gl_PointSize = 1.0;
}
