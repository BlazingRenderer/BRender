#include "drv_ip.hpp"
#include "renderer.hpp"
#include "matrix.h"
#include <log.h>

static void br_renderer_gl_sceneBegin(br_object *_self)
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

	if(self->device->msaaSamples)
		glEnable(GL_MULTISAMPLE);
	self->has_begun = true;
}

void br_renderer_gl_sceneEnd(br_object *_self)
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

static void br_renderer_gl_free(br_object *self)
{
	br_renderer_gl *rend = reinterpret_cast<br_renderer_gl*>(self);

	ObjectContainerRemove(rend->renderer_facility, (br_object *)self);

	rend->~br_renderer_gl();
	BrObjectContainerFree((br_object_container *)self, (br_token)BR_NULL_TOKEN, NULL, NULL);

	BrResFreeNoCallback(self);
}

static const char *br_renderer_gl_identifier(br_object *self)
{
	br_renderer_gl *rend = reinterpret_cast<br_renderer_gl*>(self);
	return rend->identifier;
}

static br_token br_renderer_gl_type(br_object *self)
{
	return BRT_RENDERER;
}

static br_boolean br_renderer_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_RENDERER) || (t == BRT_OBJECT);
}

static br_device *br_renderer_gl_device(br_object *self)
{
	br_renderer_gl *rend = reinterpret_cast<br_renderer_gl*>(self);
	return reinterpret_cast<br_device*>(rend->device);
}

static br_int_32 br_renderer_gl_space(br_object *self)
{
	return sizeof(br_renderer_gl);
}

#define F(f)	offsetof(struct br_renderer_gl, f)

static struct br_tv_template_entry rendererTemplateEntries[] = {
	{BRT(IDENTIFIER_CSTR),		F(identifier),				BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	//{BRT(PARTS_TL),				0,							_Q | _A,	BRTV_CONV_CUSTOM, (br_int_32)&customPartsConv},
};
#undef F

static struct br_tv_template * br_renderer_gl_templateQuery(br_object *self)
{
	br_renderer_gl *rend = reinterpret_cast<br_renderer_gl*>(self);
	return rend->device->templates.rendererTemplate;
}

static void * br_renderer_gl_listQuery(br_object_container *self)
{
	br_renderer_gl *rend = reinterpret_cast<br_renderer_gl*>(self);
	return rend->object_list;
}

static br_error br_renderer_gl_validDestination(br_renderer *self, br_boolean *bp, br_object *h)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_OK;
}

static br_error br_renderer_gl_stateStoredNew(struct br_renderer *_self, struct br_renderer_state_stored **pss, br_uint_32 mask, br_token_value *tv)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);

	//log_trace("GLREND", __FUNCTION__);

	br_renderer_state_stored_gl *state = RendererStateStoredGLAllocate(self, self->state.current, mask, tv);
	*pss = (br_renderer_state_stored*)state;
	return BRE_OK;
}

static br_error br_renderer_gl_stateStoredAvail(struct br_renderer *self, br_int_32 *psize, br_uint_32 mask, br_token_value *tv)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

static br_error br_renderer_gl_bufferStoredNew(struct br_renderer *_self, struct br_buffer_stored **psm, br_token use, struct br_device_pixelmap *pm, br_token_value *tv)
{
	//log_trace("GLREND", __FUNCTION__);

	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	br_buffer_stored_gl *buffer = BufferStoredGLAllocate(self, use, pm, tv);
	*psm = (br_buffer_stored*)buffer;

	return BRE_OK;
}

static br_error br_renderer_gl_bufferStoredAvail(struct br_renderer *self, br_int_32 *space, br_token use, br_token_value *tv)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

static br_error br_renderer_gl_flush(struct br_renderer *_self, br_boolean wait)
{
	//log_trace("GLREND", __FUNCTION__);
	//br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	/* sceneEnd() */
	_self->dispatch->__reserved1(reinterpret_cast<br_object*>(_self));
	return BRE_OK;
}

static br_error br_renderer_gl_testRender(struct br_renderer *self, br_token type, union brp_vertex *v0, union brp_vertex *v1, union brp_vertex *v2)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_OK;
}

