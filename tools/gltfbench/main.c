/*
 * gltfbench - render a glTF scene headlessly and report what it drew.
 *
 * The benchmark gltfview used to carry, without the viewer: no demo framework,
 * no editorcam, no input, and no environment variables. The scene setup is
 * gltfview's (examples/gltfview/main.c), the fixed camera is gltfdemo's, and
 * the checksum is brcapture's, so a frame means the same thing here as it does
 * to rendertest.
 *
 * The glTF load below is deliberately a copy of gltfview's rather than a
 * shared library: the two programs answer different questions, and the scene
 * setup is the part an agent reads when it wants to know what was rendered.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <brender.h>
#include <brglrend.h>
#include <brglrend1x.h>
#include <brsdl3dev.h>
#include <fmt.h>

#include <brcapture.h>
#include "parg.h"

#define GLTFBENCH_DEFAULT_WIDTH     1280
#define GLTFBENCH_DEFAULT_HEIGHT    720
#define GLTFBENCH_DEFAULT_DEVICE    "glrend"
#define GLTFBENCH_DEFAULT_WARMUP    10
#define GLTFBENCH_DEFAULT_FRAMES    1
#define GLTFBENCH_DEFAULT_DIST_MUL  2.5f
#define GLTFBENCH_DEFAULT_ANGLE     0.0f
#define GLTFBENCH_DEFAULT_ELEVATION 0.15f

/* The camera gltfdemo looks through, and the scene bounds it falls back to. */
#define GLTFBENCH_FOV_DEG 50.0f
#define GLTFBENCH_HITHER  0.1f
#define GLTFBENCH_YON     500.0f
#define GLTFBENCH_PI      3.14159265358979323846f

/* Used by the z-buffered renderer to defer blended primitives; brdemo's size. */
static br_uint_8 gltfbench_primitive_heap[8192 * 1024];

struct br_device *BR_EXPORT BrDrv1SoftPrimBegin(const char *arguments);
struct br_device *BR_EXPORT BrDrv1SoftRendBegin(const char *arguments);

typedef struct gltfbench_args {
    const char *model;
    br_int_32   width;
    br_int_32   height;
    const char *device;
    br_int_32   warmup;
    br_int_32   frames;
    float       dist;
    float       angle;
    float       elevation;
    br_boolean  checksum;
    const char *ppm_path;
    const char *csv_path;
} gltfbench_args;

typedef struct gltfbench {
    const gltfbench_args *args;

    br_pixelmap    *screen;
    br_pixelmap    *colour_buffer;
    br_pixelmap    *depth_buffer;
    br_actor       *world;
    br_actor       *camera;
    br_order_table *order_table;

    /* The CLUT the indexed software paths present through. */
    br_pixelmap *palette;

    br_colour  clear_colour;
    br_boolean hw_accel;

    SDL_Window   *window;
    br_capture_gl gl;
} gltfbench;

void _BrBeginHook(void) // NOLINT(*-reserved-identifier)
{
    BrDevAddStatic(NULL, BrDrv1SDL3Begin, NULL);
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);
    BrDevAddStatic(NULL, BrDrv1GL1xBegin, NULL);
    BrDevAddStatic(NULL, BrDrv1SoftPrimBegin, NULL);
    BrDevAddStatic(NULL, BrDrv1SoftRendBegin, NULL);
}

void _BrEndHook(void) // NOLINT(*-reserved-identifier)
{
}

/* ------------------------------------------------------------------ */
/* Command line.                                                      */
/* ------------------------------------------------------------------ */

enum {
    /* 1 is parg's return for a non-option, so the option values start past it. */
    GLTFBENCH_OPT_WIDTH = 2,
    GLTFBENCH_OPT_HEIGHT,
    GLTFBENCH_OPT_DEVICE,
    GLTFBENCH_OPT_WARMUP,
    GLTFBENCH_OPT_FRAMES,
    GLTFBENCH_OPT_DIST,
    GLTFBENCH_OPT_ANGLE,
    GLTFBENCH_OPT_ELEVATION,
    GLTFBENCH_OPT_CHECKSUM,
    GLTFBENCH_OPT_PPM,
    GLTFBENCH_OPT_CSV,
    GLTFBENCH_OPT_HELP,
};

