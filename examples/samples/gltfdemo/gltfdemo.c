/*
 * glTF viewer using brdemo framework.
 * Loads a .gltf/.glb and renders with orbit camera.
 * Supports keyframe animation via br_animation_set.
 */
#include <brdemo.h>
#include <fmt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct br_gltfdemo_args {
    br_demo_run_args args;

    const char *model_path;
    const char *screenshot_path;

    int   screenshot_frame;
    int   max_frames;
    float timeout;
    float anim_speed;
    float brightness;
    float init_dist_mul;
    float init_dist_abs;
    float init_angle;
    float init_elevation;
    float orbit_speed;
    int   center_override;
    float center_x, center_y, center_z;
} br_gltfdemo_args;

typedef struct gltfview_state {
    br_actor              *model_root;
    br_animation_instance *anim;
    float                  anim_time;
    float                  elapsed;
    int                    frame_count;
    br_vector3             scene_center;
    br_scalar              scene_radius;
    float                  orbit_angle;
    float                  orbit_elev;
    float                  orbit_dist_mul;
    br_boolean             paused;
} gltfview_state;

typedef struct {
    br_vector3 mn, mx;
    int        init;
} bbox_ctx;

static br_uint_32 BR_CALLBACK bbox_actor_cb(br_actor *a, void *arg)
{
    bbox_ctx *b = arg;
    if(a->model) {
        br_vector3 *mn = &a->model->bounds.min;
        br_vector3 *mx = &a->model->bounds.max;
        if(!b->init) {
            b->mn   = *mn;
            b->mx   = *mx;
            b->init = 1;
        } else {
            if(mn->v[0] < b->mn.v[0])
                b->mn.v[0] = mn->v[0];
            if(mn->v[1] < b->mn.v[1])
                b->mn.v[1] = mn->v[1];
            if(mn->v[2] < b->mn.v[2])
                b->mn.v[2] = mn->v[2];
            if(mx->v[0] > b->mx.v[0])
                b->mx.v[0] = mx->v[0];
            if(mx->v[1] > b->mx.v[1])
                b->mx.v[1] = mx->v[1];
            if(mx->v[2] > b->mx.v[2])
                b->mx.v[2] = mx->v[2];
        }
    }
    BrActorEnum(a, bbox_actor_cb, arg);
    return 0;
}

/*
 * Save colour buffer as PNG. Reads back from the GL backbuffer into
 * a software pixelmap, then saves via BrFmtImageSave.
 */
static void save_screenshot(br_demo *demo, const char *path)
{
    br_pixelmap *cb = demo->colour_buffer;
    br_pixelmap *copy;

    copy = BrPixelmapAllocate(BR_PMT_RGBX_888, cb->width, cb->height, NULL, BR_PMAF_NORMAL);
    if(copy == NULL) {
        BrLogError("GLTFVIEW", "Screenshot alloc failed");
        return;
    }

    BrPixelmapCopy(copy, cb);

    if(BrFmtImageSave(path, copy, BR_FMT_IMAGE_PNG))
        BrLogInfo("GLTFVIEW", "Screenshot saved: %s (%dx%d)", path, cb->width, cb->height);
    else
        BrLogError("GLTFVIEW", "Screenshot failed: %s", path);

    BrPixelmapFree(copy);
}

