#include <cstddef>
#include "drv_ip.hpp"


#define DEVICE_VERSION  BR_VERSION(1,0,0)

/*
* Device info. template
*/
static const char deviceTitle[] = "OpenGL";

static const char deviceCreator[] = "crocguy0688 <crocguy0688@protonmail.com>";
static const char deviceProduct[] = "OpenGL";

/*
* Device info. template
*/
#define _F(f)	offsetof(br_device_gl, f)
#define _A(f)    ((br_uintptr_t)(f))

static struct br_tv_template_entry templateEntries[] = {
	{BRT(IDENTIFIER_CSTR),		_F(identifier),			BRTV_QUERY | BRTV_ALL,				BRTV_CONV_COPY,},
	{BRT(VERSION_U32),			0,  		            BRTV_QUERY | BRTV_ALL,				BRTV_CONV_DIRECT, DEVICE_VERSION},
	{BRT(BRENDER_VERSION_U32),	0,	    	            BRTV_QUERY | BRTV_ALL,				BRTV_CONV_DIRECT, __BRENDER__},
	{BRT(DDI_VERSION_U32),		0,	                    BRTV_QUERY | BRTV_ALL,				BRTV_CONV_DIRECT, __BRENDER_DDI__},
	{BRT(CREATOR_CSTR),			_A(deviceCreator),	    BRTV_QUERY | BRTV_ALL | BRTV_ABS,	BRTV_CONV_COPY,},
	{BRT(TITLE_CSTR),			_A(deviceTitle),	    BRTV_QUERY | BRTV_ALL | BRTV_ABS,	BRTV_CONV_COPY,},
	{BRT(PRODUCT_CSTR),			_A(deviceProduct),	    BRTV_QUERY | BRTV_ALL | BRTV_ABS,	BRTV_CONV_COPY,},
};
#undef _F
#undef _A

/*
* List of tokens which are not significant in matching (for output facilities)
*/
br_token insignificantMatchTokens[] = {
	BRT(WINDOW_HANDLE_H),
	BRT(OPENGL_MSAA_SAMPLES_I32),
	(br_token)BR_NULL_TOKEN
};

