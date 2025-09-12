#define MAX_LIGHTS                   48 /* Must match up with BRender */
#define MAX_CLIP_PLANES              6  /* Must match up with BRender */
#define SPECULARPOW_CUTOFF           0.6172
#define BR_SCALAR_EPSILON            1.192092896e-7f

#define UV_SOURCE_MODEL              0
#define UV_SOURCE_ENV_L              1
#define UV_SOURCE_ENV_I              2

#define TEXTURE_MODE_NORMAL          0u
#define TEXTURE_MODE_INDEX           1u
#define TEXTURE_MODE_INDEX_FILTER    2u

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

layout(std140, binding=0) uniform br_scene_state
{
    vec4 eye_view; /* Eye position in view-space */
    uvec4 light_info[MAX_LIGHTS];
    vec4 light_positions[MAX_LIGHTS];
    vec4 light_directions[MAX_LIGHTS];
    vec4 light_halfs[MAX_LIGHTS];
    vec4 light_colours[MAX_LIGHTS];
    vec4 light_atten[MAX_LIGHTS];
    vec4 light_radii[MAX_LIGHTS];
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
    bool use_vertex_colour;
    int uv_source;
    bool disable_colour_key;
    uint texture_mode;
    bool enable_fog;
    float fog_scale;
};

float calculateAttenuation(in uint i, in float dist)
{
    const float attenuation_c = light_atten[i][1];
    const float attenuation_l = light_atten[i][2];
    const float attenuation_q = light_atten[i][3];

    return 1.0 / (attenuation_c + (attenuation_l * dist) + (attenuation_q * dist * dist));
}

float calculateAttenuationRadii(in uint i, in float dist, in float intensity)
{
    const float radius_inner = light_radii[i][2];
    const float radius_outer = light_radii[i][3];

    /*
     * NB: radius_outer != radius_inner is enforced CPU-side.
     */
    float t = clamp((dist - radius_inner) / (radius_outer - radius_inner), 0.0, 1.0);
    return intensity * (1.0 - t);
}

/*
 * Radial ambient lights, who ever thought such things could be?
 * See softrend/light24.c, lightingColourAmbientRadii()
 */
void lightingColourAmbientRadii(in vec3 p, in vec3 n, in uint i, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    const float intensity    = light_atten[i][0];
    const float radius_outer = light_radii[i][3];
    const vec3  position     = light_positions[i].xyz;

    float atten = 1.0f;

    vec3 dirn = position - p;
    float dist = length(dirn);

    if(dist >= radius_outer)
        return;

    atten = calculateAttenuationRadii(i, dist, intensity);

    // FIXME: should the intensity be multiplied here?
    outA += ka * intensity * light_colours[i].xyz * atten;
}

void lightingColourDirect(in vec3 p, in vec3 n, in uint i, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    const float intensity = light_atten[i][0];
    const vec3  colour    = light_colours[i].rgb;
    const vec3  direction = light_directions[i].xyz;

    float diffDot = max(dot(n, direction), 0.0);
    outD += diffDot * intensity * kd * colour;

    if(ks > 0.0) {
        float specDot = max(dot(n, light_halfs[i].xyz), 0.0);
        if(specDot > 0.0) {
            outS += ks * intensity * colour * pow(specDot, power);
        }
    }
}

void lightingColourPoint(in vec3 p, in vec3 n, in uint i, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    const uint  attenuation_type = light_info[i][1];
    const float intensity        = light_atten[i][0];
    const vec3  colour           = light_colours[i].rgb;
    const float radius_outer     = light_radii[i][3];
    const vec3  position         = light_positions[i].xyz;

    vec3 dirn = position - p;
    float dist = length(dirn);
    vec3 dirn_norm = dirn / dist;

    float atten = 0.0f;

    if(attenuation_type == BRT_RADII) {
        if(dist >= radius_outer)
            return;

        atten = calculateAttenuationRadii(i, dist, intensity);
    } else {
        atten = calculateAttenuation(i, dist);
    }

    if(atten <= 0)
        return;

    float diffDot = max(dot(n, dirn_norm), 0.0);
    outD += diffDot * kd * colour * atten;

    if(ks > 0.0) {
        float specDot = max(dot(n, normalize(eye_view.xyz + dirn_norm)), 0.0);
        if(specDot > 0.0) {
            outS += ks * colour * pow(specDot, power) * atten;
        }
    }
}

void lightingColourSpot(in vec3 p, in vec3 n, in uint i, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    const float spot_inner_cos = light_radii[i][0];
    const float spot_outer_cos = light_radii[i][1];
    const vec3  position       = light_positions[i].xyz;
    const vec3  direction      = light_directions[i].xyz;

    /*
     * FIXME: We're calculating this twice (in lightingColourPoint).
     */
    vec3 dirn_norm = normalize(position - p);

    /*
     * NB: To test this, stick a spot light on a camera and see if you see it.
     */
    float spotDot = dot(dirn_norm, direction);
    if(spotDot <= spot_outer_cos)
        return;

    float cutoff = 1.0;
    float innerOuterDiff = spot_inner_cos - spot_outer_cos;

    if(innerOuterDiff != 0.0) {
        cutoff = clamp((spotDot - spot_outer_cos) / innerOuterDiff, 0.0, 1.0);
    }

    /*
     * A spot light is just a point light with a cutoff.
     */
    vec3 outAA = vec3(0);
    vec3 outDD = vec3(0);
    vec3 outSS = vec3(0);
    lightingColourPoint(p, n, i, outAA, outDD, outSS);
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
void accumulateLights(in vec3 position, in vec3 normal, inout vec3 ambient, inout vec3 diffuse, inout vec3 specular)
{
#if DEBUG_DISABLE_LIGHTS
    diffuse += vec3(1);
    return;
#endif

    if(unlit != 0u) {
        diffuse += vec3(1);
        return;
    }

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
        lightingColourAmbientRadii(position, normal, i, ambient, diffuse, specular);
    }
#endif

#if !DEBUG_DISABLE_LIGHT_DIRECTIONAL
    for(uint i = light_start.y; i < light_end.y; ++i) {
        lightingColourDirect(position, normal, i, ambient, diffuse, specular);
    }
#endif

#if !DEBUG_DISABLE_LIGHT_POINT
    for(uint i = light_start.z; i < light_end.z; ++i) {
        lightingColourPoint(position, normal, i, ambient, diffuse, specular);
    }
#endif

#if !DEBUG_DISABLE_LIGHT_SPOT
    for(uint i = light_start.w; i < light_end.w; ++i) {
        lightingColourSpot(position, normal, i, ambient, diffuse, specular);
    }
#endif
}
