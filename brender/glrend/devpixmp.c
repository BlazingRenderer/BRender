/*
 * Device pixelmap methods
 *
 * TODO:
 * - Cleanup dangling pointers when a back/depth buffer is destroyed.
 */

#include <string.h>
#include <inttypes.h>
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
    {BRT_WIDTH_I32,               NULL, F(pm_width),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16,},
    {BRT_HEIGHT_I32,              NULL, F(pm_height),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16,},
    {BRT_PIXEL_TYPE_U8,           NULL, F(pm_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8,},
    {BRT_OUTPUT_FACILITY_O,       NULL, F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_FACILITY_O,              NULL, F(output_facility), BRTV_QUERY,            BRTV_CONV_COPY,},
    {BRT_IDENTIFIER_CSTR,         NULL, F(pm_identifier),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_MSAA_SAMPLES_I32,        NULL, F(msaa_samples),    BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,}
};
#undef F

struct pixelmapNewTokens {
    br_int_32 width;
    br_int_32 height;
    br_int_32 pixel_bits;
    br_uint_8 pixel_type;
    int msaa_samples;
};

#define F(f) offsetof(struct pixelmapNewTokens, f)
static struct br_tv_template_entry pixelmapNewTemplateEntries[] = {
    {BRT_WIDTH_I32,        NULL, F(width),        BRTV_SET, BRTV_CONV_COPY,},
    {BRT_HEIGHT_I32,       NULL, F(height),       BRTV_SET, BRTV_CONV_COPY,},
    {BRT_PIXEL_BITS_I32,   NULL, F(pixel_bits),   BRTV_SET, BRTV_CONV_COPY,},
    {BRT_PIXEL_TYPE_U8,    NULL, F(pixel_type),   BRTV_SET, BRTV_CONV_COPY,},
    {BRT_MSAA_SAMPLES_I32, NULL, F(msaa_samples), BRTV_SET, BRTV_CONV_COPY,},
};
#undef F

