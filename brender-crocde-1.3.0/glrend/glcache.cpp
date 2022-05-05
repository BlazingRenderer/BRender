#include "glstate.hpp"
#include "video_ip.hpp"
#include "drv_ip.hpp"

extern "C" {
#include "brassert.h"
}

static void dumpState(HGLSTATE_STACK hState, bool dir)
{
	for(size_t i = 0; i < GLSTATE_MAX_LIGHTS; ++i)
	{
		HGLSTATE_LIGHT hLight = &hState->light[i];
		if(hLight->type == BRT_NONE)
			break;

		float *v = dir ? hLight->direction.v : hLight->position.v;
		fprintf(stderr, "(%.3f, %.3f, %.3f) ", v[0], v[1], v[2]);
	}

	fprintf(stderr, "\n");

}
//
//static void dumpCache(HGLCACHE hCache, bool dir)
//{
//	for(size_t i = 0; i < GLSTATE_MAX_LIGHTS; ++i)
//	{
//		HGLCACHE_LIGHT hActiveLight = &hCache->scene.active_lights[i];
//		if(hActiveLight->type == BRT_NONE)
//			break;
//
//		glm::vec3 v = dir ? hActiveLight->direction : hActiveLight->position;
//		fprintf(stderr, "(%.3f, %.3f, %.3f) ", v.x, v.y, v.z);
//	}
//
//	fprintf(stderr, "\n");
//}

static void BrVector3CopyMat34Col(br_scalar *v, br_matrix34 *matrix, int col)
{
	v[0] = (matrix)->m[0][(col)];
	v[1] = (matrix)->m[1][(col)];
	v[2] = (matrix)->m[2][(col)];
}

static void BrVector3CopyMat34Row(br_scalar *v, br_matrix34 *matrix, int row)
{
	v[0] = (matrix)->m[(row)][0];
	v[1] = (matrix)->m[(row)][1];
	v[2] = (matrix)->m[(row)][2];
}

#if DEBUG
#include <glm/gtc/epsilon.hpp>
/*
** Check if two glm::matXxX's are equal.
** Used to double-check BRender's math agains glm's.
*/
#	define ASSERT_MATRIX_EQUALS(m1, m2)										\
		do																	\
		{																	\
			decltype(m1) _m1 = m1;											\
			decltype(m2) _m2 = m2;											\
			ASSERT(_m1.length() == _m2.length());							\
			for(size_t i = 0; i < _m1.length(); ++i)						\
			{																\
				ASSERT(glm::all(glm::epsilonEqual(_m1[i], _m2[i], 0.01f)));	\
			}																\
		} while(0)
#else
#	define ASSERT_MATRIX_EQUALS(m1, m2)
#endif


