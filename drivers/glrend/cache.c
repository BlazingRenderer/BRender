#include "drv.h"
#include "brassert.h"
#include "shortcut.h"
#include "vecifns.h"

static int light_type_to_int(br_token type)
{
    switch(type) {
        case BRT_AMBIENT:
            return 0;
        case BRT_DIRECT:
            return 1;
        case BRT_POINT:
            return 2;
        case BRT_SPOT:
            return 3;
        default:
            return 100;
    }
}

static int sort_lights(const void *a, const void *b)
{
    const state_light *l1    = a;
    const state_light *l2    = b;
    const int          type1 = light_type_to_int(l1->type);
    const int          type2 = light_type_to_int(l2->type);

    if(type1 < type2)
        return -1;

    if(type1 > type2)
        return 1;

    return 0;
}

/*
 * -1 = completely ignore this light.
 *  0 = ok
 */
static int casfd(br_gl_main_data_lights *lights, br_vector4 *ambient_colour, const state_light *in, size_t i, br_vector4_i *counts,
                 br_boolean *use_ambient_colour)
{
    if(in->type == BRT_NONE)
        return -1;

    float intensity = 16384.0f; /* Effectively infinite */
    if(in->attenuation_c != 0.0f)
        intensity = BR_RCP(in->attenuation_c);

    br_vector4 colour;
    BrVector4ColourSet(&colour, in->colour);

    /*
     * Regular (i.e. non-radial) ambient lights can be accumulated early,
     * moving work out of the shader.
     */
    if(in->type == BRT_AMBIENT && in->attenuation_type != BRT_RADII) {
        BrVector4AccumulateScale(ambient_colour, &colour, intensity);
        *use_ambient_colour = BR_TRUE;
        return -1;
    }

    br_gl_main_data_light_info  *info  = lights->light_info + i;
    br_gl_main_data_light_atten *atten = lights->light_atten + i;
    br_gl_main_data_light_radii *radii = lights->light_radii + i;

    switch(in->type) {
        case BRT_AMBIENT:
            info->type = 0;
            break;
        case BRT_DIRECT:
            info->type = 1;
            break;
        case BRT_POINT:
            info->type = 2;
            break;
        case BRT_SPOT:
            info->type = 3;
            break;
        default:
            return -1;
    }

    counts->v[info->type]++;

    /* See enables.c:194, BrSetupLights(). All the lights are already converted into view space. */
    BrVector4Set(lights->light_positions + i, in->position.v[0], in->position.v[1], in->position.v[2], in->type == BRT_DIRECT ? 0.0f : 1.0f);
    BrVector4Set(lights->light_directions + i, in->direction.v[0], in->direction.v[1], in->direction.v[2], 0.0f);

    if(in->type == BRT_DIRECT) {
        BrVector4Copy(lights->light_halfs + i, lights->light_directions + i);
        lights->light_halfs[i].v[2] += 1.0f;
        BrVector4Normalise(lights->light_halfs + i, lights->light_halfs + i);

        BrVector4Scale(lights->light_directions + i, lights->light_directions + i, intensity);
    }

    atten->intensity     = intensity;
    atten->attenuation_c = in->attenuation_c;
    atten->attenuation_l = in->attenuation_l;
    atten->attenuation_q = in->attenuation_q;

    lights->light_colours[i] = colour;

    if(in->type == BRT_SPOT) {
        radii->spot_cos_inner = in->spot_inner;
        radii->spot_cos_outer = in->spot_outer;
    } else {
        radii->spot_cos_inner = 0.0f;
        radii->spot_cos_outer = 0.0f;
    }

    switch(in->attenuation_type) {
        case BRT_QUADRATIC:
        default:
            info->attenuation_type = 0;
            break;

        case BRT_RADII:
            info->attenuation_type = 1;

            radii->radius_inner = in->radius_inner;
            radii->radius_outer = in->radius_outer;

            if(radii->radius_inner == radii->radius_outer)
                radii->radius_inner = 0;

            break;
    }

    return 0;
}

/*
** Process each light, doing as much once-per-frame work as possible.
** - For work that cannot be done here, see GLSTATE_ProcessActiveLights()
*/
static void ProcessSceneLights(state_cache *cache, state_light *lights)
{
    br_gl_main_data_scene *scene              = &cache->scene;
    size_t                 num_lights         = 0;
    br_boolean             use_ambient_colour = BR_FALSE;

    br_vector4_i counts = {
        .v = {0, 0, 0, 0},
    };

    /*
     * Sort the lights first - ambient < direct < point < spot.
     */
    BrQsort(lights, MAX_STATE_LIGHTS, sizeof(state_light), sort_lights);

    BrVector4ColourSet(&scene->ambient_colour, BR_COLOUR_RGBA(0, 0, 0, 0xFF));

    for(uint32_t i = 0; i < MAX_STATE_LIGHTS; ++i) {
        const state_light *light = lights + i;

        if(casfd(&cache->lights, &scene->ambient_colour, light, num_lights, &counts, &use_ambient_colour) < 0)
            continue;

        /*
         * Record what the per-draw cull needs. Taken from the light state,
         * because the light arrays are only populated for radial attenuation.
         * See StateGLBuildLightLists().
         */
        cache->light_cull[num_lights].radius_cull = light->radius_cull;
        cache->light_cull[num_lights].radius_outer = light->radius_outer;

        ++num_lights;
    }

    cache->num_lights = (br_uint_32)num_lights;

    if(use_ambient_colour) {
        BrVector4Clamp(&scene->ambient_colour, &scene->ambient_colour, BR_SCALAR(0), BR_SCALAR(1));

        if(scene->ambient_colour.v[0] == BR_SCALAR(1) && scene->ambient_colour.v[0] == BR_SCALAR(1) && scene->ambient_colour.v[0] == BR_SCALAR(1))
            use_ambient_colour = BR_FALSE;
    }

    /* No, sorry, I'm not dealing with lighting alpha. */
    scene->ambient_colour.v[3] = 1.0f;
    scene->use_ambient_colour  = use_ambient_colour;
}

