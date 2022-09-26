#include <SDL.h>
#include <brender.h>
#include <brglrend.h>

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
    br_pixelmap   *screen, *colour_buffer, *depth_buffer;
    int           width, height, ret = -1;
    br_error      r;
    br_actor      *world, *camera, *cube;
    br_uint_64    ticks_last, ticks_now;


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

    if((sdl_window = SDL_CreateWindow("BRender Sample Application", 0, 0, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI)) == NULL) {
        BrLogError("SDL", "Window creation error: %s", SDL_GetError());
        goto sdl_createwindow_failed;
    }

    BrBegin();

    {
        char args[256];

        snprintf(args, BR_ASIZE(args), "WINDOW_HANDLE=0x%" PRIxPTR, (intptr_t)sdl_window);
        BrDevAddStatic(NULL, BrDrvGLBegin, args);
    }

    SDL_GL_GetDrawableSize(sdl_window, &width, &height);

    r = BrDevBeginVar(&screen, "opengl",
                      BRT_WIDTH_I32, (br_int_32)width,
                      BRT_HEIGHT_I32, (br_int_32)height,
                      BRT_PIXEL_BITS_I32, 24,
                      BR_NULL_TOKEN
    );

    if(r != BRE_OK) {
        BrLogError("APP", "BrDevBeginVar() failed.");
        goto screen_creation_failed;
    }

    if((colour_buffer = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGB_888, width, height)) == NULL) {
        BrLogError("APP", "BrPixelmapMatchTypedSized() failed.");
        goto screen_creation_failed;
    }

    if((depth_buffer = BrPixelmapMatch(colour_buffer, BR_PMMATCH_DEPTH_16)) == NULL) {
        BrLogError("APP", "BrPixelmapMatch() failed.");
        goto screen_creation_failed;
    }

    screen->origin_x = colour_buffer->origin_x = depth_buffer->origin_x = (br_int_16)(width / 2);
    screen->origin_y = colour_buffer->origin_y = depth_buffer->origin_y = (br_int_16)(height / 2);

    BrRendererBegin(colour_buffer, NULL, NULL, primitive_heap, sizeof(primitive_heap));

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

screen_creation_failed:
    BrRendererEnd();

    if(depth_buffer != NULL)
        BrPixelmapFree(depth_buffer);

    if(colour_buffer != NULL)
        BrPixelmapFree(colour_buffer);

    if(screen != NULL)
        BrPixelmapFree(screen);

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
