#include <SDL3/SDL.h>
#include <brender.h>
#include <brglrend.h>
#include <brsdl3dev.h>
#include "editorcam.h"

/*
 * Primsitive heap - used by z-buffered renderer to defer drawing of blended primitives
 */
static uint8_t primitive_heap[1500 * 1024];

static void create_scene(br_actor *world, const char *file)
{
    br_fmt_results *results;

    // Works
    // results = BrFmtGLTFActorLoadMany("/dev/glTF-Sample-Assets/Models/TriangleWithoutIndices/glTF-Embedded/TriangleWithoutIndices.gltf");
    // results = BrFmtGLTFActorLoadMany("/dev/glTF-Sample-Assets/Models/Triangle/glTF-Embedded/Triangle.gltf");
    // results = BrFmtGLTFActorLoadMany("/dev/glTF-Sample-Assets/Models/SimpleMeshes/glTF-Embedded/SimpleMeshes.gltf");
    // results = BrFmtGLTFActorLoadMany("/dev/glTF-Sample-Assets/Models/MeshPrimitiveModes/glTF-Embedded/MeshPrimitiveModes.gltf");
    // results = BrFmtGLTFActorLoadMany("/dev/glTF-Sample-Assets/Models/Avocado/glTF-Binary/Avocado.glb");

    results = BrFmtGLTFActorLoadMany(file);


    for(br_size_t i = 0; i < results->nactors; ++i) {
        BrActorAdd(world, results->actors[i]);
    }

    for (size_t i = 0; i < results->npixelmaps; ++i) {
        BrMapUpdate(results->pixelmaps[i], BR_MAPU_ALL);
    }

    for(br_size_t i = 0; i < results->nmaterials; ++i) {
        BrMaterialUpdate(results->materials[i], BR_MATU_ALL);
    }

    for(br_size_t i = 0; i < results->nmodels; ++i) {
        if(results->models[i] != NULL) {
            BrModelUpdate(results->models[i], BR_MODU_ALL);
        }
    }
}

void _BrBeginHook(void)
{
    BrDevAddStatic(NULL, BrDrv1SDL3Begin, NULL);
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);
}

void _BrEndHook(void)
{
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

static void EditorCamProcessEvent(br_editor_camera *cam, const SDL_Event *evt)
{
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

int main(int argc, char **argv)
{
    br_pixelmap      *screen = NULL, *colour_buffer = NULL, *depth_buffer = NULL;
    int               ret = -1;
    br_error          r;
    br_actor         *world = NULL;
    br_uint_64        ticks_last, ticks_now;
    br_boolean        is_fullscreen = BR_FALSE;
    SDL_Window       *window;
    br_editor_camera *editorcam;

    /*
     * QoL for those poor X11 users.
     */
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    BrBegin();

    // clang-format off
    r = BrDevBeginVar(&screen, "SDL3",
                      BRT_WIDTH_I32,        1280,
                      BRT_HEIGHT_I32,       720,
                      BRT_PIXEL_TYPE_U8,    BR_PMT_RGB_888,
                      BRT_WINDOW_NAME_CSTR, "BRender GLTF Viewer",
                      BRT_HIDPI_B,          BR_TRUE,
                      BRT_RESIZABLE_B,      BR_TRUE,
                      BRT_OPENGL_B,         BR_TRUE,
                      BR_NULL_TOKEN);
    // clang-format on
    if(r != BRE_OK)
        goto screen_creation_failed;

    screen->origin_x = (br_int_16)(screen->width / 2);
    screen->origin_y = (br_int_16)(screen->height / 2);

    if(BrPixelmapResizeBuffers(screen, &colour_buffer, &depth_buffer) != BRE_OK)
        goto buffer_create_failed;

    window = BrSDL3UtilGetWindow(screen);
    SDL_SetWindowFullscreen(window, 0);
    is_fullscreen = BR_FALSE;

    BrRendererBegin(colour_buffer, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    editorcam = BrEditorCamAllocate(screen);
    editorcam->standard_speed = 2.0f;

    world = BrActorAllocate(BR_ACTOR_NONE, NULL);
    BrActorAdd(world, editorcam->actor);

    create_scene(world, "/home/zane/.local/share/Croc/croc-scene.gltf");

//    BrWriteModeSet(BR_FS_MODE_TEXT);
//    BrActorSave("xxxx.act", world);

    ticks_last = SDL_GetTicksNS();

    for(SDL_Event evt;;) {
        float dt;

        ticks_now  = SDL_GetTicksNS();
        dt         = (float)(ticks_now - ticks_last) / 1e9f;
        ticks_last = ticks_now;

        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_EVENT_QUIT:
                    goto done;
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    /*
                     * Window event, pass it to the driver.
                     */
                    if(BrPixelmapHandleWindowEvent(screen, &evt.window) != BRE_OK) {
                        BrLogError("APP", "Error handling window event");
                        goto buffer_create_failed;
                    }

                    /*
                     * The main screen should have been resized above.
                     * Update its origin and resize the framebuffer.
                     */
                    screen->origin_x = (br_int_16)(screen->width / 2);
                    screen->origin_y = (br_int_16)(screen->height / 2);

                    if(BrPixelmapResizeBuffers(screen, &colour_buffer, &depth_buffer) != BRE_OK) {
                        BrLogError("APP", "Error resizing window buffers");
                        goto buffer_create_failed;
                    }

                    /*
                     * Update the camera's aspect ratio to match the camera.
                     */
                    BrEditorCamSetViewportSize(editorcam, screen->width, screen->height);
                    break;
                case SDL_EVENT_KEY_DOWN: {
                    if(BrSDL3UtilIsAltEnter(&evt.key)) {
                        if(is_fullscreen) {
                            SDL_SetWindowFullscreen(window, 0);
                        } else {
                            SDL_SetWindowFullscreen(window, 1);
                        }
                        is_fullscreen = !is_fullscreen;
                        break;
                    }
                    break;
                }
            }

            EditorCamProcessEvent(editorcam, &evt);
        }

        BrEditorCamUpdate(editorcam, dt);

        BrRendererFrameBegin();
        BrPixelmapFill(colour_buffer, 0);
        BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
        BrZbSceneRender(world, editorcam->camera, colour_buffer, depth_buffer);
        BrRendererFrameEnd();

        BrPixelmapDoubleBuffer(screen, colour_buffer);
    }

done:
    ret = 0;

    BrRendererEnd();

    BrPixelmapFree(depth_buffer);
    BrPixelmapFree(colour_buffer);

buffer_create_failed:
    BrPixelmapFree(screen);

screen_creation_failed:

    BrEnd();
    return ret;
}
