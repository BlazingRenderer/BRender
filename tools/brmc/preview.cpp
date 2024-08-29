#include <SDL.h>
#include <brender.h>
#include <brglrend.h>
#include <brsdl2dev.h>

/*
 * Primitive heap - used by z-buffered renderer to defer drawing of blended primitives
 */
static uint8_t primitive_heap[1500 * 1024];

static void create_scene(br_pixelmap *screen, br_actor **_world, br_actor **_camera, br_actor *root)
{
    br_actor       *world, *camera;
    br_camera      *camdata;
    br_order_table *order_table;

    BrModelFindHook(BrModelFindFailedLoad);
    BrMapFindHook(BrMapFindFailedLoad);
    BrMaterialFindHook(BrMaterialFindFailedLoad);

    world = BrActorAllocate(BR_ACTOR_NONE, NULL);
    order_table = BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_LEAVE_BOUNDS | BR_ORDER_TABLE_BUCKET_SORT, BR_SORT_AVERAGE);
    BrZsActorOrderTableSet(world, order_table);

    camera                 = BrActorAdd(world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    camdata                = (br_camera *)camera->type_data;
    camdata->field_of_view = BR_ANGLE_DEG(60);
    camdata->hither_z      = BR_SCALAR(0.1);
    camdata->yon_z         = BR_SCALAR(1000);
    camdata->aspect        = BR_DIV(BR_SCALAR(screen->width), BR_SCALAR(screen->height));

    order_table->min_z = camdata->hither_z;
    order_table->max_z = camdata->yon_z;

    BrMatrix34Translate(&camera->t.t.mat, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(100));

    *_world  = world;
    *_camera = camera;

    BrActorAdd(world, root);
    BrMatrix34RotateX(&root->t.t.mat, BR_ANGLE_DEG(-90));
    BrMatrix34PostTranslate(&root->t.t.mat, 0, -40, 0);
}

extern "C" void _BrBeginHook(void)
{
    BrDevAddStatic(NULL, BrDrv1SDL2Begin, NULL);
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);
}

extern "C" void _BrEndHook(void)
{
}

int preview(br_actor *root)
{
    br_pixelmap *screen = NULL, *colour_buffer = NULL, *depth_buffer = NULL;
    int          ret = -1;
    br_error     r;
    br_actor    *world = NULL, *camera = NULL, *cube = NULL;
    br_camera   *camdata;
    br_uint_64   ticks_last, ticks_now;
    br_boolean   is_fullscreen = BR_FALSE;
    SDL_Window  *window;

    br_int_32 mx_rel, my_rel;
    br_scalar mwheel_rel;

    /*
     * QoL for those poor X11 users.
     */
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    // BrBegin();

    // clang-format off
    r = BrDevBeginVar(&screen, "SDL2",
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

    if(BrSDLUtilResizeBuffers(screen, &colour_buffer, &depth_buffer) != BRE_OK)
        goto buffer_create_failed;

    window = BrSDLUtilGetWindow(screen);
    SDL_SetWindowFullscreen(window, 0);
    is_fullscreen = BR_FALSE;

    BrModelEnum(
        NULL,
        [](br_model *item, void *) -> br_uint_32 {
            BrModelUpdate(item, BR_MODU_ALL);
            return 0;
        },
        NULL);

    BrRendererBegin(colour_buffer, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    create_scene(screen, &world, &camera, root);
    camdata = static_cast<br_camera *>(camera->type_data);

    ticks_last = SDL_GetTicks64();

    mx_rel     = 0;
    my_rel     = 0;
    mwheel_rel = -85.0f;

    for(SDL_Event evt;;) {
        float dt;

        ticks_now  = SDL_GetTicks64();
        dt         = (float)(ticks_now - ticks_last) / 1000.0f;
        ticks_last = ticks_now;

        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_QUIT:
                    goto done;
                case SDL_WINDOWEVENT:
                    /*
                     * Window event, pass it to the driver.
                     */
                    if(BrPixelmapHandleWindowEvent(screen, &evt.window) != BRE_OK) {
                        BrLogError("APP", "Error handling window event");
                        goto buffer_create_failed;
                    }

                    switch(evt.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            /*
                             * The main screen should have been resized above.
                             * Update its origin and resize the framebuffer.
                             */
                            screen->origin_x = (br_int_16)(screen->width / 2);
                            screen->origin_y = (br_int_16)(screen->height / 2);

                            if(BrSDLUtilResizeBuffers(screen, &colour_buffer, &depth_buffer) != BRE_OK) {
                                BrLogError("APP", "Error resizing window buffers");
                                goto buffer_create_failed;
                            }

                            /*
                             * Update the camera's aspect ratio to match the camera.
                             */
                            ((br_camera *)camera->type_data)->aspect = BR_DIV(BR_SCALAR(screen->width),
                                                                              BR_SCALAR(screen->height));
                            break;
                    }
                    break;
                case SDL_KEYDOWN: {
                    if(BrSDLUtilIsAltEnter(&evt.key)) {
                        if(is_fullscreen) {
                            SDL_SetWindowFullscreen(window, 0);
                        } else {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                        is_fullscreen = !is_fullscreen;
                        break;
                    }

                    if(evt.key.keysym.sym == SDLK_SPACE) {
                        mx_rel     = 0;
                        my_rel     = 0;
                        mwheel_rel = 85.0f;
                    }
                    break;
                }
                case SDL_MOUSEMOTION:
                    mx_rel += evt.motion.xrel;
                    my_rel += evt.motion.yrel;
                    break;
                case SDL_MOUSEWHEEL:
                    mwheel_rel += evt.wheel.preciseY;
                    break;
            }
        }

        br_scalar xdiff = BR_DIV(BR_SCALAR(mx_rel), BR_SCALAR(screen->width));
        br_scalar ydiff = BR_DIV(BR_SCALAR(my_rel), BR_SCALAR(screen->height));
        if(-mwheel_rel < 1.0f)
            mwheel_rel = -1.0f;

        fprintf(stderr, "%d, %d, %f\n", mx_rel, my_rel, mwheel_rel);
        BrMatrix34Identity(&camera->t.t.mat);

        BrMatrix34PostTranslate(&camera->t.t.mat, 0, 0, -mwheel_rel);
        BrMatrix34PostRotateY(&camera->t.t.mat, BR_ANGLE_DEG(fmodf(-xdiff * 1000, 360)));
        BrMatrix34PostRotateX(&camera->t.t.mat, BR_ANGLE_DEG(fmodf(-ydiff * 1000, 360)));

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

    // BrEnd();
    return ret;
}
