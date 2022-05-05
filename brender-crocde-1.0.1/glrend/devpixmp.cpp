#include "gl_core_3_2.h"
#include <log.h>
#include "drv_ip.hpp"
#include "pm.h"
#include "brassert.h"

static void _patchQuad(br_device_pixelmap_gl *self, float dx0, float dy0, float dx1, float dy1, float sx0, float sy0, float sx1, float sy1)
{
	/* Bottom-left */
	self->asBack.quad.tris[0].x = dx0;
	self->asBack.quad.tris[0].y = dy0;
	self->asBack.quad.tris[0].u = sx0;
	self->asBack.quad.tris[0].v = sy0;

	/* Top-left */
	self->asBack.quad.tris[1].x = dx0;
	self->asBack.quad.tris[1].y = dy1;
	self->asBack.quad.tris[1].u = sx0;
	self->asBack.quad.tris[1].v = sy1;

	/* Top-right */
	self->asBack.quad.tris[2].x = dx1;
	self->asBack.quad.tris[2].y = dy1;
	self->asBack.quad.tris[2].u = sx1;
	self->asBack.quad.tris[2].v = sy1;

	/* Bottom right */
	self->asBack.quad.tris[3].x = dx1;
	self->asBack.quad.tris[3].y = dy0;
	self->asBack.quad.tris[3].u = sx1;
	self->asBack.quad.tris[3].v = sy0;

	/* Upload the data */
	glBindBuffer(GL_ARRAY_BUFFER, self->asBack.quad.buffers[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(self->asBack.quad.tris), self->asBack.quad.tris);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*
** Patch the X/Y and U/V coordinates.
** X/Y = rect of destination
** U/V = rect of source
** Rects are in OpenGL coordianges -- (0,0) at bottom-left
**/
static void patchQuad(br_device_pixelmap_gl *self, br_rectangle *dr, br_pixelmap *src, br_rectangle *sr)
{
	float x0, y0, x1, y1;
	VIDEOI_BrRectToUVs(self, dr, x0, y0, x1, y1);

	float sx0, sy0, sx1, sy1;
	VIDEOI_BrRectToUVs(src, sr, sx0, sy0, sx1, sy1);

	_patchQuad(self, x0, y0, x1, y1, sx0, sy0, sx1, sy1);
}

static constexpr GLsizei s_QuadStride = sizeof(br_device_pixelmap_gl_tri);

/* Quad GL_TRIANGES indices. */
static GLuint s_QuadIndices[] = {0, 1, 2, 2, 3, 0};

/* Default quad data. Format is XYZRGBUV. */
static br_device_pixelmap_gl_tri s_QuadData[4] =
{
	{
		/* Bottom-left */
		-1.0f, -1.0f, 0.5f,
		1.0f, 1.0f, 1.0f,
		0.0f, 0.0f,
	},
	{
		/* Top-left */
		-1.0f, 1.0f, 0.5f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
	},
	{
		/* Top-right */
		1.0f, 1.0f, 0.5f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,
	},
	{
		/* Bottom-right */
		1.0f, -1.0f, 0.5f,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.0f,
	}
};

static glm::mat4 s_QuadOrtho = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

static void createQuad(br_device_pixelmap_gl *self)
{
	/* Create the crap needed for the operations quad. */
	memcpy(self->asBack.quad.tris, s_QuadData, sizeof(self->asBack.quad.tris));
	static_assert(sizeof(self->asBack.quad.tris) == sizeof(s_QuadData), "Quad size mismatch");


	{ /* Build the buffers */
		glGenBuffers(2, self->asBack.quad.buffers);

		glBindBuffer(GL_ARRAY_BUFFER, self->asBack.quad.buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(self->asBack.quad.tris), self->asBack.quad.tris, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->asBack.quad.buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_QuadIndices), s_QuadIndices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	{ /* Default Quad VAO */
		glGenVertexArrays(1, &self->asBack.quad.defaultVao);
		glBindVertexArray(self->asBack.quad.defaultVao);

		glBindBuffer(GL_ARRAY_BUFFER, self->asBack.quad.buffers[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->asBack.quad.buffers[1]);

		HVIDEO hVideo = &self->device->video;

		/* Positions */
		glEnableVertexAttribArray(hVideo->defaultProgram.aPosition);
		glVertexAttribPointer(hVideo->defaultProgram.aPosition, 3, GL_FLOAT, GL_FALSE, s_QuadStride, NULL);

		/* Colours */
		glEnableVertexAttribArray(hVideo->defaultProgram.aColour);
		glVertexAttribPointer(hVideo->defaultProgram.aColour, 3, GL_FLOAT, GL_FALSE, s_QuadStride, (GLvoid*)(sizeof(GLfloat) * 3));

		/* UVs */
		glEnableVertexAttribArray(hVideo->defaultProgram.aUV);
		glVertexAttribPointer(hVideo->defaultProgram.aUV, 2, GL_FLOAT, GL_FALSE, s_QuadStride, (GLvoid*)(sizeof(GLfloat) * 6));
	}

	{ /* Text Quad VAO */
		glGenVertexArrays(1, &self->asBack.quad.textVao);
		glBindVertexArray(self->asBack.quad.textVao);

		glBindBuffer(GL_ARRAY_BUFFER, self->asBack.quad.buffers[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->asBack.quad.buffers[1]);

		HVIDEO hVideo = &self->device->video;

		/* Positions */
		glEnableVertexAttribArray(hVideo->textProgram.aPosition);
		glVertexAttribPointer(hVideo->textProgram.aPosition, 3, GL_FLOAT, GL_FALSE, s_QuadStride, NULL);

		/* UVs */
		glEnableVertexAttribArray(hVideo->textProgram.aUV);
		glVertexAttribPointer(hVideo->textProgram.aUV, 2, GL_FLOAT, GL_FALSE, s_QuadStride, (GLvoid*)(sizeof(GLfloat) * 6));
	}


	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void checkGlError(const char *fn)
{
	GLenum err = glGetError();
	if(err != GL_NO_ERROR)
	{
		log_error("VIDEO", "OpenGL Error %d from %s", err, fn);
		__debugbreak();
	}
}

static br_error initPixelmapGL(br_device_pixelmap_gl *self)
{
	VIDEOI_BrPixelmapGetTypeDetails(self->pm_type, &self->glInternalFormat, &self->glFormat, &self->glType, &self->glElemBytes);

	/* Screen needs nothing. */
	if(self->useType == BRT_NONE)
		return BRE_OK;

	while(glGetError() != GL_NO_ERROR);

	/* Framebuffer? Create the FBO and texture. */
	if(self->useType == BRT_OFFSCREEN)
	{
		glGenFramebuffers(1, &self->asBack.glFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);

		glGenRenderbuffers(1, &self->asBack.glTex);
		glBindRenderbuffer(GL_RENDERBUFFER, self->asBack.glTex);

		if(self->device->msaaSamples)
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, self->device->msaaSamples, GL_RGBA8, self->pm_width, self->pm_height);
			checkGlError("glRenderbufferStorageMultisample");
		}
		else
		{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, self->pm_width, self->pm_height);
			checkGlError("glRenderbufferStorage");
		}
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, self->asBack.glTex);

		GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, DrawBuffers);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return BRE_FAIL;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		createQuad(self);
	}
	/* Depth buffer? Create a GL_DEPTH_COMPONENT renderbuffer and attach it to our parent. */
	else if(self->useType == BRT_DEPTH)
	{
		glGenRenderbuffers(1, &self->asDepth.glDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, self->asDepth.glDepth);

		if(self->device->msaaSamples)
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, self->device->msaaSamples, GL_DEPTH_COMPONENT24, self->pm_width, self->pm_height);
			checkGlError("glRenderbufferStorageMultisample");
		}
		else
		{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, self->pm_width, self->pm_height);
			checkGlError("glRenderbufferStorage");
		}
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, self->asDepth.backbuffer->asBack.glFbo);
		checkGlError("glBindFramebuffer");
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, self->asDepth.glDepth);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return BRE_FAIL;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	return BRE_OK;
}

