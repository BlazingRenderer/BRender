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

br_boolean DeviceClutSDL3IsOurs(const br_device_clut *clut)
{
    return clut->dispatch == &deviceClutDispatch;
}

/*
 * Create a new device CLUT
 */
br_device_clut *DeviceClutSDL3Allocate(br_device_pixelmap *pm, const char *identifier)
{
    br_device_clut *self;

    self = BrResAllocate(pm->device->res, sizeof(br_device_clut), BR_MEMORY_OBJECT_DATA);

    self->dispatch   = &deviceClutDispatch;
    self->device     = pm->device;
    self->identifier = BrResStrDup(self, identifier);
    self->pal        = SDL_GetSurfacePalette(pm->surface);
    self->owned      = BR_FALSE;

    UASSERT(self->pal != NULL);

    ObjectContainerAddFront(pm->device, (br_object *)self);
    return self;
}

static void BR_CMETHOD_DECL(br_device_clut_sdl3, free)(br_object *_self)
{
    br_device_clut *self = (br_device_clut *)_self;

    ObjectContainerRemove(self->device, (br_object *)self);

    if(self->owned == BR_TRUE) {
        SDL_DestroyPalette(self->pal);
        self->pal = NULL;
    }

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_clut_sdl3, identifier)(br_object *self)
{
    return ((br_device_clut *)self)->identifier;
}

static br_device *BR_CMETHOD_DECL(br_device_clut_sdl3, device)(br_object *self)
{
    return ((br_device_clut *)self)->device;
}

static br_token BR_CMETHOD_DECL(br_device_clut_sdl3, type)(br_object *self)
{
    return BRT_DEVICE_CLUT;
}

static br_boolean BR_CMETHOD_DECL(br_device_clut_sdl3, isType)(br_object *self, br_token t)
{
    return (t == BRT_DEVICE_CLUT) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_device_clut_sdl3, space)(br_object *self)
{
    return sizeof(br_device_clut);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_clut_sdl3, queryTemplate)(br_object *_self)
{
    br_device_clut *self = (br_device_clut *)_self;

    if(self->device->templates.deviceClutTemplate == NULL)
        self->device->templates.deviceClutTemplate = BrTVTemplateAllocate(self->device, deviceClutTemplateEntries,
                                                                          BR_ASIZE(deviceClutTemplateEntries));

    return self->device->templates.deviceClutTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_clut_sdl3, entrySet)(br_device_clut *self, br_int_32 index, br_colour entry)
{
    return DeviceClutEntrySetMany(self, index, 1, &entry);
}

static br_error BR_CMETHOD_DECL(br_device_clut_sdl3, entryQuery)(br_device_clut *self, br_colour *entry, br_int_32 index)
{
    return DeviceClutEntryQueryMany(self, entry, index, 1);
}

static br_error BR_CMETHOD_DECL(br_device_clut_sdl3, entrySetMany)(br_device_clut *self, br_int_32 index,
                                                                   br_int_32 count, br_colour *entries)
{
    return DeviceSDL3SetPalette(self->pal, index, count, entries, BR_FALSE);
}

static br_error BR_CMETHOD_DECL(br_device_clut_sdl3, entryQueryMany)(br_device_clut *self, br_colour *entries,
                                                                     br_int_32 index, br_int_32 count)
{
    if(index < 0 || index >= self->pal->ncolors)
        return BRE_OVERFLOW;

    if(index + count >= self->pal->ncolors)
        return BRE_OVERFLOW;

    for(br_int_32 i = 0; i < count; ++i) {
        SDL_Color col = self->pal->colors[index + i];
        entries[i]    = BR_COLOUR_RGB(col.r, col.g, col.b);
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
    ._free       = BR_CMETHOD_REF(br_device_clut_sdl3, free),
    ._identifier = BR_CMETHOD_REF(br_device_clut_sdl3, identifier),
    ._type       = BR_CMETHOD_REF(br_device_clut_sdl3, type),
    ._isType     = BR_CMETHOD_REF(br_device_clut_sdl3, isType),
    ._device     = BR_CMETHOD_REF(br_device_clut_sdl3, device),
    ._space      = BR_CMETHOD_REF(br_device_clut_sdl3, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_clut_sdl3, queryTemplate),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._entrySet       = BR_CMETHOD_REF(br_device_clut_sdl3, entrySet),
    ._entryQuery     = BR_CMETHOD_REF(br_device_clut_sdl3, entryQuery),
    ._entrySetMany   = BR_CMETHOD_REF(br_device_clut_sdl3, entrySetMany),
    ._entryQueryMany = BR_CMETHOD_REF(br_device_clut_sdl3, entryQueryMany),
};
