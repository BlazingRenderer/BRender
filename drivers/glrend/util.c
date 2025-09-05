#include "drv.h"

GLuint DeviceGLBuildWhiteTexture(const GladGLContext *gl)
{
    const static uint8_t white_rgba[] = {255, 255, 255, 255};

    GLuint tex;

    gl->GenTextures(1, &tex);

    gl->BindTexture(GL_TEXTURE_2D, tex);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_rgba);

    DeviceGLObjectLabel(gl, GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "white");

    return tex;
}

GLuint DeviceGLBuildCheckerboardTexture(const GladGLContext *gl)
{
    // clang-format off
    const static br_uint_8 checkerboard_rgba[] = {
        0x00, 0x00, 0x00, 0xFF,   0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF,   0x00, 0x00, 0x00, 0xFF,
    };
    // clang-format on

    GLuint tex;

    gl->GenTextures(1, &tex);

    gl->BindTexture(GL_TEXTURE_2D, tex);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerboard_rgba);

    DeviceGLObjectLabel(gl, GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "checkerboard");

    return tex;
}

br_error DevicePixelmapGLBindFramebuffer(const GladGLContext *gl, GLenum target, br_device_pixelmap *pm)
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

    gl->BindFramebuffer(target, fbo);
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

br_error DeviceGLPixelmapToExistingGLTexture(const GladGLContext *gl, GLuint tex, br_pixelmap *pm)
{
    GLenum                    err;
    const br_pixelmap_gl_fmt *fmt;

    if(pm->flags & BR_PMF_NO_ACCESS || pm->pixels == NULL)
        return BRE_FAIL;

    if((fmt = DeviceGLGetFormatDetails(pm->type)) == NULL)
        return BRE_FAIL;

    gl->BindTexture(GL_TEXTURE_2D, tex);

    /*
     * If the texture isn't linear, or is inverted (i.e. pm->pixels points to the end),
     * we need to go via an intermediate.
     */
    if((pm->flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) != (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) || pm->row_bytes < 0) {
        br_pixelmap *tmp = BrPixelmapAllocate(pm->type, pm->width, pm->height, NULL, BR_PMAF_NORMAL);
        for(br_int_32 row = 0; row < pm->height; ++row) {
            BrPixelmapRectangleCopy(tmp,                                  /* dst    */
                                    -tmp->origin_x, -tmp->origin_y + row, /* dx, dy */
                                    pm,                                   /* src    */
                                    -pm->origin_x, -pm->origin_y + row,   /* sx, sy */
                                    pm->width, 1                          /* w, h   */
            );
            /*
             * To flip, set sy to:
             *   pm->height - (-pm->origin_y + row) - 1
             */
        }

        gl->TexImage2D(GL_TEXTURE_2D, 0, fmt->internal_format, pm->width, pm->height, 0, fmt->format, fmt->type, tmp->pixels);
        BrPixelmapFree(tmp);
    } else {
        gl->TexImage2D(GL_TEXTURE_2D, 0, fmt->internal_format, pm->width, pm->height, 0, fmt->format, fmt->type, pm->pixels);
    }

    if((err = gl->GetError()) != 0) {
        BrLogError("GLREND", "glTexImage2D() failed with %s", DeviceGLStrError(err));
        gl->BindTexture(GL_TEXTURE_2D, 0);
        return BRE_FAIL;
    }

    if(fmt->pm_type != BR_PMT_INDEX_8)
        gl->GenerateMipmap(GL_TEXTURE_2D);

    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, fmt->swizzle_r);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, fmt->swizzle_g);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, fmt->swizzle_b);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, fmt->swizzle_a);

    gl->BindTexture(GL_TEXTURE_2D, 0);

    return BRE_OK;
}

GLuint DeviceGLPixelmapToGLTexture(const GladGLContext *gl, br_pixelmap *pm)
{
    if(pm == NULL)
        return 0;

    GLuint tex;
    gl->GenTextures(1, &tex);

    if(DeviceGLPixelmapToExistingGLTexture(gl, tex, pm) != BRE_OK) {
        gl->DeleteTextures(1, &tex);
        return 0;
    }

    DeviceGLObjectLabelF(gl, GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "temp:%s", pm->identifier ? pm->identifier : "(unnamed)");
    return tex;
}

void DeviceGLObjectLabel(const GladGLContext *gl, GLenum identifier, GLuint name, const char *s)
{
    if(gl->KHR_debug == 0)
        return;

    while(gl->GetError() != GL_NO_ERROR)
        ;

    gl->ObjectLabel(identifier, name, -1, s);

    if(DeviceGLCheckErrors(gl)) {
        BrLogDebug("GLREND", "glObjectLabel(%u, %u, %s) failed.", identifier, name, s);
    }
}

void DeviceGLObjectLabelF(const GladGLContext *gl, GLenum identifier, GLuint name, const char *fmt, ...)
{
    va_list ap;
    char   *s   = BrScratchString();
    size_t  len = BrScratchStringSize();

    if(gl->KHR_debug == 0)
        return;

    va_start(ap, fmt);
    BrVSprintfN(s, len, fmt, ap);
    va_end(ap);

    s[len - 1] = '\0';

    DeviceGLObjectLabel(gl, identifier, name, s);
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

br_boolean DeviceGLCheckErrors(const GladGLContext *gl)
{
    GLenum     err;
    br_boolean has_error = BR_FALSE;

    while((err = gl->GetError()) != GL_NO_ERROR) {
        has_error = BR_TRUE;
        BrLogTrace("GLREND", "OpenGL reported %s", DeviceGLStrError(err));
    }

    return has_error;
}

br_vector3 DeviceGLTriangleCentroid(const br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3)
{
    br_vector3 v;
    v.v[0] = BR_DIV(v1->v[0] + v2->v[0] + v3->v[0], BR_SCALAR(3));
    v.v[1] = BR_DIV(v1->v[1] + v2->v[1] + v3->v[1], BR_SCALAR(3));
    v.v[2] = BR_DIV(v1->v[2] + v2->v[2] + v3->v[2], BR_SCALAR(3));
    return v;
}

br_clip_result DevicePixelmapGLRectangleClip(br_rectangle *restrict out, const br_rectangle *restrict r, const br_pixelmap *pm)
{
    br_clip_result rr;
    br_rectangle   tmp;

    if((rr = PixelmapRectangleClip(&tmp, r, pm)) == BR_CLIP_REJECT)
        return rr;

    /*
     * Flip the rect upside down to use (0, 0) at bottom-left.
     */
    tmp.y = pm->height - tmp.h - tmp.y;
    *out  = tmp;
    return rr;
}
