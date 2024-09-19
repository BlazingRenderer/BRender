#version 150

#include "brender.common.glsl"

in vec4 position;
in vec2 uv;
in vec4 normal;
in vec4 colour;

in vec3 rawPosition;
in vec3 rawNormal;

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

void main()
{
    vec2 mappedUV = SurfaceMap(rawPosition, rawNormal, uv);

    vec4 texColour;

    if(is_indexed) {
        int index = int(texture(index_texture, uv).r);
        texColour = texelFetch(main_texture, ivec2(0, index), 0);
    } else {
        texColour = texture(main_texture, mappedUV);
    }

    if(!disable_colour_key && texColour.rgb == vec3(0.0, 0.0, 0.0))
        discard;

#if ENABLE_PHONG
    vec4 lightColour;

    bool directLightExists = false;
    lightColour = fragmainXX(position, normal, directLightExists);
    if (!directLightExists && num_lights > 0u && unlit == 0u) {
        lightColour += vec4(clear_colour.rgb, 0.0);
    }
#else
    vec4 lightColour = vec4(0, 0, 0, 0);
#endif

    vec4 surfaceColour = (surface_colour * texColour);
    vec3 fragColour = vec3((colour.rgb + lightColour.rgb) * texColour.rgb);

    /* Perform gamma correction */
#if ENABLE_GAMMA_CORRECTION
    fragColour = pow(fragColour, vec3(1.0 / 1.2));
    // fragColour = adjustContrast(fragColour, 0.1);
    // fragColour = adjustExposure(fragColour, 2.0);
#endif

#if ENABLE_SIMULATE_8BIT_COLOUR
    fragColour = floor(fragColour.rgb * vec3(15.0)) / vec3(15.0);
#endif
#if ENABLE_SIMULATE_16BIT_COLOUR
    float r = floor(fragColour.r * 31.0) / 31.0;
    float g = floor(fragColour.g * 63.0) / 63.0;
    float b = floor(fragColour.b * 31.0) / 31.0;
    fragColour = vec3(r, g, b);
#endif

    /* The actual surface colour. */
    mainColour = vec4(fragColour, surfaceColour.a);

    return;
}