void br_device_pixelmap_gl_free(br_object *_self)
{
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);

	if(self->useType == BRT_DEPTH)
	{
		// FIXME: We should be destroyed before our parent.
		// FIXME: If we haven't, should I bind the parent and detach?
		glDeleteRenderbuffers(1, &self->asDepth.glDepth);
	}
	else if(self->useType == BRT_OFFSCREEN)
	{
		glDeleteFramebuffers(1, &self->asBack.glFbo);
		glDeleteRenderbuffers(1, &self->asBack.glTex);

		/* Cleanup the quad */
		glDeleteBuffers(2, self->asBack.quad.buffers);
		glDeleteVertexArrays(1, &self->asBack.quad.defaultVao);
		glDeleteVertexArrays(1, &self->asBack.quad.textVao);
	}

	ObjectContainerRemove(self->output_facility, _self);
	BrResFreeNoCallback(self);
}

const char *br_device_pixelmap_gl_identifier(br_object *self)
{
	br_device_pixelmap_gl *devpm = reinterpret_cast<br_device_pixelmap_gl*>(self);
	return devpm->pm_identifier;
}


br_token br_device_pixelmap_gl_type(br_object *self)
{
	return BRT_DEVICE_PIXELMAP;
}

br_boolean br_device_pixelmap_gl_isType(br_object *self, br_token t)
{
	return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

br_device *br_device_pixelmap_gl_device(br_object *self)
{
	br_device_pixelmap_gl *devpm = reinterpret_cast<br_device_pixelmap_gl*>(self);
	return reinterpret_cast<br_device*>(devpm->device);
}

br_int_32 br_device_pixelmap_gl_space(br_object *self)
{
	return sizeof(br_device_pixelmap);
}


/*
* Device pixelmap info. template
*/
#define F(f)	offsetof(struct br_device_pixelmap_gl, f)

static struct br_tv_template_entry devicePixelmapTemplateEntries[] = {

	{BRT(WIDTH_I32),		F(pm_width),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_I32_U16,},
	{BRT(HEIGHT_I32),		F(pm_height),		BRTV_QUERY | BRTV_ALL,	BRTV_CONV_I32_U16,},
	{BRT(PIXEL_TYPE_U8),	F(pm_type),			BRTV_QUERY | BRTV_ALL,	BRTV_CONV_I32_U8,},
	{BRT(OUTPUT_FACILITY_O),F(output_facility),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(FACILITY_O),		F(output_facility),	BRTV_QUERY,				BRTV_CONV_COPY,},
	{BRT(IDENTIFIER_CSTR),	F(pm_identifier),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,},
	{BRT(MSAA_SAMPLES_I32),	F(msaa_samples),	BRTV_QUERY | BRTV_ALL,	BRTV_CONV_COPY,}
};
#undef F

static struct br_tv_template devicePixelmapTemplate = {
	BR_ASIZE(devicePixelmapTemplateEntries),
	devicePixelmapTemplateEntries
};


struct br_tv_template * br_device_pixelmap_gl_templateQuery(br_object *_self)
{
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);
	return self->device->templates.devicePixelmapTemplate;
}


