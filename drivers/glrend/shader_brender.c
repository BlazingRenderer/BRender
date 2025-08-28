#include "drv.h"

#include "brender.vert.glsl.h"
#include "brender.frag.glsl.h"

static void VIDEOI_GetShaderVariables(HVIDEO hVideo)
{
    glGenBuffers(1, &hVideo->brenderProgram.uboScene);
    glBindBuffer(GL_UNIFORM_BUFFER, hVideo->brenderProgram.uboScene);
    glUniformBlockBinding(hVideo->brenderProgram.program, hVideo->brenderProgram.blockIndexScene, hVideo->brenderProgram.blockBindingScene);
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingScene, hVideo->brenderProgram.uboScene);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(shader_data_scene), NULL, GL_DYNAMIC_DRAW);

    DeviceGLObjectLabel(GL_BUFFER, hVideo->brenderProgram.uboScene, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:ubo:scene");

    glUniformBlockBinding(hVideo->brenderProgram.program, hVideo->brenderProgram.blockIndexModel, hVideo->brenderProgram.blockBindingModel);

    hVideo->brenderProgram.attributes.aPosition   = glGetAttribLocation(hVideo->brenderProgram.program, "aPosition");
    hVideo->brenderProgram.attributes.aUV         = glGetAttribLocation(hVideo->brenderProgram.program, "aUV");
    hVideo->brenderProgram.attributes.aNormal     = glGetAttribLocation(hVideo->brenderProgram.program, "aNormal");
    hVideo->brenderProgram.attributes.aColour     = glGetAttribLocation(hVideo->brenderProgram.program, "aColour");
    hVideo->brenderProgram.uniforms.main_texture  = glGetUniformLocation(hVideo->brenderProgram.program, "main_texture");
    hVideo->brenderProgram.uniforms.index_texture = glGetUniformLocation(hVideo->brenderProgram.program, "index_texture");
    // glBindFragDataLocation(hVideo->textProgram.program, 0, "mainColour");
}

br_boolean VIDEOI_CompileBRenderShader(HVIDEO hVideo, const char *vertPath, const char *fragPath)
{
    GLuint vert, frag;

    hVideo->brenderProgram.mainTextureBinding  = 0;
    hVideo->brenderProgram.indexTextureBinding = 1;
    hVideo->brenderProgram.blockBindingScene   = 1;
    hVideo->brenderProgram.blockBindingModel   = 2;

    {
#define _MAX(a, b) ((a) > (b) ? (a) : (b))
        int neededSize = _MAX(sizeof(shader_data_scene), sizeof(shader_data_model));
#undef _MAX
        if(hVideo->maxUniformBlockSize < neededSize) {
            BrLogError("VIDEO", "GL_MAX_UNIFORM_BLOCK_SIZE too small, got %d, needed %d.", hVideo->maxUniformBlockSize, neededSize);
            return BR_FALSE;
        }
    }

    if(hVideo->maxUniformBufferBindings < 2) {
        BrLogError("VIDEO", "GL_MAX_UNIFORM_BUFFER_BINDINGS too small, got %d, needed 2.", hVideo->maxUniformBufferBindings);
        return BR_FALSE;
    }

    /* br_model_state */
    if(hVideo->maxVertexUniformBlocks < 1) {
        BrLogError("VIDEO", "GL_MAX_VERTEX_UNIFORM_BLOCKS too small, got %d, needed 1.", hVideo->maxVertexUniformBlocks);
        return BR_FALSE;
    }

    /* br_model_state, br_scene_state */
    if(hVideo->maxFragmentUniformBlocks < 2) {
        BrLogError("VIDEO", "GL_MAX_FRAGMENT_UNIFORM_BLOCKS too small, got %d, needed 2.", hVideo->maxFragmentUniformBlocks);
        return BR_FALSE;
    }

    vert = VIDEOI_LoadAndCompileShader(GL_VERTEX_SHADER, vertPath, g_DefaultVertexShader, sizeof(g_DefaultVertexShader));
    if(!vert)
        goto vert_failed;

    frag = VIDEOI_LoadAndCompileShader(GL_FRAGMENT_SHADER, fragPath, g_DefaultFragmentShader, sizeof(g_DefaultFragmentShader));
    if(!frag)
        goto frag_failed;

    if(!(hVideo->brenderProgram.program = VIDEOI_CreateAndCompileProgram(vert, frag)))
        goto prog_failed;

    DeviceGLObjectLabel(GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:vertex");
    DeviceGLObjectLabel(GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:fragment");
    DeviceGLObjectLabel(GL_PROGRAM, hVideo->brenderProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:program");

    hVideo->brenderProgram.blockIndexScene = glGetUniformBlockIndex(hVideo->brenderProgram.program, "br_scene_state");
    hVideo->brenderProgram.blockIndexModel = glGetUniformBlockIndex(hVideo->brenderProgram.program, "br_model_state");

    VIDEOI_GetShaderVariables(hVideo);

prog_failed:
    glDeleteShader(frag);

frag_failed:
    glDeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors();

    return hVideo->brenderProgram.program != 0;
}
