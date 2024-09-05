#version 150

#include "brender.common.glsl"

in vec3 aPosition;
in vec2 aUV;
in vec3 aNormal;
in vec4 aColour;

out vec4 position;
out vec4 normal;
out vec2 uv;
out vec4 colour;

out vec3 rawPosition;
out vec3 rawNormal;

#if ENABLE_PSX_SIMULATION
vec4 PSXify_pos(in vec4 vertex, in vec2 resolution)
{
    vec4 snappedPos = vertex;
    snappedPos.xyz = vertex.xyz / vertex.w;
    snappedPos.xy = floor(resolution.xy * snappedPos.xy) / resolution;
    snappedPos.xyz *= vertex.w;
    return snappedPos;
}
#endif

void main()
{
    vec4 pos = vec4(aPosition, 1.0);

    position = model_view * pos;
    normal = vec4(normalize(mat3(normal_matrix) * aNormal), 0);
    uv = aUV;

#if ENABLE_GOURAUD
    bool directLightExists = false;
    colour = aColour + fragmainXX(position, normal, directLightExists);
    if (!directLightExists && num_lights > 0u && unlit == 0u)
        colour += vec4(clear_colour.rgb, 0.0);
#else
    colour = aColour;
#endif

    rawPosition = aPosition;
    rawNormal   = aNormal;

#if ENABLE_PSX_SIMULATION
    pos = PSXify_pos(mvp * pos, vec2(200.0, 150.0));
#else
    pos = mvp * pos;
#endif

    gl_Position = pos;
}
