#include "drv.h"

#include "rect.vert.glsl.h"
#include "rect.frag.glsl.h"

br_boolean ShaderGLRectCompile(br_gl_rect_shader *self, const GladGLContext *gl)
{
    GLuint vert, frag;

    if((vert = DeviceGLCreateAndCompileShader(gl, GL_VERTEX_SHADER, g_RectVertexShader, sizeof(g_RectVertexShader))) == 0)
        goto vert_failed;

    if((frag = DeviceGLCreateAndCompileShader(gl, GL_FRAGMENT_SHADER, g_RectFragmentShader, sizeof(g_RectFragmentShader))) == 0)
        goto frag_failed;

    if((self->program = DeviceGLCreateAndCompileProgram(gl, vert, frag)) == 0)
        goto prog_failed;

    DeviceGLObjectLabel(gl, GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:shader:vertex");
    DeviceGLObjectLabel(gl, GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:shader:fragment");
    DeviceGLObjectLabel(gl, GL_PROGRAM, self->program, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:program");

    gl->GenVertexArrays(1, &self->vao);
    gl->BindVertexArray(self->vao);
    gl->BindVertexArray(0);

    DeviceGLObjectLabel(gl, GL_VERTEX_ARRAY, self->vao, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:vao");

    self->uSampler  = gl->GetUniformLocation(self->program, "uSampler");
    self->uIndexTex = gl->GetUniformLocation(self->program, "uIndexTex");

    self->block_index_rect_data   = gl->GetUniformBlockIndex(self->program, "RectData");
    self->block_binding_rect_data = 4;
    gl->UniformBlockBinding(self->program, self->block_index_rect_data, self->block_binding_rect_data);

    gl->GenBuffers(1, &self->ubo);
    gl->BindBuffer(GL_UNIFORM_BUFFER, self->ubo);
    gl->BindBufferBase(GL_UNIFORM_BUFFER, self->block_binding_rect_data, self->ubo);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabel(gl, GL_BUFFER, self->ubo, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:ubo");

    gl->BindFragDataLocation(self->program, 0, "main_colour");

prog_failed:
    gl->DeleteShader(frag);

frag_failed:
    gl->DeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors(gl);

    return self->program != 0;
}

void ShaderGLRectDrawCLUT(br_gl_rect_shader *self, const GladGLContext *gl, const br_gl_rect_data *data, GLuint tex, GLuint clut)
{
    gl->Disable(GL_DEPTH_TEST);
    gl->Disable(GL_CULL_FACE);
    gl->DepthMask(GL_FALSE);

    gl->UseProgram(self->program);

    if(clut != 0) {
        gl->ActiveTexture(GL_TEXTURE0);
        gl->BindTexture(GL_TEXTURE_2D, clut);
        gl->Uniform1i(self->uSampler, 0);

        gl->ActiveTexture(GL_TEXTURE1);
        gl->BindTexture(GL_TEXTURE_2D, tex);
        gl->Uniform1i(self->uIndexTex, 1);
    } else {
        gl->ActiveTexture(GL_TEXTURE0);
        gl->BindTexture(GL_TEXTURE_2D, tex);
        gl->Uniform1i(self->uSampler, 0);
    }

    gl->BindVertexArray(self->vao);
    gl->BindBuffer(GL_UNIFORM_BUFFER, self->ubo);
    gl->BufferData(GL_UNIFORM_BUFFER, sizeof(br_gl_rect_data), data, GL_STATIC_DRAW);
    gl->DrawArraysInstanced(GL_TRIANGLES, 0, 6, 2);
    gl->BindVertexArray(0);
}

void ShaderGLRectDraw(br_gl_rect_shader *self, const GladGLContext *gl, const br_gl_rect_data *rect_data, GLuint tex)
{
    return ShaderGLRectDrawCLUT(self, gl, rect_data, tex, 0);
}
