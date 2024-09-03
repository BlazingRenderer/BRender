/*
 * VIDEO methods
 */
#include "drv.h"
#include "brassert.h"

GLuint VIDEOI_CreateAndCompileShader(GLenum type, const char *shader, size_t size)
{
    GLuint s;
    GLint  _size, status;

    ASSERT(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);

    s     = glCreateShader(type);
    _size = (GLint)size;
    glShaderSource(s, 1, &shader, &_size);
    glCompileShader(s);

    status = GL_FALSE;
    glGetShaderiv(s, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        char  errorBuffer[1024];
        GLint maxLength;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &maxLength);

        if(maxLength > sizeof(errorBuffer))
            maxLength = sizeof(errorBuffer);

        glGetShaderInfoLog(s, maxLength, &maxLength, errorBuffer);
        errorBuffer[maxLength - 1] = '\0';

        BrLogError("VIDEO", "Error compiling shader:\n%s", errorBuffer);
        glDeleteShader(s);
        return 0;
    }

    return s;
}

GLuint VIDEOI_LoadAndCompileShader(GLenum type, const char *path, const char *default_data, size_t default_size)
{
    GLchar *source;
    size_t  size;
    GLuint  shader;

    if(path == NULL || (source = BrFileLoad(NULL, path, &size)) == NULL) {
        source = (GLchar *)default_data;
        size   = default_size;
    }

    shader = VIDEOI_CreateAndCompileShader(type, source, size);

    if(source != default_data)
        BrResFree(source);

    return shader;
}

GLuint VIDEOI_CreateAndCompileProgram(GLuint vert, GLuint frag)
{
    GLuint program;
    GLint  status;

    if((program = glCreateProgram()) == 0) {
        BrLogError("VIDEO", "Error creating program.");
        return 0;
    }

    glAttachShader(program, vert);
    glAttachShader(program, frag);

    glLinkProgram(program);

    status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status != GL_TRUE) {
        char  errorBuffer[1024];
        GLint maxLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        if(maxLength > sizeof(errorBuffer))
            maxLength = sizeof(errorBuffer);

        glGetProgramInfoLog(program, maxLength, &maxLength, errorBuffer);
        errorBuffer[maxLength - 1] = '\0';
        BrLogError("VIDEO", "Error linking program:\n%s", errorBuffer);

        glDetachShader(program, vert);
        glDetachShader(program, frag);
        glDeleteProgram(program);
        program = 0;
    }

    return program;
}

HVIDEO VIDEO_Open(HVIDEO hVideo, const char *vertShader, const char *fragShader)
{
    if(hVideo == NULL) {
        BrLogError("VIDEO", "Invalid handle.");
        return NULL;
    }

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &hVideo->maxUniformBlockSize);
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &hVideo->maxUniformBufferBindings);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &hVideo->maxVertexUniformBlocks);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &hVideo->maxFragmentUniformBlocks);
    glGetIntegerv(GL_MAX_SAMPLES, &hVideo->maxSamples);

    if(GLAD_GL_ARB_texture_filter_anisotropic)
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &hVideo->maxAnisotropy);

    if(!VIDEOI_CompileDefaultShader(hVideo))
        return NULL;

    if(!VIDEOI_CompileTextShader(hVideo)) {
        glDeleteProgram(hVideo->defaultProgram.program);
        return NULL;
    }

    if(!VIDEOI_CompileBRenderShader(hVideo, vertShader, fragShader)) {
        glDeleteProgram(hVideo->textProgram.program);
        glDeleteProgram(hVideo->defaultProgram.program);
        return NULL;
    }

    return hVideo;
}

void VIDEO_Close(HVIDEO hVideo)
{
    if(!hVideo)
        return;

    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    if(hVideo->brenderProgram.blockIndexScene != GL_INVALID_INDEX)
        glDeleteBuffers(1, &hVideo->brenderProgram.uboScene);

    if(hVideo->brenderProgram.blockIndexModel != GL_INVALID_INDEX)
        glDeleteBuffers(0, &hVideo->brenderProgram.uboModel);

    glDeleteProgram(hVideo->defaultProgram.program);
    glDeleteProgram(hVideo->textProgram.program);
}

// clang-format off
#define BRPM_TO_GL(_pm_type, _internal_format, _format, _type, _bytes, _blended) \
    [_pm_type] = {                                                               \
        .pm_type         = _pm_type,                                             \
        .internal_format = _internal_format,                                     \
        .format          = _format,                                              \
        .type            = _type,                                                \
        .bytes           = _bytes,                                               \
        .blended         = _blended                                              \
    }

#if BR_ENDIAN_LITTLE
#   define GL_RGBN GL_BGR
#else
#   define GL_RGBN GL_RGB
#endif

