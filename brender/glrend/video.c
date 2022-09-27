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
    size_t size;
    GLuint shader;

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

static void VIDEOI_BuildWhiteTexture(GLuint tex)
{
    const static uint8_t whiteRGBA[] = {255, 255, 255, 255};
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteRGBA);
}

extern const uint8_t VIDEOI_RawCheckerboard64x64[];

static void VIDEOI_BuildCheckerboardTexture(GLuint tex)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, VIDEOI_RawCheckerboard64x64);
}

static void VIDEOI_SetupTextures(HVIDEO hVideo)
{
    /* Generate initial textures. */
    glGenTextures(sizeof(hVideo->texture.t) / sizeof(GLuint), hVideo->texture.t);

    /* Generate a 1x1 white texture. */
    VIDEOI_BuildWhiteTexture(hVideo->texture.white);

    /* Generate a magenta/black checkerboard texture. */
    VIDEOI_BuildCheckerboardTexture(hVideo->texture.checkerboard);

    /*
     * We can't use BRender's fonts directly, so build a POT texture with
     * glyph from left-to-right. All fonts have 256 possible characters.
     */

    hVideo->fonts.fixed3x5.glTex = hVideo->texture.fnt3x5;
    hVideo->fonts.fixed3x5.font  = BrFontFixed3x5;
    VIDEOI_BuildFontAtlas(hVideo, &hVideo->fonts.fixed3x5, BrFontFixed3x5, 128, 64);

    hVideo->fonts.prop4x6.glTex = hVideo->texture.fnt4x6;
    hVideo->fonts.prop4x6.font  = BrFontProp4x6;
    VIDEOI_BuildFontAtlas(hVideo, &hVideo->fonts.prop4x6, BrFontProp4x6, 128, 64);

    hVideo->fonts.prop7x9.glTex = hVideo->texture.fnt7x9;
    hVideo->fonts.prop7x9.font  = BrFontProp7x9;
    VIDEOI_BuildFontAtlas(hVideo, &hVideo->fonts.prop7x9, BrFontProp7x9, 256, 64);
}

HVIDEO VIDEO_Open(HVIDEO hVideo, const char *vertShader, const char *fragShader)
{
    if(hVideo == NULL) {
        BrLogError("VIDEO", "Invalid handle.");
        return NULL;
    }

    glGetIntegerv(GL_MAJOR_VERSION, &hVideo->majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &hVideo->minorVersion);

    hVideo->version  = (const char *)glGetString(GL_VERSION);
    hVideo->vendor   = (const char *)glGetString(GL_VENDOR);
    hVideo->renderer = (const char *)glGetString(GL_RENDERER);

    BrLogInfo("VIDEO", "OpenGL Version  = %s", hVideo->version);
    BrLogInfo("VIDEO", "OpenGL Vendor   = %s", hVideo->vendor);
    BrLogInfo("VIDEO", "OpenGL Renderer = %s", hVideo->renderer);

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

    VIDEOI_SetupTextures(hVideo);

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

    glDeleteTextures(sizeof(hVideo->texture.t) / sizeof(GLuint), hVideo->texture.t);

    if(hVideo->brenderProgram.blockIndexScene != GL_INVALID_INDEX)
        glDeleteBuffers(1, &hVideo->brenderProgram.uboScene);

    if(hVideo->brenderProgram.blockIndexModel != GL_INVALID_INDEX)
        glDeleteBuffers(0, &hVideo->brenderProgram.uboModel);

    glDeleteProgram(hVideo->defaultProgram.program);
    glDeleteProgram(hVideo->textProgram.program);
}

