#version 450 core

#extension GL_GOOGLE_include_directive : require

#include "render.lighting.glsl"

/*
 * Fragment shader for a lit br_renderer surface with no colour map: BRender's
 * ambient/diffuse/specular model, shared with the lit-textured shader through
 * render.lighting.glsl.
 *
 * The final colour is `(ambient + diffuse) * surfaceColour + specular`: the diffuse and
 * ambient terms are modulated by the surface colour while specular is added unmodulated,
 * exactly as glrend's main() does. The surface colour is pc.colour, or the interpolated
 * vertex colour when the surface's colour source is the geometry. A pre-lit surface adds
 * its vertex colour - the lighting result the core wrote into group->vertex_colours,
 * core/v1db/prelight.c - on top, glrend's `prelighting` term.
 *
 * Where the light terms come from follows the shading mode: a GOURAUD surface took the
 * vertex stage's interpolated vLightAmbient/vLightDiffuse/vLightSpecular, while a PHONG
 * (or out-of-scope FLAT) surface accumulates them here from the interpolated normal.
 */

layout(location = 0) in vec3 vViewPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) flat in vec3 vFlatNormal;
layout(location = 4) in vec4 vColour;
layout(location = 5) in vec3 vLightAmbient;
layout(location = 6) in vec3 vLightDiffuse;
layout(location = 7) in vec3 vLightSpecular;

layout(location = 0) out vec4 outColour;

void main()
{
    vec3 ambient  = vLightAmbient;
    vec3 diffuse  = vLightDiffuse;
    vec3 specular = vLightSpecular;

    if(!gouraudShading()) {
        vec3 n = normalize(flatShading() ? vFlatNormal : vNormal);

        accumulateSceneLights(vViewPosition, n, ambient, diffuse, specular);
    }

    vec4 surfaceColour = vertexColourSource() ? vColour : pc.colour;

    vec4 fragColour = vec4((ambient + diffuse) * surfaceColour.xyz + specular, surfaceColour.a);

    if(prelightingEnabled())
        fragColour += vec4(vColour.rgb, 1.0);

    if(fogEnabled())
        fragColour = applyFog(fragColour, -vViewPosition.z);

    outColour = clamp(fragColour, 0.0, 1.0);
}
