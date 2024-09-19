#include "drv.h"

#include "default.vert.glsl.h"
#include "default.frag.glsl.h"

br_boolean VIDEOI_CompileDefaultShader(HVIDEO hVideo)
{
    GLuint vert = VIDEOI_CreateAndCompileShader(GL_VERTEX_SHADER, g_DefaultVertexShader, sizeof(g_DefaultVertexShader));
    if(!vert)
        return BR_FALSE;

    GLuint frag = VIDEOI_CreateAndCompileShader(GL_FRAGMENT_SHADER, g_DefaultFragmentShader, sizeof(g_DefaultFragmentShader));
    if(!frag) {
        glDeleteShader(vert);
        return BR_FALSE;
    }

    hVideo->defaultProgram.program = VIDEOI_CreateAndCompileProgram(vert, frag);

    DeviceGLObjectLabel(GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "default:shader:vertex");
    DeviceGLObjectLabel(GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "default:shader:fragment");
    DeviceGLObjectLabel(GL_PROGRAM, hVideo->defaultProgram.program, BR_GLREND_DEBUG_INTERNAL_PREFIX "default:program");

    glDeleteShader(vert);
    glDeleteShader(frag);

    if(hVideo->defaultProgram.program) {
        hVideo->defaultProgram.aPosition     = glGetAttribLocation(hVideo->defaultProgram.program, "aPosition");
        hVideo->defaultProgram.aColour       = glGetAttribLocation(hVideo->defaultProgram.program, "aColour");
        hVideo->defaultProgram.aUV           = glGetAttribLocation(hVideo->defaultProgram.program, "aUV");
        hVideo->defaultProgram.uSampler      = glGetUniformLocation(hVideo->defaultProgram.program, "uSampler");
        hVideo->defaultProgram.uIndexTex     = glGetUniformLocation(hVideo->defaultProgram.program, "uIndexTex");
        hVideo->defaultProgram.uMVP          = glGetUniformLocation(hVideo->defaultProgram.program, "uMVP");
        hVideo->defaultProgram.uVerticalFlip = glGetUniformLocation(hVideo->defaultProgram.program, "uVerticalFlip");
        hVideo->defaultProgram.uIndexed      = glGetUniformLocation(hVideo->defaultProgram.program, "uIndexed");
        glBindFragDataLocation(hVideo->defaultProgram.program, 0, "mainColour");
    }

    DeviceGLCheckErrors();

    return hVideo->defaultProgram.program != 0;
}
