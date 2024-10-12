#include "drv.h"

#include "rect.vert.glsl.h"
#include "rect.frag.glsl.h"

br_boolean VIDEOI_CompileRectShader(HVIDEO hVideo)
{
    GLuint vert, frag;

    if((vert = VIDEOI_CreateAndCompileShader(GL_VERTEX_SHADER, g_RectVertexShader, sizeof(g_RectVertexShader))) == 0)
        goto vert_failed;

    if((frag = VIDEOI_CreateAndCompileShader(GL_FRAGMENT_SHADER, g_RectFragmentShader, sizeof(g_RectFragmentShader))) == 0)
        goto frag_failed;

    if((hVideo->rectProgram.program = VIDEOI_CreateAndCompileProgram(vert, frag)) == 0)
        goto prog_failed;

    DeviceGLObjectLabel(GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:shader:vertex");
    DeviceGLObjectLabel(GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:shader:fragment");
    DeviceGLObjectLabel(GL_PROGRAM, hVideo->rectProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:program");

    glGenVertexArrays(1, &hVideo->rectProgram.vao);
    glBindVertexArray(hVideo->rectProgram.vao);
    glBindVertexArray(0);

    DeviceGLObjectLabel(GL_VERTEX_ARRAY, hVideo->rectProgram.vao, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:vao");

    hVideo->rectProgram.uSampler  = glGetUniformLocation(hVideo->rectProgram.program, "uSampler");
    hVideo->rectProgram.uIndexTex = glGetUniformLocation(hVideo->rectProgram.program, "uIndexTex");

    hVideo->rectProgram.block_index_rect_data   = glGetUniformBlockIndex(hVideo->rectProgram.program, "RectData");
    hVideo->rectProgram.block_binding_rect_data = 4;
    glUniformBlockBinding(hVideo->rectProgram.program, hVideo->rectProgram.block_index_rect_data,
                          hVideo->rectProgram.block_binding_rect_data);

    glGenBuffers(1, &hVideo->rectProgram.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, hVideo->rectProgram.ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->rectProgram.block_binding_rect_data, hVideo->rectProgram.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabel(GL_BUFFER, hVideo->rectProgram.ubo, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:ubo");

    glBindFragDataLocation(hVideo->rectProgram.program, 0, "main_colour");

prog_failed:
    glDeleteShader(frag);

frag_failed:
    glDeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors();

    return hVideo->rectProgram.program != 0;
}
