#include <log.h>
#include "drv_ip.hpp"
#include "brassert.h"

/*
* Renderer Type info. template
*/
#define F(f)	offsetof(struct br_renderer_facility_gl, f)

static struct br_tv_template_entry rendererFacilityTemplateEntries[] = {
	{BRT_IDENTIFIER_CSTR,            0,	F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_MAX_I32,           0,	0,					BRTV_QUERY | BRTV_ALL,	BRTV_CONV_DIRECT, 1},
	//{BRT(PARTS_TL),                  (br_int_32)&D3DRendererPartsTokens,BRTV_QUERY | BRTV_ALL | BRTV_ABS,	BRTV_CONV_LIST,},
};
#undef F

/*
* Structure used to unpack the rendererNew arguments
*/
struct newRendererTokens
{
	br_object	*dest;
};

#define F(f)	offsetof(struct newRendererTokens, f)

static struct br_tv_template_entry rendererNewTemplateEntries[] = {
	{BRT_DESTINATION_O,			0,	F(dest),		BRTV_SET,	BRTV_CONV_COPY,},
	{BRT_OUTPUT_FACILITY_O,		0,	F(dest),		BRTV_SET,	BRTV_CONV_COPY,},
};
#undef F

static void br_renderer_facility_gl_free(br_object *self)
{
	br_renderer_facility_gl *rendfcty = reinterpret_cast<br_renderer_facility_gl*>(self);
	/* Detach rendererer from device */
	ObjectContainerRemove(rendfcty->output_facility, (br_object *)self);

	/* Remove attached objects */
	BrObjectContainerFree((br_object_container *)self, (br_token)BR_NULL_TOKEN, NULL, NULL);

	BrResFreeNoCallback(self);
}

static const char *br_renderer_facility_gl_identifier(br_object *self)
{
	br_renderer_facility_gl *rendfcty = reinterpret_cast<br_renderer_facility_gl*>(self);
	return rendfcty->identifier;
}

static br_token br_renderer_facility_gl_type(br_object *self)
{
	return BRT_RENDERER_FACILITY;
}

static br_boolean br_renderer_facility_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_RENDERER_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *br_renderer_facility_gl_device(br_object *self)
{
	br_renderer_facility_gl *rendfcty = reinterpret_cast<br_renderer_facility_gl*>(self);
	return reinterpret_cast<br_device*>(rendfcty->output_facility->device);
}

static br_int_32 br_renderer_facility_gl_space(br_object *self)
{
	return sizeof(br_renderer_facility_gl);
}

static struct br_tv_template * br_renderer_facility_gl_templateQuery(br_object *self)
{
	br_renderer_facility_gl *rendfcty = reinterpret_cast<br_renderer_facility_gl*>(self);
	if(rendfcty->output_facility->device->templates.rendererFacilityTemplate == NULL)
		rendfcty->output_facility->device->templates.rendererFacilityTemplate = BrTVTemplateAllocate(
			rendfcty->output_facility->device,
			(br_tv_template_entry *)rendererFacilityTemplateEntries,
			BR_ASIZE(rendererFacilityTemplateEntries)
		);

	return rendfcty->output_facility->device->templates.rendererFacilityTemplate;
}


static void *br_renderer_facility_gl_listQuery(br_object_container *self)
{
	br_renderer_facility_gl *rendfcty = reinterpret_cast<br_renderer_facility_gl*>(self);
	return rendfcty->object_list;
}

static br_error br_renderer_facility_gl_validDestination(br_renderer_facility *self, br_boolean *bp, br_object *h)
{
	return BRE_OK;
}