br_error br_device_pixelmap_gl_validSource(br_device_pixelmap *self, br_boolean *bp, br_object *h)
{
	/* ... */
	return BRE_OK;
}


br_error br_device_pixelmap_gl_resize(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
	return BRE_FAIL;
}

br_error br_device_pixelmap_gl_match(br_device_pixelmap *_self, br_device_pixelmap **_newpm, br_token_value *tv)
{
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);

	/* We're expecting 3 tokens, BRT_WIDTH_I32, BRT_HEIGHT_I32, and BRT_USE_T. */
	br_int_32 wantedWidth = -1;
	br_int_32 wantedHeight = -1;
	br_int_32 pixelBits = -1;
	br_token useType = BRT_NONE;

	/* Get the token values, ignoring any unknown. */
	for(br_token_value *_tv = tv; _tv->t != BR_NULL_TOKEN; ++_tv)
	{
		if(_tv->t == BRT_WIDTH_I32)
			wantedWidth = _tv->v.i32;
		else if(_tv->t == BRT_HEIGHT_I32)
			wantedHeight = _tv->v.i32;
		else if(_tv->t == BRT_PIXEL_BITS_I32)
			pixelBits = _tv->v.i32;
		else if(_tv->t == BRT_USE_T)
			useType = _tv->v.t;
	}

	if(useType == BRT_NO_RENDER)
		useType = BRT_OFFSCREEN;

	/* Want a back buffer (FBO)? */
	if(useType == BRT_OFFSCREEN)
	{
		/* Must be matching from the screen. */
		//if(self->useType != BRT_NONE)
		//	return BRE_FAIL;

		/* Can't >1 backbuffer */
		//if(self->backbuffer != nullptr)
		//	return BRE_FAIL;

		///* Only same-size buffers for now. */
		//if(wantedWidth != self->pm_width || wantedHeight != self->pm_height)
		//	return BRE_FAIL;
	}
	/* Want a depth buffer? */
	else if(useType == BRT_DEPTH)
	{
		if(self->useType != BRT_OFFSCREEN)
			return BRE_FAIL;

		/* Can't >1 depth buffer */
		if(self->asBack.depthbuffer != nullptr)
			return BRE_FAIL;

		ASSERT(pixelBits == 16);
		//if(pixelBits != 16)
		//	return BRE_FAIL;


		wantedWidth = self->pm_width;
		wantedHeight = self->pm_height;
	}
	else
	{
		return BRE_FAIL;
	}

	br_device_pixelmap_gl *newpm = (br_device_pixelmap_gl*)BrResAllocate(self->output_facility, sizeof(br_device_pixelmap_gl), BR_MEMORY_OBJECT);
	if(self == nullptr)
		return BRE_FAIL;

	newpm->dispatch = self->dispatch;
	newpm->device = self->device;
	newpm->output_facility = self->output_facility;
	newpm->useType = useType;

	newpm->pm_type = self->pm_type;
	newpm->pm_width = wantedWidth;
	newpm->pm_height = wantedHeight;
	newpm->pm_origin_x = self->pm_origin_x;
	newpm->pm_origin_y = self->pm_origin_y;
	newpm->pm_base_x = self->pm_base_x;
	newpm->pm_base_y = self->pm_base_y;
	newpm->pm_flags = BR_PMF_NO_ACCESS;

	ObjectContainerAddFront(self->output_facility, (br_object *)self);

	*_newpm = reinterpret_cast<br_device_pixelmap*>(newpm);

	char nameBuffer[128];
	if(useType == BRT_OFFSCREEN)
	{
		snprintf(nameBuffer, sizeof(nameBuffer), "%s (Backbuffer)", self->pm_identifier);
		nameBuffer[127] = 0;
		newpm->asBack.depthbuffer = nullptr;
	}
	else if(useType == BRT_DEPTH)
	{
		snprintf(nameBuffer, sizeof(nameBuffer), "%s (Depth)", self->pm_identifier);
		nameBuffer[127] = 0;
		self->asBack.depthbuffer = newpm;
		newpm->asDepth.backbuffer = self;
	}

	newpm->pm_identifier = BrResStrDup(self->output_facility, nameBuffer);

	return initPixelmapGL(newpm);
}


