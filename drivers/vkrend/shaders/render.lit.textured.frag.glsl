#version 450 core

#extension GL_GOOGLE_include_directive : require

#include "render.lighting.glsl"

/*
 * Fragment shader for a lit br_renderer surface with a colour map: the lit model, with the
 * sampled texel modulating the surface. glrend composes the same way, multiplying the
 * ambient and diffuse terms by `surfaceColour * texColour` and adding the specular term
 * unmodulated (drivers/glrend/brender.frag.glsl). The alpha is `surfaceOpacity * texel.a`,
 * as glrend's `(surfaceColour * texColour).a` is.
 *
 * The colour map is fetched at the UV the vertex stage transformed by the surface's map
 * transform. A black texel discards the fragment when the primitive's colour-key bit is
 * set, which is glrend's `disable_colour_key` inverted: the key is tested on the raw texel
 * before the surface colour modulates it (resources/aidocs/pixelmap-colour-keying.md).
 *
 * An environment-mapped surface replaces that UV with the sphere-map coordinate the
 * reflection from the eye and the surface normal gives (render.common.glsl,
 * surfaceMapEnvironment()); the map transform is then applied to it, as the vertex stage
 * applies it to the plain coordinate, and the colour key is tested on the fetched texel
 * either way.
 *
 * The surface colour is pc.colour, or the interpolated vertex colour when the colour
 * source is the geometry; a pre-lit surface adds its vertex colour on top, glrend's
 * `prelighting` term. As in the untextured lit shader, a GOURAUD surface shades with the
 * vertex stage's interpolated light terms, while a PHONG (or out-of-scope FLAT) surface
 * accumulates them here from the interpolated normal.
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

layout(set = 0, binding = 1) uniform sampler2D colour_map;

void main()
{
    uint source   = mappingSource();
    vec2 mappedUV = vUV;

    if(source != MAPPING_SOURCE_NONE)
        mappedUV = surfaceMapEnvironment(vViewPosition, flatShading() ? vFlatNormal : vNormal, source);

    vec4 texel = texture(colour_map, mappedUV);

    if(colourKeyEnabled() && texel.rgb == vec3(0.0))
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
