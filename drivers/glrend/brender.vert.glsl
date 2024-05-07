#version 150

#define MAX_LIGHTS                      48 /* Must match up with BRender */
#define SPECULARPOW_CUTOFF              0.6172
#define BR_SCALAR_EPSILON               1.192092896e-7f

#define DEBUG_DISABLE_LIGHTS            0
#define DEBUG_DISABLE_LIGHT_AMBIENT     0
#define DEBUG_DISABLE_LIGHT_DIRECTIONAL 0
#define DEBUG_DISABLE_LIGHT_POINT       0
#define DEBUG_DISABLE_LIGHT_POINTATTEN  0
#define DEBUG_DISABLE_LIGHT_SPOT        1
#define DEBUG_DISABLE_LIGHT_SPECULAR    0
#define ENABLE_PSX_SIMULATION           0

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
};

in vec3 aPosition;
in vec2 aUV;
in vec3 aNormal;
in vec4 aColour;

out vec4 position;
out vec4 normal;
out vec2 uv;
out vec4 colour;

out vec3 rawPosition;
out vec3 rawNormal;

bool directLightExists;

#define SPECULAR_DOT()                    \
    {                                     \
        float rd = dot(dirn_norm, n) * 2; \
        vec4 r = n - rd;                  \
        r = r - dirn_norm;                \
                                          \
        _dot = dot(eye_view, r);          \
    }

#define SPECULAR_POWER(l) (_dot * (l)) / (power - (power * _dot) + _dot)

float calculateAttenuation(in br_light alp, in float dist)
{
    if (dist > alp.iclq.w)
        return 0.0;

    float attn;

    if (dist > alp.iclq.y)
        attn = (dist - alp.iclq.y) * alp.iclq.z;
    else
        attn = 0.0;

    return 1.0 - attn;
}

float shadingFilter(in float i)
{
/* Software shading emulation */
#if ENABLE_PSX_SIMULATION
    i = floor(i * 255.0) / 255.0;
#endif
    return i;
}

vec3 lightingColourAmbient(in vec4 p, in vec4 n, in br_light alp)
{
    return ka * alp.iclq.x * alp.colour.xyz;
}

vec3 lightingColourDirect(in vec4 p, in vec4 n, in br_light alp)
{
    /* Notes: '_dot' is 'intensity' */
    float _dot = max(dot(n, alp.direction), 0.0) * kd;

    /* Shading filters like 'toon' */
    _dot = shadingFilter(_dot);

    /*
     * -- Kludge to emulate the broken D3D lighting from v1.0 and v1.1
     * vec3 outColour = vec3((alp.colour.x + alp.colour.y + alp.colour.z) / 3.0) * _dot;
     */
    vec3 outColour = alp.colour.xyz * _dot;

#if !DEBUG_DISABLE_LIGHT_SPECULAR
    if (ks <= 0.0) {
        _dot = dot(n, alp.half_);

        if (_dot > SPECULARPOW_CUTOFF)
            outColour += SPECULAR_POWER(ks * alp.iclq.x);
    }
#endif
    return outColour;
}

vec3 lightingColourPoint(in vec4 p, in vec4 n, in br_light alp)
{
    float _dot;
    vec4 dirn, dirn_norm;

    /* Work out vector between point and light source */
    dirn = alp.position - p;
    dirn_norm = normalize(dirn);

    float dist = length(dirn);

    _dot = max(dot(n, dirn_norm), 0.0) * kd;

    /* Shading filters like 'toon' */
    _dot = shadingFilter(_dot);

    vec3 outColour = _dot * (alp.iclq.x * alp.colour.xyz);

#if !DEBUG_DISABLE_LIGHT_SPECULAR
    if (ks != 0.0) {
        /* Specular */
        SPECULAR_DOT();

        /* Phong lighting approximation from Gems IV pg. 385 */
        if (_dot > SPECULARPOW_CUTOFF)
            outColour += SPECULAR_POWER(ks);
    }
#endif
    return (outColour);
}

