#include "drv.h"

#include "line.vert.glsl.h"
#include "line.frag.glsl.h"
#include "line_int.frag.glsl.h"

br_boolean ShaderGLLineCompile(br_gl_line_shader *self, const GladGLContext *gl)
{
    GLuint vert, frag, frag_int;

    if((vert = DeviceGLCreateAndCompileShader(gl, GL_VERTEX_SHADER, g_LineVertexShader, sizeof(g_LineVertexShader))) == 0)
        goto vert_failed;

    if((frag = DeviceGLCreateAndCompileShader(gl, GL_FRAGMENT_SHADER, g_LineFragmentShader, sizeof(g_LineFragmentShader))) == 0)
        goto frag_failed;

    if((frag_int = DeviceGLCreateAndCompileShader(gl, GL_FRAGMENT_SHADER, g_LineIntFragmentShader, sizeof(g_LineIntFragmentShader))) == 0)
        goto frag_int_failed;

    if((self->program = DeviceGLCreateAndCompileProgram(gl, vert, frag)) == 0)
        goto prog_failed;

    /*
     * The same vertex shader feeds the integer-output program used to draw on an
     * indexed (GL_R8UI) destination; only the fragment stage differs.
     */
    if((self->program_int = DeviceGLCreateAndCompileProgram(gl, vert, frag_int)) == 0)
        goto prog_int_failed;

    DeviceGLObjectLabel(gl, GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:shader:vertex");
    DeviceGLObjectLabel(gl, GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:shader:fragment");
    DeviceGLObjectLabel(gl, GL_SHADER, frag_int, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:shader:fragment:indexed");
    DeviceGLObjectLabel(gl, GL_PROGRAM, self->program, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:program");
    DeviceGLObjectLabel(gl, GL_PROGRAM, self->program_int, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:program:indexed");

    gl->GenVertexArrays(1, &self->vao);
    gl->BindVertexArray(self->vao);
    gl->BindVertexArray(0);

    DeviceGLObjectLabel(gl, GL_VERTEX_ARRAY, self->vao, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:vao");

    self->block_index_line_data   = gl->GetUniformBlockIndex(self->program, "LineData");
    self->block_binding_line_data = 0;
    gl->UniformBlockBinding(self->program, self->block_index_line_data, self->block_binding_line_data);
    gl->UniformBlockBinding(self->program_int, gl->GetUniformBlockIndex(self->program_int, "LineData"), self->block_binding_line_data);

    gl->GenBuffers(1, &self->ubo);
    gl->BindBuffer(GL_UNIFORM_BUFFER, self->ubo);
    gl->BindBufferBase(GL_UNIFORM_BUFFER, self->block_binding_line_data, self->ubo);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabel(gl, GL_BUFFER, self->ubo, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:ubo");

    gl->BindFragDataLocation(self->program, 0, "main_colour");
    gl->BindFragDataLocation(self->program_int, 0, "main_colour");

    gl->DeleteShader(frag_int);
    gl->DeleteShader(frag);
    gl->DeleteShader(vert);

    DeviceGLCheckErrors(gl);
    return BR_TRUE;

prog_int_failed:
    gl->DeleteProgram(self->program);
    self->program = 0;

prog_failed:
    gl->DeleteShader(frag_int);

frag_int_failed:
    gl->DeleteShader(frag);

frag_failed:
    gl->DeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors(gl);

    return BR_FALSE;
}

static void ShaderGLLineBind(br_gl_line_shader *self, const GladGLContext *gl, GLuint program, const br_gl_line_data *data)
{
    gl->Disable(GL_DEPTH_TEST);
    gl->Disable(GL_CULL_FACE);
    gl->DepthMask(GL_FALSE);

    gl->UseProgram(program);

    gl->BindVertexArray(self->vao);
    gl->BindBuffer(GL_UNIFORM_BUFFER, self->ubo);
    gl->BufferData(GL_UNIFORM_BUFFER, sizeof(br_gl_line_data), data, GL_STATIC_DRAW);
}

void ShaderGLLineDraw(br_gl_line_shader *self, const GladGLContext *gl, const br_gl_line_data *data, GLenum mode, GLsizei count)
{
    gl->Enable(GL_BLEND);
    gl->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ShaderGLLineBind(self, gl, self->program, data);

    gl->DrawArrays(mode, 0, count);

    gl->BindVertexArray(0);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);
    gl->Disable(GL_BLEND);
}

void ShaderGLLineDrawIndexed(br_gl_line_shader *self, const GladGLContext *gl, const br_gl_line_data *data, GLenum mode, GLsizei count)
{
    /*
     * Blending and a float fragment output are both invalid against an integer
     * colour attachment, so the indexed variant neither enables blending nor
     * touches the blend state.
     */
    ShaderGLLineBind(self, gl, self->program_int, data);

    gl->DrawArrays(mode, 0, count);

    gl->BindVertexArray(0);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);
}
