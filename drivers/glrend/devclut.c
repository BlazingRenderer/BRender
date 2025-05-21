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
 * Renderer info. template
 */
#define F(f) offsetof(struct br_device_clut, f)

static struct br_tv_template_entry deviceClutTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR, NULL, F(identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef F

/*
 * Create a new device CLUT
 */
br_device_clut *DeviceClutGLAllocate(br_device_pixelmap *pm)
{
    br_device_clut *self;

    self = BrResAllocate(pm->device->res, sizeof(br_device_clut), BR_MEMORY_OBJECT_DATA);

    self->dispatch   = &deviceClutDispatch;
    self->device     = pm->device;
    self->identifier = BrResSprintf(self, "%s:clut", pm->pm_identifier);
    self->pm         = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, BR_ASIZE(self->entries), self->entries, BR_PMAF_NORMAL);
    self->gl_tex     = 0;

    ObjectContainerAddFront(pm->device, (br_object *)self);
    return self;
}

static void BR_CMETHOD_DECL(br_device_clut_gl, free)(br_object *_self)
{
    br_device_clut *self = (br_device_clut *)_self;

    ObjectContainerRemove(self->device, (br_object *)self);

    if(self->pm != NULL)
        BrPixelmapFree(self->pm);

    if(self->gl_tex != 0) {
        glDeleteTextures(1, &self->gl_tex);
    }

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_clut_gl, identifier)(br_object *self)
{
    return ((br_device_clut *)self)->identifier;
}

static br_device *BR_CMETHOD_DECL(br_device_clut_gl, device)(br_object *self)
{
    return ((br_device_clut *)self)->device;
}

static br_token BR_CMETHOD_DECL(br_device_clut_gl, type)(br_object *self)
{
    return BRT_DEVICE_CLUT;
}

static br_boolean BR_CMETHOD_DECL(br_device_clut_gl, isType)(br_object *self, br_token t)
{
    return (t == BRT_DEVICE_CLUT) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_device_clut_gl, space)(br_object *self)
{
    return sizeof(br_device_clut);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_clut_gl, queryTemplate)(br_object *_self)
{
    br_device_clut *self = (br_device_clut *)_self;

    if(self->device->templates.deviceClutTemplate == NULL)
        self->device->templates.deviceClutTemplate = BrTVTemplateAllocate(self->device, deviceClutTemplateEntries,
                                                                          BR_ASIZE(deviceClutTemplateEntries));

    return self->device->templates.deviceClutTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_clut_gl, entrySet)(br_device_clut *self, br_int_32 index, br_colour entry)
{
    return DeviceClutEntrySetMany(self, index, 1, &entry);
}

static br_error BR_CMETHOD_DECL(br_device_clut_gl, entryQuery)(br_device_clut *self, br_colour *entry, br_int_32 index)
{
    return DeviceClutEntryQueryMany(self, entry, index, 1);
}

static br_error BR_CMETHOD_DECL(br_device_clut_gl, entrySetMany)(br_device_clut *self, br_int_32 index, br_int_32 count, br_colour *entries)
{
    if(count > BR_GLREND_MAX_CLUT_ENTRIES)
        return BRE_OVERFLOW;

    if(index < 0 || index >= self->pm->height)
        return BRE_OVERFLOW;

    if(index + count > self->pm->height)
        return BRE_OVERFLOW;

    /*
     * We should be rarely-used, only create the GL object if we are.
     */
    if(self->gl_tex == 0) {
        glGenTextures(1, &self->gl_tex);
    }

    BrMemCpy(self->entries + index, entries, count * sizeof(br_colour));

    if(DeviceGLPixelmapToExistingGLTexture(self->gl_tex, self->pm) != BRE_OK)
        return BRE_FAIL;

    DeviceGLObjectLabelF(GL_TEXTURE, self->gl_tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "%s", self->identifier);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_clut_gl, entryQueryMany)(br_device_clut *self, br_colour *entries, br_int_32 index, br_int_32 count)
{
    if(index < 0 || index >= self->pm->height)
        return BRE_OVERFLOW;

    if(index + count >= self->pm->height)
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
    ._free       = BR_CMETHOD_REF(br_device_clut_gl, free),
    ._identifier = BR_CMETHOD_REF(br_device_clut_gl, identifier),
    ._type       = BR_CMETHOD_REF(br_device_clut_gl, type),
    ._isType     = BR_CMETHOD_REF(br_device_clut_gl, isType),
    ._device     = BR_CMETHOD_REF(br_device_clut_gl, device),
    ._space      = BR_CMETHOD_REF(br_device_clut_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_clut_gl, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._entrySet       = BR_CMETHOD_REF(br_device_clut_gl, entrySet),
    ._entryQuery     = BR_CMETHOD_REF(br_device_clut_gl, entryQuery),
    ._entrySetMany   = BR_CMETHOD_REF(br_device_clut_gl, entrySetMany),
    ._entryQueryMany = BR_CMETHOD_REF(br_device_clut_gl, entryQueryMany),
};
