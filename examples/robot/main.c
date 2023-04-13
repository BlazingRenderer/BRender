/*
 * BRender Robot Example
 *
 * A rewrite of the classic robot example.
 */
#include <SDL.h>
#include <brender.h>
#include <brglrend.h>
#include <brsdl2dev.h>

#define MAXFRAMES     100
#define MAXTRANSFORMS 2000

const float ANIM_FPS   = 1.0f / 30.0f;
const float CAM_XSPEED = 3.0f;
const float CAM_YSPEED = 3.0f;

typedef struct brrobot_world {
    br_actor *root;
    br_actor *camera;
    br_actor *camera_pivot;
    br_actor *light;
    br_actor *actor;

    br_vue *vue;
} brrobot_world;

/*
 * Primitive heap - used by z-buffered renderer to defer drawing of blended primitives
 */
static uint8_t primitive_heap[1500 * 1024];

void _BrBeginHook(void)
{
    BrLogSetLevel(BR_LOG_TRACE);

    BrDevAddStatic(NULL, BrDrv1SDL2Begin, NULL);
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);
}

void _BrEndHook(void)
{
}

/*
 * Custom load function to de-CLUT a pixelmap upon loading.
 */
static br_pixelmap *BR_CALLBACK map_find_failed_load(const char *name)
{
    br_pixelmap *pm, *pm2;

    if((pm = BrPixelmapLoad(name)) != NULL) {
        if((pm2 = BrPixelmapDeCLUT(pm)) != NULL) {
            BrResFree(pm->map);
            BrResFree(pm);
            pm = pm2;
        }

        if(pm->identifier != NULL)
            BrResFree(pm->identifier);

        pm->identifier = BrResStrDup(pm, name);
        BrMapAdd(pm);
    }

    return pm;
}