const static br_pixelmap_gl_fmt br2gl[BR_PMT_MAX] = {
    BRPM_TO_GL(BR_PMT_RGB_332,       GL_RGB,  GL_RGB,  GL_UNSIGNED_BYTE_3_3_2,        1, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGB_555,       GL_RGB,  GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, BR_FALSE),
    BRPM_TO_GL(BR_PMT_BGR_555,       GL_RGB,  GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGB_565,       GL_RGB,  GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       2, BR_FALSE),
    /* Using GL_BGR and GL_UNSIGNED_SHORT_5_6_5 just gives black. */
    BRPM_TO_GL(BR_PMT_BGR_565,       GL_RGB,  GL_RGB,  GL_UNSIGNED_SHORT_5_6_5_REV,   2, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGBA_4444,     GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,     2, BR_TRUE),
    BRPM_TO_GL(BR_PMT_ARGB_4444,     GL_RGBA, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV, 2, BR_TRUE),
    BRPM_TO_GL(BR_PMT_RGB_888,       GL_RGB,  GL_RGBN, GL_UNSIGNED_BYTE,              3, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGBX_888,      GL_RGB,  GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,   3, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGBA_8888,     GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,   4, BR_TRUE),
    BRPM_TO_GL(BR_PMT_RGBA_8888_ARR, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,              4, BR_TRUE),

    BRPM_TO_GL(BR_PMT_DEPTH_16,   GL_DEPTH_COMPONENT16,  GL_DEPTH_COMPONENT, 0,        2, BR_FALSE), /* mandatory */
    BRPM_TO_GL(BR_PMT_DEPTH_24,   GL_DEPTH_COMPONENT24,  GL_DEPTH_COMPONENT, 0,        3, BR_FALSE), /* mandatory */
    BRPM_TO_GL(BR_PMT_DEPTH_32,   GL_DEPTH_COMPONENT32,  GL_DEPTH_COMPONENT, 0,        4, BR_FALSE),
    BRPM_TO_GL(BR_PMT_DEPTH_FP32, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, 4, BR_FALSE), /* mandatory */

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

br_error VIDEOI_BrPixelmapToExistingTexture(GLuint tex, br_pixelmap *pm)
{
    const br_pixelmap_gl_fmt *fmt;

    if((fmt = DeviceGLGetFormatDetails(pm->type)) == NULL)
        return BRE_FAIL;

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, fmt->internal_format, pm->width, pm->height, 0, fmt->format, fmt->type, pm->pixels);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    return BRE_OK;
}

GLuint VIDEO_BrPixelmapToGLTexture(br_pixelmap *pm)
{
    if(pm == NULL)
        return 0;

    GLuint tex;
    glGenTextures(1, &tex);

    if(VIDEOI_BrPixelmapToExistingTexture(tex, pm) != BRE_OK) {
        glDeleteTextures(1, &tex);
        return 0;
    }

    return tex;
}

void VIDEOI_BrRectToGL(const br_pixelmap *pm, br_rectangle *r)
{
    br_rectangle out;
    PixelmapRectangleClip(&out, r, pm);

    /* Flip the rect upside down to use (0, 0) at bottom-left. */
    *r   = out;
    r->y = pm->height - r->h - r->y;
}

void VIDEOI_BrRectToUVs(const br_pixelmap *pm, const br_rectangle *r, float *x0, float *y0, float *x1, float *y1)
{
    *x0 = (float)r->x / (float)pm->width;
    *y0 = (float)r->y / (float)pm->height;

    *x1 = (float)(r->x + r->w) / (float)pm->width;
    *y1 = (float)(r->y + r->h) / (float)pm->height;
}

br_matrix4 *VIDEOI_D3DtoGLProjection(br_matrix4 *m)
{
    // Change the signs
    // https://cv4mar.blogspot.com.au/2009/03/transformation-matrices-between-opengl.html
    m->m[0][2] = -m->m[0][2];
    m->m[1][2] = -m->m[1][2];
    m->m[2][2] = -m->m[2][2];
    m->m[3][2] = -m->m[3][2];
    return m;
}

const char *DeviceGLStrError(GLenum err)
{
    static char errbuf[64];
    const char *s;

    switch(err) {
        case 0:
            s = "GL_NO_ERROR";
            break;
        case GL_INVALID_ENUM:
            s = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            s = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            s = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            s = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            s = "GL_OUT_OF_MEMORY";
            break;
        default:
            s = NULL;
    }

    if(s != NULL)
        BrSprintfN(errbuf, sizeof(errbuf) - 1, "error %d (%s)", err, s);
    else
        BrSprintfN(errbuf, sizeof(errbuf) - 1, "error %d", err);

    errbuf[sizeof(errbuf) - 1] = '\0';
    return errbuf;
}

br_boolean DeviceGLCheckErrors(void)
{
    GLenum     err;
    br_boolean has_error = BR_FALSE;

    while((err = glGetError()) != GL_NO_ERROR) {
        has_error = BR_TRUE;
        BrLogTrace("GLREND", "OpenGL reported %s", DeviceGLStrError(err));
    }

    return has_error;
}
