#define MAX_LIGHTS                   48 /* Must match up with BRender */
#define MAX_CLIP_PLANES              6  /* Must match up with BRender */
#define SPECULARPOW_CUTOFF           0.6172
#define BR_SCALAR_EPSILON            1.192092896e-7f

#define ENABLE_GAMMA_CORRECTION      0
#define ENABLE_SIMULATE_8BIT_COLOUR  0
#define ENABLE_SIMULATE_16BIT_COLOUR 0

#define UV_SOURCE_MODEL              0
#define UV_SOURCE_ENV_L              1
#define UV_SOURCE_ENV_I              2

#define BRT_AMBIENT                  0u
#define BRT_DIRECT                   1u
#define BRT_POINT                    2u
#define BRT_SPOT                     3u

#define DEBUG_DISABLE_LIGHTS            0
#define DEBUG_DISABLE_LIGHT_AMBIENT     0
#define DEBUG_DISABLE_LIGHT_DIRECTIONAL 0
#define DEBUG_DISABLE_LIGHT_POINT       0
#define DEBUG_DISABLE_LIGHT_SPOT        0

#define ENABLE_GOURAUD                  0
#define ENABLE_PHONG                    1

#define USE_LEGACY_ATTENUATION          0

struct br_light
{
    vec4 position;          /* (X, Y, Z, 1)                        */
    vec4 direction;         /* (X, Y, Z, 0), normalised            */
    vec4 half_;             /* (X, Y, Z, 0), normalised            */
    vec4 colour;            /* (R, G, B, 0), normalised            */
    float intensity;        /* Intensity (1/attenuation_c)         */
    float attenuation_c;    /* Constant attenuation factor.        */
    float attenuation_l;    /* Linear attenuation factor.          */
    float attenuation_q;    /* Quadratic attenuation factor.       */
    float spot_inner_cos;   /* cos(inner angle), spot lights only. */
    float spot_outer_cos;   /* cos(outer angle), spot lights only. */
    uint type;
};

layout(std140) uniform br_scene_state
{
    vec4 eye_view; /* Eye position in view-space */
    br_light lights[MAX_LIGHTS];
    vec4 clip_planes[MAX_CLIP_PLANES];
    uint num_lights;
    uint num_clip_planes;
};

layout(std140) uniform br_model_state
{
    mat4 model_view;
    mat4 projection;
    mat4 mvp;
    mat4 normal_matrix;
    mat4 environment;
    mat4 map_transform;
    vec4 surface_colour;
    vec4 clear_colour;
    vec4 eye_m; /* Eye position in model-space */

    float ka; /* Ambient mod */
    float ks; /* Specular mod (doesn't seem to be used by Croc) */
    float kd; /* Diffuse mod */
    float power;
    uint unlit; /* Is this surface unlit? */
    int uv_source;
    bool disable_colour_key;
    bool is_indexed; /* Is our texture indexed? */
    bool is_filtered;
};

float calculateAttenuation(in br_light alp, in float dist)
{
#if USE_LEGACY_ATTENUATION
    if (dist > alp.attenuation_q)
        return 0.0;

    float attn;

    if (dist > alp.attenuation_c)
        attn = (dist - alp.attenuation_c) * alp.attenuation_l;
    else
        attn = 0.0;

    return 1.0 - attn;
#else
    return 1.0 / (alp.attenuation_c + (alp.attenuation_l * dist) + (alp.attenuation_q * dist * dist));
#endif
}

void lightingColourAmbient(in vec4 p, in vec4 n, in br_light alp, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    outA += ka * alp.intensity * alp.colour.xyz;
}

void lightingColourDirect(in vec4 p, in vec4 n, in br_light alp, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    float diffDot = max(dot(n, alp.direction), 0.0);
    outD += diffDot * alp.intensity * kd * alp.colour.xyz;

    float specDot = max(dot(n, alp.half_), 0.0);
    outS += ks * alp.intensity * alp.colour.rgb * pow(specDot, power);
}

