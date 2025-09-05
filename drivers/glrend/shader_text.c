#include "drv.h"

/*
 * Lightweight vertex/fragment shader for rendering text.
 */
#include "text.vert.glsl.h"
#include "text.frag.glsl.h"

br_boolean VIDEOI_CompileTextShader(HVIDEO hVideo)
{
    const GladGLContext *gl = hVideo->gl;
    GLuint               vert, frag;

    if((vert = DeviceGLCreateAndCompileShader(gl, GL_VERTEX_SHADER, g_TextVertexShader, sizeof(g_TextVertexShader))) == 0)
        goto vert_failed;

    if((frag = DeviceGLCreateAndCompileShader(gl, GL_FRAGMENT_SHADER, g_TextFragmentShader, sizeof(g_TextFragmentShader))) == 0)
        goto frag_failed;

    if((hVideo->textProgram.program = DeviceGLCreateAndCompileProgram(gl, vert, frag)) == 0)
        goto prog_failed;

    DeviceGLObjectLabel(gl, GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:shader:vertex");
    DeviceGLObjectLabel(gl, GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:shader:fragment");
    DeviceGLObjectLabel(gl, GL_PROGRAM, hVideo->textProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:program");

    gl->GenVertexArrays(1, &hVideo->textProgram.vao_glyphs);
    gl->BindVertexArray(hVideo->textProgram.vao_glyphs);
    gl->BindVertexArray(0);

    DeviceGLObjectLabel(gl, GL_VERTEX_ARRAY, hVideo->textProgram.vao_glyphs, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:vao");

    hVideo->textProgram.uSampler = gl->GetUniformLocation(hVideo->textProgram.program, "uSampler");

    hVideo->textProgram.block_index_font_data   = gl->GetUniformBlockIndex(hVideo->textProgram.program, "FontData");
    hVideo->textProgram.block_binding_font_data = 2;
    gl->UniformBlockBinding(hVideo->textProgram.program, hVideo->textProgram.block_index_font_data, hVideo->textProgram.block_binding_font_data);

    hVideo->textProgram.block_index_glyphs   = gl->GetUniformBlockIndex(hVideo->textProgram.program, "TextData");
    hVideo->textProgram.block_binding_glyphs = 3;
    gl->UniformBlockBinding(hVideo->textProgram.program, hVideo->textProgram.block_index_glyphs, hVideo->textProgram.block_binding_glyphs);

    gl->GenBuffers(1, &hVideo->textProgram.ubo_glyphs);
    gl->BindBuffer(GL_UNIFORM_BUFFER, hVideo->textProgram.ubo_glyphs);
    gl->BindBufferBase(GL_UNIFORM_BUFFER, hVideo->textProgram.block_binding_glyphs, hVideo->textProgram.ubo_glyphs);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabel(gl, GL_BUFFER, hVideo->textProgram.ubo_glyphs, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:ubo");

    gl->BindFragDataLocation(hVideo->textProgram.program, 0, "main_colour");

prog_failed:
    gl->DeleteShader(frag);

frag_failed:
    gl->DeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors(gl);

    return hVideo->textProgram.program != 0;
}
