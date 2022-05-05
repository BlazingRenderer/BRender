#include "drv_ip.hpp"
#include "renderer.hpp"
#include "matrix.h"
#include <log.h>

#define F(f) offsetof(struct br_renderer_gl, f)

static struct br_tv_template_entry templateEntries[] = {
	{BRT(IDENTIFIER_CSTR),		F(identifier),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	//{BRT(PARTS_TL),				0,							_Q | _A,	BRTV_CONV_CUSTOM, (br_int_32)&customPartsConv},
};
#undef F

static void BR_CMETHOD_DECL(br_renderer_gl, sceneBegin)(br_object *_self)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);

	/* First draw call, so do all the per-scene crap */
	while(glGetError() != GL_NO_ERROR);

	GLCACHE_Reset(&self->state.cache);
	GLCACHE_UpdateScene(&self->state.cache, self->state.current);

	HVIDEO hVideo = &self->device->video;
	glUseProgram(hVideo->brenderProgram.program);
	glBindFramebuffer(GL_FRAMEBUFFER, self->state.cache.fbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingScene, hVideo->brenderProgram.uboScene);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(self->state.cache.scene), &self->state.cache.scene);

	/* Bind the model UBO here, it's faster than doing it for each model group */
	glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingModel, hVideo->brenderProgram.uboModel);

	if(self->pixelmap->msaa_samples)
		glEnable(GL_MULTISAMPLE);

	self->has_begun = true;
}

void BR_CMETHOD_DECL(br_renderer_gl, sceneEnd)(br_object *_self)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_MULTISAMPLE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
	self->has_begun = false;
}

static void BR_CMETHOD_DECL(br_renderer_gl, free)(br_object *self)
{
	br_renderer_gl *rend = reinterpret_cast<br_renderer_gl*>(self);

	ObjectContainerRemove(rend->renderer_facility, self);
	BrObjectContainerFree((br_object_container *)self, (br_token)BR_NULL_TOKEN, nullptr, nullptr);

	rend->~br_renderer_gl();
	BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_renderer_gl, identifier)(br_object *self)
{
	return reinterpret_cast<br_renderer_gl*>(self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_renderer_gl, type)(br_object *self)
{
	return BRT_RENDERER;
}

static br_boolean BR_CMETHOD_DECL(br_renderer_gl, isType)(br_object *self, br_token t)
{
	return (t == BRT_RENDERER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_renderer_gl, device)(br_object *self)
{
	br_renderer_gl *rend = reinterpret_cast<br_renderer_gl*>(self);
	return reinterpret_cast<br_device*>(rend->device);
}

static br_size_t BR_CMETHOD_DECL(br_renderer_gl, space)(br_object *self)
{
	return sizeof(br_renderer_gl);
}

static struct br_tv_template * BR_CMETHOD_DECL(br_renderer_gl, templateQuery)(br_object *_self)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);

	if(self->device->templates.renderer == nullptr)
		self->device->templates.renderer = BrTVTemplateAllocate(self->device, templateEntries, BR_ASIZE(templateEntries));

	return self->device->templates.renderer;
}

static void *BR_CMETHOD_DECL(br_renderer_gl, listQuery)(br_object_container *self)
{
	return reinterpret_cast<br_renderer_gl*>(self)->object_list;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, validDestination)(br_renderer *self, br_boolean *bp, br_object *h)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateStoredNew)(struct br_renderer *_self, struct br_renderer_state_stored **pss, br_uint_32 mask, br_token_value *tv)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);

	//log_trace("GLREND", "%s", __FUNCTION__);

	br_renderer_state_stored_gl *state = RendererStateStoredGLAllocate(self, self->state.current, mask, tv);
	*pss = (br_renderer_state_stored*)state;
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateStoredAvail)(struct br_renderer *self, br_int_32 *psize, br_uint_32 mask, br_token_value *tv)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, bufferStoredNew)(struct br_renderer *_self, struct br_buffer_stored **psm, br_token use, struct br_device_pixelmap *pm, br_token_value *tv)
{
	//log_trace("GLREND", "%s", __FUNCTION__);

	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	br_buffer_stored_gl *buffer = BufferStoredGLAllocate(self, use, pm, tv);
	*psm = (br_buffer_stored*)buffer;

	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, bufferStoredAvail)(struct br_renderer *self, br_int_32 *space, br_token use, br_token_value *tv)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, flush)(struct br_renderer *_self, br_boolean wait)
{
	RendererGLSceneEnd(_self);
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, testRender)(struct br_renderer *self, br_token type, union brp_vertex *v0, union brp_vertex *v1, union brp_vertex *v2)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, synchronise)(struct br_renderer *self, br_token sync_type, br_boolean block)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModeSet)(struct br_renderer *self, br_token mode)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModeQuery)(struct br_renderer *self, br_token *mode)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModeDefault)(struct br_renderer *self)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModePush)(struct br_renderer *self)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, commandModePop)(struct br_renderer *self)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}





