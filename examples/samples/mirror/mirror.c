//////////////////////////////////////////////////////////////////////
//
// Project  : BRender Mirror Sample
//            Copyright (c) 1996 Argonaut Technologies Ltd.
//            All rights reserved.
//
// Author         : Sam Littlewood
// Demo Framework : Steve Williams
//
// File     : MIRROR.C
//
// Requires : BRender V1.3
//            Microsoft DirectX 3.0, Win95/NT or DOS
//
// Comments : Mirror demo
//
//////////////////////////////////////////////////////////////////////

#include <brdemo.h>

#define ROLLING_BALL_SENSITIVITY 500

enum {
    MIRROR_KEY_L = (1 << 0),
    MIRROR_KEY_M = (1 << 1),
    MIRROR_KEY_O = (1 << 2),
};

typedef struct br_demo_mirror {
    br_scalar  mouse_x;
    br_scalar  mouse_y;
    br_uint_32 mouse_buttons;
    br_uint_32 kb_buttons;

    br_pixelmap *render_pm, *reflected_pm;
    br_pixelmap *palette;
    br_pixelmap *blend_table;

    br_actor *world;

    br_actor *model_t_actor, *model_actor;

    br_actor *floor_actor;

    br_actor *observer_r_actor;

    br_actor *mirror_t_actor, *mirror_actor;

    br_actor *light_actor, *light_t_actor;
    br_light *light;
} br_demo_mirror;

void xyprintf(br_pixelmap *pm, int x, int y, char *fmt, ...);
void PrintMatrix34(br_pixelmap *pm, int x, int y, br_matrix34 *t);

void BuildTestWorld(br_demo *demo, br_model *model_cube, br_model *model_mirror);

void BlendPixelmaps(br_pixelmap *, br_pixelmap *, br_pixelmap *, br_pixelmap *);

void ControlObject(br_demo_mirror *state, br_actor *camera_actor, br_actor *translate_actor, br_actor *rotate_actor,
                   br_scalar mouse_x, br_scalar mouse_y);

void UpdateMirror(br_actor *dest, br_actor *src, br_actor *world);

br_error MirrorInit(br_demo *demo)
{
    br_demo_mirror *state;
    br_model       *cube_model, *mirror_model;
    br_pixelmap    *shade_table;
    int             nmats;
    br_material    *mats[8];

    state      = BrResAllocate(demo, sizeof(br_demo_mirror), BR_MEMORY_APPLICATION);
    demo->user = state;

    *state = (br_demo_mirror){
        .mouse_x       = 0,
        .mouse_y       = 0,
        .mouse_buttons = 0,
        .kb_buttons    = 0,
    };

    if((state->palette = BrPixelmapLoad("winstd.pal")) == NULL) {
        BrLogError("DEMO", "Could not load winstd.pal.");
        return BRE_FAIL;
    }
    BrTableAdd(state->palette);
    BrPixelmapPaletteSet(demo->colour_buffer, state->palette);

    if((shade_table = BrPixelmapLoad("winsh8.tab")) == NULL) {
        BrLogError("DEMO", "Could not load winsh8.tab.");
        return BRE_FAIL;
    }
    BrTableAdd(shade_table);

    if((state->blend_table = BrPixelmapLoad("blend.tab")) == NULL) {
        BrLogError("DEMO", "Cound not load blend.tab.");
        return BRE_FAIL;
    }

    /*
     * Load some materials
     */

    /*
     * The demo framework will install the de-CLUT'ting hook.
     * We don't want that.
     */
    BrMapFindHook(BrMapFindFailedLoad);

    if((nmats = BrFmtScriptMaterialLoadMany("test.mat", mats, BR_ASIZE(mats))) == 0) {
        BrLogError("DEMO", "Cound not load test.mat.");
        return BRE_FAIL;
    }
    BrMaterialAddMany(mats, nmats);

    /*
     * Load and add a model
     */
    if((cube_model = BrModelLoad("cube.dat")) == NULL) {
        BrLogError("DEMO", "Error loading cube.dat.");
        return BRE_FAIL;
    }
    BrModelAdd(cube_model);

    if((mirror_model = BrModelLoad("floor.dat")) == NULL) {
        BrLogError("DEMO", "Error loading floor.dat.");
        return BRE_FAIL;
    }
    BrModelAdd(mirror_model);

    /*
     * Build world
     */
    BuildTestWorld(demo, cube_model, mirror_model);

    return BRE_OK;
}

/*
 * Tumble the actor around
 */