static const struct parg_option gltfbench_options[] = {
    {.name = "width", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_WIDTH},
    {.name = "height", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_HEIGHT},
    {.name = "device", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_DEVICE},
    {.name = "warmup", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_WARMUP},
    {.name = "frames", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_FRAMES},
    {.name = "dist", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_DIST},
    {.name = "angle", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_ANGLE},
    {.name = "elevation", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_ELEVATION},
    {.name = "checksum", .has_arg = PARG_NOARG, .val = GLTFBENCH_OPT_CHECKSUM},
    {.name = "ppm", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_PPM},
    {.name = "csv", .has_arg = PARG_REQARG, .val = GLTFBENCH_OPT_CSV},
    {.name = "help", .has_arg = PARG_NOARG, .val = GLTFBENCH_OPT_HELP},
    {.name = NULL},
};

static void gltfbench_usage(FILE *fp, const char *argv0)
{
    fprintf(fp,
            "usage: %s MODEL.gltf [options]\n"
            "\n"
            "Renders a glTF scene offscreen and reports a checksum of the last frame.\n"
            "\n"
            "  --width N       window width in pixels (default %d)\n"
            "  --height N      window height in pixels (default %d)\n"
            "  --device NAME   glrend, glrend1x or softrend (default %s)\n"
            "  --warmup N      frames to render before measuring (default %d)\n"
            "  --frames N      measured frames to render (default %d)\n"
            "  --dist F        camera distance from the scene centre (default %.1fx the scene radius)\n"
            "  --angle F       camera orbit angle in degrees (default %.1f)\n"
            "  --elevation F   camera elevation in radians (default %.2f)\n"
            "  --checksum      print the last frame's checksum and coverage\n"
            "  --ppm PATH      write the last measured frame to PATH as a binary PPM\n"
            "  --csv PATH      write one line per measured frame: milliseconds\n"
            "  --help          show this help and exit\n",
            argv0, GLTFBENCH_DEFAULT_WIDTH, GLTFBENCH_DEFAULT_HEIGHT, GLTFBENCH_DEFAULT_DEVICE, GLTFBENCH_DEFAULT_WARMUP,
            GLTFBENCH_DEFAULT_FRAMES, GLTFBENCH_DEFAULT_DIST_MUL, GLTFBENCH_DEFAULT_ANGLE, GLTFBENCH_DEFAULT_ELEVATION);
}

enum {
    GLTFBENCH_PARSE_OK = 0,
    GLTFBENCH_PARSE_HELP,
    GLTFBENCH_PARSE_ERROR,
};

