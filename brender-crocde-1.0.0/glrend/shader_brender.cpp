#include <stdio.h>
#include <vector>
#include <log.h>
#include "brender.h"
#include "brassert.h"
#include "video_ip.hpp"

static void VIDEOI_GetShaderVariables(HVIDEO hVideo)
{
	glGenBuffers(1, &hVideo->brenderProgram.uboScene);
	glBindBuffer(GL_UNIFORM_BUFFER, hVideo->brenderProgram.uboScene);
	glUniformBlockBinding(hVideo->brenderProgram.program, hVideo->brenderProgram.blockIndexScene, hVideo->brenderProgram.blockBindingScene);
	glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingScene, hVideo->brenderProgram.uboScene);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GLSTD140_SCENE_DATA), nullptr, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &hVideo->brenderProgram.uboModel);
	glBindBuffer(GL_UNIFORM_BUFFER, hVideo->brenderProgram.uboModel);
	glUniformBlockBinding(hVideo->brenderProgram.program, hVideo->brenderProgram.blockIndexModel, hVideo->brenderProgram.blockBindingModel);
	glBindBufferBase(GL_UNIFORM_BUFFER, hVideo->brenderProgram.blockBindingModel, hVideo->brenderProgram.uboModel);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GLSTD140_MODEL_DATA), nullptr, GL_DYNAMIC_DRAW);


	hVideo->brenderProgram.attributes.aPosition = glGetAttribLocation(hVideo->brenderProgram.program, "aPosition");
	hVideo->brenderProgram.attributes.aUV = glGetAttribLocation(hVideo->brenderProgram.program, "aUV");
	hVideo->brenderProgram.attributes.aNormal = glGetAttribLocation(hVideo->brenderProgram.program, "aNormal");
	hVideo->brenderProgram.attributes.aColour = glGetAttribLocation(hVideo->brenderProgram.program, "aColour");
	hVideo->brenderProgram.uniforms.main_texture = glGetUniformLocation(hVideo->brenderProgram.program, "main_texture");
	glBindFragDataLocation(hVideo->textProgram.program, 0, "mainColour");
}

bool VIDEOI_CompileBRenderShader(HVIDEO hVideo)
{
	hVideo->brenderProgram.mainTextureBinding = 0;
	hVideo->brenderProgram.blockBindingScene = 1;
	hVideo->brenderProgram.blockBindingModel = 2;

	{
		int neededSize = std::max(sizeof(GLSTD140_SCENE_DATA), sizeof(GLSTD140_MODEL_DATA));
		if(hVideo->maxUniformBlockSize < neededSize)
		{
			log_error("VIDEO", "GL_MAX_UNIFORM_BLOCK_SIZE too small, got %d, needed %d.", hVideo->maxUniformBlockSize, neededSize);
			return false;
		}
	}

	if(hVideo->maxUniformBufferBindings < 2)
	{
		log_error("VIDEO", "GL_MAX_UNIFORM_BUFFER_BINDINGS too small, got %d, needed 2.", hVideo->maxUniformBufferBindings);
		return false;
	}

	/* br_model_state */
	if(hVideo->maxVertexUniformBlocks < 1)
	{
		log_error("VIDEO", "GL_MAX_VERTEX_UNIFORM_BLOCKS too small, got %d, needed 1.", hVideo->maxVertexUniformBlocks);
		return false;
	}

	/* br_model_state, br_scene_state */
	if(hVideo->maxFragmentUniformBlocks < 2)
	{
		log_error("VIDEO", "GL_MAX_FRAGMENT_UNIFORM_BLOCKS too small, got %d, needed 2.", hVideo->maxFragmentUniformBlocks);
		return false;
	}

	size_t size;
	GLchar *vertSource = VIDEOI_LoadFileFromDisk("brender.vert", &size);
	if(vertSource == nullptr)
		return false;

	GLuint vert = VIDEOI_CreateAndCompileShader(GL_VERTEX_SHADER, vertSource, size);
	if(!vert)
	{
		BrScratchFree(vertSource);
		return false;
	}

	BrScratchFree(vertSource);

	GLchar *fragSource = VIDEOI_LoadFileFromDisk("brender.frag", &size);
	if(fragSource == nullptr)
	{
		glDeleteShader(vert);
		return false;
	}

	GLuint frag = VIDEOI_CreateAndCompileShader(GL_FRAGMENT_SHADER, fragSource, size);
	if(!frag)
	{
		glDeleteShader(vert);
		BrScratchFree(fragSource);
		return false;
	}

	BrScratchFree(fragSource);

	if(!(hVideo->brenderProgram.program = VIDEOI_CreateAndCompileProgram(vert, frag)))
	{
		glDeleteShader(vert);
		glDeleteShader(frag);
		return false;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);

	if(hVideo->brenderProgram.program)
	{
		hVideo->brenderProgram.blockIndexScene = glGetUniformBlockIndex(hVideo->brenderProgram.program, "br_scene_state");
		if(hVideo->brenderProgram.blockIndexScene == GL_INVALID_INDEX)
		{
			log_error("VIDEO", "Unable to retrieve block index for uniform block 'br_scene_state'.");
			return false;
		}

		hVideo->brenderProgram.blockIndexModel = glGetUniformBlockIndex(hVideo->brenderProgram.program, "br_model_state");
		if(hVideo->brenderProgram.blockIndexModel == GL_INVALID_INDEX)
		{
			log_error("VIDEO", "Unable to retrieve block index for uniform block 'br_model_state'.");
			return false;
		}

		VIDEOI_GetShaderVariables(hVideo);
	}

	while(glGetError() != GL_NO_ERROR);

	return hVideo->brenderProgram.program ? true : false;
}
