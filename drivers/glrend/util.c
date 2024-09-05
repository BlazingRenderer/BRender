#include "drv.h"

GLuint DeviceGLBuildWhiteTexture(void)
{
    const static uint8_t white_rgba[] = {255, 255, 255, 255};

    GLuint tex;

    glGenTextures(1, &tex);

    DeviceGLObjectLabel(GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "white");

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_rgba);
    return tex;
}

GLuint DeviceGLBuildCheckerboardTexture(void)
{
    // clang-format off
    const static br_uint_8 checkerboard_rgba[] = {
        0x00, 0x00, 0x00, 0xFF,   0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF,   0x00, 0x00, 0x00, 0xFF,
    };
    // clang-format on

    GLuint tex;

    glGenTextures(1, &tex);

    DeviceGLObjectLabel(GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "checkerboard");

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerboard_rgba);

    return tex;
}

br_error DevicePixelmapGLBindFramebuffer(GLenum target, br_device_pixelmap *pm)
{
    GLuint fbo;

    if(pm == NULL)
        return BRE_FAIL;

    switch(pm->use_type) {
        case BRT_OFFSCREEN:
            fbo = pm->asBack.glFbo;
            break;
        case BRT_DEPTH:
            if(pm->asDepth.backbuffer == NULL)
                return BRE_FAIL;
            fbo = pm->asDepth.backbuffer->asBack.glFbo;
            break;
        default:
            return BRE_FAIL;
    }

    glBindFramebuffer(target, fbo);
    return BRE_OK;
}

br_uint_8 DeviceGLTypeOrBits(br_uint_8 pixel_type, br_int_32 pixel_bits)
{
    if(pixel_type != BR_PMT_MAX)
        return pixel_type;

    switch(pixel_bits) {
        case 16:
            return BR_PMT_RGB_565;
        case 24:
            return BR_PMT_RGB_888;
        case 32:
            return BR_PMT_RGBX_888;
        default:
            break;
    }

    return BR_PMT_MAX;
}

br_error DeviceGLPixelmapToExistingGLTexture(GLuint tex, br_pixelmap *pm)
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, fmt->swizzle_r);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, fmt->swizzle_g);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, fmt->swizzle_b);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, fmt->swizzle_a);

    glBindTexture(GL_TEXTURE_2D, 0);

    return BRE_OK;
}

GLuint DeviceGLPixelmapToGLTexture(br_pixelmap *pm)
{
    if(pm == NULL)
        return 0;

    GLuint tex;
    glGenTextures(1, &tex);

    if(DeviceGLPixelmapToExistingGLTexture(tex, pm) != BRE_OK) {
        glDeleteTextures(1, &tex);
        return 0;
    }

    DeviceGLObjectLabelF(GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "temp:%s",
                         pm->identifier ? pm->identifier : "(unnamed)");
    return tex;
}

void DeviceGLObjectLabel(GLenum identifier, GLuint name, const char *s)
{
    if(GLAD_GL_KHR_debug == 0)
        return;

    glObjectLabel(identifier, name, -1, s);
}

void DeviceGLObjectLabelF(GLenum identifier, GLuint name, const char *fmt, ...)
{
    va_list ap;
    char   *s   = BrScratchString();
    size_t  len = BrScratchStringSize();

    if(GLAD_GL_KHR_debug == 0)
        return;

    va_start(ap, fmt);
    BrVSprintfN(s, len, fmt, ap);
    va_end(ap);

    s[len - 1] = '\0';

    DeviceGLObjectLabel(identifier, name, s);
}
