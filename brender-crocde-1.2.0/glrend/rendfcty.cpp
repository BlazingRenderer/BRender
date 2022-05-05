#include <log.h>
#include "drv_ip.hpp"
#include "brassert.h"

/*
* Renderer Type info. template
*/
#define F(f) offsetof(struct br_renderer_facility_gl, f)
static struct br_tv_template_entry templateEntries[] = {
	{BRT_IDENTIFIER_CSTR,            F(identifier),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT_RENDERER_MAX_I32,           0,					BRTV_QUERY | BRTV_ALL,	BRTV_CONV_DIRECT, 1},
	//{BRT(PARTS_TL),                  (br_int_32)&D3DRendererPartsTokens,BRTV_QUERY | BRTV_ALL | BRTV_ABS,	BRTV_CONV_LIST,},
};
#undef F

/* Structure used to unpack the rendererNew arguments */
struct newRendererTokens
{
	br_object *dest;
};

#define F(f) offsetof(struct newRendererTokens, f)
static struct br_tv_template_entry rendererNewTemplateEntries[] = {
	{BRT_DESTINATION_O,			F(dest),		BRTV_SET,	BRTV_CONV_COPY,},
	{BRT_OUTPUT_FACILITY_O,		F(dest),		BRTV_SET,	BRTV_CONV_COPY,},
};
#undef F

