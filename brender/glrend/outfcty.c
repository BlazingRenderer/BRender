/*
 * Output type methods
 */
#include "drv.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_output_facility_dispatch outputFacilityDispatch;

/*
 * Output Type info. template
 */
#define F(f) offsetof(br_output_facility, f)

static struct br_tv_template_entry outputFacilityTemplateEntries[] = {
    {BRT_WIDTH_I32,               NULL, F(width),             BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_WIDTH_MIN_I32,           NULL, F(width),             BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_WIDTH_MAX_I32,           NULL, F(width),             BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_HEIGHT_I32,              NULL, F(height),            BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_HEIGHT_MIN_I32,          NULL, F(height),            BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_HEIGHT_MAX_I32,          NULL, F(height),            BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_PIXEL_BITS_I32,          NULL, F(colour_bits),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_PIXEL_TYPE_U8,           NULL, F(colour_type),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_INDEXED_B,               NULL, F(indexed),           BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_WINDOW_MONITOR_I32,      NULL, F(monitor),           BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_IDENTIFIER_CSTR,         NULL, F(identifier),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_RENDERER_FACILITY_O,     NULL, F(renderer_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_TEMPORARY_B,             NULL, F(temporary),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},
    {BRT_SDL_DISPLAY_MODE_P,      NULL, F(display_mode),      BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,},

