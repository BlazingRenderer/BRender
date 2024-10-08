#include "drv.h"

/*
 * Lightweight vertex/fragment shader for rendering text.
 */
#include "text.vert.glsl.h"
#include "text.frag.glsl.h"

br_boolean VIDEOI_CompileTextShader(HVIDEO hVideo)
{
    GLuint vert = VIDEOI_CreateAndCompileShader(GL_VERTEX_SHADER, g_TextVertexShader, sizeof(g_TextVertexShader));
    if(!vert)
        return BR_FALSE;

    GLuint frag = VIDEOI_CreateAndCompileShader(GL_FRAGMENT_SHADER, g_TextFragmentShader, sizeof(g_TextFragmentShader));
    if(!frag) {
        glDeleteShader(vert);
        return BR_FALSE;
    }

    hVideo->textProgram.program = VIDEOI_CreateAndCompileProgram(vert, frag);

    DeviceGLObjectLabel(GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:shader:vertex");
    DeviceGLObjectLabel(GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:shader:fragment");
    DeviceGLObjectLabel(GL_PROGRAM, hVideo->textProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:program");

    glDeleteShader(vert);
    glDeleteShader(frag);

    glGenVertexArrays(1, &hVideo->textProgram.vao_glyphs);
    DeviceGLObjectLabel(GL_VERTEX_ARRAY, hVideo->textProgram.vao_glyphs, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:vao");

    glGenBuffers(1, &hVideo->textProgram.ubo_glyphs);
    DeviceGLObjectLabel(GL_BUFFER, hVideo->textProgram.ubo_glyphs, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:ubo");

    if(hVideo->textProgram.program) {
        hVideo->textProgram.uSampler = glGetUniformLocation(hVideo->textProgram.program, "uSampler");

        hVideo->textProgram.block_index_font_data   = glGetUniformBlockIndex(hVideo->textProgram.program, "FontData");
        hVideo->textProgram.block_binding_font_data = 2;
        glUniformBlockBinding(hVideo->textProgram.program, hVideo->textProgram.block_index_font_data,
                              hVideo->textProgram.block_binding_font_data);

        hVideo->textProgram.block_index_glyphs   = glGetUniformBlockIndex(hVideo->textProgram.program, "TextData");
        hVideo->textProgram.block_binding_glyphs = 3;
        glUniformBlockBinding(hVideo->textProgram.program, hVideo->textProgram.block_index_glyphs,
                              hVideo->textProgram.block_binding_glyphs);

        glBindBuffer(GL_UNIFORM_BUFFER, hVideo->textProgram.ubo_glyphs);
        glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->textProgram.block_binding_glyphs, hVideo->textProgram.ubo_glyphs);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindFragDataLocation(hVideo->textProgram.program, 0, "main_colour");
    }

    DeviceGLCheckErrors();

    return hVideo->textProgram.program != 0;
}