static int gltfbench_parse_args(int argc, char **argv, gltfbench_args *args)
{
    struct parg_state ps;
    int               c;

    *args = (gltfbench_args){
        .model     = NULL,
        .width     = GLTFBENCH_DEFAULT_WIDTH,
        .height    = GLTFBENCH_DEFAULT_HEIGHT,
        .device    = GLTFBENCH_DEFAULT_DEVICE,
        .warmup    = GLTFBENCH_DEFAULT_WARMUP,
        .frames    = GLTFBENCH_DEFAULT_FRAMES,
        .dist      = 0.0f,
        .angle     = GLTFBENCH_DEFAULT_ANGLE,
        .elevation = GLTFBENCH_DEFAULT_ELEVATION,
        .checksum  = BR_FALSE,
        .ppm_path  = NULL,
        .csv_path  = NULL,
    };

    parg_init(&ps);

    while((c = parg_getopt_long(&ps, argc, argv, "", gltfbench_options, NULL)) != -1) {
        switch(c) {
            case 1: /* a non-option: the model */
                if(args->model != NULL) {
                    fprintf(stderr, "%s: more than one model named\n", argv[0]);
                    return GLTFBENCH_PARSE_ERROR;
                }

                args->model = ps.optarg;
                break;

            case GLTFBENCH_OPT_WIDTH:
                args->width = BrAToI(ps.optarg);
                break;

            case GLTFBENCH_OPT_HEIGHT:
                args->height = BrAToI(ps.optarg);
                break;

            case GLTFBENCH_OPT_DEVICE:
                args->device = ps.optarg;
                break;

            case GLTFBENCH_OPT_WARMUP:
                args->warmup = BrAToI(ps.optarg);
                break;

            case GLTFBENCH_OPT_FRAMES:
                args->frames = BrAToI(ps.optarg);
                break;

            case GLTFBENCH_OPT_DIST:
                args->dist = BrAToF(ps.optarg);
                break;

            case GLTFBENCH_OPT_ANGLE:
                args->angle = BrAToF(ps.optarg);
                break;

            case GLTFBENCH_OPT_ELEVATION:
                args->elevation = BrAToF(ps.optarg);
                break;

            case GLTFBENCH_OPT_CHECKSUM:
                args->checksum = BR_TRUE;
                break;

            case GLTFBENCH_OPT_PPM:
                args->ppm_path = ps.optarg;
                break;

            case GLTFBENCH_OPT_CSV:
                args->csv_path = ps.optarg;
                break;

            case GLTFBENCH_OPT_HELP:
                return GLTFBENCH_PARSE_HELP;

            case '?':
            default:
                fprintf(stderr, "%s: unknown option or missing argument\n", argv[0]);
                return GLTFBENCH_PARSE_ERROR;
        }
    }

    if(args->model == NULL || args->model[0] == '\0') {
        fprintf(stderr, "%s: no model named\n", argv[0]);
        return GLTFBENCH_PARSE_ERROR;
    }

    if(strcmp(args->device, "glrend") != 0 && strcmp(args->device, "glrend1x") != 0 && strcmp(args->device, "softrend") != 0) {
        fprintf(stderr, "%s: unknown device `%s'\n", argv[0], args->device);
        return GLTFBENCH_PARSE_ERROR;
    }

    if(args->width <= 0 || args->height <= 0) {
        fprintf(stderr, "%s: the frame size must be positive\n", argv[0]);
        return GLTFBENCH_PARSE_ERROR;
    }

    if(args->warmup < 0) {
        fprintf(stderr, "%s: the warmup frame count cannot be negative\n", argv[0]);
        return GLTFBENCH_PARSE_ERROR;
    }

    if(args->frames < 1) {
        fprintf(stderr, "%s: at least one measured frame is needed\n", argv[0]);
        return GLTFBENCH_PARSE_ERROR;
    }

    if(args->dist < 0.0f) {
        fprintf(stderr, "%s: the camera distance cannot be negative\n", argv[0]);
        return GLTFBENCH_PARSE_ERROR;
    }

    return GLTFBENCH_PARSE_OK;
}

/* ------------------------------------------------------------------ */
/* Device, buffers and renderer.                                      */
/* ------------------------------------------------------------------ */

static br_boolean gltfbench_is_software(const gltfbench *bench)
{
    return strcmp(bench->args->device, "softrend") == 0;
}

static br_error gltfbench_create_screen(gltfbench *bench)
{
    const gltfbench_args *args = bench->args;
    br_error              err;

    if(!gltfbench_is_software(bench)) {
        // clang-format off
        err = BrDevBeginVar(&bench->screen,              "SDL3",
                            BRT_WIDTH_I32,               args->width,
                            BRT_HEIGHT_I32,              args->height,
                            BRT_WINDOW_NAME_CSTR,        "BRender glTF Benchmark",
                            BRT_HIDPI_B,                 BR_TRUE,
                            BRT_RESIZABLE_B,             BR_TRUE,
                            BRT_OPENGL_B,                BR_TRUE,
                            BRT_OPENGL_DEVICE_NAME_CSTR, args->device,
                            BR_NULL_TOKEN);
        // clang-format on

        bench->hw_accel = BR_TRUE;
    } else {
        // clang-format off
        err = BrDevBeginVar(&bench->screen,       "SDL3",
                            BRT_WIDTH_I32,        args->width,
                            BRT_HEIGHT_I32,       args->height,
                            BRT_WINDOW_NAME_CSTR, "BRender glTF Benchmark",
                            BRT_HIDPI_B,          BR_FALSE,
                            BRT_RESIZABLE_B,      BR_TRUE,
                            BRT_OPENGL_B,         BR_FALSE,
                            BR_NULL_TOKEN);
        // clang-format on

        bench->hw_accel = BR_FALSE;
    }

    if(err != BRE_OK) {
        BrLogError("GLTFBENCH", "Unable to create a window for device `%s'.", args->device);
        return err;
    }

    bench->window = BrSDL3UtilGetWindow(bench->screen);
    return BRE_OK;
}

/*
 * The colour and depth buffers. The software path renders into an INDEX_8
 * buffer, as every stored softrend reference does; a GL device's own colour
 * buffer is left to the driver.
 */
