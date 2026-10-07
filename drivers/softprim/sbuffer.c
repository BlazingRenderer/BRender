/*
 * softprim stored buffer methods
 *
 * A stored buffer is only ever used here as an opaque handle that tells
 * softrend that a map of a given shape was bound. Stage 1 does not sample any
 * of them, so this stays deliberately thin.
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Default dispatch table for stored buffer (defined at end of file)
 */
static const struct br_buffer_stored_dispatch bufferStoredDispatch;

/*
 * Stored buffer info. template
 */
#define F(f) offsetof(struct br_buffer_stored, f)

static struct br_tv_template_entry bufferStoredTemplateEntries[] = {
    {
     BRT_IDENTIFIER_CSTR, 0,
     F(identifier),
     BRTV_QUERY | BRTV_ALL,
     BRTV_CONV_COPY, },
};
#undef F

/*
 * Bytes per pixel for the formats softprim understands. Anything else is
 * treated as one byte per pixel; such buffers are never sampled in stage 1.
 */
static br_int_32 softprim_pixel_bytes(br_uint_8 type)
{
    switch(type) {
        case BR_PMT_RGB_555:
        case BR_PMT_RGB_565:
        case BR_PMT_DEPTH_16:
            return 2;

        case BR_PMT_RGB_888:
            return 3;

        case BR_PMT_RGBX_888:
        case BR_PMT_RGBA_8888:
        case BR_PMT_DEPTH_32:
            return 4;

        default:
            return 1;
    }
}

/*
 * Build a softprim_buffer structure from a pixelmap
 */
void SoftPrimSetupBuffer(softprim_buffer *rb, br_device_pixelmap *pm)
{
    br_int_32 bpp;

    if(pm == NULL) {
        BrMemSet(rb, 0, sizeof(*rb));
        return;
    }

    bpp = softprim_pixel_bytes(pm->pm_type);

    rb->type     = pm->pm_type;
    rb->width_p  = pm->pm_width;
    rb->height   = pm->pm_height;
    rb->stride_b = pm->pm_row_bytes;
    rb->bpp      = bpp;
    rb->base     = (br_uint_8 *)pm->pm_pixels + pm->pm_base_y * pm->pm_row_bytes + pm->pm_base_x * bpp;

    /*
     * A colour map is a one-pixel-wide pixelmap. Its entry type is the map's
     * own type, so an INDEX_8 texture sampled into an RGB output decodes
     * through whatever palette the bound map carries (pentprim inherits the
     * screen palette when the texture has none - match.c does that here).
     */
    rb->palette          = NULL;
    rb->palette_size     = 0;
    rb->palette_type     = 0;
    rb->palette_entry_b  = 0;
    rb->palette_stride_b = 0;
    rb->palette_stride_p = 0;

    if(pm->pm_map != NULL && pm->pm_map->width == 1 && pm->pm_map->height > 0) {
        br_int_32 entry_b = softprim_pixel_bytes(pm->pm_map->type);

        rb->palette          = (const br_uint_8 *)pm->pm_map->pixels;
        rb->palette_size     = pm->pm_map->height;
        rb->palette_type     = pm->pm_map->type;
        rb->palette_entry_b  = entry_b;
        rb->palette_stride_b = pm->pm_map->row_bytes;
        rb->palette_stride_p = (entry_b > 0) ? (pm->pm_map->row_bytes / entry_b) : 0;
    }
}

/*
 * Set up a stored buffer object
 */
struct br_buffer_stored *BufferStoredSoftPrimAllocate(struct br_primitive_library *plib, br_token use, struct br_device_pixelmap *pm,
                                                     br_token_value *tv)
{
    struct br_buffer_stored *self;
    char                    *ident;

    (void)tv;

    switch(use) {
        case BRT_TEXTURE_O:
        case BRT_COLOUR_MAP_O:
            ident = "Colour-Map";
            break;
        case BRT_INDEX_SHADE_O:
            ident = "Shade-Table";
            break;
        case BRT_INDEX_BLEND_O:
            ident = "Blend-Table";
            break;
        case BRT_SCREEN_DOOR_O:
            ident = "Screendoor-Table";
            break;
        case BRT_INDEX_LIGHT_O:
            ident = "Lighting-Table";
            break;
        case BRT_BUMP_O:
            ident = "Bump-Map";
            break;

        case BRT_UNKNOWN:
            ident = "Unknown";
            break;

        default:
            return NULL;
    }

    self = BrResAllocate(plib->device, sizeof(*self), BR_MEMORY_OBJECT);

    if(self == NULL)
        return NULL;

    self->dispatch   = &bufferStoredDispatch;
    self->identifier = ident;
    self->device     = plib->device;
    self->plib       = plib;

    self->flags |= SBUFF_SHARED;
    SoftPrimSetupBuffer(&self->buffer, pm);

    ObjectContainerAddFront(plib, (br_object *)self);

    return self;
}

static br_error BR_CMETHOD_DECL(br_buffer_stored_softprim, update)(struct br_buffer_stored *self, struct br_device_pixelmap *pm, br_token_value *tv)
{
    SoftPrimSetupBuffer(&self->buffer, pm);

    return BRE_OK;
}

static void BR_CMETHOD_DECL(br_buffer_stored_softprim, free)(br_object *_self)
{
    struct br_buffer_stored *self = (struct br_buffer_stored *)_self;

    ObjectContainerRemove(self->plib, (br_object *)self);

    BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_buffer_stored_softprim, type)(br_object *self)
{
    return BRT_BUFFER_STORED;
}

static br_boolean BR_CMETHOD_DECL(br_buffer_stored_softprim, isType)(br_object *self, br_token t)
{
    return (t == BRT_BUFFER_STORED) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_buffer_stored_softprim, space)(br_object *self)
{
    return BrResSizeTotal(self);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_buffer_stored_softprim, templateQuery)(br_object *_self)
{
    struct br_buffer_stored *self = (struct br_buffer_stored *)_self;

    if(self->device->templates.bufferStoredTemplate == NULL)
        self->device->templates.bufferStoredTemplate = BrTVTemplateAllocate(self->device, bufferStoredTemplateEntries,
                                                                            BR_ASIZE(bufferStoredTemplateEntries));

    return self->device->templates.bufferStoredTemplate;
}

/*
 * Default dispatch table for stored buffer
 */
static const struct br_buffer_stored_dispatch bufferStoredDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_buffer_stored_softprim, free),
    ._identifier = BR_CMETHOD_REF(br_object_softprim, identifier),
    ._type       = BR_CMETHOD_REF(br_buffer_stored_softprim, type),
    ._isType     = BR_CMETHOD_REF(br_buffer_stored_softprim, isType),
    ._device     = BR_CMETHOD_REF(br_object_softprim, device),
    ._space      = BR_CMETHOD_REF(br_buffer_stored_softprim, space),

    ._templateQuery = BR_CMETHOD_REF(br_buffer_stored_softprim, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._update = BR_CMETHOD_REF(br_buffer_stored_softprim, update),
};
