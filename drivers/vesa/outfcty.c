/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: outfcty.c 1.1 1997/12/10 16:54:15 jon Exp $
 * $Locker: $
 *
 * Output facility methods
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static struct br_output_facility_dispatch outputFacilityDispatch;

/*
 * Output Facility info. template
 */
#define F(f) offsetof(struct br_output_facility, f)
#define S(s) (br_uint_32)(s)
static struct br_tv_template_entry outputFacilityTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),  F(identifier),  BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},

    {BRT(WIDTH_I32),        F(width),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
    {BRT(WIDTH_MIN_I32),    F(width),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
    {BRT(WIDTH_MAX_I32),    F(width),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
    {BRT(HEIGHT_I32),       F(height),      BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
    {BRT(HEIGHT_MIN_I32),   F(height),      BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
    {BRT(HEIGHT_MAX_I32),   F(height),      BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
    {BRT(PIXEL_TYPE_U8),    F(colour_type), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},
    {BRT(PIXEL_BITS_I32),   F(colour_bits), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},

    //	{BRT(PIXEL_CHANNELS_I32),F(channels_mask),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, 0},
    //	{BRT(PIXEL_CHANNELS_TL), F(channels_list),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, 0},

    {BRT(INDEXED_B),        F(indexed),     BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0},

    //	{BRT(MEMORY_MAPPED_B), 	F(),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, 0},

    {BRT(PIXELMAP_MAX_I32), 0,              BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT,  1},
    {BRT(CLUT_MAX_I32),     0,              BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT,  0},

    //	{BRT(VIDEO_MEMORY_U32), F(),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
    //	{BRT(TEXTURE_MEMORY_U32), F(),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },
    //	{BRT(HOST_MEMORY_U32), F(),					BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY, },

    {BRT(MODE_U32),         F(vesa_mode),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0},
    {DEV(VESA_MODE_U16),    F(vesa_mode),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0},
};
#undef F

static struct masks_to_mode {
    br_uint_8 red_mask_size;
    br_uint_8 red_field_position;
    br_uint_8 green_mask_size;
    br_uint_8 green_field_position;
    br_uint_8 blue_mask_size;
    br_uint_8 blue_field_position;
    br_uint_8 rsvd_mask_size;
    br_uint_8 rsvd_field_position;

    br_uint_32 colour_type;
    br_uint_32 colour_bits;
} masksToMode[] = {
    {5, 10, 5, 5, 5, 0, 1, 15, BR_PMT_RGB_555,  15},
    {5, 10, 5, 5, 5, 0, 0, 0,  BR_PMT_RGB_555,  15},
    {5, 11, 6, 5, 5, 0, 0, 0,  BR_PMT_RGB_565,  16},
    {8, 16, 8, 8, 8, 0, 0, 0,  BR_PMT_RGB_888,  24},
    {8, 16, 8, 8, 8, 0, 8, 24, BR_PMT_RGBX_888, 32},
};

/*
 * Setup a static output facility
 */
br_error OutputFacilityVESAInitialise(int *count, br_device *dev)
{
    br_output_facility  *self;
    int                  m, i;
    struct vesa_info    *vip = DeviceVESAInfo(dev);
    struct vesa_modeinfo modeinfo;
    br_int_32            type, bits;
    char                 tmp[80];

    /*
     * Find the number of supported modes
     */
    for(m = 0; vip->videomode_ptr[m] != 0xFFFF;)
        m++;

    /*
     * Go through and investigate the available vesa modes
     */
    for(m--; m >= 0; m--) {

        if(VESAModeInfo(&modeinfo, vip->videomode_ptr[m]) != BRE_OK)
            continue;

        /*
         * See if this mode is of interest
         */
        if(!(modeinfo.mode_attributes & VESA_MA_SUPPORTED))
            continue;

        if(!(modeinfo.mode_attributes & VESA_MA_GRAPHICS))
            continue;

        if(!(modeinfo.mode_attributes & VESA_MA_LINEAR))
            continue;

        type = -1;

        switch(modeinfo.memory_model) {
            case VESA_MM_PACKED_PIXEL:
                switch(modeinfo.bits_per_pixel) {
                    case 8:
                        type = BR_PMT_INDEX_8;
                        bits = 8;
                        break;
                    case 15:
                        type = BR_PMT_RGB_555;
                        bits = 15;
                        break;
                    case 16:
                        type = BR_PMT_RGB_565;
                        bits = 16;
                        break;
                    case 24:
                        type = BR_PMT_RGB_888;
                        bits = 24;
                        break;
                    case 32:
                        type = BR_PMT_RGBX_888;
                        bits = 32;
                        break;
                }
                break;

            case VESA_MM_DIRECT:

                for(i = 0; i < BR_ASIZE(masksToMode); i++) {
                    if((masksToMode[i].red_mask_size == modeinfo.red_mask_size) &&
                       (masksToMode[i].red_field_position == modeinfo.red_field_position) &&
                       (masksToMode[i].green_mask_size == modeinfo.green_mask_size) &&
                       (masksToMode[i].green_field_position == modeinfo.green_field_position) &&
                       (masksToMode[i].blue_mask_size == modeinfo.blue_mask_size) &&
                       (masksToMode[i].blue_field_position == modeinfo.blue_field_position) &&
                       (masksToMode[i].rsvd_mask_size == modeinfo.rsvd_mask_size) &&
                       (masksToMode[i].rsvd_field_position == modeinfo.rsvd_field_position)) {
                        type = masksToMode[i].colour_type;
                        bits = masksToMode[i].colour_bits;
                        break;
                    }
                }
                break;
        }

        if(type == -1)
            continue;

        /*
         * Allocate an object and fill it in
         */
        self = BrResAllocate(DeviceVESAResource(dev), sizeof(*self), BR_MEMORY_OBJECT_DATA);

        self->indexed     = (type == BR_PMT_INDEX_8);
        self->colour_type = type;
        self->colour_bits = bits;
        self->dispatch    = &outputFacilityDispatch;
        self->device      = dev;
        self->width       = modeinfo.x_resolution;
        self->height      = modeinfo.y_resolution;
        self->vesa_mode   = vip->videomode_ptr[m];
        self->modeinfo    = modeinfo;

        self->identifier = BrResSprintf(self, "%dx%dx%d", self->width, self->height, bits);

        self->object_list = BrObjectListAllocate(self);

        ObjectContainerAddFront(dev, (br_object *)self);

        (*count)++;
    }

    return BRE_OK;
}

/*
 * Common object methods
 */
static void BR_CMETHOD_DECL(br_output_facility_vesa, free)(br_object *_self)
{
    br_output_facility *self = (br_output_facility *)_self;

    ObjectContainerRemove(ObjectDevice(self), (br_object *)self);

    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_output_facility_vesa, identifier)(br_object *_self)
{
    br_output_facility *self = (br_output_facility *)_self;
    return self->identifier;
}

static br_token BR_CMETHOD_DECL(br_output_facility_vesa, type)(br_object *self)
{
    return BRT_OUTPUT_FACILITY;
}

static br_boolean BR_CMETHOD_DECL(br_output_facility_vesa, isType)(br_object *self, br_token t)
{
    return (t == BRT_OUTPUT_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_output_facility_vesa, device)(br_object *_self)
{
    br_output_facility *self = (br_output_facility *)_self;
    return self->device;
}

static br_size_t BR_CMETHOD_DECL(br_output_facility_vesa, space)(br_object *self)
{
    return sizeof(br_output_facility);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_output_facility_vesa, templateQuery)(br_object *_self)
{
    br_output_facility *self = (br_output_facility *)_self;

    if(self->device->templates.outputFacilityTemplate == NULL)
        self->device->templates.outputFacilityTemplate = BrTVTemplateAllocate(self->device, outputFacilityTemplateEntries,
                                                                              BR_ASIZE(outputFacilityTemplateEntries));

    return self->device->templates.outputFacilityTemplate;
}

static br_error BR_CMETHOD_DECL(br_output_facility_vesa, validSource)(br_output_facility *self, br_boolean *bp, br_object *h)
{
    return BRE_OK;
}

/*
 * Instantiate an output pixelmap from the output facility
 *
 */
static br_error BR_CMETHOD_DECL(br_output_facility_vesa, pixelmapNew)(br_output_facility *self, br_device_pixelmap **ppmap, br_token_value *tv)
{
    br_device_pixelmap *pm;

    /*
     * Create device pixelmap structure pointing at display memory
     */
    pm = DevicePixelmapVESAAllocateMode(ObjectDevice(self), (br_output_facility *)self, self->width, self->height);

    if(pm == NULL)
        return BRE_FAIL;

    *ppmap = pm;
    self->num_instances++;

    return BRE_OK;
}

/*
 * Cannot create new CLUTs, stuck with the single hardware one
 */
static br_error BR_CMETHOD_DECL(br_output_facility_vesa, clutNew)(br_output_facility *self, br_device_clut **pclut, br_token_value *tv)
{
    return BRE_FAIL;
}

/*
 * No querying ability yet
 */
static br_error BR_CMETHOD_DECL(br_output_facility_vesa, queryCapability)(br_output_facility *self, br_token_value *buffer_in,
                                                                          br_token_value *buffer_out, br_size_t size_buffer_out)
{
    (void)self;
    (void)buffer_in;
    (void)buffer_out;
    (void)size_buffer_out;
    return BRE_FAIL;
}

static void *BR_CMETHOD_DECL(br_output_facility_vesa, listQuery)(br_object_container *_self)
{
    br_output_facility *self = (br_output_facility *)_self;

    return self->object_list;
}

/*
 * Default dispatch table for device
 */
static struct br_output_facility_dispatch outputFacilityDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_output_facility_vesa, free),
    ._identifier = BR_CMETHOD_REF(br_output_facility_vesa, identifier),
    ._type       = BR_CMETHOD_REF(br_output_facility_vesa, type),
    ._isType     = BR_CMETHOD_REF(br_output_facility_vesa, isType),
    ._device     = BR_CMETHOD_REF(br_output_facility_vesa, device),
    ._space      = BR_CMETHOD_REF(br_output_facility_vesa, space),

    ._templateQuery = BR_CMETHOD_REF(br_output_facility_vesa, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._queryCapability      = BR_CMETHOD_REF(br_output_facility_vesa, queryCapability),
    ._listQuery            = BR_CMETHOD_REF(br_output_facility_vesa, listQuery),
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

    ._validSource = BR_CMETHOD_REF(br_output_facility_vesa, validSource),
    ._pixelmapNew = BR_CMETHOD_REF(br_output_facility_vesa, pixelmapNew),
    ._clutNew     = BR_CMETHOD_REF(br_output_facility_vesa, clutNew),
};