br_error VIDEOI_BrPixelmapGetTypeDetails(br_uint_8 pmType, GLint *internalFormat, GLenum *format, GLenum *type,
                                         GLsizeiptr *elemBytes)
{
    switch(pmType) {
        case BR_PMT_RGBA_8888_ARR:
            *internalFormat = GL_RGBA;
            *format         = GL_RGBA;
            *type           = GL_UNSIGNED_BYTE;
            *elemBytes      = 4;
            break;
        case BR_PMT_RGB_555:
            //assert(!"RGB_555");
            *internalFormat = GL_RGB;
            *format         = GL_RGBA;
            *type           = GL_UNSIGNED_SHORT_5_5_5_1;
            *elemBytes      = 2;
            break;
        case BR_PMT_RGB_565:
            *internalFormat = GL_RGB;
            *format         = GL_RGB;
            *type           = GL_UNSIGNED_SHORT_5_6_5;
            *elemBytes      = 2;
            break;
        case BR_PMT_RGB_888:
            *internalFormat = GL_RGB;
            *format         = GL_RGB;
            *type           = GL_UNSIGNED_BYTE;
            *elemBytes      = 3;
            break;
        case BR_PMT_RGBX_888:
            *internalFormat = GL_RGB;
            *format         = GL_RGBA;
            *type           = GL_UNSIGNED_INT_8_8_8_8;
            *elemBytes      = 3;
            break;
        case BR_PMT_RGBA_8888:
            *internalFormat = GL_RGBA;
            *format         = GL_RGBA;
            *type           = GL_UNSIGNED_INT_8_8_8_8;
            *elemBytes      = 4;
            break;
        case BR_PMT_ARGB_8888:
            *internalFormat = GL_RGBA;
            *format         = GL_BGRA;
            *type           = GL_UNSIGNED_INT_8_8_8_8_REV;
            *elemBytes      = 4;
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
            break;
        case BR_PMT_ARGB_4444:
            *internalFormat = GL_RGBA;
            *format         = GL_BGRA;
            *type           = GL_UNSIGNED_SHORT_4_4_4_4_REV;
            *elemBytes      = 2;
            break;
        case BR_PMT_RGB_332:
            *internalFormat = GL_RGB;
            *format         = GL_RGB;
            *type           = GL_UNSIGNED_BYTE_3_3_2;
            *elemBytes      = 1;
            break;
        case BR_PMT_DEPTH_8:
            *internalFormat = GL_DEPTH_COMPONENT;
            *format         = GL_DEPTH_COMPONENT;
            *type           = GL_UNSIGNED_BYTE;
            *elemBytes      = 1;
            break;
        case BR_PMT_DEPTH_16:
            *internalFormat = GL_DEPTH_COMPONENT;
            *format         = GL_DEPTH_COMPONENT;
            *type           = GL_UNSIGNED_SHORT;
            *elemBytes      = 2;
            break;
        default:
            return BRE_FAIL;
    }
    return BRE_OK;
}

void VIDEOI_BrPixelmapToExistingTexture(GLuint tex, br_pixelmap *pm)
{
    glBindTexture(GL_TEXTURE_2D, tex);

    GLint      internalFormat;
    GLenum     format;
    GLenum     type;
    GLsizeiptr elemBytes;
    VIDEOI_BrPixelmapGetTypeDetails(pm->type, &internalFormat, &format, &type, &elemBytes);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pm->width, pm->height, 0, format, type, pm->pixels);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint VIDEO_BrPixelmapToGLTexture(br_pixelmap *pm)
{
    if(pm == NULL)
        return 0;

    GLuint tex;
    glGenTextures(1, &tex);

    VIDEOI_BrPixelmapToExistingTexture(tex, pm);

    return tex;
}

void VIDEOI_BrRectToGL(br_pixelmap *pm, br_rectangle *r)
{
    /* Convert from the pixelmap's origin to (0, 0) at top-left. */
    r->x += pm->origin_x;
    r->y += pm->origin_y;

    /* Flip the rect upside down to use (0, 0) at bottom-left. */
    r->y = pm->height - r->h - r->y;
}

void VIDEOI_BrRectToUVs(br_pixelmap *pm, br_rectangle *r, float *x0, float *y0, float *x1, float *y1)
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