static br_error gltfbench_resize_buffers(gltfbench *bench)
{
    if(!bench->hw_accel) {
        const br_token_value tv[] = {
            {.t = BRT_WIDTH_I32,      .v = {.i32 = bench->screen->width} },
            {.t = BRT_HEIGHT_I32,     .v = {.i32 = bench->screen->height}},
            {.t = BRT_PIXEL_TYPE_U8,  .v = {.u8 = BR_PMT_INDEX_8}        },
            {.t = BRT_PIXEL_BITS_I32, .v = {.i32 = 16}                   },
            {.t = BR_NULL_TOKEN,      .v = {}                            },
        };

        return BrPixelmapResizeBuffersTV(bench->screen, &bench->colour_buffer, &bench->depth_buffer, tv);
    }

    br_token_value tv[3];
    int            n = 0;

    tv[n].t       = BRT_WIDTH_I32;
    tv[n++].v.i32 = bench->screen->width;
    tv[n].t       = BRT_HEIGHT_I32;
    tv[n++].v.i32 = bench->screen->height;
    tv[n].t       = BR_NULL_TOKEN;

    return BrPixelmapResizeBuffersTV(bench->screen, &bench->colour_buffer, &bench->depth_buffer, tv);
}

static br_colour gltfbench_clear_colour(br_uint_8 type)
{
    switch(type) {
        case BR_PMT_INDEX_8:
            return 0;
        case BR_PMT_RGB_555:
            return BR_COLOUR_RGB_555(0, 0, 0);
        case BR_PMT_RGB_565:
            return BR_COLOUR_RGB_565(0, 0, 0);
        default:
            return BR_COLOUR_RGBA(0, 0, 0, 255);
    }
}

/* ------------------------------------------------------------------ */
/* Scene.                                                             */
/* ------------------------------------------------------------------ */

/*
 * A material's lookup tables are INDEX_8 pixelmaps with no palette; the
 * rasterisers read their bytes directly, so they must not be requantised.
 */
static br_boolean gltfbench_is_lookup_table(const br_pixelmap *pm)
{
    return pm->type == BR_PMT_INDEX_8 && pm->map == NULL;
}

/*
 * Build the CLUT the indexed software paths need. The loader leaves a NULL
 * entry for an image it could not obtain, so the table can have holes in it.
 *
 * A frame whose indices come from a lookup-table band rather than a texture is
 * outside what this can describe and comes out black - deliberate, and the same
 * limitation gltfview records.
 */
static br_pixelmap *gltfbench_build_clut(br_pixelmap *const *maps, size_t nmaps)
{
    br_pixelmap *clut;

    clut = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL);
    BrPixelmapPixelSet(clut, 0, 0, 0);

    BrQuantBegin();
    for(size_t i = 0; i < nmaps; ++i) {
        br_pixelmap *pm = maps[i];

        if(pm == NULL || gltfbench_is_lookup_table(pm))
            continue;

        for(int y = 0; y < pm->height; y++) {
            for(int x = 0; x < pm->width; x++) {
                /* Assumes we're BR_PMT_RGBA_8888. */
                br_colour col = BrPixelmapPixelGet(pm, -pm->origin_x + x, -pm->origin_y + y);

                br_uint_8 rgb[3] = {
                    [0] = BR_RED(col),
                    [1] = BR_GRN(col),
                    [2] = BR_BLU(col),
                };

                BrQuantAddColours(rgb, 1);
            }
        }
    }

    BrQuantMakePalette(1, 255, clut);
    BrQuantEnd();

    return clut;
}

