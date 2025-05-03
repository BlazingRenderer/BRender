/*
 * Editor/Unity-like Camera for BRender
 * https://github.com/BlazingRenderer/BRender
 *
 * SPDX-License-Identifier: MIT
 * Copyright(c) 2025 Zane van Iperen
 */
#include <brdemo.h>
#include "editorcam.h"

static br_error EditorCamInit(br_demo *demo)
{
    br_actor         *obj, *light;
    br_editor_camera *cam;

    cam          = BrEditorCamAllocate(demo);
    demo->user   = cam;
    demo->camera = cam->camera;
    BrActorAdd(demo->world, cam->actor);

    demo->order_table->min_z = cam->camera_data.hither_z;
    demo->order_table->max_z = cam->camera_data.yon_z;

    /*
     * Add a box at origin.
     */
    obj         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    obj->t.type = BR_TRANSFORM_MATRIX34;

    /*
     * Add a directional light and switch it on.
     */
    light                                = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    ((br_light *)light->type_data)->type = BR_LIGHT_DIRECT;
    BrLightEnable(light);

    /*
     * Move the camera back a bit so it can see the cube.
     */
    BrMatrix34Translate(&cam->actor->t.t.mat, 0, 0, 5);

    /*
     * Our scene is small, drop the camera's speed to 10 (from 50).
     */
    cam->standard_speed = BR_SCALAR(10);
    return BRE_OK;
}

typedef struct KeyMap {
    SDL_Keycode key;
    br_uint_32  flag;
} KeyMap;

const static KeyMap keymap[] = {
    {.key = SDLK_w,      .flag = BR_ECAMF_MOVE_FORWARD},
    {.key = SDLK_s,      .flag = BR_ECAMF_MOVE_BACK   },
    {.key = SDLK_a,      .flag = BR_ECAMF_MOVE_LEFT   },
    {.key = SDLK_d,      .flag = BR_ECAMF_MOVE_RIGHT  },
    {.key = SDLK_q,      .flag = BR_ECAMF_MOVE_DOWN   },
    {.key = SDLK_e,      .flag = BR_ECAMF_MOVE_UP     },
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
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            switch(evt->key.keysym.sym) {
                case SDLK_SPACE:
                    BrEditorCamReset(cam);
                    BrMatrix34Translate(&cam->actor->t.t.mat, 0, 0, 5);
                    break;

                case SDLK_ESCAPE:
                    SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});
                    break;

                default:
                    break;
            }

            for(size_t i = 0; i < BR_ASIZE(keymap); ++i) {
                if(evt->key.keysym.sym == keymap[i].key) {
                    if(evt->type == SDL_KEYDOWN)
                        cam->flags |= keymap[i].flag;

                    if(evt->type == SDL_KEYUP)
                        cam->flags &= ~keymap[i].flag;

                    break;
                }
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            for(size_t i = 0; i < BR_ASIZE(buttonmap); ++i) {
                if(evt->button.button == buttonmap[i].button) {
                    if(evt->button.state == SDL_PRESSED)
                        cam->flags |= buttonmap[i].flag;

                    if(evt->button.state == SDL_RELEASED)
                        cam->flags &= ~buttonmap[i].flag;
                }
            }
            break;

        case SDL_MOUSEMOTION:
            cam->mx_rel += evt->motion.xrel;
            cam->my_rel += evt->motion.yrel;
            break;

        case SDL_MOUSEWHEEL:
            cam->mwheel_rel += BR_SCALAR(evt->wheel.preciseY / 4.0f);
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

        BrPixelmapTextF(demo->colour_buffer, base_x, base_y, demo->text_colour, BrFontProp7x9, "Current Speed: %f",
                        cam->current_speed);
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
