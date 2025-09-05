#include "drv.h"

#include "line.vert.glsl.h"
#include "line.frag.glsl.h"

br_boolean VIDEOI_CompileLineShader(HVIDEO hVideo)
{
    const GladGLContext *gl = hVideo->gl;

    GLuint vert, frag;

    if((vert = DeviceGLCreateAndCompileShader(gl, GL_VERTEX_SHADER, g_LineVertexShader, sizeof(g_LineVertexShader))) == 0)
        goto vert_failed;

    if((frag = DeviceGLCreateAndCompileShader(gl, GL_FRAGMENT_SHADER, g_LineFragmentShader, sizeof(g_LineFragmentShader))) == 0)
        goto frag_failed;

    if((hVideo->lineProgram.program = DeviceGLCreateAndCompileProgram(gl, vert, frag)) == 0)
        goto prog_failed;

    DeviceGLObjectLabel(gl, GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:shader:vertex");
    DeviceGLObjectLabel(gl, GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:shader:fragment");
    DeviceGLObjectLabel(gl, GL_PROGRAM, hVideo->lineProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:program");

    gl->GenVertexArrays(1, &hVideo->lineProgram.vao);
    gl->BindVertexArray(hVideo->lineProgram.vao);
    gl->BindVertexArray(0);

    DeviceGLObjectLabel(gl, GL_VERTEX_ARRAY, hVideo->lineProgram.vao, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:vao");

    hVideo->lineProgram.block_index_line_data   = gl->GetUniformBlockIndex(hVideo->lineProgram.program, "LineData");
    hVideo->lineProgram.block_binding_line_data = 0;
    gl->UniformBlockBinding(hVideo->lineProgram.program, hVideo->lineProgram.block_index_line_data, hVideo->lineProgram.block_binding_line_data);

    gl->GenBuffers(1, &hVideo->lineProgram.ubo);
    gl->BindBuffer(GL_UNIFORM_BUFFER, hVideo->lineProgram.ubo);
    gl->BindBufferBase(GL_UNIFORM_BUFFER, hVideo->lineProgram.block_binding_line_data, hVideo->lineProgram.ubo);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabel(gl, GL_BUFFER, hVideo->lineProgram.ubo, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:ubo");

    gl->BindFragDataLocation(hVideo->lineProgram.program, 0, "main_colour");

prog_failed:
    gl->DeleteShader(frag);

frag_failed:
    gl->DeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors(gl);

    return hVideo->lineProgram.program != 0;
}