static br_error BR_CMETHOD_DECL(br_renderer_gl, statePush)(struct br_renderer *_self, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", "%s", __FUNCTION__);
	return GLSTATE_Push(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, statePop)(struct br_renderer *_self, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", "%s", __FUNCTION__);
	return GLSTATE_Pop(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateSave)(struct br_renderer *_self, struct br_renderer_state_stored *_save, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	br_renderer_state_stored_gl *save = reinterpret_cast<br_renderer_state_stored_gl*>(_save);
	//log_trace("GLREND", "%s", __FUNCTION__);

	GLSTATE_Copy(&save->state, self->state.current, mask);
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateRestore)(struct br_renderer *_self, struct br_renderer_state_stored *_save, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	br_renderer_state_stored_gl *save = reinterpret_cast<br_renderer_state_stored_gl*>(_save);
	//log_trace("GLREND", "%s", __FUNCTION__);

	GLSTATE_Copy(self->state.current, &save->state, mask);
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateDefault)(struct br_renderer *_self, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", "%s", __FUNCTION__);
	GLSTATE_Default(&self->state, mask);
	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateMask)(struct br_renderer *self, br_uint_32 *mask, br_token *parts, int n_parts)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}






/*
* Setting current state
*/
static br_error BR_CMETHOD_DECL(br_renderer_gl, partSet)(struct br_renderer *_self, br_token part, br_int_32 index, br_token t, br_value_passthrough value)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", "%s", __FUNCTION__);

	struct br_tv_template *tp = GLSTATE_GetStateTemplate(&self->state, part, index);

	if(tp == nullptr)
		return BRE_FAIL;

	br_uint_32 m = 0;
	br_error r = BrTokenValueSet(self->state.current, &m, t, BR_VALUE_PASSTHROUGH(value), tp);
	if(m)
		GLSTATE_TemplateActions(&self->state, m);

	return r;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partSetMany)(struct br_renderer *_self, br_token part, br_int_32 index, br_token_value * tv, br_int_32 *pcount)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//.log_trace("GLREND", "%s", __FUNCTION__);

	struct br_tv_template *tp = GLSTATE_GetStateTemplate(&self->state, part, index);

	if(tp == nullptr)
		return BRE_FAIL;

	br_uint_32 m = 0;
	br_error r = BrTokenValueSetMany(self->state.current, pcount, &m, tv, tp);
	if(m)
		GLSTATE_TemplateActions(&self->state, m);

	return r;
}

/*
* Reading current state
*/
static br_error BR_CMETHOD_DECL(br_renderer_gl, partQuery)(struct br_renderer *self, br_token part, br_int_32 index, void *pvalue, br_token t)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryBuffer)(struct br_renderer *_self, br_token part, br_int_32 index, void *pvalue, void *buffer, br_size_t buffer_size, br_token t)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", "%s", __FUNCTION__);

	struct br_tv_template *tp = GLSTATE_GetStateTemplate(&self->state, part, index);
	if(tp == nullptr)
		return BRE_FAIL;

	return BrTokenValueQuery(pvalue, buffer, buffer_size, t, self->state.current, tp);
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryMany)(struct br_renderer *self, br_token part, br_int_32 index, br_token_value *tv, void *extra, br_size_t extra_size, br_int_32 *pcount)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryManySize)(struct br_renderer *self, br_token part, br_int_32 index, br_size_t *pextra_size, br_token_value *tv)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryAll)(struct br_renderer *self, br_token part, br_int_32 index, br_token_value *buffer, br_size_t buffer_size)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryAllSize)(struct br_renderer *self, br_token part, br_int_32 index, br_size_t *psize)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partQueryCapability)(struct br_renderer *self, br_token part, br_int_32 index, br_token_value *buffer, br_size_t buffer_size)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, partIndexQuery)(struct br_renderer *self, br_token part, br_int_32 *pnindex)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, stateQueryPerformance)(struct br_renderer *self, br_fixed_lu *speed)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}



