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

static br_error custom_query(br_value *pvalue, void **extra, br_size_t *pextra_size, void *block, const struct br_tv_template_entry *tep)
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
static struct br_tv_template_entry devicePixelmapTemplateEntries[] = {
    {BRT(WIDTH_I32),          F(pm_width),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(HEIGHT_I32),         F(pm_height),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(PIXEL_TYPE_U8),      F(pm_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8,  0                    },
    {BRT(OUTPUT_FACILITY_O),  F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(FACILITY_O),         F(output_facility), BRTV_QUERY,            BRTV_CONV_COPY,    0                    },
    {BRT(CLUT_O),             0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(IDENTIFIER_CSTR),    F(pm_identifier),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(MSAA_SAMPLES_I32),   F(msaa_samples),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {DEV(OPENGL_TEXTURE_U32), 0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
};
#undef F

/*
 * (Re)create the renderbuffers and attach them to the framebuffer.
 */
static br_error recreate_renderbuffers(br_device_pixelmap *self)
{
    GLuint fbo = 0;
    // GLenum binding_point = self->msaa_samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    GLenum binding_point = GL_TEXTURE_2D;

    UASSERT(self->use_type == BRT_OFFSCREEN || self->use_type == BRT_DEPTH);

    if(self->use_type == BRT_OFFSCREEN) {
        const GLenum              draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
        const br_pixelmap_gl_fmt *fmt             = DeviceGLGetFormatDetails(self->pm_type);

        fbo = self->asBack.glFbo;
        UASSERT(fbo != 0);

        /* Delete */
        glDeleteTextures(1, &self->asBack.glTex);

        /* Create */
        glGenTextures(1, &self->asBack.glTex);
        glBindTexture(binding_point, self->asBack.glTex);

        glTexParameteri(binding_point, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(binding_point, 0, fmt->internal_format, self->pm_width, self->pm_height, 0, fmt->format, fmt->type, NULL);

        DeviceGLObjectLabelF(GL_TEXTURE, self->asBack.glTex, "%s:colour", self->pm_identifier);

        glBindTexture(binding_point, 0);

        /* Attach */
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, binding_point, self->asBack.glTex, 0);
        glDrawBuffers(1, draw_buffers);
    } else if(self->use_type == BRT_DEPTH) {
        fbo = self->asDepth.backbuffer->asBack.glFbo;
        UASSERT(fbo != 0);

        /* Delete */
        glDeleteTextures(1, &self->asDepth.glDepth);

        /* Create */
        glGenTextures(1, &self->asDepth.glDepth);
        glBindTexture(binding_point, self->asDepth.glDepth);

        glTexImage2D(binding_point, 0, GL_DEPTH_COMPONENT, self->pm_width, self->pm_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        DeviceGLObjectLabelF(GL_TEXTURE, self->asDepth.glDepth, "%s:depth", self->pm_identifier);

        glBindTexture(binding_point, 0);

        /* Attach */
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, binding_point, self->asDepth.glDepth, 0);
    }

    DeviceGLObjectLabelF(GL_FRAMEBUFFER, fbo, "%s:fbo", self->pm_identifier);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return BRE_FAIL;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return BRE_OK;
}

static void delete_gl_resources(br_device_pixelmap *self)
{
    if(self->use_type == BRT_DEPTH) {
        // FIXME: We should be destroyed before our parent.
        // FIXME: If we haven't, should I bind the parent and detach?
        glDeleteTextures(1, &self->asDepth.glDepth);
    } else if(self->use_type == BRT_OFFSCREEN) {
        glDeleteFramebuffers(1, &self->asBack.glFbo);
        glDeleteTextures(1, &self->asBack.glTex);
    }
}

static void BR_CMETHOD_DECL(br_device_pixelmap_gl, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    UASSERT(self->num_refs == 0);

    BrLogTrace("GLREND", "Freeing %s", self->pm_identifier);

    delete_gl_resources(self);

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

static struct br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_gl, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapTemplate == NULL)
        self->device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(self->device, devicePixelmapTemplateEntries,
                                                                              BR_ASIZE(devicePixelmapTemplateEntries));

    return self->device->templates.devicePixelmapTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    self->pm_width  = width;
    self->pm_height = height;
    return recreate_renderbuffers(self);
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
static struct br_tv_template_entry pixelmapMatchTemplateEntries[] = {
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
    HVIDEO                     hVideo;
    struct pixelmapMatchTokens mt = {
        .width        = self->pm_width,
        .height       = self->pm_height,
        .pixel_bits   = -1,
        .type         = BR_PMT_MAX,
        .use_type     = BRT_NONE,
        .msaa_samples = 0,
    };

    hVideo = &self->screen->asFront.video;

    if(self->device->templates.pixelmapMatchTemplate == NULL) {
        self->device->templates.pixelmapMatchTemplate = BrTVTemplateAllocate(self->device, pixelmapMatchTemplateEntries,
                                                                             BR_ASIZE(pixelmapMatchTemplateEntries));
    }

    err = BrTokenValueSetMany(&mt, &count, NULL, tv, self->device->templates.pixelmapMatchTemplate);
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
     * Refuse creation of indexed pixelmaps.
     */
    if(fmt->indexed)
        return BRE_FAIL;

    if(mt.msaa_samples < 0)
        mt.msaa_samples = 0;
    else if(mt.msaa_samples > hVideo->maxSamples)
        mt.msaa_samples = hVideo->maxSamples;

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
        pm->asBack.clut        = DeviceClutGLAllocate(pm);
        glGenFramebuffers(1, &pm->asBack.glFbo);
    } else {
        UASSERT(mt.use_type == BRT_DEPTH);
        self->asBack.depthbuffer = pm;
        pm->asDepth.backbuffer   = self;
    }

    if(recreate_renderbuffers(pm) != BRE_OK) {
        DevicePixelmapGLDecRef(self->screen);
        delete_gl_resources(pm);
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

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleStretchCopy)(br_device_pixelmap *self, br_rectangle *d,
                                                                             br_device_pixelmap *src, br_rectangle *s)
{
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

    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);

    glBlitFramebuffer(srect.x, srect.y, srect.x + srect.w, srect.y + srect.h, drect.x, drect.y, drect.x + drect.w, drect.y + drect.h, bits,
                      GL_NEAREST);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

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
    GLuint       fbo;
    GLbitfield   mask;
    br_rectangle drect;

    /*
     * Clip rectangle to pixelmap
     */
    if(DevicePixelmapGLRectangleClip(&drect, rect, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if(self->use_type == BRT_OFFSCREEN) {
        br_uint_8 r8 = 0, g8 = 0, b8 = 0, a8 = 255;
        BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);

        fbo  = self->asBack.glFbo;
        mask = GL_COLOR_BUFFER_BIT;

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor((float)r8 / 255.0f, (float)g8 / 255.0f, (float)b8 / 255.0f, (float)a8 / 255.0f);
    } else if(self->use_type == BRT_DEPTH) {
        UASSERT(colour == 0xFFFFFFFF);
        fbo  = self->asDepth.backbuffer->asBack.glFbo;
        mask = GL_DEPTH_BUFFER_BIT;
        glDepthMask(GL_TRUE);
        glClearDepthf(1.0f);
    } else {
        return BRE_UNSUPPORTED;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, self->pm_width, self->pm_height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(drect.x, drect.y, drect.w, drect.h);

    glClear(mask);

    glDisable(GL_SCISSOR_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return BRE_OK;
}

static br_error BR_CMETHOD(br_device_pixelmap_gl, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *d,
                                                                          br_device_pixelmap *_src, br_rectangle *s)
{
    /* Pixelmap->Device, addressable stretch copy. */

#pragma pack(push, 16)
    typedef struct br_rect_gl_data {
        alignas(16) br_matrix4 mvp;
        alignas(16) br_vector4 src_rect;
        alignas(16) br_vector4 dst_rect;
        alignas(4) float vertical_flip;
        alignas(4) float indexed;
    } br_rect_gl_data;
#pragma pack(pop)

    HVIDEO                    hVideo = &self->screen->asFront.video;
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

    if(stored != NULL && ObjectDevice(stored) == self->device) {
        tex = BufferStoredGLGetTexture(stored);
        fmt = stored->fmt;
    } else {
        tex     = DeviceGLPixelmapToGLTexture(src);
        tex_tmp = BR_TRUE;
        fmt     = DeviceGLGetFormatDetails(src->type);
    }

    if(fmt == NULL) {
        return BRE_FAIL;
    }

    clut = self->asBack.clut->gl_tex ? self->asBack.clut->gl_tex : self->screen->asFront.tex_white;
    if(fmt->indexed && src->map != NULL) {
        if(src->map->stored != NULL) {
            clut = BufferStoredGLGetCLUTTexture(stored, self, clut);
        } else {
            clut     = DeviceGLPixelmapToGLTexture(src->map);
            clut_tmp = BR_TRUE;
        }
    }

    if(tex == 0) {
        if(clut != 0 && clut_tmp)
            glDeleteTextures(1, &clut);
        return BRE_FAIL;
    }

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);

    glViewport(0, 0, self->pm_width, self->pm_height);

    /* Render it */
    glUseProgram(hVideo->rectProgram.program);

    if(fmt->indexed) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, clut);
        glUniform1i(hVideo->rectProgram.uSampler, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(hVideo->rectProgram.uIndexTex, 1);
    } else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        glUniform1i(hVideo->rectProgram.uSampler, 0);
    }

    br_rect_gl_data rect_data = {
        .mvp      = {},
        .src_rect = BR_VECTOR4((float)srect.x / (float)src->width, (float)srect.y / (float)src->height, (float)srect.w / (float)src->width,
                               (float)srect.h / (float)src->height),
        .dst_rect = BR_VECTOR4((float)drect.x / (float)self->pm_width, (float)drect.y / (float)self->pm_height,
                               (float)drect.w / (float)self->pm_width, (float)drect.h / (float)self->pm_height),
        .vertical_flip = 1,
        .indexed       = (float)fmt->indexed,
    };

    BrMatrix4Orthographic(&rect_data.mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

    glBindVertexArray(hVideo->rectProgram.vao);
    glBindBuffer(GL_UNIFORM_BUFFER, hVideo->rectProgram.ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(rect_data), &rect_data, GL_STATIC_DRAW);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 2);

    glBindVertexArray(0);

    if(tex_tmp)
        glDeleteTextures(1, &tex);

    if(clut_tmp)
        glDeleteTextures(1, &clut);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    br_error                  err;
    void                     *row_temp;
    const br_pixelmap_gl_fmt *fmt;
    br_rectangle              srect;
    br_point                  dpoint;
    br_uint_16                bytes_per_pixel;
    void                     *dst_pixels;

    /*
     * Need contig pixels and whole rows.
     */
    if(!(dest->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)))
        return BRE_FAIL;

