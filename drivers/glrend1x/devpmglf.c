/*
 * Device pixelmap implementation, front/screen edition.
 */

#include "drv.h"
#include <brassert.h>
#include <string.h>

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_pixelmap_dispatch devicePixelmapFrontDispatch;

/*
 * Device pixelmap info. template
 */
#define F(f)  offsetof(br_device_pixelmap, f)
#define FF(f) offsetof(br_device_pixelmap, asFront.f)
static br_tv_template_entry devicePixelmapFrontTemplateEntries[] = {
    {BRT(WIDTH_I32),                 F(pm_width),                BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT(HEIGHT_I32),                F(pm_height),               BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT(PIXEL_TYPE_U8),             F(pm_type),                 BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8 },
    {BRT(OUTPUT_FACILITY_O),         F(output_facility),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT(FACILITY_O),                F(output_facility),         BRTV_QUERY,            BRTV_CONV_COPY   },
    {BRT(PRIMITIVE_LIBRARY_O),       F(primitive_library),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT(IDENTIFIER_CSTR),           F(pm_identifier),           BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT(OPENGL_EXT_PROCS_P),        F(asFront.ext_procs),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_PTR    },
    {BRT(OPENGL_VERSION_CSTR),       FF(gl_version),             BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT(OPENGL_VENDOR_CSTR),        FF(gl_vendor),              BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT(OPENGL_RENDERER_CSTR),      FF(gl_renderer),            BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },

    {DEV(OPENGL_NUM_EXTENSIONS_I32), FF(gl_num_extensions),      BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {DEV(OPENGL_EXTENSIONS_PL),      FF(gl_extensions),          BRTV_QUERY | BRTV_ALL, BRTV_CONV_LIST   },
    {DEV(OPENGL_NATIVE_CONTEXT_P),   F(asFront.native_context),  BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {DEV(OPENGL_GLAD_CONTEXT_P),     F(asFront.glad_gl_context), BRTV_QUERY | BRTV_ALL, BRTV_CONV_PTR    },
};
#undef FF
#undef F

struct pixelmapNewTokens {
    br_int_32               width;
    br_int_32               height;
    br_int_32               pixel_bits;
    br_uint_8               pixel_type;
    br_device_gl_ext_procs *ext_procs;
};

#define F(f) offsetof(struct pixelmapNewTokens, f)
static br_tv_template_entry pixelmapNewTemplateEntries[] = {
    {BRT(WIDTH_I32),          F(width),      BRTV_SET, BRTV_CONV_COPY},
    {BRT(HEIGHT_I32),         F(height),     BRTV_SET, BRTV_CONV_COPY},
    {BRT(PIXEL_BITS_I32),     F(pixel_bits), BRTV_SET, BRTV_CONV_COPY},
    {BRT(PIXEL_TYPE_U8),      F(pixel_type), BRTV_SET, BRTV_CONV_COPY},
    {BRT(OPENGL_EXT_PROCS_P), F(ext_procs),  BRTV_SET, BRTV_CONV_COPY},
};
#undef F

static char **build_extensions_list(void *vparent, const GladGLContext *gl, GLint *p_num_extensions)
{
    const char *src;
    char       *buf;
    char      **extensions     = NULL;
    GLint       num_extensions = 0;

    if((src = (char *)gl->GetString(GL_EXTENSIONS)) == NULL)
        src = "";

    buf = BrResStrDup(vparent, src);

    for(char *p = buf; *p; ++p)
        if(*p == ' ')
            ++num_extensions;
    num_extensions += 1;

    extensions = BrResAllocate(vparent, sizeof(char *) * (num_extensions + 1), BR_MEMORY_DRIVER);

    num_extensions = 0;
    for(char *p = buf; *p;) {
        while(*p == ' ')
            *p++ = '\0';
        if(*p == '\0')
            break;
        extensions[num_extensions++] = p;
        while(*p && *p != ' ')
            ++p;
    }
    extensions[num_extensions] = NULL;

    *p_num_extensions = num_extensions;
    return extensions;
}

br_device_pixelmap *DevicePixelmapGL1xAllocateFront(br_device *dev, br_output_facility *outfcty, br_token_value *tv)
{
    br_device_pixelmap       *self;
    br_int_32                 count;
    const br_pixelmap_gl_fmt *fmt;
    br_device_gl_context_info context_info;
    int                       glad_version, glad_major, glad_minor;
    const GladGLContext      *gl;
    struct pixelmapNewTokens  pt = {
         .width      = -1,
         .height     = -1,
         .pixel_bits = -1,
         .pixel_type = BR_PMT_MAX,
         .ext_procs  = NULL,
    };

    if(dev->templates.pixelmapNewTemplate == NULL) {
        dev->templates.pixelmapNewTemplate = BrTVTemplateAllocate(dev, pixelmapNewTemplateEntries, BR_ASIZE(pixelmapNewTemplateEntries));
    }

    BrTokenValueSetMany(&pt, &count, NULL, tv, dev->templates.pixelmapNewTemplate);

    if(pt.ext_procs == NULL || pt.width <= 0 || pt.height <= 0)
        return NULL;

    if((pt.pixel_type = DeviceGL1xTypeOrBits(pt.pixel_type, pt.pixel_bits)) == BR_PMT_MAX)
        return NULL;

    if((fmt = DeviceGL1xGetFormatDetails(pt.pixel_type)) == NULL)
        return NULL;

    /*
     * Refuse creation of an indexed front screen.
     * Have these ever existed?
     */
    if(fmt->indexed)
        return NULL;

    self                  = BrResAllocate(dev->res, sizeof(br_device_pixelmap), BR_MEMORY_OBJECT);
    self->pm_identifier   = BrResSprintf(self, "OpenGL:Screen:%dx%d", pt.width, pt.height);
    self->dispatch        = &devicePixelmapFrontDispatch;
    self->device          = dev;
    self->output_facility = outfcty;
    self->use_type        = BRT_NONE;
    self->screen          = self;

    self->pm_type   = pt.pixel_type;
    self->pm_width  = pt.width;
    self->pm_height = pt.height;
    self->pm_flags |= BR_PMF_NO_ACCESS;

    self->num_refs = 0;

    /*
     * Make a copy, so they can't switch things out from under us.
     */
    self->asFront.ext_procs = *pt.ext_procs;

    if(DevicePixelmapGL1xExtCreateContext(self, &context_info) != BRE_OK) {
        BrResFreeNoCallback(self);
        return NULL;
    }
    self->asFront.native_context = context_info.native;

    BrLogInfo("GLREND", "Created an OpenGL%s %d.%d%s context.", context_info.profile == BRT_OPENGL_PROFILE_ES ? " ES" : "",
              context_info.major, context_info.minor,
              context_info.profile == BRT_OPENGL_PROFILE_COMPATIBILITY
                  ? " (Compatibility Profile)"
                  : (context_info.profile == BRT_OPENGL_PROFILE_CORE ? " (Core Profile)" : ""));

    if(context_info.major < 1 || (context_info.minor == 1 && context_info.minor < 2)) {
        BrLogError("GLREND", "Got an OpenGL %d.%d context, expected 1.2", context_info.major, context_info.minor);
        goto cleanup_context;
    }

    if(DevicePixelmapGL1xExtMakeCurrent(self, self->asFront.native_context) != BRE_OK)
        goto cleanup_context;

    if((glad_version = gladLoadGLContextUserPtr(&self->asFront.glad_gl_context, DevicePixelmapGL1xExtGetGetProcAddress(self), self)) == 0) {
        BrLogError("GLREND", "Unable to load OpenGL functions.");
        goto cleanup_context;
    }

    glad_major = GLAD_VERSION_MAJOR(glad_version);
    glad_minor = GLAD_VERSION_MINOR(glad_version);

    gl = &self->asFront.glad_gl_context;

    self->asFront.gl_version  = BrResStrDup(self, (const char *)gl->GetString(GL_VERSION));
    self->asFront.gl_vendor   = BrResStrDup(self, (const char *)gl->GetString(GL_VENDOR));
    self->asFront.gl_renderer = BrResStrDup(self, (const char *)gl->GetString(GL_RENDERER));

    /*
     * Get a copy of the extension list.
     * NULL-terminate so we can expose it as a BRT_POINTER_LIST.
     */
    self->asFront.gl_extensions = build_extensions_list(self, gl, &self->asFront.gl_num_extensions);

    BrLogInfo("GLREND", "OpenGL Version  = %s", self->asFront.gl_version);
    BrLogInfo("GLREND", "OpenGL Vendor   = %s", self->asFront.gl_vendor);
    BrLogInfo("GLREND", "OpenGL Renderer = %s", self->asFront.gl_renderer);

    if(glad_major < 1 || (glad_major == 1 && glad_minor < 2)) {
        BrLogError("GLREND", "Got OpenGL %d.%d context, expected 1.2", glad_major, glad_minor);
        goto cleanup_context;
    }

    /*
     * Create the GL primitives.
     * 1. It needs access to the GL context, and
     * 2. We need to expose a token for it, so devsetup.c can find it and not fall through to softprmf.
     */
    if((self->primitive_library = PrimitiveLibraryGL1xAllocate(self, "OpenGL-Primitives", gl)) == NULL) {
        BrLogError("GLREND", "Unable to init OpenGL primitives.");
        goto cleanup_context;
    }

    ObjectContainerAddFront(self->output_facility, (br_object *)self);

    self->pm_type = BR_PMT_RGBX_888;

    return self;

cleanup_context:
    DevicePixelmapGL1xExtDeleteContext(self, self->asFront.native_context);
    DevicePixelmapGL1xExtFree(self);
    BrResFreeNoCallback(self);

    return NULL;
}

static void BR_CMETHOD_DECL(br_device_pixelmap_glf, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    BrLogTrace("GLREND", "Freeing %s", self->pm_identifier);

    UASSERT(self->num_refs == 0);

    BrResFree(self->primitive_library);

    DevicePixelmapGL1xExtDeleteContext(self, self->asFront.native_context);

    ObjectContainerRemove(self->output_facility, (br_object *)self);

    DevicePixelmapGL1xExtFree(self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_pixelmap_glf, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_glf, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_glf, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_glf, device)(br_object *self)
{
    (void)self;
    return ((br_device_pixelmap *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_glf, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device_pixelmap);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_glf, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapFrontTemplate == NULL)
        self->device->templates.devicePixelmapFrontTemplate = BrTVTemplateAllocate(self->device, devicePixelmapFrontTemplateEntries,
                                                                                   BR_ASIZE(devicePixelmapFrontTemplateEntries));

    return self->device->templates.devicePixelmapFrontTemplate;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_glf, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    br_error err;

    if((err = DevicePixelmapGL1xExtResize(self, width, height)) != BRE_OK)
        return err;

    /*
     * Resizing, assumed to have been done externally
     * via whatever window we're representing.
     */
    self->pm_width  = width;
    self->pm_height = height;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_glf, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{

    /*
     * Ignore self-blit.
     */
    if(self == src)
        return BRE_OK;

    if(ObjectDevice(src) != self->device)
        return BRE_UNSUPPORTED;

    if(self->use_type != BRT_NONE || src->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    /*
     * Call our pre-swap hook.
     */
    DevicePixelmapGL1xExtPreSwap(self, 0);

    /*
     * Finally, swap the buffers.
     */
    DevicePixelmapGL1xExtSwapBuffers(self);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_glf, handleWindowEvent)(br_device_pixelmap *self, void *arg)
{
    return DevicePixelmapGL1xExtHandleWindowEvent(self, arg);
}

/*
 * Default dispatch table for device pixelmap
 */
static const struct br_device_pixelmap_dispatch devicePixelmapFrontDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_glf, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_glf, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_glf, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_glf, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_glf, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_glf, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_glf, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_null, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_glf, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_gl1x, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_fail, allocateSub),

    ._copy         = BR_CMETHOD_REF(br_device_pixelmap_fail, copy),
    ._copyTo       = BR_CMETHOD_REF(br_device_pixelmap_fail, copyTo),
    ._copyFrom     = BR_CMETHOD_REF(br_device_pixelmap_fail, copyFrom),
    ._fill         = BR_CMETHOD_REF(br_device_pixelmap_fail, fill),
    ._doubleBuffer = BR_CMETHOD_REF(br_device_pixelmap_glf, doubleBuffer),

    ._copyDirty         = BR_CMETHOD_REF(br_device_pixelmap_fail, copyDirty),
    ._copyToDirty       = BR_CMETHOD_REF(br_device_pixelmap_fail, copyToDirty),
    ._copyFromDirty     = BR_CMETHOD_REF(br_device_pixelmap_fail, copyFromDirty),
    ._fillDirty         = BR_CMETHOD_REF(br_device_pixelmap_fail, fillDirty),
    ._doubleBufferDirty = BR_CMETHOD_REF(br_device_pixelmap_fail, doubleBufferDirty),

    ._rectangle                = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangle),
    ._rectangle2               = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangle2),
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleCopyTo),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyTo),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_fail, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_fail, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_fail, text),
    ._textBounds = BR_CMETHOD_REF(br_device_pixelmap_gen, textBounds),

    ._rowSize  = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSize),
    ._rowQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, rowQuery),
    ._rowSet   = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSet),

    ._pixelQuery        = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelQuery),
    ._pixelAddressQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressQuery),

    ._pixelAddressSet = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressSet),
    ._originSet       = BR_CMETHOD_REF(br_device_pixelmap_gen, originSet),

    ._flush        = BR_CMETHOD_REF(br_device_pixelmap_fail, flush),
    ._synchronise  = BR_CMETHOD_REF(br_device_pixelmap_fail, synchronise),
    ._directLock   = BR_CMETHOD_REF(br_device_pixelmap_fail, directLock),
    ._directUnlock = BR_CMETHOD_REF(br_device_pixelmap_fail, directUnlock),
    ._getControls  = BR_CMETHOD_REF(br_device_pixelmap_fail, getControls),
    ._setControls  = BR_CMETHOD_REF(br_device_pixelmap_fail, setControls),

    ._handleWindowEvent = BR_CMETHOD_REF(br_device_pixelmap_glf, handleWindowEvent),
};
