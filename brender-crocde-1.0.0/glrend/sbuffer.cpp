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

static struct br_tv_template_entry bufferStoredTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,	0,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
};
#undef F


static br_error br_buffer_stored_gl_update(br_buffer_stored *_self, br_device_pixelmap *_pm, br_token_value *tv)
{
	br_buffer_stored_gl *self = reinterpret_cast<br_buffer_stored_gl*>(_self);
	br_pixelmap *pm = reinterpret_cast<br_pixelmap*>(_pm);

	if(self->glTex == 0)
		glGenTextures(1, &self->glTex);

	self->source = pm;

	/* Find out where the pixelmap comes from */
	br_device *pm_device = _pm->dispatch->_device((br_object*)pm);

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
		__debugbreak();
	}
	else
		return BRE_FAIL;

	return BRE_OK;
}

static void br_buffer_stored_gl_free(br_object *_self)
{
	br_buffer_stored_gl *self = reinterpret_cast<br_buffer_stored_gl*>(_self);
	glDeleteTextures(1, &self->glTex);
	ObjectContainerRemove(self->renderer, (br_object *)self);
}

static const char *br_buffer_stored_gl_identifier(br_object *_self)
{
	br_buffer_stored_gl *self = reinterpret_cast<br_buffer_stored_gl*>(_self);
	return self->identifier;
}

static br_token br_buffer_stored_gl_type(br_object *self)
{
	return BRT_BUFFER_STORED;
}

static br_boolean br_buffer_stored_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_BUFFER_STORED) || (t == BRT_OBJECT);
}

static br_device *br_buffer_stored_gl_device(br_object *_self)
{
	br_buffer_stored_gl *self = reinterpret_cast<br_buffer_stored_gl*>(_self);
	return (br_device*)self->renderer->device;
}

static br_int_32 br_buffer_stored_gl_space(br_object *self)
{
	return sizeof(br_buffer_stored_gl);
}

static struct br_tv_template * br_buffer_stored_gl_templateQuery(br_object *_self)
{
	br_buffer_stored_gl *self = reinterpret_cast<br_buffer_stored_gl*>(_self);
	return self->renderer->device->templates.bufferStoredTemplate;
}

/*
* Default dispatch table for device
*/
static struct br_buffer_stored_dispatch bufferStoredDispatch = {
	NULL,
	NULL,
	NULL,
	NULL,
	br_buffer_stored_gl_free,
	br_buffer_stored_gl_identifier,
	br_buffer_stored_gl_type,
	br_buffer_stored_gl_isType,
	br_buffer_stored_gl_device,
	br_buffer_stored_gl_space,

	br_buffer_stored_gl_templateQuery,
	BR_CMETHOD(br_object,				query),
	BR_CMETHOD(br_object, 				queryBuffer),
	BR_CMETHOD(br_object, 				queryMany),
	BR_CMETHOD(br_object, 				queryManySize),
	BR_CMETHOD(br_object, 				queryAll),
	BR_CMETHOD(br_object,	 			queryAllSize),

	br_buffer_stored_gl_update,
};


/*
* Set up a static device object
*/
br_buffer_stored_gl *BufferStoredGLAllocate(br_renderer_gl *renderer, br_token use, br_device_pixelmap *pm, br_token_value *tv)
{
	br_buffer_stored_gl *self;
	char *ident;

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
			return NULL;
	}

	self = (br_buffer_stored_gl*)BrResAllocate(renderer, sizeof(*self), BR_MEMORY_OBJECT);

	if(self == nullptr)
		return nullptr;

	self->dispatch = &bufferStoredDispatch;
	self->identifier = ident;
	self->renderer = renderer;

	/*
	* Should get this flag from the token-value arguments
	*/
	self->flags |= SBUFF_SHARED;
	self->glTex = 0;

	BufferStoredUpdate(self, pm, tv);

	ObjectContainerAddFront(renderer, (br_object *)self);

	if(renderer->device->templates.bufferStoredTemplate == nullptr)
		renderer->device->templates.bufferStoredTemplate = BrTVTemplateAllocate(renderer->device, bufferStoredTemplateEntries, BR_ASIZE(bufferStoredTemplateEntries));


	return self;
}
