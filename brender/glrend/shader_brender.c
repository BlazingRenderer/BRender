#include "drv.h"

/* REMEMBER TO UPDATE THESE IF YOU CHANGE THE EXTERNAL SHADERS */

static const GLchar g_DefaultVertexShader[] =
                        "#version 150\n"
                        "\n"
                        "#define MAX_LIGHTS                      48 /* Must match up with BRender */\n"
                        "#define SPECULARPOW_CUTOFF              0.6172\n"
                        "#define BR_SCALAR_EPSILON               1.192092896e-7f\n"
                        "\n"
                        "#define DEBUG_DISABLE_LIGHTS            0\n"
                        "#define DEBUG_DISABLE_LIGHT_DIRECTIONAL 0\n"
                        "#define DEBUG_DISABLE_LIGHT_POINT       0\n"
                        "#define DEBUG_DISABLE_LIGHT_POINTATTEN  0\n"
                        "#define DEBUG_DISABLE_LIGHT_SPOT        1\n"
                        "#define DEBUG_DISABE_LIGHT_SPECULAR     0\n"
                        "#define ENABLE_PSX_SIMULATION           0\n"
                        "\n"
                        "in vec3 aPosition;\n"
                        "in vec2 aUV;\n"
                        "in vec3 aNormal;\n"
                        "in vec4 aColour;\n"
                        "\n"
                        "out vec4 position;\n"
                        "out vec4 normal;\n"
                        "out vec2 uv;\n"
                        "out vec4 colour;\n"
                        "bool directLightExists;\n"
                        "\n"
                        "struct br_light\n"
                        "{\n"
                        "    vec4 position;    /* (X, Y, Z, 1) */\n"
                        "    vec4 direction;   /* (X, Y, Z, 0), normalised */\n"
                        "    vec4 half_;       /* (X, Y, Z, 0), normalised */\n"
                        "    vec4 colour;      /* (R, G, B, 0), normalised */\n"
                        "    vec4 iclq;        /* (intensity, constant, linear, attenutation) */\n"
                        "    vec2 spot_angles; /* (inner, outer), if (0.0, 0.0), then this is a point light. */\n"
                        "};\n"
                        "\n"
                        "layout(std140) uniform br_scene_state\n"
                        "{\n"
                        "    vec4 eye_view; /* Eye position in view-space */\n"
                        "    vec4 global_ambient_colour;\n"
                        "    br_light lights[MAX_LIGHTS];\n"
                        "    uint num_lights;\n"
                        "};\n"
                        "\n"
                        "layout(std140) uniform br_model_state\n"
                        "{\n"
                        "    mat4 model_view;\n"
                        "    mat4 projection;\n"
                        "    mat4 mvp;\n"
                        "    mat4 normal_matrix;\n"
                        "    vec4 surface_colour;\n"
                        "\n"
                        "    vec4 ambient_colour; /* Material ambient colour. Is multiplied by surface_colour. */\n"
                        "    vec4 clear_colour;\n"
                        "    float ka; /* Ambient mod */\n"
                        "    float ks; /* Specular mod (doesn't seem to be used by Croc) */\n"
                        "    float kd; /* Diffuse mod */\n"
                        "    float power;\n"
                        "    uint unlit; /* Is this surface unlit? */\n"
                        "};\n"
                        "\n"
                        "#define SPECULAR_DOT()                    \\\n"
                        "    {                                     \\\n"
                        "        float rd = dot(dirn_norm, n) * 2; \\\n"
                        "        vec4 r = n - rd;                  \\\n"
                        "        r = r - dirn_norm;                \\\n"
                        "                                          \\\n"
                        "        _dot = dot(eye_view, r);          \\\n"
                        "    }\n"
                        "\n"
                        "#define SPECULAR_POWER(l) (_dot * (l)) / (power - (power * _dot) + _dot)\n"
                        "\n"
                        "float calculateAttenuation(in br_light alp, in float dist)\n"
                        "{\n"
                        "    if (dist > alp.iclq.w)\n"
                        "        return 0.0;\n"
                        "\n"
                        "    float attn;\n"
                        "\n"
                        "    if (dist > alp.iclq.y)\n"
                        "        attn = (dist - alp.iclq.y) * alp.iclq.z;\n"
                        "    else\n"
                        "        attn = 0.0;\n"
                        "\n"
                        "    return 1.0 - attn;\n"
                        "}\n"
                        "\n"
                        "float shadingFilter(in float i)\n"
                        "{\n"
                        "/* Software shading emulation */\n"
                        "#if ENABLE_PSX_SIMULATION\n"
                        "    i = floor(i * 255.0) / 255.0;\n"
                        "#endif\n"
                        "    return i;\n"
                        "}\n"
                        "\n"
                        "vec3 lightingColourDirect(in vec4 p, in vec4 n, in vec3 colour, in br_light alp)\n"
                        "{\n"
                        "    /* Notes: '_dot' is 'intensity' */\n"
                        "    float _dot = max(dot(n, alp.direction), 0.0) * kd;\n"
                        "\n"
                        "    /* Shading filters like 'toon' */\n"
                        "    _dot = shadingFilter(_dot);\n"
                        "\n"
                        "    /*\n"
                        "     * -- Kludge to emulate the broken D3D lighting from v1.0 and v1.1\n"
                        "     * vec3 outColour = vec3((alp.colour.x + alp.colour.y + alp.colour.z) / 3.0) * _dot;\n"
                        "     */\n"
                        "    vec3 outColour = alp.colour.xyz * _dot;\n"
                        "\n"
                        "#if !DEBUG_DISABE_LIGHT_SPECULAR\n"
                        "    if (ks <= 0.0) {\n"
                        "        _dot = dot(n, alp.half_);\n"
                        "\n"
                        "        if (_dot > SPECULARPOW_CUTOFF)\n"
                        "            outColour += SPECULAR_POWER(ks * alp.iclq.x);\n"
                        "    }\n"
                        "#endif\n"
                        "    return outColour;\n"
                        "}\n"
                        "\n"
                        "vec3 lightingColourPoint(in vec4 p, in vec4 n, in vec3 colour, in br_light alp)\n"
                        "{\n"
                        "    float _dot;\n"
                        "    vec4 dirn, dirn_norm;\n"
                        "\n"
                        "    /* Work out vector between point and light source */\n"
                        "    dirn = alp.position - p;\n"
                        "    dirn_norm = normalize(dirn);\n"
                        "\n"
                        "    float dist = length(dirn);\n"
                        "\n"
                        "    _dot = max(dot(n, dirn_norm), 0.0) * kd;\n"
                        "\n"
                        "    /* Shading filters like 'toon' */\n"
                        "    _dot = shadingFilter(_dot);\n"
                        "\n"
                        "    vec3 outColour = _dot * (alp.iclq.x * alp.colour.xyz);\n"
                        "\n"
                        "#if !DEBUG_DISABE_LIGHT_SPECULAR\n"
                        "    if (ks != 0.0) {\n"
                        "        /* Specular */\n"
                        "        SPECULAR_DOT();\n"
                        "\n"
                        "        /* Phong lighting approximation from Gems IV pg. 385 */\n"
                        "        if (_dot > SPECULARPOW_CUTOFF)\n"
                        "            outColour += SPECULAR_POWER(ks);\n"
                        "    }\n"
                        "#endif\n"
                        "    return (outColour);\n"
                        "}\n"
                        "\n"
                        "vec3 lightingColourPointAtten(in vec4 p, in vec4 n, in vec3 colour, in br_light alp)\n"
                        "{\n"
                        "    float _dot;\n"
                        "    vec4 dirn, dirn_norm;\n"
                        "\n"
                        "    /* Work out vector between point and light source */\n"
                        "    dirn = alp.position - p;\n"
                        "    dirn_norm = normalize(dirn);\n"
                        "    _dot = ((max(dot(n, dirn_norm), 0.0) * kd));\n"
                        "\n"
                        "    /* Shading filters like 'toon' */\n"
                        "    _dot = shadingFilter(_dot);\n"
                        "\n"
                        "    float dist = length(dirn);\n"
                        "    float atten = calculateAttenuation(alp, dist);\n"
                        "\n"
                        "    vec3 outColour = _dot * alp.colour.xyz;\n"
                        "\n"
                        "#if !DEBUG_DISABE_LIGHT_SPECULAR\n"
                        "    if (ks != 0.0) {\n"
                        "        /* Specular */\n"
                        "        SPECULAR_DOT();\n"
                        "\n"
                        "        /* Phong lighting approximation from Gems IV pg. 385 */\n"
                        "        if (_dot > SPECULARPOW_CUTOFF)\n"
                        "            outColour += SPECULAR_POWER(ks);\n"
                        "    }\n"
                        "#endif\n"
                        "    return outColour * atten;\n"
                        "}\n"
                        "\n"
                        "vec3 lightingColourSpot(in vec4 p, in vec4 n, in vec3 colour, in br_light alp)\n"
                        "{\n"
                        "    /* Croc doesn't use spot lights */\n"
                        "    return vec3(0);\n"
                        "}\n"
                        "\n"
                        "vec3 lightingColourSpotAtten(in vec4 p, in vec4 n, in vec3 colour, in br_light alp)\n"
                        "{\n"
                        "    /* Croc doesn't use spot lights */\n"
                        "    return vec3(0);\n"
                        "}\n"
                        "\n"
                        "vec4 fragmain()\n"
                        "{\n"
                        "#if DEBUG_DISABLE_LIGHTS\n"
                        "    return surface_colour;\n"
                        "#endif\n"
                        "\n"
                        "    if (num_lights == 0u || unlit != 0u) {\n"
                        "        return surface_colour;\n"
                        "    }\n"
                        "\n"
                        "    vec4 normalDirection = normal;\n"
                        "\n"
                        "    // -- FIX: fixes BR_TWO_SIDED faces\n"
                        "    /*if(gl_FrontFacing != true)\n"
                        "    {\n"
                        "        normalDirection *= -1.0;\n"
                        "    }*/\n"
                        "\n"
                        "    vec3 _colour = surface_colour.xyz * ka;\n"
                        "\n"
                        "    /* This is shit, but this is the way the engine does it */\n"
                        "    vec3 lightColour = vec3(0.0);\n"
                        "    vec3 directLightColour = vec3(0.0);\n"
                        "    directLightExists = false;\n"
                        "\n"
                        "    for (uint i = 0u; i < num_lights; ++i) {\n"
                        "        if (lights[i].position.w == 0) {\n"
                        "#if !DEBUG_DISABLE_LIGHT_DIRECTIONAL\n"
                        "            directLightExists = true;\n"
                        "            directLightColour += lightingColourDirect(position, normalDirection, _colour, lights[i]);\n"
                        "#endif\n"
                        "        } else {\n"
                        "            if (lights[i].spot_angles == vec2(0.0, 0.0)) {\n"
                        "                if (lights[i].iclq.zw == vec2(0)) {\n"
                        "#if !DEBUG_DISABLE_LIGHT_POINT\n"
                        "                    lightColour += lightingColourPoint(position, normalDirection, _colour, lights[i]);\n"
                        "#endif\n"
                        "                } else {\n"
                        "#if !DEBUG_DISABLE_LIGHT_POINTATTEN\n"
                        "                    lightColour += lightingColourPointAtten(position, normalDirection, _colour, lights[i]);\n"
                        "#endif\n"
                        "                }\n"
                        "            } else {\n"
                        "#if !DEBUG_DISABLE_LIGHT_SPOT\n"
                        "                if (lights[i].iclq.zw == vec2(0))\n"
                        "                    lightColour += lightingColourSpot(position, normalDirection, _colour, lights[i]);\n"
                        "                else\n"
                        "                    lightColour += lightingColourSpotAtten(position, normalDirection, _colour, lights[i]);\n"
                        "#endif\n"
                        "            }\n"
                        "        }\n"
                        "    }\n"
                        "\n"
                        "    /* Force minimum 'ambient' light if a directional light exists */\n"
                        "    if (directLightExists)\n"
                        "        lightColour += global_ambient_colour.rgb + directLightColour;\n"
                        "\n"
                        "    return vec4(_colour * lightColour, surface_colour.a);\n"
                        "}\n"
                        "\n"
                        "#if ENABLE_PSX_SIMULATION\n"
                        "vec4 PSXify_pos(in vec4 vertex, in vec2 resolution)\n"
                        "{\n"
                        "    vec4 snappedPos = vertex;\n"
                        "    snappedPos.xyz = vertex.xyz / vertex.w;\n"
                        "    snappedPos.xy = floor(resolution.xy * snappedPos.xy) / resolution;\n"
                        "    snappedPos.xyz *= vertex.w;\n"
                        "    return snappedPos;\n"
                        "}\n"
                        "#endif\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    vec4 pos = vec4(aPosition, 1.0);\n"
                        "\n"
                        "    position = model_view * pos;\n"
                        "    normal = normalize(vec4(mat3(normal_matrix) * aNormal, 0));\n"
                        "    uv = aUV;\n"
                        "    colour = aColour + fragmain();\n"
                        "\n"
                        "    if (!directLightExists && num_lights > 0u && unlit == 0u)\n"
                        "        colour += vec4(clear_colour.rgb, 0.0);\n"
                        "\n"
                        "#if ENABLE_PSX_SIMULATION\n"
                        "    pos = PSXify_pos(mvp * pos, vec2(200.0, 150.0));\n"
                        "#else\n"
                        "    pos = mvp * pos;\n"
                        "#endif\n"
                        "\n"
                        "    gl_Position = pos;\n"
                        "}";