br_error br_device_pixelmap_gl_allocateSub(br_device_pixelmap *self, br_device_pixelmap **newpm, br_rectangle *rect)
{
	return BRE_FAIL;
}


static void fuckfuck(br_device_pixelmap_gl *self, br_rectangle *dr, br_pixelmap *src, br_rectangle *sr)
{
	ASSERT(self->useType == BRT_OFFSCREEN);

	/* Convert the rects to OpenGL-coordinates */
	VIDEOI_BrRectToGL(src, sr);
	VIDEOI_BrRectToGL(self, dr);

	/* Patch the quad */
	patchQuad(self, dr, src, sr);

	GLuint srcTex = VIDEO_BrPixelmapToGLTexture(src);

	HVIDEO hVideo = &self->device->video;

	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);

	glViewport(0, 0, self->pm_width, self->pm_height);

	/* Render it */
	glUseProgram(hVideo->defaultProgram.program);
	glUniformMatrix4fv(hVideo->defaultProgram.uMVP, 1, GL_FALSE, glm::value_ptr(s_QuadOrtho));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, srcTex);
	glUniform1i(hVideo->defaultProgram.uSampler, 0);
	glUniform1f(hVideo->defaultProgram.uVerticalFlip, 1);

	glBindVertexArray(self->asBack.quad.defaultVao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);
	glDeleteTextures(1, &srcTex);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

br_error br_device_pixelmap_gl_rectangleStretchCopy(br_device_pixelmap *_self, br_rectangle *d, br_device_pixelmap *_src, br_rectangle *s)
{
	/* Device->Device non-addressable stretch copy */
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);
	br_device_pixelmap_gl *src = reinterpret_cast<br_device_pixelmap_gl*>(_src);

	GLbitfield bits;
	GLuint srcFbo, dstFbo;
	if(self->useType == BRT_NONE)
	{
		if(src->useType != BRT_OFFSCREEN)
			return BRE_FAIL;
		dstFbo = 0;
		srcFbo = src->asBack.glFbo;
		bits = GL_COLOR_BUFFER_BIT;
	}
	else if(self->useType == BRT_OFFSCREEN)
	{
		if(src->useType != BRT_OFFSCREEN)
			return BRE_FAIL;
		dstFbo = self->asBack.glFbo;
		srcFbo = src->asBack.glFbo;
		bits = GL_COLOR_BUFFER_BIT;
	}
	else if(self->useType == BRT_DEPTH)
	{
		if(src->useType != BRT_DEPTH)
			return BRE_FAIL;

		dstFbo = self->asDepth.backbuffer->asBack.glFbo;
		srcFbo = src->asDepth.backbuffer->asBack.glFbo;
		bits = GL_DEPTH_BUFFER_BIT;
	}
	else
	{
		return BRE_FAIL;
	}

	/* Ignore self-blit. */
	if(self == src)
		return BRE_OK;

	VIDEOI_BrRectToGL(self, d);
	VIDEOI_BrRectToGL(src, s);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);

	glBlitFramebuffer(
		s->x, s->y, s->x + s->w, s->y + s->h,
		d->x, d->y, d->x + d->w, d->y + d->h,
		bits,
		GL_NEAREST
	);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return BRE_OK;
}

