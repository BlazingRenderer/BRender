/*
 * Primitive library methods
 */
#include "drv.h"
#include <brassert.h>

static const struct br_primitive_library_dispatch primitiveLibraryDispatch;

const br_token PrimPartsGL1xTokens[] = {BRT_PRIMITIVE, BRT_OUTPUT, 0};

#define F(f) offsetof(br_primitive_library, f)

static struct br_tv_template_entry primitiveLibraryTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR), F(identifier),                                               BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY},
    {BRT(PARTS_TL),        (br_uintptr_t)&PrimPartsGL1xTokens,                          BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_LIST},
    {BRT(PARTS_U32),       MASK_STATE_CACHE | MASK_STATE_OUTPUT | MASK_STATE_PRIMITIVE, BRTV_QUERY | BRTV_ABS,            BRTV_CONV_COPY},
};
#undef F

static struct br_tv_template primitiveLibraryTemplate = {BR_ASIZE(primitiveLibraryTemplateEntries), primitiveLibraryTemplateEntries};

static GLuint build_white_texture(const GladGLContext *gl)
{
    const static br_uint_8 white_rgba[] = {255, 255, 255, 255};
    GLuint                 tex;

    gl->GenTextures(1, &tex);
    gl->BindTexture(GL_TEXTURE_2D, tex);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_rgba);
    return tex;
}

br_primitive_library *PrimitiveLibraryGL1xAllocate(br_device_pixelmap *pm, const char *identifier, const GladGLContext *gl)
{
    br_primitive_library *self;

    self = BrResAllocate(pm, sizeof(*self), BR_MEMORY_OBJECT);

    if(self == NULL)
        return NULL;

    self->dispatch   = &primitiveLibraryDispatch;
    self->identifier = BrResStrDup(self, identifier);
    self->device     = ObjectDevice(pm);

    self->object_list = BrObjectListAllocate(self);
    self->gl          = gl;
    self->tex_white   = build_white_texture(gl);

    return self;
}

static const char *BR_CMETHOD_DECL(br_primitive_library_gl, identifier)(br_object *self)
{
    return ((br_primitive_library *)self)->identifier;
}

static br_device *BR_CMETHOD_DECL(br_primitive_library_gl, device)(br_object *self)
{
    return ((br_primitive_library *)self)->device;
}

static void BR_CMETHOD_DECL(br_primitive_library_gl, free)(br_object *_self)
{
    br_primitive_library *self = (br_primitive_library *)_self;

    self->gl->DeleteTextures(1, &self->tex_white);

    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_primitive_library_gl, type)(br_object *_self)
{
    return BRT_PRIMITIVE_LIBRARY;
}

static br_boolean BR_CMETHOD_DECL(br_primitive_library_gl, isType)(br_object *_self, br_token t)
{
    return (t == BRT_PRIMITIVE_LIBRARY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_primitive_library_gl, space)(br_object *_self)
{
    return sizeof(br_primitive_library);
}

static br_tv_template *BR_CMETHOD_DECL(br_primitive_library_gl, templateQuery)(br_object *_self)
{
    primitiveLibraryTemplate.res = DeviceGLResource(((br_primitive_library *)_self)->device);
    return &primitiveLibraryTemplate;
}

static void *BR_CMETHOD_DECL(br_primitive_library_gl, listQuery)(br_object_container *_self)
{
    return ((br_primitive_library *)_self)->object_list;
}

static br_error BR_CMETHOD_DECL(br_primitive_library_gl, stateNew)(br_primitive_library *self, br_primitive_state **rps)
{
    UASSERT(rps);

    if((*rps = PrimitiveStateGL1xAllocate(self)) == NULL)
        return BRE_DEV_FAIL;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_primitive_library_gl, bufferStoredNew)(br_primitive_library *self, br_buffer_stored **psm, br_token use,
                                                                          br_device_pixelmap *pm, br_token_value *tv)
{
    UASSERT(psm);

    if((*psm = BufferStoredGL1xAllocate(self, use, pm, tv)) == NULL)
        return BRE_DEV_FAIL;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_primitive_library_gl, bufferStoredAvail)(br_primitive_library *self, br_int_32 *space, br_token use,
                                                                            br_token_value *tv)
{
    (void)self;
    (void)space;
    (void)use;
    (void)tv;
    return BRE_DEV_UNSUPPORTED;
}

static br_error BR_CMETHOD_DECL(br_primitive_library_gl, flush)(br_primitive_library *self, br_boolean wait)
{
    (void)wait;

    self->gl->Flush();
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_primitive_library_gl, synchronise)(br_primitive_library *self, br_token sync_type, br_boolean block)
{
    (void)sync_type;
    (void)block;

    self->gl->Finish();
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_primitive_library_gl, mask)(br_primitive_library *self, br_uint_32 *mask, br_token *parts, int n_parts)
{
    int        i;
    br_uint_32 m = 0;
    (void)self;

    for(i = 0; i < n_parts; i++) {
        switch(parts[i]) {
            case BRT_PRIMITIVE:
                m |= MASK_STATE_PRIMITIVE;
                break;
            case BRT_OUTPUT:
                m |= MASK_STATE_OUTPUT;
                break;
        }
    }
    *mask = m;
    return BRE_OK;
}

static const struct br_primitive_library_dispatch primitiveLibraryDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_primitive_library_gl, free),
    ._identifier = BR_CMETHOD_REF(br_primitive_library_gl, identifier),
    ._type       = BR_CMETHOD_REF(br_primitive_library_gl, type),
    ._isType     = BR_CMETHOD_REF(br_primitive_library_gl, isType),
    ._device     = BR_CMETHOD_REF(br_primitive_library_gl, device),
    ._space      = BR_CMETHOD_REF(br_primitive_library_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_primitive_library_gl, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery        = BR_CMETHOD_REF(br_primitive_library_gl, listQuery),
    ._tokensMatchBegin = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch      = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd   = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._addFront         = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront      = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove           = BR_CMETHOD_REF(br_object_container, remove),
    ._find             = BR_CMETHOD_REF(br_object_container, find),
    ._findMany         = BR_CMETHOD_REF(br_object_container, findMany),
    ._count            = BR_CMETHOD_REF(br_object_container, count),

    ._stateNew          = BR_CMETHOD_REF(br_primitive_library_gl, stateNew),
    ._bufferStoredNew   = BR_CMETHOD_REF(br_primitive_library_gl, bufferStoredNew),
    ._bufferStoredAvail = BR_CMETHOD_REF(br_primitive_library_gl, bufferStoredAvail),
    ._flush             = BR_CMETHOD_REF(br_primitive_library_gl, flush),
    ._synchronise       = BR_CMETHOD_REF(br_primitive_library_gl, synchronise),
    ._mask              = BR_CMETHOD_REF(br_primitive_library_gl, mask),
};
