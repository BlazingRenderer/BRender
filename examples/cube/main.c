/*
 * BRender Cube Example
 *
 * This is meant to be a simple example of how to get a basic BRender OpenGL window
 * up and running, supporting window resizing, ALT+ENTER fullscreen, etc., with some _very_
 * basic geometry.
 *
 * If you want to make it more complex - don't.
 */
#include <SDL3/SDL.h>
#include <brender.h>
#include <brglrend.h>
#include <brsdl3dev.h>

/*
 * Primitive heap - used by z-buffered renderer to defer drawing of blended primitives
 */
static uint8_t primitive_heap[1500 * 1024];

static void create_scene(br_pixelmap *screen, br_actor **_world, br_actor **_camera, br_actor **_cube)
{
    br_actor       *world, *camera, *cube;
    br_camera      *camdata;
    br_order_table *order_table;

    BrModelFindHook(BrModelFindFailedLoad);
    BrMapFindHook(BrMapFindFailedLoad);
    BrMaterialFindHook(BrMaterialFindFailedLoad);

    world       = BrActorAllocate(BR_ACTOR_NONE, NULL);
    order_table = BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_LEAVE_BOUNDS | BR_ORDER_TABLE_BUCKET_SORT, BR_SORT_AVERAGE);
    BrZsActorOrderTableSet(world, order_table);

    camera                 = BrActorAdd(world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    camdata                = camera->type_data;
    camdata->field_of_view = BR_ANGLE_DEG(60);
    camdata->hither_z      = BR_SCALAR(0.1);
    camdata->aspect        = BR_DIV(BR_SCALAR(screen->width), BR_SCALAR(screen->height));

    order_table->min_z = camdata->hither_z;
    order_table->max_z = camdata->yon_z;

    BrMatrix34Translate(&camera->t.t.mat, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(2));

    cube           = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    cube->model    = BrModelFind("cube.dat");
    cube->material = BrMaterialFind("checkerboard.mat");

    *_world  = world;
    *_camera = camera;
    *_cube   = cube;
}

void _BrBeginHook(void)
{
    BrDevAddStatic(NULL, BrDrv1SDL3Begin, NULL);
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);
}

void _BrEndHook(void)
{
}

int main(int argc, char **argv)
{
    br_pixelmap *screen = NULL, *colour_buffer = NULL, *depth_buffer = NULL;
    int          ret = -1;
    br_error     r;
    br_actor    *world = NULL, *camera = NULL, *cube = NULL;
    br_uint_64   ticks_last, ticks_now;
    br_boolean   is_fullscreen = BR_FALSE;
    SDL_Window  *window;

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
                      BRT_WINDOW_NAME_CSTR, "BRender Cube Example",
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

    create_scene(screen, &world, &camera, &cube);

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
                    ((br_camera *)camera->type_data)->aspect = BR_DIV(BR_SCALAR(screen->width), BR_SCALAR(screen->height));
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
        }

        BrMatrix34PostRotateY(&cube->t.t.mat, BR_ANGLE_DEG(BR_SCALAR(50) * BR_SCALAR(dt)));

        BrRendererFrameBegin();
        BrPixelmapFill(colour_buffer, BR_COLOUR_RGB(66, 66, 66));
        BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
        BrZbSceneRender(world, camera, colour_buffer, depth_buffer);
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
