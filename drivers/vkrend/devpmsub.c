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
static br_tv_template_entry devicePixelmapSubTemplateEntries[] = {
    {BRT(WIDTH_I32),       F(pm_width),      BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0},
    {BRT(HEIGHT_I32),      F(pm_height),     BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0},
    {BRT(PIXEL_TYPE_U8),   F(pm_type),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8,  0},
    {BRT(IDENTIFIER_CSTR), F(pm_identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
};
#undef F

/*
 * Whether a pixelmap carries the sub-pixelmap dispatch. The dispatch is file-local, so
 * DevicePixelmapVkResolve() (devpixmp.c) asks this rather than comparing to it again. A
 * sub's as_sub member aliases as_back, so the owner it points at is the only valid image.
 */
br_boolean DevicePixelmapVkIsSub(const br_device_pixelmap *pm)
{
    return pm->dispatch == &devicePixelmapSubDispatch;
}

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

static void BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    UASSERT(self->num_refs == 0);

    BrLogTrace("VKREND", "Freeing %s", self->pm_identifier);

    DevicePixelmapVkDecRef(self->as_sub.parent);

    ObjectContainerRemove(self->output_facility, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, device)(br_object *self)
{
    (void)self;
    return ((br_device_pixelmap *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device_pixelmap);
}

static br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapSubTemplate == NULL)
        self->device->templates.devicePixelmapSubTemplate = BrTVTemplateAllocate(self->device, devicePixelmapSubTemplateEntries,
                                                                                 BR_ASIZE(devicePixelmapSubTemplateEntries));

    return self->device->templates.devicePixelmapSubTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    br_point p = {
        .x = -self->pm_origin_x,
        .y = -self->pm_origin_y,
    };

    p = map_to_parent_point(self, self->as_sub.parent, p);

    br_rectangle rect = {
        .x = p.x,
        .y = p.y,
        .w = width,
        .h = height,
    };
    br_rectangle out;

    if(PixelmapRectangleClip(&out, &rect, (br_pixelmap *)self->as_sub.parent) == BR_CLIP_REJECT)
        return BRE_FAIL;

    self->pm_width  = out.w;
    self->pm_height = out.h;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, rectangleCopy)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                          br_rectangle *sr)
{
    br_point dpoint = map_to_parent_point(self, self->as_sub.parent, *p);
    return DevicePixelmapRectangleCopy(self->as_sub.parent, &dpoint, src, sr);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    br_rectangle drect = map_to_parent_rect(self, self->as_sub.parent, *rect);
    return DevicePixelmapRectangleFill(self->as_sub.parent, &drect, colour);
}

static br_error BR_CMETHOD(br_device_pixelmap_sub_vk, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *d,
                                                                              br_device_pixelmap *src, br_rectangle *s)
{
    br_rectangle drect = map_to_parent_rect(self, self->as_sub.parent, *d);
    return DevicePixelmapRectangleStretchCopyTo(self->as_sub.parent, &drect, src, s);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, rectangleCopyTo)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                            br_rectangle *sr)
{
    br_point dpoint = map_to_parent_point(self, self->as_sub.parent, *p);
    return DevicePixelmapRectangleCopyTo(self->as_sub.parent, &dpoint, src, sr);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p,
                                                                              br_device_pixelmap *dest, br_rectangle *r)
{
    br_point spoint = map_to_parent_point(self, self->as_sub.parent, *p);
    return DevicePixelmapRectangleCopyFrom(self->as_sub.parent, &spoint, dest, r);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, rectangleStretchCopy)(br_device_pixelmap *self, br_rectangle *d,
                                                                                 br_device_pixelmap *src, br_rectangle *s)
{
    br_rectangle drect = map_to_parent_rect(self, self->as_sub.parent, *d);
    return DevicePixelmapRectangleStretchCopy(self->as_sub.parent, &drect, src, s);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, line)(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
    br_point spoint = map_to_parent_point(self, self->as_sub.parent, *s);
    br_point epoint = map_to_parent_point(self, self->as_sub.parent, *e);
    return DevicePixelmapLine(self->as_sub.parent, &spoint, &epoint, colour);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    br_point point = map_to_parent_point(self, self->as_sub.parent, *p);
    return DevicePixelmapPixelSet(self->as_sub.parent, &point, colour);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, text)(br_device_pixelmap *self, br_point *point, br_font *font, const char *text,
                                                                 br_uint_32 colour)
{
    br_point p = map_to_parent_point(self, self->as_sub.parent, *point);
    return DevicePixelmapText(self->as_sub.parent, &p, font, text, colour);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_sub_vk, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_rectangle *rect)
{
    br_rectangle drect = map_to_parent_rect(self, self->as_sub.parent, *rect);
    return DevicePixelmapAllocateSub(self->as_sub.parent, newpm, &drect);
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_rectangle *rect)
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

    pm->as_sub.parent = self;
    pm->num_refs      = 0;

    DevicePixelmapVkIncRef(self);

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
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_null, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, allocateSub),

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
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, rectangleCopy),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, rectangleStretchCopy),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_fail, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_sub_vk, text),
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
