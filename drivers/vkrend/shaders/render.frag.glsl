#version 450 core

#extension GL_GOOGLE_include_directive : require

#include "render.common.glsl"

/*
 * Fragment shader for an unlit br_renderer surface: the surface colour, or the
 * interpolated vertex colour when the surface's colour source is the geometry (a pre-lit
 * material with no BR_MATF_LIGHT). A surface with BRT_LIGHTING_B clear draws with this
 * pipeline; the lit pipeline uses render.lit.frag.glsl. The renderer keys its pipeline
 * cache on that split.
 */

layout(location = 0) in vec3 vViewPosition;
layout(location = 4) in vec4 vColour;

layout(location = 0) out vec4 outColour;

void main()
{
    vec4 fragColour = vertexColourSource() ? vColour : pc.colour;

    if(fogEnabled())
        fragColour = applyFog(fragColour, -vViewPosition.z);

    outColour = clamp(fragColour, 0.0, 1.0);
}
