/*
 * Stored buffer methods
 */
#include <stddef.h>

#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for primitive state (defined at end of file)
 */
static struct br_buffer_stored_dispatch bufferStoredDispatch;


/*
 * Primitive state info. template
 */
#define F(f) offsetof(struct br_buffer_stored, f)

static struct br_tv_template_entry bufferStoredTemplateEntries[] = {
    {BRT_IDENTIFIER_CSTR, NULL, F(identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};

#undef F


/*
 * Set up a static device object
 */
struct br_buffer_stored *BufferStoredGLAllocate(br_renderer *renderer,
                                                br_token use, struct br_device_pixelmap *pm, br_token_value *tv)
{
    struct br_buffer_stored *self;
    char                    *ident;

    switch(use) {

        case BRT_TEXTURE_O:
        case BRT_COLOUR_MAP_O:
            ident = "Colour-Map";
            break;

        case BRT_INDEX_SHADE_O:
            ident = "Shade-Table";
            break;

        case BRT_INDEX_BLEND_O:
            ident = "Blend-Table";
            break;

        case BRT_SCREEN_DOOR_O:
            ident = "Screendoor-Table";
            break;

        case BRT_INDEX_LIGHT_O:
            ident = "Lighting-Table";
            break;

        case BRT_BUMP_O:
            ident = "Bump-Map";
            break;

        case BRT_UNKNOWN:
            ident = "Unknown";
            break;

        default:
            return NULL;
    }

    self = BrResAllocate(renderer, sizeof(*self), BR_MEMORY_OBJECT);
    if(self == NULL)
        return NULL;

    self->dispatch   = &bufferStoredDispatch;
    self->identifier = ident;
    self->device     = ObjectDevice(renderer);
    self->renderer   = renderer;
    self->gl_tex     = 0;
    self->templates  = BrTVTemplateAllocate(
        self,
        (br_tv_template_entry *)bufferStoredTemplateEntries,
        BR_ASIZE(bufferStoredTemplateEntries)
    );

    BufferStoredUpdate(self, pm, tv);

    ObjectContainerAddFront(renderer, (br_object *)self);

    return self;
}

static br_boolean is_compatible(br_buffer_stored *self, br_pixelmap *pm, GLenum internal_format)
{
    if(self->source == NULL)
        return BR_FALSE;

    if(self->gl_tex == 0 || self->gl_internal_format == 0 || self->gl_format == 0 || self->gl_type == 0)
        return BR_FALSE;

    if(self->source->width != pm->width || self->source->height != pm->height)
        return BR_FALSE;

    if(self->gl_internal_format != internal_format)
        return BR_FALSE;

    return BR_TRUE;
}

static const char *gl_strerror(GLenum err)
{
    static char errbuf[64];
    const char  *s;

    switch(err) {
        case 0:
            s = "GL_NO_ERROR";
            break;
        case GL_INVALID_ENUM:
            s = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            s = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            s = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            s = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            s = "GL_OUT_OF_MEMORY";
            break;
        default:
            s = NULL;
    }

    if(s != NULL)
        BrSprintfN(errbuf, sizeof(errbuf) - 1, "error %d (%s)", err, s);
    else
        BrSprintfN(errbuf, sizeof(errbuf) - 1, "error %d", err);

    errbuf[sizeof(errbuf) - 1] = '\0';
    return errbuf;
}

static br_error updateMemory(br_buffer_stored *self, br_pixelmap *pm)
{
    GLint      internal_format;
    GLenum     format, type, err;
    GLsizeiptr elem_bytes;
    br_error   r;

    /*
     * The pixelmap is a plain BRender memory pixelmap. Make sure that the pixels can be accessed
     */
    if((pm->flags & BR_PMF_NO_ACCESS) || pm->pixels == NULL)
        return BRE_FAIL;

    r = VIDEOI_BrPixelmapGetTypeDetails(pm->type, &internal_format, &format, &type, &elem_bytes, &self->blended);
    if(r != BRE_OK)
        return r;

    /*
     * If we're compatible, update the existing texture.
     */
    if(is_compatible(self, pm, internal_format) == BR_TRUE) {
        ASSERT(self->gl_tex != 0);

        // TODO:
        BrDebugBreak();

        return BRE_OK;
    }

    if(self->gl_tex == 0) {
        glGenTextures(1, &self->gl_tex);
        if((err = glGetError()) != 0) {
            BrLogError("GLREND", "glGenTextures() failed with %s", gl_strerror(err));
            return BRE_FAIL;
        }
    }

    glBindTexture(GL_TEXTURE_2D, self->gl_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, pm->width, pm->height, 0, format, type, pm->pixels);

    if((err = glGetError()) != 0) {
        BrLogError("GLREND", "glGenTextures() failed with %s", gl_strerror(err));
        glBindTexture(GL_TEXTURE_2D, 0);
        return BRE_FAIL;
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    self->source       = pm;
    self->source_flags = pm->flags;

    glBindTexture(GL_TEXTURE_2D, 0);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_buffer_stored_gl, update)(
    struct br_buffer_stored *self,
    struct br_device_pixelmap *pm,
    br_token_value *tv)
{
    br_device *pm_device;
    (void)tv;

    /*
     * Find out where the pixelmap comes from
     */
    pm_device = ObjectDevice(pm);
    if(pm_device == NULL) {
        return updateMemory(self, (br_pixelmap *)pm);
    } else if(pm_device == self->device) {
        /*
         * The pixelmaps came from this device, copy it directly.
         * TODO: this
         */
        return BRE_FAIL;
    } else {
        /*
         * The pixelmap is from another device, we can't use it
         */
        return BRE_FAIL;
    }
}

static void BR_CMETHOD_DECL(br_buffer_stored_gl, free)(br_object *_self)
{
    br_buffer_stored *self = (br_buffer_stored *)_self;

    glDeleteTextures(1, &self->gl_tex);
    self->gl_tex = 0;

    ObjectContainerRemove(self->renderer, (br_object *)self);
}

static const char *BR_CMETHOD_DECL(br_buffer_stored_gl, identifier)(br_object *self)
{
    return ((br_buffer_stored *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_buffer_stored_gl, type)(br_object *self)
{
    (void)self;
    return BRT_BUFFER_STORED;
}

static br_boolean BR_CMETHOD_DECL(br_buffer_stored_gl, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_BUFFER_STORED) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_buffer_stored_gl, device)(br_object *self)
{
    return ((br_buffer_stored *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_buffer_stored_gl, space)(br_object *self)
{
    return BrResSizeTotal(self);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_buffer_stored_gl, templateQuery)(br_object *_self)
{
    return ((br_buffer_stored *)_self)->templates;
}

/*
 * Default dispatch table for device
 */
static struct br_buffer_stored_dispatch bufferStoredDispatch = {
    .__reserved0    = NULL,
    .__reserved1    = NULL,
    .__reserved2    = NULL,
    .__reserved3    = NULL,
    ._free          = BR_CMETHOD_REF(br_buffer_stored_gl, free),
    ._identifier    = BR_CMETHOD_REF(br_buffer_stored_gl, identifier),
    ._type          = BR_CMETHOD_REF(br_buffer_stored_gl, type),
    ._isType        = BR_CMETHOD_REF(br_buffer_stored_gl, isType),
    ._device        = BR_CMETHOD_REF(br_buffer_stored_gl, device),
    ._space         = BR_CMETHOD_REF(br_buffer_stored_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_buffer_stored_gl, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._update        = BR_CMETHOD_REF(br_buffer_stored_gl, update),
};
