/*
 * Subpixelmap methods
 */

#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_pixelmap_dispatch devicePixelmapSubDispatch;

/*
 * Device pixelmap info. template
 */
#define F(f) offsetof(struct br_device_pixelmap, f)
static struct br_tv_template_entry devicePixelmapSubTemplateEntries[] = {
    {BRT(WIDTH_I32),       F(pm_width),      BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0},
    {BRT(HEIGHT_I32),      F(pm_height),     BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0},
    {BRT(PIXEL_TYPE_U8),   F(pm_type),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8,  0},
    {BRT(IDENTIFIER_CSTR), F(pm_identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
};
#undef F

static br_point map_to_parent_point(br_device_pixelmap *restrict self, br_device_pixelmap *restrict parent, br_point p)
{
    /* Remap us back to top-left at (0, 0). */
    p.x = p.x + self->pm_origin_x;
    p.y = p.y + self->pm_origin_y;

    /* Map to our parent's (0, 0) by undoing the base. */
    p.x = p.x + self->pm_base_x;
    p.y = p.y + self->pm_base_y;

    /* Map to our parent's space. */
    p.x = p.x - parent->pm_origin_x;
    p.y = p.y - parent->pm_origin_y;

    return p;
}

static br_rectangle map_to_parent_rect(br_device_pixelmap *self, br_device_pixelmap *parent, br_rectangle rect)
{
    br_point p = {.x = rect.x, .y = rect.y};
    p          = map_to_parent_point(self, parent, p);

    return (br_rectangle){
        .x = p.x,
        .y = p.y,
        .w = rect.w,
        .h = rect.h,
    };
}

static void BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    UASSERT(self->num_refs == 0);

    BrLogTrace("GLREND", "Freeing %s", self->pm_identifier);

    DevicePixelmapGLDecRef(self->asSub.parent);

    ObjectContainerRemove(self->output_facility, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, device)(br_object *self)
{
    (void)self;
    return ((br_device_pixelmap *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device_pixelmap);
}

static br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapTemplate == NULL)
        self->device->templates.devicePixelmapSubTemplate = BrTVTemplateAllocate(self->device, devicePixelmapSubTemplateEntries,
                                                                                 BR_ASIZE(devicePixelmapSubTemplateEntries));

    return self->device->templates.devicePixelmapSubTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    br_point p = {
        .x = -self->pm_origin_x,
        .y = -self->pm_origin_y,
    };

    p = map_to_parent_point(self, self->asSub.parent, p);

    br_rectangle rect = {
        .x = p.x,
        .y = p.y,
        .w = width,
        .h = height,
    };
    br_rectangle out;

    if(PixelmapRectangleClip(&out, &rect, (br_pixelmap *)self->asSub.parent) == BR_CLIP_REJECT)
        return BRE_FAIL;

    self->pm_width  = out.w;
    self->pm_height = out.h;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, rectangleCopy)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                          br_rectangle *sr)
{
    br_point dpoint = map_to_parent_point(self, self->asSub.parent, *p);
    return DevicePixelmapRectangleCopy(self->asSub.parent, &dpoint, src, sr);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    br_rectangle drect = map_to_parent_rect(self, self->asSub.parent, *rect);
    return DevicePixelmapRectangleFill(self->asSub.parent, &drect, colour);
}

static br_error BR_CMETHOD(br_device_pixelmap_sub_gl, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *d,
                                                                              br_device_pixelmap *src, br_rectangle *s)
{
    br_rectangle drect = map_to_parent_rect(self, self->asSub.parent, *d);
    return DevicePixelmapRectangleStretchCopyTo(self->asSub.parent, &drect, src, s);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, rectangleCopyTo)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                            br_rectangle *sr)
{
    br_point dpoint = map_to_parent_point(self, self->asSub.parent, *p);
    return DevicePixelmapRectangleCopyTo(self->asSub.parent, &dpoint, src, sr);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p,
                                                                              br_device_pixelmap *dest, br_rectangle *r)
{
    br_point spoint = map_to_parent_point(self, self->asSub.parent, *p);
    return DevicePixelmapRectangleCopyFrom(self->asSub.parent, &spoint, dest, r);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, rectangleStretchCopy)(br_device_pixelmap *self, br_rectangle *d,
                                                                                 br_device_pixelmap *src, br_rectangle *s)
{
    br_rectangle drect = map_to_parent_rect(self, self->asSub.parent, *d);
    return DevicePixelmapRectangleStretchCopy(self->asSub.parent, &drect, src, s);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, line)(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
    br_point spoint = map_to_parent_point(self, self->asSub.parent, *s);
    br_point epoint = map_to_parent_point(self, self->asSub.parent, *e);
    return DevicePixelmapLine(self->asSub.parent, &spoint, &epoint, colour);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, text)(br_device_pixelmap *self, br_point *point, br_font *font, const char *text,
                                                                 br_uint_32 colour)
{
    br_point p = map_to_parent_point(self, self->asSub.parent, *point);
    return DevicePixelmapText(self->asSub.parent, &p, font, text, colour);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_gl, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_rectangle *rect)
{
    br_rectangle drect = map_to_parent_rect(self, self->asSub.parent, *rect);
    return DevicePixelmapAllocateSub(self->asSub.parent, newpm, &drect);
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_rectangle *rect)
{
    br_rectangle        out;
    br_device_pixelmap *pm;

    /*
     * Only allow sub-pixelmap backbuffers.
     */
    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    /*
     * Create sub-window (clipped against original)
     */
    if(PixelmapRectangleClip(&out, rect, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_FAIL;

    /*
     * Create the new structure and copy
     */
    pm = BrResAllocate(self->device->res, sizeof(*pm), BR_MEMORY_OBJECT);

    *pm = *self;

    pm->dispatch = &devicePixelmapSubDispatch;

    pm->pm_identifier = BrResSprintf(pm, "%s:submap:%dx%d", self->pm_identifier, out.w, out.h);

    pm->pm_base_x += (br_uint_16)out.x;
    pm->pm_base_y += (br_uint_16)out.y;
    pm->pm_width  = (br_uint_16)out.w;
    pm->pm_height = (br_uint_16)out.h;

    pm->pm_origin_x = 0;
    pm->pm_origin_y = 0;

    pm->pm_stored = NULL;

    /*
     * Pixel rows may not be contiguous
     */
    if(self->pm_width != pm->pm_width)
        pm->pm_flags &= ~BR_PMF_LINEAR;

    pm->asSub.parent = self;
    pm->num_refs     = 0;

    DevicePixelmapGLIncRef(self);

    *newpm = pm;

    return BRE_OK;
}

/*
 * Default dispatch table for device pixelmap
 */
static const struct br_device_pixelmap_dispatch devicePixelmapSubDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_null, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, allocateSub),

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
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, rectangleCopy),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, rectangleStretchCopy),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_fail, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_sub_gl, text),
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