static const GLchar g_DefaultFragmentShader[] =
                        "#version 150\n"
                        "\n"
                        "#define MAX_LIGHTS                   48 /* Must match up with BRender */\n"
                        "\n"
                        "#define DEBUG_DISABLE_COLOUR_KEY     0\n"
                        "#define ENABLE_GAMMA_CORRECTION      0\n"
                        "#define ENABLE_SIMULATE_8BIT_COLOUR  0\n"
                        "#define ENABLE_SIMULATE_16BIT_COLOUR 0\n"
                        "\n"
                        "struct br_light\n"
                        "{\n"
                        "    vec4 position;    /* (X, Y, Z, 1) */\n"
                        "    vec4 direction;   /* (X, Y, Z, 0), normalised */\n"
                        "    vec4 half_;       /* (X, Y, Z, 0), normalised */\n"
                        "    vec4 colour;      /* (R, G, B, 0), normalised */\n"
                        "    vec4 iclq;        /* (intensity, constant, linear, attenutation) */\n"
                        "    vec2 spot_angles; /* (inner, outer), if (0.0, 0.0), then this is a point light. */\n"
                        "};\n"
                        "\n"
                        "layout(std140) uniform br_scene_state\n"
                        "{\n"
                        "    vec4 eye_view; /* Eye position in view-space */\n"
                        "    vec4 global_ambient_colour;\n"
                        "    br_light lights[MAX_LIGHTS];\n"
                        "    uint num_lights;\n"
                        "};\n"
                        "\n"
                        "layout(std140) uniform br_model_state\n"
                        "{\n"
                        "    mat4 model_view;\n"
                        "    mat4 projection;\n"
                        "    mat4 mvp;\n"
                        "    mat4 normal_matrix;\n"
                        "    vec4 surface_colour;\n"
                        "\n"
                        "    vec4 ambient_colour; /* Material ambient colour. Is multiplied by surface_colour. */\n"
                        "    vec4 clear_colour;\n"
                        "    float ka; /* Ambient mod */\n"
                        "    float ks; /* Specular mod (doesn't seem to be used by Croc) */\n"
                        "    float kd; /* Diffuse mod */\n"
                        "    float power;\n"
                        "    uint unlit; /* Is this surface unlit? */\n"
                        "};\n"
                        "\n"
                        "in vec4 position;\n"
                        "in vec2 uv;\n"
                        "in vec4 normal;\n"
                        "in vec4 colour;\n"
                        "\n"
                        "out vec4 mainColour;\n"
                        "\n"
                        "uniform sampler2D main_texture;\n"
                        "\n"
                        "vec3 adjustBrightness(in vec3 colour, in float brightness)\n"
                        "{\n"
                        "    return colour + brightness;\n"
                        "}\n"
                        "\n"
                        "/* use values between -1 and 1 */\n"
                        "vec3 adjustContrast(in vec3 colour, in float contrast)\n"
                        "{\n"
                        "    return 0.5 + (1.0 + contrast) * (colour - 0.5);\n"
                        "}\n"
                        "\n"
                        "/* use values between -1 and 1 */\n"
                        "vec3 adjustExposure(in vec3 colour, in float exposure)\n"
                        "{\n"
                        "    // return (1.0 + exposure) * colour;\n"
                        "    return vec3(1.0) - exp(-colour * exposure);\n"
                        "}\n"
                        "\n"
                        "vec3 adjustSaturation(in vec3 colour, in float saturation)\n"
                        "{\n"
                        "    /* https://www.w3.org/TR/WCAG21/#dfn-relative-luminance */\n"
                        "    const vec3 luminosityFactor = vec3(0.2126, 0.7152, 0.0722);\n"
                        "    vec3 grayscale = vec3(dot(colour, luminosityFactor));\n"
                        "\n"
                        "    return mix(grayscale, colour, 1.0 + saturation);\n"
                        "}\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    vec4 texColour = texture(main_texture, uv);\n"
                        "\n"
                        "#if !DEBUG_DISABLE_COLOUR_KEY\n"
                        "    /* Discard black, used for sprite transparency */\n"
                        "    if (/*texColour.a < 0.01 || */ texColour.rgb == vec3(0.0, 0.0, 0.0))\n"
                        "        discard;\n"
                        "#endif\n"
                        "\n"
                        "    vec4 surfaceColour = surface_colour * texColour;\n"
                        "    vec3 fragColour = vec3(colour.rgb * texColour.rgb);\n"
                        "\n"
                        "    /* Perform gamma correction */\n"
                        "#if ENABLE_GAMMA_CORRECTION\n"
                        "    fragColour = pow(fragColour, vec3(1.0 / 1.2));\n"
                        "    // fragColour = adjustContrast(fragColour, 0.1);\n"
                        "    // fragColour = adjustExposure(fragColour, 2.0);\n"
                        "#endif\n"
                        "\n"
                        "#if ENABLE_SIMULATE_8BIT_COLOUR\n"
                        "    fragColour = floor(fragColour.rgb * vec3(15.0)) / vec3(15.0);\n"
                        "#endif\n"
                        "#if ENABLE_SIMULATE_16BIT_COLOUR\n"
                        "    float r = floor(fragColour.r * 31.0) / 31.0;\n"
                        "    float g = floor(fragColour.g * 63.0) / 63.0;\n"
                        "    float b = floor(fragColour.b * 31.0) / 31.0;\n"
                        "    fragColour = vec3(r, g, b);\n"
                        "#endif\n"
                        "\n"
                        "    /* The actual surface colour. */\n"
                        "    mainColour = vec4(fragColour, surfaceColour.a);\n"
                        "\n"
                        "    return;\n"
                        "}";