static br_error GltfviewInit(br_demo *demo)
{
    gltfview_state         *state;
    const br_gltfdemo_args *args;
    br_fmt_results         *results;
    br_gltf_options         opts = {0};
    br_actor               *light;
    br_camera              *cam;
    char                    base_path[512];
    int                     i;

    args = (const br_gltfdemo_args *)demo->args;

    state      = BrResAllocate(demo, sizeof(*state), BR_MEMORY_APPLICATION);
    demo->user = state;

    /* Derive base_path from model filename */
    {
        const char *slash = strrchr(args->model_path, '/');
        if(slash) {
            int len = (int)(slash - args->model_path + 1);
            if(len >= (int)sizeof(base_path))
                len = sizeof(base_path) - 1;
            memcpy(base_path, args->model_path, len);
            base_path[len] = '\0';
        } else {
            base_path[0] = '.';
            base_path[1] = '/';
            base_path[2] = '\0';
        }
    }

    opts.base_path = base_path;
    opts.pm_type   = BR_PMT_RGBX_888;

    results = BrFmtGLTFActorLoadMany(args->model_path, &opts);
    if(!results || results->nactors == 0) {
        BrLogError("GLTFVIEW", "Failed to load %s", args->model_path);
        return BRE_FAIL;
    }

    BrLogInfo("GLTFVIEW", "Loaded %d actors from %s", results->nactors, args->model_path);

    /* Add all loaded actors to the world */
    state->model_root             = BrActorAllocate(BR_ACTOR_NONE, NULL);
    state->model_root->identifier = BrResStrDup(state->model_root, "gltf_root");
    BrActorAdd(demo->world, state->model_root);

    for(i = 0; i < (int)results->nactors; i++)
        BrActorAdd(state->model_root, results->actors[i]);

    /* Register loaded resources (BrModelAdd calls BrModelUpdate internally) */
    for(i = 0; i < (int)results->npixelmaps; i++) {
        if(results->pixelmaps[i] != NULL)
            BrMapAdd(results->pixelmaps[i]);
    }
    for(i = 0; i < (int)results->nmaterials; i++) {
        if(results->materials[i] != NULL)
            BrMaterialAdd(results->materials[i]);
    }
    for(i = 0; i < (int)results->nmodels; i++) {
        if(results->models[i] != NULL) {
            results->models[i]->flags |= BR_MODF_UPDATEABLE;
            BrModelAdd(results->models[i]);
        }
    }

    /* Compute scene bounds from model->bounds (populated by BrModelUpdate) */
    {
        bbox_ctx bb = {{{0}}, {{0}}, 0};
        BrActorEnum(state->model_root, bbox_actor_cb, &bb);
        if(bb.init) {
            br_scalar dx, dy, dz;
            state->scene_center.v[0] = BR_MUL(BR_ADD(bb.mn.v[0], bb.mx.v[0]), BR_SCALAR(0.5f));
            state->scene_center.v[1] = BR_MUL(BR_ADD(bb.mn.v[1], bb.mx.v[1]), BR_SCALAR(0.5f));
            state->scene_center.v[2] = BR_MUL(BR_ADD(bb.mn.v[2], bb.mx.v[2]), BR_SCALAR(0.5f));
            dx                       = BR_SUB(bb.mx.v[0], bb.mn.v[0]);
            dy                       = BR_SUB(bb.mx.v[1], bb.mn.v[1]);
            dz                       = BR_SUB(bb.mx.v[2], bb.mn.v[2]);
            state->scene_radius      = BR_MUL(BR_SCALAR(0.5f), BR_LENGTH3(dx, dy, dz));
            if(state->scene_radius < BR_SCALAR(0.1f))
                state->scene_radius = BR_SCALAR(5.0f);
        } else {
            state->scene_center = (br_vector3){
                {0, BR_SCALAR(4), 0}
            };
            state->scene_radius = BR_SCALAR(8.0f);
        }
    }

    /* Apply brightness to all loaded materials */
    for(i = 0; i < (int)results->nmaterials; i++) {
        br_material *mat = results->materials[i];
        if(mat != NULL) {
            mat->flags |= BR_MATF_LIGHT;
            mat->ka = BR_UFRACTION(args->brightness);
            mat->kd = BR_UFRACTION(1.0f - args->brightness);
            BrMaterialUpdate(mat, BR_MATU_ALL);
        }
    }

    /* Use animation data loaded by BrFmtGLTFActorLoadMany (if present) */
    state->anim        = (results->nanimation_instances > 0) ? results->animation_instances[0] : NULL;
    state->anim_time   = 0.0f;
    state->elapsed     = 0.0f;
    state->frame_count = 0;

    if(state->anim)
        BrLogInfo("GLTFVIEW", "Loaded %d animation clip(s)", state->anim->set->nclips);

    /* Camera */
    demo->camera       = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    cam                = demo->camera->type_data;
    cam->field_of_view = BR_ANGLE_DEG(50);
    cam->hither_z      = BR_SCALAR(0.1);
    cam->yon_z         = BR_SCALAR(500);

    demo->order_table->min_z = cam->hither_z;
    demo->order_table->max_z = cam->yon_z;

    /* Light */
    light                                  = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    ((br_light *)light->type_data)->colour = BR_COLOUR_RGB(255, 255, 255);
    ((br_light *)light->type_data)->type   = BR_LIGHT_DIRECT;
    light->t.type                          = BR_TRANSFORM_EULER;
    light->t.t.euler.e.order               = BR_EULER_YXZ_S;
    light->t.t.euler.e.a                   = BR_ANGLE_DEG(-30);
    light->t.t.euler.e.b                   = BR_ANGLE_DEG(-45);
    light->t.t.euler.e.c                   = 0;
    BrLightEnable(light);

    /* Apply center override */
    if(args->center_override) {
        state->scene_center.v[0] = BR_SCALAR(args->center_x);
        state->scene_center.v[1] = BR_SCALAR(args->center_y);
        state->scene_center.v[2] = BR_SCALAR(args->center_z);
    }

    state->orbit_angle    = args->init_angle;
    state->orbit_elev     = args->init_elevation;
    state->orbit_dist_mul = (args->init_dist_abs > 0.0f) ? args->init_dist_abs / BrScalarToFloat(state->scene_radius) : args->init_dist_mul;
    state->paused         = BR_FALSE;

    BrLogInfo("GLTFVIEW", "Scene center=(%.1f,%.1f,%.1f) radius=%.1f", BrScalarToFloat(state->scene_center.v[0]),
              BrScalarToFloat(state->scene_center.v[1]), BrScalarToFloat(state->scene_center.v[2]), BrScalarToFloat(state->scene_radius));

    return BRE_OK;
}

