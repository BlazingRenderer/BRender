/*
 * Output type methods
 */
#include <stddef.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_output_facility_dispatch outputFacilityDispatch;

/*
 * Output Type info. template
 */
#define F(f) offsetof(struct br_output_facility, f)

static struct br_tv_template_entry outputFacilityTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR,     NULL, F(identifier),          BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},

    {BRT_WIDTH_I32,           NULL, F(width),               BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_WIDTH_MIN_I32,       NULL, F(width),               BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_WIDTH_MAX_I32,       NULL, F(width),               BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_HEIGHT_I32,          NULL, F(height),              BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_HEIGHT_MIN_I32,      NULL, F(height),              BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_HEIGHT_MAX_I32,      NULL, F(height),              BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},

    {BRT_PIXEL_TYPE_U8,       NULL, F(colour_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},
    {BRT_PIXEL_BITS_I32,      NULL, F(colour_bits),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},

    {BRT_WINDOW_MONITOR_I32,  NULL, F(monitor),             BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,   0},

    /* Unused, just required for matching. */
    {BRT_WINDOW_HANDLE_H,     NULL, (br_uintptr_t)NULL,     BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_WINDOW_NAME_CSTR,    NULL, (br_uintptr_t)NULL,     BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_USE_T,               NULL, (br_uintptr_t)BRT_NONE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_HIDPI_B,             NULL, (br_uintptr_t)BR_FALSE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_RESIZABLE_B,         NULL, (br_uintptr_t)BR_FALSE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_OPENGL_B,            NULL, (br_uintptr_t)BR_FALSE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_WINDOW_FULLSCREEN_B, NULL, (br_uintptr_t)BR_FALSE, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
    {BRT_SDL_EXT_PROCS_P,     NULL, 0,                      BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 0},
};

#undef F

#define F(f) offsetof(struct pixelmap_new_tokens, f)
static struct br_tv_template_entry pixelmapNewTemplateEntries[] = {
    {BRT(WIDTH_I32),           F(width),      BRTV_SET, BRTV_CONV_COPY, 0                            },
    {BRT(HEIGHT_I32),          F(height),     BRTV_SET, BRTV_CONV_COPY, 0                            },
    {BRT(PIXEL_BITS_I32),      F(pixel_bits), BRTV_SET, BRTV_CONV_COPY, 0                            },
    {BRT(PIXEL_TYPE_U8),       F(pixel_type), BRTV_SET, BRTV_CONV_COPY, 0                            },
};
#undef F

br_output_facility *OutputFacilityDOSCreateGeneric(br_device *dev)
{
    br_output_facility *self;

    if((self = BrResAllocate(dev->res, sizeof(br_output_facility), BR_MEMORY_OBJECT)) == NULL)
        return NULL;

    self->dispatch    = &outputFacilityDispatch;
    self->identifier  = BrResStrDup(self, "DOS");
    self->device      = dev;
    self->object_list = BrObjectListAllocate(self);
    ObjectContainerAddFront(dev, (br_object *)self);

    self->width       = -1;
    self->height      = -1;
    self->colour_bits = -1;
    self->colour_type = BR_PMT_MAX;
    self->monitor     = -1;

    BrLogTrace("DOS", "Created output facility: %s", self->identifier);
    return self;
}

/*
 * Common object methods
 */
static void BR_CMETHOD_DECL(br_output_facility_dos, free)(br_object *self)
{
    ObjectContainerRemove(ObjectDevice(self), (br_object *)self);

    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_output_facility_dos, identifier)(br_object *self)
{
    return ((br_output_facility *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_output_facility_dos, type)(br_object *self)
{
    return BRT_OUTPUT_FACILITY;
}

static br_boolean BR_CMETHOD_DECL(br_output_facility_dos, isType)(br_object *self, br_token t)
{
    return (t == BRT_OUTPUT_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_output_facility_dos, device)(br_object *self)
{
    return ((br_output_facility *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_output_facility_dos, space)(br_object *self)
{
    return sizeof(br_output_facility);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_output_facility_dos, queryTemplate)(br_object *_self)
{
    br_output_facility *self = (br_output_facility *)_self;

    if(self->device->templates.outputFacilityTemplate == NULL)
        self->device->templates.outputFacilityTemplate = BrTVTemplateAllocate(self->device, outputFacilityTemplateEntries,
                                                                              BR_ASIZE(outputFacilityTemplateEntries));

    return self->device->templates.outputFacilityTemplate;
}

static br_error BR_CMETHOD_DECL(br_output_facility_dos, validSource)(br_output_facility *self, br_boolean *bp, br_object *h)
{
    return BRE_OK;
}

static br_uint_8 FigureOutFormat(br_uint_8 type, br_int_32 bits)
{
    if(type != BR_PMT_MAX)
        return type;

    switch(bits) {
        case 8:
            return BR_PMT_INDEX_8;
        case 15:
            return BR_PMT_RGB_555;
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

/*
 * Instantiate an output pixelmap from the output type
 *
 */
static br_error BR_CMETHOD_DECL(br_output_facility_dos, pixelmapNew)(br_output_facility *self, br_device_pixelmap **ppmap, br_token_value *tv)
{
    br_device_pixelmap *pm;
    br_int_32           count;
    int                 bpp     = -1;

    struct pixelmap_new_tokens pt = {
        .width      = -1,
        .height     = -1,
        .pixel_bits = -1,
        .pixel_type = BR_PMT_MAX,
    };

    if(self->device->templates.pixelmapNewTemplate == NULL) {
        self->device->templates.pixelmapNewTemplate = BrTVTemplateAllocate(self->device, pixelmapNewTemplateEntries,
                                                                           BR_ASIZE(pixelmapNewTemplateEntries));
    }

    BrTokenValueSetMany(&pt, &count, NULL, tv, self->device->templates.pixelmapNewTemplate);

    pt.pixel_type = FigureOutFormat(pt.pixel_type, pt.pixel_bits);

    if((pm = DevicePixelmapDOSAllocate(self->device, self, pt.width, pt.height)) == NULL) {
        return BRE_FAIL;
    }

    /*
     * If the the user's requested a specific pixel format, and the window or surface doesn't have it, die.
     */
    if(pt.pixel_type != BR_PMT_MAX && pm->pm_type != pt.pixel_type) {
        ObjectFree(pm);
        return BRE_FAIL;
    }

    *ppmap = pm;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_output_facility_dos, clutNew)(br_output_facility *self, struct br_device_clut **pclut, br_token_value *tv)
{
    (void)self;
    (void)pclut;
    (void)tv;
    return BRE_FAIL;
}

/*
 * No querying ability yet
 */
static br_error BR_CMETHOD_DECL(br_output_facility_dos, queryCapability)(br_output_facility *self, br_token_value *buffer_in,
                                                                          br_token_value *buffer_out, br_size_t size_buffer_out)
{
    (void)self;
    (void)buffer_in;
    (void)buffer_out;
    (void)size_buffer_out;
    return BRE_FAIL;
}

static void *BR_CMETHOD_DECL(br_output_facility_dos, listQuery)(br_object_container *self)
{
    return ((br_output_facility *)self)->object_list;
}

/*
 * Default dispatch table for device
 */
static const struct br_output_facility_dispatch outputFacilityDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_output_facility_dos, free),
    ._identifier = BR_CMETHOD_REF(br_output_facility_dos, identifier),
    ._type       = BR_CMETHOD_REF(br_output_facility_dos, type),
    ._isType     = BR_CMETHOD_REF(br_output_facility_dos, isType),
    ._device     = BR_CMETHOD_REF(br_output_facility_dos, device),
    ._space      = BR_CMETHOD_REF(br_output_facility_dos, space),

    ._templateQuery = BR_CMETHOD_REF(br_output_facility_dos, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery        = BR_CMETHOD_REF(br_output_facility_dos, listQuery),
    ._tokensMatchBegin = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch      = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd   = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._addFront         = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront      = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove           = BR_CMETHOD_REF(br_object_container, remove),
    ._find             = BR_CMETHOD_REF(br_object_container, find),
    ._findMany         = BR_CMETHOD_REF(br_object_container, findMany),
    ._count            = BR_CMETHOD_REF(br_object_container, count),

    ._validSource     = BR_CMETHOD_REF(br_output_facility_dos, validSource),
    ._pixelmapNew     = BR_CMETHOD_REF(br_output_facility_dos, pixelmapNew),
    ._clutNew         = BR_CMETHOD_REF(br_output_facility_dos, clutNew),
    ._queryCapability = BR_CMETHOD_REF(br_output_facility_dos, queryCapability),
};