br_error br_device_pixelmap_gl_rectangleCopy(br_device_pixelmap *_self, br_point *p, br_device_pixelmap *_src, br_rectangle *sr)
{
	/* Device->Device non-addressable same-size copy. */
	br_rectangle r;
	r.x = -_self->pm_origin_x;
	r.y = -_self->pm_origin_y;
	r.w = _self->pm_width;
	r.h = _self->pm_height;
	return br_device_pixelmap_gl_rectangleStretchCopy(_self, &r, _src, sr);
}

br_error br_device_pixelmap_gl_rectangleFill(br_device_pixelmap *_self, br_rectangle *rect, br_uint_32 colour)
{
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);

	ASSERT(self->useType == BRT_OFFSCREEN || self->useType == BRT_DEPTH);

	float a = ((colour & 0xFF000000) >> 24) / 255.0f;
	float r = ((colour & 0x00FF0000) >> 16) / 255.0f;
	float g = ((colour & 0x0000FF00) >> 8) / 255.0f;
	float b = ((colour & 0x000000FF) >> 0) / 255.0f;

	VIDEOI_BrRectToGL(self, rect);

	GLuint fbo;
	GLbitfield mask;
	if(self->useType == BRT_OFFSCREEN)
	{
		fbo = self->asBack.glFbo;
		mask = GL_COLOR_BUFFER_BIT;
		glClearColor(r, g, b, a);
	}
	else if(self->useType == BRT_DEPTH)
	{
		ASSERT(colour == 0xFFFFFFFF);
		fbo = self->asBack.depthbuffer->asBack.glFbo;
		mask = GL_DEPTH_BUFFER_BIT;
		glClearDepth(1.0f);
	}
	else
	{
		fbo = 0;
		mask = GL_COLOR_BUFFER_BIT;
		glClearColor(r, g, b, a);
	}


	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, self->pm_width, self->pm_height);

	glEnable(GL_SCISSOR_TEST);
	glScissor(rect->x, rect->y, rect->w, rect->h);

	glClear(mask);

	glDisable(GL_SCISSOR_TEST);

	return BRE_OK;
}


br_error br_device_pixelmap_gl_rectangleStretchCopyTo(br_device_pixelmap *_self, br_rectangle *d, br_device_pixelmap *src, br_rectangle *s)
{
	/* Pixelmap->Device, addressable stretch copy. */
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);
	fuckfuck(self, d, (br_pixelmap*)src, s);
	return BRE_OK;
}

br_error br_device_pixelmap_gl_rectangleCopyTo(br_device_pixelmap *_self, br_point *p, br_device_pixelmap *src, br_rectangle *sr)
{
	/* Pixelmap->Device, addressable same-size copy. */

	/* FIXME: Still have no idea what p is for. */

	/* Do a stretch copy with no stretching. */
	br_rectangle r;
	r.x = -_self->pm_origin_x;
	r.y = -_self->pm_origin_y;
	r.w = _self->pm_width;
	r.h = _self->pm_height;
	return br_device_pixelmap_gl_rectangleStretchCopyTo(_self, &r, src, sr);
}

br_error br_device_pixelmap_gl_rectangleStretchCopyFrom(br_device_pixelmap *_self, br_rectangle *d, br_device_pixelmap *dest, br_rectangle *s)
{
	/* Device->Pixelmap, addressable stretch copy. */
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);
	return BRE_FAIL;
}


