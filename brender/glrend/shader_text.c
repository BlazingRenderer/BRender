#include "drv.h"

/*
 * Lightweight vertex/fragment shader for rendering text.
 */
static const GLchar g_TextVertexShader[] = "#version 150\n"
                                           "in vec3 aPosition;\n"
                                           "in vec2 aUV;\n"
                                           "out vec2 uv;\n"
                                           "\n"
                                           "uniform mat4 uMVP;\n"
                                           "\n"
                                           "void main()\n"
                                           "{\n"
                                           "	gl_Position = uMVP * vec4(aPosition, 1.0);\n"
                                           "	uv = aUV;\n"
                                           "}\n";

static const GLchar g_TextFragmentShader[] = "#version 150\n"
                                             "\n"
                                             "in vec2 uv;\n"
                                             "uniform sampler2D uSampler;\n"
                                             "uniform vec3 uColour;\n"
                                             "\n"
                                             "out vec4 mainColour;\n"
                                             "\n"
                                             "void main()\n"
                                             "{\n"
                                             "	vec4 texColour = texture(uSampler, uv);\n"
                                             "	if(texColour.rgb == vec3(0.0, 0.0, 0.0))\n"
                                             "		discard;\n"
                                             "	mainColour = texColour * vec4(uColour.rgb, 1.0);\n"
                                             "}\n";

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

    glDeleteShader(vert);
    glDeleteShader(frag);

    if(hVideo->textProgram.program) {
        hVideo->textProgram.aPosition = glGetAttribLocation(hVideo->textProgram.program, "aPosition");
        hVideo->textProgram.aUV       = glGetAttribLocation(hVideo->textProgram.program, "aUV");
        hVideo->textProgram.uSampler  = glGetUniformLocation(hVideo->textProgram.program, "uSampler");
        hVideo->textProgram.uMVP      = glGetUniformLocation(hVideo->textProgram.program, "uMVP");
        hVideo->textProgram.uColour   = glGetUniformLocation(hVideo->textProgram.program, "uColour");
        glBindFragDataLocation(hVideo->textProgram.program, 0, "mainColour");
    }

    while(glGetError() != GL_NO_ERROR)
        ;

    return hVideo->textProgram.program != 0;
}