static void BR_CMETHOD_DECL(br_device_gl, free)(br_object *self)
{
	br_device_gl *dev = reinterpret_cast<br_device_gl*>(self);
	VIDEO_Close(&dev->video);
	BrObjectContainerFree(reinterpret_cast<br_object_container*>(self), (br_token)BR_NULL_TOKEN, nullptr, nullptr);
	BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_object_gl, identifier)(br_object *self)
{
	return reinterpret_cast<br_device_gl*>(self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_device_gl, type)(br_object *self)
{
	return BRT_DEVICE;
}

static br_boolean BR_CMETHOD_DECL(br_device_gl, isType)(br_object *self, br_token t)
{
	return (t == BRT_DEVICE) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device * BR_CMETHOD_DECL(br_object_gl, device)(br_object *self)
{
	br_device_gl *dev = reinterpret_cast<br_device_gl*>(self);
	return reinterpret_cast<br_device*>(dev->device);
}

static br_size_t BR_CMETHOD_DECL(br_device_gl, space)(br_object *self)
{
	return sizeof(br_device_gl);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_gl, templateQuery)(br_object *_self)
{
	br_device_gl *self = reinterpret_cast<br_device_gl*>(_self);

    if(self->templates.device == nullptr)
        self->templates.device = BrTVTemplateAllocate(self, templateEntries, BR_ASIZE(templateEntries));

	return self->device->templates.device;
}

static void *BR_CMETHOD_DECL(br_device_gl, listQuery)(br_object_container *self)
{
	return reinterpret_cast<br_device_gl*>(self)->object_list;
}

/*
* Default token matching does nothing other than make all tokens match
*
* makes a copy of token/value list
*/
struct token_match
{
	br_token_value *original;
	br_token_value *query;
	br_int_32 n;
	void *extra;
	br_size_t extra_size;
};


void *BR_CMETHOD_DECL(br_device_gl, tokensMatchBegin)(struct br_object_container *self, br_token t, br_token_value *tv)
{
	if(tv == nullptr)
		return nullptr;

	token_match *tm = BrResAllocate<token_match>(self, BR_MEMORY_APPLICATION);
	tm->original = tv;

	br_int_32 i;
	for(i = 0; tv[i].t != BR_NULL_TOKEN; i++)
		;

	tm->n = i + 1;
	tm->query = (br_token_value*)BrResAllocate(tm, tm->n * sizeof(br_token_value), BR_MEMORY_APPLICATION);
	BrMemCpy(tm->query, tv, i * sizeof(br_token_value));
	return (void *)tm;
}

br_boolean BR_CMETHOD_DECL(br_device_gl, tokensMatch)(struct br_object_container *self, br_object *h, void *arg)
{
	struct token_match *tm = (struct token_match*)arg;
	br_size_t s;
	br_int_32 n;

	if(arg == nullptr)
		return BR_TRUE;

	/* Make a query on the object and then compare with the original tokens */
	ObjectQueryManySize(h, &s, tm->query);

	if(s > tm->extra_size)
	{
		if(tm->extra)
			BrResFree(tm->extra);
		tm->extra = BrResAllocate(tm, s, BR_MEMORY_APPLICATION);
		tm->extra_size = s;
	}

	ObjectQueryMany(h, tm->query, tm->extra, tm->extra_size, &n);

	/*
	* Ensure that all tokens were found
	*/
	if(tm->query[n].t != BR_NULL_TOKEN)
		return BR_FALSE;

	/*
	* Compare the two token lists
	*/
	return BrTokenValueComparePartial(tm->original, tm->query, insignificantMatchTokens);
}

void BR_CMETHOD_DECL(br_device_gl, tokensMatchEnd)(struct br_object_container *self, void *arg)
{
	if(arg)
		BrResFree(arg);
}

static struct br_device_dispatch deviceDispatch = {
	.__reserved0		= nullptr,
	.__reserved1		= nullptr,
	.__reserved2		= nullptr,
	.__reserved3		= nullptr,

	._free				= BR_CMETHOD(br_device_gl,			free),
	._identifier		= BR_CMETHOD(br_object_gl,			identifier),
	._type				= BR_CMETHOD(br_device_gl,			type),
	._isType			= BR_CMETHOD(br_device_gl,			isType),
	._device			= BR_CMETHOD(br_object_gl,			device),
	._space				= BR_CMETHOD(br_device_gl,			space),

	._templateQuery		= BR_CMETHOD(br_device_gl,			templateQuery),
	._query				= BR_CMETHOD(br_object,				query),
	._queryBuffer		= BR_CMETHOD(br_object,				queryBuffer),
	._queryMany			= BR_CMETHOD(br_object,				queryMany),
	._queryManySize		= BR_CMETHOD(br_object,				queryManySize),
	._queryAll			= BR_CMETHOD(br_object,				queryAll),
	._queryAllSize		= BR_CMETHOD(br_object,				queryAllSize),

	._listQuery			= BR_CMETHOD(br_device_gl,			listQuery),
	._tokensMatchBegin	= BR_CMETHOD(br_device_gl,			tokensMatchBegin),
	._tokensMatch		= BR_CMETHOD(br_device_gl,			tokensMatch),
	._tokensMatchEnd	= BR_CMETHOD(br_device_gl,			tokensMatchEnd),
	._addFront			= BR_CMETHOD(br_object_container,	addFront),
	._removeFront		= BR_CMETHOD(br_object_container,	removeFront),
	._remove			= BR_CMETHOD(br_object_container,	remove),
	._find				= BR_CMETHOD(br_object_container,	find),
	._findMany			= BR_CMETHOD(br_object_container,	findMany),
	._count				= BR_CMETHOD(br_object_container,	count),
};

/*
* Structure used to unpack driver arguments
*/
#define F(f)	offsetof(br_device_gl, f)

static struct br_tv_template_entry deviceArgsTemplateEntries[] = {
	{BRT(WINDOW_HANDLE_H),				F(glfwWindow),	BRTV_SET,	BRTV_CONV_COPY,},
	{BRT(OPENGL_VERTEX_SHADER_STR),		F(vertShader),	BRTV_SET,	BRTV_CONV_COPY,},
	{BRT(OPENGL_FRAGMENT_SHADER_STR),	F(fragShader),	BRTV_SET,	BRTV_CONV_COPY,},
};
#undef F

static struct br_tv_template deviceArgsTemplate = {
	BR_ASIZE(deviceArgsTemplateEntries),
	deviceArgsTemplateEntries
};

br_device_gl *DeviceGLAllocate(const char *arguments)
{
	br_device_gl *self = BrResAllocate<br_device_gl>(nullptr, BR_MEMORY_OBJECT);
	self->dispatch = &deviceDispatch;
	self->identifier = "OpenGL";
	self->device = self;

	self->object_list = BrObjectListAllocate(self);
	self->output_facility_count = 0;

	memset(&self->templates, 0, sizeof(self->templates));

	br_int_32 argCount = 0;
	if(arguments != nullptr)
	{
		br_token_value args_tv[256];
		BrStringToTokenValue(args_tv, sizeof(args_tv), arguments);
		deviceArgsTemplate.res = self;
		BrTokenValueSetMany(self, &argCount, nullptr, args_tv, &deviceArgsTemplate);
	}

	if(self->glfwWindow == nullptr || self->vertShader == nullptr || self->fragShader == nullptr)
	{
		BrResFree(self);
		return nullptr;
	}

	if(VIDEO_Open(&self->video, self->vertShader, self->fragShader) == nullptr)
	{
		BrResFree(self);
		return nullptr;
	}

	return self;
}