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
#define _A(f)    ((br_int_32)(f))

static struct br_tv_template_entry deviceTemplateEntries[] = {
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

	BRT_WINDOW_HANDLE_H,
	(br_token)BR_NULL_TOKEN
};

static struct br_tv_template deviceTemplate = {
	BR_ASIZE(deviceTemplateEntries),
	deviceTemplateEntries
};

static void BR_METHOD br_device_gl_free(br_object *self)
{
	br_device_gl *dev = reinterpret_cast<br_device_gl*>(self);

	VIDEO_Close(&dev->video);
	BrObjectContainerFree(reinterpret_cast<br_object_container*>(self), (br_token)BR_NULL_TOKEN, NULL, NULL);
}

static const char * BR_METHOD br_object_gl_identifier(br_object *self)
{
	br_device_gl *dev = reinterpret_cast<br_device_gl*>(self);
	return dev->identifier;
}

static br_token BR_METHOD br_device_gl_type(br_object *self)
{
	return BRT_DEVICE;
}

static br_boolean BR_METHOD br_device_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_DEVICE) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device * BR_METHOD br_object_gl_device(br_object *self)
{
	br_device_gl *dev = reinterpret_cast<br_device_gl*>(self);
	return reinterpret_cast<br_device*>(dev->device);
}

static br_int_32 BR_METHOD br_device_gl_space(br_object *self)
{
	return sizeof(br_device_gl);
}

static struct br_tv_template * BR_METHOD br_device_gl_templateQuery(br_object *self)
{
	br_device_gl *dev = reinterpret_cast<br_device_gl*>(self);
	return dev->templates.deviceTemplate;
}

static void * BR_METHOD br_device_gl_listQuery(br_object_container *self)
{
	br_device_gl *dev = reinterpret_cast<br_device_gl*>(self);
	return dev->object_list;
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


void * br_device_gl_tokensMatchBegin(struct br_object_container *self, br_token t, br_token_value *tv)
{
	struct token_match *tm;
	br_int_32 i;

	if(tv == NULL)
		return NULL;

	tm = (struct token_match*)BrResAllocate(self, sizeof(*tm), BR_MEMORY_APPLICATION);
	tm->original = tv;

	for(i = 0; tv[i].t != BR_NULL_TOKEN; i++)
		;

	tm->n = i + 1;
	tm->query = (br_token_value*)BrResAllocate(tm, tm->n * sizeof(br_token_value), BR_MEMORY_APPLICATION);
	BrMemCpy(tm->query, tv, i * sizeof(br_token_value));
	return (void *)tm;
}

br_boolean br_device_gl_tokensMatch(struct br_object_container *self, br_object *h, void *arg)
{
	struct token_match *tm = (struct token_match*)arg;
	br_size_t s;
	br_int_32 n;

	if(arg == NULL)
		return BR_TRUE;

	/*
	* Make a query on the object and then compare with the original tokens
	*/
	//ObjectQueryManySize(h, &s, tm->query);
	((br_object_container*)h)->dispatch->_queryManySize(h, &s, tm->query);

	if(s > tm->extra_size)
	{
		if(tm->extra)
			BrResFree(tm->extra);
		tm->extra = BrResAllocate(tm, s, BR_MEMORY_APPLICATION);
		tm->extra_size = s;
	}

	//ObjectQueryMany(h, tm->query, tm->extra, tm->extra_size, &n);
	((br_object_container*)h)->dispatch->_queryMany(h, tm->query, tm->extra, tm->extra_size, &n);

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

void br_device_gl_tokensMatchEnd(struct br_object_container *self, void *arg)
{
	if(arg)
		BrResFree(arg);
}

static struct br_device_dispatch deviceDispatch = {
	nullptr,	// reserved0
	nullptr,	// reserved1
	nullptr,	// reserved2
	nullptr,	// reserved3

	br_device_gl_free,
	br_object_gl_identifier,
	br_device_gl_type,
	br_device_gl_isType,
	br_object_gl_device,
	br_device_gl_space,

	br_device_gl_templateQuery,
	BR_CMETHOD(br_object,			query),
	BR_CMETHOD(br_object, 			queryBuffer),
	BR_CMETHOD(br_object, 			queryMany),
	BR_CMETHOD(br_object, 			queryManySize),
	BR_CMETHOD(br_object, 			queryAll),
	BR_CMETHOD(br_object, 			queryAllSize),

	br_device_gl_listQuery,
	br_device_gl_tokensMatchBegin,
	br_device_gl_tokensMatch,
	br_device_gl_tokensMatchEnd,
	BR_CMETHOD(br_object_container,	addFront),
	BR_CMETHOD(br_object_container,	removeFront),
	BR_CMETHOD(br_object_container,	remove),
	BR_CMETHOD(br_object_container,	find),
	BR_CMETHOD(br_object_container, findMany),
	BR_CMETHOD(br_object_container, count),
};

/*
* Structure used to unpack driver arguments
*/
#define F(f)	offsetof(br_device_gl, f)

static struct br_tv_template_entry deviceArgsTemplateEntries[] = {
	{BRT(WINDOW_HANDLE_H),	F(glfwWindow),						BRTV_SET,	BRTV_CONV_COPY,},
	{BRT(MSAA_SAMPLES_I32),	F(msaaSamples),						BRTV_SET,	BRTV_CONV_COPY,},
};
#undef F

static struct br_tv_template deviceArgsTemplate = {
	BR_ASIZE(deviceArgsTemplateEntries),
	deviceArgsTemplateEntries
};

br_device_gl *DeviceGLAllocate(const char *arguments)
{
	br_device_gl *self = reinterpret_cast<br_device_gl*>(BrResAllocate(nullptr, sizeof(br_device_gl), BR_MEMORY_OBJECT));
	self->identifier = "OpenGL";
	self->dispatch = &deviceDispatch;
	self->device = self;

	self->object_list = BrObjectListAllocate(self);
	self->output_facility_count = 0;

	memset(&self->templates, 0, sizeof(self->templates));
	self->templates.deviceTemplate = BrTVTemplateAllocate(self, deviceTemplateEntries, BR_ASIZE(deviceTemplateEntries));

	br_int_32 argCount = 0;
	if(arguments != NULL)
	{
		br_token_value args_tv[256];
		BrStringToTokenValue(args_tv, sizeof(args_tv), (char*)arguments);
		deviceArgsTemplate.res = self->res;
		BrTokenValueSetMany(self, &argCount, NULL, args_tv, &deviceArgsTemplate);
	}

	if(self->glfwWindow == nullptr)
	{
		BrResFree(self);
		nullptr;
	}

	if(VIDEO_Open(&self->video) == nullptr)
	{
		BrResFree(self);
		return nullptr;
	}

	if(self->msaaSamples < 0)
		self->msaaSamples = 0;
	else if(self->msaaSamples > self->video.maxSamples)
		self->msaaSamples = self->video.maxSamples;

	return self;
}