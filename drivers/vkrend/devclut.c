/*
 * CLUT methods
 */
#include <stddef.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Default dispatch table for device_clut (defined at end of file)
 */
static const struct br_device_clut_dispatch deviceClutDispatch;

/*
 * Device CLUT info. template
 */
#define F(f) offsetof(struct br_device_clut, f)

static br_tv_template_entry deviceClutTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR, NULL, F(identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef F

/*
 * Create a new device CLUT
 */
br_device_clut *DeviceClutVkAllocate(br_device_pixelmap *pm)
{
    br_device_clut *self;

    self = BrResAllocate(pm, sizeof(br_device_clut), BR_MEMORY_OBJECT);

    self->dispatch   = &deviceClutDispatch;
    self->device     = pm->device;
    self->identifier = BrResSprintf(self, "%s:clut", pm->pm_identifier != NULL ? pm->pm_identifier : "vkrend");
    self->pm         = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, BR_ASIZE(self->entries), self->entries, BR_PMAF_NORMAL);
    self->populated  = BR_FALSE;

    return self;
}

static void BR_CMETHOD_DECL(br_device_clut_vk, free)(br_object *_self)
{
    br_device_clut *self = (br_device_clut *)_self;

    if(self->pm != NULL)
        BrPixelmapFree(self->pm);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_clut_vk, identifier)(br_object *self)
{
    return ((br_device_clut *)self)->identifier;
}

static br_device *BR_CMETHOD_DECL(br_device_clut_vk, device)(br_object *self)
{
    return ((br_device_clut *)self)->device;
}

static br_token BR_CMETHOD_DECL(br_device_clut_vk, type)(br_object *self)
{
    return BRT_DEVICE_CLUT;
}

static br_boolean BR_CMETHOD_DECL(br_device_clut_vk, isType)(br_object *self, br_token t)
{
    return (t == BRT_DEVICE_CLUT) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_device_clut_vk, space)(br_object *self)
{
    return sizeof(br_device_clut);
}

static br_tv_template *BR_CMETHOD_DECL(br_device_clut_vk, queryTemplate)(br_object *_self)
{
    br_device_clut *self = (br_device_clut *)_self;

    if(self->device->templates.deviceClutTemplate == NULL)
        self->device->templates.deviceClutTemplate = BrTVTemplateAllocate(self->device, deviceClutTemplateEntries,
                                                                          BR_ASIZE(deviceClutTemplateEntries));

    return self->device->templates.deviceClutTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_clut_vk, entrySet)(br_device_clut *self, br_int_32 index, br_colour entry)
{
    return DeviceClutEntrySetMany(self, index, 1, &entry);
}

static br_error BR_CMETHOD_DECL(br_device_clut_vk, entryQuery)(br_device_clut *self, br_colour *entry, br_int_32 index)
{
    return DeviceClutEntryQueryMany(self, entry, index, 1);
}

static br_error BR_CMETHOD_DECL(br_device_clut_vk, entrySetMany)(br_device_clut *self, br_int_32 index, br_int_32 count, br_colour *entries)
{
    if(count > BR_VKREND_MAX_CLUT_ENTRIES)
        return BRE_OVERFLOW;

    if(index < 0 || index >= self->pm->height)
        return BRE_OVERFLOW;

    if(index + count > self->pm->height)
        return BRE_OVERFLOW;

    BrMemCpy(self->entries + index, entries, count * sizeof(br_colour));

    self->populated = BR_TRUE;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_clut_vk, entryQueryMany)(br_device_clut *self, br_colour *entries, br_int_32 index, br_int_32 count)
{
    if(index < 0 || index >= self->pm->height)
        return BRE_OVERFLOW;

    if(index + count > self->pm->height)
        return BRE_OVERFLOW;

    for(br_int_32 i = 0; i < count; ++i) {
        entries[i] = BrPixelmapPixelGet(self->pm, 0, index + i);
    }

    return BRE_OK;
}

/*
 * Default dispatch table for device CLUT
 */
static const struct br_device_clut_dispatch deviceClutDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_clut_vk, free),
    ._identifier = BR_CMETHOD_REF(br_device_clut_vk, identifier),
    ._type       = BR_CMETHOD_REF(br_device_clut_vk, type),
    ._isType     = BR_CMETHOD_REF(br_device_clut_vk, isType),
    ._device     = BR_CMETHOD_REF(br_device_clut_vk, device),
    ._space      = BR_CMETHOD_REF(br_device_clut_vk, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_clut_vk, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._entrySet       = BR_CMETHOD_REF(br_device_clut_vk, entrySet),
    ._entryQuery     = BR_CMETHOD_REF(br_device_clut_vk, entryQuery),
    ._entrySetMany   = BR_CMETHOD_REF(br_device_clut_vk, entrySetMany),
    ._entryQueryMany = BR_CMETHOD_REF(br_device_clut_vk, entryQueryMany),
};
