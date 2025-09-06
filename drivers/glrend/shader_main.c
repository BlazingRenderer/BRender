#include "drv.h"

#include "brender.vert.glsl.h"
#include "brender.frag.glsl.h"

static void get_shader_variables(br_gl_main_shader *self, const GladGLContext *gl)
{
    gl->GenBuffers(1, &self->ubo_scene);
    gl->BindBuffer(GL_UNIFORM_BUFFER, self->ubo_scene);
    gl->UniformBlockBinding(self->program, self->block_index_scene, self->block_binding_scene);
    gl->BindBufferBase(GL_UNIFORM_BUFFER, self->block_binding_scene, self->ubo_scene);
    gl->BufferData(GL_UNIFORM_BUFFER, sizeof(br_gl_main_data_scene), NULL, GL_DYNAMIC_DRAW);

    DeviceGLObjectLabel(gl, GL_BUFFER, self->ubo_scene, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:ubo:scene");

    gl->UniformBlockBinding(self->program, self->block_index_model, self->block_binding_model);

    self->attributes.aPosition   = gl->GetAttribLocation(self->program, "aPosition");
    self->attributes.aUV         = gl->GetAttribLocation(self->program, "aUV");
    self->attributes.aNormal     = gl->GetAttribLocation(self->program, "aNormal");
    self->attributes.aColour     = gl->GetAttribLocation(self->program, "aColour");
    self->uniforms.main_texture  = gl->GetUniformLocation(self->program, "main_texture");
    self->uniforms.index_texture = gl->GetUniformLocation(self->program, "index_texture");
    gl->BindFragDataLocation(self->program, 0, "mainColour");
}

br_boolean ShaderGLMainCompile(br_gl_main_shader *self, const GladGLContext *gl, const char *vert_path, const char *frag_path)
{
    br_gl_context_state *ctx = GLContextState(gl);
    GLuint               vert, frag;

    self->main_texture_binding  = 0;
    self->index_texture_binding = 1;
    self->block_binding_scene   = 1;
    self->block_binding_model   = 2;

    {
#define _MAX(a, b) ((a) > (b) ? (a) : (b))
        int neededSize = _MAX(sizeof(br_gl_main_data_scene), sizeof(br_gl_main_data_model));
#undef _MAX
        if(ctx->limits.max_uniform_block_size < neededSize) {
            BrLogError("GLREND", "GL_MAX_UNIFORM_BLOCK_SIZE too small, got %d, needed %d.", ctx->limits.max_uniform_block_size, neededSize);
            return BR_FALSE;
        }
    }

    if(ctx->limits.max_uniform_buffer_bindings < 2) {
        BrLogError("GLREND", "GL_MAX_UNIFORM_BUFFER_BINDINGS too small, got %d, needed 2.", ctx->limits.max_uniform_buffer_bindings);
        return BR_FALSE;
    }

    /* br_model_state */
    if(ctx->limits.max_vertex_uniform_blocks < 1) {
        BrLogError("GLREND", "GL_MAX_VERTEX_UNIFORM_BLOCKS too small, got %d, needed 1.", ctx->limits.max_vertex_uniform_blocks);
        return BR_FALSE;
    }

    /* br_model_state, br_scene_state */
    if(ctx->limits.max_fragment_uniform_blocks < 2) {
        BrLogError("GLREND", "GL_MAX_FRAGMENT_UNIFORM_BLOCKS too small, got %d, needed 2.", ctx->limits.max_fragment_uniform_blocks);
        return BR_FALSE;
    }

    vert = DeviceGLLoadAndCompileShader(gl, GL_VERTEX_SHADER, vert_path, g_DefaultVertexShader, sizeof(g_DefaultVertexShader));
    if(!vert)
        goto vert_failed;

    frag = DeviceGLLoadAndCompileShader(gl, GL_FRAGMENT_SHADER, frag_path, g_DefaultFragmentShader, sizeof(g_DefaultFragmentShader));
    if(!frag)
        goto frag_failed;

    if(!((self->program = DeviceGLCreateAndCompileProgram(gl, vert, frag))))
        goto prog_failed;

    DeviceGLObjectLabel(gl, GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:vertex");
    DeviceGLObjectLabel(gl, GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:fragment");
    DeviceGLObjectLabel(gl, GL_PROGRAM, self->program, BR_GLREND_DEBUG_INTERNAL_PREFIX "brender:shader:program");

    self->block_index_scene = gl->GetUniformBlockIndex(self->program, "br_scene_state");
    self->block_index_model = gl->GetUniformBlockIndex(self->program, "br_model_state");

    get_shader_variables(self, gl);

prog_failed:
    gl->DeleteShader(frag);

frag_failed:
    gl->DeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors(gl);

    return self->program != 0;
}
