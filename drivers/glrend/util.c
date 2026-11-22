#include "drv.h"
#include "brassert.h"

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

static br_error DeviceGLUploadMapLevel0(const GladGLContext *gl, const br_pixelmap_gl_fmt *fmt, br_pixelmap *pm)
{
    br_pixelmap *tmp;

    /*
     * glTexImage2D() reads tight rows from the buffer's first byte, so a map that is padded,
     * or inverted (pm->pixels points at its last row), has to go via an intermediate.
     */
    if((pm->flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) && pm->row_bytes > 0) {
        gl->TexImage2D(GL_TEXTURE_2D, 0, fmt->internal_format, pm->width, pm->height, 0, fmt->format, fmt->type, pm->pixels);
        return BRE_OK;
    }

    if((tmp = BrPixelmapAllocate(pm->type, pm->width, pm->height, NULL, BR_PMAF_NORMAL)) == NULL)
        return BRE_FAIL;

    for(br_int_32 row = 0; row < pm->height; ++row) {
        BrPixelmapRectangleCopy(tmp,                                  /* dst    */
                                -tmp->origin_x, -tmp->origin_y + row, /* dx, dy */
                                pm,                                   /* src    */
                                -pm->origin_x, -pm->origin_y + row,   /* sx, sy */
                                pm->width, 1                          /* w, h   */
        );
    }

    gl->TexImage2D(GL_TEXTURE_2D, 0, fmt->internal_format, pm->width, pm->height, 0, fmt->format, fmt->type, tmp->pixels);
    BrPixelmapFree(tmp);

    return BRE_OK;
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

    if(fmt->pm_type == BR_PMT_INDEX_8) {
        /*
         * An index plane is fetched through the palette rather than filtered, so it keeps its
         * one level.
         */
        if(DeviceGLUploadMapLevel0(gl, fmt, pm) != BRE_OK) {
            gl->BindTexture(GL_TEXTURE_2D, 0);
            return BRE_FAIL;
        }

        if((err = gl->GetError()) != 0) {
            BrLogError("GLREND", "glTexImage2D() failed with %s", DeviceGLStrError(err));
            gl->BindTexture(GL_TEXTURE_2D, 0);
            return BRE_FAIL;
        }
    } else {
        /*
         * The map's own chain is used when it has one; otherwise one is built here.
         * glGenerateMipmap() would be cheaper, but its filter is implementation-defined, and the
         * drivers have to hold the same texels for a witness to be able to name one.
         */
        br_pixelmap *chain;

        if(pm->mip_offset != 0 && (pm->flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) &&
           pm->row_bytes > 0) {
            chain = pm;
        } else {
            if(pm->mip_offset != 0)
                BrLogInfo("GLREND", "\"%s\" carries a mip chain whose level 0 is not tightly packed and top-down; building one from the map instead.",
                          pm->identifier != NULL ? pm->identifier : "pixelmap");

            chain = BrPixelmapMakeMipMap(pm, pm->type, pm->map, 0, 256, BR_QUANTIZE_YIQ);
        }

        if(chain == NULL) {
            /*
             * The format table gives some types a GL format that the converter table cannot
             * read, so the builder refused. One level with a non-mipmap sampler beats no texture.
             */
            BrLogInfo("GLREND", "\"%s\" cannot take a mip chain - its type has no converter - so it is uploaded at one level.",
                      pm->identifier != NULL ? pm->identifier : "pixelmap");

            if(DeviceGLUploadMapLevel0(gl, fmt, pm) != BRE_OK) {
                gl->BindTexture(GL_TEXTURE_2D, 0);
                return BRE_FAIL;
            }

            if((err = gl->GetError()) != 0) {
                BrLogError("GLREND", "glTexImage2D() failed with %s", DeviceGLStrError(err));
                gl->BindTexture(GL_TEXTURE_2D, 0);
                return BRE_FAIL;
            }
        } else {
            br_uint_32 levels    = BrPixelmapMipLevelCount(chain);
            br_boolean bad_level = BR_FALSE;

            /* GL's default unpack alignment of 4 misreads a level whose row is not a multiple of 4. */
            gl->PixelStorei(GL_UNPACK_ALIGNMENT, 1);

            for(br_uint_32 level = 0; level < levels; ++level) {
                br_pixelmap_mip_level geometry;

                if(PixelmapMipLevelGeometry(chain, level, &geometry) != BRE_OK) {
                    BrLogError("GLREND", "could not reach mip level %u of \"%s\".", level,
                               chain->identifier != NULL ? chain->identifier : "pixelmap");
                    bad_level = BR_TRUE;
                    break;
                }

                gl->TexImage2D(GL_TEXTURE_2D, (GLint)level, fmt->internal_format, geometry.width, geometry.height, 0, fmt->format,
                               fmt->type, (br_uint_8 *)chain->pixels + geometry.offset);
            }

            gl->PixelStorei(GL_UNPACK_ALIGNMENT, 4);

            if(chain != pm)
                BrPixelmapFree(chain);

            if(bad_level) {
                gl->BindTexture(GL_TEXTURE_2D, 0);
                return BRE_FAIL;
            }

            if((err = gl->GetError()) != 0) {
                BrLogError("GLREND", "glTexImage2D() failed with %s", DeviceGLStrError(err));
                gl->BindTexture(GL_TEXTURE_2D, 0);
                return BRE_FAIL;
            }
        }
    }

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

static void GLAD_API_PTR DeviceGLObjectLabelNone(GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    (void)identifier;
    (void)name;
    (void)length;
    (void)label;
}

void DeviceGLObjectLabelDisable(GladGLContext *gl)
{
    gl->ObjectLabel = DeviceGLObjectLabelNone;
}

void DeviceGLObjectLabel(const GladGLContext *gl, GLenum identifier, GLuint name, const char *s)
{
    if(gl->KHR_debug == 0 || gl->ObjectLabel == DeviceGLObjectLabelNone)
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

    if(gl->KHR_debug == 0 || gl->ObjectLabel == DeviceGLObjectLabelNone)
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

br_vector3 DeviceGLTriangleCentroid(const br_vector3_f *v1, const br_vector3_f *v2, const br_vector3_f *v3)
{
    br_vector3 v;
    v.v[0] = BR_DIV(BR_SCALAR(v1->v[0] + v2->v[0] + v3->v[0]), BR_SCALAR(3));
    v.v[1] = BR_DIV(BR_SCALAR(v1->v[1] + v2->v[1] + v3->v[1]), BR_SCALAR(3));
    v.v[2] = BR_DIV(BR_SCALAR(v1->v[2] + v2->v[2] + v3->v[2]), BR_SCALAR(3));
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

GLuint DeviceGLCreateAndCompileShader(const GladGLContext *gl, GLenum type, const char *shader, size_t size)
{
    GLuint s;
    GLint  _size, status;

    ASSERT(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);

    s     = gl->CreateShader(type);
    _size = (GLint)size;
    gl->ShaderSource(s, 1, &shader, &_size);
    gl->CompileShader(s);

    status = GL_FALSE;
    gl->GetShaderiv(s, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        char  errorBuffer[1024];
        GLint maxLength;
        gl->GetShaderiv(s, GL_INFO_LOG_LENGTH, &maxLength);

        if(maxLength > sizeof(errorBuffer))
            maxLength = sizeof(errorBuffer);

        gl->GetShaderInfoLog(s, maxLength, &maxLength, errorBuffer);
        errorBuffer[maxLength - 1] = '\0';

        BrLogError("GLREND", "Error compiling shader:\n%s", errorBuffer);
        gl->DeleteShader(s);
        return 0;
    }

    return s;
}

GLuint DeviceGLLoadAndCompileShader(const GladGLContext *gl, GLenum type, const char *path, const char *default_data, size_t default_size)
{
    GLchar *source;
    size_t  size;
    GLuint  shader;

    if(path == NULL || (source = BrFileLoad(NULL, path, &size)) == NULL) {
        source = (GLchar *)default_data;
        size   = default_size;
    }

    shader = DeviceGLCreateAndCompileShader(gl, type, source, size);

    if(source != default_data)
        BrResFree(source);

    return shader;
}

GLuint DeviceGLCreateAndCompileProgram(const GladGLContext *gl, GLuint vert, GLuint frag)
{
    GLuint program;
    GLint  status;

    if((program = gl->CreateProgram()) == 0) {
        BrLogError("GLREND", "Error creating program.");
        return 0;
    }

    gl->AttachShader(program, vert);
    gl->AttachShader(program, frag);

    gl->LinkProgram(program);

    status = GL_FALSE;
    gl->GetProgramiv(program, GL_LINK_STATUS, &status);
    if(status != GL_TRUE) {
        char  errorBuffer[1024];
        GLint maxLength;
        gl->GetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        if(maxLength > sizeof(errorBuffer))
            maxLength = sizeof(errorBuffer);

        gl->GetProgramInfoLog(program, maxLength, &maxLength, errorBuffer);
        errorBuffer[maxLength - 1] = '\0';
        BrLogError("GLREND", "Error linking program:\n%s", errorBuffer);

        gl->DetachShader(program, vert);
        gl->DetachShader(program, frag);
        gl->DeleteProgram(program);
        program = 0;
    }

    return program;
}