static void GltfviewUpdate(br_demo *demo, br_scalar dt)
{
    gltfview_state         *state = demo->user;
    const br_gltfdemo_args *args  = (const br_gltfdemo_args *)demo->args;
    br_scalar               dist, cam_x, cam_y, cam_z;
    float                   angle_rad;
    float                   fdt = BrScalarToFloat(dt);

    if(!state->paused) {
        state->orbit_angle += args->orbit_speed * fdt;
        state->anim_time += fdt * args->anim_speed;
    }
    if(state->orbit_angle >= 360.0f)
        state->orbit_angle -= 360.0f;
    if(state->orbit_angle < 0.0f)
        state->orbit_angle += 360.0f;

    /* Evaluate animation */
    if(state->anim)
        BrAnimationInstanceUpdate(state->anim, state->anim_time);

    angle_rad = state->orbit_angle * 3.14159f / 180.0f;
    dist      = BR_MUL(state->scene_radius, BR_SCALAR(state->orbit_dist_mul));

    cam_x = BR_ADD(state->scene_center.v[0], BR_MUL(dist, BR_SCALAR(cosf(state->orbit_elev) * sinf(angle_rad))));
    cam_z = BR_ADD(state->scene_center.v[2], BR_MUL(dist, BR_SCALAR(cosf(state->orbit_elev) * cosf(angle_rad))));
    cam_y = BR_ADD(state->scene_center.v[1], BR_MUL(dist, BR_SCALAR(sinf(state->orbit_elev))));

    demo->camera->t.type                = BR_TRANSFORM_LOOK_UP;
    demo->camera->t.t.look_up.look.v[0] = BR_SUB(state->scene_center.v[0], cam_x);
    demo->camera->t.t.look_up.look.v[1] = BR_SUB(state->scene_center.v[1], cam_y);
    demo->camera->t.t.look_up.look.v[2] = BR_SUB(state->scene_center.v[2], cam_z);
    demo->camera->t.t.look_up.up.v[0]   = BR_SCALAR(0);
    demo->camera->t.t.look_up.up.v[1]   = BR_SCALAR(1);
    demo->camera->t.t.look_up.up.v[2]   = BR_SCALAR(0);
    demo->camera->t.t.look_up.t.v[0]    = cam_x;
    demo->camera->t.t.look_up.t.v[1]    = cam_y;
    demo->camera->t.t.look_up.t.v[2]    = cam_z;

    state->elapsed += fdt;
}

