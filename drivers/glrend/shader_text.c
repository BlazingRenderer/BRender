#include "drv.h"

/*
 * Lightweight vertex/fragment shader for rendering text.
 */
#include "text.vert.glsl.h"
#include "text.frag.glsl.h"

br_boolean VIDEOI_CompileTextShader(HVIDEO hVideo)
{
    GLuint vert, frag;

    if((vert = VIDEOI_CreateAndCompileShader(GL_VERTEX_SHADER, g_TextVertexShader, sizeof(g_TextVertexShader))) == 0)
        goto vert_failed;

    if((frag = VIDEOI_CreateAndCompileShader(GL_FRAGMENT_SHADER, g_TextFragmentShader, sizeof(g_TextFragmentShader))) == 0)
        goto frag_failed;

    if((hVideo->textProgram.program = VIDEOI_CreateAndCompileProgram(vert, frag)) == 0)
        goto prog_failed;

    DeviceGLObjectLabel(GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:shader:vertex");
    DeviceGLObjectLabel(GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:shader:fragment");
    DeviceGLObjectLabel(GL_PROGRAM, hVideo->textProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:program");

    glGenVertexArrays(1, &hVideo->textProgram.vao_glyphs);
    glBindVertexArray(hVideo->textProgram.vao_glyphs);
    glBindVertexArray(0);

    DeviceGLObjectLabel(GL_VERTEX_ARRAY, hVideo->textProgram.vao_glyphs, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:vao");

    hVideo->textProgram.uSampler = glGetUniformLocation(hVideo->textProgram.program, "uSampler");

    hVideo->textProgram.block_index_font_data   = glGetUniformBlockIndex(hVideo->textProgram.program, "FontData");
    hVideo->textProgram.block_binding_font_data = 2;
    glUniformBlockBinding(hVideo->textProgram.program, hVideo->textProgram.block_index_font_data, hVideo->textProgram.block_binding_font_data);

    hVideo->textProgram.block_index_glyphs   = glGetUniformBlockIndex(hVideo->textProgram.program, "TextData");
    hVideo->textProgram.block_binding_glyphs = 3;
    glUniformBlockBinding(hVideo->textProgram.program, hVideo->textProgram.block_index_glyphs, hVideo->textProgram.block_binding_glyphs);

    glGenBuffers(1, &hVideo->textProgram.ubo_glyphs);
    glBindBuffer(GL_UNIFORM_BUFFER, hVideo->textProgram.ubo_glyphs);
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->textProgram.block_binding_glyphs, hVideo->textProgram.ubo_glyphs);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabel(GL_BUFFER, hVideo->textProgram.ubo_glyphs, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:ubo");

    glBindFragDataLocation(hVideo->textProgram.program, 0, "main_colour");

prog_failed:
    glDeleteShader(frag);

frag_failed:
    glDeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors();

    return hVideo->textProgram.program != 0;
}
