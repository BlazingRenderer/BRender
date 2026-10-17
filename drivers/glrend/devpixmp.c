/*
 * Device pixelmap methods
 *
 * TODO:
 * - Cleanup dangling pointers when a back/depth buffer is destroyed.
 */

#include <string.h>
#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_pixelmap_dispatch devicePixelmapDispatch;

static br_error custom_query(br_value *pvalue, void **extra, br_size_t *pextra_size, void *block, const struct br_tv_template_entry *tep,
                             void *context)
{
    const br_device_pixelmap *self = block;

    if(tep->token == BRT_OPENGL_TEXTURE_U32) {
        if(self->use_type == BRT_OFFSCREEN)
            pvalue->u32 = self->asBack.glTex;
        else if(self->use_type == BRT_DEPTH)
            pvalue->u32 = self->asDepth.glDepth;
        else
            pvalue->u32 = 0;

        return BRE_OK;
    } else if(tep->token == BRT_CLUT_O) {
        if(self->use_type == BRT_OFFSCREEN)
            pvalue->o = (br_object *)self->asBack.clut;
        else
            pvalue->o = NULL;

        return BRE_OK;
    }

    return BRE_UNKNOWN;
}

static const br_tv_custom custom = {
    .query      = custom_query,
    .set        = NULL,
    .extra_size = NULL,
};

/*
 * Device pixelmap info. template
 */
