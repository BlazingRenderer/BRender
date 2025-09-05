#include "drv.h"

#include "rect.vert.glsl.h"
#include "rect.frag.glsl.h"

br_boolean VIDEOI_CompileRectShader(HVIDEO hVideo)
{
    const GladGLContext *gl = hVideo->gl;
    GLuint               vert, frag;

    if((vert = VIDEOI_CreateAndCompileShader(gl, GL_VERTEX_SHADER, g_RectVertexShader, sizeof(g_RectVertexShader))) == 0)
        goto vert_failed;

    if((frag = VIDEOI_CreateAndCompileShader(gl, GL_FRAGMENT_SHADER, g_RectFragmentShader, sizeof(g_RectFragmentShader))) == 0)
        goto frag_failed;

    if((hVideo->rectProgram.program = VIDEOI_CreateAndCompileProgram(gl, vert, frag)) == 0)
        goto prog_failed;

    DeviceGLObjectLabel(gl, GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:shader:vertex");
    DeviceGLObjectLabel(gl, GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:shader:fragment");
    DeviceGLObjectLabel(gl, GL_PROGRAM, hVideo->rectProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:program");

    gl->GenVertexArrays(1, &hVideo->rectProgram.vao);
    gl->BindVertexArray(hVideo->rectProgram.vao);
    gl->BindVertexArray(0);

    DeviceGLObjectLabel(gl, GL_VERTEX_ARRAY, hVideo->rectProgram.vao, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:vao");

    hVideo->rectProgram.uSampler  = gl->GetUniformLocation(hVideo->rectProgram.program, "uSampler");
    hVideo->rectProgram.uIndexTex = gl->GetUniformLocation(hVideo->rectProgram.program, "uIndexTex");

    hVideo->rectProgram.block_index_rect_data   = gl->GetUniformBlockIndex(hVideo->rectProgram.program, "RectData");
    hVideo->rectProgram.block_binding_rect_data = 4;
    gl->UniformBlockBinding(hVideo->rectProgram.program, hVideo->rectProgram.block_index_rect_data, hVideo->rectProgram.block_binding_rect_data);

    gl->GenBuffers(1, &hVideo->rectProgram.ubo);
    gl->BindBuffer(GL_UNIFORM_BUFFER, hVideo->rectProgram.ubo);
    gl->BindBufferBase(GL_UNIFORM_BUFFER, hVideo->rectProgram.block_binding_rect_data, hVideo->rectProgram.ubo);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabel(gl, GL_BUFFER, hVideo->rectProgram.ubo, BR_GLREND_DEBUG_INTERNAL_PREFIX "rect:ubo");

    gl->BindFragDataLocation(hVideo->rectProgram.program, 0, "main_colour");

prog_failed:
    gl->DeleteShader(frag);

frag_failed:
    gl->DeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors(gl);

    return hVideo->rectProgram.program != 0;
}
