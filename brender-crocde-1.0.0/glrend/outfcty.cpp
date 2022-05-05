#include <cstddef>
#include <log.h>
#include "brlists.h"
#include "drv_ip.hpp"

/*
* Output Type info. template
*/
#define F(f)	offsetof(br_output_facility_gl, f)

static struct br_tv_template_entry outputFacilityTemplateEntries[] = {

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
	{BRT(WINDOW_HANDLE_H),          0,						BRTV_QUERY | BRTV_ALL,	BRTV_CONV_DIRECT,},
};

#undef F

static void BR_METHOD br_output_facility_gl_free(br_object *self)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);
}

static const char * BR_METHOD br_output_facility_gl_identifier(br_object *self)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);
	return outfcty->identifier;
}

static br_token BR_METHOD br_output_facility_gl_type(br_object *self)
{
	return BRT_OUTPUT_FACILITY;
}

static br_boolean BR_METHOD br_output_facility_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_OUTPUT_FACILITY) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device * BR_METHOD br_output_facility_gl_device(br_object *self)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);
	return reinterpret_cast<br_device*>(outfcty->device);
}

static br_int_32 BR_METHOD br_output_facility_gl_space(br_object *self)
{
	return sizeof(br_output_facility_gl);
}

static struct br_tv_template * BR_METHOD br_output_facility_gl_queryTemplate(br_object *self)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);
	return outfcty->device->templates.outputFacilityTemplate;
}

static void * BR_METHOD br_output_facility_gl_listQuery(br_object_container *self)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);
	return outfcty->object_list;
}

static br_error BR_METHOD br_output_facility_gl_validSource(br_output_facility *self, br_boolean *bp, br_object *h)
{
	return BRE_OK;
}

static br_error BR_METHOD br_output_facility_gl_pixelmapNew(br_output_facility *self, br_device_pixelmap **ppmap, br_token_value *tv)
{
	br_output_facility_gl *outfcty = reinterpret_cast<br_output_facility_gl*>(self);
	/*
	* Create a device pixelmap structure representing display memory
	*/
	br_device_pixelmap_gl *pm = DevicePixelmapGLAllocate(outfcty->device, outfcty);

	if(pm == NULL)
		return BRE_FAIL;

	*ppmap = reinterpret_cast<br_device_pixelmap*>(pm);

	return BRE_OK;
}

static br_error BR_METHOD br_output_facility_gl_clutNew(br_output_facility *self, br_device_clut **pclut, br_token_value *tv)
{
	return BRE_FAIL;
}

static br_error BR_METHOD br_output_facility_gl_queryCapability(br_output_facility *self, br_token_value *buffer_in, br_token_value *buffer_out, br_size_t size_buffer_out)
{
	return BRE_FAIL;
}

static const struct br_output_facility_dispatch outputFacilityDispatch = {
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	br_output_facility_gl_free,
	br_output_facility_gl_identifier,
	br_output_facility_gl_type,
	br_output_facility_gl_isType,
	br_output_facility_gl_device,
	br_output_facility_gl_space,

	br_output_facility_gl_queryTemplate,
	BR_CMETHOD(br_object,				query),
	BR_CMETHOD(br_object,				queryBuffer),
	BR_CMETHOD(br_object,				queryMany),
	BR_CMETHOD(br_object,				queryManySize),
	BR_CMETHOD(br_object,				queryAll),
	BR_CMETHOD(br_object,				queryAllSize),

	br_output_facility_gl_listQuery,
	BR_CMETHOD(br_object_container,		tokensMatchBegin),
	BR_CMETHOD(br_object_container,		tokensMatch),
	BR_CMETHOD(br_object_container,		tokensMatchEnd),
	BR_CMETHOD(br_object_container,		addFront),
	BR_CMETHOD(br_object_container,		removeFront),
	BR_CMETHOD(br_object_container,		remove),
	BR_CMETHOD(br_object_container,		find),
	BR_CMETHOD(br_object_container,		findMany),
	BR_CMETHOD(br_object_container,		count),

	br_output_facility_gl_validSource,
	br_output_facility_gl_pixelmapNew,
	br_output_facility_gl_clutNew,
	br_output_facility_gl_queryCapability,
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

static br_output_facility_gl *createMode(br_device_gl *dev, br_int_32 monitorIndex, const GLFWvidmode *mode)
{
	br_uint_16 type;
	if(glfwToBRrenderSurfaceType(mode, &type) != BRE_OK)
		return nullptr;

	br_output_facility_gl *self = reinterpret_cast<br_output_facility_gl*>(BrResAllocate(dev->res, sizeof(br_output_facility_gl), BR_MEMORY_OBJECT));
	self->dispatch = &outputFacilityDispatch;
	self->device = dev;


	self->object_list = BrObjectListAllocate(self);

	/* Fill in display format */
	self->colour_type = type;
	self->colour_bits = BrType_BPP(type);
	self->indexed = (type <= BR_PMT_INDEX_8);
	self->monitor = monitorIndex;

	self->width = mode->width;
	self->height = mode->height;

	/* Attach a descriptive identifier */
	char tmp[64];
	BrSprintfN(tmp, sizeof(tmp) - 1, "%dx%dx%d", self->width, self->height, self->colour_bits);
	log_trace("GLFW", "Registering screen mode %s.", tmp);
	self->identifier = BrResStrDup(self, tmp);

	self->renderer_facility = nullptr;
	return self;
}

br_error OutputFacilityGLEnumerate(br_device_gl *device)
{
	int numMonitors;
	GLFWmonitor **monitors = glfwGetMonitors(&numMonitors);
	if(!monitors)
		return BRE_FAIL;

	device->templates.outputFacilityTemplate = BrTVTemplateAllocate(device, outputFacilityTemplateEntries, BR_ASIZE(outputFacilityTemplateEntries));

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
			br_output_facility_gl *mode = createMode(device, i, &modes[j]);
			if(!mode)
				continue;

			/* If it succeeded, add it to the main list. */
			ObjectContainerAddFront(device, (br_object*)mode);
			++device->output_facility_count;

			RendererFacilityGLInit(mode);
		}
	}

	return BRE_OK;
}