static br_error BR_CMETHOD_DECL(br_renderer_gl, modelMul)(struct br_renderer *_self, br_matrix34_f *m)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", "%s", __FUNCTION__);

	br_matrix34 om = self->state.current->matrix.model_to_view;

	BrMatrix34Mul(&self->state.current->matrix.model_to_view, (br_matrix34 *)m, &om);

	self->state.current->matrix.model_to_view_hint = BRT_NONE;

	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, modelPopPushMul)(struct br_renderer *_self, br_matrix34_f *m)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", "%s", __FUNCTION__);
	if(self->state.top == 0)
		return BRE_UNDERFLOW;

	BrMatrix34Mul(
		&self->state.current->matrix.model_to_view,
		(br_matrix34 *)m,
		&self->state.stack[0].matrix.model_to_view);

	self->state.current->matrix.model_to_view_hint = BRT_NONE;

	return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_renderer_gl, modelInvert)(struct br_renderer *_self)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", "%s", __FUNCTION__);

	br_matrix34 old;

	BrMatrix34Copy(&old, &self->state.current->matrix.model_to_view);

	if(self->state.current->matrix.model_to_view_hint == BRT_LENGTH_PRESERVING)
		BrMatrix34LPInverse(&self->state.current->matrix.model_to_view, &old);
	else
		BrMatrix34Inverse(&self->state.current->matrix.model_to_view, &old);

	return BRE_OK;
}


static br_error BR_CMETHOD_DECL(br_renderer_gl, boundsTest)(struct br_renderer *_self, br_token *r, br_bounds3_f *bounds)
{
	//log_trace("GLREND", "%s", __FUNCTION__);
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);

	br_token GLOnScreenCheck(br_renderer_gl *self, const br_matrix4 *model_to_screen, const br_bounds3_f *bounds);

	// FIXME: Should probably cache this.
	br_matrix4 m2s;
	BrMatrix4Mul34(&m2s, &self->state.current->matrix.model_to_view, &self->state.current->matrix.view_to_screen);
	*r = GLOnScreenCheck(self, &m2s, bounds);
	return BRE_OK;
}


static br_error BR_CMETHOD_DECL(br_renderer_gl, coverageTest)(struct br_renderer *self, br_float *r, br_bounds3_f *bounds)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}


static br_error BR_CMETHOD_DECL(br_renderer_gl, viewDistance)(struct br_renderer *self, br_float *r)
{
	log_trace("GLREND", "%s", __FUNCTION__);
	return BRE_FAIL;
}


