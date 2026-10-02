#define MAX_LIGHTS                   48 /* Must match up with BRender */
#define MAX_CLIP_PLANES              6  /* Must match up with BRender */
#define SPECULARPOW_CUTOFF           0.6172
#define BR_SCALAR_EPSILON            1.192092896e-7f

#define UV_SOURCE_MODEL              0
#define UV_SOURCE_ENV_L              1
#define UV_SOURCE_ENV_I              2
#define UV_SOURCE_GEOMETRY_X         3
#define UV_SOURCE_GEOMETRY_Y         4
#define UV_SOURCE_GEOMETRY_Z         5

#define TEXTURE_MODE_NORMAL          0u
#define TEXTURE_MODE_INDEX           1u
#define TEXTURE_MODE_INDEX_FILTER    2u

#define BRT_AMBIENT                  0u
#define BRT_DIRECT                   1u
#define BRT_POINT                    2u
#define BRT_SPOT                     3u

#define BRT_QUADRATIC                0u /* Quadratic attenuation, i.e. standard 1/clq ... */
#define BRT_RADII                    1u /* Radial attenuation, i.e. linear falloff. */

#define COLOUR_SOURCE_GEOMETRY       0u
#define COLOUR_SOURCE_SURFACE        1u

#define SHADING_MODE_FLAT            0u
#define SHADING_MODE_GOURAUD         1u
#define SHADING_MODE_PHONG           2u

#define DEBUG_DISABLE_LIGHTS            0
#define DEBUG_DISABLE_LIGHT_AMBIENT     0
#define DEBUG_DISABLE_LIGHT_DIRECTIONAL 0
#define DEBUG_DISABLE_LIGHT_POINT       0
#define DEBUG_DISABLE_LIGHT_SPOT        0

/*
 * One light, mirroring br_gl_main_data_light. Fields are interleaved so the
 * used part of the block is a contiguous prefix.
 */
struct br_light {
    uvec4 info;      /* (type, atten_type, 0, 0) */
    vec4  position;  /* (X, Y, Z, w) */
    vec4  direction; /* (X, Y, Z, 0), normalised */
    vec4  halfway;   /* (X, Y, Z, 0), normalised */
    vec4  colour;    /* (R, G, B, 0) */
    vec4  atten;     /* (intensity, C, L, Q) */
    vec4  radii;     /* (cos(inner), cos(outer), radius_inner, radius_outer) */
};

layout(std140, binding=0) uniform br_scene_state
{
    vec4 eye_view; /* Eye position in view-space */
    vec4 clip_planes[MAX_CLIP_PLANES];
    vec4 ambient_colour;
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
    bool lighting;      /* BRT_LIGHTING_B */
    bool prelighting;   /* BRT_PRELIGHTING_B */
    int colour_source;
    int uv_source;
    bool disable_colour_key;
    uint texture_mode;
    bool enable_fog;
    float fog_scale;
    int shading_mode;

    /*
     * The lights for this draw, packed by type. These are the same names the
     * light functions have always used - they simply live in the per-draw block
     * now, so a draw only carries the lights it can actually see.
     */
    uvec4 light_start;
    uvec4 light_end;
    br_light lights[MAX_LIGHTS];
};

float calculateAttenuation(in uint i, in float dist)
{
    const float attenuation_c = lights[i].atten[1];
    const float attenuation_l = lights[i].atten[2];
    const float attenuation_q = lights[i].atten[3];

    return 1.0 / (attenuation_c + (attenuation_l * dist) + (attenuation_q * dist * dist));
}

float calculateAttenuationRadii(in uint i, in float dist, in float intensity)
{
    const float radius_inner = lights[i].radii[2];
    const float radius_outer = lights[i].radii[3];

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
    const float intensity    = lights[i].atten[0];
    const float radius_outer = lights[i].radii[3];
    const vec3  position     = lights[i].position.xyz;

    float atten = 1.0f;

    vec3 dirn = position - p;
    float dist = length(dirn);

    if(dist >= radius_outer)
        return;

    atten = calculateAttenuationRadii(i, dist, intensity);

    // FIXME: should the intensity be multiplied here?
    outA += ka * intensity * lights[i].colour.xyz * atten;
}

void lightingColourDirect(in vec3 p, in vec3 n, in uint i, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    const float intensity = lights[i].atten[0];
    const vec3  colour    = lights[i].colour.rgb;
    const vec3  direction = lights[i].direction.xyz;

    float diffDot = max(dot(n, direction), 0.0);
    outD += diffDot * kd * colour; /* NB: Intensity is scaled into the direction CPU-side (in cache.c) */

    if(ks > 0.0) {
        float specDot = max(dot(n, lights[i].halfway.xyz), 0.0);
        if(specDot > 0.0) {
            outS += ks * intensity * colour * pow(specDot, power);
        }
    }
}

void lightingColourPoint(in vec3 p, in vec3 n, in uint i, inout vec3 outA, inout vec3 outD, inout vec3 outS)
{
    const uint  attenuation_type = lights[i].info[1];
    const float intensity        = lights[i].atten[0];
    const vec3  colour           = lights[i].colour.rgb;
    const float radius_outer     = lights[i].radii[3];
    const vec3  position         = lights[i].position.xyz;

    vec3 dirn = position - p;
    float dist_sqr = dot(dirn, dirn);

    if(attenuation_type == BRT_RADII && dist_sqr >= radius_outer * radius_outer)
        return;

    float dist = sqrt(dist_sqr);
    float atten = 0.0f;

    if(attenuation_type == BRT_RADII) {
        atten = calculateAttenuationRadii(i, dist, intensity);
    } else {
        atten = calculateAttenuation(i, dist);
    }

    if(atten <= 0)
        return;

    vec3 dirn_norm = dirn / dist;

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
    const float spot_inner_cos = lights[i].radii[0];
    const float spot_outer_cos = lights[i].radii[1];
    const vec3  position       = lights[i].position.xyz;
    const vec3  direction      = lights[i].direction.xyz;

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
    if(!lighting) {
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