brrobot_world *world_allocate(void)
{
    brrobot_world *world;
    br_camera     *camera_data;
    br_light      *light_data;
    br_actor      *a;
    br_material   *materials[20];
    br_uint_32     nmaterials;
    br_model      *models[50];
    br_uint_32     nmodels;

    /*
     * Configure our load hooks.
     * The OpenGL renderer doesn't support indexed textures, so de-CLUT them
     * upon load.
     */
    BrTableFindHook(BrTableFindFailedLoad);
    BrMapFindHook(map_find_failed_load);

    if((nmaterials = BrFmtScriptMaterialLoadMany("robo.msc", materials, BR_ASIZE(materials))) == 0) {
        BrLogError("ROBOT", "Unable to load robo.msc");
        return NULL;
    }
    BrLogInfo("ROBOT", "Loaded %u materials from robo.msc", nmaterials);

    BrMaterialAddMany(materials, nmaterials);

    if((nmodels = BrModelLoadMany("robo.dat", models, BR_ASIZE(models))) == 0) {
        BrLogError("ROBOT", "Unable to load robo.dat");
        return NULL;
    }
    BrLogInfo("ROBOT", "Loaded %u models from robo.dat", nmodels);

    /*
     * HACKFIX: Set this so BrModelUpdate() doesn't choke.
     * Figure out why.
     */
    for(int i = 0; i < nmodels; ++i)
        models[i]->flags |= BR_MODF_UPDATEABLE;

    BrModelAddMany(models, nmodels);

    /*
     * Allocate the world and resources.
     */
    world        = BrResAllocate(NULL, sizeof(brrobot_world), BR_MEMORY_APPLICATION);
    world->root  = BrActorAllocate(BR_ACTOR_NONE, NULL);
    world->actor = BrActorAdd(world->root, BrActorAllocate(BR_ACTOR_NONE, NULL));

    if((a = BrActorLoad("robo.act")) == NULL) {
        BrLogError("ROBOT", "Unable to load robo.act");
        BrResFree(world);
        return NULL;
    }
    BrActorAdd(world->actor, a);

    BrLogInfo("ROBOT", "Loaded robo.act");

    world->vue = BrVueAllocate(MAXFRAMES, MAXTRANSFORMS);
    BrResAdd(world, world->vue);

    BrLoadVUE("robo.vue", a, world->vue);

    /*
     * Turn to face camera
     */
    BrMatrix34RotateX(&world->actor->t.t.mat, BR_ANGLE_DEG(-90.0));

    /*
     * Camera
     */
    world->camera_pivot = BrActorAdd(world->root, BrActorAllocate(BR_ACTOR_NONE, NULL));
    world->camera       = BrActorAdd(world->camera_pivot, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    camera_data         = world->camera->type_data;

    camera_data->type          = BR_CAMERA_PERSPECTIVE_OLD;
    camera_data->field_of_view = BR_ANGLE_DEG(45.0);
    camera_data->hither_z      = BR_SCALAR(50.0);
    camera_data->yon_z         = BR_SCALAR(3000.0);
    camera_data->aspect        = BR_SCALAR(1.8);

    BrMatrix34Translate(&world->camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(1700));

    /*
     * Light
     */
    world->light              = BrActorAdd(world->root, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    light_data                = world->light->type_data;
    light_data->type          = BR_LIGHT_DIRECT;
    light_data->attenuation_c = BR_SCALAR(1.0);

    BrMatrix34RotateY(&world->light->t.t.mat, BR_ANGLE_DEG(45.0));
    BrMatrix34PostRotateZ(&world->light->t.t.mat, BR_ANGLE_DEG(45.0));

    BrLightEnable(world->light);

    return world;
}

int main(int argc, char **argv)
{
    br_pixelmap   *screen = NULL, *colour_buffer = NULL, *depth_buffer = NULL;
    int            ret = -1;
    br_error       r;
    brrobot_world *world;
    br_uint_64     ticks_last, ticks_now;
    br_boolean     is_fullscreen = BR_FALSE;
    SDL_Window    *window;
    float          accum;

    /*
     * QoL for those poor X11 users.
     */
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    BrBegin();

    // clang-format off
   r = BrDevBeginVar(&screen, "SDL2",
                     BRT_WIDTH_I32,        1280,
                     BRT_HEIGHT_I32,       720,
                     BRT_PIXEL_TYPE_U8,    BR_PMT_RGB_888,
                     BRT_WINDOW_NAME_CSTR, "BRender Sample Application",
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

    BrRendererBegin(colour_buffer, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    world = world_allocate();

    ticks_last = SDL_GetTicks64();

    br_fvector2 cam_robot = BR_VECTOR2(0, 0);

    accum = 0.0f;
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
                            ((br_camera *)world->camera->type_data)->aspect = BR_DIV(BR_SCALAR(screen->width),
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

                    switch(evt.key.keysym.sym) {
                        case SDLK_SPACE:
                            BrMatrix34Identity(&world->camera_pivot->t.t.mat);
                            break;
                        case 'q':
                            SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});
                            break;
                    }
                    break;
                }
                case SDL_MOUSEMOTION: {

                    if(evt.motion.state & SDL_BUTTON_LMASK) {
                        br_angle ax, ay;
                        /* NB: dt is already accounted for in xrel/yrel. */
                        cam_robot.v[0] -= ((float)evt.motion.xrel * CAM_XSPEED) / (float)screen->width;
                        cam_robot.v[0] = fmodf(cam_robot.v[0], 1.0f);

                        cam_robot.v[1] -= ((float)evt.motion.yrel * CAM_YSPEED) / (float)screen->height;
                        cam_robot.v[1] = fmodf(cam_robot.v[1], 1.0f);

                        ax = BrScalarToAngle(BrFloatToScalar(cam_robot.v[0]));
                        ay = BrScalarToAngle(BrFloatToScalar(cam_robot.v[1]));

                        BrMatrix34RotateY(&world->camera_pivot->t.t.mat, ax);
                        BrMatrix34PreRotateX(&world->camera_pivot->t.t.mat, ay);
                    }
                    break;
                }
            }
        }

        BrMatrix34Translate(&world->camera->t.t.mat, 0, 0, 1700);

        /*
         * Apply the animation once we've hit our accumulator.
         */
        accum += dt;
        while(accum >= ANIM_FPS) {
            BrApplyVue(world->vue, world->actor);
            accum -= ANIM_FPS;
        }

        BrRendererFrameBegin();
        BrPixelmapFill(colour_buffer, BR_COLOUR_RGB(66, 66, 66));
        BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
        BrPixelmapTextF(colour_buffer, -colour_buffer->origin_x + 10, -colour_buffer->origin_y + 10, 0xFFFFFFFF,
                        BrFontProp7x9, "dt = %f", dt);
        BrZbSceneRender(world->root, world->camera, colour_buffer, depth_buffer);
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
