#version 150

#include "brender.common.glsl"

in vec3 aPosition;
in vec2 aUV;
in vec3 aNormal;
in vec4 aColour;

out vec4 position;
out vec3 normal;
out vec2 uv;
out vec4 colour;

out vec3 vertexLightA;
out vec3 vertexLightD;
out vec3 vertexLightS;

out vec3 rawPosition;
out vec3 rawNormal;

out float viewDistance;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_ClipDistance[MAX_CLIP_PLANES];
};

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
    normal = normalize(mat3(normal_matrix) * aNormal);
    uv = aUV;
    colour = aColour;

    viewDistance = -position.z;

    vertexLightA = vec3(0);
    vertexLightD = vec3(0);
    vertexLightS = vec3(0);

#if ENABLE_GOURAUD
    accumulateLights(position, vec4(normal, 0), vertexLightA, vertexLightD, vertexLightS);
#endif

    rawPosition = aPosition;
    rawNormal   = aNormal;

#if ENABLE_PSX_SIMULATION
    pos = PSXify_pos(mvp * pos, vec2(200.0, 150.0));
#else
    pos = mvp * pos;
#endif

    gl_Position = pos;

    for(uint i = 0u; i < num_clip_planes; ++i) {
        gl_ClipDistance[i] = dot(position, clip_planes[i]);
    }
}
