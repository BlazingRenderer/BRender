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

/*
 * Device pixelmap info. template
 */
#define F(f) offsetof(struct br_device_pixelmap, f)
static struct br_tv_template_entry devicePixelmapTemplateEntries[] = {
    {BRT_WIDTH_I32,         NULL, F(pm_width),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT_HEIGHT_I32,        NULL, F(pm_height),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT_PIXEL_TYPE_U8,     NULL, F(pm_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8 },
    {BRT_OUTPUT_FACILITY_O, NULL, F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT_FACILITY_O,        NULL, F(output_facility), BRTV_QUERY,            BRTV_CONV_COPY   },
    {BRT_IDENTIFIER_CSTR,   NULL, F(pm_identifier),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT_MSAA_SAMPLES_I32,  NULL, F(msaa_samples),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
};
#undef F

/*
 * (Re)create the renderbuffers and attach them to the framebuffer.
 */
static br_error recreate_renderbuffers(br_device_pixelmap *self)
{
    GLenum binding_point = self->msaa_samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    UASSERT(self->use_type == BRT_OFFSCREEN || self->use_type == BRT_DEPTH);

    if(self->use_type == BRT_OFFSCREEN) {
        const GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};

        UASSERT(self->asBack.glFbo != 0);

        /* Delete */
        glDeleteTextures(1, &self->asBack.glTex);

        /* Create */
        glGenTextures(1, &self->asBack.glTex);
        glBindTexture(binding_point, self->asBack.glTex);
        glTexParameteri(binding_point, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        if(self->msaa_samples) {
            glTexImage2DMultisample(binding_point, self->msaa_samples, GL_RGBA8, self->pm_width, self->pm_height, GL_TRUE);
        } else {
            glTexImage2D(binding_point, 0, GL_RGBA8, self->pm_width, self->pm_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        }

        glBindTexture(binding_point, 0);

        /* Attach */
        glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, binding_point, self->asBack.glTex, 0);
        glDrawBuffers(1, draw_buffers);
    } else if(self->use_type == BRT_DEPTH) {
        UASSERT(self->asDepth.backbuffer->asBack.glFbo != 0);

        /* Delete */
        glGenTextures(1, &self->asDepth.glDepth);

        /* Create */
        glGenTextures(1, &self->asDepth.glDepth);
        glBindTexture(binding_point, self->asDepth.glDepth);

        if(self->msaa_samples) {
            glTexImage2DMultisample(binding_point, self->msaa_samples, GL_DEPTH_COMPONENT, self->pm_width,
                                    self->pm_height, GL_TRUE);
        } else {
            glTexImage2D(binding_point, 0, GL_DEPTH_COMPONENT, self->pm_width, self->pm_height, 0, GL_DEPTH_COMPONENT,
                         GL_UNSIGNED_BYTE, NULL);
        }

        glBindTexture(binding_point, 0);

        /* Attach */
        glBindFramebuffer(GL_FRAMEBUFFER, self->asDepth.backbuffer->asBack.glFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, binding_point, self->asDepth.glDepth, 0);
    }

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

        /* Cleanup the quad */
        DeviceGLFiniQuad(&self->asBack.quad);
    }
}

void BR_CMETHOD_DECL(br_device_pixelmap_gl, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    BrLogTrace("GLREND", "Freeing %s", self->pm_identifier);

    delete_gl_resources(self);

    ObjectContainerRemove(self->output_facility, (br_object *)self);

    --self->screen->asFront.num_refs;

    BrResFreeNoCallback(self);
}

const char *BR_CMETHOD_DECL(br_device_pixelmap_gl, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

br_token BR_CMETHOD_DECL(br_device_pixelmap_gl, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE_PIXELMAP;
}

br_boolean BR_CMETHOD_DECL(br_device_pixelmap_gl, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

br_device *BR_CMETHOD_DECL(br_device_pixelmap_gl, device)(br_object *self)
{
    (void)self;
    return ((br_device_pixelmap *)self)->device;
}

br_size_t BR_CMETHOD_DECL(br_device_pixelmap_gl, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device_pixelmap);
}

struct br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_gl, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapTemplate == NULL)
        self->device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(self->device, devicePixelmapTemplateEntries,
                                                                              BR_ASIZE(devicePixelmapTemplateEntries));

