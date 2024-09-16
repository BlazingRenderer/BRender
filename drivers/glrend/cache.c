#include "drv.h"
#include "brassert.h"
#include "shortcut.h"
#include "vecifns.h"

/* softrend/setup.c, from ActiveLightsUpdate() */
static void accumulate_ambient(shader_data_scene *scache, const state_light *lights)
{
    br_boolean found_ambient = BR_FALSE;

    scache->ambient_red       = BR_SCALAR(0.0);
    scache->ambient_green     = BR_SCALAR(0.0);
    scache->ambient_blue      = BR_SCALAR(0.0);
    scache->ambient_intensity = BR_SCALAR(0.0);

    for(int i = 0; i < MAX_STATE_LIGHTS; ++i) {
        const state_light *lp = lights + i;
        br_scalar          intensity;

        if(lp->type != BRT_AMBIENT || lp->attenuation_type == BRT_RADII)
            continue;

        found_ambient = BR_TRUE;

        if(lp->culled)
            continue;

        intensity = BR_RCP(lp->attenuation_c);

        scache->ambient_red += BR_MUL(BrFixedToScalar(BR_RED(lp->colour) << 8), intensity);
        scache->ambient_green += BR_MUL(BrFixedToScalar(BR_GRN(lp->colour) << 8), intensity);
        scache->ambient_blue += BR_MUL(BrFixedToScalar(BR_BLU(lp->colour) << 8), intensity);
        scache->ambient_intensity += intensity;
    }

    if(found_ambient) {
        if(scache->ambient_red > BR_SCALAR(1.0))
            scache->ambient_red = BR_SCALAR(1.0);

        if(scache->ambient_green > BR_SCALAR(1.0))
            scache->ambient_green = BR_SCALAR(1.0);

        if(scache->ambient_blue > BR_SCALAR(1.0))
            scache->ambient_blue = BR_SCALAR(1.0);

        scache->use_ambient_colour = scache->ambient_red != BR_SCALAR(1.0) || scache->ambient_green != BR_SCALAR(1.0) ||
                                     scache->ambient_blue != BR_SCALAR(1.0);

        if(scache->ambient_intensity > BR_SCALAR(1.0))
            scache->ambient_intensity = BR_SCALAR(1.0);

        scache->use_ambient_intensity = scache->ambient_intensity != BR_SCALAR(1.0);

    } else {
        scache->use_ambient_colour    = BR_FALSE;
        scache->use_ambient_intensity = BR_FALSE;
    }
}

/**
 * \brief Process each light, doing as much once-per-frame work as possible.
 *
 * \remark BRender does support model/view-space lights. We only support view-space because
 *  there's no benefit to model-space. Luckily, the position/direction is always transformed
 *  in to view-space anyway, so we can just (ab)use that.
 *
 * \remark See BrSetupLighs() in `core/v1db/enables.c` for reference.
 */