static void MirrorUpdate(br_demo *demo, br_scalar dt)
{
    br_demo_mirror *state = demo->user;

    if(state->mouse_x || state->mouse_y) {
        if(state->kb_buttons & MIRROR_KEY_L) {
            ControlObject(state, demo->camera, state->light_t_actor, state->light_actor, state->mouse_x, state->mouse_y);
        } else if(state->kb_buttons & MIRROR_KEY_M) {
            ControlObject(state, demo->camera, state->mirror_t_actor, state->mirror_actor, state->mouse_x, state->mouse_y);
        } else if(state->kb_buttons & MIRROR_KEY_O) {
            ControlObject(state, demo->camera, demo->camera, state->observer_r_actor, -state->mouse_x, -state->mouse_y);
        } else {
            ControlObject(state, demo->camera, state->model_t_actor, state->model_actor, state->mouse_x, state->mouse_y);
        }
        state->mouse_x = state->mouse_y = 0;
    }
}

static void MirrorProcessEvent(br_demo *demo, const SDL_Event *evt)
{
    br_demo_mirror *state = demo->user;

    switch(evt->type) {
        case SDL_EVENT_MOUSE_MOTION:
            state->mouse_x += evt->motion.xrel;
            state->mouse_y += evt->motion.yrel;
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if(evt->button.button == SDL_BUTTON_LEFT)
                state->mouse_buttons |= SDL_BUTTON_LMASK;

            if(evt->button.button == SDL_BUTTON_RIGHT)
                state->mouse_buttons |= SDL_BUTTON_RMASK;

            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            if(evt->button.button == SDL_BUTTON_LEFT)
                state->mouse_buttons &= ~SDL_BUTTON_LMASK;

            if(evt->button.button == SDL_BUTTON_RIGHT)
                state->mouse_buttons &= ~SDL_BUTTON_RMASK;

            break;

        case SDL_EVENT_KEY_DOWN: {
            switch(evt->key.key) {
                case 'q':
                    SDL_PushEvent(&(SDL_Event){.type = SDL_EVENT_QUIT});
                    break;

                case SDLK_F1:
                    state->light->type = (state->light->type & BR_LIGHT_VIEW) | BR_LIGHT_DIRECT;
                    break;

                case SDLK_F2:
                    state->light->type = (state->light->type & BR_LIGHT_VIEW) | BR_LIGHT_POINT;
                    break;

                case SDLK_F3:
                    state->light->type = (state->light->type & BR_LIGHT_VIEW) | BR_LIGHT_SPOT;
                    break;

                case SDLK_F4:
                    state->light->type ^= BR_LIGHT_VIEW;
                    break;

                case SDLK_F5:
                    state->light->cone_inner += BR_ANGLE_DEG(1.0);
                    state->light->cone_outer = state->light->cone_inner + BR_ANGLE_DEG(5.0);
                    break;

                case SDLK_F6:
                    state->light->cone_inner -= BR_ANGLE_DEG(1.0);
                    state->light->cone_outer = state->light->cone_inner + BR_ANGLE_DEG(5.0);
                    break;

                case 'd': {
                    br_actor *a = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
                    a->model    = state->model_actor->model;
                    a->material = state->model_actor->material;
                    a->t.type   = BR_TRANSFORM_MATRIX34_LP;
                    BrActorToActorMatrix34(&a->t.t.mat, state->model_actor, state->world);
                    break;
                }

                case 'l':
                    state->kb_buttons |= MIRROR_KEY_L;
                    break;

                case 'm':
                    state->kb_buttons |= MIRROR_KEY_M;
                    break;

                case 'o':
                    state->kb_buttons |= MIRROR_KEY_O;
                    break;

                default:
                    break;
            }
            break;
        }

        case SDL_EVENT_KEY_UP:
            switch(evt->key.key) {
                case 'l':
                    state->kb_buttons &= ~MIRROR_KEY_L;
                    break;

                case 'm':
                    state->kb_buttons &= ~MIRROR_KEY_M;
                    break;

                case 'o':
                    state->kb_buttons &= ~MIRROR_KEY_O;
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

static void MirrorRender(br_demo *demo)
{
    br_demo_mirror *state = demo->user;

    /*
     * Clear the buffers
     */

    BrPixelmapFill(state->render_pm, 0);
    BrPixelmapFill(state->reflected_pm, 0);

    if(demo->colour_buffer->type != BR_PMT_INDEX_8) {
        BrPixelmapTextF(demo->colour_buffer,
                        -(BrPixelmapTextWidth(demo->colour_buffer, BrFontProp7x9, "ONLY 8 BIT SCREEN MODES ARE SUPPORTED") / 2),
                        (BrFontProp7x9->glyph_y) / 2, BR_COLOUR_RGB(200, 200, 200), BrFontProp7x9,
                        "ONLY 8 BIT SCREEN MODES ARE SUPPORTED");
        return;
    }

    BrClipPlaneEnable(state->mirror_actor);

    BrPixelmapFill(demo->depth_buffer, 0xFFFFFFFF);

    state->floor_actor->render_style = BR_RSTYLE_NONE;
    state->world->render_style       = BR_RSTYLE_DEFAULT;

    /*
     * Render the scene
     */
    BrMatrix34Identity(&state->world->t.t.mat);
    BrZbSceneRender(demo->world, demo->camera, state->render_pm, demo->depth_buffer);

    /*
     * Render the reflection
     */
    UpdateMirror(state->world, state->mirror_actor, state->world);
    BrZbSceneRender(demo->world, demo->camera, state->reflected_pm, demo->depth_buffer);

    BrClipPlaneDisable(state->mirror_actor);

    /*
     * Render the reflecting floor
     */
    state->floor_actor->render_style = BR_RSTYLE_DEFAULT;
    state->world->render_style       = BR_RSTYLE_NONE;
    BrActorToActorMatrix34(&state->floor_actor->t.t.mat, state->mirror_actor, state->world);
    BrZbSceneRender(demo->world, demo->camera, state->render_pm, demo->depth_buffer);

    BlendPixelmaps(state->render_pm, state->render_pm, state->reflected_pm, state->blend_table);

#if 1
    PrintMatrix34(demo->colour_buffer, 0, 0, &state->model_actor->t.t.mat);
#endif
}

void MirrorDestroy(br_demo *demo)
{
    br_demo_mirror *state = demo->user;

    BrPixelmapFree(state->reflected_pm);
}

void MirrorOnResize(br_demo *demo, br_uint_16 width, br_uint_16 height)
{
    br_demo_mirror *state = demo->user;

    BrDemoDefaultOnResize(demo, width, height);

    state->render_pm = demo->colour_buffer;

    state->render_pm->origin_x = (br_int_16)(state->render_pm->width / 2);
    state->render_pm->origin_y = (br_int_16)(state->render_pm->height / 2);

    if(state->reflected_pm != NULL)
        BrPixelmapFree(state->reflected_pm);

    state->reflected_pm           = BrPixelmapMatch(demo->colour_buffer, BR_PMMATCH_OFFSCREEN);
    state->reflected_pm->origin_x = (br_int_16)(state->reflected_pm->width / 2);
    state->reflected_pm->origin_y = (br_int_16)(state->reflected_pm->height / 2);

    BrPixelmapPaletteSet(state->reflected_pm, state->palette);
}

#define MSCALE BR_SCALAR(0.006)

void ControlObject(br_demo_mirror *state, br_actor *camera_actor, br_actor *translate_actor, br_actor *rotate_actor,
                   br_scalar mouse_x, br_scalar mouse_y)
{
    br_matrix34 mat_roll;
    br_scalar   tx, ty, tz;

    if((state->mouse_buttons & SDL_BUTTON_LMASK) && (state->mouse_buttons & SDL_BUTTON_RMASK)) {
        // move object along the z axis

        tx = BR_MUL(mouse_x, MSCALE);
        tz = BR_MUL(mouse_y, MSCALE);

        BrMatrix34PostTranslate(&translate_actor->t.t.mat, tx, BR_SCALAR(0.0), tz);

    } else if(state->mouse_buttons & SDL_BUTTON_LMASK) {
        // Move object in the XY plane

        tx = BR_MUL(mouse_x, MSCALE);
        ty = -BR_MUL(mouse_y, MSCALE);

        BrMatrix34PostTranslate(&translate_actor->t.t.mat, tx, ty, BR_SCALAR(0.0));

    } else if(state->mouse_buttons & SDL_BUTTON_RMASK) {
        // Rotate object via rolling ball interface
        BrMatrix34RollingBall(&mat_roll, (int)mouse_x, (int)-mouse_y, ROLLING_BALL_SENSITIVITY);

        BrMatrix34Post(&rotate_actor->t.t.mat, &mat_roll);
    }
}

void PrintMatrix34(br_pixelmap *pm, int x, int y, br_matrix34 *t)
{
    int i, j;

    for(i = 0; i < 3; i++)
        for(j = 0; j < 4; j++)
            xyprintf(pm, x + i * 12, y + j * 2, "%11.5f", BrScalarToFloat(t->m[j][i]));
}

void xyprintf(br_pixelmap *pm, int x, int y, char *fmt, ...)
{
    char    temp[256];
    va_list args;
    int     o = 0;
    /*
     * Build output string
     */
    va_start(args, fmt);
    BrVSprintf(temp, fmt, args);
    va_end(args);

    BrPixelmapText(pm, x * 4, y * 6, 255, BrFontFixed3x5, temp);
}

void BuildTestWorld(br_demo *demo, br_model *model_cube, br_model *model_mirror)
{
    br_demo_mirror *state = demo->user;
    br_camera      *camera;

    demo->world->t.type = BR_TRANSFORM_IDENTITY;

    /*
     * Build the world
     */
    state->world         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_NONE, NULL));
    state->world->t.type = BR_TRANSFORM_MATRIX34_LP;

    state->observer_r_actor         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_NONE, NULL));
    state->observer_r_actor->t.type = BR_TRANSFORM_MATRIX34_LP;

    demo->camera         = BrActorAdd(state->observer_r_actor, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    demo->camera->t.type = BR_TRANSFORM_MATRIX34_LP;
    BrMatrix34Translate(&demo->camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(5.0));

    camera           = demo->camera->type_data;
    camera->type     = BR_CAMERA_PERSPECTIVE;
    camera->hither_z = BR_SCALAR(0.1);
    camera->yon_z    = BR_SCALAR(10.0);

    demo->order_table->min_z = camera->hither_z;
    demo->order_table->max_z = camera->yon_z;

    state->model_t_actor         = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_NONE, NULL));
    state->model_t_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
    state->model_actor           = BrActorAdd(state->model_t_actor, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    state->model_actor->model    = model_cube;
    state->model_actor->t.type   = BR_TRANSFORM_MATRIX34_LP;
    state->model_actor->material = BrMaterialFind("test_mat");
    BrMatrix34PostRotateX(&state->model_actor->t.t.mat, BR_ANGLE_DEG(45));
    BrMatrix34PostRotateY(&state->model_actor->t.t.mat, BR_ANGLE_DEG(45));

    state->light_t_actor         = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_NONE, NULL));
    state->light_t_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
    BrVector3Set(&state->light_t_actor->t.t.translate.t, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(2.0));

    state->light_actor         = BrActorAdd(state->light_t_actor, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    state->light_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
    state->light               = state->light_actor->type_data;
    state->light->colour       = BR_COLOUR_RGB(255, 255, 255);
    BrLightEnable(state->light_actor);

    state->mirror_t_actor         = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_NONE, NULL));
    state->mirror_t_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
    state->mirror_actor           = BrActorAdd(state->mirror_t_actor, BrActorAllocate(BR_ACTOR_CLIP_PLANE, NULL));
    state->mirror_actor->t.type   = BR_TRANSFORM_MATRIX34_LP;
    BrMatrix34PostRotateY(&state->mirror_actor->t.t.mat, BR_ANGLE_DEG(45));

    state->floor_actor           = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    state->floor_actor->t.type   = BR_TRANSFORM_MATRIX34_LP;
    state->floor_actor->model    = model_mirror;
    state->floor_actor->material = BrMaterialFind("tiles");
}

