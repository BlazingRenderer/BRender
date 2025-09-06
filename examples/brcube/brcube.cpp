#include <SDL3/SDL.h>
#include <cassert>
#include <brender.h>
#include <brsdl3dev.h>
#include <brglrend.h>

#include "editor.hpp"

#include "editorcam.h"

typedef struct BrCubeApplication {
    SDL_Window *sdl_window;

    br_pixelmap *screen;
    br_pixelmap *colour_buffer;
    br_pixelmap *depth_buffer;

    br_actor *world;
    br_actor *pivot;
    br_actor *light;
    br_light *lightdata;

    /*
     * Primitive heap - used by z-buffered renderer to defer drawing of blended primitives
     */
    uint8_t primitive_heap[1500 * 1024];
} BrCubeApplication;

static br_actor *create_light(br_actor *parent, const char *identifier)
{
    br_actor *light;
    br_light *lightdata;

    light = BrActorAdd(parent, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    BrMatrix34RotateX(&light->t.t.mat, BR_ANGLE_DEG(-90));

    light->identifier = BrResStrDup(light, identifier);

    lightdata                = reinterpret_cast<br_light *>(light->type_data);
    lightdata->type          = BR_LIGHT_SPOT;
    lightdata->colour        = BR_COLOUR_RGB(255, 255, 255);
    lightdata->attenuation_c = BR_UFRACTION(0.2);
    lightdata->attenuation_l = BR_UFRACTION(0);
    lightdata->attenuation_q = BR_UFRACTION(0);
    lightdata->cone_inner    = BR_ANGLE_DEG(0);
    lightdata->cone_outer    = BR_ANGLE_DEG(20);

    BrLightEnable(light);
    return light;
}

static br_actor *create_box(br_actor *parent, const char *identifier)
{
    br_actor *box   = BrActorAdd(parent, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    box->identifier = BrResStrDup(box, identifier);
    box->model      = BrModelFind("cube.dat");
    box->material   = BrMaterialFind("checkerboard.mat");
    if(strcmp(identifier, "MainBox"))
        create_light(box, "Light");

    box->material->ka    = 0.1f;
    box->material->kd    = 1.0f;
    box->material->ks    = 0.1f;
    box->material->power = 64.0f;
    BrMaterialUpdate(box->material, BR_MATU_ALL);
    return box;
}

void XXBrMatrix34FromTRSEuler(br_matrix34 *A, const br_vector3 *T, const br_euler *R, const br_vector3 *S)
{
    br_matrix34 rotmat, tmpmat;
    BrEulerToMatrix34(&rotmat, R);

    BrMatrix34Scale(A, S->v[0], S->v[1], S->v[2]);
    BrMatrix34Mul(&tmpmat, &rotmat, A);
    BrMatrix34PostTranslate(&tmpmat, T->v[0], T->v[1], T->v[2]);
    BrMatrix34Copy(A, &tmpmat);
}

static br_actor *create_bigscene(br_actor **_pivot)
{
    br_actor       *root, *ground, *light;
    br_actor       *main_box, *pivot, *left_box, *right_box, *front_box, *back_box;
    br_order_table *order_table;
    br_camera      *flycam_data;
    br_light       *light_data;

    root        = BrActorAllocate(BR_ACTOR_NONE, NULL);
    order_table = BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_LEAVE_BOUNDS | BR_ORDER_TABLE_BUCKET_SORT, BR_SORT_AVERAGE);
    BrZsActorOrderTableSet(root, order_table);

    //    {
    //        flycam      = BrActorAdd(root, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    //        flycam_data = reinterpret_cast<br_camera *>(flycam->type_data);
    //
    //        flycam_data->field_of_view = BR_ANGLE_DEG(62);
    //        flycam_data->hither_z      = BR_SCALAR(0.01);
    //        flycam_data->yon_z         = BR_SCALAR(1000.0);
    //        flycam_data->aspect        = BR_SCALAR(1280.0f / 720.0f);
    //
    //        order_table->min_z = flycam_data->hither_z;
    //        order_table->max_z = flycam_data->yon_z;
    //
    //        //        BrMatrix34Translate(&flycam->t.t.mat, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(10));
    //
    //        //        camNode->local_position(-32_s, 30_s, -32_s);
    //        //        camNode->local_rotation(140_s, 225_s, 180_s, math::angle_type::degrees);
    //
    //        BrMatrix34Translate(&flycam->t.t.mat, BR_SCALAR(-32), BR_SCALAR(30), BR_SCALAR(32));
    //
    //        br_matrix34 rot, tmp;
    //        br_euler    e = {
    //               .a     = BR_ANGLE_DEG(140),
    //               .b     = BR_ANGLE_DEG(225),
    //               .c     = BR_ANGLE_DEG(180),
    //               .order = BR_EULER_XYZ_S,
    //        };
    //        BrEulerToMatrix34(&rot, &e);
    //
    //        BrMatrix34Mul(&tmp, &rot, &flycam->t.t.mat);
    //        BrMatrix34Copy(&flycam->t.t.mat, &tmp);
    //    }

    light             = BrActorAdd(root, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    light->identifier = BrResStrDup(light, "Sun");
    {
        br_vector3 t = BR_VECTOR3(0, 0, 0);
        br_vector3 s = BR_VECTOR3(1, 1, 1);
        br_euler   r = {
            //.a = BR_ANGLE_DEG(90),
              .a     = BR_ANGLE_DEG(150),
              .b     = BR_ANGLE_DEG(54),
              .c     = BR_ANGLE_DEG(0),
              .order = BR_EULER_XYZ_S,
        };
        XXBrMatrix34FromTRSEuler(&light->t.t.mat, &t, &r, &s);
    }
    light_data                = reinterpret_cast<br_light *>(light->type_data);
    light_data->type          = BR_LIGHT_DIRECT;
    light_data->colour        = BR_COLOUR_RGB(255, 255, 255);
    light_data->attenuation_c = BR_UFRACTION(1);
    light_data->attenuation_l = BR_UFRACTION(0);
    light_data->attenuation_q = BR_UFRACTION(0);
    BrLightEnable(light);

    {
        br_actor *ambient_light;

        ambient_light      = BrActorAdd(root, BrActorAllocate(BR_ACTOR_LIGHT, nullptr));
        light_data         = reinterpret_cast<br_light *>(ambient_light->type_data);
        light_data->type   = BR_LIGHT_AMBIENT;
        light_data->colour = BR_COLOUR_RGB(255, 255, 255);

        BrLightEnable(ambient_light);
    }
    ground             = BrActorAdd(root, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    ground->identifier = BrResStrDup(ground, "Ground");
    ground->model      = BrModelFind("quad.dat");
    ground->material   = BrMaterialFind("checkerboard.mat");

    {
        br_vector3 t = BR_VECTOR3(0, -0.5, 0);
        br_vector3 s = BR_VECTOR3(50, 50, 50);
        br_euler   r = {
              .a     = BR_ANGLE_DEG(-90),
              .b     = BR_ANGLE_DEG(0),
              .c     = BR_ANGLE_DEG(0),
              .order = BR_EULER_XYZ_S,
        };
        XXBrMatrix34FromTRSEuler(&ground->t.t.mat, &t, &r, &s);
    }

    main_box = create_box(root, "MainBox");
    BrMatrix34Scale(&main_box->t.t.mat, BR_SCALAR(10), BR_SCALAR(10), BR_SCALAR(10));
    BrMatrix34PostTranslate(&main_box->t.t.mat, BR_SCALAR(0), BR_SCALAR(5), BR_SCALAR(0));

    {
        br_actor *teapot   = BrActorAdd(main_box, BrActorAllocate(BR_ACTOR_MODEL, NULL));
        teapot->identifier = BrResStrDup(teapot, "Teapot");
        teapot->model      = BrModelFind("teapot.dat");

        BrMatrix34PreTranslate(&teapot->t.t.mat, BR_SCALAR(0), BR_SCALAR(2), BR_SCALAR(0));
        BrMatrix34PreRotateX(&teapot->t.t.mat, BR_ANGLE_DEG(-90));

        br_material *teamat = BrMaterialAllocate("teamat");
        teamat->colour      = BR_COLOUR_RGB(255, 255, 255);
        teamat->opacity     = 255;
        teamat->ka          = BR_UFRACTION(0.05);
        teamat->kd          = BR_UFRACTION(0.45);
        teamat->ks          = BR_UFRACTION(0.50);
        teamat->power       = BR_SCALAR(20);
        teamat->flags |= BR_MATF_LIGHT | BR_MATF_DISABLE_COLOUR_KEY | BR_MATF_ENVIRONMENT_L;

        teamat->colour_map = BrMapFind("testenv1.pix");
        BrMaterialAdd(teamat);

        teapot->material = teamat;

        BrEnvironmentSet(main_box);
    }

    pivot = BrActorAdd(main_box, BrActorAllocate(BR_ACTOR_NONE, NULL));

    left_box = create_box(pivot, "LeftBox");
    BrMatrix34Scale(&left_box->t.t.mat, BR_SCALAR(0.5), BR_SCALAR(0.5), BR_SCALAR(0.5));
    BrMatrix34PostTranslate(&left_box->t.t.mat, BR_SCALAR(-2), BR_SCALAR(1), BR_SCALAR(0));

    right_box = create_box(pivot, "RightBox");
    BrMatrix34Scale(&right_box->t.t.mat, BR_SCALAR(0.5), BR_SCALAR(0.5), BR_SCALAR(0.5));
    BrMatrix34PostTranslate(&right_box->t.t.mat, BR_SCALAR(2), BR_SCALAR(1), BR_SCALAR(0));

    front_box = create_box(pivot, "FrontBox");
    BrMatrix34Scale(&front_box->t.t.mat, BR_SCALAR(0.5), BR_SCALAR(0.5), BR_SCALAR(0.5));
    BrMatrix34PostTranslate(&front_box->t.t.mat, BR_SCALAR(0), BR_SCALAR(1), BR_SCALAR(-2));

    back_box = create_box(pivot, "BackBox");
    BrMatrix34Scale(&back_box->t.t.mat, BR_SCALAR(0.5), BR_SCALAR(0.5), BR_SCALAR(0.5));
    BrMatrix34PostTranslate(&back_box->t.t.mat, BR_SCALAR(0), BR_SCALAR(1), BR_SCALAR(2));

    *_pivot = pivot;
    return root;
}

static void create_scene(BrCubeApplication *app, Editor::Editor *ed)
{
    assert(app->world == nullptr);
    // assert(app->camera == nullptr);

    BrModelFindHook(BrModelFindFailedLoad);
    BrMapFindHook(BrMapFindFailedLoad);
    BrMaterialFindHook(BrMaterialFindFailedLoad);
    app->world = create_bigscene(&app->pivot);
    // app->camera = BrEditorCamAllocate(app->world);
    // BrActorAdd(app->world, app->camera->mTop);

    br_order_table *order_table = BrZsActorOrderTableGet(app->world);
    order_table->min_z          = ed->GetCameraPane()->GetCamera()->camera_data.hither_z;
    order_table->max_z          = ed->GetCameraPane()->GetCamera()->camera_data.yon_z;
}

extern "C" void _BrBeginHook(void)
{
    BrDevAddStatic(nullptr, BrDrv1SDL3Begin, nullptr);
    BrDevAddStatic(nullptr, BrDrv1GLBegin, nullptr);
}

extern "C" void _BrEndHook(void)
{
}

static uint8_t primitive_heap[1500 * 1024];

static BrCubeApplication app = {0}; /* eww global, but we don't care */

int main(int argc, char **argv)
{
    br_pixelmap  *screen = NULL, *colour_buffer = NULL, *depth_buffer = NULL;
    int           ret = -1;
    br_error      r;
    br_actor     *world = NULL;
    br_uint_64    ticks_last, ticks_now;
    br_boolean    is_fullscreen = BR_FALSE;
    SDL_Window   *window;
    SDL_GLContext glctx;

    /*
     * QoL for those poor X11 users.
     */
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    BrBegin();

    Editor::Editor editor;

    BrLogHandlerSet(editor.GetLogHandler());
    BrLogSetLevel(BR_LOG_TRACE);

    br_device_sdl_ext_procs ext_procs = {
        .preswap = Editor::Editor::PreSwapHook,
        .user    = &editor,
    };

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // clang-format off
    r = BrDevBeginVar(&screen, "SDL3",
                      BRT_WIDTH_I32,        1280,
                      BRT_HEIGHT_I32,       720,
                      BRT_PIXEL_TYPE_U8,    BR_PMT_RGB_888,
                      BRT_WINDOW_NAME_CSTR, "BRender Cube Example",
                      BRT_HIDPI_B,          BR_TRUE,
                      BRT_RESIZABLE_B,      BR_TRUE,
                      BRT_OPENGL_B,         BR_TRUE,
                      BRT_SDL_EXT_PROCS_P,  &ext_procs,
                      BR_NULL_TOKEN);
    // clang-format on
    if(r != BRE_OK)
        goto screen_creation_failed;

    screen->origin_x = (br_int_16)(screen->width / 2);
    screen->origin_y = (br_int_16)(screen->height / 2);

    if(BrPixelmapResizeBuffers(screen, &colour_buffer, &depth_buffer) != BRE_OK)
        goto buffer_create_failed;

    window = BrSDL3UtilGetWindow(screen);
    SDL_SetWindowFullscreen(window, false);
    is_fullscreen = BR_FALSE;

    gladLoadGL(SDL_GL_GetProcAddress);

    glctx = SDL_GL_GetCurrentContext();

    BrRendererBegin(colour_buffer, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    editor.Init(window, glctx, screen);

    create_scene(&app, &editor);
    world = app.world;

    editor.SetWorld(world);
    // editor.SetViewportSize(screen->width, screen->height);

    ticks_last = SDL_GetTicksNS();

    for(SDL_Event evt;;) {
        float dt;

        ticks_now  = SDL_GetTicksNS();
        dt         = (float)(ticks_now - ticks_last) / 1e9f;
        ticks_last = ticks_now;

        while(SDL_PollEvent(&evt) > 0) {

            if(editor.ProcessEvent(&evt))
                continue;

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
                    // editor.SetViewportSize(screen->width, screen->height);
                    break;

                case SDL_EVENT_KEY_DOWN: {
                    if(BrSDL3UtilIsAltEnter(&evt.key)) {
                        if(is_fullscreen) {
                            SDL_SetWindowFullscreen(window, false);
                        } else {
                            SDL_SetWindowFullscreen(window, true);
                        }
                        is_fullscreen = !is_fullscreen;
                        break;
                    }
                    break;
                }
            }
        }

        BrMatrix34PostRotateY(&app.pivot->t.t.mat, BR_ANGLE_DEG(BR_SCALAR(50) * BR_SCALAR(dt)));

        editor.Update(dt);

        // BrPixelmapFill(colour_buffer, BR_COLOUR_RGBA(0, 0, 0, 0xFF));
        // BrPixelmapFill(depth_buffer, 0xFFFFFFFF);

        //        BrRendererFrameBegin();
        //        // BrZbSceneRender(world, editor.GetCamera()->camera, colour_buffer, depth_buffer);
        //        BrRendererFrameEnd();

        // editor.SetCameraTarget(colour_buffer);
        editor.Render();

        // Editor::Editor::PreSwapHook(screen, &editor);

        BrPixelmapDoubleBuffer(screen, colour_buffer);

        // SDL_GL_SwapWindow(window);
    }

done:
    ret = 0;

    BrRendererEnd();

    editor.Shutdown();

    BrPixelmapFree(depth_buffer);
    BrPixelmapFree(colour_buffer);

buffer_create_failed:
    BrPixelmapFree(screen);

screen_creation_failed:

    BrEnd();
    return ret;
}