static void GltfviewProcessEvent(br_demo *demo, const SDL_Event *evt)
{
    gltfview_state *state = demo->user;

    if(evt->type == SDL_EVENT_KEY_DOWN) {
        switch(evt->key.key) {
            case SDLK_SPACE:
                state->paused = !state->paused;
                break;
            case SDLK_LEFT:
                state->orbit_angle -= 5.0f;
                break;
            case SDLK_RIGHT:
                state->orbit_angle += 5.0f;
                break;
            case SDLK_UP:
                state->orbit_elev += 0.1f;
                if(state->orbit_elev > 1.4f)
                    state->orbit_elev = 1.4f;
                break;
            case SDLK_DOWN:
                state->orbit_elev -= 0.1f;
                if(state->orbit_elev < -0.5f)
                    state->orbit_elev = -0.5f;
                break;
            case SDLK_1:
                state->orbit_dist_mul -= 0.25f;
                if(state->orbit_dist_mul < 0.5f)
                    state->orbit_dist_mul = 0.5f;
                break;
            case SDLK_0:
                state->orbit_dist_mul += 0.25f;
                if(state->orbit_dist_mul > 10.0f)
                    state->orbit_dist_mul = 10.0f;
                break;
            case SDLK_A:
                if(state->anim && state->anim->set->nclips > 0) {
                    state->anim->active = (state->anim->active + 1) % state->anim->set->nclips;
                    state->anim_time    = 0.0f;
                    BrLogInfo("GLTFVIEW", "Playing clip %d: %s", state->anim->active,
                              state->anim->set->clips[state->anim->active].identifier ? state->anim->set->clips[state->anim->active].identifier
                                                                                      : "(unnamed)");
                }
                break;
        }
    }
}

static void GltfviewRender(br_demo *demo)
{
    gltfview_state         *state = demo->user;
    const br_gltfdemo_args *args  = (const br_gltfdemo_args *)demo->args;

    BrRendererFrameBegin();
    BrPixelmapFill(demo->colour_buffer, demo->clear_colour);
    BrPixelmapFill(demo->depth_buffer, 0xFFFFFFFF);

    if(demo->camera != NULL)
        BrZbSceneRender(demo->world, demo->camera, demo->colour_buffer, demo->depth_buffer);

    BrRendererFrameEnd();

    /* Screenshot on target frame */
    if(args->screenshot_path && state->frame_count == args->screenshot_frame)
        save_screenshot(demo, args->screenshot_path);

    state->frame_count++;

    /* Exit conditions */
    if((args->max_frames > 0 && state->frame_count >= args->max_frames) || (args->timeout > 0.0f && state->elapsed >= args->timeout)) {
        SDL_Event quit_evt;
        if(args->timeout > 0.0f && state->elapsed >= args->timeout)
            BrLogInfo("GLTFVIEW", "Timeout reached (%.1fs)", (double)args->timeout);
        quit_evt.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit_evt);
    }
}

