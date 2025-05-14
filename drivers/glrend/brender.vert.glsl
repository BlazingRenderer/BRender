#version 430 core

#include "brender.common.glsl"

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aUV;
layout(location=2) in vec3 aNormal;
layout(location=3) in vec4 aColour;

layout(location=0) out vec4 position;
layout(location=1) out vec3 normal;
layout(location=2) out vec2 uv;
layout(location=3) out vec4 colour;

/*
 * [0] = Ambient
 * [1] = Diffuse
 * [2] = Specular
 */
layout(location=4) out mat3 vertexLighting;

layout(location=7) out vec3 rawPosition;
layout(location=8) out vec3 rawNormal;

layout(location=9) out float viewDistance;

out float gl_ClipDistance[MAX_CLIP_PLANES];

void main()
{
    vec4 pos = vec4(aPosition, 1.0);

    position = model_view * pos;
    normal = normalize(mat3(normal_matrix) * aNormal);
    uv = aUV;
    colour = aColour;

    viewDistance = -position.z;

    vertexLighting = mat3(0);

#if ENABLE_GOURAUD
    accumulateLights(position.xyz, normal, vertexLighting[0], vertexLighting[1], vertexLighting[2]);
#endif

    rawPosition = aPosition;
    rawNormal   = aNormal;

    pos = mvp * pos;

    gl_Position = pos;

    /*
     * Switched from "num_clip_planes" to "MAX_CLIP_PLANES" so it will be unrolled.
     */
    for(int i = 0; i < MAX_CLIP_PLANES; ++i) {
        gl_ClipDistance[i] = dot(position, clip_planes[i]);
    }
}
