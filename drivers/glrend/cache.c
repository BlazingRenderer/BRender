#include "drv.h"
#include "brassert.h"
#include "shortcut.h"
#include "vecifns.h"

static int sort_lights(const void *a, const void *b)
{
    const shader_data_light *l1 = a;
    const shader_data_light *l2 = b;

    if(l1->light_type < l2->light_type)
        return -1;

    if(l1->light_type > l2->light_type)
        return 1;

    return 0;
}

/*
** Process each light, doing as much once-per-frame work as possible.
** - For work that cannot be done here, see GLSTATE_ProcessActiveLights()
*/
static void ProcessSceneLights(state_cache *cache, const state_light *lights)
{
    shader_data_scene *scene      = &cache->scene;
    size_t             num_lights = 0;

    br_vector4_i counts = {
        .v = {0, 0, 0, 0},
    };

    for(uint32_t i = 0; i < MAX_STATE_LIGHTS; ++i) {
        const state_light *light = lights + i;
        shader_data_light *alp   = cache->scene.lights + num_lights;

        switch(light->type) {
            case BRT_AMBIENT:
                alp->light_type = 0;
                ++counts.v[0];
                break;
            case BRT_DIRECT:
                alp->light_type = 1;
                ++counts.v[1];
                break;
            case BRT_POINT:
                alp->light_type = 2;
                ++counts.v[2];
                break;
            case BRT_SPOT:
                alp->light_type = 3;
                ++counts.v[3];
                break;
            default:
                continue;
        }

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

        alp->intensity     = intensity;
        alp->attenuation_c = light->attenuation_c;
        alp->attenuation_l = light->attenuation_l;
        alp->attenuation_q = light->attenuation_q;

        BrVector4Set(&alp->colour, BR_RED(light->colour) / 255.0f, BR_GRN(light->colour) / 255.0f,
                     BR_BLU(light->colour) / 255.0f, light->type == BRT_AMBIENT ? 1.0f : 0.0f);

        if(light->type == BRT_SPOT) {
            alp->spot_inner_cos = light->spot_inner;
            alp->spot_outer_cos = light->spot_outer;
        } else {
            alp->spot_inner_cos = 0.0f;
            alp->spot_outer_cos = 0.0f;
        }

        switch(light->attenuation_type) {
            case BRT_QUADRATIC:
            default:
                alp->attenuation_type = 0;
                break;

            case BRT_RADII:
                alp->attenuation_type = 1;
                alp->radius_inner     = light->radius_inner;
                alp->radius_outer     = light->radius_outer;
                break;
        }

        ++alp;
        ++num_lights;
    }

    BrQsort(scene->lights, num_lights, sizeof(shader_data_light), sort_lights);

    scene->light_start.v[0] = 0;
    scene->light_end.v[0]   = counts.v[0];

    scene->light_start.v[1] = scene->light_end.v[0];
    scene->light_end.v[1]   = scene->light_start.v[1] + counts.v[1];

    scene->light_start.v[2] = scene->light_end.v[1];
    scene->light_end.v[2]   = scene->light_start.v[2] + counts.v[2];

    scene->light_start.v[3] = scene->light_end.v[2];
    scene->light_end.v[3]   = scene->light_start.v[3] + counts.v[3];
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

    cache->scene.num_clip_planes = 0;
    if(state->valid & MASK_STATE_CLIP) {
        for(int i = 0; i < BR_ASIZE(cache->scene.clip_planes); ++i) {
            const state_clip *cp = state->clip + i;
            if(cp->type != BRT_PLANE)
                continue;

            /*
             * BrSetupClipPlanes() does "Push plane through to screen space".
             * We need to undo that particular transformation.
             */
            BrMatrix4TApply(cache->scene.clip_planes + i, &cp->plane, &state->matrix.view_to_screen);

            ++cache->scene.num_clip_planes;
        }
    }
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
    alp->intensity      = 0.0f;
    alp->attenuation_c  = 0.0f;
    alp->attenuation_l  = 0.0f;
    alp->attenuation_q  = 0.0f;
    alp->spot_inner_cos = 0.0f;
    alp->spot_outer_cos = 0.0f;
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

    BrMemSet(&cache->scene.light_start, 0, sizeof(cache->scene.light_start));
    BrMemSet(&cache->scene.light_end, 0, sizeof(cache->scene.light_end));

    for(int i = 0; i < BR_ASIZE(cache->scene.clip_planes); ++i) {
        BrVector4Set(cache->scene.clip_planes + i, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(0.0));
    }

    cache->scene.num_clip_planes = 0;
}
