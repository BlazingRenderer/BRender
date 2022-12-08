#include <SDL.h>
#include <brender.h>
#include <brsdl.h>
#include <inttypes.h>

/*
 * Primitive heap - used by z-buffered renderer to defer drawing of blended primitives
 */
static uint8_t primitive_heap[1500 * 1024];

static void create_scene(br_actor **_world, br_actor **_camera, br_actor **_cube)
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

    camera  = BrActorAdd(world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    camdata = camera->type_data;
    camdata->field_of_view = BR_ANGLE_DEG(60);
    camdata->hither_z      = BR_SCALAR(0.1);
    camdata->aspect        = BR_SCALAR(1280) / BR_SCALAR(720);

    order_table->min_z = camdata->hither_z;
    order_table->max_z = camdata->yon_z;

    BrMatrix34Translate(&camera->t.t.mat, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(2));

    cube = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    cube->model    = BrModelFind("cube.dat");
    cube->material = BrMaterialFind("checkerboard.mat");

    *_world  = world;
    *_camera = camera;
    *_cube   = cube;
}

int main(int argc, char **argv)
{
    SDL_Window    *sdl_window;
    br_pixelmap   *screen = NULL, *colour_buffer = NULL, *depth_buffer = NULL;
    int           ret = -1;
    br_error      r;
    br_actor      *world = NULL, *camera = NULL, *cube = NULL;
    br_uint_64    ticks_last, ticks_now;
    br_boolean    is_fullscreen = BR_FALSE;


    /*
     * Init SDL
     */
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        BrLogError("SDL", "Initialisation error: %s", SDL_GetError());
        goto sdl_init_failed;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    if((sdl_window = SDL_CreateWindow("BRender Sample Application", 0, 0, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE)) == NULL) {
        BrLogError("SDL", "Window creation error: %s", SDL_GetError());
        goto sdl_createwindow_failed;
    }

    SDL_SetWindowMinimumSize(sdl_window, 320, 200);

    BrBegin();

    BrSDLDevAddStaticGL(sdl_window);

    r = BrSDLUtilOnResize(sdl_window, "opengl", &screen, &colour_buffer, &depth_buffer, primitive_heap, sizeof(primitive_heap), NULL);
    if(r != BR_TRUE)
        goto screen_creation_failed;

    is_fullscreen = BrSDLUtilSetFullscreen(sdl_window, BR_FALSE);

    create_scene(&world, &camera, &cube);

    ticks_last = SDL_GetTicks64();

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
                    switch(evt.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            (void)BrSDLUtilOnResize(sdl_window, "opengl", &screen, &colour_buffer, &depth_buffer, primitive_heap, sizeof(primitive_heap), camera->type_data);
                            break;
                    }
                    break;
                case SDL_KEYDOWN: {
                    if(BrSDLUtilIsAltEnter(&evt.key)) {
                        is_fullscreen = BrSDLUtilSetFullscreen(sdl_window, !is_fullscreen);
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

    BrSDLUtilCleanupScreen(&screen, &colour_buffer, &depth_buffer);

screen_creation_failed:

    BrEnd();

    SDL_DestroyWindow(sdl_window);


sdl_createwindow_failed:
    /*
     * Close down SDL
     */
    SDL_Quit();

sdl_init_failed:
    return ret;
}