    return self->device->templates.devicePixelmapTemplate;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
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
    GLint                      gl_internal_format;
    GLenum                     gl_format, gl_type;
    GLsizeiptr                 gl_elem_bytes;
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
            typestring = "Backbuffer";
            break;
        case BRT_DEPTH:
            typestring = "Depth";

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

            /*
             * Not supporting non-16bpp depth buffers.
             */
            if(mt.pixel_bits != 16)
                return BRE_UNSUPPORTED;

            mt.type = BR_PMT_DEPTH_16;
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

    err = VIDEOI_BrPixelmapGetTypeDetails(mt.type, &gl_internal_format, &gl_format, &gl_type, &gl_elem_bytes, NULL);
    if(err != BRE_OK)
        return err;

    if(mt.msaa_samples < 0)
        mt.msaa_samples = 0;
    else if(mt.msaa_samples > hVideo->maxSamples)
        mt.msaa_samples = hVideo->maxSamples;

    pm                  = BrResAllocate(self->device, sizeof(br_device_pixelmap), BR_MEMORY_OBJECT);
    pm->dispatch        = &devicePixelmapDispatch;
    pm->pm_identifier   = BrResSprintf(pm, "OpenGL:%s:%dx%d", typestring, mt.width, mt.height);
    pm->device          = self->device;
    pm->output_facility = self->output_facility;
    pm->use_type        = mt.use_type;
    pm->msaa_samples    = mt.msaa_samples;
    pm->screen          = self->screen;
    ++self->screen->asFront.num_refs;

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
        glGenFramebuffers(1, &pm->asBack.glFbo);
        DeviceGLInitQuad(&pm->asBack.quad, hVideo);
    } else {
        UASSERT(mt.use_type == BRT_DEPTH);
        self->asBack.depthbuffer = pm;
        pm->asDepth.backbuffer   = self;
    }

    if(recreate_renderbuffers(pm) != BRE_OK) {
        --self->screen->asFront.num_refs;
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

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleStretchCopy)(br_device_pixelmap *self, br_rectangle *d,
                                                                      br_device_pixelmap *src, br_rectangle *s)
{
    /*
     * Device->Device non-addressable stretch copy.
     */
    GLbitfield bits;
    GLuint     srcFbo, dstFbo;

    /*
     * Refusing copy/blit to screen.
     */
    if(self->use_type == BRT_NONE)
        return BRE_FAIL;

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

    VIDEOI_BrRectToGL((br_pixelmap *)self, d);
    VIDEOI_BrRectToGL((br_pixelmap *)src, s);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);

    glBlitFramebuffer(s->x, s->y, s->x + s->w, s->y + s->h, d->x, d->y, d->x + d->w, d->y + d->h, bits, GL_NEAREST);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleCopy)(br_device_pixelmap *self, br_point *p,
                                                               br_device_pixelmap *src, br_rectangle *sr)
{
    /* Device->Device non-addressable same-size copy. */
    br_rectangle r = {
        .x = p->x,
        .y = p->y,
        .w = self->pm_width,
        .h = self->pm_height,
    };

    return BR_CMETHOD(br_device_pixelmap_gl, rectangleStretchCopy)(self, &r, src, sr);
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    GLuint     fbo;
    GLbitfield mask;
    float      a = (float)((colour & 0xFF000000) >> 24) / 255.0f;
    float      r = (float)((colour & 0x00FF0000) >> 16) / 255.0f;
    float      g = (float)((colour & 0x0000FF00) >> 8) / 255.0f;
    float      b = (float)((colour & 0x000000FF) >> 0) / 255.0f;
    // TODO: handle the colour format correctly

    VIDEOI_BrRectToGL((br_pixelmap *)self, rect);

    if(self->use_type == BRT_OFFSCREEN) {
        fbo  = self->asBack.glFbo;
        mask = GL_COLOR_BUFFER_BIT;
        glClearColor(r, g, b, a);
        self->asBack.clearColour[0] = r;
        self->asBack.clearColour[1] = g;
        self->asBack.clearColour[2] = b;
        self->asBack.clearColour[3] = a;
    } else if(self->use_type == BRT_DEPTH) {
        UASSERT(colour == 0xFFFFFFFF);
        fbo  = self->asBack.depthbuffer->asBack.glFbo;
        mask = GL_DEPTH_BUFFER_BIT;
        glClearDepth(1.0f);
        self->asDepth.clearValue = 1.0f;
    } else {
        return BRE_UNSUPPORTED;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, self->pm_width, self->pm_height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect->x, rect->y, rect->w, rect->h);

    glClear(mask);

    glDisable(GL_SCISSOR_TEST);

    return BRE_OK;
}