void lightingColourPoint(in vec4 p, in vec4 n, in br_light alp, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    vec4 dirn = alp.position - p;
    vec4 dirn_norm = normalize(dirn);

    float dist = length(dirn);
    float atten = calculateAttenuation(alp, dist);

    float diffDot = max(dot(n, dirn_norm), 0.0);

    // FIXME: handle linear falloff
    // - BRT_ATTENUATION_TYPE_T == BRT_QUADRATIC = default, what we'd expect
    //                          == BRT_RADII     = Uses CALCULATE_ATTENUATION_RADII(), uses radius_{inner,outer}
    outD += diffDot * kd * alp.colour.xyz * atten;

    /*
     * lightingColourPoint() multiplies it by the intensity if it's got no
     * linear or quadratic components - but I've not seen this ever look correct.
     * In contrast lightingColourPointAttn() doesn't, as the intensity is just 1/C,
     * and C is accounted for in the attenuation.
     */
#if 0
    {
        vec3 xxx = diffDot * kd * alp.colour.xyz * atten;

        if (alp.attenuation_q == 0 && alp.attenuation_l == 0)
            xxx *= alp.intensity;

        outD += xxx;
    }
#endif
    float specDot = max(dot(n, normalize(eye_view + dirn_norm)), 0.0);
    outS += ks * alp.intensity * alp.colour.xyz * pow(specDot, power) * atten;
}

void lightingColourSpot(in vec4 p, in vec4 n, in br_light alp, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    /*
     * FIXME: We're calculating this twice (in lightingColourPoint).
     */
    vec4 dirn_norm = normalize(alp.position - p);

    /*
     * NB: To test this, stick a spot light on a camera and see if you see it.
     */
    float spotDot = dot(dirn_norm.xyz, alp.direction.xyz);
    if(spotDot <= alp.spot_outer_cos)
        return;

    float cutoff = 1.0;
    float innerOuterDiff = alp.spot_inner_cos - alp.spot_outer_cos;

    if(innerOuterDiff != 0.0) {
        cutoff = clamp((spotDot - alp.spot_outer_cos) / innerOuterDiff, 0.0, 1.0);
    }

    /*
     * A spot light is just a point light with a cutoff.
     */
    vec3 outAA = vec3(0);
    vec3 outDD = vec3(0);
    vec3 outSS = vec3(0);
    lightingColourPoint(p, n, alp, outAA, outDD, outSS);
    outA += outAA * cutoff;
    outD += outDD * cutoff;
    outS += outSS * cutoff;
    return;
}

void accumulateLights(in vec4 position, in vec4 normal, inout vec3 ambient, inout vec3 diffuse, inout vec3 specular)
{
#if DEBUG_DISABLE_LIGHTS
    diffuse += vec3(1);
    return;
#endif

    if (num_lights == 0u || unlit != 0u) {
        diffuse += vec3(1);
        return;
    }

    vec4 normalDirection = normal;

    /* This is shit, but this is the way the engine does it */
    bool directLightExists = false;

    for (uint i = 0u; i < num_lights; ++i) {
        switch(lights[i].type) {
#if !DEBUG_DISABLE_LIGHT_AMBIENT
            case BRT_AMBIENT:
                lightingColourAmbient(position, normalDirection, lights[i], ambient, diffuse, specular);
                break;
#endif

#if !DEBUG_DISABLE_LIGHT_DIRECTIONAL
            case BRT_DIRECT:
                directLightExists = true;
                lightingColourDirect(position, normalDirection, lights[i], ambient, diffuse, specular);
                break;
#endif

#if !DEBUG_DISABLE_LIGHT_POINT
            case BRT_POINT:
                lightingColourPoint(position, normalDirection, lights[i], ambient, diffuse, specular);
                break;
#endif

#if !DEBUG_DISABLE_LIGHT_SPOT
            case BRT_SPOT:
                lightingColourSpot(position, normalDirection, lights[i], ambient, diffuse, specular);
                break;
#endif
        }
    }

    if (!directLightExists && num_lights > 0u && unlit == 0u) {
        diffuse += clear_colour.rgb;
    }
}
