/*
 * Device pixelmap methods
 */

#include <string.h>
#include "drv.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_pixelmap_dispatch devicePixelmapDispatch;

/*
 * Device pixelmap info. template
 */
#define F(f) offsetof(struct br_device_pixelmap, f)
static br_tv_template_entry devicePixelmapTemplateEntries[] = {
    {BRT(WIDTH_I32),         F(pm_width),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0},
    {BRT(HEIGHT_I32),        F(pm_height),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0},
    {BRT(PIXEL_TYPE_U8),     F(pm_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8,  0},
    {BRT(OUTPUT_FACILITY_O), F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
    {BRT(FACILITY_O),        F(output_facility), BRTV_QUERY,            BRTV_CONV_COPY,    0},
    {BRT(IDENTIFIER_CSTR),   F(pm_identifier),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
};
#undef F

static void BR_CMETHOD_DECL(br_device_pixelmap_vk, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    ObjectContainerRemove(self->output_facility, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_pixelmap_vk, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_vk, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_vk, device)(br_object *self)
{
    (void)self;
    return ((br_device_pixelmap *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_vk, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device_pixelmap);
}

static br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_vk, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapTemplate == NULL) {
        self->device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(self->device, devicePixelmapTemplateEntries,
                                                                              BR_ASIZE(devicePixelmapTemplateEntries));
    }

    return self->device->templates.devicePixelmapTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    return BRE_OK;
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

br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv)
{
    br_int_32                  count;
    br_error                   err;
    br_device_pixelmap        *pm;
    struct pixelmapMatchTokens mt = {
        .width      = self->pm_width,
        .height     = self->pm_height,
        .pixel_bits = -1,
        .type       = BR_PMT_MAX,
        .use_type   = BRT_NONE,
    };

    if(self->device->templates.pixelmapMatchTemplate == NULL) {
        self->device->templates.pixelmapMatchTemplate = BrTVTemplateAllocate(self->device, pixelmapMatchTemplateEntries,
                                                                             BR_ASIZE(pixelmapMatchTemplateEntries));
    }

    err = BrTokenValueSetMany(&mt, &count, NULL, tv, self->device->templates.pixelmapMatchTemplate);
    if(err != BRE_OK)
        return err;

    pm                  = BrResAllocate(self->device, sizeof(br_device_pixelmap), BR_MEMORY_OBJECT);
    pm->dispatch        = &devicePixelmapDispatch;
    pm->device          = self->device;
    pm->output_facility = self->output_facility;
    pm->use_type        = mt.use_type;
    pm->screen          = self->screen;

    pm->pm_type     = mt.type;
    pm->pm_width    = mt.width;
    pm->pm_height   = mt.height;
    pm->pm_flags    = BR_PMF_NO_ACCESS;
    pm->pm_origin_x = 0;
    pm->pm_origin_y = 0;
    pm->pm_base_x   = 0;
    pm->pm_origin_y = 0;

    /*
     * Copy origin over.
     */
    pm->pm_origin_x = self->pm_origin_x;
    pm->pm_origin_y = self->pm_origin_y;

    *newpm = pm;
    ObjectContainerAddFront(self->output_facility, (br_object *)pm);
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_rectangle *rect)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleStretchCopy)(br_device_pixelmap *self, br_rectangle *d,
                                                                             br_device_pixelmap *src, br_rectangle *s)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleCopy)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                      br_rectangle *sr)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD(br_device_pixelmap_vk, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *d,
                                                                          br_device_pixelmap *_src, br_rectangle *s)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleCopyTo)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                        br_rectangle *sr)
{
    return BRE_FAIL;
}

/*
 * Device->Pixelmap, addressable same-size copy.
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *dest,
                                                                          br_rectangle *r)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD(br_device_pixelmap_vk, text)(br_device_pixelmap *self, br_point *point, br_font *font, const char *text, br_uint_32 colour)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, line)(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    return BRE_FAIL;
}

/*
 * Default dispatch table for device pixelmap
 */
static const struct br_device_pixelmap_dispatch devicePixelmapDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_vk, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_vk, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_vk, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_vk, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_vk, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_vk, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_vk, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_null, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_vk, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_vk, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_vk, allocateSub),

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
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleCopy),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleStretchCopy),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_vk, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_vk, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_fail, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_vk, text),
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
