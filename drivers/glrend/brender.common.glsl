#define MAX_LIGHTS                   48 /* Must match up with BRender */
#define MAX_CLIP_PLANES              6  /* Must match up with BRender */
#define SPECULARPOW_CUTOFF           0.6172
#define BR_SCALAR_EPSILON            1.192092896e-7f

#define UV_SOURCE_MODEL              0
#define UV_SOURCE_ENV_L              1
#define UV_SOURCE_ENV_I              2

#define BRT_AMBIENT                  0u
#define BRT_DIRECT                   1u
#define BRT_POINT                    2u
#define BRT_SPOT                     3u

#define BRT_QUADRATIC                0u /* Quadratic attenuation, i.e. standard 1/clq ... */
#define BRT_RADII                    1u /* Radial attenuation, i.e. linear falloff. */

#define DEBUG_DISABLE_LIGHTS            0
#define DEBUG_DISABLE_LIGHT_AMBIENT     0
#define DEBUG_DISABLE_LIGHT_DIRECTIONAL 0
#define DEBUG_DISABLE_LIGHT_POINT       0
#define DEBUG_DISABLE_LIGHT_SPOT        0

#define ENABLE_GOURAUD                  1
#define ENABLE_PHONG                    0

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
    uint attenuation_type;  /* Attenuation type, BRT_QUADRATIC or BRT_RADII. */
    float radius_inner;
    float radius_outer;
};

layout(std140, binding=0) uniform br_scene_state
{
    vec4 eye_view; /* Eye position in view-space */
    br_light lights[MAX_LIGHTS];
    vec4 clip_planes[MAX_CLIP_PLANES];
    vec4 ambient_colour;
    uvec4 light_start;
    uvec4 light_end;
    uint num_clip_planes;
    bool use_ambient_colour;
};

layout(std140, binding=1) uniform br_model_state
{
    mat4 model_view;
    mat4 projection;
    mat4 mvp;
    mat4 normal_matrix;
    mat4 environment;
    mat4 map_transform;
    vec4 surface_colour;
    vec4 eye_m; /* Eye position in model-space */
    vec4 fog_colour;
    vec2 fog_range; /* (min, max) */

    float ka; /* Ambient mod */
    float ks; /* Specular mod (doesn't seem to be used by Croc) */
    float kd; /* Diffuse mod */
    float power;
    uint unlit; /* Is this surface unlit? */
    int uv_source;
    bool disable_colour_key;
    bool is_indexed; /* Is our texture indexed? */
    bool is_filtered;
    bool enable_fog;
    float fog_scale;
};

float calculateAttenuation(in br_light alp, in float dist)
{
    return 1.0 / (alp.attenuation_c + (alp.attenuation_l * dist) + (alp.attenuation_q * dist * dist));
}

float calculateAttenuationRadii(in br_light alp, in float dist, in float intensity)
{
    /*
     * NB: radius_outer != radius_inner is enforced CPU-side.
     */
    float t = clamp((dist - alp.radius_inner) / (alp.radius_outer - alp.radius_inner), 0.0, 1.0);
    return intensity * (1.0 - t);
}

/*
 * Radial ambient lights, who ever thought such things could be?
 * See softrend/light24.c, lightingColourAmbientRadii()
 */
void lightingColourAmbientRadii(in vec4 p, in vec4 n, in br_light alp, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    float atten = 1.0f;

    vec4 dirn = alp.position - p;
    float dist = length(dirn);

    if(dist >= alp.radius_outer)
        return;

    atten = calculateAttenuationRadii(alp, dist, alp.intensity);

    outA += ka * alp.intensity * alp.colour.xyz * atten;
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
    float atten = 0.0f;

    if(alp.attenuation_type == BRT_RADII) {
        if(dist >= alp.radius_outer)
            return;

        // atten = calculateAttenuationRadii(alp, dist, calculateAttenuation(alp, dist));
        atten = calculateAttenuationRadii(alp, dist, alp.intensity);
    } else {
        atten = calculateAttenuation(alp, dist);
    }

    if(atten <= 0)
        return;

    float diffDot = max(dot(n, dirn_norm), 0.0);
    outD += diffDot * kd * alp.colour.xyz * atten;

    float specDot = max(dot(n, normalize(eye_view + dirn_norm)), 0.0);
    outS += ks * alp.colour.xyz * pow(specDot, power) * atten;
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

/*
 * Lighting accumulation function. Does A/D/S separately.
 *
 * NB: For regular (i.e. non-radial/non-linear-falloff) ambient lights, if there's no global contribution
 *     then we need to apply the ambient constant (ka) flat to the diffuse colour.
 *     - See softrend/light24.c, SurfaceColourLit(), use_ambient_colour
 *     - See softrend/setup.c, ActiveLightsUpdate(), use_ambient_colour
 */
void accumulateLights(in vec4 position, in vec4 normal, inout vec3 ambient, inout vec3 diffuse, inout vec3 specular)
{
#if DEBUG_DISABLE_LIGHTS
    diffuse += vec3(1);
    return;
#endif

    if(unlit != 0u) {
        diffuse += vec3(1);
        return;
    }

    vec4 normalDirection = normal;

#if !DEBUG_DISABLE_LIGHT_AMBIENT
    /*
     * If no non-radial ambient contributions, apply ka flat.
     * See above note.
     */
    if(use_ambient_colour) {
        diffuse += ka * ambient_colour.xyz;
    } else {
        diffuse += ka;
    }

    for(uint i = light_start.x; i < light_end.x; ++i) {
        lightingColourAmbientRadii(position, normalDirection, lights[i], ambient, diffuse, specular);
    }
#endif

#if !DEBUG_DISABLE_LIGHT_DIRECTIONAL
    for(uint i = light_start.y; i < light_end.y; ++i) {
        lightingColourDirect(position, normalDirection, lights[i], ambient, diffuse, specular);
    }
#endif

#if !DEBUG_DISABLE_LIGHT_POINT
    for(uint i = light_start.z; i < light_end.z; ++i) {
        lightingColourPoint(position, normalDirection, lights[i], ambient, diffuse, specular);
    }
#endif

#if !DEBUG_DISABLE_LIGHT_SPOT
    for(uint i = light_start.w; i < light_end.w; ++i) {
        lightingColourSpot(position, normalDirection, lights[i], ambient, diffuse, specular);
    }
#endif
}
