/*
 * Device methods
 */
#include "drv.h"

#define DEVICE_TITLE   "OpenGL v3.2"
#define DEVICE_VERSION BR_VERSION(1, 0, 0)
#define DEVICE_CREATOR "Zane van Iperen"
#define DEVICE_PRODUCT "OpenGL"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_dispatch deviceDispatch;

/*
 * Device info. template
 */
static const char deviceTitle[] = DEVICE_TITLE;

static const char deviceCreator[] = DEVICE_CREATOR;

static const char deviceProduct[] = DEVICE_PRODUCT;

#define F(f) offsetof(br_device, f)
#define A(a) ((br_uintptr_t)(a))

static struct br_tv_template_entry deviceTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR,     NULL, F(identifier),    BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0              },
    {BRT_VERSION_U32,         NULL, 0,                BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, DEVICE_VERSION },
    {BRT_BRENDER_VERSION_U32, NULL, 0,                BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, __BRENDER__    },
    {BRT_DDI_VERSION_U32,     NULL, 0,                BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, __BRENDER_DDI__},
    {BRT_CREATOR_CSTR,        NULL, A(deviceCreator), BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
    {BRT_TITLE_CSTR,          NULL, A(deviceTitle),   BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
    {BRT_PRODUCT_CSTR,        NULL, A(deviceProduct), BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },


     /*
      * Minimum version of OpenGL supported by this driver.
      * Other devices (e.g. SDL) may query these to create appropriate windows.
      */
    {BRT_OPENGL_VERSION_MAJOR_U8, NULL, 0, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 3                      },
    {BRT_OPENGL_VERSION_MINOR_U8, NULL, 0, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, 2                      },
    {BRT_OPENGL_PROFILE_T,        NULL, 0, BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT, BRT_OPENGL_PROFILE_CORE},
};
#undef F
#undef A

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
    BRT_MSAA_SAMPLES_I32,
    BRT_WINDOW_HANDLE_H,
    BRT_OPENGL_EXT_PROCS_P,
    BRT_OPENGL_VERTEX_SHADER_STR,
    BRT_OPENGL_FRAGMENT_SHADER_STR,
    BR_NULL_TOKEN,
};
// clang-format on

br_device *DeviceGLAllocate(const char *identifier, const char *arguments)
{
    br_device *self;

    /*
     * Set up device block and resource anchor
     */
    self              = BrResAllocate(NULL, sizeof(*self), BR_MEMORY_OBJECT);
    self->res         = BrResAllocate(self, 0, BR_MEMORY_DRIVER);
    self->identifier  = identifier;
    self->dispatch    = &deviceDispatch;
    self->device      = self;
    self->object_list = BrObjectListAllocate(self);

    if((self->renderer_facility = RendererFacilityGLInit(self)) == NULL) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    if((self->output_facility = OutputFacilityGLInit(self, self->renderer_facility)) == NULL) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    return self;
}

static void BR_CMETHOD_DECL(br_device_gl, free)(struct br_object *_self)
{
    br_device *self = (br_device *)_self;

    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    /*
     * Remove resources
     */
    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_gl, identifier)(struct br_object *self)
{
    return ((br_device *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_device_gl, type)(struct br_object *self)
{
    (void)self;
    return BRT_DEVICE;
}

static br_boolean BR_CMETHOD_DECL(br_device_gl, isType)(struct br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_gl, device)(struct br_object *self)
{
    return ((br_device *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_gl, space)(struct br_object *self)
{
    (void)self;
    return sizeof(br_device);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_gl, templateQuery)(struct br_object *_self)
{
    br_device *self = (br_device *)_self;

    if(self->templates.deviceTemplate == NULL) {
        self->templates.deviceTemplate = BrTVTemplateAllocate(self, deviceTemplateEntries, BR_ASIZE(deviceTemplateEntries));
    }

    return self->templates.deviceTemplate;
}

static void *BR_CMETHOD_DECL(br_device_gl, listQuery)(struct br_object_container *self)
{
    return ((br_device *)self)->object_list;
}

static const br_tv_match_info *BR_CMETHOD_DECL(br_device_gl, tokensMatchInfoQuery)(br_object_container *self)
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
    ._free       = BR_CMETHOD_REF(br_device_gl, free),
    ._identifier = BR_CMETHOD_REF(br_device_gl, identifier),
    ._type       = BR_CMETHOD_REF(br_device_gl, type),
    ._isType     = BR_CMETHOD_REF(br_device_gl, isType),
    ._device     = BR_CMETHOD_REF(br_device_gl, device),
    ._space      = BR_CMETHOD_REF(br_device_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_gl, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery            = BR_CMETHOD_REF(br_device_gl, listQuery),
    ._tokensMatchBegin     = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch          = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd       = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._tokensMatchInfoQuery = BR_CMETHOD_REF(br_device_gl, tokensMatchInfoQuery),
    ._addFront             = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront          = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove               = BR_CMETHOD_REF(br_object_container, remove),
    ._find                 = BR_CMETHOD_REF(br_object_container, find),
    ._findMany             = BR_CMETHOD_REF(br_object_container, findMany),
    ._count                = BR_CMETHOD_REF(br_object_container, count),
};
