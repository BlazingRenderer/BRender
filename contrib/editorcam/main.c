/*
 * Editor/Unity-like Camera for BRender
 * https://github.com/BlazingRenderer/BRender
 *
 * SPDX-License-Identifier: MIT
 * Copyright(c) 2025 Zane van Iperen
 */
#include <brdemo.h>
#include "editorcam.h"

static br_error EditorCamLoadHWRes(br_demo *demo)
{
    return BRE_OK;
}

static br_error EditorCamLoadSWRes(br_demo *demo)
{
    br_pixelmap *std_pal;
    br_pixelmap *shade_tab;

    if((std_pal = BrPixelmapLoad("/home/zane/Documents/Coding/CrocDE-BRender/examples/dat/std.pal")) == NULL) {
        BrLogError("DEMO", "Unable to load std.pal");
        return BRE_FAIL;
    }

    if((shade_tab = BrPixelmapLoad("/home/zane/Documents/Coding/CrocDE-BRender/examples/dat/shade.tab")) == NULL) {
        BrLogError("DEMO", "Unable to load shade.tab");
        return BRE_FAIL;
    }
    BrTableAdd(shade_tab);

    /*
     * Indexed targets need a palette.
     */
    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        BrPixelmapPaletteSet(demo->colour_buffer, std_pal);
    }

    return BRE_OK;
}

static br_error EditorCamInit(br_demo *demo)
{
    br_actor         *obj, *light;
    br_editor_camera *cam;
    br_error          err;

    if((err = demo->hw_accel ? EditorCamLoadHWRes(demo) : EditorCamLoadSWRes(demo)) != BRE_OK)
        return err;

    cam          = BrEditorCamAllocate(demo);
    demo->user   = cam;
    demo->camera = cam->camera;
    BrActorAdd(demo->world, cam->actor);

    // cam->camera_data.yon_z = 10000.0f;

    demo->order_table->min_z = cam->camera_data.hither_z;
    demo->order_table->max_z = cam->camera_data.yon_z;

    //
    // /*
    //  * Add a box at origin.
    //  */
    // obj         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    // obj->t.type = BR_TRANSFORM_MATRIX34;
    //
    /*
     * Add a directional light and switch it on.
     */
    light                                = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    ((br_light *)light->type_data)->type = BR_LIGHT_DIRECT;

    //((br_light*)light->type_data)->attenuation_c = 0.5f;
    BrLightEnable(light);

    /*
     * Our scene is small, drop the camera's speed to 10 (from 50).
     */
    cam->standard_speed = BR_SCALAR(10);

    br_model *ship = BrModelLoad("/home/zane/Documents/Coding/CrocDE-BRender/examples/dat/trade-federation-ship.dat");
    BrModelUpdate(ship, BR_MODU_ALL);

    br_actor *shipa = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    shipa->model    = ship;

    br_material *mat = BrMaterialAllocate("ship");
    // mat->flags &= ~(BR_MATF_LIGHT);
    mat->flags = 0
               | BR_MATF_PRELIT
               //| BR_MATF_LIGHT
               | BR_MATF_SMOOTH
               //| BR_MATF_SUBDIVIDE
               | BR_MATF_PERSPECTIVE
    ;

    // mat->subdivide_tolerance = 150;
    BrMaterialUpdate(mat, BR_MATU_ALL);

    shipa->material = mat;

    shipa->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Scale(&shipa->t.t.mat, 0.05f, 0.05f, 0.05f);

    return BRE_OK;
}

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

static void EditorCamProcessEvent(br_demo *demo, const SDL_Event *evt)
{
    br_editor_camera *cam = demo->user;

    switch(evt->type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            switch(evt->key.key) {
                case SDLK_SPACE:
                    BrEditorCamReset(cam);
                    BrMatrix34Translate(&cam->actor->t.t.mat, 0, 0, 5);
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

static void EditorCamUpdate(br_demo *demo, float dt)
{
    br_editor_camera *cam = demo->user;
    BrEditorCamUpdate(cam, dt);
}

static void EditorCamOnResize(br_demo *demo, br_uint_16 width, br_uint_16 height)
{
    br_editor_camera *cam = demo->user;
    BrEditorCamSetViewportSize(cam, width, height);
}

static void EditorCamRender(br_demo *demo)
{
    br_editor_camera *cam = demo->user;

    BrRendererFrameBegin();
    BrPixelmapFill(demo->colour_buffer, demo->clear_colour);
    BrPixelmapFill(demo->depth_buffer, 0xFFFFFFFF);

    // BrZsSceneRender(demo->world, demo->camera, demo->colour_buffer);
    BrZbSceneRender(demo->world, demo->camera, demo->colour_buffer, demo->depth_buffer);

    {
        br_int_32 text_height = BrPixelmapTextHeight(demo->colour_buffer, BrFontProp7x9);
        br_int_32 base_x      = -demo->colour_buffer->origin_x + 5;
        br_int_32 base_y      = -demo->colour_buffer->origin_y + 5 + (text_height * 2);

        static const char *usage_lines[] = {
            "Usage:",
            "  Esc          = Quit",
            "  W/S/A/D = Forward/Back/Left/Right",
            "  Q            = Move Down",
            "  E             = Move Up",
            "  Left Shift  = Speed Up",
            "  RMB         = Look Mode",
            "  MMB         = Pan Mode",
            "  Scroll       = +/- Zoom",
            "  Space       = Reset View",
        };

        for(size_t i = 0; i < BR_ASIZE(usage_lines); ++i) {
            base_y += text_height * 2;
            BrPixelmapText(demo->colour_buffer, base_x, base_y, demo->text_colour, BrFontProp7x9, usage_lines[i]);
        }

        base_y += text_height * 2;
        base_y += text_height * 2;
        base_y += text_height * 2;

        BrPixelmapTextF(demo->colour_buffer, base_x, base_y, demo->text_colour, BrFontProp7x9, "Current Speed: %f", cam->current_speed);
    }

    BrRendererFrameEnd();
}

static void EditorCamDestroy(br_demo *demo)
{
    br_editor_camera *cam = demo->user;
    BrEditorCamFree(cam);
    demo->user = NULL;
}

const static br_demo_dispatch dispatch = {
    .init          = EditorCamInit,
    .process_event = EditorCamProcessEvent,
    .update        = EditorCamUpdate,
    .render        = EditorCamRender,
    .on_resize     = EditorCamOnResize,
    .destroy       = EditorCamDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("EditorCam Demo", &dispatch, argc, argv);
}