#define F(f) offsetof(struct br_device_pixelmap, f)
static br_tv_template_entry devicePixelmapTemplateEntries[] = {
    {BRT(WIDTH_I32),          F(pm_width),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(HEIGHT_I32),         F(pm_height),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(PIXEL_TYPE_U8),      F(pm_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8,  0                    },
    {BRT(OUTPUT_FACILITY_O),  F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(FACILITY_O),         F(output_facility), BRTV_QUERY,            BRTV_CONV_COPY,    0                    },
    {BRT(CLUT_O),             0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(IDENTIFIER_CSTR),    F(pm_identifier),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(MSAA_SAMPLES_I32),   F(msaa_samples),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(OPENGL_TEXTURE_U32), 0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
};
#undef F

/*
 * (Re)create the renderbuffers and attach them to the framebuffer.
 */
static br_error recreate_renderbuffers(br_device_pixelmap *self, const GladGLContext *gl)
{
    GLuint fbo           = 0;
    GLenum binding_point = self->msaa_samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    UASSERT(self->use_type == BRT_OFFSCREEN || self->use_type == BRT_DEPTH);

    if(self->use_type == BRT_OFFSCREEN) {
        const GLenum              draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
        const br_pixelmap_gl_fmt *fmt             = DeviceGLGetFormatDetails(self->pm_type);

        fbo = self->asBack.glFbo;
        UASSERT(fbo != 0);

        /* Delete */
        gl->DeleteTextures(1, &self->asBack.glTex);

        /* Create */
        gl->GenTextures(1, &self->asBack.glTex);
        gl->BindTexture(binding_point, self->asBack.glTex);

        if(self->msaa_samples) {
            gl->TexImage2DMultisample(binding_point, self->msaa_samples, fmt->internal_format, self->pm_width, self->pm_height, GL_TRUE);
        } else {
            /*
             * Both filters must be NEAREST: an indexed (GL_R8UI) texture is
             * non-filterable, and a LINEAR magnification filter would leave it
             * incomplete and sample as zero when used as a copy source.
             */
            gl->TexParameteri(binding_point, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            gl->TexParameteri(binding_point, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            gl->TexImage2D(binding_point, 0, fmt->internal_format, self->pm_width, self->pm_height, 0, fmt->format, fmt->type, NULL);
        }

        DeviceGLObjectLabelF(gl, GL_TEXTURE, self->asBack.glTex, "%s:colour", self->pm_identifier);

        gl->BindTexture(binding_point, 0);

        /* Attach */
        gl->BindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, binding_point, self->asBack.glTex, 0);
        gl->DrawBuffers(1, draw_buffers);

        /*
         * TexImage2D(..., NULL) leaves the new storage undefined, and a new or resized
         * offscreen is displayed before anything has filled it, so a driver that recycles
         * texture memory shows its old contents. Give the attachment a defined clear: an
         * indexed (GL_R8UI) target takes the integer clear - a float glClearColor() is not
         * defined to convert to an integer attachment - and a colour target is cleared
         * opaque, so an unused alpha reads as the X-means-opaque rule expects.
         */
        if(fmt->indexed) {
            const GLuint value[4] = {0, 0, 0, 0};

            gl->ClearBufferuiv(GL_COLOR, 0, value);
        } else {
            gl->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            gl->Clear(GL_COLOR_BUFFER_BIT);
        }
    } else if(self->use_type == BRT_DEPTH) {
        fbo = self->asDepth.backbuffer->asBack.glFbo;
        UASSERT(fbo != 0);

        /* Delete */
        gl->DeleteTextures(1, &self->asDepth.glDepth);

        /* Create */
        gl->GenTextures(1, &self->asDepth.glDepth);
        gl->BindTexture(binding_point, self->asDepth.glDepth);

        if(self->msaa_samples) {
            gl->TexImage2DMultisample(binding_point, self->msaa_samples, GL_DEPTH_COMPONENT, self->pm_width, self->pm_height, GL_TRUE);
        } else {
            gl->TexImage2D(binding_point, 0, GL_DEPTH_COMPONENT, self->pm_width, self->pm_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
        }

        DeviceGLObjectLabelF(gl, GL_TEXTURE, self->asDepth.glDepth, "%s:depth", self->pm_identifier);

        gl->BindTexture(binding_point, 0);

        /* Attach */
        gl->BindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, binding_point, self->asDepth.glDepth, 0);
    }

    DeviceGLObjectLabelF(gl, GL_FRAMEBUFFER, fbo, "%s:fbo", self->pm_identifier);

    if(gl->CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
        return BRE_FAIL;
    }
    gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
    return BRE_OK;
}

static void delete_gl_resources(br_device_pixelmap *self, const GladGLContext *gl)
{
    if(self->use_type == BRT_DEPTH) {
        // FIXME: We should be destroyed before our parent.
        // FIXME: If we haven't, should I bind the parent and detach?
        gl->DeleteTextures(1, &self->asDepth.glDepth);
    } else if(self->use_type == BRT_OFFSCREEN) {
        gl->DeleteFramebuffers(1, &self->asBack.glFbo);
        gl->DeleteTextures(1, &self->asBack.glTex);
    }
}

static void BR_CMETHOD_DECL(br_device_pixelmap_gl, free)(br_object *_self)
{
    br_device_pixelmap  *self = (br_device_pixelmap *)_self;
    const GladGLContext *gl   = DevicePixelmapGLGetGLContext(self);

    UASSERT(self->num_refs == 0);

    BrLogTrace("GLREND", "Freeing %s", self->pm_identifier);

    delete_gl_resources(self, gl);

    ObjectContainerRemove(self->output_facility, (br_object *)self);

    DevicePixelmapGLDecRef(self->screen);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_pixelmap_gl, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_gl, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_gl, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_gl, device)(br_object *self)
{
    (void)self;
    return ((br_device_pixelmap *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_gl, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device_pixelmap);
}

static br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_gl, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapTemplate == NULL)
        self->device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(self->device, devicePixelmapTemplateEntries,
                                                                              BR_ASIZE(devicePixelmapTemplateEntries));

    return self->device->templates.devicePixelmapTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    const GladGLContext *gl = DevicePixelmapGLGetGLContext(self);

    self->pm_width  = width;
    self->pm_height = height;
    return recreate_renderbuffers(self, gl);
}

/*
 * Structure used to unpack the 'match' tokens/values
 */
struct pixelmapMatchTokens {
    br_int_32 width;
    br_int_32 height;
    br_int_32 pixel_bits;
    br_uint_8 type;
    br_token  use_type;
    br_int_32 msaa_samples;
};

#define F(f) offsetof(struct pixelmapMatchTokens, f)
static br_tv_template_entry pixelmapMatchTemplateEntries[] = {
    {BRT_WIDTH_I32,        NULL, F(width),        BRTV_SET, BRTV_CONV_COPY},
    {BRT_HEIGHT_I32,       NULL, F(height),       BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_BITS_I32,   NULL, F(pixel_bits),   BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_TYPE_U8,    NULL, F(type),         BRTV_SET, BRTV_CONV_COPY},
    {BRT_USE_T,            NULL, F(use_type),     BRTV_SET, BRTV_CONV_COPY},
    {BRT_MSAA_SAMPLES_I32, NULL, F(msaa_samples), BRTV_SET, BRTV_CONV_COPY},
};
#undef F

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv)
{
    br_int_32                  count;
    br_error                   err;
    br_device_pixelmap        *pm;
    const char                *typestring;
    const br_pixelmap_gl_fmt  *fmt;
    const GladGLContext       *gl;
    br_gl_context_state       *ctx;
    struct pixelmapMatchTokens mt = {
        .width        = self->pm_width,
        .height       = self->pm_height,
        .pixel_bits   = -1,
        .type         = BR_PMT_MAX,
        .use_type     = BRT_NONE,
        .msaa_samples = 0,
    };

    gl  = DevicePixelmapGLGetGLContext(self);
    ctx = GLContextState(gl);

    if(self->device->templates.pixelmapMatchTemplate == NULL) {
        self->device->templates.pixelmapMatchTemplate = BrTVTemplateAllocate(self->device, pixelmapMatchTemplateEntries,
                                                                             BR_ASIZE(pixelmapMatchTemplateEntries));
    }

    err = BrTokenValueSetMany(&mt, &count, NULL, tv, self->device->templates.pixelmapMatchTemplate, self);
    if(err != BRE_OK)
        return err;

    if(mt.use_type == BRT_NO_RENDER)
        mt.use_type = BRT_OFFSCREEN;

    switch(mt.use_type) {
        case BRT_OFFSCREEN:
            typestring = "backbuffer";
            break;
        case BRT_DEPTH:
            typestring = "depth";

            /*
             * Depth buffers must be matched with the backbuffer.
             */
            if(self->use_type != BRT_OFFSCREEN)
                return BRE_UNSUPPORTED;

            /*
             * Can't have >1 depth buffer.
             */
            if(self->asBack.depthbuffer != NULL)
                return BRE_FAIL;

            switch(mt.pixel_bits) {
                case 16:
                    mt.type = BR_PMT_DEPTH_16;
                    break;
                case -1:
                case 32:
                    mt.type = BR_PMT_DEPTH_FP32;
                    break;
                default:
                    return BRE_UNSUPPORTED;
            }

            break;
        default:
            return BRE_UNSUPPORTED;
    }

    /*
     * Only allow backbuffers to be instantiated from the frontbuffer.
     */
    if(self->use_type == BRT_NONE && mt.use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if(mt.type == BR_PMT_MAX)
        mt.type = self->pm_type;

    if((fmt = DeviceGLGetFormatDetails(mt.type)) == NULL)
        return BRE_FAIL;

    /*
     * A multisampled integer texture need not be supported, and an indexed
     * device pixelmap is not rendered to through the colour pipeline anyway,
     * so never request one for an indexed format.
     */
    if(fmt->indexed)
        mt.msaa_samples = 0;

    if(mt.msaa_samples < 0)
        mt.msaa_samples = 0;
    else if(mt.msaa_samples > ctx->limits.max_samples)
        mt.msaa_samples = ctx->limits.max_samples;

    pm                  = BrResAllocate(self->device, sizeof(br_device_pixelmap), BR_MEMORY_OBJECT);
    pm->dispatch        = &devicePixelmapDispatch;
    pm->pm_identifier   = BrResSprintf(pm, BR_GLREND_DEBUG_USER_PREFIX "%s:%dx%d", typestring, mt.width, mt.height);
    pm->device          = self->device;
    pm->output_facility = self->output_facility;
    pm->use_type        = mt.use_type;
    pm->msaa_samples    = mt.msaa_samples;
    pm->screen          = self->screen;
    DevicePixelmapGLIncRef(self->screen);

    pm->pm_type     = mt.type;
    pm->pm_width    = mt.width;
    pm->pm_height   = mt.height;
    pm->pm_flags    = BR_PMF_NO_ACCESS;
    pm->pm_origin_x = 0;
    pm->pm_origin_y = 0;
    pm->pm_base_x   = 0;
    pm->pm_origin_y = 0;

    if(mt.use_type == BRT_OFFSCREEN) {
        pm->asBack.depthbuffer = NULL;
        pm->asBack.clut        = DeviceClutGLAllocate(pm, gl);
        gl->GenFramebuffers(1, &pm->asBack.glFbo);
    } else {
        UASSERT(mt.use_type == BRT_DEPTH);
        self->asBack.depthbuffer = pm;
        pm->asDepth.backbuffer   = self;
    }

    if(recreate_renderbuffers(pm, gl) != BRE_OK) {
        DevicePixelmapGLDecRef(self->screen);
        delete_gl_resources(pm, gl);
        BrResFreeNoCallback(pm);
        return BRE_FAIL;
    }

    /*
     * Copy origin over.
     */
    pm->pm_origin_x = self->pm_origin_x;
    pm->pm_origin_y = self->pm_origin_y;

    *newpm = pm;
    ObjectContainerAddFront(self->output_facility, (br_object *)pm);
    return BRE_OK;
}

/*
 * Draw a source texture region into this offscreen through the rect shader.
 *
 * This is the only path for an indexed (GL_R8UI) source into a colour
 * destination: a blit cannot convert between an integer and a colour
 * attachment, and the shader resolves each index through the palette.
 *
 * src_is_fbo distinguishes a source held in a framebuffer texture, which like
 * this destination is bottom-up, from a host upload, which is top-down; the two
 * need opposite vertical flips.
 */
static void DevicePixelmapGLRectDrawTexture(br_device_pixelmap *self, const GladGLContext *gl, br_gl_context_state *ctx,
                                            br_rectangle *drect, br_rectangle *srect, br_int_32 src_w, br_int_32 src_h, GLuint tex,
                                            GLuint clut, br_boolean indexed, br_boolean src_is_fbo)
{
    br_gl_rect_data rect_data = {
        .mvp           = {},
        .src_rect      = BR_VECTOR4((float)srect->x / (float)src_w, (float)srect->y / (float)src_h, (float)srect->w / (float)src_w,
                                    (float)srect->h / (float)src_h),
        .dst_rect      = BR_VECTOR4((float)drect->x / (float)self->pm_width, (float)drect->y / (float)self->pm_height,
                                    (float)drect->w / (float)self->pm_width, (float)drect->h / (float)self->pm_height),
        .vertical_flip = src_is_fbo ? 0.0f : 1.0f,
        .indexed       = indexed ? 1.0f : 0.0f,
    };

    BrMatrix4Orthographic(&rect_data.mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

    gl->BindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
    gl->Viewport(0, 0, self->pm_width, self->pm_height);

    if(indexed)
        ShaderGLRectDrawCLUT(&ctx->rect_shader, gl, &rect_data, tex, clut);
    else
        ShaderGLRectDraw(&ctx->rect_shader, gl, &rect_data, tex);

    gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
}

/*
 * Upload an indexed host source into an indexed (GL_R8UI) destination.
 *
 * An indexed destination holds CLUT indices, and a colour source cannot be
 * mapped back to them, so only an indexed source is served here. The rows are
 * staged top-down and written upside down because a framebuffer texture is
 * bottom-up, matching rectangleCopyFrom()'s readback flip. A nearest-neighbour
 * scale is applied when the rectangles differ in size.
 */
static br_error DevicePixelmapGLUploadIndexed(br_device_pixelmap *self, const GladGLContext *gl, br_rectangle *drect,
                                              br_device_pixelmap *src, br_rectangle *srect)
{
    const br_pixelmap *srcpm = (const br_pixelmap *)src;
    br_pixelmap       *tmp   = NULL;
    const br_uint_8   *src_pixels;
    br_int_32          src_stride;
    br_int_32          src_br_x, src_br_y;
    br_uint_8         *buffer;

    if(srcpm->pixels == NULL || (srcpm->flags & BR_PMF_NO_ACCESS))
        return BRE_UNSUPPORTED;

    /*
     * DevicePixelmapGLRectangleClip() flipped the source rectangle into GL
     * coordinates; convert its top row back to the top-down row host pixels
     * use.
     */
    src_br_x = srect->x;
    src_br_y = (br_int_32)srcpm->height - srect->h - srect->y;

    if((srcpm->flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)) == (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS) && srcpm->row_bytes > 0) {
        src_pixels = (const br_uint_8 *)srcpm->pixels;
        src_stride = srcpm->row_bytes;
    } else {
        /*
         * Non-linear or right-to-left storage is rare; stage a normal copy
         * rather than hand-rolling the row arithmetic for every case.
         */
        tmp = BrPixelmapAllocate(srcpm->type, srect->w, srect->h, NULL, BR_PMAF_NORMAL);
        if(tmp == NULL)
            return BRE_FAIL;

        BrPixelmapRectangleCopy(tmp, 0, 0, (br_pixelmap *)srcpm, src_br_x, src_br_y, srect->w, srect->h);

        src_pixels = (const br_uint_8 *)tmp->pixels;
        src_stride = tmp->row_bytes;
        src_br_x   = 0;
        src_br_y   = 0;
    }

    buffer = BrScratchAllocate((br_size_t)drect->w * (br_size_t)drect->h);

    for(br_int_32 j = 0; j < drect->h; ++j) {
        br_int_32 sy = src_br_y + (br_int_32)(((br_uint_32)(drect->h - 1 - j) * (br_uint_32)srect->h) / (br_uint_32)drect->h);

        for(br_int_32 i = 0; i < drect->w; ++i) {
            br_int_32 sx = src_br_x + (br_int_32)(((br_uint_32)i * (br_uint_32)srect->w) / (br_uint_32)drect->w);

            buffer[j * drect->w + i] = src_pixels[sy * src_stride + sx];
        }
    }

    if(tmp != NULL)
        BrPixelmapFree(tmp);

    gl->BindTexture(GL_TEXTURE_2D, self->asBack.glTex);
    gl->PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl->TexSubImage2D(GL_TEXTURE_2D, 0, drect->x, drect->y, drect->w, drect->h, GL_RED_INTEGER, GL_UNSIGNED_BYTE, buffer);
    gl->PixelStorei(GL_UNPACK_ALIGNMENT, 4);
    gl->BindTexture(GL_TEXTURE_2D, 0);

    BrScratchFree(buffer);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleStretchCopy)(br_device_pixelmap *self, br_rectangle *d,
                                                                             br_device_pixelmap *src, br_rectangle *s)
{
    const GladGLContext *gl = DevicePixelmapGLGetGLContext(self);

    /*
     * Device->Device non-addressable stretch copy.
     */
    GLbitfield   bits;
    GLuint       srcFbo, dstFbo;
    br_rectangle srect, drect;

    /*
     * Refusing copy/blit to screen.
     */
    if(self->use_type == BRT_NONE)
        return BRE_FAIL;

    if(DevicePixelmapGLRectangleClip(&srect, s, (const br_pixelmap *)src) == BR_CLIP_REJECT)
        return BRE_OK;

    if(DevicePixelmapGLRectangleClip(&drect, d, (const br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if(self->use_type == BRT_OFFSCREEN) {
        if(src->use_type != BRT_OFFSCREEN)
            return BRE_FAIL;
        dstFbo = self->asBack.glFbo;
        srcFbo = src->asBack.glFbo;
        bits   = GL_COLOR_BUFFER_BIT;
    } else if(self->use_type == BRT_DEPTH) {
        if(src->use_type != BRT_DEPTH)
            return BRE_FAIL;

        dstFbo = self->asDepth.backbuffer->asBack.glFbo;
        srcFbo = src->asDepth.backbuffer->asBack.glFbo;
        bits   = GL_DEPTH_BUFFER_BIT;
    } else {
        return BRE_FAIL;
    }

    /*
     * Ignore self-blit.
     */
    if(self == src)
        return BRE_OK;

    if(self->use_type == BRT_OFFSCREEN) {
        const br_pixelmap_gl_fmt *src_fmt = DeviceGLGetFormatDetails(src->pm_type);
        const br_pixelmap_gl_fmt *dst_fmt = DeviceGLGetFormatDetails(self->pm_type);

        if(src_fmt == NULL || dst_fmt == NULL)
            return BRE_FAIL;

        /*
         * An indexed source into a colour destination is resolved through its
         * palette by the rect shader, as a blit cannot convert an integer
         * attachment to a colour one. The reverse has no meaning - colours do
         * not map back to indices - and stays unsupported.
         */
        if(src_fmt->indexed != dst_fmt->indexed) {
            br_gl_context_state *ctx;
            GLuint               clut;

            if(!src_fmt->indexed)
                return BRE_UNSUPPORTED;

            ctx = GLContextState(gl);
            clut = src->asBack.clut->gl_tex ? src->asBack.clut->gl_tex : (self->asBack.clut->gl_tex ? self->asBack.clut->gl_tex : ctx->tex_white);

            DevicePixelmapGLRectDrawTexture(self, gl, ctx, &drect, &srect, src->pm_width, src->pm_height, src->asBack.glTex, clut, BR_TRUE, BR_TRUE);
            return BRE_OK;
        }
    }

    gl->BindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
    gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);

    gl->BlitFramebuffer(srect.x, srect.y, srect.x + srect.w, srect.y + srect.h, drect.x, drect.y, drect.x + drect.w, drect.y + drect.h,
                        bits, GL_NEAREST);

    gl->BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleCopy)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                      br_rectangle *sr)
{
    /* Device->Device non-addressable same-size copy. */
    br_rectangle r = {
        .x = p->x,
        .y = p->y,
        .w = src->pm_width,
        .h = src->pm_height,
    };

    return BR_CMETHOD(br_device_pixelmap_gl, rectangleStretchCopy)(self, &r, src, sr);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    GLuint               fbo;
    GLbitfield           mask;
    br_rectangle         drect;
    const GladGLContext *gl = DevicePixelmapGLGetGLContext(self);

    /*
     * Clip rectangle to pixelmap
     */
    if(DevicePixelmapGLRectangleClip(&drect, rect, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if(self->use_type == BRT_OFFSCREEN) {
        const br_pixelmap_gl_fmt *fmt = DeviceGLGetFormatDetails(self->pm_type);
        br_uint_8                 r8 = 0, g8 = 0, b8 = 0, a8 = 255;

        fbo  = self->asBack.glFbo;
        mask = GL_COLOR_BUFFER_BIT;

        /*
         * An indexed pixelmap is a GL_R8UI texture holding CLUT indices. A
         * float glClearColor() clear is not defined to convert to an integer
         * buffer, so the rectangle is cleared with the integer clear instead;
         * the index is the colour argument itself, as BrColourUnpack() refuses
         * the indexed types. glClearBufferuiv() honours the scissor test, so a
         * sub-rectangle is the same call with the scissor set.
         */
        if(fmt->indexed) {
            const GLuint value[4] = {colour & 0xffu, 0, 0, 0};

            gl->BindFramebuffer(GL_FRAMEBUFFER, fbo);
            gl->ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            gl->Enable(GL_SCISSOR_TEST);
            gl->Scissor(drect.x, drect.y, drect.w, drect.h);
            gl->ClearBufferuiv(GL_COLOR, 0, value);
            gl->Disable(GL_SCISSOR_TEST);
            gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
            return BRE_OK;
        }

        BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);

        gl->ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        gl->ClearColor((float)r8 / 255.0f, (float)g8 / 255.0f, (float)b8 / 255.0f, (float)a8 / 255.0f);
    } else if(self->use_type == BRT_DEPTH) {
        UASSERT(colour == 0xFFFFFFFF);
        fbo  = self->asDepth.backbuffer->asBack.glFbo;
        mask = GL_DEPTH_BUFFER_BIT;
        gl->DepthMask(GL_TRUE);
        gl->ClearDepth(1.0f);
    } else {
        return BRE_UNSUPPORTED;
    }

    gl->BindFramebuffer(GL_FRAMEBUFFER, fbo);
    gl->Viewport(0, 0, self->pm_width, self->pm_height);

    gl->Enable(GL_SCISSOR_TEST);
    gl->Scissor(drect.x, drect.y, drect.w, drect.h);

    gl->Clear(mask);

    gl->Disable(GL_SCISSOR_TEST);

    gl->BindFramebuffer(GL_FRAMEBUFFER, 0);

    return BRE_OK;
}

static br_error BR_CMETHOD(br_device_pixelmap_gl, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *d,
                                                                          br_device_pixelmap *_src, br_rectangle *s)
{
    /* Pixelmap->Device, addressable stretch copy. */

    const GladGLContext      *gl     = DevicePixelmapGLGetGLContext(self);
    br_gl_context_state      *ctx    = GLContextState(gl);
    br_pixelmap              *src    = (br_pixelmap *)_src;
    br_buffer_stored         *stored = src->stored;
    br_rectangle              srect, drect;
    br_boolean                tex_tmp = BR_FALSE;
    GLuint                    tex;
    br_boolean                clut_tmp = BR_FALSE;
    GLuint                    clut     = 0;
    const br_pixelmap_gl_fmt *fmt;

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if(DevicePixelmapGLRectangleClip(&srect, s, src) == BR_CLIP_REJECT)
        return BRE_OK;

    if(DevicePixelmapGLRectangleClip(&drect, d, (const br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    /*
     * An indexed destination holds CLUT indices and is a GL_R8UI integer
     * texture, which the rect shader cannot write (it outputs a colour) and a
     * colour source cannot be mapped back to. Only an indexed source is served,
     * by uploading its index bytes directly.
     */
    if(DeviceGLGetFormatDetails(self->pm_type)->indexed) {
        const br_pixelmap_gl_fmt *src_fmt = DeviceGLGetFormatDetails(src->type);

        if(src_fmt == NULL || !src_fmt->indexed)
            return BRE_UNSUPPORTED;

        return DevicePixelmapGLUploadIndexed(self, gl, &drect, _src, &srect);
    }

    if(stored != NULL && ObjectDevice(stored) == self->device) {
        tex = BufferStoredGLGetTexture(stored);
        fmt = stored->fmt;
    } else {
        tex     = DeviceGLPixelmapToGLTexture(gl, src);
        tex_tmp = BR_TRUE;
        fmt     = DeviceGLGetFormatDetails(src->type);
    }

    if(fmt == NULL) {
        return BRE_FAIL;
    }

    clut = self->asBack.clut->gl_tex ? self->asBack.clut->gl_tex : ctx->tex_white;
    if(fmt->indexed && src->map != NULL) {
        if(src->map->stored != NULL) {
            clut = BufferStoredGLGetCLUTTexture(stored, self, clut);
        } else {
            clut     = DeviceGLPixelmapToGLTexture(gl, src->map);
            clut_tmp = BR_TRUE;
        }
    }

    if(tex == 0) {
        if(clut != 0 && clut_tmp)
            gl->DeleteTextures(1, &clut);
        return BRE_FAIL;
    }

    DevicePixelmapGLRectDrawTexture(self, gl, ctx, &drect, &srect, src->width, src->height, tex, clut, fmt->indexed, BR_FALSE);

    if(tex_tmp)
        gl->DeleteTextures(1, &tex);

    if(clut_tmp)
        gl->DeleteTextures(1, &clut);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleCopyTo)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                        br_rectangle *sr)
{
    /* Pixelmap->Device, addressable same-size copy. */

    br_rectangle r = {
        .x = p->x,
        .y = p->y,
        .w = src->pm_width,
        .h = src->pm_height,
    };

    return BR_CMETHOD(br_device_pixelmap_gl, rectangleStretchCopyTo)(self, &r, src, sr);
}

#define DevicePixelmapMemAddress(pm, x, y, bpp)                                                                                                         \
    ((char *)(((br_device_pixelmap *)(pm))->pm_pixels) + (((br_device_pixelmap *)(pm))->pm_base_y + (y)) * ((br_device_pixelmap *)(pm))->pm_row_bytes + \
     (((br_device_pixelmap *)(pm))->pm_base_x + (x)) * (bpp))

/*
 * Device->Pixelmap, addressable same-size copy.
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *dest,
                                                                          br_rectangle *r)
{
    br_error             err;
    br_rectangle         srect;
    br_point             dpoint;
    br_uint_16           bytes_per_pixel;
    br_pixelmap         *readback  = NULL;
    br_pixelmap         *converted = NULL;
    const GladGLContext *gl        = DevicePixelmapGLGetGLContext(self);

    /*
     * Need contig pixels and whole rows.
     */
    if(!(dest->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)))
        return BRE_FAIL;

    if(PixelmapRectangleClipTwo(&srect, &dpoint, r, p, (br_pixelmap *)dest, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if((err = DevicePixelmapGLBindFramebuffer(gl, GL_READ_FRAMEBUFFER, self)) != BRE_OK)
        return err;

    /*
     * glReadPixels() does not apply the texture swizzle, so the destination's
     * upload format/type pair does not describe BRender memory order here. Read
     * the clipped region back in GL_RGBA/GL_UNSIGNED_BYTE - R,G,B,A bytes in
     * that order whatever the host endianness - into a temporary BR_PMT_RGBA_8888_ARR
     * pixelmap, then let BrPixelmapConvert() produce the destination type.
     * Readback is not a hot path, so the temporary allocations are acceptable.
     */
    if((readback = BrPixelmapAllocate(BR_PMT_RGBA_8888_ARR, srect.w, srect.h, NULL, BR_PMAF_NORMAL)) == NULL)
        return BRE_FAIL;

    gl->PixelStorei(GL_PACK_ROW_LENGTH, srect.w);
    gl->ReadPixels(srect.x, self->pm_height - srect.y - srect.h, srect.w, srect.h, GL_RGBA, GL_UNSIGNED_BYTE, readback->pixels);
    gl->PixelStorei(GL_PACK_ROW_LENGTH, 0);

    /*
     * GL reads bottom-up; flip the temporary into BRender's top-down row order.
     */
    size_t bytes_per_subrow = (size_t)srect.w * 4;
    void  *row_temp         = BrScratchAllocate(bytes_per_subrow);
    for(br_uint_16 j = 0; j < srect.h / 2; ++j) {
        void *top = (char *)readback->pixels + (size_t)j * readback->row_bytes;
        void *bot = (char *)readback->pixels + (size_t)(srect.h - j - 1) * readback->row_bytes;

        memcpy(row_temp, top, bytes_per_subrow);
        memcpy(top, bot, bytes_per_subrow);
        memcpy(bot, row_temp, bytes_per_subrow);
    }
    BrScratchFree(row_temp);

    if((converted = BrPixelmapConvert(readback, dest->pm_type, NULL)) == NULL) {
        BrLogError("GLREND", "Cannot convert the readback to destination pixel type %u.", (unsigned)dest->pm_type);
        BrPixelmapFree(readback);
        return BRE_FAIL;
    }

    bytes_per_pixel = BrPixelmapPixelSize((br_pixelmap *)dest) >> 3;
    for(br_uint_16 j = 0; j < srect.h; ++j) {
        void *dst = DevicePixelmapMemAddress(dest, dpoint.x, dpoint.y + j, bytes_per_pixel);
        void *src = DevicePixelmapMemAddress(converted, 0, j, bytes_per_pixel);

        memcpy(dst, src, (size_t)srect.w * bytes_per_pixel);
    }

    BrPixelmapFree(converted);
    BrPixelmapFree(readback);
    return BRE_OK;
}

static br_error BR_CMETHOD(br_device_pixelmap_gl, text)(br_device_pixelmap *self, br_point *point, br_font *font, const char *text, br_uint_32 colour)
{

    size_t                    len = strlen(text);
    br_point                  pp;
    const br_gl_text_font    *gl_font;
    const GladGLContext      *gl  = DevicePixelmapGLGetGLContext(self);
    br_gl_context_state      *ctx = GLContextState(gl);
    br_gl_text_data          *text_data;
    const br_pixelmap_gl_fmt *fmt = DeviceGLGetFormatDetails(self->pm_type);
    br_uint_8                 r8 = 0, g8 = 0, b8 = 0, a8 = 255;

    /*
     * Make sure we're an offscreen pixelmap.
     */
    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    /*
     * Quit if off top, bottom or right screen
     */
    if(PixelmapPointClip(&pp, point, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if(pp.y <= -font->glyph_y || pp.y >= self->pm_height || pp.x >= self->pm_width)
        return BRE_OK;

    /*
     * Ensure we're a valid font.
     */

    if(font == BrFontFixed3x5)
        gl_font = &ctx->font_fixed3x5;
    else if(font == BrFontProp4x6)
        gl_font = &ctx->font_prop4x6;
    else if(font == BrFontProp7x9)
        gl_font = &ctx->font_prop7x9;
    else
        return BRE_FAIL;

    /*
     * All valid, set up the render state.
     */

    /*
     * Create the per-model/text state.
     */
    text_data = BrScratchAllocate(sizeof(br_gl_text_data));
    BrMatrix4Orthographic(&text_data->mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

    /*
     * An indexed pixelmap is a GL_R8UI integer texture, so its glyphs are drawn
     * with the integer-output program and the colour argument is the CLUT index
     * itself; BrColourUnpack() refuses the indexed types.
     */
    if(fmt != NULL && fmt->indexed) {
        text_data->colour = (br_vector4)BR_VECTOR4((float)(colour & 0xff), 0.0f, 0.0f, 0.0f);
    } else {
        BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);
        text_data->colour = (br_vector4)BR_VECTOR4(r8 / 255.0f, g8 / 255.0f, b8 / 255.0f, a8 / 255.0f);
    }

    gl->BindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
    gl->Viewport(0, 0, self->pm_width, self->pm_height);

    if(fmt != NULL && fmt->indexed)
        ShaderGLTextBeginIndexed(&ctx->text_shader, gl, gl_font);
    else
        ShaderGLTextBegin(&ctx->text_shader, gl, gl_font);

    br_rectangle r = {
        .x = point->x,
        .y = point->y,
        .h = font->glyph_y,
        .w = 0,
    };

    do {
        size_t chunk = len;
        if(chunk > BR_GL_TEXT_CHUNK_SIZE) {
            chunk = BR_GL_TEXT_CHUNK_SIZE;
        }

        for(size_t i = 0; i < chunk; ++i) {
            br_uint_32   glyph = (br_uint_32)text[i];
            br_uint_16   width = (font->flags & BR_FONTF_PROPORTIONAL) ? font->width[glyph] : font->glyph_x;
            br_rectangle dr;

            r.w = width;

            /*
             * Bail early if the rest of the string is entirely offscreen.
             */
            dr = r;
            if(DevicePixelmapGLRectangleClip(&dr, &r, (br_pixelmap *)self) == BR_CLIP_REJECT) {
                chunk = i;
                len   = chunk;
                break;
            }

            // clang-format off
            text_data->rects[i] = (br_vector4_f)BR_VECTOR4(
                (float)dr.x / (float)self->pm_width,
                (float)dr.y / (float)self->pm_height,
                (float)dr.w / (float)self->pm_width,
                (float)dr.h / (float)self->pm_height
            );
            text_data->chars[i] = (br_uint_32)glyph;
            // clang-format on

            r.x += width + 1;
            r.w += width;
        }

        ShaderGLTextDrawInstanced(&ctx->text_shader, gl, text_data, (GLsizei)chunk);

        len -= chunk;
        text += chunk;
    } while(len > 0);

    BrScratchFree(text_data);

    ShaderGLTextEnd(&ctx->text_shader, gl);
    gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, line)(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
    br_point                  spoint, epoint;
    br_uint_8                 r8 = 0, g8 = 0, b8 = 0, a8 = 255;
    br_gl_line_data           line_data;
    const br_pixelmap_gl_fmt *fmt = DeviceGLGetFormatDetails(self->pm_type);
    const GladGLContext      *gl  = DevicePixelmapGLGetGLContext(self);
    br_gl_context_state      *ctx = GLContextState(gl);

    if(PixelmapLineClip(&spoint, &epoint, s, e, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    /*
     * An indexed pixelmap is a GL_R8UI integer texture. Its line is drawn with
     * the integer-output program and the colour is the CLUT index itself, as
     * BrColourUnpack() refuses the indexed types.
     */
    if(fmt != NULL && fmt->indexed) {
        line_data.colour = (br_vector4)BR_VECTOR4((float)(colour & 0xff), 0.0f, 0.0f, 0.0f);
    } else {
        BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);
        line_data.colour = (br_vector4)BR_VECTOR4(r8 / 255.0f, g8 / 255.0f, b8 / 255.0f, a8 / 255.0f);
    }

    /*
     * Place the endpoints on the pixel centre, (p + 0.5), like the vkrend line
     * shader and the CPU memory pixelmap: an integer pixel covers [p, p+1), and
     * its representative sample is the centre. Mapping the integer straight to
     * [0,1] puts the vertex on the pixel corner and shifts the whole line up and
     * left by one.
     */
    BrMatrix4Orthographic(&line_data.mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    line_data.start = (br_vector2)BR_VECTOR2(((float)spoint.x + 0.5f) / (float)self->pm_width,
                                             1.0f - (((float)spoint.y + 0.5f) / (float)self->pm_height));
    line_data.end   = (br_vector2)BR_VECTOR2(((float)epoint.x + 0.5f) / (float)self->pm_width,
                                             1.0f - (((float)epoint.y + 0.5f) / (float)self->pm_height));

    gl->BindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
    gl->Viewport(0, 0, self->pm_width, self->pm_height);
    if(fmt != NULL && fmt->indexed)
        ShaderGLLineDrawIndexed(&ctx->line_shader, gl, &line_data, GL_LINES, 2);
    else
        ShaderGLLineDraw(&ctx->line_shader, gl, &line_data, GL_LINES, 2);
    gl->BindFramebuffer(GL_FRAMEBUFFER, 0);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    br_point                  point;
    br_uint_8                 r8 = 0, g8 = 0, b8 = 0, a8 = 255;
    br_gl_line_data           line_data;
    const br_pixelmap_gl_fmt *fmt = DeviceGLGetFormatDetails(self->pm_type);
    const GladGLContext      *gl  = DevicePixelmapGLGetGLContext(self);
    br_gl_context_state      *ctx = GLContextState(gl);

    if(PixelmapPointClip(&point, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if(fmt != NULL && fmt->indexed) {
        line_data.colour = (br_vector4)BR_VECTOR4((float)(colour & 0xff), 0.0f, 0.0f, 0.0f);
    } else {
        BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);
        line_data.colour = (br_vector4)BR_VECTOR4(r8 / 255.0f, g8 / 255.0f, b8 / 255.0f, a8 / 255.0f);
    }

    /*
     * Place the point on the pixel centre, (p + 0.5), matching the line path.
     */
    BrMatrix4Orthographic(&line_data.mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    line_data.start = (br_vector2)BR_VECTOR2(((float)point.x + 0.5f) / (float)self->pm_width,
                                             1.0f - (((float)point.y + 0.5f) / (float)self->pm_height));

    /*
     * What is a point, but half a line?
     */
    gl->BindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
    gl->Viewport(0, 0, self->pm_width, self->pm_height);
    if(fmt != NULL && fmt->indexed)
        ShaderGLLineDrawIndexed(&ctx->line_shader, gl, &line_data, GL_POINTS, 1);
    else
        ShaderGLLineDraw(&ctx->line_shader, gl, &line_data, GL_POINTS, 1);
    gl->BindFramebuffer(GL_FRAMEBUFFER, 0);

    return BRE_OK;
}

void DevicePixelmapGLIncRef(br_device_pixelmap *self)
{
    UASSERT(self->num_refs >= 0);
    ++self->num_refs;
}

void DevicePixelmapGLDecRef(br_device_pixelmap *self)
{
    UASSERT(self->num_refs > 0);
    --self->num_refs;
}

br_rectangle DevicePixelmapGLGetViewport(const br_device_pixelmap *pm)
{
    br_rectangle rect = {
        .x = pm->pm_base_x,
        .y = 0,
        .w = pm->pm_width,
        .h = pm->pm_height,
    };

    /* FIXME: Figure out a better way to detect a sub-pixelmap. */
    if(pm->dispatch != &devicePixelmapDispatch) {
        const br_device_pixelmap *parent = pm->asSub.parent;
        rect.y                           = parent->pm_height - (pm->pm_height + pm->pm_base_y);
    }

    return rect;
}

const GladGLContext *DevicePixelmapGLGetGLContext(br_device_pixelmap *self)
{
    return &self->screen->asFront.glad_gl_context;
}

br_gl_context_state *GLContextState(const GladGLContext *gl)
{
    return gl->userptr;
}

/*
 * Default dispatch table for device pixelmap
 */
static const struct br_device_pixelmap_dispatch devicePixelmapDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_gl, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_gl, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_gl, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_gl, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_gl, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_gl, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_null, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_gl, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_gl, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_gl, allocateSub),

    ._copy         = BR_CMETHOD_REF(br_device_pixelmap_gen, copy),
    ._copyTo       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyTo),
    ._copyFrom     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFrom),
    ._fill         = BR_CMETHOD_REF(br_device_pixelmap_gen, fill),
    ._doubleBuffer = BR_CMETHOD_REF(br_device_pixelmap_fail, doubleBuffer),

    ._copyDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, copyDirty),
    ._copyToDirty       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyToDirty),
    ._copyFromDirty     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFromDirty),
    ._fillDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, fillDirty),
    ._doubleBufferDirty = BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBufferDirty),

    ._rectangle                = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle),
    ._rectangle2               = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle2),
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleCopy),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleStretchCopy),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_gl, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_gl, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_fail, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_gl, text),
    ._textBounds = BR_CMETHOD_REF(br_device_pixelmap_gen, textBounds),

    ._rowSize  = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSize),
    ._rowQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, rowQuery),
    ._rowSet   = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSet),

    ._pixelQuery        = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelQuery),
    ._pixelAddressQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressQuery),

    ._pixelAddressSet = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressSet),
    ._originSet       = BR_CMETHOD_REF(br_device_pixelmap_gen, originSet),

    ._flush        = BR_CMETHOD_REF(br_device_pixelmap_fail, flush),
    ._synchronise  = BR_CMETHOD_REF(br_device_pixelmap_fail, synchronise),
    ._directLock   = BR_CMETHOD_REF(br_device_pixelmap_fail, directLock),
    ._directUnlock = BR_CMETHOD_REF(br_device_pixelmap_fail, directUnlock),
    ._getControls  = BR_CMETHOD_REF(br_device_pixelmap_fail, getControls),
    ._setControls  = BR_CMETHOD_REF(br_device_pixelmap_fail, setControls),

    ._handleWindowEvent = BR_CMETHOD_REF(br_device_pixelmap_gen, handleWindowEvent),
};