vec3 lightingColourPointAtten(in vec4 p, in vec4 n, in br_light alp)
{
    float _dot;
    vec4 dirn, dirn_norm;

    /* Work out vector between point and light source */
    dirn = alp.position - p;
    dirn_norm = normalize(dirn);
    _dot = ((max(dot(n, dirn_norm), 0.0) * kd));

    /* Shading filters like 'toon' */
    _dot = shadingFilter(_dot);

    float dist = length(dirn);
    float atten = calculateAttenuation(alp, dist);

    vec3 outColour = _dot * alp.colour.xyz;

#if !DEBUG_DISABLE_LIGHT_SPECULAR
    if (ks != 0.0) {
        /* Specular */
        SPECULAR_DOT();

        /* Phong lighting approximation from Gems IV pg. 385 */
        if (_dot > SPECULARPOW_CUTOFF)
            outColour += SPECULAR_POWER(ks);
    }
#endif
    return outColour * atten;
}

vec3 lightingColourSpot(in vec4 p, in vec4 n, in br_light alp)
{
    /* Croc doesn't use spot lights */
    return vec3(0);
}

vec3 lightingColourSpotAtten(in vec4 p, in vec4 n, in br_light alp)
{
    /* Croc doesn't use spot lights */
    return vec3(0);
}

vec4 fragmain()
{
#if DEBUG_DISABLE_LIGHTS
    return surface_colour;
#endif

    if (num_lights == 0u || unlit != 0u) {
        return surface_colour;
    }

    vec4 normalDirection = normal;

    vec3 _colour = surface_colour.xyz;

    /* This is shit, but this is the way the engine does it */
    vec3 lightColour = vec3(0.0);
    vec3 directLightColour = vec3(0.0);
    directLightExists = false;

    for (uint i = 0u; i < num_lights; ++i) {
#if !DEBUG_DISABLE_LIGHT_AMBIENT
        if(lights[i].colour.w != 0.0) {
            lightColour += lightingColourAmbient(position, normalDirection, lights[i]);
            continue;
        }
#endif
        if (lights[i].position.w == 0) {
#if !DEBUG_DISABLE_LIGHT_DIRECTIONAL
            directLightExists = true;
            directLightColour += lightingColourDirect(position, normalDirection, lights[i]);
#endif
        } else {
            if (lights[i].spot_angles == vec2(0.0, 0.0)) {
                if (lights[i].iclq.zw == vec2(0)) {
#if !DEBUG_DISABLE_LIGHT_POINT
                    lightColour += lightingColourPoint(position, normalDirection, lights[i]);
#endif
                } else {
#if !DEBUG_DISABLE_LIGHT_POINTATTEN
                    lightColour += lightingColourPointAtten(position, normalDirection, lights[i]);
#endif
                }
            } else {
#if !DEBUG_DISABLE_LIGHT_SPOT
                if (lights[i].iclq.zw == vec2(0))
                    lightColour += lightingColourSpot(position, normalDirection, lights[i]);
                else
                    lightColour += lightingColourSpotAtten(position, normalDirection, lights[i]);
#endif
            }
        }
    }

    lightColour += directLightColour;
    lightColour *= _colour;

    lightColour = clamp(lightColour, 0.0, 1.0);
    return vec4(lightColour, surface_colour.a);
}

#if ENABLE_PSX_SIMULATION
vec4 PSXify_pos(in vec4 vertex, in vec2 resolution)
{
    vec4 snappedPos = vertex;
    snappedPos.xyz = vertex.xyz / vertex.w;
    snappedPos.xy = floor(resolution.xy * snappedPos.xy) / resolution;
    snappedPos.xyz *= vertex.w;
    return snappedPos;
}
#endif

void main()
{
    vec4 pos = vec4(aPosition, 1.0);

    position = model_view * pos;
    normal = vec4(normalize(mat3(normal_matrix) * aNormal), 0);
    uv = aUV;
    colour = aColour + fragmain();

    rawPosition = aPosition;
    rawNormal   = aNormal;

    if (!directLightExists && num_lights > 0u && unlit == 0u)
        colour += vec4(clear_colour.rgb, 0.0);

#if ENABLE_PSX_SIMULATION
    pos = PSXify_pos(mvp * pos, vec2(200.0, 150.0));
#else
    pos = mvp * pos;
#endif

    gl_Position = pos;
}