void UpdateMirror(br_actor *dest, br_actor *src, br_actor *world)
{
    br_matrix34 m, mi, mr, mt;

    BrMatrix34Identity(&mr);
    mr.m[2][2] = BR_SCALAR(-1);

    BrActorToActorMatrix34(&m, src, world);
    BrMatrix34Inverse(&mi, &m);

    BrMatrix34Mul(&mt, &mi, &mr);
    BrMatrix34Mul(&dest->t.t.mat, &mt, &m);
}

void BlendPixelmaps(br_pixelmap *dest, br_pixelmap *a, br_pixelmap *b, br_pixelmap *op)
{
    unsigned char *dp, *ap, *bp;
    unsigned char(*blend)[256] = (void *)op->pixels;
    int i, c = dest->width * dest->height;

    if(!dest->pixels) {
        BrPixelmapDirectLock(dest, BR_TRUE);
    }
    dp = dest->pixels;

    if(!a->pixels) {
        BrPixelmapDirectLock(a, BR_TRUE);
    }
    ap = a->pixels;

    if(!b->pixels) {
        BrPixelmapDirectLock(b, BR_TRUE);
    }
    bp = b->pixels;

    for(i = 0; i < c; i++) {
        *dp = blend[*ap][*bp];

        dp++;
        ap++;
        bp++;
    }

    if(dest->pixels) {
        BrPixelmapDirectUnlock(dest);
        dp = dest->pixels;
    }

    if(a->pixels) {
        BrPixelmapDirectUnlock(a);
        ap = a->pixels;
    }

    if(b->pixels) {
        BrPixelmapDirectUnlock(b);
        bp = b->pixels;
    }
}

const static br_demo_dispatch dispatch = {
    .init          = MirrorInit,
    .process_event = MirrorProcessEvent,
    .update        = MirrorUpdate,
    .render        = MirrorRender,
    .on_resize     = MirrorOnResize,
    .destroy       = MirrorDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRun("Mirror Demo", 1280, 720, &dispatch);
}
