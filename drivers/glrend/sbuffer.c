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

static br_tv_template_entry bufferStoredTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),    F(identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {DEV(OPENGL_TEXTURE_U32), F(gl_tex),     BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY}
};

#undef F

/*
 * Set up a static device object
 */
struct br_buffer_stored *BufferStoredGLAllocate(br_renderer *renderer, br_token use, struct br_device_pixelmap *pm, br_token_value *tv)
{
    struct br_buffer_stored *self;
    char                    *ident;
    const GladGLContext     *gl = renderer->gl;

    switch(use) {

        case BRT_TEXTURE_O:
        case BRT_COLOUR_MAP_O:
            ident = "colour_map";
            break;

        case BRT_INDEX_SHADE_O:
            ident = "shade_table";
            break;

        case BRT_INDEX_BLEND_O:
            ident = "blend_table";
            break;

        case BRT_SCREEN_DOOR_O:
            ident = "screendoor_table";
            break;

        case BRT_INDEX_LIGHT_O:
            ident = "lighting_table";
            break;

        case BRT_BUMP_O:
            ident = "bump_map";
            break;

        case BRT_UNKNOWN:
            ident = "unknown";
            break;

        default:
            return NULL;
    }

    self = BrResAllocate(renderer, sizeof(*self), BR_MEMORY_OBJECT);
    if(self == NULL)
        return NULL;

    self->dispatch   = &bufferStoredDispatch;
    self->identifier = BrResSprintf(self, BR_GLREND_DEBUG_USER_PREFIX "%s:%s", ident, pm->pm_identifier ? pm->pm_identifier : "(unnamed)");
    self->device     = ObjectDevice(renderer);
    self->renderer   = renderer;
    self->gl         = gl;
    self->gl_tex     = 0;
    self->templates = BrTVTemplateAllocate(self, (br_tv_template_entry *)bufferStoredTemplateEntries, BR_ASIZE(bufferStoredTemplateEntries));

    BufferStoredUpdate(self, pm, tv);

    ObjectContainerAddFront(renderer, (br_object *)self);

    return self;
}

static br_error updateMemory(br_buffer_stored *self, br_pixelmap *pm)
{
    br_error                  berr;
    const br_pixelmap_gl_fmt *fmt;
    const GladGLContext      *gl = self->gl;

    /*
     * The pixelmap is a plain BRender memory pixelmap. Make sure that the pixels can be accessed
     */
    if((pm->flags & BR_PMF_NO_ACCESS) || pm->pixels == NULL)
        return BRE_FAIL;

    if((fmt = DeviceGLGetFormatDetails(pm->type)) == NULL)
        return BRE_FAIL;

    self->fmt = fmt;

    if(self->gl_tex == 0) {
        gl->GenTextures(1, &self->gl_tex);
    }

    if((berr = DeviceGLPixelmapToExistingGLTexture(gl, self->gl_tex, pm)) != BRE_OK) {
        return berr;
    }

    DeviceGLObjectLabel(gl, GL_TEXTURE, self->gl_tex, self->identifier);

    self->source       = pm;
    self->source_flags = pm->flags;
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_buffer_stored_gl, update)(struct br_buffer_stored *self, struct br_device_pixelmap *pm, br_token_value *tv)
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
        ASSERT(self->source == NULL || self->source == (br_pixelmap *)pm);
        self->gl_tex       = 0; /* Unused for us. */
        self->source       = (br_pixelmap *)pm;
        self->source_flags = pm->pm_flags;
        self->fmt          = DeviceGLGetFormatDetails(pm->pm_type);
        return BRE_OK;
    } else {
        /*
         * The pixelmap is from another device, we can't use it
         */
        return BRE_FAIL;
    }
}

static void BR_CMETHOD_DECL(br_buffer_stored_gl, free)(br_object *_self)
{
    br_buffer_stored    *self = (br_buffer_stored *)_self;
    const GladGLContext *gl   = self->gl;

    gl->DeleteTextures(1, &self->gl_tex);
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

static br_tv_template *BR_CMETHOD_DECL(br_buffer_stored_gl, templateQuery)(br_object *_self)
{
    return ((br_buffer_stored *)_self)->templates;
}

GLuint BufferStoredGLGetTexture(const br_buffer_stored *self)
{
    if(self->source == NULL)
        return self->gl_tex;

    if(ObjectDevice(self->source) == self->device) {
        br_device_pixelmap *pm = (br_device_pixelmap *)self->source;
        switch(pm->use_type) {
            case BRT_NONE:
            default:
                return 0;

            case BRT_OFFSCREEN:
                return pm->asBack.glTex;

            case BRT_DEPTH:
                return pm->asDepth.glDepth;
        }
    }

    return self->gl_tex;
}

GLuint BufferStoredGLGetCLUTTexture(const br_buffer_stored *self, br_device_pixelmap *target, GLuint fallback)
{
    GLuint clut;

    if(self == NULL)
        return fallback;

    /*
     * If we have a CLUT, use it.
     */
    if(self->source == NULL)
        goto use_target;

    if(self->source->map == NULL)
        goto use_target;

    if(self->source->map->stored == NULL)
        goto use_target;

    if((clut = BufferStoredGLGetTexture(self->source->map->stored)) != 0)
        return clut;

use_target:
    /*
     * If there's no target pixelmap, see if our renderer's current state has one.
     */
    if(target == NULL) {
        if(!(self->renderer->state.current->valid & BR_STATE_OUTPUT))
            return fallback;

        target = self->renderer->state.current->output.colour;
    }

    /*
     * Try to use the target's CLUT.
     */
    if(target == NULL)
        return fallback;

    if(target->use_type != BRT_OFFSCREEN)
        return fallback;

    if(target->asBack.clut == NULL)
        return fallback;

    if(target->asBack.clut->gl_tex == 0)
        return fallback;

    return target->asBack.clut->gl_tex;
}

/*
 * Default dispatch table for device
 */
static struct br_buffer_stored_dispatch bufferStoredDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_buffer_stored_gl, free),
    ._identifier = BR_CMETHOD_REF(br_buffer_stored_gl, identifier),
    ._type       = BR_CMETHOD_REF(br_buffer_stored_gl, type),
    ._isType     = BR_CMETHOD_REF(br_buffer_stored_gl, isType),
    ._device     = BR_CMETHOD_REF(br_buffer_stored_gl, device),
    ._space      = BR_CMETHOD_REF(br_buffer_stored_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_buffer_stored_gl, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._update = BR_CMETHOD_REF(br_buffer_stored_gl, update),
};
