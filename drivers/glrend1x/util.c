#include "drv.h"
#include "brassert.h"

br_uint_8 DeviceGL1xTypeOrBits(br_uint_8 pixel_type, br_int_32 pixel_bits)
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

br_error DeviceGL1xPixelmapToExistingGLTextureConvert(const GladGLContext *gl, GLuint tex, br_pixelmap *pm, const br_pixelmap_convert_options *opts)
{
    GLenum                    err;
    const br_pixelmap_gl_fmt *fmt;
    br_pixelmap              *expanded = NULL;

    if(pm->flags & BR_PMF_NO_ACCESS || pm->pixels == NULL)
        return BRE_FAIL;

    if((fmt = DeviceGL1xGetFormatDetails(BR_PMT_RGBA_8888)) == NULL)
        return BRE_FAIL;

    /*
     * Because GL 1.x has no (nice) way to do colour keying, we need to do apply it to every texture
     * at upload-time, then use GL_ALPHA_TEST with alpha > 0.5 to cut it out.
     *
     * This also has the added bonus of making sure the pixelmap is linear, has whole pixels, and a positive stride,
     * meaning we don't have to potentially flip it later.
     */
    if((expanded = BrPixelmapConvert(pm, BR_PMT_RGBA_8888, opts)) == NULL)
        return BRE_FAIL;

    pm = expanded;

    gl->BindTexture(GL_TEXTURE_2D, tex);
    gl->TexImage2D(GL_TEXTURE_2D, 0, fmt->internal_format, pm->width, pm->height, 0, fmt->format, fmt->type, pm->pixels);

    if((err = gl->GetError()) != 0) {
        BrLogError("GLREND", "glTexImage2D() failed with %s", DeviceGL1xStrError(err));
        gl->BindTexture(GL_TEXTURE_2D, 0);
        BrPixelmapFree(expanded);
        return BRE_FAIL;
    }

    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    gl->BindTexture(GL_TEXTURE_2D, 0);

    BrPixelmapFree(expanded);

    return BRE_OK;
}

br_error DeviceGL1xPixelmapToExistingGLTexture(const GladGLContext *gl, GLuint tex, br_pixelmap *pm)
{
    GLenum                    err;
    const br_pixelmap_gl_fmt *fmt;

    if(pm->flags & BR_PMF_NO_ACCESS || pm->pixels == NULL)
        return BRE_FAIL;

    if((fmt = DeviceGL1xGetFormatDetails(pm->type)) == NULL)
        return BRE_FAIL;

    /*
     * If indexed, undo it via a convert.
     */
    if(fmt->indexed) {
        const br_pixelmap_convert_options opts = {
            .enable_colour_key = BR_FALSE,
        };
        return DeviceGL1xPixelmapToExistingGLTextureConvert(gl, tex, pm, &opts);
    }

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
        BrLogError("GLREND", "glTexImage2D() failed with %s", DeviceGL1xStrError(err));
        gl->BindTexture(GL_TEXTURE_2D, 0);
        return BRE_FAIL;
    }

    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    gl->BindTexture(GL_TEXTURE_2D, 0);

    return BRE_OK;
}

GLuint DeviceGL1xPixelmapToGLTexture(const GladGLContext *gl, br_pixelmap *pm)
{
    if(pm == NULL)
        return 0;

    GLuint tex;
    gl->GenTextures(1, &tex);

    if(DeviceGL1xPixelmapToExistingGLTexture(gl, tex, pm) != BRE_OK) {
        gl->DeleteTextures(1, &tex);
        return 0;
    }

    DeviceGL1xObjectLabelF(gl, GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "temp:%s", pm->identifier ? pm->identifier : "(unnamed)");
    return tex;
}

void DeviceGL1xObjectLabel(const GladGLContext *gl, GLenum identifier, GLuint name, const char *s)
{
    if(gl->KHR_debug == 0)
        return;

    while(gl->GetError() != GL_NO_ERROR)
        ;

    gl->ObjectLabel(identifier, name, -1, s);

    if(DeviceGL1xCheckErrors(gl)) {
        BrLogDebug("GLREND", "glObjectLabel(%u, %u, %s) failed.", identifier, name, s);
    }
}

void DeviceGL1xObjectLabelF(const GladGLContext *gl, GLenum identifier, GLuint name, const char *fmt, ...)
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

    DeviceGL1xObjectLabel(gl, identifier, name, s);
}

const char *DeviceGL1xStrError(GLenum err)
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
#if 0
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            s = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
#endif
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

br_boolean DeviceGL1xCheckErrors(const GladGLContext *gl)
{
    GLenum     err;
    br_boolean has_error = BR_FALSE;

    while((err = gl->GetError()) != GL_NO_ERROR) {
        has_error = BR_TRUE;
        BrLogTrace("GLREND", "OpenGL reported %s", DeviceGL1xStrError(err));
    }

    return has_error;
}
