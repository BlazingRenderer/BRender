/*
 * Device methods
 */
#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_dispatch deviceDispatch;

#define DEVICE_VERSION BR_VERSION(1, 0, 0)

/*
 * Device info. template
 */
static const char deviceTitle[] = "SDL2";

static const char deviceCreator[] = "Zane van Iperen";
static const char deviceProduct[] = "SDL2";

/*
 * Device info. template
 */
#define F(f) offsetof(br_device, f)
#define A(f) ((br_uintptr_t)(f))
static struct br_tv_template_entry deviceTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR,     NULL, F(identifier),    BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0              },
    {BRT_VERSION_U32,         NULL, 0,                BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, DEVICE_VERSION },
    {BRT_BRENDER_VERSION_U32, NULL, 0,                BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, __BRENDER__    },
    {BRT_DDI_VERSION_U32,     NULL, 0,                BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, __BRENDER_DDI__},
    {BRT_CREATOR_CSTR,        NULL, A(deviceCreator), BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
    {BRT_TITLE_CSTR,          NULL, A(deviceTitle),   BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
    {BRT_PRODUCT_CSTR,        NULL, A(deviceProduct), BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
};
#undef A
#undef F

/*
 * Set up a static device object
 */
br_device *DeviceSDL2Allocate(const char *identifier)
{
    br_device *self;

    if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        BrLogError("SDL2", "Initialisation failure: %s", SDL_GetError());
        return NULL;
    }

    /*
     * Set up device block and resource anchor
     */
    self      = BrResAllocate(NULL, sizeof(*self), BR_MEMORY_OBJECT);
    self->res = BrResAllocate(self, 0, BR_MEMORY_DRIVER);

    self->identifier = identifier;
    self->dispatch   = &deviceDispatch;
    self->device     = self;

    self->object_list = BrObjectListAllocate(self);

    self->templates.deviceTemplate = BrTVTemplateAllocate(self, deviceTemplateEntries, BR_ASIZE(deviceTemplateEntries));

    return self;
}

static void BR_CMETHOD_DECL(br_device_sdl2, free)(struct br_object *self)
{
    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    /*
     * Remove resources
     */
    BrResFreeNoCallback(self);

    /*
     * Tidy up SDL2.
     */
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static const char *BR_CMETHOD_DECL(br_device_sdl2, identifier)(struct br_object *self)
{
    return ((br_device *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_device_sdl2, type)(struct br_object *self)
{
    (void)self;
    return BRT_DEVICE;
}

static br_boolean BR_CMETHOD_DECL(br_device_sdl2, isType)(struct br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_sdl2, device)(struct br_object *self)
{
    return ((br_device *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_sdl2, space)(struct br_object *self)
{
    (void)self;
    return sizeof(br_device);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_sdl2, templateQuery)(struct br_object *_self)
{
    return ((br_device *)_self)->templates.deviceTemplate;
}

static void *BR_CMETHOD_DECL(br_device_sdl2, listQuery)(struct br_object_container *self)
{
    return ((br_device *)self)->object_list;
}

/*
 * List of tokens which are not significant in matching (for output facilities)
 */
// clang-format off
static const br_token insignificantMatchTokens[] = {
    BRT_WIDTH_I32,
    BRT_HEIGHT_I32,
    BRT_PIXEL_BITS_I32,
    BRT_PIXEL_TYPE_U8,
    BRT_WINDOW_MONITOR_I32,
    BRT_WINDOW_HANDLE_H,
    BRT_WINDOW_NAME_CSTR,
    BRT_SURFACE_HANDLE_H,
    BRT_USE_T,
    BRT_HIDPI_B,
    BRT_RESIZABLE_B,
    BRT_OPENGL_B,
    BRT_SDL_EXT_PROCS_P,
    BR_NULL_TOKEN,
};
// clang-format on

static const br_tv_match_info *BR_CMETHOD_DECL(br_device_sdl2, tokensMatchInfoQuery)(br_object_container *self)
{
    static const br_tv_match_info matchinfo[] = {
        {.type = BRT_OUTPUT_FACILITY, .insignificant = insignificantMatchTokens},
        {.type = BR_NULL_TOKEN,       .insignificant = NULL                    },
    };

    (void)self;
    return matchinfo;
}

/*
 * Default dispatch table for device
 */
static const struct br_device_dispatch deviceDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_sdl2, free),
    ._identifier = BR_CMETHOD_REF(br_device_sdl2, identifier),
    ._type       = BR_CMETHOD_REF(br_device_sdl2, type),
    ._isType     = BR_CMETHOD_REF(br_device_sdl2, isType),
    ._device     = BR_CMETHOD_REF(br_device_sdl2, device),
    ._space      = BR_CMETHOD_REF(br_device_sdl2, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_sdl2, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery            = BR_CMETHOD_REF(br_device_sdl2, listQuery),
    ._tokensMatchBegin     = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch          = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd       = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._tokensMatchInfoQuery = BR_CMETHOD_REF(br_device_sdl2, tokensMatchInfoQuery),
    ._addFront             = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront          = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove               = BR_CMETHOD_REF(br_object_container, remove),
    ._find                 = BR_CMETHOD_REF(br_object_container, find),
    ._findMany             = BR_CMETHOD_REF(br_object_container, findMany),
    ._count                = BR_CMETHOD_REF(br_object_container, count),
};
