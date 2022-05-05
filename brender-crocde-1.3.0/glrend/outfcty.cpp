#include <cstddef>
#include <log.h>
#include "brlists.h"
#include "drv_ip.hpp"

/*
* Output Type info. template
*/
#define F(f)	offsetof(br_output_facility_gl, f)

static struct br_tv_template_entry templateEntries[] = {

	{BRT(WIDTH_I32),				F(width),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(WIDTH_MIN_I32),			F(width),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(WIDTH_MAX_I32),			F(width),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(HEIGHT_I32),				F(height),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(HEIGHT_MIN_I32),			F(height),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(HEIGHT_MAX_I32),			F(height),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(PIXEL_BITS_I32),			F(colour_bits),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(PIXEL_TYPE_U8),			F(colour_type),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(INDEXED_B),  				F(indexed),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(WINDOW_MONITOR_I32),		F(monitor),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(IDENTIFIER_CSTR),			F(identifier),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(RENDERER_FACILITY_O),		F(renderer_facility),	BRTV_QUERY | BRTV_ALL,  BRTV_CONV_COPY,},
	{BRT(TEMPORARY_B),  			F(temporary),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},

	/*
	 * We don't use these, but we need BrDevBeginVar to accept them.
	 * These are passed to pixelmapNew, which will handle them.
	 */
	{BRT(WINDOW_HANDLE_H),			0,						BRTV_QUERY | BRTV_ALL,	BRTV_CONV_DIRECT,},
	{BRT(OPENGL_MSAA_SAMPLES_I32),	0,						BRTV_QUERY | BRTV_ALL,	BRTV_CONV_DIRECT,},
};

#undef F

static void BR_CMETHOD_DECL(br_output_facility_gl, free)(br_object *self)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);

	--outfcty->device->output_facility_count;
	ObjectContainerRemove(outfcty->device, self);

	BrObjectContainerFree((br_object_container *)self, (br_token)BR_NULL_TOKEN, nullptr, nullptr);

	BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_output_facility_gl, identifier)(br_object *self)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);
	return outfcty->identifier;
}

static br_token BR_CMETHOD_DECL(br_output_facility_gl, type)(br_object *self)
{
	return BRT_OUTPUT_FACILITY;
}

static br_boolean BR_CMETHOD_DECL(br_output_facility_gl, isType)(br_object *self, br_token t)
{
	return (t == BRT_OUTPUT_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_output_facility_gl, device)(br_object *self)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);
	return reinterpret_cast<br_device*>(outfcty->device);
}

static br_size_t BR_CMETHOD_DECL(br_output_facility_gl, space)(br_object *self)
{
	return sizeof(br_output_facility_gl);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_output_facility_gl, queryTemplate)(br_object *_self)
{
	br_output_facility_gl *self = reinterpret_cast<br_output_facility_gl*>(_self);

    if(self->device->templates.outputFacility == nullptr)
        self->device->templates.outputFacility = BrTVTemplateAllocate(self->device, templateEntries, BR_ASIZE(templateEntries));

	return self->device->templates.outputFacility;
}

static void *BR_CMETHOD_DECL(br_output_facility_gl, listQuery)(br_object_container *self)
{
	return reinterpret_cast<br_output_facility_gl*>(self)->object_list;
}

static br_error BR_CMETHOD_DECL(br_output_facility_gl, validSource)(br_output_facility *self, br_boolean *bp, br_object *h)
{
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_output_facility_gl, pixelmapNew)(br_output_facility *self, br_device_pixelmap **ppmap, br_token_value *tv)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);
	/*
	* Create a device pixelmap structure representing display memory
	*/
	br_device_pixelmap_gl *pm = DevicePixelmapGLAllocate(outfcty->device, outfcty, tv);

	if(pm == nullptr)
		return BRE_FAIL;

	*ppmap = reinterpret_cast<br_device_pixelmap*>(pm);

	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_output_facility_gl, clutNew)(br_output_facility *self, br_device_clut **pclut, br_token_value *tv)
{
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_output_facility_gl, queryCapability)(br_output_facility *self, br_token_value *buffer_in, br_token_value *buffer_out, br_size_t size_buffer_out)
{
	return BRE_FAIL;
}

static const struct br_output_facility_dispatch outputFacilityDispatch = {
	.__reserved0		= nullptr,
	.__reserved1		= nullptr,
	.__reserved2		= nullptr,
	.__reserved3		= nullptr,
	._free				= BR_CMETHOD(br_output_facility_gl,	free),
	._identifier		= BR_CMETHOD(br_output_facility_gl,	identifier),
	._type				= BR_CMETHOD(br_output_facility_gl,	type),
	._isType			= BR_CMETHOD(br_output_facility_gl,	isType),
	._device			= BR_CMETHOD(br_output_facility_gl,	device),
	._space				= BR_CMETHOD(br_output_facility_gl,	space),

	._templateQuery		= BR_CMETHOD(br_output_facility_gl,	queryTemplate),
	._query				= BR_CMETHOD(br_object,				query),
	._queryBuffer		= BR_CMETHOD(br_object,				queryBuffer),
	._queryMany			= BR_CMETHOD(br_object,				queryMany),
	._queryManySize		= BR_CMETHOD(br_object,				queryManySize),
	._queryAll			= BR_CMETHOD(br_object,				queryAll),
	._queryAllSize		= BR_CMETHOD(br_object,				queryAllSize),

	._listQuery			= BR_CMETHOD(br_output_facility_gl,	listQuery),
	._tokensMatchBegin	= BR_CMETHOD(br_object_container,	tokensMatchBegin),
	._tokensMatch		= BR_CMETHOD(br_object_container,	tokensMatch),
	._tokensMatchEnd	= BR_CMETHOD(br_object_container,	tokensMatchEnd),
	._addFront			= BR_CMETHOD(br_object_container,	addFront),
	._removeFront		= BR_CMETHOD(br_object_container,	removeFront),
	._remove			= BR_CMETHOD(br_object_container,	remove),
	._find				= BR_CMETHOD(br_object_container,	find),
	._findMany			= BR_CMETHOD(br_object_container,	findMany),
	._count				= BR_CMETHOD(br_object_container,	count),

	._validSource		= BR_CMETHOD(br_output_facility_gl,	validSource),
	._pixelmapNew		= BR_CMETHOD(br_output_facility_gl,	pixelmapNew),
	._clutNew			= BR_CMETHOD(br_output_facility_gl,	clutNew),
	._queryCapability	= BR_CMETHOD(br_output_facility_gl,	queryCapability)
};