static void actually_patch_quad(br_device_pixelmap_gl_quad *self,
                                float dx0, float dy0, float dx1, float dy1,
                                float sx0, float sy0, float sx1, float sy1)
{
    /* Bottom-left */
    self->tris[0].x = dx0;
    self->tris[0].y = dy0;
    self->tris[0].u = sx0;
    self->tris[0].v = sy0;

    /* Top-left */
    self->tris[1].x = dx0;
    self->tris[1].y = dy1;
    self->tris[1].u = sx0;
    self->tris[1].v = sy1;

    /* Top-right */
    self->tris[2].x = dx1;
    self->tris[2].y = dy1;
    self->tris[2].u = sx1;
    self->tris[2].v = sy1;

    /* Bottom right */
    self->tris[3].x = dx1;
    self->tris[3].y = dy0;
    self->tris[3].u = sx1;
    self->tris[3].v = sy0;

    /* Upload the data */
    glBindBuffer(GL_ARRAY_BUFFER, self->buffers[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(self->tris), self->tris);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*
 * Patch the X/Y and U/V coordinates.
 * X/Y = rect of destination
 * U/V = rect of source
 * Rects are in OpenGL coordianges -- (0,0) at bottom-left
 */
static void patch_quad(br_device_pixelmap *self, br_rectangle *dr, br_pixelmap *src, br_rectangle *sr)
{
    float x0, y0, x1, y1;
    VIDEOI_BrRectToUVs((br_pixelmap *)self, dr, &x0, &y0, &x1, &y1);

    float sx0, sy0, sx1, sy1;
    VIDEOI_BrRectToUVs((br_pixelmap *)src, sr, &sx0, &sy0, &sx1, &sy1);

    actually_patch_quad(&self->asBack.quad, x0, y0, x1, y1, sx0, sy0, sx1, sy1);
}

const static GLsizei s_QuadStride = sizeof(br_device_pixelmap_gl_tri);

/* Quad GL_TRIANGES indices. */
static GLuint s_QuadIndices[] = {0, 1, 2, 2, 3, 0};

/* Default quad data. Format is XYZRGBUV. */
static br_device_pixelmap_gl_tri s_QuadData[4] = {
    {
        /* Bottom-left */
        -1.0f, -1.0f, 0.5f,
        1.0f, 1.0f, 1.0f,
        0.0f, 0.0f,
    },
    {
        /* Top-left */
        -1.0f, 1.0f,  0.5f,
        1.0f, 1.0f, 1.0f,
        0.0f, 1.0f,
    },
    {
        /* Top-right */
        1.0f,  1.0f,  0.5f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,
    },
    {
        /* Bottom-right */
        1.0f,  -1.0f, 0.5f,
        1.0f, 1.0f, 1.0f,
        1.0f, 0.0f,
    }
};

static void create_quad(br_device_pixelmap *self)
{
    HVIDEO hVideo = &self->device->video;

    /* Create the crap needed for the operations quad. */
    memcpy(self->asBack.quad.tris, s_QuadData, sizeof(self->asBack.quad.tris));
    static_assert(sizeof(self->asBack.quad.tris) == sizeof(s_QuadData), "Quad size mismatch");

    { /* Build the buffers */
        glGenBuffers(2, self->asBack.quad.buffers);

        glBindBuffer(GL_ARRAY_BUFFER, self->asBack.quad.buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(self->asBack.quad.tris), self->asBack.quad.tris, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->asBack.quad.buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_QuadIndices), s_QuadIndices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    { /* Default Quad VAO */
        glGenVertexArrays(1, &self->asBack.quad.defaultVao);
        glBindVertexArray(self->asBack.quad.defaultVao);

        glBindBuffer(GL_ARRAY_BUFFER, self->asBack.quad.buffers[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->asBack.quad.buffers[1]);

        /* Positions */
        glEnableVertexAttribArray(hVideo->defaultProgram.aPosition);
        glVertexAttribPointer(hVideo->defaultProgram.aPosition, 3, GL_FLOAT, GL_FALSE, s_QuadStride, NULL);

        /* Colours */
        glEnableVertexAttribArray(hVideo->defaultProgram.aColour);
        glVertexAttribPointer(hVideo->defaultProgram.aColour, 3, GL_FLOAT, GL_FALSE, s_QuadStride,
                              (GLvoid *)(sizeof(GLfloat) * 3));

        /* UVs */
        glEnableVertexAttribArray(hVideo->defaultProgram.aUV);
        glVertexAttribPointer(hVideo->defaultProgram.aUV, 2, GL_FLOAT, GL_FALSE, s_QuadStride,
                              (GLvoid *)(sizeof(GLfloat) * 6));
    }

    { /* Text Quad VAO */
        glGenVertexArrays(1, &self->asBack.quad.textVao);
        glBindVertexArray(self->asBack.quad.textVao);

        glBindBuffer(GL_ARRAY_BUFFER, self->asBack.quad.buffers[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->asBack.quad.buffers[1]);

        /* Positions */
        glEnableVertexAttribArray(hVideo->textProgram.aPosition);
        glVertexAttribPointer(hVideo->textProgram.aPosition, 3, GL_FLOAT, GL_FALSE, s_QuadStride, NULL);

        /* UVs */
        glEnableVertexAttribArray(hVideo->textProgram.aUV);
        glVertexAttribPointer(hVideo->textProgram.aUV, 2, GL_FLOAT, GL_FALSE, s_QuadStride,
                              (GLvoid *)(sizeof(GLfloat) * 6));
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /*
     * Some of the above can fail if the user changes the shader.
     * Drain all the errors to prevent them falling through to future checks.
     */
    while(glGetError() != GL_NO_ERROR);
}


/*
 * (Re)create the renderbuffers and attach them to the framebuffer.
 */
static br_error recreate_renderbuffers(br_device_pixelmap *self)
{
    ASSERT(self->use_type == BRT_OFFSCREEN || self->use_type == BRT_DEPTH);

    if(self->use_type == BRT_OFFSCREEN) {
        const GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};

        ASSERT(self->asBack.glFbo != 0);

        /* Delete */
        glDeleteRenderbuffers(1, &self->asBack.glTex);

        /* Create */
        glGenRenderbuffers(1, &self->asBack.glTex);
        glBindRenderbuffer(GL_RENDERBUFFER, self->asBack.glTex);

        if(self->msaa_samples)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, self->msaa_samples, GL_RGBA8, self->pm_width,
                                             self->pm_height);
        else
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, self->pm_width, self->pm_height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        /* Attach */
        glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, self->asBack.glTex);
        glDrawBuffers(1, draw_buffers);
    } else if(self->use_type == BRT_DEPTH) {
        ASSERT(self->asDepth.backbuffer->asBack.glFbo != 0);

        /* Delete */
        glDeleteRenderbuffers(1, &self->asDepth.glDepth);

        /* Create */
        glGenRenderbuffers(1, &self->asDepth.glDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, self->asDepth.glDepth);

        if(self->msaa_samples)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, self->msaa_samples, GL_DEPTH_COMPONENT24, self->pm_width,
                                             self->pm_height);
        else
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, self->pm_width, self->pm_height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        /* Attach */
        glBindFramebuffer(GL_FRAMEBUFFER, self->asDepth.backbuffer->asBack.glFbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, self->asDepth.glDepth);
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
        glDeleteRenderbuffers(1, &self->asDepth.glDepth);
    } else if(self->use_type == BRT_OFFSCREEN) {
        glDeleteFramebuffers(1, &self->asBack.glFbo);
        glDeleteRenderbuffers(1, &self->asBack.glTex);

        /* Cleanup the quad */
        glDeleteBuffers(2, self->asBack.quad.buffers);
        glDeleteVertexArrays(1, &self->asBack.quad.defaultVao);
        glDeleteVertexArrays(1, &self->asBack.quad.textVao);
    }
}

static br_error create_pixelmap(br_device_pixelmap **newpm, br_object *parent, br_token use_type,
                                br_uint_8 type, br_uint_16 width, br_uint_16 height, br_int_32 msaa_samples)
{
    br_device_pixelmap *pm, *ppm;
    br_output_facility *outfcty;
    br_device          *device;
    char               buf[128];
    const char         *tag;
    GLint              gl_internal_format;
    GLenum             gl_format, gl_type;
    GLsizeiptr         gl_elem_bytes;
    br_error           err;

    device = ObjectDevice(parent);

    if(ObjectIsType(parent, BRT_OUTPUT_FACILITY)) {
        ASSERT(use_type == BRT_NONE);
        ppm     = NULL;
        outfcty = (br_output_facility *)parent;
    } else if(ObjectIsType(parent, BRT_DEVICE_PIXELMAP)) {
        ppm     = ((br_device_pixelmap *)parent);
        outfcty = ppm->output_facility;
    } else {
        return BRE_FAIL;
    }

    if(msaa_samples < 0)
        msaa_samples = 0;
    else if(msaa_samples > device->video.maxSamples)
        msaa_samples = device->video.maxSamples;

    err = VIDEOI_BrPixelmapGetTypeDetails(type, &gl_internal_format, &gl_format, &gl_type, &gl_elem_bytes, NULL);
    if(err != BRE_OK)
        return err;

    switch(use_type) {
        case BRT_NONE:
            ASSERT(ObjectIsType(parent, BRT_OUTPUT_FACILITY));
            tag = "Screen";
            break;
        case BRT_NO_RENDER:
            use_type = BRT_OFFSCREEN;
        case BRT_OFFSCREEN:
            ASSERT(ObjectIsType(parent, BRT_DEVICE_PIXELMAP));
            tag = "Backbuffer";
            break;
        case BRT_DEPTH:
            ASSERT(ObjectIsType(parent, BRT_DEVICE_PIXELMAP));
            tag = "Depth";
            break;
        default:
            return BRE_FAIL;
    }

    BrSprintfN(buf, sizeof(buf), "%" PRIu16 "x%" PRIu16 " (%s)", width, height, tag);

    pm = BrResAllocate(device, sizeof(br_device_pixelmap), BR_MEMORY_OBJECT);
    pm->dispatch        = &devicePixelmapDispatch;
    pm->output_facility = outfcty;
    pm->pm_identifier   = BrResStrDup(pm, buf);
    pm->device          = device;
    pm->use_type        = use_type;
    pm->msaa_samples    = msaa_samples;

    pm->pm_type   = type;
    pm->pm_width  = width;
    pm->pm_height = height;
    pm->pm_flags  = BR_PMF_NO_ACCESS;

    if(use_type == BRT_NONE) {
        pm->pm_origin_x = 0;
        pm->pm_origin_y = 0;
        pm->pm_base_x   = 0;
        pm->pm_base_y   = 0;

        *newpm = pm;
        ObjectContainerAddFront(outfcty, (br_object *)pm);
        return BRE_OK;
    }

    pm->pm_origin_x = ppm->pm_origin_x;
    pm->pm_origin_y = ppm->pm_origin_y;
    pm->pm_base_x   = ppm->pm_base_x;
    pm->pm_base_y   = ppm->pm_base_y;

    if(use_type == BRT_OFFSCREEN) {
        pm->asBack.depthbuffer = NULL;
        glGenFramebuffers(1, &pm->asBack.glFbo);
        create_quad(pm);
    } else {
        ASSERT(use_type == BRT_DEPTH);
        ppm->asBack.depthbuffer = pm;
        pm->asDepth.backbuffer  = ppm;
    }

    if(recreate_renderbuffers(pm) != BRE_OK) {
        delete_gl_resources(pm);
        BrResFreeNoCallback(newpm);
        return BRE_FAIL;
    }

    *newpm = pm;
    ObjectContainerAddFront(outfcty, (br_object *)pm);
    return BRE_OK;
}


/*
 * Create a new device pixelmap and set a display mode
 */
br_device_pixelmap *DevicePixelmapGLAllocate(br_device *device, br_output_facility *facility, br_token_value *tv)
{
    br_device_pixelmap       *self;
    br_int_32                count;
    br_error                 err;
    struct pixelmapNewTokens pt = {
        .width        = -1,
        .height       = -1,
        .pixel_bits   = -1,
        .pixel_type   = BR_PMT_MAX,
        .msaa_samples = 0,
    };

    if(device->templates.pixelmapNewTemplate == NULL) {
        device->templates.pixelmapNewTemplate = BrTVTemplateAllocate(
            device,
            pixelmapNewTemplateEntries,
            BR_ASIZE(pixelmapNewTemplateEntries)
        );
    }

    BrTokenValueSetMany(&pt, &count, NULL, tv, device->templates.pixelmapNewTemplate);

    if(pt.width <= 0 || pt.height <= 0)
        return NULL;

    /*
     * If the user's manually specified a type, use it.
     * Otherwise, figure one out from the bit depth.
     */
    if(pt.pixel_type == BR_PMT_MAX) {
        switch(pt.pixel_bits) {
            case 16:
                pt.pixel_type = BR_PMT_RGB_565;
                break;
            case 24:
                pt.pixel_type = BR_PMT_RGB_888;
                break;
            case 32:
                pt.pixel_type = BR_PMT_RGBX_888;
                break;
            default:
                BrLogError("GLREND", "Unsupported pixel depth %d requested.", pt.pixel_bits);
                return NULL;
        }
    }

    err = create_pixelmap(&self, (br_object *)facility, BRT_NONE, pt.pixel_type,
                          pt.width, pt.height, pt.msaa_samples);
    if(err != BRE_OK)
        return NULL;

    return self;
}


void BR_CMETHOD_DECL(br_device_pixelmap_gl, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    delete_gl_resources(self);

    ObjectContainerRemove(self->output_facility, (br_object *)self);

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
        self->device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(
            self->device,
            devicePixelmapTemplateEntries,
            BR_ASIZE(devicePixelmapTemplateEntries)
        );

    return self->device->templates.devicePixelmapTemplate;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    /* Front buffer, ¯\_(ツ)_/¯ */
    if(self->use_type == BRT_NONE) {
        self->pm_width  = width;
        self->pm_height = height;
        return BRE_OK;
    }

    if(self->use_type == BRT_OFFSCREEN || self->use_type == BRT_DEPTH) {
        self->pm_width  = width;
        self->pm_height = height;
        return recreate_renderbuffers(self);
    }

    return BRE_FAIL;
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
};

#define F(f) offsetof(struct pixelmapMatchTokens, f)
static struct br_tv_template_entry pixelmapMatchTemplateEntries[] = {
    {BRT_WIDTH_I32,      NULL, F(width),      BRTV_SET, BRTV_CONV_COPY,},
    {BRT_HEIGHT_I32,     NULL, F(height),     BRTV_SET, BRTV_CONV_COPY,},
    {BRT_PIXEL_BITS_I32, NULL, F(pixel_bits), BRTV_SET, BRTV_CONV_COPY,},
    {BRT_PIXEL_TYPE_U8,  NULL, F(type),       BRTV_SET, BRTV_CONV_COPY,},
    {BRT_USE_T,          NULL, F(use_type),   BRTV_SET, BRTV_CONV_COPY,},
};
#undef F

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, match)(br_device_pixelmap *self,
                                                       br_device_pixelmap **_newpm, br_token_value *tv)
{
    br_int_32                  count;
    br_uint_8                  type;
    struct pixelmapMatchTokens mt = {.width = -1, .height = -1, .pixel_bits = -1, .type = BR_PMT_MAX, .use_type = BRT_NONE};

    if(self->device->templates.pixelmapMatchTemplate == NULL) {
        self->device->templates.pixelmapMatchTemplate = BrTVTemplateAllocate(
            self->device, pixelmapMatchTemplateEntries,
            BR_ASIZE(pixelmapMatchTemplateEntries)
        );
    }

    BrTokenValueSetMany(&mt, &count, NULL, tv, self->device->templates.pixelmapMatchTemplate);

    if(mt.use_type == BRT_NO_RENDER)
        mt.use_type = BRT_OFFSCREEN;

    if(mt.use_type == BRT_DEPTH) {
        /* Depth buffers must be matched with the backbuffer */
        if(self->use_type != BRT_OFFSCREEN)
            return BRE_FAIL;

        /* Can't have >1 depth buffer */
        if(self->asBack.depthbuffer != NULL)
            return BRE_FAIL;

        /* Not supporting non-16bpp depth buffers */
        if(mt.pixel_bits != 16)
            return BRE_FAIL;

        type = BR_PMT_DEPTH_16;
        mt.width  = self->pm_width;
        mt.height = self->pm_height;
    } else if(mt.type != BR_PMT_MAX) {
        type = mt.type;
    } else {
        type = self->pm_type;
    }

    return create_pixelmap(_newpm, (br_object *)self, mt.use_type, type, mt.width, mt.height, self->msaa_samples);
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    if(self->use_type != BRT_NONE || src->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    /* Blit the FBO to the screen and swap. */
    BrPixelmapCopy((br_pixelmap *)self, (br_pixelmap *)src);

    /* Call our hook */
    DeviceGLPreSwap(self->device, src->asBack.glFbo);
    while(glGetError() != GL_NO_ERROR);

    DeviceGLSwapBuffers(self->device, self);
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleStretchCopy)(br_device_pixelmap *self, br_rectangle *d,
                                                                      br_device_pixelmap *src, br_rectangle *s)
{
    /* Device->Device non-addressable stretch copy */
    GLbitfield bits;
    GLuint     srcFbo, dstFbo;
    if(self->use_type == BRT_NONE) {
        if(src->use_type != BRT_OFFSCREEN)
            return BRE_FAIL;
        dstFbo = 0;
        srcFbo = src->asBack.glFbo;
        bits   = GL_COLOR_BUFFER_BIT;
    } else if(self->use_type == BRT_OFFSCREEN) {
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

    /* Ignore self-blit. */
    if(self == src)
        return BRE_OK;

    VIDEOI_BrRectToGL((br_pixelmap *)self, d);
    VIDEOI_BrRectToGL((br_pixelmap *)src, s);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);

    glBlitFramebuffer(
        s->x, s->y, s->x + s->w, s->y + s->h,
        d->x, d->y, d->x + d->w, d->y + d->h,
        bits,
        GL_NEAREST
    );

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleCopy)(br_device_pixelmap *self,
                                                               br_point *p, br_device_pixelmap *src, br_rectangle *sr)
{
    /* Device->Device non-addressable same-size copy. */
    br_rectangle r = {
        .x = -self->pm_origin_x,
        .y = -self->pm_origin_y,
        .w = self->pm_width,
        .h = self->pm_height,
    };

    return BR_CMETHOD(br_device_pixelmap_gl, rectangleStretchCopy)(self, &r, src, sr);
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleFill)(br_device_pixelmap *self,
                                                               br_rectangle *rect, br_uint_32 colour)
{
    GLuint     fbo;
    GLbitfield mask;
    float      a = (float)((colour & 0xFF000000) >> 24) / 255.0f;
    float      r = (float)((colour & 0x00FF0000) >> 16) / 255.0f;
    float      g = (float)((colour & 0x0000FF00) >>  8) / 255.0f;
    float      b = (float)((colour & 0x000000FF) >>  0) / 255.0f;

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
        ASSERT(colour == 0xFFFFFFFF);
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
    HVIDEO      hVideo = &self->device->video;
    br_pixelmap *src   = (br_pixelmap*)_src;
    GLuint      tex;
    br_matrix4  mvp;

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if((tex = VIDEO_BrPixelmapToGLTexture(src)) == 0)
        return BRE_FAIL;

    /* Convert the rects to OpenGL-coordinates */
    VIDEOI_BrRectToGL(src, sr);
    VIDEOI_BrRectToGL((br_pixelmap *)self, dr);

    /* Patch the quad */
    patch_quad(self, dr, src, sr);


    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);

    glViewport(0, 0, self->pm_width, self->pm_height);

    /* Render it */
    glUseProgram(hVideo->defaultProgram.program);

    BrMatrix4Orthographic(&mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    VIDEOI_D3DtoGLProjection(&mvp);

    glUniformMatrix4fv(hVideo->defaultProgram.uMVP, 1, GL_FALSE, (GLfloat *)&mvp);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(hVideo->defaultProgram.uSampler, 0);
    glUniform1f(hVideo->defaultProgram.uVerticalFlip, 1);

    glBindVertexArray(self->asBack.quad.defaultVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
    glDeleteTextures(1, &tex);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleCopyTo)
    (br_device_pixelmap *self, br_point *p, br_device_pixelmap *src, br_rectangle *sr)
{
    /* Pixelmap->Device, addressable same-size copy. */

    /* FIXME: Still have no idea what p is for. */

    /* Do a stretch copy with no stretching. */
    br_rectangle r = {
        .x = -self->pm_origin_x,
        .y = -self->pm_origin_y,
        .w = self->pm_width,
        .h = self->pm_height,
    };
    return BR_CMETHOD(br_device_pixelmap_gl, rectangleStretchCopyTo)(self, &r, src, sr);
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p,
                                                                   br_device_pixelmap *dest, br_rectangle *dr)
{
    /* Device->Pixelmap, addressable same-size copy. */

    /* Need contig pixels. */
    if(!(dest->pm_flags & BR_PMF_LINEAR))
        return BRE_FAIL;

    br_rectangle glDr = *dr;
    VIDEOI_BrRectToGL((br_pixelmap *)dest, &glDr);

    /* Only do fullscreen copies */
    if(glDr.x != 0 && glDr.y != 0 && glDr.w != self->pm_width && glDr.h != self->pm_height)
        return BRE_FAIL;

    GLint      internalFormat;
    GLenum     format, type;
    GLsizeiptr elemBytes;
    VIDEOI_BrPixelmapGetTypeDetails(dest->pm_type, &internalFormat, &format, &type, &elemBytes, NULL);

    glReadPixels(0, 0, self->pm_width, self->pm_height, format, type, dest->pm_pixels);

    void *rowTemp = BrScratchAllocate(dest->pm_row_bytes);

    /* Flip it */
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
                                                 char *text, br_uint_32 colour)
{
    /* Quit if off top, bottom or right screen */
    br_int_32 x = point->x + self->pm_origin_x;
    br_int_32 y = point->y + self->pm_origin_y;

    if(y <= -font->glyph_y || y >= self->pm_height || x >= self->pm_width)
        return BRE_OK;

    /* Ensure we're a valid font. */
    br_font_gl *gl_font;
    if(font == BrFontFixed3x5)
        gl_font = &self->device->font_fixed3x5;
    else if(font == BrFontProp4x6)
        gl_font = &self->device->font_prop4x6;
    else if(font == BrFontProp7x9)
        gl_font = &self->device->font_prop7x9;
    else
        return BRE_FAIL;

    /* Set up the render state. The font UVs match the texture, so no need to flip here. */
    HVIDEO hVideo = &self->device->video;

    glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
    glViewport(0, 0, self->pm_width, self->pm_height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glUseProgram(hVideo->textProgram.program);

    br_vector3_f col = {{
                            BR_RED(colour) / 255.0f,
                            BR_GRN(colour) / 255.0f,
                            BR_BLU(colour) / 255.0f
                        }};

    br_matrix4 mvp;
    BrMatrix4Orthographic(&mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    VIDEOI_D3DtoGLProjection(&mvp);

    glUniformMatrix4fv(hVideo->textProgram.uMVP, 1, GL_FALSE, (GLfloat *)&mvp);
    glActiveTexture(GL_TEXTURE0);

    if(gl_font->tex != 0)
        glBindTexture(GL_TEXTURE_2D, gl_font->tex);
    else
        glBindTexture(GL_TEXTURE_2D, self->device->tex_checkerboard);

    glUniform1i(hVideo->textProgram.uSampler, 0);
    glUniform3fv(hVideo->textProgram.uColour, 1, col.v);

    br_rectangle r = {.x = point->x, .y = point->y, .h = font->glyph_y, .w = 0,};

    for(; *text && r.w <= self->pm_width; ++text) {
        br_uint_8  glyph = (br_uint_8)*text;
        br_uint_16 width = (font->flags & BR_FONTF_PROPORTIONAL) ? font->width[glyph] : font->glyph_x;
        r.w = width;

        br_rectangle dr = r;
        VIDEOI_BrRectToGL((br_pixelmap *)self, &dr);

        float x0, y0, x1, y1;
        VIDEOI_BrRectToUVs((br_pixelmap *)self, &dr, &x0, &y0, &x1, &y1);

        actually_patch_quad(&self->asBack.quad,
                            x0, y0, x1, y1,
                            gl_font->glyph[glyph].x0,
                            gl_font->glyph[glyph].y0,
                            gl_font->glyph[glyph].x1,
                            gl_font->glyph[glyph].y1
        );

        glBindVertexArray(self->asBack.quad.textVao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

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
    .__reserved0                = NULL,
    .__reserved1                = NULL,
    .__reserved2                = NULL,
    .__reserved3                = NULL,
    ._free                      = BR_CMETHOD_REF(br_device_pixelmap_gl, free),
    ._identifier                = BR_CMETHOD_REF(br_device_pixelmap_gl, identifier),
    ._type                      = BR_CMETHOD_REF(br_device_pixelmap_gl, type),
    ._isType                    = BR_CMETHOD_REF(br_device_pixelmap_gl, isType),
    ._device                    = BR_CMETHOD_REF(br_device_pixelmap_gl, device),
    ._space                     = BR_CMETHOD_REF(br_device_pixelmap_gl, space),

    ._templateQuery             = BR_CMETHOD_REF(br_device_pixelmap_gl, templateQuery),
    ._query                     = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer               = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany                 = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize             = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll                  = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize              = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource               = BR_CMETHOD_REF(br_device_pixelmap_null, validSource),
    ._resize                    = BR_CMETHOD_REF(br_device_pixelmap_gl, resize),
    ._match                     = BR_CMETHOD_REF(br_device_pixelmap_gl, match),
    ._allocateSub               = BR_CMETHOD_REF(br_device_pixelmap_fail, allocateSub),

    ._copy                      = BR_CMETHOD_REF(br_device_pixelmap_gen, copy),
    ._copyTo                    = BR_CMETHOD_REF(br_device_pixelmap_gen, copyTo),
    ._copyFrom                  = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFrom),
    ._fill                      = BR_CMETHOD_REF(br_device_pixelmap_gen, fill),
    ._doubleBuffer              = BR_CMETHOD_REF(br_device_pixelmap_gl, doubleBuffer),

    ._copyDirty                 = BR_CMETHOD_REF(br_device_pixelmap_gen, copyDirty),
    ._copyToDirty               = BR_CMETHOD_REF(br_device_pixelmap_gen, copyToDirty),
    ._copyFromDirty             = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFromDirty),
    ._fillDirty                 = BR_CMETHOD_REF(br_device_pixelmap_gen, fillDirty),
    ._doubleBufferDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBufferDirty),

    ._rectangle                 = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle),
    ._rectangle2                = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle2),
    ._rectangleCopy             = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleCopy),
    ._rectangleCopyTo           = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleCopyTo),
    ._rectangleCopyFrom         = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleCopyFrom),
    ._rectangleStretchCopy      = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleStretchCopy),
    ._rectangleStretchCopyTo    = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom  = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill             = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleFill),
    ._pixelSet                  = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelSet),
    ._line                      = BR_CMETHOD_REF(br_device_pixelmap_fail, line),
    ._copyBits                  = BR_CMETHOD_REF(br_device_pixelmap_fail, copyBits),

    ._text                      = BR_CMETHOD_REF(br_device_pixelmap_gl, text),
    ._textBounds                = BR_CMETHOD_REF(br_device_pixelmap_gen, textBounds),

    ._rowSize                   = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSize),
    ._rowQuery                  = BR_CMETHOD_REF(br_device_pixelmap_fail, rowQuery),
    ._rowSet                    = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSet),

    ._pixelQuery                = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelQuery),
    ._pixelAddressQuery         = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressQuery),

    ._pixelAddressSet           = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressSet),
    ._originSet                 = BR_CMETHOD_REF(br_device_pixelmap_gen, originSet),

    ._flush                     = BR_CMETHOD_REF(br_device_pixelmap_fail, flush),
    ._synchronise               = BR_CMETHOD_REF(br_device_pixelmap_fail, synchronise),
    ._directLock                = BR_CMETHOD_REF(br_device_pixelmap_fail, directLock),
    ._directUnlock              = BR_CMETHOD_REF(br_device_pixelmap_fail, directUnlock),
    ._getControls               = BR_CMETHOD_REF(br_device_pixelmap_fail, getControls),
    ._setControls               = BR_CMETHOD_REF(br_device_pixelmap_fail, setControls),
};

