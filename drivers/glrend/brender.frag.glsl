#version 150

#include "brender.common.glsl"
#include "common.glsl"

in vec4 position;
in vec2 uv;
in vec4 normal;
in vec4 colour;
in vec3 vertexLightA;
in vec3 vertexLightD;
in vec3 vertexLightS;

in vec3 rawPosition;
in vec3 rawNormal;

in float viewDistance;

out vec4 mainColour;

uniform sampler2D  main_texture;
uniform usampler2D index_texture;

vec3 adjustBrightness(in vec3 colour, in float brightness)
{
    return colour + brightness;
}

/* use values between -1 and 1 */
vec3 adjustContrast(in vec3 colour, in float contrast)
{
    return 0.5 + (1.0 + contrast) * (colour - 0.5);
}

/* use values between -1 and 1 */
vec3 adjustExposure(in vec3 colour, in float exposure)
{
    // return (1.0 + exposure) * colour;
    return vec3(1.0) - exp(-colour * exposure);
}

vec3 adjustSaturation(in vec3 colour, in float saturation)
{
    /* https://www.w3.org/TR/WCAG21/#dfn-relative-luminance */
    const vec3 luminosityFactor = vec3(0.2126, 0.7152, 0.0722);
    vec3 grayscale = vec3(dot(colour, luminosityFactor));

    return mix(grayscale, colour, 1.0 + saturation);
}

vec4 applyFog(in vec4 inColour, in float dist)
{
    float fog_min = fog_range.x; /* NB: To match softrend, set this to 0. */
    float fog_max = fog_range.y;

    if(fog_min == fog_max)
        fog_max += 0.001f;

    float fog_factor = clamp((fog_max - dist) / (fog_max - fog_min), 0.0, 1.0);
    return mix(fog_colour, inColour, fog_factor);
}

vec2 SurfaceMapEnvironment(in vec3 eye, in vec3 normal, in mat4 model_to_environment) {
    vec3 r;
    vec4 wr;
    float d, cu, cv;

    /*
     * Generate reflected vector
     */
    r = reflect(eye, normalize(normal));

    /*
     * Rotate vector into the environment frame.
     * This should be the identity matrix if no environment is set.
     */
    wr = model_to_environment * vec4(r, 0.0);
    vec3 wr2 = normalize(wr.xyz);

    /*
     * Convert vector to environment coordinates
     */
    cu = 0.5 + atan(wr2.x, -wr2.z) * 0.159154943091895; /* 1/(2*PI) */
    cv = 0.5 + -wr2.y * 0.5;

    return vec2(cu, cv);
}

vec2 SurfaceMap(in vec3 position, in vec3 normal, in vec2 uv)
{
    if(uv_source == UV_SOURCE_ENV_L) {
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
    } else {
        /* nop */
    }

    /* Apply the map transformation. */
    return (map_transform * vec4(uv, 1.0, 0.0)).xy;
}

vec4 getTexColour(in vec2 uv)
{
    if(!is_indexed) {
        return texture(main_texture, uv);
    }

    if(is_filtered) {
        return bilinearFilter(index_texture, main_texture, uv);
    }

    return texturei(index_texture, main_texture, uv);
}

void main()
{
    vec2 mappedUV = SurfaceMap(rawPosition, rawNormal, uv);

    vec4 texColour = getTexColour(mappedUV);

    if(!disable_colour_key && texColour.rgb == vec3(0.0, 0.0, 0.0))
        discard;

    vec3 fragmentLightA = vertexLightA;
    vec3 fragmentLightD = vertexLightD;
    vec3 fragmentLightS = vertexLightS;
#if ENABLE_PHONG
    accumulateLights(position, normal, fragmentLightA, fragmentLightD, fragmentLightS);
#endif

    vec4 fragColour = ((vec4(fragmentLightA + fragmentLightD, 1.0)) * (surface_colour * texColour)) + vec4(fragmentLightS, 0.0);

    fragColour = clamp(fragColour, 0, 1);

    /* Perform gamma correction */
#if ENABLE_GAMMA_CORRECTION
    fragColour.rgb = pow(fragColour.rgb, vec3(1.0 / 1.2));
    // fragColour.rgb = adjustContrast(fragColour.rgb, 0.1);
    // fragColour.rgb = adjustExposure(fragColour.rgb, 2.0);
#endif

#if ENABLE_SIMULATE_8BIT_COLOUR
    fragColour.rgb = floor(fragColour.rgb * vec3(15.0)) / vec3(15.0);
#endif
#if ENABLE_SIMULATE_16BIT_COLOUR
    float r = floor(fragColour.r * 31.0) / 31.0;
    float g = floor(fragColour.g * 63.0) / 63.0;
    float b = floor(fragColour.b * 31.0) / 31.0;
    fragColour.rgb = vec3(r, g, b);
#endif

    if(enable_fog) {
        fragColour = applyFog(fragColour, viewDistance);
    }

    mainColour = fragColour;

    return;
}