static br_error gltfbench_load_scene(gltfbench *bench)
{
    const gltfbench_args *args = bench->args;
    br_fmt_results       *results;
    br_pixelmap          *clut;

    br_gltf_options opts = {
        .pm_type = bench->colour_buffer->type,
    };

    if(bench->hw_accel) {
        opts.pm_type = BR_PMT_RGBA_8888;
    } else {
        switch(bench->colour_buffer->type) {
            case BR_PMT_INDEX_8:
                /*
                 * We build an optimal CLUT below.
                 */
                opts.pm_type = BR_PMT_RGBA_8888;
                break;
            case BR_PMT_RGB_565:
            case BR_PMT_RGB_555:
                opts.pm_type = BR_PMT_INDEX_8;
                break;

            default:
                break;
        }
    }

    results = BrFmtGLTFActorLoadMany(args->model, &opts);
    if(results == NULL || results->nactors == 0) {
        BrLogError("GLTFBENCH", "Failed to load %s", args->model);
        return BRE_FAIL;
    }

    for(br_size_t i = 0; i < results->nactors; ++i)
        BrActorAdd(bench->world, results->actors[i]);

    /*
     * Build an optimal CLUT for all our pixelmaps. This only matters if our
     * backbuffer is BR_PMT_INDEX_8, but it doesn't hurt to do.
     */
    clut           = gltfbench_build_clut(results->pixelmaps, results->npixelmaps);
    bench->palette = clut;
    BrPixelmapPaletteSet(bench->colour_buffer, clut);

    for(size_t i = 0; i < results->npixelmaps; ++i) {
        br_pixelmap *pm = results->pixelmaps[i];

        if(pm == NULL)
            continue;

        if(bench->colour_buffer->type == BR_PMT_INDEX_8 && !gltfbench_is_lookup_table(pm)) {
            /*
             * If our destination is indexed, re-quantise every colour map to our
             * palette by doing a conversion then cannibalising its parts.
             */
            const br_pixelmap_convert_options cvtopts = {
                .index_alpha_threshold = 0,
                .target_clut           = clut,
            };
            br_pixelmap *tmp = BrPixelmapConvert(pm, BR_PMT_INDEX_8, &cvtopts);

            pm->type      = BR_PMT_INDEX_8;
            pm->row_bytes = tmp->row_bytes;

            BrResFree(pm->pixels);
            pm->pixels = BrResAdd(pm, BrResRemove(tmp->pixels));

            if(pm->map != NULL)
                BrResFree(pm->map);

            pm->map = clut;

            BrPixelmapFree(tmp);
        } else {
            if(pm->map != NULL)
                BrTableUpdate(pm->map, BR_TABU_ALL);
        }

        BrMapUpdate(pm, BR_MAPU_ALL);
    }

    for(br_size_t i = 0; i < results->nmaterials; ++i)
        BrMaterialUpdate(results->materials[i], BR_MATU_ALL);

    for(br_size_t i = 0; i < results->nmodels; ++i) {
        if(results->models[i] != NULL)
            BrModelUpdate(results->models[i], BR_MODU_ALL);
    }

    return BRE_OK;
}

typedef struct gltfbench_bounds {
    br_vector3 min;
    br_vector3 max;
    br_boolean any;
} gltfbench_bounds;

static br_uint_32 BR_CALLBACK gltfbench_bounds_cb(br_actor *a, void *arg)
{
    gltfbench_bounds *bounds = arg;

    if(a->model != NULL) {
        const br_vector3 *min = &a->model->bounds.min;
        const br_vector3 *max = &a->model->bounds.max;

        if(!bounds->any) {
            bounds->min = *min;
            bounds->max = *max;
            bounds->any = BR_TRUE;
        } else {
            for(int i = 0; i < 3; ++i) {
                if(min->v[i] < bounds->min.v[i])
                    bounds->min.v[i] = min->v[i];

                if(max->v[i] > bounds->max.v[i])
                    bounds->max.v[i] = max->v[i];
            }
        }
    }

    BrActorEnum(a, gltfbench_bounds_cb, arg);
    return 0;
}

/*
 * gltfdemo's fixed orbit position: the camera sits dist from the scene centre,
 * angle degrees around it and elevation radians above it, and looks straight
 * at the centre.
 */
