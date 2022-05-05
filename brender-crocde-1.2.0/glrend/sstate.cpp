/*
* Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
*
* $Id: sstate.c 1.2 1997/08/11 14:26:07 jon Exp JON $
* $Locker: JON $
*
* Stored renderer state
*/
#include <stddef.h>
#include <string.h>

#include "drv_ip.hpp"
#include "sstate.hpp"

/*
* Geometry format info. template
*/
#define F(f)	offsetof(br_renderer_state_stored_gl, f)

static struct br_tv_template_entry templateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_O,		F(renderer),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_PARTS_U32,			F(state.valid),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
};
#undef F

static void BR_CMETHOD_DECL(br_renderer_state_stored_gl, free)(br_object *_self)
{
	br_renderer_state_stored_gl *self = reinterpret_cast<br_renderer_state_stored_gl*>(_self);

	ObjectContainerRemove(self->renderer, self);

	/*
	* Any associated primitive state will have been attached as a resource
	*/
	BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_renderer_state_stored_gl, identifier)(br_object *_self)
{
	return reinterpret_cast<br_renderer_state_stored_gl*>(_self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_renderer_state_stored_gl, type)(br_object *self)
{
	return BRT_RENDERER_STATE_STORED;
}

static br_boolean BR_CMETHOD_DECL(br_renderer_state_stored_gl, isType)(br_object *self, br_token t)
{
	return (t == BRT_RENDERER_STATE_STORED) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_renderer_state_stored_gl, device)(br_object *_self)
{
	br_renderer_state_stored_gl *self = reinterpret_cast<br_renderer_state_stored_gl*>(_self);
	return reinterpret_cast<br_device*>(self->renderer->device);
}

static br_size_t BR_CMETHOD_DECL(br_renderer_state_stored_gl, space)(br_object *self)
{
	return sizeof(br_renderer_state_stored_gl);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_renderer_state_stored_gl, templateQuery)(br_object *_self)
{
	br_renderer_state_stored_gl *self = reinterpret_cast<br_renderer_state_stored_gl*>(_self);

	if(self->device->templates.rendererStateStored == nullptr)
		self->device->templates.rendererStateStored = BrTVTemplateAllocate(self->device, templateEntries, BR_ASIZE(templateEntries));

	return self->renderer->device->templates.rendererStateStored;
}

/*
* Default dispatch table for renderer type (defined at and of file)
*/
static const struct br_renderer_state_stored_dispatch rendererStateStoredDispatch = {
	.__reserved0	= nullptr,
	.__reserved1	= nullptr,
	.__reserved2	= nullptr,
	.__reserved3	= nullptr,
	._free			= BR_CMETHOD(br_renderer_state_stored_gl,	free),
	._identifier	= BR_CMETHOD(br_renderer_state_stored_gl,	identifier),
	._type			= BR_CMETHOD(br_renderer_state_stored_gl,	type),
	._isType		= BR_CMETHOD(br_renderer_state_stored_gl,	isType),
	._device		= BR_CMETHOD(br_renderer_state_stored_gl,	device),
	._space			= BR_CMETHOD(br_renderer_state_stored_gl,	space),

	._templateQuery	= BR_CMETHOD(br_renderer_state_stored_gl,	templateQuery),
	._query			= BR_CMETHOD(br_object,						query),
	._queryBuffer	= BR_CMETHOD(br_object,						queryBuffer),
	._queryMany		= BR_CMETHOD(br_object,						queryMany),
	._queryManySize	= BR_CMETHOD(br_object,						queryManySize),
	._queryAll		= BR_CMETHOD(br_object,						queryAll),
	._queryAllSize	= BR_CMETHOD(br_object,						queryAllSize),
};

br_renderer_state_stored_gl *RendererStateStoredGLAllocate(br_renderer_gl *renderer, HGLSTATE_STACK base_state, br_uint_32 m, br_token_value *tv)
{
	br_renderer_state_stored_gl *self = BrResAllocate<br_renderer_state_stored_gl>(renderer, BR_MEMORY_OBJECT);
	self->dispatch = &rendererStateStoredDispatch;
	self->identifier = nullptr;
	self->device = renderer->device;
	self->renderer = renderer;

	//GLSTATEI_Copy(&self->state, &renderer->state.default, m);
	GLSTATE_Copy(&self->state, base_state, m);
	ObjectContainerAddFront(renderer, self);
	return self;
}
