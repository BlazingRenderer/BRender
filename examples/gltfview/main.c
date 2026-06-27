#include <SDL3/SDL.h>
#include <brender.h>
#include <brdemo.h>
#include "editorcam.h"

typedef struct br_gltfview_state {
    br_editor_camera *editorcam;
} br_gltfview_state;

typedef struct KeyMap {
    SDL_Keycode key;
    br_uint_32  flag;
} KeyMap;

const static KeyMap keymap[] = {
    {.key = SDLK_W,      .flag = BR_ECAMF_MOVE_FORWARD},
    {.key = SDLK_S,      .flag = BR_ECAMF_MOVE_BACK   },
    {.key = SDLK_A,      .flag = BR_ECAMF_MOVE_LEFT   },
    {.key = SDLK_D,      .flag = BR_ECAMF_MOVE_RIGHT  },
    {.key = SDLK_Q,      .flag = BR_ECAMF_MOVE_DOWN   },
    {.key = SDLK_E,      .flag = BR_ECAMF_MOVE_UP     },
    {.key = SDLK_LSHIFT, .flag = BR_ECAMF_FASTER      },
};

typedef struct ButtonMap {
    Uint8      button;
    br_uint_32 flag;
} ButtonMap;

const static ButtonMap buttonmap[] = {
    {.button = SDL_BUTTON_MIDDLE, .flag = BR_ECAMF_MODE_PAN },
    {.button = SDL_BUTTON_RIGHT,  .flag = BR_ECAMF_MODE_LOOK},
};

typedef struct FindCameraCtx {
    br_actor         *found;
    br_editor_camera *exclude;
} FindCameraCtx;

static br_uint_32 BR_CALLBACK FindFirstCameraCallback(br_actor *a, void *arg)
{
    FindCameraCtx *ctx = arg;

    if(ctx->found != nullptr)
        return 1;

    if(a == ctx->exclude->actor || a == ctx->exclude->camera)
        return 0;

    if(a->type == BR_ACTOR_CAMERA) {
        ctx->found = a;
        return 1;
    }

    BrActorEnum(a, FindFirstCameraCallback, arg);
    return 0;
}

static void FocusCamOnSceneCamera(br_editor_camera *ecam, br_actor *world)
{
    FindCameraCtx ctx = {.found = nullptr, .exclude = ecam};
    BrActorEnum(world, FindFirstCameraCallback, &ctx);
    br_actor *scene_cam = ctx.found;

    if(scene_cam == NULL) {
        BrEditorCamReset(ecam);
        return;
    }

    br_camera  *scdata = scene_cam->type_data;
    br_matrix34 world_mat;

    BrActorToActorMatrix34(&world_mat, scene_cam, world);

    BrMatrix34Identity(&ecam->actor->t.t.mat);
    ecam->actor->t.t.mat.m[3][0] = world_mat.m[3][0];
    ecam->actor->t.t.mat.m[3][1] = world_mat.m[3][1];
    ecam->actor->t.t.mat.m[3][2] = world_mat.m[3][2];

    BrMatrix34Copy(&ecam->camera->t.t.mat, &world_mat);
    ecam->camera->t.t.mat.m[3][0] = BR_SCALAR(0);
    ecam->camera->t.t.mat.m[3][1] = BR_SCALAR(0);
    ecam->camera->t.t.mat.m[3][2] = BR_SCALAR(0);

    ecam->camera_data.field_of_view = scdata->field_of_view;
    // ecam->camera_data.hither_z      = scdata->hither_z;
    // ecam->camera_data.yon_z         = scdata->yon_z;
}