    if(PixelmapRectangleClipTwo(&srect, &dpoint, r, p, (br_pixelmap *)dest, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if((fmt = DeviceGLGetFormatDetails(dest->pm_type)) == NULL)
        return BRE_FAIL;

    if((err = DevicePixelmapGLBindFramebuffer(GL_READ_FRAMEBUFFER, self)) != BRE_OK)
        return err;

    bytes_per_pixel = BrPixelmapPixelSize((br_pixelmap *)dest) >> 3;
    dst_pixels      = DevicePixelmapMemAddress(dest, dpoint.x, dpoint.y, bytes_per_pixel);

    glPixelStorei(GL_PACK_ROW_LENGTH, srect.w);
    glReadPixels(srect.x, self->pm_height - srect.y - srect.h, srect.w, srect.h, fmt->format, fmt->type, dst_pixels);

    size_t bytes_per_subrow = srect.w * bytes_per_pixel;
    row_temp                = BrScratchAllocate(bytes_per_subrow);
    for(br_uint_16 j = 0; j < srect.h / 2; ++j) {
        void *top = DevicePixelmapMemAddress(dest, dpoint.x, dpoint.y + j, bytes_per_pixel);
        void *bot = DevicePixelmapMemAddress(dest, dpoint.x, dpoint.y + srect.h - j - 1, bytes_per_pixel);

        memcpy(row_temp, top, bytes_per_subrow);
        memcpy(top, bot, bytes_per_subrow);
        memcpy(bot, row_temp, bytes_per_subrow);
    }

    /* TODO: If on little-endian systems, swap the byte order. */
    BrScratchFree(row_temp);
    return BRE_OK;
}

static br_error BR_CMETHOD(br_device_pixelmap_gl, text)(br_device_pixelmap *self, br_point *point, br_font *font, const char *text, br_uint_32 colour)
{

    size_t      len = strlen(text);
    br_point    pp;
    br_font_gl *gl_font;
    HVIDEO      hVideo = &self->screen->asFront.video;
    br_text_gl *text_data;
    br_uint_8   r8 = 0, g8 = 0, b8 = 0, a8 = 255;

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
        gl_font = &self->screen->asFront.font_fixed3x5;
    else if(font == BrFontProp4x6)
        gl_font = &self->screen->asFront.font_prop4x6;
    else if(font == BrFontProp7x9)
        gl_font = &self->screen->asFront.font_prop7x9;
    else
        return BRE_FAIL;

    /*
     * All valid, set up the render state.
     */
    glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
    glViewport(0, 0, self->pm_width, self->pm_height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glUseProgram(hVideo->textProgram.program);
    glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->textProgram.block_binding_font_data, gl_font->font_data);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, gl_font->tex);
    glUniform1i(hVideo->textProgram.uSampler, 0);

    /*
     * Create the per-model/text state.
     */
    text_data = BrScratchAllocate(sizeof(br_text_gl));
    BrMatrix4Orthographic(&text_data->mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

    BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);
    text_data->colour = (br_vector4)BR_VECTOR4(r8 / 255.0f, g8 / 255.0f, b8 / 255.0f, a8 / 255.0f);

    glBindVertexArray(self->screen->asFront.video.textProgram.vao_glyphs);
    glBindBuffer(GL_UNIFORM_BUFFER, self->screen->asFront.video.textProgram.ubo_glyphs);

    br_rectangle r = {
        .x = point->x,
        .y = point->y,
        .h = font->glyph_y,
        .w = 0,
    };

    do {
        size_t chunk = len;
        if(chunk > BR_TEXT_GL_CHUNK_SIZE) {
            chunk = BR_TEXT_GL_CHUNK_SIZE;
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

        glBufferData(GL_UNIFORM_BUFFER, sizeof(br_text_gl), text_data, GL_STATIC_DRAW);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)chunk);

        len -= chunk;
        text += chunk;
    } while(len > 0);