static br_error br_renderer_gl_synchronise(struct br_renderer *self, br_token sync_type, br_boolean block)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_OK;
}

static br_error br_renderer_gl_commandModeSet(struct br_renderer *self, br_token mode)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

static br_error br_renderer_gl_commandModeQuery(struct br_renderer *self, br_token *mode)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

static br_error br_renderer_gl_commandModeDefault(struct br_renderer *self)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

static br_error br_renderer_gl_commandModePush(struct br_renderer *self)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

static br_error br_renderer_gl_commandModePop(struct br_renderer *self)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}





br_error br_renderer_gl_statePush(struct br_renderer *_self, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", __FUNCTION__);
	return GLSTATE_Push(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

br_error br_renderer_gl_statePop(struct br_renderer *_self, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", __FUNCTION__);
	return GLSTATE_Pop(&self->state, mask) ? BRE_OK : BRE_OVERFLOW;
}

br_error br_renderer_gl_stateSave(struct br_renderer *_self, struct br_renderer_state_stored *_save, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	br_renderer_state_stored_gl *save = reinterpret_cast<br_renderer_state_stored_gl*>(_save);
	//log_trace("GLREND", __FUNCTION__);

	GLSTATE_Copy(&save->state, self->state.current, mask);
	return BRE_OK;
}

br_error br_renderer_gl_stateRestore(struct br_renderer *_self, struct br_renderer_state_stored *_save, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	br_renderer_state_stored_gl *save = reinterpret_cast<br_renderer_state_stored_gl*>(_save);
	//log_trace("GLREND", __FUNCTION__);

	GLSTATE_Copy(self->state.current, &save->state, mask);
	return BRE_OK;
}

br_error br_renderer_gl_stateDefault(struct br_renderer *_self, br_uint_32 mask)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", __FUNCTION__);
	GLSTATE_Default(&self->state, mask);
	return BRE_OK;
}

br_error br_renderer_gl_stateMask(struct br_renderer *self, br_uint_32 *mask, br_token *parts, int n_parts)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}






/*
* Setting current state
*/
br_error br_renderer_gl_partSet(struct br_renderer *_self, br_token part, br_int_32 index, br_token t, br_uint_32 value)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", __FUNCTION__);

	struct br_tv_template *tp = GLSTATE_GetStateTemplate(&self->state, part, index);

	if(tp == nullptr)
		return BRE_FAIL;

	br_uint_32 m = 0;
	br_error r = BrTokenValueSet(self->state.current, &m, t, value, tp);
	if(m)
		GLSTATE_TemplateActions(&self->state, m);

	return r;
}

br_error br_renderer_gl_partSetMany(struct br_renderer *_self, br_token part, br_int_32 index, br_token_value * tv, br_int_32 *pcount)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//.log_trace("GLREND", __FUNCTION__);

	struct br_tv_template *tp = GLSTATE_GetStateTemplate(&self->state, part, index);

	if(tp == nullptr)
		return BRE_FAIL;

	br_uint_32 m;
	br_error r = BrTokenValueSetMany(self->state.current, pcount, &m, tv, tp);
	if(m)
		GLSTATE_TemplateActions(&self->state, m);

	return r;
}

/*
* Reading current state
*/
br_error br_renderer_gl_partQuery(struct br_renderer *self, br_token part, br_int_32 index, br_uint_32 *pvalue, br_token t)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_partQueryBuffer(struct br_renderer *_self, br_token part, br_int_32 index, br_uint_32 *pvalue, br_uint_32 *buffer, br_size_t buffer_size, br_token t)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", __FUNCTION__);

	struct br_tv_template *tp = GLSTATE_GetStateTemplate(&self->state, part, index);
	if(tp == nullptr)
		return BRE_FAIL;

	return BrTokenValueQuery(pvalue, buffer, buffer_size, t, self->state.current, tp);
}

