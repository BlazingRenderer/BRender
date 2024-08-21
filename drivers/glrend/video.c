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

    if(GLAD_GL_EXT_texture_filter_anisotropic)
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &hVideo->maxAnisotropy);

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

br_error VIDEOI_BrPixelmapGetTypeDetails(br_uint_8 pmType, GLint *internalFormat, GLenum *format, GLenum *type,
                                         GLsizeiptr *elemBytes, br_boolean *blended)
{
    br_boolean is_blended = BR_FALSE;
    switch(pmType) {
        case BR_PMT_RGBA_8888_ARR:
            *internalFormat = GL_RGBA;
            *format         = GL_RGBA;
            *type           = GL_UNSIGNED_BYTE;
            *elemBytes      = 4;
            is_blended      = BR_TRUE;
            break;
        case BR_PMT_RGB_555:
            *internalFormat = GL_RGB;
            *format         = GL_BGRA;
            *type           = GL_UNSIGNED_SHORT_1_5_5_5_REV;
            *elemBytes      = 2;
            break;
        case BR_PMT_RGB_565:
            *internalFormat = GL_RGB;
            *format         = GL_RGB;
            *type           = GL_UNSIGNED_SHORT_5_6_5;
            *elemBytes      = 2;
            break;
        case BR_PMT_BGR_565:
            *internalFormat = GL_RGB;
            *format         = GL_RGB; /* Using GL_BGR and GL_UNSIGNED_SHORT_5_6_5 just gives black. */
            *type           = GL_UNSIGNED_SHORT_5_6_5_REV;
            *elemBytes      = 2;
            break;
        case BR_PMT_RGB_888:
            *internalFormat = GL_RGB;
#if BR_ENDIAN_LITTLE
            *format = GL_BGR;
#else
            *format = GL_RGB;
#endif
            *type      = GL_UNSIGNED_BYTE;
            *elemBytes = 3;
            break;
        case BR_PMT_RGBX_888:
            *internalFormat = GL_RGB;
            *format         = GL_BGRA;
            *type           = GL_UNSIGNED_INT_8_8_8_8_REV;
            *elemBytes      = 3;
            break;
        case BR_PMT_RGBA_8888:
            *internalFormat = GL_RGBA;
            *format         = GL_BGRA;
            *type           = GL_UNSIGNED_INT_8_8_8_8_REV;
            *elemBytes      = 4;
            is_blended      = BR_TRUE;
            break;
        case BR_PMT_BGR_555:
            *internalFormat = GL_RGB;
            *format         = GL_BGR;
            *type           = GL_UNSIGNED_SHORT_5_5_5_1;
            *elemBytes      = 2;
            break;
        case BR_PMT_RGBA_4444:
            *internalFormat = GL_RGBA;
            *format         = GL_RGBA;
            *type           = GL_UNSIGNED_SHORT_4_4_4_4;
            *elemBytes      = 2;
            is_blended      = BR_TRUE;
            break;
        case BR_PMT_ARGB_4444:
            *internalFormat = GL_RGBA;
            *format         = GL_BGRA;
            *type           = GL_UNSIGNED_SHORT_4_4_4_4_REV;
            *elemBytes      = 2;
            is_blended      = BR_TRUE;
            break;
        case BR_PMT_RGB_332:
            *internalFormat = GL_RGB;
            *format         = GL_RGB;
            *type           = GL_UNSIGNED_BYTE_3_3_2;
            *elemBytes      = 1;
            break;
        case BR_PMT_DEPTH_16:
            *internalFormat = GL_DEPTH_COMPONENT16;
            *format         = GL_DEPTH_COMPONENT;
            *type           = GL_UNSIGNED_SHORT;
            *elemBytes      = 2;
            break;
        case BR_PMT_DEPTH_24:
            *internalFormat = GL_DEPTH_COMPONENT24;
            *format         = GL_DEPTH_COMPONENT;
            *type           = GL_FLOAT;
            *elemBytes      = 3;
            break;
        case BR_PMT_DEPTH_32:
            *internalFormat = GL_DEPTH_COMPONENT32;
            *format         = GL_DEPTH_COMPONENT;
            *type           = GL_FLOAT;
            *elemBytes      = 4;
            break;
        case BR_PMT_DEPTH_FP32:
            *internalFormat = GL_DEPTH_COMPONENT32F;
            *format         = GL_DEPTH_COMPONENT;
            *type           = GL_FLOAT;
            *elemBytes      = 4;
            break;
        default:
            BrLogTrace("GLREND", "Unsupported BRender texture format %d.", pmType);
            return BRE_FAIL;
    }

    if(blended != NULL)
        *blended = is_blended;

    return BRE_OK;
}

br_error VIDEOI_BrPixelmapToExistingTexture(GLuint tex, br_pixelmap *pm)
{
    GLint      internalFormat;
    GLenum     format;
    GLenum     type;
    GLsizeiptr elemBytes;
    br_error   r;

    r = VIDEOI_BrPixelmapGetTypeDetails(pm->type, &internalFormat, &format, &type, &elemBytes, NULL);
    if(r != BRE_OK)
        return r;

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pm->width, pm->height, 0, format, type, pm->pixels);

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

    if(VIDEOI_BrPixelmapToExistingTexture(tex, pm) != BRE_OK)
        return 0;

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