    BrScratchFree(text_data);

    glBindVertexArray(0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
    return BRE_OK;
}

#pragma pack(push, 16)
typedef struct br_line_gl {
    alignas(16) br_matrix4 mvp;
    alignas(8) br_vector2 start;
    alignas(8) br_vector2 end;
    alignas(16) br_vector4 colour;
} br_line_gl;
#pragma pack(pop)

static br_error render_line(br_device_pixelmap *self, const br_line_gl *line_data, GLenum mode, GLsizei count)
{
    HVIDEO hVideo = &self->screen->asFront.video;

    glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
    glViewport(0, 0, self->pm_width, self->pm_height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glUseProgram(hVideo->lineProgram.program);

    glBindVertexArray(self->screen->asFront.video.lineProgram.vao);
    glBindBuffer(GL_UNIFORM_BUFFER, self->screen->asFront.video.lineProgram.ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(br_line_gl), line_data, GL_STATIC_DRAW);

    glDrawArrays(mode, 0, count);

    glBindVertexArray(0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, line)(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
    br_point   spoint, epoint;
    br_uint_8  r8 = 0, g8 = 0, b8 = 0, a8 = 255;
    br_line_gl line_data;

    if(PixelmapLineClip(&spoint, &epoint, s, e, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);

    BrMatrix4Orthographic(&line_data.mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    line_data.start  = (br_vector2)BR_VECTOR2((float)spoint.x / (float)self->pm_width, 1.0f - ((float)spoint.y / (float)self->pm_height));
    line_data.end    = (br_vector2)BR_VECTOR2((float)epoint.x / (float)self->pm_width, 1.0f - ((float)epoint.y / (float)self->pm_height));
    line_data.colour = (br_vector4)BR_VECTOR4(r8 / 255.0f, g8 / 255.0f, b8 / 255.0f, a8 / 255.0f);

    return render_line(self, &line_data, GL_LINES, 2);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    br_point   point;
    br_uint_8  r8 = 0, g8 = 0, b8 = 0, a8 = 255;
    br_line_gl line_data;

    if(PixelmapPointClip(&point, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);

    /*
     * FIXME: For some reason I cbf figuring out now, when rendering a point, both components need a -1.
     */
    BrMatrix4Orthographic(&line_data.mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    line_data.start = (br_vector2)BR_VECTOR2((float)(point.x - 1) / (float)self->pm_width, 1.0f - ((float)(point.y - 1) / (float)self->pm_height));
    line_data.colour = (br_vector4)BR_VECTOR4(r8 / 255.0f, g8 / 255.0f, b8 / 255.0f, a8 / 255.0f);

    /*
     * What is a point, but half a line?
     */
    return render_line(self, &line_data, GL_POINTS, 1);
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
