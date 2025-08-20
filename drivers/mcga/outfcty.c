/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: outfcty.c 1.1 1997/12/10 16:45:46 jon Exp $
 * $Locker: $
 *
 * Output type methods
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_output_facility_dispatch outputFacilityDispatch;

/*
 * Output Type info. template
 */
#define F(f) offsetof(struct br_output_facility, f)

static br_tv_template_entry outputFacilityTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),  F(identifier),  BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},

    {BRT(WIDTH_I32),        F(width),       BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},
    {BRT(WIDTH_MIN_I32),    F(width),       BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},
    {BRT(WIDTH_MAX_I32),    F(width),       BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},
    {BRT(HEIGHT_I32),       F(height),      BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},
    {BRT(HEIGHT_MIN_I32),   F(height),      BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},
    {BRT(HEIGHT_MAX_I32),   F(height),      BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},

    {BRT(PIXEL_TYPE_U8),    F(colour_type), BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},
    {BRT(PIXEL_BITS_I32),   F(colour_bits), BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},

    //	{BRT(PIXEL_CHANNELS_I32),F(channels_mask),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY },
    //	{BRT(PIXEL_CHANNELS_TL), F(channels_list),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY },

    {BRT(INDEXED_B),        F(indexed),     BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0},

    //	{BRT(MEMORY_MAPPED_B), 	F(),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY },

    {BRT(PIXELMAP_MAX_I32), 0,              BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, 1},
    {BRT(CLUT_MAX_I32),     0,              BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, 0},

    //	{BRT(VIDEO_MEMORY_U32), F(),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY },
    //	{BRT(TEXTURE_MEMORY_U32), F(),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY },
    //	{BRT(HOST_MEMORY_U32), F(),					BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY },

    {BRT(MODE_U32),         0x13,           BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0},
};
#undef F

/*
 * Setup a static output facility
 */
br_output_facility *OutputFacilityVGAAllocate(br_device *dev, const char *identifier, br_int_32 mode, br_int_32 width, br_int_32 height,
                                              br_int_32 bits, br_int_32 type, br_boolean indexed)
{
    br_output_facility *self;

    self = BrResAllocate(NULL, sizeof(*self), BR_MEMORY_OBJECT_DATA);

    self->dispatch   = &outputFacilityDispatch;
    self->identifier = identifier;
    self->device     = dev;

    self->bios_mode   = mode;
    self->width       = width;
    self->height      = height;
    self->colour_type = type;
    self->colour_bits = bits;
    self->indexed     = indexed;

    self->num_instances = 0;
    self->object_list   = BrObjectListAllocate(DeviceVGAResource(dev));

    ObjectContainerAddFront(dev, (br_object *)self);
    return self;
}

/*
 * Common object methods
 */
static void BR_CMETHOD_DECL(br_output_facility_vga, free)(br_object *self)
{
    ObjectContainerRemove(ObjectDevice(self), self);

    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_output_facility_vga, type)(br_object *self)
{
    return BRT_OUTPUT_FACILITY;
}

static br_boolean BR_CMETHOD_DECL(br_output_facility_vga, isType)(br_object *self, br_token t)
{
    return (t == BRT_OUTPUT_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_output_facility_vga, space)(br_object *self)
{
    return sizeof(br_output_facility);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_output_facility_vga, queryTemplate)(br_object *_self)
{
    br_output_facility *self = (br_output_facility *)_self;

    if(self->device->templates.outputFacilityTemplate == NULL)
        self->device->templates.outputFacilityTemplate = BrTVTemplateAllocate(self->device, outputFacilityTemplateEntries,
                                                                              BR_ASIZE(outputFacilityTemplateEntries));

    return self->device->templates.outputFacilityTemplate;
}

static br_error BR_CMETHOD_DECL(br_output_facility_vga, validSource)(br_output_facility *self, br_boolean *bp, br_object *h)
{
    return BRE_OK;
}

/*
 * Instantiate an output pixelmap from the output type
 *
 */
static br_error BR_CMETHOD_DECL(br_output_facility_vga, pixelmapNew)(br_output_facility *self, br_device_pixelmap **ppmap, br_token_value *tv)
{
    br_device_pixelmap *pm;

    /*
     * Create device pixelmap structure pointing at display memory
     */
    pm = DevicePixelmapMCGAAllocateMode(ObjectDevice(self), self, self->width, self->height);

    if(pm == NULL)
        return BRE_FAIL;

    *ppmap = pm;
    self->num_instances++;

    return BRE_OK;
}

/*
 * Cannot create new CLUTs, stuck with the single hardware one
 */
static br_error BR_CMETHOD_DECL(br_output_facility_vga, clutNew)(br_output_facility *self, br_device_clut **pclut, br_token_value *tv)
{
    return BRE_FAIL;
}

/*
 * No querying ability yet
 */
static br_error BR_CMETHOD_DECL(br_output_facility_vga, queryCapability)(br_output_facility *self, br_token_value *buffer_in,
                                                                         br_token_value *buffer_out, br_size_t size_buffer_out)
{
    return BRE_FAIL;
}

static void *BR_CMETHOD_DECL(br_output_facility_vga, listQuery)(br_object_container *_self)
{
    br_output_facility *self = (br_output_facility *)_self;
    return self->object_list;
}

/*
 * Default dispatch table for device
 */
static const struct br_output_facility_dispatch outputFacilityDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_output_facility_vga, free),
    ._identifier = BR_CMETHOD_REF(br_object_vga, identifier),
    ._type       = BR_CMETHOD_REF(br_output_facility_vga, type),
    ._isType     = BR_CMETHOD_REF(br_output_facility_vga, isType),
    ._device     = BR_CMETHOD_REF(br_object_vga, device),
    ._space      = BR_CMETHOD_REF(br_output_facility_vga, space),

    ._templateQuery = BR_CMETHOD_REF(br_output_facility_vga, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery            = BR_CMETHOD_REF(br_output_facility_vga, listQuery),
    ._tokensMatchBegin     = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch          = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd       = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._tokensMatchInfoQuery = BR_CMETHOD_REF(br_object_container, tokensMatchInfoQuery),
    ._addFront             = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront          = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove               = BR_CMETHOD_REF(br_object_container, remove),
    ._find                 = BR_CMETHOD_REF(br_object_container, find),
    ._findMany             = BR_CMETHOD_REF(br_object_container, findMany),
    ._count                = BR_CMETHOD_REF(br_object_container, count),

    ._validSource     = BR_CMETHOD_REF(br_output_facility_vga, validSource),
    ._pixelmapNew     = BR_CMETHOD_REF(br_output_facility_vga, pixelmapNew),
    ._clutNew         = BR_CMETHOD_REF(br_output_facility_vga, clutNew),
    ._queryCapability = BR_CMETHOD_REF(br_output_facility_vga, queryCapability),
};
