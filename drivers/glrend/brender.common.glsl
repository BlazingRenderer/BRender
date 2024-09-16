#define MAX_LIGHTS                   48 /* Must match up with BRender */
#define SPECULARPOW_CUTOFF           0.6172
#define BR_SCALAR_EPSILON            1.192092896e-7f

#define ENABLE_GAMMA_CORRECTION      0
#define ENABLE_SIMULATE_8BIT_COLOUR  0
#define ENABLE_SIMULATE_16BIT_COLOUR 0

#define UV_SOURCE_MODEL              0
#define UV_SOURCE_ENV_L              1
#define UV_SOURCE_ENV_I              2

#define DEBUG_DISABLE_LIGHTS            0
#define DEBUG_DISABLE_LIGHT_AMBIENT     1
#define DEBUG_DISABLE_LIGHT_DIRECTIONAL 0
#define DEBUG_DISABLE_LIGHT_POINT       0
#define DEBUG_DISABLE_LIGHT_POINTATTEN  0
#define DEBUG_DISABLE_LIGHT_SPOT        1
#define DEBUG_DISABLE_LIGHT_SPECULAR    1
#define ENABLE_GOURAUD                  0
#define ENABLE_PHONG                    1
#define ENABLE_PSX_SIMULATION           0

struct br_light
{
    vec4 position;    /* (X, Y, Z, 1) */
    vec4 direction;   /* (X, Y, Z, 0), normalised */
    vec4 half_;       /* (X, Y, Z, 0), normalised */
    vec4 colour;      /* (R, G, B, 0), normalised */
    vec4 iclq;        /* (intensity, constant, linear, attenutation) */
    vec2 spot_angles; /* (inner, outer), if (0.0, 0.0), then this is a point light. */
    float falloff;
    float cutoff;
    float spot_falloff;
};

layout(std140) uniform br_scene_state
{
    vec4 eye_view; /* Eye position in view-space */
    uint num_lights;
    uint use_ambient_colour;
    float ambient_red;
    float ambient_green;
    float ambient_blue;
    uint use_ambient_intensity;
    float ambient_intensity;
    br_light lights[MAX_LIGHTS];
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

#define SPECULAR_DOT()                    \
    {                                     \
        float rd = dot(dirn_norm, n) * 2; \
        vec4 r = n - rd;                  \
        r = r - dirn_norm;                \
                                          \
        _dot = dot(eye_view, r);          \
    }

#define SPECULAR_POWER(l) (_dot * (l)) / (power - (power * _dot) + _dot)

float shadingFilter(in float i)
{
    /* Software shading emulation */
    #if ENABLE_PSX_SIMULATION
    i = floor(i * 255.0) / 255.0;
    #endif
    return i;
}



vec4 _SpecColour = vec4(1.0);
float _Shininess = 64.0;

float calculateAttenuation(in br_light alp, in float dist)
{
    float c = alp.iclq.y;
    float l = alp.iclq.z;
    float q = alp.iclq.w;

    return 1.0 / (c + (l * dist) + (q * dist * dist));
    if (dist > alp.iclq.w)
        return 0.0;

    float attn;

    if (dist > alp.iclq.y)
        attn = (dist - alp.iclq.y) * alp.iclq.z;
    else
        attn = 0.0;

    return 1.0 - attn;
}

float calculateSpecularPhong(in vec3 lightDir, in vec3 normal)
{
    vec3 viewDir = normalize(-vFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    return pow(max(dot(viewDir, reflectDir), 0.0), _Shininess);
}

float calculateSpecularBlinnPhong(in vec3 lightDir, in vec3 normal)
{
    vec3 viewDir = normalize(-vFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    return pow(max(dot(normal, halfwayDir), 0.0), _Shininess * 4);
}

float calculateSpecular(in vec3 lightDir, in vec3 normal)
{
    return calculateSpecularBlinnPhong(lightDir, normal);
}


vec3 lightingColourAmbient(in vec4 p, in vec4 n, in br_light alp)
{
    return ka * alp.iclq.x * alp.colour.xyz;
}


vec3 calculateASD(in br_light alp, in vec3 lightDir, in vec3 normal, in vec3 surfaceColour)
{
    vec3 lightColour = alp.colour.xyz;
    float intensity = alp.iclq.x;

    vec3 ambient = ka * lightColour * surfaceColour;

    /* Do diffuse. */
    float _dot = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = kd * _dot * lightColour * surfaceColour;

    /* Only do specular if the light's actually hitting the surface. */
    vec3 specular = vec3(0.0);
    if(_dot > 0)
        specular = ks * calculateSpecular(lightDir, normal) * lightColour * _SpecColour.xyz;

    return (ambient + diffuse + specular) * intensity;
}

vec3 lightDirect(in br_light alp, in vec3 normal, in vec3 surfaceColour)
{
    vec3 lightDir = -alp.direction.xyz;
    return calculateASD(alp, lightDir, normal, surfaceColour);
}


vec3 lightPoint(in br_light alp, in vec3 normal, in vec3 surfaceColour)
{
    vec3 lightDir = alp.position.xyz - vFragPos;
    float dist = length(lightDir);
    lightDir = normalize(lightDir);

    vec3 colour = calculateASD(i, lightDir, normal, surfaceColour);
    float atten = calculateAttenuation(alp.iclq.yzw, dist);

    return colour * atten * alp.iclq.x;
}

vec3 lightingColourDirect(in vec4 p, in vec4 n, in br_light alp)
{
    return lightDirect(alp, n.xyz, vec3(1, 1, 1));
    //vec3 lightDir = -alp.direction.xyz;


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



vec4 fragmainXX(in vec4 position, in vec4 normal, out bool directLightExists)
{
//#if DEBUG_DISABLE_LIGHTS
    //return surface_colour;
//#endif

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