br_error br_device_pixelmap_gl_rectangleCopyFrom(br_device_pixelmap *_self, br_point *p, br_device_pixelmap *dest, br_rectangle *dr)
{
	/* Device->Pixelmap, addressable same-size copy. */
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);

	/* Do a stretch copy with no stretching. */
	//br_rectangle r;
	//r.x = -_self->pm_origin_x;
	//r.y = -_self->pm_origin_y;
	//r.w = _self->pm_width;
	//r.h = _self->pm_height;
	////return br_device_pixelmap_gl_rectangleStretchCopyFrom(_self, &r, dest, dr);

	/* Need contig pixels. */
	if(!(dest->pm_flags & BR_PMF_LINEAR))
		return BRE_FAIL;

	br_rectangle glDr = *dr;
	VIDEOI_BrRectToGL(reinterpret_cast<br_pixelmap*>(dest), &glDr);

	/* Only do fullscreen copies */
	if(glDr.x != 0 && glDr.y != 0 && glDr.w != self->pm_width && glDr.h != self->pm_height)
		return BRE_FAIL;

	GLenum internalFormat, format, type;
	GLsizei elemBytes;
	VIDEOI_BrPixelmapGetTypeDetails(dest->pm_type, &internalFormat, &format, &type, &elemBytes);

	glReadPixels(0, 0, self->pm_width, self->pm_height, format, type, dest->pm_pixels);

	void *rowTemp = BrScratchAllocate(dest->pm_row_bytes);

	/* Flip it */
	for(br_uint_16 j = 0; j < dest->pm_height / 2; ++j)
	{
		void *top = reinterpret_cast<char*>(dest->pm_pixels) + (j * dest->pm_row_bytes);
		void *bot = reinterpret_cast<char*>(dest->pm_pixels) + ((dest->pm_height - j - 1) * dest->pm_row_bytes);
		memcpy(rowTemp, top, dest->pm_row_bytes);
		memcpy(top, bot, dest->pm_row_bytes);
		memcpy(bot, rowTemp, dest->pm_row_bytes);
	}

	/* TODO: If on little-endian systems, swap the byte order. */
	BrScratchFree(rowTemp);
	return BRE_OK;
}

br_error br_device_pixelmap_lock_gl_pixelSet(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
	/* Sorry, no. */
	return BRE_FAIL;
}

br_error br_device_pixelmap_lock_gl_line(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
	/* Sorry, no. */
	return BRE_FAIL;
}

br_error br_device_pixelmap_lock_gl_copyBits(br_device_pixelmap *_self, br_point *point, br_uint_8 *src, br_uint_16 s_stride, br_rectangle *bit_rect, br_uint_32 colour)
{
	//br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);
	//br_device_pixelmap *fuck = (br_device_pixelmap*)BrPixelmapAllocate(self->pm_type, self->pm_width, self->pm_height, nullptr, BR_PMAF_NORMAL);
	//
	//glBindFramebuffer(GL_FRAMEBUFFER, self->glFbo);
	//glReadPixels(0, 0, self->pm_width, self->pm_height, self->glFormat, self->glType, fuck->pm_pixels);
	//
	//fuck->dispatch->_copyBits(fuck, point, src, s_stride, bit_rect, colour);
	//
	//br_rectangle r;
	//r.x = -self->pm_origin_x;
	//r.y = -self->pm_origin_y;
	//r.w = self->pm_width;
	//r.h = self->pm_height;
	//br_device_pixelmap_gl_rectangleCopy(_self, point, fuck, &r);
	//
	//
	//BrPixelmapFree((br_pixelmap*)fuck);
	/* Sorry, no. */
	return BRE_FAIL;
}

br_error br_device_pixelmap_gl_text(br_device_pixelmap *_self, br_point *point, br_font *font, char *text, br_uint_32 colour)
{
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);

	/* Quit if off top, bottom or right screen */
	br_int_32 x = point->x + self->pm_origin_x;
	br_int_32 y = point->y + self->pm_origin_y;

	if(y <= -font->glyph_y || y >= self->pm_height || x >= self->pm_width)
		return BRE_OK;

	/* Ensure we're a valid font. */
	HVIDEO_FONT hFont;
	if(font == BrFontFixed3x5)
		hFont = &self->device->video.fonts.fixed3x5;
	else if(font == BrFontProp4x6)
		hFont = &self->device->video.fonts.prop4x6;
	else if(font == BrFontProp7x9)
		hFont = &self->device->video.fonts.prop7x9;
	else
		return BRE_FAIL;

	/* Set up the render state. The font UVs match the texture, so no need to flip here. */
	HVIDEO hVideo = &self->device->video;

	glBindFramebuffer(GL_FRAMEBUFFER, self->asBack.glFbo);
	glViewport(0, 0, self->pm_width, self->pm_height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glUseProgram(hVideo->textProgram.program);

	glm::vec3 col(
		BR_RED(colour) / 255.0f,
		BR_GRN(colour) / 255.0f,
		BR_BLU(colour) / 255.0f
	);
	glUniformMatrix4fv(hVideo->textProgram.uMVP, 1, GL_FALSE, glm::value_ptr(s_QuadOrtho));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hFont->glTex);
	glUniform1i(hVideo->textProgram.uSampler, 0);
	glUniform3fv(hVideo->textProgram.uColour, 1, glm::value_ptr(col));

	br_rectangle r;
	r.x = point->x;
	r.y = point->y;
	r.h = font->glyph_y;
	r.w = 0;

	for(; *text && r.w <= self->pm_width; ++text)
	{
		br_uint_8 glyph = (br_uint_8)*text;
		br_uint_16 width = (font->flags & BR_FONTF_PROPORTIONAL) ? font->width[glyph] : font->glyph_x;
		r.w = width;

		br_rectangle dr = r;
		VIDEOI_BrRectToGL(self, &dr);

		float x0, y0, x1, y1;
		VIDEOI_BrRectToUVs(self, &dr, x0, y0, x1, y1);

		_patchQuad(self,
				   x0, y0, x1, y1,
				   hFont->glyph[glyph].x0,
				   hFont->glyph[glyph].y0,
				   hFont->glyph[glyph].x1,
				   hFont->glyph[glyph].y1
		);

		glBindVertexArray(self->asBack.quad.textVao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		r.x += width + 1;
		r.w += width;
	}

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
	return BRE_OK;
}


