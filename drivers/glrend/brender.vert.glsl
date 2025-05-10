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

    pos = mvp * pos;

    gl_Position = pos;

    /*
     * Switched from "num_clip_planes" to "MAX_CLIP_PLANES" so it will be unrolled.
     */
    for(int i = 0; i < MAX_CLIP_PLANES; ++i) {
        gl_ClipDistance[i] = dot(position, clip_planes[i]);
    }
}