br_error BR_CMETHOD(br_device_pixelmap_gl, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *dr,
                                                                   br_device_pixelmap *_src, br_rectangle *sr)
{
    /* Pixelmap->Device, addressable stretch copy. */
    HVIDEO       hVideo = &self->screen->asFront.video;
    br_pixelmap *src    = (br_pixelmap *)_src;
    GLuint       tex;
    br_matrix4   mvp;

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if((tex = VIDEO_BrPixelmapToGLTexture(src)) == 0)
        return BRE_FAIL;

    /* Convert the rects to OpenGL-coordinates */
    VIDEOI_BrRectToGL(src, sr);
    VIDEOI_BrRectToGL((br_pixelmap *)self, dr);

    /* Patch the quad */
    DeviceGLPatchQuad(&self->asBack.quad, (br_pixelmap *)self, dr, src, sr);

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);

    glViewport(0, 0, self->pm_width, self->pm_height);

    /* Render it */
    glUseProgram(hVideo->defaultProgram.program);

    BrMatrix4Orthographic(&mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    VIDEOI_D3DtoGLProjection(&mvp);

    glUniformMatrix4fv(hVideo->defaultProgram.uMVP, 1, GL_FALSE, (GLfloat *)&mvp);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, BufferStoredGLGetTexture((br_buffer_stored *)src->stored));
    glUniform1i(hVideo->defaultProgram.uSampler, 0);
    glUniform1f(hVideo->defaultProgram.uVerticalFlip, 1);

    DeviceGLDrawQuad(&self->asBack.quad);

    glBindVertexArray(0);
    glDeleteTextures(1, &tex);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleCopyTo)(br_device_pixelmap *self, br_point *p,
                                                                 br_device_pixelmap *src, br_rectangle *sr)
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

/*
 * Device->Pixelmap, addressable same-size copy.
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p,
                                                                   br_device_pixelmap *dest, br_rectangle *r)
{
    br_error   err;
    GLint      internalFormat;
    GLenum     format, type;
    GLsizeiptr elemBytes;
    void      *rowTemp;

    UASSERT(p->x == -self->pm_origin_x);
    UASSERT(p->y == -self->pm_origin_y);

    /*
     * Need contig pixels and whole rows.
     */
    if(!(dest->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)))
        return BRE_FAIL;

    /*
     * Flip to bottom-up. It's already been clipped.
     */
    r->y = self->pm_height - r->h - r->y;

    if((err = VIDEOI_BrPixelmapGetTypeDetails(dest->pm_type, &internalFormat, &format, &type, &elemBytes, NULL)) != BRE_OK)
        return err;

    if((err = DevicePixelmapGLBindFramebuffer(GL_READ_FRAMEBUFFER, self)) != BRE_OK)
        return err;

    glReadPixels(r->x, r->y, r->w, r->h, format, type, dest->pm_pixels);

    /*
     * Flip it
     */
    rowTemp = BrScratchAllocate(dest->pm_row_bytes);
    for(br_uint_16 j = 0; j < dest->pm_height / 2; ++j) {
        void *top = (void *)((br_uintptr_t)dest->pm_pixels + (j * dest->pm_row_bytes));
        void *bot = (void *)((br_uintptr_t)dest->pm_pixels + ((dest->pm_height - j - 1) * dest->pm_row_bytes));
        memcpy(rowTemp, top, dest->pm_row_bytes);
        memcpy(top, bot, dest->pm_row_bytes);
        memcpy(bot, rowTemp, dest->pm_row_bytes);
    }

    /* TODO: If on little-endian systems, swap the byte order. */
    BrScratchFree(rowTemp);
    return BRE_OK;
}

