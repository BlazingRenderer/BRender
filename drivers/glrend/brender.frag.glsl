#version 150

#define MAX_LIGHTS                   48 /* Must match up with BRender */

#define ENABLE_GAMMA_CORRECTION      0
#define ENABLE_SIMULATE_8BIT_COLOUR  0
#define ENABLE_SIMULATE_16BIT_COLOUR 0

struct br_light
{
    vec4 position;    /* (X, Y, Z, 1) */
    vec4 direction;   /* (X, Y, Z, 0), normalised */
    vec4 half_;       /* (X, Y, Z, 0), normalised */
    vec4 colour;      /* (R, G, B, 0), normalised */
    vec4 iclq;        /* (intensity, constant, linear, attenutation) */
    vec2 spot_angles; /* (inner, outer), if (0.0, 0.0), then this is a point light. */
};

layout(std140) uniform br_scene_state
{
    vec4 eye_view; /* Eye position in view-space */
    br_light lights[MAX_LIGHTS];
    uint num_lights;
};

layout(std140) uniform br_model_state
{
    mat4 model_view;
    mat4 projection;
    mat4 mvp;
    mat4 normal_matrix;
    mat4 environment;
    vec4 surface_colour;
    vec4 clear_colour;
    vec4 eye_m; /* Eye position in model-space */

    float ka; /* Ambient mod */
    float ks; /* Specular mod (doesn't seem to be used by Croc) */
    float kd; /* Diffuse mod */
    float power;
    uint unlit; /* Is this surface unlit? */
    bool disable_colour_key;
};

in vec4 position;
in vec2 uv;
in vec4 normal;
in vec4 colour;

out vec4 mainColour;

uniform sampler2D main_texture;

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

void main()
{
    vec4 texColour = texture(main_texture, uv);

    if(!disable_colour_key && texColour.rgb == vec3(0.0, 0.0, 0.0))
        discard;

    vec4 surfaceColour = surface_colour * texColour;
    vec3 fragColour = vec3(colour.rgb * texColour.rgb);

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
