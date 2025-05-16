#version 430 core

#include "brender.common.glsl"
#include "common.glsl"

layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
layout(location=3) in vec4 colour;


layout(location=4) in mat3 vertexLighting;

layout(location=7) in vec3 rawPosition;
layout(location=8) in vec3 rawNormal;

layout(location=9) in float viewDistance;

layout(location=0) out vec4 mainColour;

uniform sampler2D  main_texture;
uniform usampler2D index_texture;

vec4 applyFog(in vec4 inColour, in float dist)
{
    float fog_min = fog_range.x; /* NB: To match softrend, set this to 0. */
    float fog_max = max(fog_range.y + 0.001f, fog_range.y);

    float fog_factor = clamp((fog_max - dist) / (fog_max - fog_min), 0.0, 1.0);
    return mix(fog_colour, inColour, fog_factor);
}

vec2 SurfaceMapEnvironment(in vec3 eye, in vec3 normal, in mat4 model_to_environment) {
    vec3 r;
    float d, cu, cv;

    /*
     * Generate reflected vector
     */
    r = reflect(eye, normalize(normal));

    /*
     * Rotate vector into the environment frame.
     * This should be the identity matrix if no environment is set.
     */
    vec3 wr2 = normalize(mat3(model_to_environment) * r);

    /*
     * Convert vector to environment coordinates
     */
    cu = atan(wr2.x, -wr2.z) * 0.159154943091895; /* 1/(2*PI) */
    cv = -wr2.y * 0.5;

    return 0.5 + vec2(cu, cv);
}

vec2 SurfaceMap(in vec3 position, in vec3 normal, in vec2 uv)
{
    if(uv_source == UV_SOURCE_MODEL) {
        /*
         * NB: We need this no-op branch explicitly first because
         * NX Homebrew (Mesa 20.1.0-rc3 + nouveau) is much faster
         * with it here.
         */
    } else if(uv_source == UV_SOURCE_ENV_L) {
        /*
         * Generate U,V for environment assuming local eye.
         *
         * softrend/mapping.c - SurfaceMapEnvironmentLocal()
         */
        vec3 eye = normalize(eye_m.xyz - position);
        uv = SurfaceMapEnvironment(eye, normal, environment);
    } else if(uv_source == UV_SOURCE_ENV_I) {
        /*
         * Generate U,V for environment assuming infinite eye.
         *
         * softrend/mapping.c - SurfaceMapEnvironmentInfinite()
         */
        vec3 eye = normalize(eye_m.xyz);
        uv = SurfaceMapEnvironment(eye, normal, environment);
    }

    /* Apply the map transformation. */
    return (mat3(map_transform) * vec3(uv, 1.0)).xy;
}

vec4 getTexColour(in vec2 uv)
{
    switch(texture_mode) {
        case TEXTURE_MODE_NORMAL:
            return texture(main_texture, uv);
        case TEXTURE_MODE_INDEX:
            return texturei(index_texture, main_texture, uv);
        case TEXTURE_MODE_INDEX_FILTER:
            return bilinearFilter(index_texture, main_texture, uv);
        default:
            return vec4(1, 0, 1, 1);
    }
}

void main()
{
    vec2 mappedUV = SurfaceMap(rawPosition, rawNormal, uv);

    vec4 texColour = getTexColour(mappedUV);

    if(!disable_colour_key && texColour.rgb == vec3(0.0, 0.0, 0.0))
        discard;

    vec3 lightA = vertexLighting[0];
    vec3 lightD = vertexLighting[1];
    vec3 lightS = vertexLighting[2];
#if ENABLE_PHONG
    accumulateLights(position.xyz, normal, lightA, lightD, lightS);
#endif

    vec4 fragColour = ((vec4(lightA + lightD, 1.0)) * (surface_colour * texColour)) + vec4(lightS, 0.0);

    if(enable_fog) {
        fragColour = applyFog(fragColour, viewDistance);
    }

    fragColour = clamp(fragColour, 0, 1);

    mainColour = fragColour;

    return;
}
