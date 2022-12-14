/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: renderer.c 1.5 1998/07/22 14:22:36 johng Exp $
 * $Locker: $
 *
 * Renderer methods
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

BR_RCS_ID("$Id: renderer.c 1.5 1998/07/22 14:22:36 johng Exp $");

/*
 * Default dispatch table for renderer (defined at end of file)
 */
static const struct br_renderer_dispatch rendererDispatch;

/*
 * 0 terminated list of tokens that are used to represent the state for
 * this library
 */
const br_token RendererPartsTokens[] = {
	BRT_CULL,
	BRT_SURFACE,
	BRT_MATRIX,
	BRT_ENABLE,
	BRT_LIGHT,
	BRT_CLIP,
	0
};

/*
 * Renderer info. template
 */

/*
 * Return a token list of ourself and that of the primitive library
 */
static br_error BR_CALLBACK customPartsQuery(br_value *pvalue, void **pextra, br_size_t *pextra_size,
	void *block, const struct br_tv_template_entry *tep)
{
	struct br_renderer *self = block;
	int i;
	br_uint_32 dummy;
	br_error r;
	br_token **ppt = (br_token **)pextra;

	if(pextra == NULL || *pextra == NULL || pextra_size == NULL)
		return BRE_FAIL;

	*((void **)pvalue) = *pextra;

	/*
	 * Check there is space 
	 */
	if(((BR_ASIZE(RendererPartsTokens)-1) * sizeof(br_token)) > *pextra_size)
		return BRE_FAIL;

	/*
	 * Add our own tokens to list
	 */
	for(i=0; i < BR_ASIZE(RendererPartsTokens)-1; i++) {
		*(*ppt)++ = RendererPartsTokens[i];
		*pextra_size -= sizeof(br_token);
	}
	
	/*
	 * Hand off to primitive library for rest of list
	 */
	r = ObjectQueryBuffer(self->plib, &dummy, *ppt, *pextra_size, BRT_PARTS_TL);

	if(r != BRE_OK)
		return r;

	/*
	 * Work out how much space was consumed
	 */
	for(i = 0; (*ppt)[i] ; i++)
		*pextra_size -= sizeof(br_token);
		
	/*
	 * NULL
	 */
	*pextra_size -= sizeof(br_token);
		
	return BRE_OK;
}

static br_size_t BR_CALLBACK customPartsExtra(void *block, const struct br_tv_template_entry *tep)
{
	struct br_renderer *self = block;
	br_size_t s;
	br_token_value tv[] = {
		{BRT_PARTS_TL,0},
		{0,0}
	};

	/*
	 * Find out size of primitive's list
	 */
	if(ObjectQueryManySize((br_object *)self->plib, &s, tv) != BRE_OK)
		return 0;

	/*
	 * Add our own contribution
	 */
	return s + (BR_ASIZE(RendererPartsTokens)-1) * sizeof(br_token);
}

static struct br_tv_custom customPartsConv = {
    .query      = customPartsQuery,
    .set        = NULL,
    .extra_size = customPartsExtra,
};

#define F(f)	offsetof(struct br_renderer, f)

/*
 * Shortcuts for template flags
 */
#define _S BRTV_SET
#define _Q BRTV_QUERY
#define _A BRTV_ALL

#if BASED_FIXED
#define _AX BRTV_ALL
#else 
#define _AX 0
#endif

#if BASED_FLOAT
#define _AF BRTV_ALL
#else 
#define _AF 0
#endif

static struct br_tv_template_entry rendererTemplateEntries[] = {
	{BRT(IDENTIFIER_CSTR),		F(identifier),				_Q | _A,	BRTV_CONV_COPY, },
	{BRT(PARTS_TL),				0,							_Q | _A,	BRTV_CONV_CUSTOM, (br_uintptr_t)&customPartsConv},

	{BRT(RENDERER_FACILITY_O),	F(renderer_facility),		_Q | _A,	BRTV_CONV_COPY, },

	/*
	 * Useful stuff for poking around
	 */
	{BRT(PRIMITIVE_LIBRARY_O),	F(plib),					_Q | _A,	BRTV_CONV_COPY, },
	{DEV(PRIMITIVE_STATE_O),	F(state.pstate),			_Q | _A,	BRTV_CONV_COPY, },
};
#undef F

/*
 * Create a new renderer
 */
br_renderer * RendererSoftAllocate(br_device *dev, struct br_renderer_facility *type, struct br_primitive_library *prims)
{
	br_renderer *self;
	br_uint_32 m;

	ASSERT(prims != NULL);
	
	self = BrResAllocate(dev, sizeof(*self), BR_MEMORY_OBJECT_DATA);

	if(self == NULL)
		return NULL;

	self->dispatch = (struct br_renderer_dispatch *)&rendererDispatch;
 	self->identifier = type->identifier;
    self->device = dev;
 	self->renderer_facility = type;

	/*
	 * Set up out object list
	 */
	self->object_list = BrObjectListAllocate(self);

	/*
	 * Set the primitives that this renderer will use
	 */
	self->plib = prims;

	/*
	 * Remember default state
	 */
	self->default_state = &type->default_state;
 	self->state.renderer = self;

	/*
	 * Set valid bits from primitive library
	 */
	m = 0;
	ObjectQuery(self->plib, &m, BRT_PARTS_U32);
	self->state.valid |= m;

	/*
	 * State starts out as default
	 */
	RendererStateDefault(self,(br_uint_32)BR_STATE_ALL);

	ObjectContainerAddFront(type,(br_object *)self);

	return self;
}

