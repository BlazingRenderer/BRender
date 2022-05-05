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

static struct br_tv_template_entry rendererStateStoredTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	0,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_O,		0,	F(renderer),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_PARTS_U32,			0,	F(state.valid),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
};
#undef F

static void br_renderer_state_stored_gl_free(br_object *_self)
{
	br_renderer_state_stored_gl *self = reinterpret_cast<br_renderer_state_stored_gl*>(_self);

	ObjectContainerRemove(self->renderer, reinterpret_cast<br_object*>(self));

	/*
	* Any associated primitive state will have been attached as a resource
	*/
	BrResFreeNoCallback(self);
}

static const char *br_renderer_state_stored_gl_identifier(br_object *_self)
{
	br_renderer_state_stored_gl *self = reinterpret_cast<br_renderer_state_stored_gl*>(_self);

	return self->identifier;
}

static br_token br_renderer_state_stored_gl_type(br_object *self)
{
	return BRT_RENDERER_STATE_STORED;
}

static br_boolean br_renderer_state_stored_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_RENDERER_STATE_STORED) || (t == BRT_OBJECT);
}

static br_device *br_renderer_state_stored_gl_device(br_object *_self)
{
	br_renderer_state_stored_gl *self = reinterpret_cast<br_renderer_state_stored_gl*>(_self);
	return reinterpret_cast<br_device*>(self->renderer->device);
}

static br_int_32 br_renderer_state_stored_gl_space(br_object *self)
{
	return sizeof(br_renderer_state_stored_gl);
}

static struct br_tv_template *br_renderer_state_stored_gl_templateQuery(br_object *_self)
{
	return reinterpret_cast<br_renderer_state_stored_gl*>(_self)->renderer->device->templates.rendererStateStoredTemplate;
}

/*
* Default dispatch table for renderer type (defined at and of file)
*/
static const struct br_renderer_state_stored_dispatch rendererStateStoredDispatch = {
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	br_renderer_state_stored_gl_free,
	br_renderer_state_stored_gl_identifier,
	br_renderer_state_stored_gl_type,
	br_renderer_state_stored_gl_isType,
	br_renderer_state_stored_gl_device,
	br_renderer_state_stored_gl_space,

	br_renderer_state_stored_gl_templateQuery,
	BR_CMETHOD(br_object,							query),
	BR_CMETHOD(br_object,							queryBuffer),
	BR_CMETHOD(br_object,							queryMany),
	BR_CMETHOD(br_object,							queryManySize),
	BR_CMETHOD(br_object,							queryAll),
	BR_CMETHOD(br_object,							queryAllSize),
};

br_renderer_state_stored_gl *RendererStateStoredGLAllocate(br_renderer_gl *renderer, HGLSTATE_STACK base_state, br_uint_32 m, br_token_value *tv)
{
	br_renderer_state_stored_gl *self = (br_renderer_state_stored_gl*)BrResAllocate(renderer, sizeof(br_renderer_state_stored_gl), BR_MEMORY_OBJECT);

	if(renderer->device->templates.rendererStateStoredTemplate == nullptr)
		renderer->device->templates.rendererStateStoredTemplate = BrTVTemplateAllocate(renderer->device, (br_tv_template_entry *)rendererStateStoredTemplateEntries, BR_ASIZE(rendererStateStoredTemplateEntries));

	self->dispatch = (br_renderer_state_stored_dispatch*)&rendererStateStoredDispatch;
	self->identifier = nullptr;
	self->renderer = renderer;

	//GLSTATEI_Copy(&self->state, &renderer->state.default, m);
	GLSTATE_Copy(&self->state, base_state, m);
	ObjectContainerAddFront(renderer, (br_object *)self);
	return self;
}
