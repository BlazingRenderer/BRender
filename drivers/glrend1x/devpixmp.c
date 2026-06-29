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

    if(tep->token == BRT_CLUT_O) {
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
    {BRT(WIDTH_I32),           F(pm_width),          BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(HEIGHT_I32),          F(pm_height),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(PIXEL_TYPE_U8),       F(pm_type),           BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8,  0                    },
    {BRT(OUTPUT_FACILITY_O),   F(output_facility),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(FACILITY_O),          F(output_facility),   BRTV_QUERY,            BRTV_CONV_COPY,    0                    },
    {BRT(PRIMITIVE_LIBRARY_O), F(primitive_library), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(CLUT_O),              0,                    BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(IDENTIFIER_CSTR),     F(pm_identifier),     BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
};
#undef F

static void prepareRectRenderState(br_device_pixelmap *self)
{
    const GladGLContext *gl = DevicePixelmapGL1xGetGLContext(self);

    gl->MatrixMode(GL_PROJECTION);
    {
        gl->PushMatrix();
        gl->LoadIdentity();
        gl->Ortho(0.0, (double)self->pm_width, (double)self->pm_height, 0.0, -1.0, 1.0);
    }

    gl->MatrixMode(GL_MODELVIEW);
    {
        gl->PushMatrix();
        gl->LoadIdentity();
    }

    gl->PushAttrib(GL_ALL_ATTRIB_BITS);

    gl->Viewport(0, 0, self->pm_width, self->pm_height);

    gl->Enable(GL_BLEND);
    gl->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl->Disable(GL_DEPTH_TEST);
    gl->Disable(GL_CULL_FACE);
    gl->Disable(GL_TEXTURE_2D);
    gl->DepthMask(GL_FALSE);

    gl->ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

static void finishRectState(br_device_pixelmap *self)
{
    const GladGLContext *gl = DevicePixelmapGL1xGetGLContext(self);

    gl->PopAttrib();

    gl->MatrixMode(GL_MODELVIEW);
    gl->PopMatrix();

    gl->MatrixMode(GL_PROJECTION);
    gl->PopMatrix();
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    self->pm_width  = width;
    self->pm_height = height;
    return BRE_OK;
}

static void BR_CMETHOD_DECL(br_device_pixelmap_gl, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    UASSERT(self->num_refs == 0);

    BrLogTrace("GLREND", "Freeing %s", self->pm_identifier);

    ObjectContainerRemove(self->output_facility, (br_object *)self);

    DevicePixelmapGL1xDecRef(self->screen);

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
static br_tv_template_entry pixelmapMatchTemplateEntries[] = {
    {BRT_WIDTH_I32,      NULL, F(width),      BRTV_SET, BRTV_CONV_COPY},
    {BRT_HEIGHT_I32,     NULL, F(height),     BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_BITS_I32, NULL, F(pixel_bits), BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_TYPE_U8,  NULL, F(type),       BRTV_SET, BRTV_CONV_COPY},
    {BRT_USE_T,          NULL, F(use_type),   BRTV_SET, BRTV_CONV_COPY},
};
#undef F

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl1x, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv)
{
    br_int_32                  count;
    br_error                   err;
    br_device_pixelmap        *pm;
    const char                *typestring;
    const br_pixelmap_gl_fmt  *fmt;
    const GladGLContext       *gl;
    struct pixelmapMatchTokens mt = {
        .width      = self->pm_width,
        .height     = self->pm_height,
        .pixel_bits = -1,
        .type       = BR_PMT_MAX,
        .use_type   = BRT_NONE,
    };

    gl = DevicePixelmapGL1xGetGLContext(self);

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
            /*
             * Only the front screen (use_type == BRT_NONE) can create an
             * offscreen backbuffer.  GL 1.x has no FBOs, so there is
             * exactly one backbuffer and one depth buffer.
             */
            if(self->use_type != BRT_NONE)
                return BRE_UNSUPPORTED;

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

    if((fmt = DeviceGL1xGetFormatDetails(mt.type)) == NULL)
        return BRE_FAIL;

    /*
     * Refuse creation of indexed pixelmaps.
     */
    if(fmt->indexed)
        return BRE_FAIL;

    pm                    = BrResAllocate(self->device, sizeof(br_device_pixelmap), BR_MEMORY_OBJECT);
    pm->dispatch          = &devicePixelmapDispatch;
    pm->pm_identifier     = BrResSprintf(pm, BR_GLREND_DEBUG_USER_PREFIX "%s:%dx%d", typestring, mt.width, mt.height);
    pm->device            = self->device;
    pm->output_facility   = self->output_facility;
    pm->primitive_library = self->primitive_library;
    pm->use_type          = mt.use_type;
    pm->screen            = self->screen;
    DevicePixelmapGL1xIncRef(self->screen);

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
        pm->asBack.clut        = DeviceClutGL1xAllocate(pm);
    } else {
        UASSERT(mt.use_type == BRT_DEPTH);
        self->asBack.depthbuffer = pm;
        pm->asDepth.backbuffer   = self;
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

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    GLbitfield           mask;
    br_rectangle         drect;
    const GladGLContext *gl = DevicePixelmapGL1xGetGLContext(self);

    /*
     * Clip rectangle to pixelmap
     */
    if(PixelmapRectangleClip(&drect, rect, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    drect.y = self->pm_height - drect.h - drect.y;

    if(self->use_type == BRT_OFFSCREEN) {
        br_uint_8 r8 = 0, g8 = 0, b8 = 0, a8 = 255;
        BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);

        mask = GL_COLOR_BUFFER_BIT;

        gl->ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        gl->ClearColor((float)r8 / 255.0f, (float)g8 / 255.0f, (float)b8 / 255.0f, (float)a8 / 255.0f);
    } else if(self->use_type == BRT_DEPTH) {
        UASSERT(colour == 0xFFFFFFFF);
        mask = GL_DEPTH_BUFFER_BIT;
        gl->DepthMask(GL_TRUE);
        gl->ClearDepth(1.0f);
    } else {
        return BRE_UNSUPPORTED;
    }

    gl->Viewport(0, 0, self->pm_width, self->pm_height);

    gl->Enable(GL_SCISSOR_TEST);
    gl->Scissor(drect.x, drect.y, drect.w, drect.h);

    gl->Clear(mask);

    gl->Disable(GL_SCISSOR_TEST);

    return BRE_OK;
}

static br_error BR_CMETHOD(br_device_pixelmap_gl, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *d,
                                                                          br_device_pixelmap *_src, br_rectangle *s)
{
    /* Pixelmap->Device, addressable stretch copy. */

    const GladGLContext *gl     = DevicePixelmapGL1xGetGLContext(self);
    br_pixelmap         *src    = (br_pixelmap *)_src;
    br_buffer_stored    *stored = src->stored;
    br_rectangle         srect, drect;
    br_boolean           tex_tmp = BR_FALSE;
    GLuint               tex;

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if(PixelmapRectangleClip(&srect, s, src) == BR_CLIP_REJECT)
        return BRE_OK;

    if(PixelmapRectangleClip(&drect, d, (const br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if(stored != NULL && ObjectDevice(stored) == self->device) {
        tex = BufferStoredGL1xGetTexture(stored, BR_FALSE);
    } else if(src->type == BR_PMT_INDEX_8 && src->map == NULL) {

        /*
         * Indexed source has no palette - inherit the destination's CLUT.
         */
        if(self->asBack.clut == NULL)
            return BRE_FAIL;

        src->map = self->asBack.clut->pm;
        tex      = DeviceGL1xPixelmapToGLTexture(gl, src);
        src->map = NULL;
        tex_tmp  = BR_TRUE;
    } else {
        tex     = DeviceGL1xPixelmapToGLTexture(gl, src);
        tex_tmp = BR_TRUE;
    }

    if(tex == 0) {
        if(tex_tmp)
            gl->DeleteTextures(1, &tex);
        return BRE_FAIL;
    }

    prepareRectRenderState(self);

    {
        float u0 = (float)srect.x / (float)src->width;
        float u1 = (float)(srect.x + srect.w) / (float)src->width;
        float v0 = (float)srect.y / (float)src->height;
        float v1 = (float)(srect.y + srect.h) / (float)src->height;

        gl->Enable(GL_TEXTURE_2D);
        gl->BindTexture(GL_TEXTURE_2D, tex);
        gl->Begin(GL_QUADS);
        {
            gl->TexCoord2f(u0, v0);
            gl->Vertex3i(drect.x, drect.y, 0);

            gl->TexCoord2f(u1, v0);
            gl->Vertex3i(drect.x + drect.w, drect.y, 0);

            gl->TexCoord2f(u1, v1);
            gl->Vertex3i(drect.x + drect.w, drect.y + drect.h, 0);

            gl->TexCoord2f(u0, v1);
            gl->Vertex3i(drect.x, drect.y + drect.h, 0);
        }
        gl->End();
        gl->Disable(GL_TEXTURE_2D);
    }

    finishRectState(self);

    if(tex_tmp)
        gl->DeleteTextures(1, &tex);
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
    void                     *row_temp;
    const br_pixelmap_gl_fmt *fmt;
    br_rectangle              srect;
    br_point                  dpoint;
    br_uint_16                bytes_per_pixel;
    void                     *dst_pixels;
    const GladGLContext      *gl = DevicePixelmapGL1xGetGLContext(self);

    /*
     * Need contig pixels and whole rows.
     */
    if(!(dest->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)))
        return BRE_FAIL;

    if(PixelmapRectangleClipTwo(&srect, &dpoint, r, p, (br_pixelmap *)dest, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    if((fmt = DeviceGL1xGetFormatDetails(dest->pm_type)) == NULL)
        return BRE_FAIL;

    bytes_per_pixel = BrPixelmapPixelSize((br_pixelmap *)dest) >> 3;
    dst_pixels      = DevicePixelmapMemAddress(dest, dpoint.x, dpoint.y, bytes_per_pixel);

    gl->PixelStorei(GL_PACK_ROW_LENGTH, srect.w);
    gl->ReadPixels(srect.x, self->pm_height - srect.y - srect.h, srect.w, srect.h, fmt->format, fmt->type, dst_pixels);

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

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, line)(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
    br_point             spoint, epoint;
    br_uint_8            r8 = 0, g8 = 0, b8 = 0, a8 = 255;
    const GladGLContext *gl = DevicePixelmapGL1xGetGLContext(self);

    if(PixelmapLineClip(&spoint, &epoint, s, e, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);

    prepareRectRenderState(self);
    gl->Begin(GL_LINES);
    {
        gl->Color4ub(r8, g8, b8, a8);
        gl->Vertex3i(spoint.x, spoint.y, 0);
        gl->Vertex3i(epoint.x, epoint.y, 0);
    }
    gl->End();
    finishRectState(self);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, copyBits)(br_device_pixelmap *self, br_point *point, const br_uint_8 *src,
                                                                 br_uint_16 s_stride, br_rectangle *bit_rect, br_uint_32 colour)
{
    br_uint_8            r8 = 0, g8 = 0, b8 = 0, a8 = 255;
    const GladGLContext *gl = DevicePixelmapGL1xGetGLContext(self);

    BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);

    prepareRectRenderState(self);

    gl->Begin(GL_POINTS);
    gl->Color4ub(r8, g8, b8, a8);

    for(br_int_32 y = bit_rect->y; y < bit_rect->y + bit_rect->h; y++) {
        for(br_int_32 x = bit_rect->x; x < bit_rect->x + bit_rect->w; x++) {
            if(src[(y * s_stride) + (x / 8)] & 0x80 >> (x % 8)) {
                br_point in_p = {.x = point->x + x, .y = point->y + y};
                br_point out_p;

                if(PixelmapPointClip(&out_p, &in_p, (br_pixelmap *)self) == BR_CLIP_REJECT)
                    continue;

                gl->Vertex3i(out_p.x - 1, out_p.y - 1, 0);
            }
        }
    }
    gl->End();

    finishRectState(self);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    br_point             point;
    br_uint_8            r8 = 0, g8 = 0, b8 = 0, a8 = 255;
    const GladGLContext *gl = DevicePixelmapGL1xGetGLContext(self);

    if(PixelmapPointClip(&point, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    BrColourUnpack(colour, self->pm_type, &r8, &g8, &b8, &a8);

    prepareRectRenderState(self);

    gl->Begin(GL_POINTS);
    {
        /*
         * FIXME: For some reason I cbf figuring out now, when rendering a point, both components need a -1.
         */
        gl->Color4ub(r8, g8, b8, a8);
        gl->Vertex3i(point.x - 1, point.y - 1, 0);
    }
    gl->End();

    finishRectState(self);
    return BRE_OK;
}

void DevicePixelmapGL1xIncRef(br_device_pixelmap *self)
{
    UASSERT(self->num_refs >= 0);
    ++self->num_refs;
}

void DevicePixelmapGL1xDecRef(br_device_pixelmap *self)
{
    UASSERT(self->num_refs > 0);
    --self->num_refs;
}

br_rectangle DevicePixelmapGL1xGetViewport(const br_device_pixelmap *pm)
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

const GladGLContext *DevicePixelmapGL1xGetGLContext(br_device_pixelmap *self)
{
    return &self->screen->asFront.glad_gl_context;
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
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_gl1x, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_gl1x, allocateSub),

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
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleCopyTo),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyTo),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_gl, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_gl, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_gl, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_gl, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_gen, text),
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