/*
** Process each light, doing as much once-per-frame work as possible.
** - For work that cannot be done here, see GLSTATE_ProcessActiveLights()
*/
void GLCACHEI_ProcessSceneLights(HGLCACHE hCache, const HGLSTATE_LIGHT lights)
{
	hCache->scene.num_lights = 0;
	for(uint32_t i = 0; i < GLSTATE_MAX_LIGHTS; ++i)
	{
		const HGLSTATE_LIGHT hLight = &lights[i];

		if(hLight->type == BRT_NONE)
			continue;

		HGLSTD140_LIGHT hActiveLight = &hCache->scene.lights[hCache->scene.num_lights];

		/* See enables.c:194, BrSetupLights(). All the lights are already converted into view space. */
		hActiveLight->position = glm::vec4(
			glm::make_vec3(hLight->position.v),
			hLight->type == BRT_DIRECT ? 0.0f : 1.0f
		);

		hActiveLight->direction = glm::vec4(
			glm::make_vec3(hLight->direction.v),
			0.0f
		);

		float intensity = BR_RCP(hLight->attenuation_c);

		if(hLight->type == BRT_DIRECT)
		{
			hActiveLight->half = glm::normalize(hActiveLight->direction + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
			hActiveLight->direction *= intensity;
		}

		hActiveLight->iclq = glm::vec4(
			intensity,
			hLight->attenuation_c,
			hLight->attenuation_l,
			hLight->attenuation_q
		);

		hActiveLight->colour = glm::vec4(
			BR_RED(hLight->colour) / 255.0f,
			BR_GRN(hLight->colour) / 255.0f,
			BR_BLU(hLight->colour) / 255.0f,
			0.0f
		);

		if(hLight->type == BRT_SPOT)
		{
			hActiveLight->spot_angles = glm::vec2(
				BrAngleToRadian(hLight->spot_inner),
				BrAngleToRadian(hLight->spot_outer)
			);
		}
		else
		{
			hActiveLight->spot_angles = glm::vec2(0.0f, 0.0f);
		}
		++hActiveLight;
		++hCache->scene.num_lights;
	}
}

/*
** Update the per-model matrices.
**
** A good reference of the types is here:
** http://cse.csusb.edu/tongyu/courses/cs520/notes/glsl.php
*/
void GLCACHEI_UpdateMatrices(HGLCACHE hCache, HGLSTATE_MATRIX hMatrix)
{
	if(hMatrix->view_to_environment_hint != BRT_DONT_CARE)
	{
		BrMatrix34Mul(&hCache->model_to_environment,
					  &hMatrix->model_to_view,
					  &hMatrix->view_to_environment);
	}

	{ /* Projection matrix */
	  //// Standard GLM
	  ////extern "C" br_actor *observer;
	  //br_camera *cam = (br_camera*)observer->type_data;
	  //glm::mat4x4 glmStandard = glm::perspective(BrAngleToScalar(cam->field_of_view) * 2 * glm::pi<float>(), cam->aspect, cam->hither_z, cam->yon_z);
	  //glm::mat4x4 glmStandardRH = glm::perspectiveRH(BrAngleToScalar(cam->field_of_view) * 2 * glm::pi<float>(), cam->aspect, cam->hither_z, cam->yon_z);
	  //ASSERT(glmStandard == glmStandardRH);

		hCache->model.p = VIDEOI_BrMatrix4ToGLM(&hMatrix->view_to_screen);
	}

	/* ModelView */
	hCache->model.mv = VIDEOI_BrMatrix34ToGLM44(&hMatrix->model_to_view);

	/* MVP */
	hCache->model.mvp = hCache->model.p * hCache->model.mv;

	/* Normal Matrix */
	hCache->model.normal = glm::inverseTranspose(hCache->model.mv);
}

GLSTATE_API void GLCACHE_UpdateModel(HGLCACHE hCache, HGLSTATE_MATRIX hMatrix)
{
	GLCACHEI_UpdateMatrices(hCache, hMatrix);
}

GLSTATE_API void GLCACHE_UpdateScene(HGLCACHE hCache, HGLSTATE_STACK hState)
{
	ASSERT(hState->output.colour);
	hCache->fbo = hState->output.colour->asBack.glFbo;
	hCache->scene.eye_view = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	hCache->scene.global_ambient_colour = glm::vec4(
		BR_RED(hState->global.ambient_colour) / 255.0f,
		BR_GRN(hState->global.ambient_colour) / 255.0f,
		BR_BLU(hState->global.ambient_colour) / 255.0f,
		1.0f
	);

	GLCACHEI_ProcessSceneLights(hCache, hState->light);
}

static void GLSTATEI_ResetCacheLight(HGLSTD140_LIGHT hLight)
{
	/* NB: For future reference, if shit crashes check that we're aligned properly.
	uintptr_t p = reinterpret_cast<uintptr_t>(&hLight->position);
	if(p % 16 != 0)
	{
		fprintf(stderr, "not aligned\n");
		BrDebugBreak();
	}
	*/
	hLight->position = glm::vec4();
	hLight->direction = glm::vec4();
	hLight->half = glm::vec4();
	hLight->colour = glm::vec4();
	hLight->iclq = glm::vec4();
	hLight->spot_angles = glm::vec2();
}

GLSTATE_API void GLCACHE_Reset(HGLCACHE hCache)
{
	hCache->model.p = glm::mat4x4();
	hCache->model.mv = glm::mat4x4();
	hCache->model.mvp = glm::mat4x4();
	hCache->model.normal = glm::mat3x3();

	hCache->scene.eye_view = glm::vec4();
	for(auto& l : hCache->scene.lights)
		GLSTATEI_ResetCacheLight(&l);

	hCache->scene.num_lights = 0;
}