br_error BR_CMETHOD(br_device_pixelmap_gl, text)(br_device_pixelmap *self, br_point *point, br_font *font,
                                                 const char *text, br_uint_32 colour)
{
    GLuint tex;

    /* Quit if off top, bottom or right screen */
    br_int_32 x = point->x + self->pm_origin_x;
    br_int_32 y = point->y + self->pm_origin_y;

    /*
     * Make sure we're an offscreen pixelmap.
     */
    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if(y <= -font->glyph_y || y >= self->pm_height || x >= self->pm_width)
        return BRE_OK;

    /* Ensure we're a valid font. */
    br_font_gl *gl_font;
    if(font == BrFontFixed3x5)
        gl_font = &self->screen->asFront.font_fixed3x5;
    else if(font == BrFontProp4x6)
        gl_font = &self->screen->asFront.font_prop4x6;
    else if(font == BrFontProp7x9)
        gl_font = &self->screen->asFront.font_prop7x9;
    else
        return BRE_FAIL;

    /* Set up the render state. The font UVs match the texture, so no need to flip here. */
    HVIDEO hVideo = &self->screen->asFront.video;

    glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
    glViewport(0, 0, self->pm_width, self->pm_height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glUseProgram(hVideo->textProgram.program);

    br_vector3_f col = {
        {BR_RED(colour) / 255.0f, BR_GRN(colour) / 255.0f, BR_BLU(colour) / 255.0f}
    };

    br_matrix4 mvp;
    BrMatrix4Orthographic(&mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    VIDEOI_D3DtoGLProjection(&mvp);

    glUniformMatrix4fv(hVideo->textProgram.uMVP, 1, GL_FALSE, (GLfloat *)&mvp);
    glActiveTexture(GL_TEXTURE0);

    if(gl_font->tex != 0)
        glBindTexture(GL_TEXTURE_2D, gl_font->tex);
    else
        glBindTexture(GL_TEXTURE_2D, self->screen->asFront.tex_checkerboard);

    glUniform1i(hVideo->textProgram.uSampler, 0);
    glUniform3fv(hVideo->textProgram.uColour, 1, col.v);

    br_rectangle r = {
        .x = point->x,
        .y = point->y,
        .h = font->glyph_y,
        .w = 0,
    };

    for(; *text && r.w <= self->pm_width; ++text) {
        br_uint_8  glyph = (br_uint_8)*text;
        br_uint_16 width = (font->flags & BR_FONTF_PROPORTIONAL) ? font->width[glyph] : font->glyph_x;
        r.w              = width;

        br_rectangle dr = r;
        VIDEOI_BrRectToGL((br_pixelmap *)self, &dr);

        DeviceGLPatchQuadFont(&self->asBack.quad, (br_pixelmap *)self, &dr, gl_font, glyph);
        DeviceGLDrawQuadText(&self->asBack.quad);

        r.x += width + 1;
        r.w += width;
    }

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
    return BRE_OK;
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
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_fail, allocateSub),

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
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_fail, line),
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