static br_error gltfbench_setup_camera(gltfbench *bench)
{
    const gltfbench_args *args   = bench->args;
    gltfbench_bounds      bounds = {0};
    br_vector3            centre;
    br_scalar             radius;
    br_scalar             dist;
    br_scalar             x, y, z;
    float                 dist_mul;
    float                 angle_rad;
    br_camera            *cam;

    BrActorEnum(bench->world, gltfbench_bounds_cb, &bounds);

    if(bounds.any) {
        br_scalar dx, dy, dz;

        for(int i = 0; i < 3; ++i)
            centre.v[i] = BR_MUL(BR_ADD(bounds.min.v[i], bounds.max.v[i]), BR_SCALAR(0.5f));

        dx     = BR_SUB(bounds.max.v[0], bounds.min.v[0]);
        dy     = BR_SUB(bounds.max.v[1], bounds.min.v[1]);
        dz     = BR_SUB(bounds.max.v[2], bounds.min.v[2]);
        radius = BR_MUL(BR_SCALAR(0.5f), BR_LENGTH3(dx, dy, dz));

        if(radius < BR_SCALAR(0.1f))
            radius = BR_SCALAR(5.0f);
    } else {
        centre = (br_vector3){
            {0, BR_SCALAR(4), 0}
        };
        radius = BR_SCALAR(8.0f);
    }

    /*
     * --dist is an absolute distance, as gltfdemo's is; the default is its
     * orbit multiplier of the scene radius.
     */
    dist_mul = (args->dist > 0.0f) ? args->dist / BrScalarToFloat(radius) : GLTFBENCH_DEFAULT_DIST_MUL;
    dist     = BR_MUL(radius, BR_SCALAR(dist_mul));

    angle_rad = args->angle * GLTFBENCH_PI / 180.0f;

    x = BR_ADD(centre.v[0], BR_MUL(dist, BR_SCALAR(cosf(args->elevation) * sinf(angle_rad))));
    z = BR_ADD(centre.v[2], BR_MUL(dist, BR_SCALAR(cosf(args->elevation) * cosf(angle_rad))));
    y = BR_ADD(centre.v[1], BR_MUL(dist, BR_SCALAR(sinf(args->elevation))));

    if((bench->camera = BrActorAdd(bench->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL))) == NULL) {
        BrLogError("GLTFBENCH", "Could not allocate the camera.");
        return BRE_FAIL;
    }

    cam                = bench->camera->type_data;
    cam->field_of_view = BR_ANGLE_DEG(GLTFBENCH_FOV_DEG);
    cam->hither_z      = BR_SCALAR(GLTFBENCH_HITHER);
    cam->yon_z         = BR_SCALAR(GLTFBENCH_YON);
    cam->aspect        = BR_DIV(BR_SCALAR(bench->colour_buffer->width), BR_SCALAR(bench->colour_buffer->height));

    bench->order_table->min_z = cam->hither_z;
    bench->order_table->max_z = cam->yon_z;

    bench->camera->t.type                = BR_TRANSFORM_LOOK_UP;
    bench->camera->t.t.look_up.look.v[0] = BR_SUB(centre.v[0], x);
    bench->camera->t.t.look_up.look.v[1] = BR_SUB(centre.v[1], y);
    bench->camera->t.t.look_up.look.v[2] = BR_SUB(centre.v[2], z);
    bench->camera->t.t.look_up.up.v[0]   = BR_SCALAR(0);
    bench->camera->t.t.look_up.up.v[1]   = BR_SCALAR(1);
    bench->camera->t.t.look_up.up.v[2]   = BR_SCALAR(0);
    bench->camera->t.t.look_up.t.v[0]    = x;
    bench->camera->t.t.look_up.t.v[1]    = y;
    bench->camera->t.t.look_up.t.v[2]    = z;

    return BRE_OK;
}

/* ------------------------------------------------------------------ */
/* Capture.                                                           */
/* ------------------------------------------------------------------ */

/*
 * The GL entry points BrCaptureReadbackGL() reads through. SDL gives us the
 * addresses, so the driver's glad loader stays out of this.
 */
static void gltfbench_load_gl(gltfbench *bench)
{
    bench->gl.ReadPixels  = (br_capture_pfn_ReadPixels)SDL_GL_GetProcAddress("glReadPixels");
    bench->gl.ReadBuffer  = (br_capture_pfn_ReadBuffer)SDL_GL_GetProcAddress("glReadBuffer");
    bench->gl.BindTexture = (br_capture_pfn_BindTexture)SDL_GL_GetProcAddress("glBindTexture");
    bench->gl.GetTexImage = (br_capture_pfn_GetTexImage)SDL_GL_GetProcAddress("glGetTexImage");
    bench->gl.PixelStorei = (br_capture_pfn_PixelStorei)SDL_GL_GetProcAddress("glPixelStorei");
}

/*
 * Convert the software colour buffer into the bottom-up RGBA8 block the PPM
 * export takes. An indexed buffer keeps its palette in the device CLUT, not
 * pm->map, so hand the palette over through a shadow of the source; the
 * buffer's own map is part of the checksum and is left alone.
 */