static const br_demo_dispatch gltfview_dispatch = {
    .init          = GltfviewInit,
    .process_event = GltfviewProcessEvent,
    .update        = GltfviewUpdate,
    .render        = GltfviewRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

#include "../brdemo/parg.h"

#define ARGDEF_WIDTH             'w'
#define ARGDEF_HEIGHT            'h'
#define ARGDEF_VERBOSE           'v'
#define ARGDEF_HELP              301
#define ARGDEF_FORCE_SOFTWARE    302
#define ARGDEF_SOFTWARE_BPP      303
#define ARGDEF_BACKBUFFER_WIDTH  304
#define ARGDEF_BACKBUFFER_HEIGHT 305

#define ARGDEF_DIST_MUL          401
#define ARGDEF_DIST              402
#define ARGDEF_ANGLE             403
#define ARGDEF_ELEVATION         404
#define ARGDEF_ORBIT_SPEED       405
#define ARGDEF_CENTER            406
#define ARGDEF_ANIMSPEED         407
#define ARGDEF_SCREENSHOT        408
#define ARGDEF_FRAME             409
#define ARGDEF_FRAMES            410
#define ARGDEF_TIMEOUT           411
#define ARGDEF_BRIGHTNESS        412

const static struct parg_option argdefs[] = {
    {.name = "width",             .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_WIDTH            },
    {.name = "height",            .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_HEIGHT           },
    {.name = "verbose",           .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_VERBOSE          },
    {.name = "help",              .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_HELP             },
    {.name = "force-software",    .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_FORCE_SOFTWARE   },
    {.name = "software-bpp",      .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_SOFTWARE_BPP     },
    {.name = "backbuffer-width",  .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_BACKBUFFER_WIDTH },
    {.name = "backbuffer-height", .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_BACKBUFFER_HEIGHT},

    {.name = "dist-mul",          .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_DIST_MUL         },
    {.name = "dist",              .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_DIST             },
    {.name = "angle",             .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_ANGLE            },
    {.name = "elevation",         .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_ELEVATION        },
    {.name = "orbit-speed",       .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_ORBIT_SPEED      },
    {.name = "center",            .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_CENTER           },

    {.name = "animspeed",         .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_ANIMSPEED        },

    {.name = "screenshot",        .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_SCREENSHOT       },
    {.name = "frame",             .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_FRAME            },
    {.name = "frames",            .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_FRAMES           },
    {.name = "timeout",           .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_TIMEOUT          },

    {.name = "brightness",        .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_BRIGHTNESS       },

    {.name = NULL,                .has_arg = 0,           .flag = NULL, .val = 0                       },
};

static int parse_args(int argc, char *const argv[], br_gltfdemo_args *args)
{
    struct parg_state ps;
    parg_init(&ps);
    int software_bpp = 8;

    for(int c; (c = parg_getopt_long(&ps, argc, argv, "w:h:v", argdefs, NULL)) != -1;) {
        switch(c) {
            case ARGDEF_WIDTH:
                args->args.width = BrAToI(ps.optarg);
                break;

            case ARGDEF_HEIGHT:
                args->args.height = BrAToI(ps.optarg);
                break;

            case ARGDEF_VERBOSE:
                ++args->args.verbose;
                break;

            case ARGDEF_FORCE_SOFTWARE:
                ++args->args.force_software;
                break;

            case ARGDEF_SOFTWARE_BPP:
                software_bpp = BrAToI(ps.optarg);
                break;

            case ARGDEF_BACKBUFFER_WIDTH:
                args->args.backbuffer_width = BrAToI(ps.optarg);
                break;

            case ARGDEF_BACKBUFFER_HEIGHT:
                args->args.backbuffer_height = BrAToI(ps.optarg);
                break;

            case ARGDEF_SCREENSHOT:
                args->screenshot_path = ps.optarg;
                break;

            case ARGDEF_FRAME:
                args->screenshot_frame = BrAToI(ps.optarg);
                break;

            case ARGDEF_FRAMES:
                args->max_frames = BrAToI(ps.optarg);
                break;

            case ARGDEF_TIMEOUT:
                args->timeout = BrAToF(ps.optarg);
                break;

            case ARGDEF_ANIMSPEED:
                args->anim_speed = BrAToF(ps.optarg);
                break;

            case ARGDEF_BRIGHTNESS:
                args->brightness = BrAToF(ps.optarg);
                break;

            case ARGDEF_DIST_MUL:
                args->init_dist_mul = BrAToF(ps.optarg);
                break;

            case ARGDEF_DIST:
                args->init_dist_abs = BrAToF(ps.optarg);
                break;

            case ARGDEF_ANGLE:
                args->init_angle = BrAToF(ps.optarg);
                break;

            case ARGDEF_ELEVATION:
                args->init_elevation = BrAToF(ps.optarg);
                break;

            case ARGDEF_ORBIT_SPEED:
                args->orbit_speed = BrAToF(ps.optarg);
                break;

            case ARGDEF_CENTER:
                args->center_override = 1;
                if(BrSScanf(ps.optarg, "%f,%f,%f", &args->center_x, &args->center_y, &args->center_z) < 3)
                    return 2;
                break;

            case 1:
                if(args->model_path != NULL)
                    return 2;

                args->model_path = ps.optarg;
                break;

            case ARGDEF_HELP:
            case ':':
            case '?':
            default:
                return 2;
        }
    }

    if(args->args.width < 0)
        return 2;

    if(args->args.height < 0)
        return 2;

    if(args->model_path == NULL || args->model_path[0] == '\0')
        return 2;

    switch(software_bpp) {
        case 16:
            args->args.software_pm_type = BR_PMT_RGB_565;
            break;
        case 15:
            args->args.software_pm_type = BR_PMT_RGB_555;
            break;
        case 24:
            args->args.software_pm_type = BR_PMT_RGB_888;
            break;
        default:
        case 8:
            args->args.software_pm_type = BR_PMT_INDEX_8;
            break;
    }

    if(args->args.backbuffer_width < 0)
        args->args.backbuffer_width = 0;

    if(args->args.backbuffer_height < 0)
        args->args.backbuffer_height = 0;

    if(args->args.title == NULL || args->args.title[0] == '\0')
        args->args.title = "BRender Application";

    return 0;
}

// clang-format off
static const char *usage_options =
    "Usage: gltfdemo [options] model.glb\n"
    "\n"
    "Framework:\n"
    "  -w, --width          Set the initial window width (default: 1280).\n"
    "  -h, --height         Set the initial window height (default: 720).\n"
    "  -v, --verbose        Increase verbosity level. May be given multiple times.\n"
    "  --force-software     Force use of the software renderer. Only valid on x86.\n"
    "  --software-bpp       Software renderer bit depth. Valid options: 8 (default), 15, 16.\n"
    "  --backbuffer-width   Width of the backbuffer, defaults to the same as the screen.\n"
    "  --backbuffer-height  Height of the backbuffer, defaults to the same as the screen.\n"
    "  --help               Display this message.\n"
    "\n"
    "Camera:\n"
    "  --dist-mul F         orbit distance as multiplier of scene radius (default 2.5)\n"
    "  --dist F             absolute orbit distance (overrides --dist-mul)\n"
    "  --angle F            starting orbit angle in degrees (default 0)\n"
    "  --elevation F        camera elevation in radians (default 0.15)\n"
    "  --orbit-speed F      orbit rotation speed in deg/sec (default 60, 0 = static)\n"
    "  --center X,Y,Z       override scene look-at point\n"
    "\n"
    "Animation:\n"
    "  --animspeed F        animation playback speed multiplier (default 1.0)\n"
    "\n"
    "Capture:\n"
    "  --screenshot P       save PNG screenshot to path P\n"
    "  --frame N            capture screenshot on frame N (default 10)\n"
    "  --frames N           render N frames then exit (0 = unlimited)\n"
    "  --timeout S          exit after S seconds (0 = unlimited)\n"
    "\n"
    "Display:\n"
    "  --brightness F       material ambient brightness (default 0.60)\n"
    "\n"
    "Interactive:\n"
    "  Space                pause/resume\n"
    "  A                    cycle animation clip\n"
    "  Left/Right           orbit camera\n"
    "  Up/Down              adjust elevation\n"
    "  0/1                  zoom out/in\n"
    "\n"
;
// clang-format on

int main(int argc, char **argv)
{
    int r;

    br_gltfdemo_args args = {
        .args             = {},
        .model_path       = NULL,
        .screenshot_path  = NULL,
        .screenshot_frame = 10,
        .max_frames       = 0,
        .timeout          = 0.0f,
        .anim_speed       = 1.0f,
        .brightness       = 0.60f,
        .init_dist_mul    = 2.5f,
        .init_dist_abs    = 0.0f,
        .init_angle       = 0.0f,
        .init_elevation   = 0.15f,
        .orbit_speed      = 60.0f,
        .center_override  = 0,
        .center_x         = 0.0f,
        .center_y         = 0.0f,
        .center_z         = 0.0f,
    };

    BrDemoDefaultArgs(&args.args);
    args.args.title = "glTF Viewer";

    struct parg_state ps;
    parg_init(&ps);
    if((r = parse_args(argc, argv, &args)) != 0) {
        fprintf(stderr, "%s\n", usage_options);
        return r;
    }

    return BrDemoRunArg(&gltfview_dispatch, &args.args);
}