br_error br_renderer_gl_partQueryMany(struct br_renderer *self, br_token part, br_int_32 index, br_token_value *tv, void *extra, br_size_t extra_size, br_int_32 *pcount)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_partQueryManySize(struct br_renderer *self, br_token part, br_int_32 index, br_size_t *pextra_size, br_token_value *tv)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_partQueryAll(struct br_renderer *self, br_token part, br_int_32 index, br_token_value *buffer, br_size_t buffer_size)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_partQueryAllSize(struct br_renderer *self, br_token part, br_int_32 index, br_size_t *psize)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_partQueryCapability(struct br_renderer *self, br_token part, br_int_32 index, br_token_value *buffer, br_size_t buffer_size)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_partIndexQuery(struct br_renderer *self, br_token part, br_int_32 *pnindex)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_stateQueryPerformance(struct br_renderer *self, br_fixed_lu *speed)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}



br_error br_renderer_gl_modelMulF(struct br_renderer *_self, br_matrix34_f *m)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", __FUNCTION__);

	br_matrix34 om = self->state.current->matrix.model_to_view;

	BrMatrix34Mul(&self->state.current->matrix.model_to_view, (br_matrix34 *)m, &om);

	self->state.current->matrix.model_to_view_hint = BRT_NONE;

	return BRE_OK;
}

br_error br_renderer_gl_modelPopPushMulF(struct br_renderer *_self, br_matrix34_f *m)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", __FUNCTION__);
	if(self->state.top == 0)
		return BRE_UNDERFLOW;

	BrMatrix34Mul(
		&self->state.current->matrix.model_to_view,
		(br_matrix34 *)m,
		&self->state.stack[0].matrix.model_to_view);

	self->state.current->matrix.model_to_view_hint = BRT_NONE;

	return BRE_OK;
}

br_error br_renderer_gl_modelInvert(struct br_renderer *_self)
{
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);
	//log_trace("GLREND", __FUNCTION__);

	br_matrix34 old;

	BrMatrix34Copy(&old, &self->state.current->matrix.model_to_view);

	if(self->state.current->matrix.model_to_view_hint == BRT_LENGTH_PRESERVING)
		BrMatrix34LPInverse(&self->state.current->matrix.model_to_view, &old);
	else
		BrMatrix34Inverse(&self->state.current->matrix.model_to_view, &old);

	return BRE_OK;
}




br_error br_renderer_gl_modelMulX(struct br_renderer *self, br_matrix34_x *m)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_modelPopPushMulX(struct br_renderer *self, br_matrix34_x *m)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}








br_error br_renderer_gl_boundsTestX(struct br_renderer *self, br_token *r, br_bounds3_x *bounds_in)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_boundsTestF(struct br_renderer *_self, br_token *r, br_bounds3_f *bounds)
{
	//log_trace("GLREND", __FUNCTION__);
	br_renderer_gl *self = reinterpret_cast<br_renderer_gl*>(_self);

	br_token GLOnScreenCheck(br_renderer_gl *self, const br_matrix4 *model_to_screen, const br_bounds3_f *bounds);

	// FIXME: Should probably cache this.
	br_matrix4 m2s;
	BrMatrix4Mul34(&m2s, &self->state.current->matrix.model_to_view, &self->state.current->matrix.view_to_screen);
	*r = GLOnScreenCheck(self, &m2s, bounds);
	return BRE_OK;
}


br_error br_renderer_gl_coverageTestF(struct br_renderer *self, br_float *r, br_bounds3_f *bounds)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_coverageTestX(struct br_renderer *self, br_fixed_ls *r, br_bounds3_x *bounds)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}


br_error br_renderer_gl_viewDistanceF(struct br_renderer *self, br_float *r)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}

br_error br_renderer_gl_viewDistanceX(struct br_renderer *self, br_fixed_ls *r)
{
	log_trace("GLREND", __FUNCTION__);
	return BRE_FAIL;
}