static void BR_CMETHOD_DECL(br_renderer_soft, free)(br_object *_self)
{
	br_renderer *self = (br_renderer*)_self;

	ObjectContainerRemove(self->renderer_facility, (br_object *)self);

	self->renderer_facility->num_instances--;

	BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

	BrResFreeNoCallback(self);
}

static br_token BR_CMETHOD_DECL(br_renderer_soft, type)(br_object *self)
{
	return BRT_RENDERER;
}

static br_boolean BR_CMETHOD_DECL(br_renderer_soft, isType)(br_object *self, br_token t)
{
	return (t == BRT_RENDERER) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_renderer_soft, space)(br_object *self)
{
	return sizeof(br_renderer);
}

static struct br_tv_template * BR_CMETHOD_DECL(br_renderer_soft,templateQuery)(br_object *_self)
{
    br_renderer *self = (br_renderer*)_self;

    if(self->device->templates.rendererTemplate == NULL)
        self->device->templates.rendererTemplate = BrTVTemplateAllocate(self->device,
            rendererTemplateEntries,
            BR_ASIZE(rendererTemplateEntries));

    return self->device->templates.rendererTemplate;
}

static void * BR_CMETHOD_DECL(br_renderer_soft,listQuery)(br_object_container *self)
{
	return ((br_renderer*)self)->object_list;
}

static br_error BR_CMETHOD_DECL(br_renderer_soft, validDestination)(br_renderer *self, br_boolean *bp, br_object *h)
{
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_soft, stateStoredNew)
	(struct br_renderer *self, struct br_renderer_state_stored **pss, br_uint_32 mask, br_token_value *tv)
{
	br_renderer_state_stored *ss;

	ss = RendererStateStoredSoftAllocate(self, &self->state, mask, tv);

	if(ss) {
		*pss = ss;
		return BRE_OK;
	} else
		return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_soft, stateStoredAvail)
	(struct br_renderer *self, br_int_32 *psize, br_uint_32 mask, br_token_value *tv)
{
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_soft, bufferStoredNew)
	(struct br_renderer *self, struct br_buffer_stored **psm,
	br_token use, struct br_device_pixelmap *pm, br_token_value *tv)
{
	/*
	 * Pass on to primitive library
	 */
	return PrimitiveLibraryBufferStoredNew(self->plib, psm, use, pm, tv);
}

static br_error BR_CMETHOD_DECL(br_renderer_soft, bufferStoredAvail)(
	struct br_renderer *self,
		br_int_32 *space,
		br_token use,
		br_token_value *tv)
{
	/*
	 * Pass on to primitive library
	 */
	return PrimitiveLibraryBufferStoredAvail(self->plib, space, use, tv);
}

static br_error BR_CMETHOD_DECL(br_renderer_soft, flush)
	(struct br_renderer *self, br_boolean wait)
{
	/*
	 * Pass on to primitive library
	 */
	return PrimitiveLibraryFlush(self->plib,wait);
}

static br_error BR_CMETHOD_DECL(br_renderer_soft, synchronise)
	(struct br_renderer *self, br_token sync_type, br_boolean block)
{
	/*
	 * Pass on to primitive library
	 */
	return PrimitiveLibrarySynchronise(self->plib, sync_type, block);
}

static br_error BR_CMETHOD_DECL(br_renderer_soft,commandModeSet)
	(struct br_renderer *self, br_token mode)
{
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_soft,commandModeQuery)
	(struct br_renderer *self, br_token *mode)
{
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_soft,commandModeDefault)
	(struct br_renderer *self)
{
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_soft,commandModePush)
	(struct br_renderer *self)
{
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_soft,commandModePop)
	(struct br_renderer *self)
{
	return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_renderer_soft, frameBegin)(br_renderer *self)
{
	return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_renderer_soft, frameEnd)(br_renderer *self)
{
	return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_renderer_soft, focusLossBegin)(br_renderer *self)
{
	return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_renderer_soft, focusLossEnd)(br_renderer *self)
{
	return BRE_OK;
}

/*
 * Default dispatch table for renderer
 */
