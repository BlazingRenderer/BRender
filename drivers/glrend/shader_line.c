#include "drv.h"

#include "line.vert.glsl.h"
#include "line.frag.glsl.h"

br_boolean VIDEOI_CompileLineShader(HVIDEO hVideo)
{
    GLuint vert, frag;

    if((vert = VIDEOI_CreateAndCompileShader(GL_VERTEX_SHADER, g_LineVertexShader, sizeof(g_LineVertexShader))) == 0)
        goto vert_failed;

    if((frag = VIDEOI_CreateAndCompileShader(GL_FRAGMENT_SHADER, g_LineFragmentShader, sizeof(g_LineFragmentShader))) == 0)
        goto frag_failed;

    if((hVideo->lineProgram.program = VIDEOI_CreateAndCompileProgram(vert, frag)) == 0)
        goto prog_failed;

    DeviceGLObjectLabel(GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:shader:vertex");
    DeviceGLObjectLabel(GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:shader:fragment");
    DeviceGLObjectLabel(GL_PROGRAM, hVideo->lineProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:program");

    glGenVertexArrays(1, &hVideo->lineProgram.vao);
    glBindVertexArray(hVideo->lineProgram.vao);
    glBindVertexArray(0);

    DeviceGLObjectLabel(GL_VERTEX_ARRAY, hVideo->lineProgram.vao, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:vao");

    hVideo->lineProgram.block_index_line_data   = glGetUniformBlockIndex(hVideo->lineProgram.program, "LineData");
    hVideo->lineProgram.block_binding_line_data = 0;
    glUniformBlockBinding(hVideo->lineProgram.program, hVideo->lineProgram.block_index_line_data, hVideo->lineProgram.block_binding_line_data);

    glGenBuffers(1, &hVideo->lineProgram.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, hVideo->lineProgram.ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->lineProgram.block_binding_line_data, hVideo->lineProgram.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabel(GL_BUFFER, hVideo->lineProgram.ubo, BR_GLREND_DEBUG_INTERNAL_PREFIX "line:ubo");

    glBindFragDataLocation(hVideo->lineProgram.program, 0, "main_colour");

prog_failed:
    glDeleteShader(frag);

frag_failed:
    glDeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors();

    return hVideo->lineProgram.program != 0;
}