    /*
     * We don't use these, but we need BrDevBeginVar to accept them.
     * These are passed to pixelmapNew, which will handle them.
     */
    {BRT_WINDOW_HANDLE_H,         NULL, 0,                    BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT,},
    {BRT_MSAA_SAMPLES_I32,        NULL, 0,                    BRTV_QUERY | BRTV_ALL, BRTV_CONV_DIRECT,},
};

#undef F

static int BrType_BPP(int type)
{
    switch(type) {
        case BR_PMT_INDEX_8:
            return 8;
        case BR_PMT_RGB_555:
            return 15;
        case BR_PMT_RGB_565:
            return 16;
        case BR_PMT_RGB_888:
            return 24;
        case BR_PMT_RGBX_888:
        case BR_PMT_RGBA_8888:
        case BR_PMT_ARGB_8888:
            return 32;
        default:
            return 0;
    }
}

static br_output_facility *OutputFacilityGLCreateMode(br_device *dev, br_uint_16 type,
                                                      br_int_32 width, br_int_32 height,
                                                      br_int_32 monitorIndex, br_boolean temporary)
{
    br_output_facility *self;
    char               tmp[64];

    self = BrResAllocate(dev, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch    = &outputFacilityDispatch;
    self->device      = dev;
    self->object_list = BrObjectListAllocate(self);

    /* Fill in display format */
    self->colour_type = type;
    self->colour_bits = BrType_BPP(type);
    self->indexed     = (type <= BR_PMT_INDEX_8);
    self->monitor     = monitorIndex;

    self->width     = width;
    self->height    = height;
    self->temporary = temporary;

    /* Attach a descriptive identifier */
    BrSprintfN(tmp, sizeof(tmp) - 1, "%dx%dx%d", self->width, self->height, self->colour_bits);
    BrLogTrace("SDL", "Registering %sscreen mode %s.", temporary ? "temporary " : "", tmp);
    self->identifier = BrResStrDup(self, tmp);

    self->renderer_facility = dev->renderer_facility;

    self->display_mode = NULL;

    ObjectContainerAddFront(dev, (br_object *)self);
    return self;
}


static br_error sdlToBRrenderSurfaceType(const SDL_DisplayMode *mode, br_uint_16 *pType)
{
    switch(mode->format) {
        case SDL_PIXELFORMAT_RGB555:
            *pType = BR_PMT_RGB_555;
            break;
        case SDL_PIXELFORMAT_RGB565:
            *pType = BR_PMT_RGB_565;
            break;
        case SDL_PIXELFORMAT_RGB888:
            *pType = BR_PMT_RGB_888;
            break;
        case SDL_PIXELFORMAT_ARGB8888:
            *pType = BR_PMT_ARGB_8888;
            break;
        default:
            BrLogWarn("GLREND", "Unhandled SDL pixel format: %s", SDL_GetPixelFormatName(mode->format));
            return BRE_FAIL;
    }

    return BRE_OK;
}


static br_output_facility *OutputFacilityGLCreateModeFromSDL(br_device *dev, br_int_32 monitorIndex,
                                                             const SDL_DisplayMode *mode)
{
    br_uint_16         type;
    br_output_facility *outfcty;

    if(sdlToBRrenderSurfaceType(mode, &type) != BRE_OK)
        return NULL;

    outfcty = OutputFacilityGLCreateMode(dev, type, mode->w, mode->h, monitorIndex, 0);
    outfcty->_display_mode = *mode;
    outfcty->display_mode  = &outfcty->_display_mode;
    return outfcty;
}

br_output_facility *OutputFacilityGLCreateTemporary(br_device *dev, br_token_value *tv)
{
    int        width = -1, height = -1, bpp = -1;
    br_uint_16 pmt;

    for(br_token_value *v = tv; v->t != 0; ++v) {
        if(v->t == BRT_WIDTH_I32)
            width = v->v.i32;
        else if(v->t == BRT_HEIGHT_I32)
            height = v->v.i32;
        else if(v->t == BRT_PIXEL_BITS_I32)
            bpp = v->v.i32;
    }

    if(width <= 0 || height <= 0)
        return NULL;

    if(bpp == 16)
        pmt = BR_PMT_RGB_565;
    else if(bpp == 24)
        pmt = BR_PMT_RGB_888;
    else if(bpp == 32)
        pmt = BR_PMT_RGBA_8888;
    else
        return NULL;

    return OutputFacilityGLCreateMode(dev, pmt, width, height, -1, 1);
}

br_error OutputFacilityGLEnumerate(br_device *device)
{
    int numMonitors = SDL_GetNumVideoDisplays();

    for(int i = 0; i < numMonitors; ++i) {
        int numModes = SDL_GetNumDisplayModes(i);

        /* Add all the fullscreen modes. */
        for(int j = 0; j < numModes; ++j) {
            SDL_DisplayMode mode;

            /* Create the mode. */
            if(SDL_GetDisplayMode(i, j, &mode) < 0) {
                BrLogWarn("SDL", "SDL_GetDisplayMode(%d, %d) failed: %s", i, j, SDL_GetError());
                continue;
            }

            (void)OutputFacilityGLCreateModeFromSDL(device, i, &mode);
        }
    }

    return BRE_OK;
}


/*
 * Common object methods
 */
static void BR_CMETHOD_DECL(br_output_facility_sdl, free)(br_object *_self)
{
    br_output_facility *self = (br_output_facility *)_self;

    ObjectContainerRemove(self->device, (br_object *)self);

    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_output_facility_sdl, identifier)(br_object *self)
{
    return ((br_output_facility *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_output_facility_sdl, type)(br_object *self)
{
    (void)self;
    return BRT_OUTPUT_FACILITY;
}

static br_boolean BR_CMETHOD_DECL(br_output_facility_sdl, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_OUTPUT_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_output_facility_sdl, device)(br_object *self)
{
    return ((br_output_facility *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_output_facility_sdl, space)(br_object *self)
{
    (void)self;
    return sizeof(br_output_facility);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_output_facility_sdl, queryTemplate)(br_object *_self)
{
    br_output_facility *self = (br_output_facility *)_self;

    if(self->device->templates.outputFacilityTemplate == NULL)
        self->device->templates.outputFacilityTemplate = BrTVTemplateAllocate(
            self->device,
            outputFacilityTemplateEntries,
            BR_ASIZE(outputFacilityTemplateEntries)
        );

    return self->device->templates.outputFacilityTemplate;
}

static br_error BR_CMETHOD_DECL(br_output_facility_sdl, validSource)(br_output_facility *self,
                                                                     br_boolean *bp, br_object *h)
{
    (void)self;
    (void)bp;
    (void)h;
    return BRE_OK;
}

/*
 * Instantiate an output pixelmap from the output type
 */
static br_error BR_CMETHOD_DECL(br_output_facility_sdl, pixelmapNew)
    (br_output_facility *self, br_device_pixelmap **ppmap, br_token_value *tv)
{
    br_device_pixelmap *pm;

    /*
     * Create a device pixelmap structure representing display memory
     */
    pm = DevicePixelmapGLAllocate(self->device, self, tv);

    if(pm == NULL)
        return BRE_FAIL;

    *ppmap = pm;

    return BRE_OK;
}

/*
 * Cannot create new CLUTs, stuck with the single hardware one
 */
static br_error BR_CMETHOD_DECL(br_output_facility_sdl, clutNew)(br_output_facility *self,
                                                                 br_device_clut **pclut, br_token_value *tv)
{
    (void)self;
    (void)pclut;
    (void)tv;
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_output_facility_sdl, queryCapability)(
    br_output_facility *self, br_token_value *buffer_in, br_token_value *buffer_out, br_size_t size_buffer_out)
{
    (void)self;
    (void)buffer_in;
    (void)buffer_out;
    (void)size_buffer_out;
    return BRE_FAIL;
}


static void *BR_CMETHOD_DECL(br_output_facility_sdl, listQuery)(struct br_object_container *self)
{
    return ((br_output_facility *)self)->object_list;
}

/*
 * Output facility dispatch table
 */
static const struct br_output_facility_dispatch outputFacilityDispatch = {
    .__reserved0        = NULL,
    .__reserved1        = NULL,
    .__reserved2        = NULL,
    .__reserved3        = NULL,
    ._free              = BR_CMETHOD_REF(br_output_facility_sdl, free),
    ._identifier        = BR_CMETHOD_REF(br_output_facility_sdl, identifier),
    ._type              = BR_CMETHOD_REF(br_output_facility_sdl, type),
    ._isType            = BR_CMETHOD_REF(br_output_facility_sdl, isType),
    ._device            = BR_CMETHOD_REF(br_output_facility_sdl, device),
    ._space             = BR_CMETHOD_REF(br_output_facility_sdl, space),

    ._templateQuery     = BR_CMETHOD_REF(br_output_facility_sdl, queryTemplate),
    ._query             = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer       = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany         = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize     = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll          = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize      = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery         = BR_CMETHOD_REF(br_output_facility_sdl, listQuery),
    ._tokensMatchBegin  = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch       = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd    = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._addFront          = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront       = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove            = BR_CMETHOD_REF(br_object_container, remove),
    ._find              = BR_CMETHOD_REF(br_object_container, find),
    ._findMany          = BR_CMETHOD_REF(br_object_container, findMany),
    ._count             = BR_CMETHOD_REF(br_object_container, count),

    ._validSource       = BR_CMETHOD_REF(br_output_facility_sdl, validSource),
    ._pixelmapNew       = BR_CMETHOD_REF(br_output_facility_sdl, pixelmapNew),
    ._clutNew           = BR_CMETHOD_REF(br_output_facility_sdl, clutNew),
    ._queryCapability   = BR_CMETHOD_REF(br_output_facility_sdl, queryCapability),
};