/*
 * Upper bound on the largest singular value of the model->view 3x3, via the
 * Frobenius norm (always >= the spectral norm). Scales the model's bounding
 * radius from model space into view space.
 *
 * model_to_view is not necessarily an isometry - models can be scaled, and
 * non-uniformly - so the radius has to be scaled rather than used directly.
 */
static br_scalar ModelToViewMaxScale(const br_matrix4 *mv)
{
    float sum = 0.0f;

    for(int r = 0; r < 3; ++r)
        for(int c = 0; c < 3; ++c) {
            float v = BrScalarToFloat(mv->m[r][c]);
            sum += v * v;
        }

    return BrFloatToScalar(sqrtf(sum));
}

/*
 * Build the lights this draw can see.
 *
 * The survivors of the model cull are copied into the model's light block,
 * grouped by type with light_start/light_end giving each type's range. The
 * shader's loops are then identical to the old per-frame set - the arrays it
 * indexes just hold fewer lights, packed from zero.
 *
 * This is what makes the cull pay: keeping the indices affine is the whole
 * point. An index list or a per-light branch both make the light arrays
 * indirect, which costs more than the skipped work saves.
 *
 * The cull itself follows softrend/setup.c: a light with radius culling enabled
 * cannot reach the model if it lies further from the model's origin than
 * radius_outer plus the model's bounding radius. It is evaluated in view space,
 * which is the space the shader shades in. (softrend's BRT_MODEL path shades in
 * model space and so culls in model space - each path is consistent with
 * itself; the space is not interchangeable.)
 *
 * TODO: angle culling for spot lights, i.e. sphereIntersectsCone() in
 *       softrend/setup.c. Spots currently stay lit, which is conservative.
 */
void StateGLBuildLightLists(const state_cache *cache, const struct v11model *v11m, br_gl_main_data_model *model)
{
    const br_matrix4 *mv = &cache->model.mv;

    /* Model-space origin in view space, i.e. the translation row of mv. */
    br_scalar ox = mv->m[3][0];
    br_scalar oy = mv->m[3][1];
    br_scalar oz = mv->m[3][2];

    /*
     * When v11m is NULL the model's bounds are unknown (the per-triangle path
     * draws from the transient buffer), so nothing is culled.
     */
    br_scalar bound = v11m != NULL ? BR_MUL(ModelToViewMaxScale(mv), v11m->radius) : BR_SCALAR(0.0);

    br_uint_32 out = 0;

    for(br_uint_32 t = 0; t < 4; ++t) {
        br_uint_32 start = out;

        model->light_start.v[t] = (br_int_32)start;

        for(br_uint_32 i = 0; i < cache->num_lights; ++i) {
            if(cache->lights.light_info[i].type != t)
                continue;

            if(v11m != NULL && cache->light_cull[i].radius_cull) {
                const br_vector4 *lp = &cache->lights.light_positions[i];
                br_scalar         dx = lp->v[0] - ox;
                br_scalar         dy = lp->v[1] - oy;
                br_scalar         dz = lp->v[2] - oz;
                br_scalar         radius = cache->light_cull[i].radius_outer + bound;

                /*
                 * A negative radius_outer would make the test meaningless, so
                 * clamp before squaring.
                 */
                if(radius < BR_SCALAR(0.0))
                    radius = BR_SCALAR(0.0);

                if(BR_MUL(dx, dx) + BR_MUL(dy, dy) + BR_MUL(dz, dz) > BR_MUL(radius, radius))
                    continue;
            }

            model->lights.light_info[out]      = cache->lights.light_info[i];
            model->lights.light_positions[out] = cache->lights.light_positions[i];
            model->lights.light_directions[out] = cache->lights.light_directions[i];
            model->lights.light_halfs[out]     = cache->lights.light_halfs[i];
            model->lights.light_colours[out]   = cache->lights.light_colours[i];
            model->lights.light_atten[out]     = cache->lights.light_atten[i];
            model->lights.light_radii[out]     = cache->lights.light_radii[i];

            ++out;
        }

        model->light_end.v[t] = (br_int_32)out;
    }

    ASSERT(out <= BR_MAX_LIGHTS);
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
    cache->model.mv_det3 = BrMatrix34Determinant3(&matrix->model_to_view);

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
            // clang-format off
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
            // clang-format on

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

void StateGLReset(state_cache *cache)
{
    BrMatrix4Identity(&cache->model.p);
    BrMatrix4Identity(&cache->model.mv);
    BrMatrix4Identity(&cache->model.mvp);
    BrMatrix4Identity(&cache->model.normal);
    BrMatrix4Identity(&cache->model.environment);
    cache->model.mv_det3 = 0;
    BrVector4Set(&cache->model.eye_m, 0, 0, 0, 1);

    BrVector4Set(&cache->scene.eye_view, 0, 0, 0, 0);

    BrMemSet(&cache->scene, 0, sizeof(cache->scene));
}