static br_boolean gltfbench_software_readback(const gltfbench *bench, br_uint_8 *rgba)
{
    br_pixelmap  shadow;
    br_pixelmap *src = bench->colour_buffer;
    br_pixelmap *conv;

    if(src->type == BR_PMT_INDEX_8 && src->map == NULL) {
        if(bench->palette == NULL) {
            BrLogError("GLTFBENCH", "Indexed colour buffer has no palette to present through.");
            return BR_FALSE;
        }

        shadow     = *src;
        shadow.map = bench->palette;
        src        = &shadow;
    }

    if((conv = BrPixelmapConvert(src, BR_PMT_RGBA_8888_ARR, NULL)) == NULL) {
        BrLogError("GLTFBENCH", "Could not convert the software frame to RGBA.");
        return BR_FALSE;
    }

    for(br_int_32 y = 0; y < conv->height; ++y) {
        const br_uint_8 *row = (const br_uint_8 *)conv->pixels + (br_size_t)y * conv->row_bytes;
        br_uint_8       *dst = rgba + (br_size_t)(conv->height - 1 - y) * (br_size_t)conv->width * 4;

        BrMemCpy(dst, row, (br_size_t)conv->width * 4);
    }

    BrPixelmapFree(conv);
    return BR_TRUE;
}

/*
 * Read the last measured frame back and report it. The frame is captured before
 * the present: glrend1x renders into the window's back buffer, and a swap leaves
 * it undefined.
 */
static br_error gltfbench_capture(gltfbench *bench)
{
    const gltfbench_args *args     = bench->args;
    br_boolean            want_ppm = args->ppm_path != NULL;
    br_uint_8            *rgba     = NULL;
    br_uint_64            hash     = 1469598103934665603ull;
    br_uint_32            covered = 0, total = 0;
    br_int_32             w = 0, h = 0;

    if(bench->hw_accel) {
        SDL_GetWindowSizeInPixels(bench->window, &w, &h);

        if(w <= 0 || h <= 0) {
            BrLogError("GLTFBENCH", "The window has no pixel size to read back.");
            return BRE_FAIL;
        }

        total = (br_uint_32)w * (br_uint_32)h;
        rgba  = BrResAllocate(NULL, (br_size_t)total * 4, BR_MEMORY_APPLICATION);

        if(rgba == NULL)
            return BRE_FAIL;

        if(!BrCaptureReadbackGL(&bench->gl, bench->colour_buffer, w, h, rgba)) {
            BrLogError("GLTFBENCH", "No glReadPixels or colour texture; the frame cannot be read back.");
            BrResFree(rgba);
            return BRE_FAIL;
        }

        for(br_uint_32 i = 0; i < total; ++i) {
            const br_uint_8 *p = rgba + (br_size_t)i * 4;

            for(int c = 0; c < 4; ++c) {
                hash ^= p[c];
                hash *= 1099511628211ull;
            }

            if(p[0] || p[1] || p[2])
                ++covered;
        }
    } else {
        br_pixelmap *pm = bench->colour_buffer;

        if(pm == NULL || pm->pixels == NULL || pm->width == 0 || pm->height == 0 || BrPixelmapPixelSize(pm) == 0) {
            BrLogError("GLTFBENCH", "No colour buffer to checksum.");
            return BRE_FAIL;
        }

        hash = BrCaptureHashPixelmap(hash, pm, &covered);

        if(pm->map != NULL && pm->map->pixels != NULL && pm->map->width != 0 && pm->map->height != 0 && BrPixelmapPixelSize(pm->map) != 0)
            hash = BrCaptureHashPixelmap(hash, pm->map, NULL);

        w     = pm->width;
        h     = pm->height;
        total = (br_uint_32)w * (br_uint_32)h;

        if(want_ppm) {
            rgba = BrResAllocate(NULL, (br_size_t)total * 4, BR_MEMORY_APPLICATION);

            if(rgba == NULL || !gltfbench_software_readback(bench, rgba)) {
                BrResFree(rgba);
                return BRE_FAIL;
            }
        }
    }

    if(args->checksum)
        printf("checksum=%016llx coverage=%.2f%%\n", (unsigned long long)hash, 100.0 * (double)covered / (double)total);

    if(want_ppm && BrCaptureExportPPM(args->ppm_path, rgba, w, h) != BRE_OK) {
        BrLogError("GLTFBENCH", "Could not write the frame to %s.", args->ppm_path);
        BrResFree(rgba);
        return BRE_FAIL;
    }

    if(rgba != NULL)
        BrResFree(rgba);

    return BRE_OK;
}

