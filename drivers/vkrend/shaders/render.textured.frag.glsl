#version 450 core

#extension GL_GOOGLE_include_directive : require

#include "render.common.glsl"

/*
 * Fragment shader for an unlit br_renderer surface with a colour map. The surface colour
 * modulates the sampled texel, exactly as glrend's `surfaceColour * texColour` does for a
 * surface with no lighting and no prelighting. A surface with BRT_LIGHTING_B clear and a
 * colour map draws with this pipeline; the untextured unlit pipeline is render.frag.glsl
 * and the lit ones are render.lit.frag.glsl / render.lit.textured.frag.glsl.
 *
 * The colour map is fetched at the UV the vertex stage transformed by the surface's map
 * transform, and a black texel discards the fragment when the primitive's colour-key bit is
 * set (see render.lit.textured.frag.glsl). The surface colour - pc.colour, or the
 * interpolated vertex colour when the colour source is the geometry - modulates the texel.
 *
 * An environment-mapped surface replaces that UV with the sphere-map coordinate the
 * reflection from the eye and the surface normal gives (render.common.glsl,
 * surfaceMapEnvironment()); the map transform is then applied to it, as the vertex stage
 * applies it to the plain coordinate, and the colour key is tested on the fetched texel
 * either way.
 */

layout(location = 0) in vec3 vViewPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) flat in vec3 vFlatNormal;

layout(location = 4) in vec4 vColour;

layout(location = 3) in vec2 vUV;

layout(location = 0) out vec4 outColour;

layout(set = 0, binding = 1) uniform sampler2D colour_map;

void main()
{
    uint source    = mappingSource();
    vec2 mappedUV  = vUV;

    if(source != MAPPING_SOURCE_NONE)
        mappedUV = surfaceMapEnvironment(vViewPosition, flatShading() ? vFlatNormal : vNormal, source);

    vec4 texel = texture(colour_map, mappedUV);

    if(colourKeyEnabled() && texel.rgb == vec3(0.0))
        discard;

    vec4 surfaceColour = vertexColourSource() ? vColour : pc.colour;

    vec4 fragColour = surfaceColour * texel;

    if(fogEnabled())
        fragColour = applyFog(fragColour, -vViewPosition.z);

    outColour = clamp(fragColour, 0.0, 1.0);
}