br_error br_device_pixelmap_lock_gl_pixelQuery(br_device_pixelmap *self, br_uint_32 *pcolour, br_point *p)
{
	/* Sorry, no. */
	return BRE_FAIL;
}


/*
* Routines for unsupported features (could go into library)
*/
br_error br_device_pixelmap_fail_gl_rowSize(br_device_pixelmap *self, br_size_t *sizep)
{
	return BRE_FAIL;
}

br_error br_device_pixelmap_fail_gl_rowQuery(br_device_pixelmap *self, void *buffer, br_size_t buffer_size, br_uint_32 row)
{
	return BRE_FAIL;
}

br_error br_device_pixelmap_fail_gl_rowSet(br_device_pixelmap *self, void *buffer, br_size_t buffer_size, br_uint_32 row)
{
	return BRE_FAIL;
}

br_error br_device_pixelmap_fail_gl_pixelAddressQuery(br_device_pixelmap *self, void **pptr, br_uint_32 *pqual, br_point *p)
{
	return BRE_FAIL;
}

br_error br_device_pixelmap_fail_gl_pixelAddressSet(br_device_pixelmap *self, void *ptr, br_uint_32 *qual)
{
	return BRE_FAIL;
}




br_error br_device_pixelmap_gl_originSet(br_device_pixelmap *self, br_point *p)
{
	self->pm_origin_x = (br_uint_16)p->x;
	self->pm_origin_y = (br_uint_16)p->y;

	return BRE_OK;
}


br_error br_device_pixelmap_gl_flush(br_device_pixelmap *self)
{
	return BRE_FAIL;
}

br_error br_device_pixelmap_gl_synchronise(br_device_pixelmap *self, br_token sync_type, br_boolean block)
{
	return BRE_FAIL;
}

br_error br_device_pixelmap_gl_directLock(br_device_pixelmap *_self, br_boolean block)
{
	/* "Lock" the device, making the pixel data addressable. */
	return BRE_FAIL;
}

br_error br_device_pixelmap_gl_directUnlock(br_device_pixelmap *self)
{
	/* "Unlock" the device, making the pixel data non-addressable. */
	return BRE_FAIL;
}


extern "C" void _GLREND_PreSwapHook(GLuint fbo);

br_error br_device_pixelmap_gl_doubleBuffer(br_device_pixelmap *_self, br_device_pixelmap *_src)
{
	br_device_pixelmap_gl *self = reinterpret_cast<br_device_pixelmap_gl*>(_self);
	br_device_pixelmap_gl *src = reinterpret_cast<br_device_pixelmap_gl*>(_src);

	ASSERT(self->useType == BRT_NONE && src->useType == BRT_OFFSCREEN);

	//br_pixelmap *fuck = self->device->video.fuck;
	//BrPixelmapRectangleStretchCopy((br_pixelmap*)src, -src->pm_origin_x, -src->pm_origin_y, fuck->width, fuck->height, (br_pixelmap*)fuck, -fuck->origin_x, -fuck->origin_y, fuck->width, fuck->height);

	/* Blit the FBO to the screen and swap. */
	BrPixelmapCopy((br_pixelmap*)_self, (br_pixelmap*)_src);

	/* Call our hook */
	_GLREND_PreSwapHook(src->asBack.glFbo);
	while(glGetError() != GL_NO_ERROR);

	glfwSwapBuffers(self->device->glfwWindow);
	return BRE_OK;
}