/* ------------------------------------------------------------------ */
/* The run.                                                           */
/* ------------------------------------------------------------------ */

static br_error gltfbench_run(gltfbench *bench, FILE *csv)
{
    const gltfbench_args *args       = bench->args;
    br_uint_32            total      = (br_uint_32)(args->warmup + args->frames);
    br_boolean            want_frame = args->checksum || args->ppm_path != NULL;

    for(br_uint_32 i = 0; i < total; ++i) {
        br_uint_64 start_ns = SDL_GetTicksNS();
        br_boolean measured = i >= (br_uint_32)args->warmup;
        br_boolean last     = (i + 1 == total);

        BrRendererFrameBegin();

        BrPixelmapFill(bench->colour_buffer, bench->clear_colour);
        BrPixelmapFill(bench->depth_buffer, 0xFFFFFFFF);
        BrZbSceneRender(bench->world, bench->camera, bench->colour_buffer, bench->depth_buffer);

        BrRendererFrameEnd();

        if(last && want_frame && gltfbench_capture(bench) != BRE_OK)
            return BRE_FAIL;

        BrPixelmapDoubleBuffer(bench->screen, bench->colour_buffer);

        if(measured && csv != NULL) {
            double ms = (double)(SDL_GetTicksNS() - start_ns) / 1e6;

            if(fprintf(csv, "%.3f\n", ms) < 0) {
                BrLogError("GLTFBENCH", "Could not write the frame time to the CSV.");
                return BRE_FAIL;
            }
        }
    }

    return BRE_OK;
}

int main(int argc, char **argv)
{
    gltfbench      bench = {0};
    gltfbench_args args;
    FILE          *csv = NULL;
    int            ret = 1;
    int            parse;

    parse = gltfbench_parse_args(argc, argv, &args);

    switch(parse) {
        case GLTFBENCH_PARSE_HELP:
            gltfbench_usage(stdout, argv[0]);
            return 0;

        case GLTFBENCH_PARSE_ERROR:
            gltfbench_usage(stderr, argv[0]);
            return 2;

        case GLTFBENCH_PARSE_OK:
            break;
    }

    bench.args = &args;

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    BrBegin();

    if(gltfbench_create_screen(&bench) != BRE_OK)
        goto cleanup;

    bench.screen->origin_x = (br_int_16)(bench.screen->width >> 1);
    bench.screen->origin_y = (br_int_16)(bench.screen->height >> 1);

    if(gltfbench_resize_buffers(&bench) != BRE_OK) {
        BrLogError("GLTFBENCH", "Error allocating the colour and depth buffers.");
        goto cleanup;
    }

    bench.clear_colour = gltfbench_clear_colour(bench.colour_buffer->type);

    BrRendererBegin(bench.colour_buffer, NULL, NULL, gltfbench_primitive_heap, sizeof(gltfbench_primitive_heap));

    if((bench.world = BrActorAllocate(BR_ACTOR_NONE, NULL)) == NULL) {
        BrLogError("GLTFBENCH", "Could not allocate the world.");
        goto cleanup;
    }

    if((bench.order_table = BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_LEAVE_BOUNDS | BR_ORDER_TABLE_BUCKET_SORT, BR_SORT_AVERAGE)) == NULL) {
        BrLogError("GLTFBENCH", "Could not allocate the order table.");
        goto cleanup;
    }

    BrZsActorOrderTableSet(bench.world, bench.order_table);

    if(gltfbench_load_scene(&bench) != BRE_OK || gltfbench_setup_camera(&bench) != BRE_OK)
        goto cleanup;

    if(bench.hw_accel)
        gltfbench_load_gl(&bench);

    if(args.csv_path != NULL && (csv = fopen(args.csv_path, "w")) == NULL) {
        BrLogError("GLTFBENCH", "Could not open %s for writing.", args.csv_path);
        goto cleanup;
    }

    if(gltfbench_run(&bench, csv) == BRE_OK)
        ret = 0;

cleanup:
    if(csv != NULL)
        fclose(csv);

    if(BrV1dbRendererQuery() != NULL)
        BrRendererEnd();

    BrPixelmapFree(bench.depth_buffer);
    BrPixelmapFree(bench.colour_buffer);
    BrPixelmapFree(bench.screen);

    if(bench.world != NULL)
        BrActorFree(bench.world);

    BrEnd();

    return ret;
}
