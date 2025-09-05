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

// clang-format off
#define BRPM_TO_GL(_pm_type, _internal_format, _format, _type, _bytes, _blended, _indexed, swr, swg, swb, swa) \
    [_pm_type] = {                                                               \
        .pm_type         = _pm_type,                                             \
        .internal_format = _internal_format,                                     \
        .format          = _format,                                              \
        .type            = _type,                                                \
        .bytes           = _bytes,                                               \
        .blended         = _blended,                                             \
        .indexed         = _indexed,                                             \
        .swizzle_r       = GL_##swr,                                             \
        .swizzle_g       = GL_##swg,                                             \
        .swizzle_b       = GL_##swb,                                             \
        .swizzle_a       = GL_##swa,                                             \
    }

#if BR_ENDIAN_LITTLE
#   define GL_RGBN GL_BGR
#else
#   define GL_RGBN GL_RGB
#endif

const static br_pixelmap_gl_fmt br2gl[BR_PMT_MAX] = {
    BRPM_TO_GL(BR_PMT_INDEX_8,       GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE,              1, BR_FALSE, BR_TRUE,  RED, ZERO, ZERO,    ONE),
    BRPM_TO_GL(BR_PMT_RGB_332,       GL_RGB,  GL_RGB,         GL_UNSIGNED_BYTE_3_3_2,        1, BR_FALSE, BR_FALSE, RED, GREEN, BLUE,   ONE),
    BRPM_TO_GL(BR_PMT_RGB_555,       GL_RGB,  GL_RGBA,        GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, BR_FALSE, BR_FALSE, BLUE, GREEN, RED,   ONE),
    BRPM_TO_GL(BR_PMT_BGR_555,       GL_RGB,  GL_RGBA,        GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, BR_FALSE, BR_FALSE, RED, GREEN, BLUE,   ONE),
    BRPM_TO_GL(BR_PMT_RGB_565,       GL_RGB,  GL_RGB,         GL_UNSIGNED_SHORT_5_6_5,       2, BR_FALSE, BR_FALSE, RED, GREEN, BLUE,   ONE),
    BRPM_TO_GL(BR_PMT_BGR_565,       GL_RGB,  GL_RGB,         GL_UNSIGNED_SHORT_5_6_5,       2, BR_FALSE, BR_FALSE, BLUE, GREEN, RED,   ONE),
    BRPM_TO_GL(BR_PMT_RGBA_4444,     GL_RGBA, GL_RGBA,        GL_UNSIGNED_SHORT_4_4_4_4,     2, BR_TRUE,  BR_FALSE, RED, GREEN, BLUE, ALPHA),
    BRPM_TO_GL(BR_PMT_ARGB_4444,     GL_RGBA, GL_RGBA,        GL_UNSIGNED_SHORT_4_4_4_4,     2, BR_TRUE,  BR_FALSE, GREEN, BLUE, ALPHA, RED),
    BRPM_TO_GL(BR_PMT_RGB_888,       GL_RGB,  GL_RGBN,        GL_UNSIGNED_BYTE,              3, BR_FALSE, BR_FALSE, RED, GREEN, BLUE,   ONE),
    BRPM_TO_GL(BR_PMT_RGBX_888,      GL_RGB,  GL_RGBA,        GL_UNSIGNED_INT_8_8_8_8_REV,   3, BR_FALSE, BR_FALSE, BLUE, GREEN, RED,   ONE),
    BRPM_TO_GL(BR_PMT_RGBA_8888,     GL_RGBA, GL_RGBA,        GL_UNSIGNED_INT_8_8_8_8_REV,   4, BR_TRUE,  BR_FALSE, BLUE, GREEN, RED, ALPHA),
    BRPM_TO_GL(BR_PMT_RGBA_8888_ARR, GL_RGBA, GL_RGBA,        GL_UNSIGNED_BYTE,              4, BR_TRUE,  BR_FALSE, RED, GREEN, BLUE, ALPHA),
    BRPM_TO_GL(BR_PMT_R8G8B8A8,      GL_RGBA, GL_RGBA,        GL_UNSIGNED_INT_8_8_8_8,       4, BR_TRUE,  BR_FALSE, RED, GREEN, BLUE, ALPHA),
    BRPM_TO_GL(BR_PMT_DEPTH_16,      GL_DEPTH_COMPONENT16,    GL_DEPTH_COMPONENT, 0,         2, BR_FALSE, BR_FALSE, RED, GREEN, BLUE, ALPHA), /* mandatory */
    BRPM_TO_GL(BR_PMT_DEPTH_24,      GL_DEPTH_COMPONENT24,    GL_DEPTH_COMPONENT, 0,         3, BR_FALSE, BR_FALSE, RED, GREEN, BLUE, ALPHA), /* mandatory */
    BRPM_TO_GL(BR_PMT_DEPTH_32,      GL_DEPTH_COMPONENT32,    GL_DEPTH_COMPONENT, 0,         4, BR_FALSE, BR_FALSE, RED, GREEN, BLUE, ALPHA),
    BRPM_TO_GL(BR_PMT_DEPTH_FP32,    GL_DEPTH_COMPONENT32F,   GL_DEPTH_COMPONENT, GL_FLOAT,  4, BR_FALSE, BR_FALSE, RED, GREEN, BLUE, ALPHA), /* mandatory */

};
#undef GL_RGBN
#undef BRPM_TO_GL
// clang-format on

const br_pixelmap_gl_fmt *DeviceGLGetFormatDetails(br_uint_8 type)
{
    const br_pixelmap_gl_fmt *fmt;
    if(type >= BR_PMT_MAX)
        return NULL;

    fmt = br2gl + type;
    if(fmt->pm_type != type)
        return NULL;

    return fmt;
}