static void BR_CMETHOD_DECL(br_renderer_facility_gl, free)(br_object *self)
{
	br_renderer_facility_gl *rendfcty = reinterpret_cast<br_renderer_facility_gl*>(self);
	/* Detach rendererer from device */
	ObjectContainerRemove(rendfcty->output_facility, self);

	/* Remove attached objects */
	BrObjectContainerFree((br_object_container *)self, (br_token)BR_NULL_TOKEN, nullptr, nullptr);

	BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_renderer_facility_gl, identifier)(br_object *self)
{
	return reinterpret_cast<br_renderer_facility_gl*>(self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_renderer_facility_gl, type)(br_object *self)
{
	return BRT_RENDERER_FACILITY;
}

static br_boolean BR_CMETHOD_DECL(br_renderer_facility_gl, isType)(br_object *self, br_token t)
{
	return (t == BRT_RENDERER_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_renderer_facility_gl, device)(br_object *self)
{
	br_renderer_facility_gl *rendfcty = reinterpret_cast<br_renderer_facility_gl*>(self);
	return reinterpret_cast<br_device*>(rendfcty->output_facility->device);
}

static br_size_t BR_CMETHOD_DECL(br_renderer_facility_gl, space)(br_object *self)
{
	return sizeof(br_renderer_facility_gl);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_renderer_facility_gl, templateQuery)(br_object *_self)
{
	br_renderer_facility_gl *self = reinterpret_cast<br_renderer_facility_gl*>(_self);

	if(self->device->templates.rendererFacility == nullptr)
		self->device->templates.rendererFacility = BrTVTemplateAllocate(self, templateEntries, BR_ASIZE(templateEntries));

	return self->device->templates.rendererFacility;
}


static void *BR_CMETHOD_DECL(br_renderer_facility_gl, listQuery)(br_object_container *self)
{
	return reinterpret_cast<br_renderer_facility_gl*>(self)->object_list;
}

static br_error BR_CMETHOD_DECL(br_renderer_facility_gl, validDestination)(br_renderer_facility *self, br_boolean *bp, br_object *h)
{
	if(h != nullptr && ObjectType(h) == BRT_DEVICE_PIXELMAP && ObjectDevice(self) == ObjectDevice(h))
		return BRE_OK;

	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_facility_gl, rendererNew)(struct br_renderer_facility *_self, struct br_renderer **prenderer, br_token_value *tv)
{
	br_renderer_facility_gl *self = reinterpret_cast<br_renderer_facility_gl*>(_self);

	struct newRendererTokens rt = { .dest = nullptr };
	br_int_32 count;

	/* Process any options */
	if(self->device->templates.rendererNew == nullptr)
		self->device->templates.rendererNew = BrTVTemplateAllocate(self->device, rendererNewTemplateEntries, BR_ASIZE(rendererNewTemplateEntries));

	BrTokenValueSetMany(&rt, &count, nullptr, tv, self->device->templates.rendererNew);

	/* Check that the destination is valid */
	br_object_gl *glo = reinterpret_cast<br_object_gl*>(rt.dest);
	if(glo == nullptr || ObjectDevice(rt.dest) != (br_device*)self->device || !ObjectIsType(rt.dest, BRT_DEVICE_PIXELMAP))
		return BRE_UNSUPPORTED;

	br_renderer_gl *renderer = RendererGLAllocate(
		self->device,
		self,
		reinterpret_cast<br_device_pixelmap_gl*>(rt.dest)
	);

	if(renderer == nullptr)
		return BRE_FAIL;

	*prenderer = reinterpret_cast<br_renderer*>(renderer);
	return BRE_OK;
}

static const struct br_renderer_facility_dispatch rendererFacilityDispatch = {
	.__reserved0		= nullptr,
	.__reserved1		= nullptr,
	.__reserved2		= nullptr,
	.__reserved3		= nullptr,
	._free				= BR_CMETHOD(br_renderer_facility_gl,	free),
	._identifier		= BR_CMETHOD(br_renderer_facility_gl,	identifier),
	._type				= BR_CMETHOD(br_renderer_facility_gl,	type),
	._isType			= BR_CMETHOD(br_renderer_facility_gl,	isType),
	._device			= BR_CMETHOD(br_renderer_facility_gl,	device),
	._space				= BR_CMETHOD(br_renderer_facility_gl,	space),

	._templateQuery		= BR_CMETHOD(br_renderer_facility_gl,	templateQuery),
	._query				= BR_CMETHOD(br_object,					query),
	._queryBuffer		= BR_CMETHOD(br_object,					queryBuffer),
	._queryMany			= BR_CMETHOD(br_object,					queryMany),
	._queryManySize		= BR_CMETHOD(br_object,					queryManySize),
	._queryAll			= BR_CMETHOD(br_object,					queryAll),
	._queryAllSize		= BR_CMETHOD(br_object,					queryAllSize),

	._listQuery			= BR_CMETHOD(br_renderer_facility_gl,	listQuery),
	._tokensMatchBegin	= BR_CMETHOD(br_object_container,		tokensMatchBegin),
	._tokensMatch		= BR_CMETHOD(br_object_container,		tokensMatch),
	._tokensMatchEnd	= BR_CMETHOD(br_object_container,		tokensMatchEnd),
	._addFront			= BR_CMETHOD(br_object_container,		addFront),
	._removeFront		= BR_CMETHOD(br_object_container,		removeFront),
	._remove			= BR_CMETHOD(br_object_container,		remove),
	._find				= BR_CMETHOD(br_object_container,		find),
	._findMany			= BR_CMETHOD(br_object_container,		findMany),
	._count				= BR_CMETHOD(br_object_container,		count),

	._validDestination	= BR_CMETHOD(br_renderer_facility_gl,	validDestination),

	._rendererNew		= BR_CMETHOD(br_renderer_facility_gl,	rendererNew),
};

br_renderer_facility_gl *RendererFacilityGLInit(br_output_facility_gl *outfcty)
{
	ASSERT(outfcty->renderer_facility == nullptr);

	br_renderer_facility_gl *self = BrResAllocate<br_renderer_facility_gl>(outfcty, BR_MEMORY_OBJECT);
	self->dispatch = &rendererFacilityDispatch;
	self->identifier = "OpenGL Renderer facility";
	self->device = outfcty->device;
	self->output_facility = outfcty;
	self->object_list = BrObjectListAllocate(self);
	outfcty->renderer_facility = self;

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

	return self;
}