static void EditorCamProcessEvent(br_demo *demo, br_editor_camera *cam, const SDL_Event *evt)
{
    switch(evt->type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            switch(evt->key.key) {
                case SDLK_SPACE:
                    FocusCamOnSceneCamera(cam, demo->world);
                    break;

                case SDLK_ESCAPE:
                    SDL_PushEvent(&(SDL_Event){.type = SDL_EVENT_QUIT});
                    break;

                default:
                    break;
            }

            for(size_t i = 0; i < BR_ASIZE(keymap); ++i) {
                if(evt->key.key == keymap[i].key) {
                    if(evt->type == SDL_EVENT_KEY_DOWN)
                        cam->flags |= keymap[i].flag;

                    if(evt->type == SDL_EVENT_KEY_UP)
                        cam->flags &= ~keymap[i].flag;

                    break;
                }
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            for(size_t i = 0; i < BR_ASIZE(buttonmap); ++i) {
                if(evt->button.button == buttonmap[i].button) {
                    if(evt->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                        cam->flags |= buttonmap[i].flag;

                    if(evt->type == SDL_EVENT_MOUSE_BUTTON_UP)
                        cam->flags &= ~buttonmap[i].flag;
                }
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            cam->mx_rel += evt->motion.xrel;
            cam->my_rel += evt->motion.yrel;
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            cam->mwheel_rel += BR_SCALAR(evt->wheel.y / 4.0f);
            break;

        default:
            break;
    }
}

static br_pixelmap *build_clut(br_pixelmap *const *maps, size_t nmaps)
{
    br_pixelmap *clut;

    clut = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL);
    BrPixelmapPixelSet(clut, 0, 0, 0);

    BrQuantBegin();
    for(size_t i = 0; i < nmaps; ++i) {
        br_pixelmap *pm = maps[i];

        for(int y = 0; y < pm->height; y++) {
            for(int x = 0; x < pm->width; x++) {
                /* Assumes we're BR_PMT_RGBA_8888. */
                br_colour col = BrPixelmapPixelGet(pm, -pm->origin_x + x, -pm->origin_y + y);

                br_uint_8 rgb[3] = {
                    [0] = BR_RED(col),
                    [1] = BR_GRN(col),
                    [2] = BR_BLU(col),
                };

                BrQuantAddColours(rgb, 1);
            }
        }
    }

    BrQuantMakePalette(1, 255, clut);
    BrQuantEnd();

    return clut;
}

static br_error GLTFInit(br_demo *demo)
{
    br_fmt_results    *results;
    const char        *filename = demo->args->pos_argv[0];
    br_gltfview_state *state    = BrResAllocate(demo, sizeof(br_gltfview_state), BR_MEMORY_APPLICATION);
    demo->user                  = state;

    if(filename == NULL || filename[0] == '\0') {
        BrLogError("APP", "No GLTF filename passed");
        return BRE_FAIL;
    }

    br_gltf_options opts = {
        .pm_type = demo->colour_buffer->type,
    };

    if(demo->hw_accel) {
        opts.pm_type = BR_PMT_RGBA_8888;
        // opts.pm_type = BR_PMT_INDEX_8;
    } else {
        switch(demo->colour_buffer->type) {
            case BR_PMT_INDEX_8:
                /*
                 * We build an optimal CLUT below.
                 */
                opts.pm_type = BR_PMT_RGBA_8888;
                break;
            case BR_PMT_RGB_565:
            case BR_PMT_RGB_555:
                opts.pm_type = BR_PMT_INDEX_8;
                break;

            default:
                break;
        }
    }

    results = BrFmtGLTFActorLoadMany(filename, &opts);
    if(!results || results->nactors == 0) {
        BrLogError("APP", "Failed to load %s", filename);
        return BRE_FAIL;
    }

    for(br_size_t i = 0; i < results->nactors; ++i) {
        BrActorAdd(demo->world, results->actors[i]);
    }

    /*
     * Build an optimal CLUT for all our pixelmaps.
     * This only matters if our backbuffer is BR_PMT_INDEX_8, but it doesn't hurt to do.
     */
    br_pixelmap *clut = build_clut(results->pixelmaps, results->npixelmaps);
    BrPixelmapPaletteSet(demo->colour_buffer, clut);

    for(size_t i = 0; i < results->npixelmaps; ++i) {
        br_pixelmap *pm = results->pixelmaps[i];

        if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
            /*
             * If our destination is indexed, re-quantise everything to our palette
             * by doing a conversion then cannibalising its parts.
             */
            const br_pixelmap_convert_options cvtopts = {
                .index_alpha_threshold = 0,
                .target_clut           = clut,
            };
            br_pixelmap *tmp = BrPixelmapConvert(pm, BR_PMT_INDEX_8, &cvtopts);

            pm->type      = BR_PMT_INDEX_8;
            pm->row_bytes = tmp->row_bytes;

            BrResFree(pm->pixels);
            pm->pixels = BrResAdd(pm, BrResRemove(tmp->pixels));

            if(pm->map != NULL)
                BrResFree(pm->map);

            pm->map = clut;

            BrPixelmapFree(tmp);
        } else {
            if(pm->map != NULL) {
                BrTableUpdate(pm->map, BR_TABU_ALL);
            }
        }

        BrMapUpdate(pm, BR_MAPU_ALL);
    }

    for(br_size_t i = 0; i < results->nmaterials; ++i) {
        BrMaterialUpdate(results->materials[i], BR_MATU_ALL);
    }

    for(br_size_t i = 0; i < results->nmodels; ++i) {
        if(results->models[i] != NULL) {
            BrModelUpdate(results->models[i], BR_MODU_ALL);
        }
    }

    state->editorcam                 = BrEditorCamAllocate(state);
    state->editorcam->standard_speed = 2.0f;
    demo->camera                     = state->editorcam->camera;
    BrActorAdd(demo->world, state->editorcam->actor);

    demo->order_table->min_z = state->editorcam->camera_data.hither_z;
    demo->order_table->max_z = state->editorcam->camera_data.yon_z;

    FocusCamOnSceneCamera(state->editorcam, demo->world);

    return BRE_OK;
}

static void GLTFOnResize(br_demo *demo, br_uint_16 width, br_uint_16 height)
{
    br_gltfview_state *state = demo->user;
    BrEditorCamSetViewportSize(state->editorcam, width, height);
}

static void GLTFProcessEvent(br_demo *demo, const SDL_Event *evt)
{
    br_gltfview_state *state = demo->user;
    EditorCamProcessEvent(demo, state->editorcam, evt);
}

static void GLTFUpdate(br_demo *demo, br_scalar dt)
{
    br_gltfview_state *state = demo->user;
    BrEditorCamUpdate(state->editorcam, dt);
}

static void GLTFRender(br_demo *demo)
{
    BrRendererFrameBegin();
    BrPixelmapFill(demo->colour_buffer, demo->clear_colour);
    BrPixelmapFill(demo->depth_buffer, 0xFFFFFFFF);

    // BrZsSceneRender(demo->world, demo->camera, demo->colour_buffer);
    BrZbSceneRender(demo->world, demo->camera, demo->colour_buffer, demo->depth_buffer);

    BrRendererFrameEnd();
}

const static br_demo_dispatch dispatch = {
    .init          = GLTFInit,
    .process_event = GLTFProcessEvent,
    .update        = GLTFUpdate,
    .render        = GLTFRender,
    .on_resize     = GLTFOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("BRender GLTF Viewer", &dispatch, argc, argv);
}