static br_error br_renderer_facility_gl_rendererNew(struct br_renderer_facility *self, struct br_renderer **prenderer, br_token_value *tv)
{
	br_renderer_facility_gl *rendfcty = reinterpret_cast<br_renderer_facility_gl*>(self);

	struct newRendererTokens rt = {NULL};
	br_int_32 count;

	/* Process any options */
	if(rendfcty->output_facility->device->templates.rendererNewTemplate == NULL)
		rendfcty->output_facility->device->templates.rendererNewTemplate = BrTVTemplateAllocate(rendfcty->output_facility->device,
																								rendererNewTemplateEntries, BR_ASIZE(rendererNewTemplateEntries));

	BrTokenValueSetMany(&rt, &count, NULL, tv, rendfcty->output_facility->device->templates.rendererNewTemplate);

	/* Check that the destination is valid */
	br_object_gl *glo = reinterpret_cast<br_object_gl*>(rt.dest);
	if(glo == NULL || glo->dispatch->_device(rt.dest) != (br_device*)rendfcty->output_facility->device || !glo->dispatch->_isType(rt.dest, BRT_DEVICE_PIXELMAP))
		return BRE_UNSUPPORTED;

	br_renderer_gl *renderer = RendererGLAllocate(
		rendfcty->output_facility->device,
		rendfcty,
		reinterpret_cast<br_device_pixelmap_gl*>(rt.dest)
	);

	if(renderer == NULL)
		return BRE_FAIL;

	*prenderer = reinterpret_cast<br_renderer*>(renderer);
	return BRE_OK;
}

static const struct br_renderer_facility_dispatch rendererFacilityDispatch = {
	NULL,
	NULL,
	NULL,
	NULL,
	br_renderer_facility_gl_free,
	br_renderer_facility_gl_identifier,
	br_renderer_facility_gl_type,
	br_renderer_facility_gl_isType,
	br_renderer_facility_gl_device,
	br_renderer_facility_gl_space,

	br_renderer_facility_gl_templateQuery,
	BR_CMETHOD(br_object,					query),
	BR_CMETHOD(br_object,					queryBuffer),
	BR_CMETHOD(br_object,					queryMany),
	BR_CMETHOD(br_object,					queryManySize),
	BR_CMETHOD(br_object,					queryAll),
	BR_CMETHOD(br_object,					queryAllSize),

	br_renderer_facility_gl_listQuery,
	BR_CMETHOD(br_object_container,			tokensMatchBegin),
	BR_CMETHOD(br_object_container,			tokensMatch),
	BR_CMETHOD(br_object_container,			tokensMatchEnd),
	BR_CMETHOD(br_object_container,			addFront),
	BR_CMETHOD(br_object_container,			removeFront),
	BR_CMETHOD(br_object_container,			remove),
	BR_CMETHOD(br_object_container,			find),
	BR_CMETHOD(br_object_container,			findMany),
	BR_CMETHOD(br_object_container,			count),

	br_renderer_facility_gl_validDestination,

	br_renderer_facility_gl_rendererNew,
};

br_renderer_facility_gl *RendererFacilityGLInit(br_output_facility_gl *outfcty)
{
	ASSERT(outfcty->renderer_facility == nullptr);

	br_renderer_facility_gl *self = reinterpret_cast<br_renderer_facility_gl*>(BrResAllocate(outfcty, sizeof(br_renderer_facility_gl), BR_MEMORY_OBJECT));
	self->dispatch = (struct br_renderer_facility_dispatch *)&rendererFacilityDispatch;
	self->output_facility = outfcty;
	/* Can't use self here, not a dynamic object. */
	self->object_list = BrObjectListAllocate(outfcty);


	/*
	* Create default state
	*/
	//StateInitialise(&self->default_state);

	/* Create geometry objects */
	if(GeometryV1BucketsGLAllocate(self, "V1Buckets") == nullptr ||
	   GeometryPrimitivesGLAllocate(self, "Primitives") == nullptr ||
	   GeometryLightingGLAllocate(self, "Lighting") == nullptr ||
	   GeometryV1ModelGLAllocate(self, "V1Model") == nullptr)
	{
		log_error("GLREND", "Error creating geometry objects.");
		BrResFree(self);
		return nullptr;
	}

	self->identifier = "OpenGL Renderer facility";
	outfcty->renderer_facility = self;
	return self;
}