static const struct br_device_pixelmap_dispatch devicePixelmapDispatch = {
	NULL,
	NULL,
	NULL,
	NULL,
	br_device_pixelmap_gl_free,
	br_device_pixelmap_gl_identifier,
	br_device_pixelmap_gl_type,
	br_device_pixelmap_gl_isType,
	br_device_pixelmap_gl_device,
	br_device_pixelmap_gl_space,

	br_device_pixelmap_gl_templateQuery,
	BR_CMETHOD(br_object,              query),
	BR_CMETHOD(br_object,              queryBuffer),
	BR_CMETHOD(br_object,              queryMany),
	BR_CMETHOD(br_object,              queryManySize),
	BR_CMETHOD(br_object,              queryAll),
	BR_CMETHOD(br_object,              queryAllSize),

	br_device_pixelmap_gl_validSource,
	br_device_pixelmap_gl_resize,
	br_device_pixelmap_gl_match,
	br_device_pixelmap_gl_allocateSub,

	BR_CMETHOD(br_device_pixelmap_gen, copy),
	BR_CMETHOD(br_device_pixelmap_gen, copyTo),
	BR_CMETHOD(br_device_pixelmap_gen, copyFrom),
	BR_CMETHOD(br_device_pixelmap_gen, fill),
	br_device_pixelmap_gl_doubleBuffer,

	BR_CMETHOD(br_device_pixelmap_gen, copyDirty),
	BR_CMETHOD(br_device_pixelmap_gen, copyToDirty),
	BR_CMETHOD(br_device_pixelmap_gen, copyFromDirty),
	BR_CMETHOD(br_device_pixelmap_gen, fillDirty),
	BR_CMETHOD(br_device_pixelmap_gen, doubleBufferDirty),

	BR_CMETHOD(br_device_pixelmap_gen, rectangle),
	BR_CMETHOD(br_device_pixelmap_gen, rectangle2),
	br_device_pixelmap_gl_rectangleCopy,
	br_device_pixelmap_gl_rectangleCopyTo,
	br_device_pixelmap_gl_rectangleCopyFrom,
	br_device_pixelmap_gl_rectangleStretchCopy,
	br_device_pixelmap_gl_rectangleStretchCopyTo,
	br_device_pixelmap_gl_rectangleStretchCopyFrom,
	br_device_pixelmap_gl_rectangleFill,
	br_device_pixelmap_lock_gl_pixelSet,
	br_device_pixelmap_lock_gl_line,
	br_device_pixelmap_lock_gl_copyBits,

	br_device_pixelmap_gl_text,
	BR_CMETHOD(br_device_pixelmap_gen, textBounds),

	br_device_pixelmap_fail_gl_rowSize,
	br_device_pixelmap_fail_gl_rowQuery,
	br_device_pixelmap_fail_gl_rowSet,

	br_device_pixelmap_lock_gl_pixelQuery,
	br_device_pixelmap_fail_gl_pixelAddressQuery,

	br_device_pixelmap_fail_gl_pixelAddressSet,
	br_device_pixelmap_gl_originSet,

	br_device_pixelmap_gl_flush,
	br_device_pixelmap_gl_synchronise,
	br_device_pixelmap_gl_directLock,
	br_device_pixelmap_gl_directUnlock,
};

br_device_pixelmap_gl *DevicePixelmapGLAllocate(br_device_gl *device, br_output_facility_gl *facility)
{
	br_device_pixelmap_gl *self = (br_device_pixelmap_gl*)BrResAllocate(facility, sizeof(br_device_pixelmap_gl), BR_MEMORY_OBJECT);
	if(self == nullptr)
		return nullptr;

	self->dispatch = (struct br_device_pixelmap_dispatch*)&devicePixelmapDispatch;

	self->device = device;
	self->output_facility = facility;
	self->useType = BRT_NONE;

	self->pm_type = (br_uint_8)facility->colour_type;
	self->pm_width = (br_uint_16)facility->width;
	self->pm_height = (br_uint_16)facility->height;
	self->pm_origin_x = 0;
	self->pm_origin_y = 0;
	self->pm_base_x = 0;
	self->pm_base_y = 0;
	self->pm_flags = BR_PMF_NO_ACCESS;

	self->pm_identifier = facility->identifier;
	device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(device, devicePixelmapTemplateEntries, BR_ASIZE(devicePixelmapTemplateEntries));

	ObjectContainerAddFront(facility, (br_object *)self);

	if(initPixelmapGL(self) != BRE_OK)
		return nullptr;

	return self;
}