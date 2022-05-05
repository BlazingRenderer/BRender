/*
* Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
*
* $Id: Sbuffer.c 1.3 1997/08/12 17:27:48 jon Exp JON $
* $Locker: JON $
*
* Stored buffer methods
*/
#include <stddef.h>
#include <string.h>

#include "drv_ip.hpp"

/*
* Primitive state info. template
*/
#define F(f) offsetof(br_buffer_stored_gl, f)

static struct br_tv_template_entry templateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
};
#undef F


static br_error BR_CMETHOD_DECL(br_buffer_stored_gl, update)(br_buffer_stored *_self, br_device_pixelmap *_pm, br_token_value *tv)
{
	br_buffer_stored_gl *self = reinterpret_cast<br_buffer_stored_gl*>(_self);
	br_pixelmap *pm = reinterpret_cast<br_pixelmap*>(_pm);

	if(self->glTex == 0)
		glGenTextures(1, &self->glTex);

	self->source_flags = pm->flags;
	self->source = pm;

	/* Find out where the pixelmap comes from */
	br_device *pm_device = ObjectDevice(pm);
	if(pm_device == NULL)
	{
		/* The pixelmap is a plain BRender memory pixelmap.  Make sure that the pixels can be accessed */
		if(pm->flags & BR_PMF_NO_ACCESS || pm->pixels == NULL)
			return BRE_FAIL;

		VIDEOI_BrPixelmapToExistingTexture(self->glTex, pm);
	}
	else if(pm_device != (br_device*)self->renderer->device)
	{
		// pixelmap is this device, copy it
		BrDebugBreak();
	}
	else
		return BRE_FAIL;

	return BRE_OK;
}

static void BR_CMETHOD_DECL(br_buffer_stored_gl, free)(br_object *_self)
{
	br_buffer_stored_gl *self = reinterpret_cast<br_buffer_stored_gl*>(_self);
	glDeleteTextures(1, &self->glTex);
	ObjectContainerRemove(self->renderer, (br_object *)self);
}

static const char *BR_CMETHOD_DECL(br_buffer_stored_gl, identifier)(br_object *_self)
{
	return reinterpret_cast<br_buffer_stored_gl*>(_self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_buffer_stored_gl, type)(br_object *self)
{
	return BRT_BUFFER_STORED;
}

static br_boolean BR_CMETHOD_DECL(br_buffer_stored_gl, isType)(br_object *self, br_token t)
{
	return (t == BRT_BUFFER_STORED) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_buffer_stored_gl, device)(br_object *_self)
{
	br_buffer_stored_gl *self = reinterpret_cast<br_buffer_stored_gl*>(_self);
	return (br_device*)self->renderer->device;
}

static br_size_t BR_CMETHOD_DECL(br_buffer_stored_gl, space)(br_object *self)
{
	return sizeof(br_buffer_stored_gl);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_buffer_stored_gl, templateQuery)(br_object *_self)
{
	br_buffer_stored_gl *self = reinterpret_cast<br_buffer_stored_gl*>(_self);

	if(self->device->templates.bufferStored == nullptr)
		self->device->templates.bufferStored = BrTVTemplateAllocate(self->device, templateEntries, BR_ASIZE(templateEntries));

	return self->renderer->device->templates.bufferStored;
}

/*
* Default dispatch table for device
*/
static struct br_buffer_stored_dispatch bufferStoredDispatch = {
	.__reserved0	= nullptr,
	.__reserved1	= nullptr,
	.__reserved2	= nullptr,
	.__reserved3	= nullptr,
	._free			= BR_CMETHOD(br_buffer_stored_gl,	free),
	._identifier	= BR_CMETHOD(br_buffer_stored_gl,	identifier),
	._type			= BR_CMETHOD(br_buffer_stored_gl,	type),
	._isType		= BR_CMETHOD(br_buffer_stored_gl,	isType),
	._device		= BR_CMETHOD(br_buffer_stored_gl,	 device),
	._space			= BR_CMETHOD(br_buffer_stored_gl,	space),

	._templateQuery	= BR_CMETHOD(br_buffer_stored_gl,	templateQuery),
	._query			= BR_CMETHOD(br_object,				query),
	._queryBuffer	= BR_CMETHOD(br_object,				queryBuffer),
	._queryMany		= BR_CMETHOD(br_object,				queryMany),
	._queryManySize	= BR_CMETHOD(br_object,				queryManySize),
	._queryAll		= BR_CMETHOD(br_object,				queryAll),
	._queryAllSize	= BR_CMETHOD(br_object,				queryAllSize),

	._update		= BR_CMETHOD(br_buffer_stored_gl,	update),
};


/*
* Set up a static device object
*/
br_buffer_stored_gl *BufferStoredGLAllocate(br_renderer_gl *renderer, br_token use, br_device_pixelmap *pm, br_token_value *tv)
{
	const char *ident;

	switch(use)
	{

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
			return nullptr;
	}

	br_buffer_stored_gl *self = BrResAllocate<br_buffer_stored_gl>(renderer, BR_MEMORY_OBJECT);
	self->dispatch = &bufferStoredDispatch;
	self->identifier = ident;
	self->renderer = renderer;

	/*
	* Should get this flag from the token-value arguments
	*/
	self->flags |= SBUFF_SHARED;
	self->glTex = 0;

	BufferStoredUpdate(self, pm, tv);

	ObjectContainerAddFront(renderer, self);

	return self;
}