/*
* Default dispatch table for renderer
*/
static const struct br_renderer_dispatch rendererDispatch = {
	br_renderer_gl_sceneBegin,
	br_renderer_gl_sceneEnd,
	nullptr,
	nullptr,
	br_renderer_gl_free,
	br_renderer_gl_identifier,
	br_renderer_gl_type,
	br_renderer_gl_isType,
	br_renderer_gl_device,
	br_renderer_gl_space,
	br_renderer_gl_templateQuery,
	BR_CMETHOD(br_object, query),
	BR_CMETHOD(br_object, queryBuffer),
	BR_CMETHOD(br_object, queryMany),
	BR_CMETHOD(br_object, queryManySize),
	BR_CMETHOD(br_object, queryAll),
	BR_CMETHOD(br_object, queryAllSize),
	br_renderer_gl_listQuery,
	BR_CMETHOD(br_object_container,             tokensMatchBegin),
	BR_CMETHOD(br_object_container,             tokensMatch),
	BR_CMETHOD(br_object_container,             tokensMatchEnd),
	BR_CMETHOD(br_object_container,             addFront),
	BR_CMETHOD(br_object_container,             removeFront),
	BR_CMETHOD(br_object_container,             remove),
	BR_CMETHOD(br_object_container,             find),
	BR_CMETHOD(br_object_container,             findMany),
	BR_CMETHOD(br_object_container,             count),

	br_renderer_gl_validDestination,
	br_renderer_gl_stateStoredNew,
	br_renderer_gl_stateStoredAvail,
	br_renderer_gl_bufferStoredNew,
	br_renderer_gl_bufferStoredAvail,
	br_renderer_gl_partSet,
	br_renderer_gl_partSetMany,
	br_renderer_gl_partQuery,
	br_renderer_gl_partQueryBuffer,
	br_renderer_gl_partQueryMany,
	br_renderer_gl_partQueryManySize,
	br_renderer_gl_partQueryAll,
	br_renderer_gl_partQueryAllSize,
	br_renderer_gl_partIndexQuery,
	br_renderer_gl_modelMulF,
	br_renderer_gl_modelMulX,
	br_renderer_gl_modelPopPushMulF,
	br_renderer_gl_modelPopPushMulX,
	br_renderer_gl_modelInvert,
	br_renderer_gl_statePush,
	br_renderer_gl_statePop,
	br_renderer_gl_stateSave,
	br_renderer_gl_stateRestore,
	br_renderer_gl_stateMask,
	br_renderer_gl_stateDefault,
	br_renderer_gl_boundsTestF,
	br_renderer_gl_boundsTestX,
	br_renderer_gl_coverageTestF,
	br_renderer_gl_coverageTestX,
	br_renderer_gl_viewDistanceF,
	br_renderer_gl_viewDistanceX,
	br_renderer_gl_commandModeSet,
	br_renderer_gl_commandModeQuery,
	br_renderer_gl_commandModeDefault,
	br_renderer_gl_commandModePush,
	br_renderer_gl_commandModePop,
	br_renderer_gl_flush,
	br_renderer_gl_synchronise,
	br_renderer_gl_testRender,
	br_renderer_gl_partQueryCapability,
	br_renderer_gl_stateQueryPerformance,
};


br_renderer_gl *RendererGLAllocate(br_device_gl *device, br_renderer_facility_gl *facility, br_device_pixelmap_gl *dest)
{
	/* Check that destination is valid */
	if(dest == NULL || dest->dispatch->_device((br_object*)dest) != (br_device*)device)
		return NULL;

	br_renderer_gl *self = (br_renderer_gl*)BrResAllocate(facility, sizeof(br_renderer_gl), BR_MEMORY_OBJECT);
	if(self == nullptr)
		return nullptr;

	new (self) br_renderer_gl();

	self->dispatch = (br_renderer_dispatch*)&rendererDispatch;
	self->identifier = facility->identifier;
	self->device = device;
	self->renderer_facility = facility;

	ObjectContainerAddFront(facility, (br_object *)self);

	if(self->device->templates.rendererTemplate == nullptr)
		self->device->templates.rendererTemplate = BrTVTemplateAllocate(self->device, rendererTemplateEntries, BR_ASIZE(rendererTemplateEntries));

	GLSTATE_Init(&self->state, self->device);
	/*
	* State starts out as default
	*/
	RendererStateDefault(self, (br_uint_32)BR_STATE_ALL);

	self->has_begun = false;
	return self;
}