static void ProcessSceneLights(state_cache *cache, const state_light *lights)
{
    accumulate_ambient(&cache->scene, lights);

    cache->scene.num_lights = 0;
    for(uint32_t i = 0; i < MAX_STATE_LIGHTS; ++i) {
        const state_light *light     = lights + i;
        shader_data_light *alp       = cache->scene.lights + cache->scene.num_lights;
        br_token           type      = light->type & BR_LIGHT_TYPE;
        float              intensity = 16384.0f;

        if(type == BRT_NONE || light->culled)
            continue;

        /* See enables.c:194, BrSetupLights(). All the lights are already converted into view space. */
        BrVector4Set(&alp->position, light->position.v[0], light->position.v[1], light->position.v[2],
                     light->type == BRT_DIRECT ? 0.0f : 1.0f);

        BrVector4Set(&alp->direction, light->direction.v[0], light->direction.v[1], light->direction.v[2], 0.0f);

        if(light->attenuation_c != BR_SCALAR(0))
            intensity = BR_RCP(light->attenuation_c);

        if(light->type == BRT_DIRECT) {
            /*
             * Work out a unit half vector:
             *  eye = (0,0,1)
             *  half = normalise(light_direection + eye)
             */
            BrVector4Copy(&alp->half, &alp->direction);
            alp->half.v[2] += 1.0f;
            BrVector4Normalise(&alp->half, &alp->half);

            BrVector4Scale(&alp->direction, &alp->direction, intensity);
        }

        BrVector4Set(&alp->iclq, intensity, light->attenuation_c, light->attenuation_l, light->attenuation_q);

        BrVector4Set(&alp->colour, BR_RED(light->colour) / 255.0f, BR_GRN(light->colour) / 255.0f,
                     BR_BLU(light->colour) / 255.0f, light->type == BRT_AMBIENT ? 1.0f : 0.0f);

        if(light->attenuation_type == BRT_RADII) {
            if(light->radius_inner != light->radius_outer)
                alp->falloff = BR_DIV(alp->iclq.v[0], light->radius_inner - light->radius_outer);

            alp->cutoff = BR_SQR(light->radius_outer);
        }

        if(light->type == BRT_SPOT) {
            BrVector2Set(&alp->spot_angles, BrAngleToRadian(light->spot_inner), BrAngleToRadian(light->spot_outer));
            alp->spot_falloff = BR_RCP(light->spot_outer - light->spot_inner);
        } else {
            BrVector2Set(&alp->spot_angles, 0, 0);
            alp->spot_falloff = 0.0f;
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
        br_matrix34 tmp;
        BrMatrix34Mul(&tmp, &matrix->model_to_view, &matrix->view_to_environment);
        BrMatrix4Copy34(&cache->model.environment, &tmp);
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
     * Inverse of ModelView.
     */
    BrMatrix4Inverse(&cache->model.view_to_model, &cache->model.mv);

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

/*
 * Find centre of projection in model space
 */
static br_vector4 EyeInModel(const state_cache *cache, const state_matrix *matrix)
{
    br_matrix4 s2m;
    br_vector4 eye_m;

    /*
     * Spot special, easy, cases
     */
    if(matrix->model_to_view_hint == BRT_LENGTH_PRESERVING) {
        if(matrix->view_to_screen_hint == BRT_PERSPECTIVE) {
            eye_m.v[0] = -BR_MAC3(matrix->model_to_view.m[3][0], matrix->model_to_view.m[0][0],
                                  matrix->model_to_view.m[3][1], matrix->model_to_view.m[0][1],
                                  matrix->model_to_view.m[3][Z], matrix->model_to_view.m[0][2]);
            eye_m.v[1] = -BR_MAC3(matrix->model_to_view.m[3][0], matrix->model_to_view.m[1][0],
                                  matrix->model_to_view.m[3][1], matrix->model_to_view.m[1][1],
                                  matrix->model_to_view.m[3][Z], matrix->model_to_view.m[1][2]);
            eye_m.v[2] = -BR_MAC3(matrix->model_to_view.m[3][0], matrix->model_to_view.m[2][0],
                                  matrix->model_to_view.m[3][1], matrix->model_to_view.m[2][1],
                                  matrix->model_to_view.m[3][Z], matrix->model_to_view.m[2][2]);

            eye_m.v[3] = BR_SCALAR(1.0);

            return eye_m;
        }

        if(matrix->view_to_screen_hint == BRT_PARALLEL) {
            BrVector3CopyMat34Col((br_vector3 *)&eye_m, &matrix->model_to_view, 2);
            eye_m.v[3] = BR_SCALAR(0.0);
            return eye_m;
        }

    } else {
        if(matrix->view_to_screen_hint == BRT_PERSPECTIVE) {
            BrVector3CopyMat34Row((br_vector3 *)&eye_m, &cache->model.view_to_model, 3);
            eye_m.v[3] = BR_SCALAR(1.0);
            return eye_m;
        }

        if(matrix->view_to_screen_hint == BRT_PARALLEL) {
            BrVector3CopyMat34Row((br_vector3 *)&eye_m, &cache->model.view_to_model, 2);
            eye_m.v[3] = BR_SCALAR(0.0);
            return eye_m;
        }
    }

    /*
     * If reached here, then we need to invert model_to_screen
     */
    BrMatrix4Inverse(&s2m, &cache->model.mvp);

    eye_m.v[0] = s2m.m[Z][0];
    eye_m.v[1] = s2m.m[Z][1];
    eye_m.v[2] = s2m.m[Z][2];
    eye_m.v[3] = s2m.m[Z][3];
    return eye_m;
}

void StateGLUpdateModel(state_cache *cache, state_matrix *matrix)
{
    UpdateMatrices(cache, matrix);

    cache->model.eye_m = EyeInModel(cache, matrix);
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
    BrMatrix4Identity(&cache->model.environment);
    BrVector4Set(&cache->model.eye_m, 0, 0, 0, 1);

    BrVector4Set(&cache->scene.eye_view, 0, 0, 0, 0);

    for(int i = 0; i < BR_ASIZE(cache->scene.lights); ++i) {
        ResetCacheLight(cache->scene.lights + i);
    }

    cache->scene.num_lights = 0;
}