static void VIDEOI_GetShaderVariables(HVIDEO hVideo)
{
    glGenBuffers(1, &hVideo->brenderProgram.uboScene);
    glBindBuffer(GL_UNIFORM_BUFFER, hVideo->brenderProgram.uboScene);
    glUniformBlockBinding(hVideo->brenderProgram.program, hVideo->brenderProgram.blockIndexScene,
                          hVideo->brenderProgram.blockBindingScene);
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingScene, hVideo->brenderProgram.uboScene);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLSTD140_SCENE_DATA), NULL, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &hVideo->brenderProgram.uboModel);
    glBindBuffer(GL_UNIFORM_BUFFER, hVideo->brenderProgram.uboModel);
    glUniformBlockBinding(hVideo->brenderProgram.program, hVideo->brenderProgram.blockIndexModel,
                          hVideo->brenderProgram.blockBindingModel);
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingModel, hVideo->brenderProgram.uboModel);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLSTD140_MODEL_DATA), NULL, GL_DYNAMIC_DRAW);


    hVideo->brenderProgram.attributes.aPosition  = glGetAttribLocation(hVideo->brenderProgram.program, "aPosition");
    hVideo->brenderProgram.attributes.aUV        = glGetAttribLocation(hVideo->brenderProgram.program, "aUV");
    hVideo->brenderProgram.attributes.aNormal    = glGetAttribLocation(hVideo->brenderProgram.program, "aNormal");
    hVideo->brenderProgram.attributes.aColour    = glGetAttribLocation(hVideo->brenderProgram.program, "aColour");
    hVideo->brenderProgram.uniforms.main_texture = glGetUniformLocation(hVideo->brenderProgram.program, "main_texture");
    glBindFragDataLocation(hVideo->textProgram.program, 0, "mainColour");
}

