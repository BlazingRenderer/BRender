/*
 * VIDEO methods
 */
#include "drv.h"
#include "brassert.h"

HVIDEO VIDEO_Open(HVIDEO hVideo, const GladGLContext *gl, const char *vertShader, const char *fragShader)
{
    if(hVideo == NULL) {
        BrLogError("VIDEO", "Invalid handle.");
        return NULL;
    }

    BrMemSet(hVideo, 0, sizeof(VIDEO));

    hVideo->gl = gl;

    gl->GetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &hVideo->maxUniformBlockSize);
    gl->GetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &hVideo->maxUniformBufferBindings);
    gl->GetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &hVideo->maxVertexUniformBlocks);
    gl->GetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &hVideo->maxFragmentUniformBlocks);
    gl->GetIntegerv(GL_MAX_SAMPLES, &hVideo->maxSamples);

    if(gl->EXT_texture_filter_anisotropic)
        gl->GetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &hVideo->maxAnisotropy);

    if(!VIDEOI_CompileRectShader(hVideo))
        return NULL;

    if(!VIDEOI_CompileTextShader(hVideo)) {
        gl->DeleteProgram(hVideo->rectProgram.program);
        return NULL;
    }

    if(!VIDEOI_CompileLineShader(hVideo)) {
        gl->DeleteProgram(hVideo->textProgram.program);
        gl->DeleteProgram(hVideo->rectProgram.program);
        return NULL;
    }

    if(!VIDEOI_CompileBRenderShader(hVideo, vertShader, fragShader)) {
        gl->DeleteProgram(hVideo->textProgram.program);
        gl->DeleteProgram(hVideo->rectProgram.program);
        gl->DeleteProgram(hVideo->lineProgram.program);
        return NULL;
    }

    return hVideo;
}

void VIDEO_Close(HVIDEO hVideo)
{
    const GladGLContext *gl;

    if(!hVideo)
        return;

    gl = hVideo->gl;

    gl->UseProgram(0);
    gl->BindVertexArray(0);
    gl->BindBuffer(GL_ARRAY_BUFFER, 0);
    gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);

    if(hVideo->brenderProgram.blockIndexScene != GL_INVALID_INDEX)
        gl->DeleteBuffers(1, &hVideo->brenderProgram.uboScene);

    gl->DeleteProgram(hVideo->rectProgram.program);
    gl->DeleteBuffers(1, &hVideo->rectProgram.ubo);
    gl->DeleteVertexArrays(1, &hVideo->rectProgram.vao);

    gl->DeleteBuffers(1, &hVideo->lineProgram.ubo);
    gl->DeleteVertexArrays(1, &hVideo->lineProgram.vao);
    gl->DeleteProgram(hVideo->lineProgram.program);

    gl->DeleteProgram(hVideo->textProgram.program);
}