static const struct br_renderer_dispatch rendererDispatch = {
    .__reserved0       = NULL,
    .__reserved1       = NULL,
    .__reserved2       = NULL,
    .__reserved3       = NULL,
    ._free             = BR_CMETHOD_REF(br_renderer_soft, free),
    ._identifier       = BR_CMETHOD_REF(br_object_soft, identifier),
    ._type             = BR_CMETHOD_REF(br_renderer_soft, type),
    ._isType           = BR_CMETHOD_REF(br_renderer_soft, isType),
    ._device           = BR_CMETHOD_REF(br_object_soft, device),
    ._space            = BR_CMETHOD_REF(br_renderer_soft, space),
    ._templateQuery    = BR_CMETHOD_REF(br_renderer_soft, templateQuery),
    ._query            = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer      = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany        = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize    = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll         = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize     = BR_CMETHOD_REF(br_object, queryAllSize),
    ._listQuery        = BR_CMETHOD_REF(br_renderer_soft, listQuery),
    ._tokensMatchBegin = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch      = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd   = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._addFront         = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront      = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove           = BR_CMETHOD_REF(br_object_container, remove),
    ._find             = BR_CMETHOD_REF(br_object_container, find),
    ._findMany         = BR_CMETHOD_REF(br_object_container, findMany),
    ._count            = BR_CMETHOD_REF(br_object_container, count),

    ._validDestination      = BR_CMETHOD_REF(br_renderer_soft, validDestination),
    ._stateStoredNew        = BR_CMETHOD_REF(br_renderer_soft, stateStoredNew),
    ._stateStoredAvail      = BR_CMETHOD_REF(br_renderer_soft, stateStoredAvail),
    ._bufferStoredNew       = BR_CMETHOD_REF(br_renderer_soft, bufferStoredNew),
    ._bufferStoredAvail     = BR_CMETHOD_REF(br_renderer_soft, bufferStoredAvail),
    ._partSet               = BR_CMETHOD_REF(br_renderer_soft, partSet),
    ._partSetMany           = BR_CMETHOD_REF(br_renderer_soft, partSetMany),
    ._partQuery             = BR_CMETHOD_REF(br_renderer_soft, partQuery),
    ._partQueryBuffer       = BR_CMETHOD_REF(br_renderer_soft, partQueryBuffer),
    ._partQueryMany         = BR_CMETHOD_REF(br_renderer_soft, partQueryMany),
    ._partQueryManySize     = BR_CMETHOD_REF(br_renderer_soft, partQueryManySize),
    ._partQueryAll          = BR_CMETHOD_REF(br_renderer_soft, partQueryAll),
    ._partQueryAllSize      = BR_CMETHOD_REF(br_renderer_soft, partQueryAllSize),
    ._partIndexQuery        = BR_CMETHOD_REF(br_renderer_soft, partIndexQuery),
    ._modelMul              = BR_CMETHOD_REF(br_renderer_soft, modelMulF),
    ._modelPopPushMul       = BR_CMETHOD_REF(br_renderer_soft, modelPopPushMulF),
    ._modelInvert           = BR_CMETHOD_REF(br_renderer_soft, modelInvert),
    ._statePush             = BR_CMETHOD_REF(br_renderer_soft, statePush),
    ._statePop              = BR_CMETHOD_REF(br_renderer_soft, statePop),
    ._stateSave             = BR_CMETHOD_REF(br_renderer_soft, stateSave),
    ._stateRestore          = BR_CMETHOD_REF(br_renderer_soft, stateRestore),
    ._stateMask             = BR_CMETHOD_REF(br_renderer_soft, stateMask),
    ._stateDefault          = BR_CMETHOD_REF(br_renderer_soft, stateDefault),
    ._boundsTest            = BR_CMETHOD_REF(br_renderer_soft, boundsTestF),
    ._coverageTest          = BR_CMETHOD_REF(br_renderer_soft, coverageTestF),
    ._viewDistance          = BR_CMETHOD_REF(br_renderer_soft, viewDistanceF),
    ._commandModeSet        = BR_CMETHOD_REF(br_renderer_soft, commandModeSet),
    ._commandModeQuery      = BR_CMETHOD_REF(br_renderer_soft, commandModeQuery),
    ._commandModeDefault    = BR_CMETHOD_REF(br_renderer_soft, commandModeDefault),
    ._commandModePush       = BR_CMETHOD_REF(br_renderer_soft, commandModePush),
    ._commandModePop        = BR_CMETHOD_REF(br_renderer_soft, commandModePop),
    ._flush                 = BR_CMETHOD_REF(br_renderer_soft, flush),
    ._synchronise           = BR_CMETHOD_REF(br_renderer_soft, synchronise),
    ._partQueryCapability   = BR_CMETHOD_REF(br_renderer_soft, partQueryCapability),
    ._stateQueryPerformance = BR_CMETHOD_REF(br_renderer_soft, stateQueryPerformance),
    ._frameBegin            = BR_CMETHOD_REF(br_renderer_soft, frameBegin),
    ._frameEnd              = BR_CMETHOD_REF(br_renderer_soft, frameEnd),
    ._focusLossBegin        = BR_CMETHOD_REF(br_renderer_soft, focusLossBegin),
    ._focusLossEnd          = BR_CMETHOD_REF(br_renderer_soft, focusLossEnd),
};