br_boolean VIDEOI_CompileBRenderShader(HVIDEO hVideo, const char *vertPath, const char *fragPath)
{
    GLuint vert, frag;

    hVideo->brenderProgram.mainTextureBinding = 0;
    hVideo->brenderProgram.blockBindingScene  = 1;
    hVideo->brenderProgram.blockBindingModel  = 2;

    {
#define _MAX(a, b) ((a)>(b)?(a):(b))
        int neededSize = _MAX(sizeof(GLSTD140_SCENE_DATA), sizeof(GLSTD140_MODEL_DATA));
#undef _MAX
        if(hVideo->maxUniformBlockSize < neededSize) {
            BrLogError("VIDEO", "GL_MAX_UNIFORM_BLOCK_SIZE too small, got %d, needed %d.",
                       hVideo->maxUniformBlockSize, neededSize);
            return BR_FALSE;
        }
    }

    if(hVideo->maxUniformBufferBindings < 2) {
        BrLogError("VIDEO", "GL_MAX_UNIFORM_BUFFER_BINDINGS too small, got %d, needed 2.",
                   hVideo->maxUniformBufferBindings);
        return BR_FALSE;
    }

    /* br_model_state */
    if(hVideo->maxVertexUniformBlocks < 1) {
        BrLogError("VIDEO", "GL_MAX_VERTEX_UNIFORM_BLOCKS too small, got %d, needed 1.",
                   hVideo->maxVertexUniformBlocks);
        return BR_FALSE;
    }

    /* br_model_state, br_scene_state */
    if(hVideo->maxFragmentUniformBlocks < 2) {
        BrLogError("VIDEO", "GL_MAX_FRAGMENT_UNIFORM_BLOCKS too small, got %d, needed 2.",
                   hVideo->maxFragmentUniformBlocks);
        return BR_FALSE;
    }

    vert = VIDEOI_LoadAndCompileShader(GL_VERTEX_SHADER, vertPath, g_DefaultVertexShader,
                                       sizeof(g_DefaultVertexShader));
    if(!vert)
        return BR_FALSE;

    frag = VIDEOI_LoadAndCompileShader(GL_FRAGMENT_SHADER, fragPath, g_DefaultFragmentShader,
                                       sizeof(g_DefaultFragmentShader));
    if(!frag) {
        glDeleteShader(vert);
        return BR_FALSE;
    }

    if(!(hVideo->brenderProgram.program = VIDEOI_CreateAndCompileProgram(vert, frag))) {
        glDeleteShader(vert);
        glDeleteShader(frag);
        return BR_FALSE;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    if(hVideo->brenderProgram.program) {
        hVideo->brenderProgram.blockIndexScene = glGetUniformBlockIndex(hVideo->brenderProgram.program,
                                                                        "br_scene_state");
        if(hVideo->brenderProgram.blockIndexScene == GL_INVALID_INDEX) {
            BrLogError("VIDEO", "Unable to retrieve block index for uniform block 'br_scene_state'.");
            return BR_FALSE;
        }

        hVideo->brenderProgram.blockIndexModel = glGetUniformBlockIndex(hVideo->brenderProgram.program,
                                                                        "br_model_state");
        if(hVideo->brenderProgram.blockIndexModel == GL_INVALID_INDEX) {
            BrLogError("VIDEO", "Unable to retrieve block index for uniform block 'br_model_state'.");
            return BR_FALSE;
        }

        VIDEOI_GetShaderVariables(hVideo);
    }

    while(glGetError() != GL_NO_ERROR);

    return hVideo->brenderProgram.program != 0;
}
