/*
 * Device methods
 */
#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_dispatch deviceDispatch;

/*
 * Device info. template
 */
#define F(f) offsetof(br_device, f)

static struct br_tv_template_entry deviceTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR, NULL, F(identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef F

/*
 * Set up a static device object
 */
br_device *DeviceNullAllocate(const char *identifier)
{
    br_device *self;

    /*
     * Set up device block and resource anchor
     */
    self      = BrResAllocate(NULL, sizeof(*self), BR_MEMORY_OBJECT);
    self->res = BrResAllocate(self, 0, BR_MEMORY_DRIVER);
    if(self->res == NULL) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    self->identifier = identifier;
    self->dispatch   = &deviceDispatch;
    self->device     = self;

    self->object_list = BrObjectListAllocate(self);
    if(self->object_list == NULL) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    self->templates.deviceTemplate = BrTVTemplateAllocate(self, deviceTemplateEntries, BR_ASIZE(deviceTemplateEntries));

    if(self->templates.deviceTemplate == NULL) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    return self;
}

static void BR_CMETHOD_DECL(br_device_nulldev, free)(struct br_object *self)
{
    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    /*
     * Remove resources
     */
    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_nulldev, identifier)(struct br_object *self)
{
    return ((br_device *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_device_nulldev, type)(struct br_object *self)
{
    (void)self;
    return BRT_DEVICE;
}

static br_boolean BR_CMETHOD_DECL(br_device_nulldev, isType)(struct br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_nulldev, device)(struct br_object *self)
{
    return ((br_device *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_nulldev, space)(struct br_object *self)
{
    (void)self;
    return sizeof(br_device);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_nulldev, templateQuery)(struct br_object *_self)
{
    return ((br_device *)_self)->templates.deviceTemplate;
}

static void *BR_CMETHOD_DECL(br_device_nulldev, listQuery)(struct br_object_container *self)
{
    return ((br_device *)self)->object_list;
}

/*
 * Default dispatch table for device
 */
static const struct br_device_dispatch deviceDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_nulldev, free),
    ._identifier = BR_CMETHOD_REF(br_device_nulldev, identifier),
    ._type       = BR_CMETHOD_REF(br_device_nulldev, type),
    ._isType     = BR_CMETHOD_REF(br_device_nulldev, isType),
    ._device     = BR_CMETHOD_REF(br_device_nulldev, device),
    ._space      = BR_CMETHOD_REF(br_device_nulldev, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_nulldev, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery            = BR_CMETHOD_REF(br_device_nulldev, listQuery),
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
};
