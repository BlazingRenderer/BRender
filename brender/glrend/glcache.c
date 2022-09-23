#include <stdio.h>

#include "drv.h"
#include "brassert.h"

static void dumpState(HGLSTATE_STACK hState, int dir)
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
		BrVector4Set(
            &hActiveLight->position,
			hLight->position.v[0],
			hLight->position.v[1],
			hLight->position.v[2],
			hLight->type == BRT_DIRECT ? 0.0f : 1.0f
		);

		BrVector4Set(
            &hActiveLight->direction,
			hLight->direction.v[0],
			hLight->direction.v[1],
			hLight->direction.v[2],
			0.0f
		);

		float intensity = 16384.0f; /* Effectively infinite */
		if(hLight->attenuation_c != 0)
			intensity = BR_RCP(hLight->attenuation_c);

		if(hLight->type == BRT_DIRECT)
		{
			BrVector4Copy(&hActiveLight->half, &hActiveLight->direction);
			hActiveLight->half.v[2] += 1.0f;
			BrVector4Normalise(&hActiveLight->half, &hActiveLight->half);

			BrVector4Scale(&hActiveLight->direction, &hActiveLight->direction, intensity);
		}

        BrVector4Set(
            &hActiveLight->iclq,
			intensity,
			hLight->attenuation_c,
			hLight->attenuation_l,
			hLight->attenuation_q
		);

		BrVector4Set(
            &hActiveLight->colour,
			BR_RED(hLight->colour) / 255.0f,
			BR_GRN(hLight->colour) / 255.0f,
			BR_BLU(hLight->colour) / 255.0f,
			0.0f
		);

		if(hLight->type == BRT_SPOT)
		{
			BrVector2Set(
                &hActiveLight->spot_angles,
				BrAngleToRadian(hLight->spot_inner),
				BrAngleToRadian(hLight->spot_outer)
			);
		}
		else
		{
            BrVector2Set(&hActiveLight->spot_angles, 0, 0);
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

	/*
	 * Projection Matrix
	 */
	BrMatrix4Copy(&hCache->model.p, &hMatrix->view_to_screen);
	VIDEOI_D3DtoGLProjection(&hCache->model.p);

	/*
	 * ModelView Matrix
	 */
	BrMatrix4Copy34(&hCache->model.mv, &hMatrix->model_to_view);

	/*
	 * MVP Matrix
	 */
	BrMatrix4Mul(&hCache->model.mvp, &hCache->model.mv, &hCache->model.p);

	/*
	 * Normal Matrix
	 */
	BrMatrix4Inverse(&hCache->model.normal, &hCache->model.mv);
	BrMatrix4Transpose(&hCache->model.normal);
}

GLSTATE_API void GLCACHE_UpdateModel(HGLCACHE hCache, HGLSTATE_MATRIX hMatrix)
{
	GLCACHEI_UpdateMatrices(hCache, hMatrix);
}

GLSTATE_API void GLCACHE_UpdateScene(HGLCACHE hCache, HGLSTATE_STACK hState)
{
	ASSERT(hState->output.colour);
	hCache->fbo = hState->output.colour->asBack.glFbo;

    BrVector4Set(&hCache->scene.eye_view, 0.0f, 0.0f, 1.0f, 0.0f);
	BrVector4Set(
        &hCache->scene.global_ambient_colour,
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

    BrVector4Set(&hLight->position, 0, 0, 0, 0);
    BrVector4Set(&hLight->direction, 0, 0, 0, 0);
    BrVector4Set(&hLight->half, 0, 0, 0, 0);
    BrVector4Set(&hLight->colour, 0, 0, 0, 0);
    BrVector4Set(&hLight->iclq, 0, 0, 0, 0);
    BrVector2Set(&hLight->spot_angles, 0, 0);
}

GLSTATE_API void GLCACHE_Reset(HGLCACHE hCache)
{
	BrMatrix4Identity(&hCache->model.p);
	BrMatrix4Identity(&hCache->model.mv);
	BrMatrix4Identity(&hCache->model.mvp);
	BrMatrix4Identity(&hCache->model.normal);

	BrVector4Set(&hCache->scene.eye_view, 0, 0, 0, 0);

    for(int i = 0; i < BR_ASIZE(hCache->scene.lights); ++i) {
        GLSTATEI_ResetCacheLight(hCache->scene.lights + i);
    }

	hCache->scene.num_lights = 0;
}
