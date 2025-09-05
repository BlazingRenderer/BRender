#include "drv.h"

#include "brender.vert.glsl.h"
#include "brender.frag.glsl.h"

static void VIDEOI_GetShaderVariables(HVIDEO hVideo, const GladGLContext *gl)
{
    gl->GenBuffers(1, &hVideo->brenderProgram.uboScene);
    gl->BindBuffer(GL_UNIFORM_BUFFER, hVideo->brenderProgram.uboScene);
    gl->UniformBlockBinding(hVideo->brenderProgram.program, hVideo->brenderProgram.blockIndexScene, hVideo->brenderProgram.blockBindingScene);
    gl->BindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingScene, hVideo->brenderProgram.uboScene);
    gl->BufferData(GL_UNIFORM_BUFFER, sizeof(shader_data_scene), NULL, GL_DYNAMIC_DRAW);

    DeviceGLObjectLabel(gl, GL_BUFFER, hVideo->brenderProgram.uboScene, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:ubo:scene");

    gl->UniformBlockBinding(hVideo->brenderProgram.program, hVideo->brenderProgram.blockIndexModel, hVideo->brenderProgram.blockBindingModel);

    hVideo->brenderProgram.attributes.aPosition   = gl->GetAttribLocation(hVideo->brenderProgram.program, "aPosition");
    hVideo->brenderProgram.attributes.aUV         = gl->GetAttribLocation(hVideo->brenderProgram.program, "aUV");
    hVideo->brenderProgram.attributes.aNormal     = gl->GetAttribLocation(hVideo->brenderProgram.program, "aNormal");
    hVideo->brenderProgram.attributes.aColour     = gl->GetAttribLocation(hVideo->brenderProgram.program, "aColour");
    hVideo->brenderProgram.uniforms.main_texture  = gl->GetUniformLocation(hVideo->brenderProgram.program, "main_texture");
    hVideo->brenderProgram.uniforms.index_texture = gl->GetUniformLocation(hVideo->brenderProgram.program, "index_texture");
    gl->BindFragDataLocation(hVideo->textProgram.program, 0, "mainColour");
}

br_boolean VIDEOI_CompileBRenderShader(HVIDEO hVideo, const char *vertPath, const char *fragPath)
{
    const GladGLContext *gl  = hVideo->gl;
    br_gl_context_state *ctx = GLContextState(gl);
    GLuint               vert, frag;

    hVideo->brenderProgram.mainTextureBinding  = 0;
    hVideo->brenderProgram.indexTextureBinding = 1;
    hVideo->brenderProgram.blockBindingScene   = 1;
    hVideo->brenderProgram.blockBindingModel   = 2;

    {
#define _MAX(a, b) ((a) > (b) ? (a) : (b))
        int neededSize = _MAX(sizeof(shader_data_scene), sizeof(shader_data_model));
#undef _MAX
        if(ctx->limits.max_uniform_block_size < neededSize) {
            BrLogError("VIDEO", "GL_MAX_UNIFORM_BLOCK_SIZE too small, got %d, needed %d.", ctx->limits.max_uniform_block_size, neededSize);
            return BR_FALSE;
        }
    }

    if(ctx->limits.max_uniform_buffer_bindings < 2) {
        BrLogError("VIDEO", "GL_MAX_UNIFORM_BUFFER_BINDINGS too small, got %d, needed 2.", ctx->limits.max_uniform_buffer_bindings);
        return BR_FALSE;
    }

    /* br_model_state */
    if(ctx->limits.max_vertex_uniform_blocks < 1) {
        BrLogError("VIDEO", "GL_MAX_VERTEX_UNIFORM_BLOCKS too small, got %d, needed 1.", ctx->limits.max_vertex_uniform_blocks);
        return BR_FALSE;
    }

    /* br_model_state, br_scene_state */
    if(ctx->limits.max_fragment_uniform_blocks < 2) {
        BrLogError("VIDEO", "GL_MAX_FRAGMENT_UNIFORM_BLOCKS too small, got %d, needed 2.", ctx->limits.max_fragment_uniform_blocks);
        return BR_FALSE;
    }

    vert = DeviceGLLoadAndCompileShader(gl, GL_VERTEX_SHADER, vertPath, g_DefaultVertexShader, sizeof(g_DefaultVertexShader));
    if(!vert)
        goto vert_failed;

    frag = DeviceGLLoadAndCompileShader(gl, GL_FRAGMENT_SHADER, fragPath, g_DefaultFragmentShader, sizeof(g_DefaultFragmentShader));
    if(!frag)
        goto frag_failed;

    if(!((hVideo->brenderProgram.program = DeviceGLCreateAndCompileProgram(gl, vert, frag))))
        goto prog_failed;

    DeviceGLObjectLabel(gl, GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:vertex");
    DeviceGLObjectLabel(gl, GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:fragment");
    DeviceGLObjectLabel(gl, GL_PROGRAM, hVideo->brenderProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:program");

    hVideo->brenderProgram.blockIndexScene = gl->GetUniformBlockIndex(hVideo->brenderProgram.program, "br_scene_state");
    hVideo->brenderProgram.blockIndexModel = gl->GetUniformBlockIndex(hVideo->brenderProgram.program, "br_model_state");

    VIDEOI_GetShaderVariables(hVideo, gl);

prog_failed:
    gl->DeleteShader(frag);

frag_failed:
    gl->DeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors(gl);

    return hVideo->brenderProgram.program != 0;
}