/*
* Default dispatch table for renderer
*/
static const struct br_renderer_dispatch rendererDispatch = {
	.__reserved0			= BR_CMETHOD(br_renderer_gl,		sceneBegin),
	.__reserved1			= BR_CMETHOD(br_renderer_gl,		sceneEnd),
	.__reserved2			= nullptr,
	.__reserved3			= nullptr,
	._free					= BR_CMETHOD(br_renderer_gl,		free),
	._identifier			= BR_CMETHOD(br_renderer_gl,		identifier),
	._type					= BR_CMETHOD(br_renderer_gl,		type),
	._isType				= BR_CMETHOD(br_renderer_gl,		isType),
	._device				= BR_CMETHOD(br_renderer_gl,		device),
	._space					= BR_CMETHOD(br_renderer_gl,		space),
	._templateQuery			= BR_CMETHOD(br_renderer_gl,		templateQuery),
	._query					= BR_CMETHOD(br_object,				query),
	._queryBuffer			= BR_CMETHOD(br_object,				queryBuffer),
	._queryMany				= BR_CMETHOD(br_object,				queryMany),
	._queryManySize			= BR_CMETHOD(br_object,				queryManySize),
	._queryAll				= BR_CMETHOD(br_object,				queryAll),
	._queryAllSize			= BR_CMETHOD(br_object,				queryAllSize),
	._listQuery				= BR_CMETHOD(br_renderer_gl,		listQuery),
	._tokensMatchBegin		= BR_CMETHOD(br_object_container,	tokensMatchBegin),
	._tokensMatch			= BR_CMETHOD(br_object_container,	tokensMatch),
	._tokensMatchEnd		= BR_CMETHOD(br_object_container,	tokensMatchEnd),
	._addFront				= BR_CMETHOD(br_object_container,	addFront),
	._removeFront			= BR_CMETHOD(br_object_container,	removeFront),
	._remove				= BR_CMETHOD(br_object_container,	remove),
	._find					= BR_CMETHOD(br_object_container,	find),
	._findMany				= BR_CMETHOD(br_object_container,	findMany),
	._count					= BR_CMETHOD(br_object_container,	count),

	._validDestination		= BR_CMETHOD(br_renderer_gl,		validDestination),
	._stateStoredNew		= BR_CMETHOD(br_renderer_gl,		stateStoredNew),
	._stateStoredAvail		= BR_CMETHOD(br_renderer_gl,		stateStoredAvail),
	._bufferStoredNew		= BR_CMETHOD(br_renderer_gl,		bufferStoredNew),
	._bufferStoredAvail		= BR_CMETHOD(br_renderer_gl,		bufferStoredAvail),
	._partSet				= BR_CMETHOD(br_renderer_gl,		partSet),
	._partSetMany			= BR_CMETHOD(br_renderer_gl,		partSetMany),
	._partQuery				= BR_CMETHOD(br_renderer_gl,		partQuery),
	._partQueryBuffer		= BR_CMETHOD(br_renderer_gl,		partQueryBuffer),
	._partQueryMany			= BR_CMETHOD(br_renderer_gl,		partQueryMany),
	._partQueryManySize		= BR_CMETHOD(br_renderer_gl,		partQueryManySize),
	._partQueryAll			= BR_CMETHOD(br_renderer_gl,		partQueryAll),
	._partQueryAllSize		= BR_CMETHOD(br_renderer_gl,		partQueryAllSize),
	._partIndexQuery		= BR_CMETHOD(br_renderer_gl,		partIndexQuery),
	._modelMul				= BR_CMETHOD(br_renderer_gl,		modelMul),
	._modelPopPushMul		= BR_CMETHOD(br_renderer_gl,		modelPopPushMul),
	._modelInvert			= BR_CMETHOD(br_renderer_gl,		modelInvert),
	._statePush				= BR_CMETHOD(br_renderer_gl,		statePush),
	._statePop				= BR_CMETHOD(br_renderer_gl,		statePop),
	._stateSave				= BR_CMETHOD(br_renderer_gl,		stateSave),
	._stateRestore			= BR_CMETHOD(br_renderer_gl,		stateRestore),
	._stateMask				= BR_CMETHOD(br_renderer_gl,		stateMask),
	._stateDefault			= BR_CMETHOD(br_renderer_gl,		stateDefault),
	._boundsTest			= BR_CMETHOD(br_renderer_gl,		boundsTest),
	._coverageTest			= BR_CMETHOD(br_renderer_gl,		coverageTest),
	._viewDistance			= BR_CMETHOD(br_renderer_gl,		viewDistance),
	._commandModeSet		= BR_CMETHOD(br_renderer_gl,		commandModeSet),
	._commandModeQuery		= BR_CMETHOD(br_renderer_gl,		commandModeQuery),
	._commandModeDefault	= BR_CMETHOD(br_renderer_gl,		commandModeDefault),
	._commandModePush		= BR_CMETHOD(br_renderer_gl,		commandModePush),
	._commandModePop		= BR_CMETHOD(br_renderer_gl,		commandModePop),
	._flush					= BR_CMETHOD(br_renderer_gl,		flush),
	._synchronise			= BR_CMETHOD(br_renderer_gl,		synchronise),
	._testRender			= BR_CMETHOD(br_renderer_gl,		testRender),
	._partQueryCapability	= BR_CMETHOD(br_renderer_gl,		partQueryCapability),
	._stateQueryPerformance	= BR_CMETHOD(br_renderer_gl,		stateQueryPerformance),
};

br_renderer_gl *RendererGLAllocate(br_device_gl *device, br_renderer_facility_gl *facility, br_device_pixelmap_gl *dest)
{
	/* Check that destination is valid */
	if(dest == nullptr || ObjectDevice(dest) != reinterpret_cast<br_device*>(device))
		return nullptr;

	br_renderer_gl *self = BrResAllocate<br_renderer_gl>(facility, BR_MEMORY_OBJECT);

	new (self) br_renderer_gl();

	self->dispatch = &rendererDispatch;
	self->identifier = facility->identifier;
	self->device = device;
	self->object_list = BrObjectListAllocate(self);
	self->pixelmap = dest;
	self->renderer_facility = facility;

	ObjectContainerAddFront(facility, self);

	GLSTATE_Init(&self->state, self->device);
	/*
	* State starts out as default
	*/
	RendererStateDefault(self, (br_uint_32)BR_STATE_ALL);

	self->has_begun = false;
	return self;
}