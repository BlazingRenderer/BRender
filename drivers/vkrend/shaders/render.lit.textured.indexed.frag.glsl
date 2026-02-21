#version 450 core

#extension GL_GOOGLE_include_directive : require

#include "render.lighting.glsl"
#include "render.indexed.glsl"

/*
 * Fragment shader for a lit br_renderer surface with an INDEX_8 colour map. It is
 * render.lit.textured.frag.glsl with the texel taken from the palette: the index plane and
 * the palette are the two textures the indexed variant binds, and the palette colour
 * modulates the surface exactly as a plain colour map does.
 *
 * The colour key is tested on the raw index, before the palette lookup - palette index 0 is
 * the transparent entry - as glrend and softprim do
 * (resources/aidocs/pixelmap-colour-keying.md).
 *
 * An environment-mapped surface replaces the vertex-stage UV with the sphere-map coordinate
 * the reflection gives (render.common.glsl, surfaceMapEnvironment()).
 */

layout(location = 0) in vec3 vViewPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) flat in vec3 vFlatNormal;
layout(location = 3) in vec2 vUV;
layout(location = 4) in vec4 vColour;
layout(location = 5) in vec3 vLightAmbient;
layout(location = 6) in vec3 vLightDiffuse;
layout(location = 7) in vec3 vLightSpecular;

layout(location = 0) out vec4 outColour;

void main()
{
    uint source   = mappingSource();
    vec2 mappedUV = vUV;

    if(source != MAPPING_SOURCE_NONE)
        mappedUV = surfaceMapEnvironment(vViewPosition, flatShading() ? vFlatNormal : vNormal, source);

    uint index;
    vec4 texel = indexedColourMap(mappedUV, index);

    if(colourKeyEnabled() && index == 0u)
        discard;

    vec3 ambient  = vLightAmbient;
    vec3 diffuse  = vLightDiffuse;
    vec3 specular = vLightSpecular;

    if(!gouraudShading()) {
        vec3 n = normalize(flatShading() ? vFlatNormal : vNormal);

        accumulateSceneLights(vViewPosition, n, ambient, diffuse, specular);
    }

    vec4 surfaceColour = vertexColourSource() ? vColour : pc.colour;

    vec4 fragColour = vec4((ambient + diffuse) * surfaceColour.xyz * texel.rgb + specular, surfaceColour.a * texel.a);

    if(prelightingEnabled())
        fragColour += vec4(vColour.rgb, 1.0);

    if(fogEnabled())
        fragColour = applyFog(fragColour, -vViewPosition.z);

    outColour = clamp(fragColour, 0.0, 1.0);
}