static br_error glfwToBRrenderSurfaceType(const GLFWvidmode *mode, br_uint_16 *pType)
{
	/* Only support RGB555, RGB555, and RGB888 */
	if(mode->redBits == 5 && mode->greenBits == 5 && mode->blueBits == 5)
		*pType = BR_PMT_RGB_555;
	else if(mode->redBits == 5 && mode->greenBits == 6 && mode->blueBits == 5)
		*pType = BR_PMT_RGB_565;
	else if(mode->redBits == 8 && mode->greenBits == 8 && mode->blueBits == 8)
		*pType = BR_PMT_RGB_888;
	else
		return BRE_FAIL;

	return BRE_OK;
}

static int BrType_BPP(int type)
{
	switch(type)
	{
		case BR_PMT_INDEX_8: return 8;
		case BR_PMT_RGB_555: return 15;
		case BR_PMT_RGB_565: return 16;
		case BR_PMT_RGB_888: return 24;
		case BR_PMT_RGBX_888: return 32;
		case BR_PMT_RGBA_8888: return 32;
	}

	return 0;
}


br_output_facility_gl *OutputFacilityCreateMode(br_device_gl *dev, br_uint_16 type, br_int_32 width, br_int_32 height, br_int_32 monitorIndex, br_boolean temporary)
{
	br_output_facility_gl *self = BrResAllocate<br_output_facility_gl>(dev, BR_MEMORY_OBJECT);
	self->dispatch = &outputFacilityDispatch;
	self->device = dev;
	self->object_list = BrObjectListAllocate(self);

	/* Fill in display format */
	self->colour_type = type;
	self->colour_bits = BrType_BPP(type);
	self->indexed = (type <= BR_PMT_INDEX_8);
	self->monitor = monitorIndex;

	self->width  = width;
	self->height = height;
	self->temporary = temporary;

	/* Attach a descriptive identifier */
	char tmp[64];
	BrSprintfN(tmp, sizeof(tmp) - 1, "%dx%dx%d", self->width, self->height, self->colour_bits);
	log_trace("GLFW", "Registering %sscreen mode %s.", temporary ? "temporary " : "", tmp);
	self->identifier = BrResStrDup(self, tmp);

	self->renderer_facility = nullptr;
	return self;
}

static br_output_facility_gl *OutputFacilityCreateModeFromGLFW(br_device_gl *dev, br_int_32 monitorIndex, const GLFWvidmode *mode)
{
	br_uint_16 type;
	if(glfwToBRrenderSurfaceType(mode, &type) != BRE_OK)
		return nullptr;

	return OutputFacilityCreateMode(dev, type, mode->width, mode->height, monitorIndex, 0);
}

br_error OutputFacilityOnPixelmapFree(br_output_facility_gl *outfcty, br_device_pixelmap_gl *pixmp)
{
    br_device_pixelmap_gl *pm = nullptr;

    if(!outfcty->temporary)
        return BRE_OK;

    /* Find a device pixelmap. If this fails, we don't care. */
    (void)ObjectContainerFind(outfcty, (struct br_object**)&pm, BRT_DEVICE_PIXELMAP, nullptr, nullptr);

    if(pm != nullptr)
        return BRE_OK;

    /* No pixelmaps left, destroy */
    ObjectFree(outfcty);
    return BRE_OK;
}

br_error OutputFacilityGLEnumerate(br_device_gl *device)
{
	int numMonitors;
	GLFWmonitor **monitors = glfwGetMonitors(&numMonitors);
	if(!monitors)
		return BRE_FAIL;

	for(int i = 0; i < numMonitors; ++i)
	{
		int numModes;
		const GLFWvidmode *modes = glfwGetVideoModes(monitors[i], &numModes);
		if(!modes)
			continue;

		/* Add all the fullscreen modes. */
		for(int j = 0; j < numModes; ++j)
		{
			/* Create the mode. */
			br_output_facility_gl *mode = OutputFacilityCreateModeFromGLFW(device, i, &modes[j]);
			if(!mode)
				continue;

			/* If it succeeded, add it to the main list. */
			ObjectContainerAddFront(device, mode);
			++device->output_facility_count;

			RendererFacilityGLInit(mode);
		}
	}

	return BRE_OK;
}
