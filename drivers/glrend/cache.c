#include "drv.h"
#include "brassert.h"

/*
** Process each light, doing as much once-per-frame work as possible.
** - For work that cannot be done here, see GLSTATE_ProcessActiveLights()
*/
static void ProcessSceneLights(state_cache *cache, const state_light *lights)
{
    cache->scene.num_lights = 0;
    for(uint32_t i = 0; i < MAX_STATE_LIGHTS; ++i) {
        const state_light *light = lights + i;

        if(light->type == BRT_NONE)
            continue;

        shader_data_light *alp = cache->scene.lights + cache->scene.num_lights;

        /* See enables.c:194, BrSetupLights(). All the lights are already converted into view space. */
        BrVector4Set(&alp->position, light->position.v[0], light->position.v[1], light->position.v[2],
                     light->type == BRT_DIRECT ? 0.0f : 1.0f);

        BrVector4Set(&alp->direction, light->direction.v[0], light->direction.v[1], light->direction.v[2], 0.0f);

        float intensity = 16384.0f; /* Effectively infinite */
        if(light->attenuation_c != 0)
            intensity = BR_RCP(light->attenuation_c);

        if(light->type == BRT_DIRECT) {
            BrVector4Copy(&alp->half, &alp->direction);
            alp->half.v[2] += 1.0f;
            BrVector4Normalise(&alp->half, &alp->half);

            BrVector4Scale(&alp->direction, &alp->direction, intensity);
        }

        BrVector4Set(&alp->iclq, intensity, light->attenuation_c, light->attenuation_l, light->attenuation_q);

        BrVector4Set(&alp->colour, BR_RED(light->colour) / 255.0f, BR_GRN(light->colour) / 255.0f,
                     BR_BLU(light->colour) / 255.0f, light->type == BRT_AMBIENT ? 1.0f : 0.0f);

        if(light->type == BRT_SPOT) {
            BrVector2Set(&alp->spot_angles, BrAngleToRadian(light->spot_inner), BrAngleToRadian(light->spot_outer));
        } else {
            BrVector2Set(&alp->spot_angles, 0, 0);
        }
        ++alp;
        ++cache->scene.num_lights;
    }
}

/*
** Update the per-model matrices.
**
** A good reference of the types is here:
** http://cse.csusb.edu/tongyu/courses/cs520/notes/glsl.php
*/
static void UpdateMatrices(state_cache *cache, state_matrix *matrix)
{
    if(matrix->view_to_environment_hint != BRT_DONT_CARE) {
        BrMatrix34Mul(&cache->model_to_environment, &matrix->model_to_view, &matrix->view_to_environment);
    }

    /*
     * Projection Matrix
     */
    BrMatrix4Copy(&cache->model.p, &matrix->view_to_screen);
    VIDEOI_D3DtoGLProjection(&cache->model.p);

    /*
     * ModelView Matrix
     */
    BrMatrix4Copy34(&cache->model.mv, &matrix->model_to_view);

    /*
     * MVP Matrix
     */
    BrMatrix4Mul(&cache->model.mvp, &cache->model.mv, &cache->model.p);

    /*
     * Normal Matrix
     */
    BrMatrix4Inverse(&cache->model.normal, &cache->model.mv);
    BrMatrix4Transpose(&cache->model.normal);
}

void StateGLUpdateModel(state_cache *cache, state_matrix *matrix)
{
    UpdateMatrices(cache, matrix);
}

void StateGLUpdateScene(state_cache *cache, state_stack *state)
{
    ASSERT(state->output.colour);
    cache->fbo = state->output.colour->asBack.glFbo;

    BrVector4Set(&cache->scene.eye_view, 0.0f, 0.0f, 1.0f, 0.0f);

    ProcessSceneLights(cache, state->light);
}

static void ResetCacheLight(shader_data_light *alp)
{
    /* NB: For future reference, if shit crashes check that we're aligned properly.
    uintptr_t p = reinterpret_cast<uintptr_t>(&hLight->position);
    if(p % 16 != 0)
    {
        fprintf(stderr, "not aligned\n");
        BrDebugBreak();
    }
    */

    BrVector4Set(&alp->position, 0, 0, 0, 0);
    BrVector4Set(&alp->direction, 0, 0, 0, 0);
    BrVector4Set(&alp->half, 0, 0, 0, 0);
    BrVector4Set(&alp->colour, 0, 0, 0, 0);
    BrVector4Set(&alp->iclq, 0, 0, 0, 0);
    BrVector2Set(&alp->spot_angles, 0, 0);
}

void StateGLReset(state_cache *cache)
{
    BrMatrix4Identity(&cache->model.p);
    BrMatrix4Identity(&cache->model.mv);
    BrMatrix4Identity(&cache->model.mvp);
    BrMatrix4Identity(&cache->model.normal);

    BrVector4Set(&cache->scene.eye_view, 0, 0, 0, 0);

    for(int i = 0; i < BR_ASIZE(cache->scene.lights); ++i) {
        ResetCacheLight(cache->scene.lights + i);
    }

    cache->scene.num_lights = 0;
}
