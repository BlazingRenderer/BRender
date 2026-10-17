#include <stdio.h>
#include <inttypes.h>

#include <SDL3/SDL.h>
#include <brender.h>
#include <brglrend.h>
#include <brglrend1x.h>
#include <brsdl3dev.h>
#include <brvkrend.h>
#include "parg.h"

#include "brdemo.h"

struct br_device *BR_EXPORT BrDrv1SoftPrimBegin(const char *arguments);
struct br_device *BR_EXPORT BrDrv1SoftRendBegin(const char *arguments);

/* begin hook */
void _BrBeginHook(void) // NOLINT(*-reserved-identifier)
{
    struct br_device *sdl = NULL;

    BrDevAddStatic(&sdl, BrDrv1SDL3Begin, NULL);
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);
    BrDevAddStatic(NULL, BrDrv1GL1xBegin, NULL);
    BrDevAddStatic(NULL, BrDrv1SoftPrimBegin, NULL);
    BrDevAddStatic(NULL, BrDrv1SoftRendBegin, NULL);

    if(sdl != NULL) {
        char buf[48];
        BrSprintfN(buf, sizeof(buf), "VKREND_INIT_DEVICE=%" PRIuPTR, (br_uintptr_t)sdl);
        BrDevAddStatic(NULL, BrDrv1VkBegin, buf);
    }
}

/* end hook */
void _BrEndHook(void) // NOLINT(*-reserved-identifier)
{
}

br_error BrDemoDefaultInit(br_demo *demo)
{
    (void)demo;
    return BRE_OK;
}

void BrDemoDefaultProcessEvent(br_demo *demo, const SDL_Event *evt)
{
    (void)demo;
    (void)evt;
}

void BrDemoDefaultUpdate(br_demo *demo, br_scalar dt)
{
    (void)demo;
    (void)dt;
}

void BrDemoDefaultRender(br_demo *demo)
{
    BrRendererFrameBegin();
    BrPixelmapFill(demo->colour_buffer, demo->clear_colour);
    BrPixelmapFill(demo->depth_buffer, 0xFFFFFFFF);

    if(demo->camera != NULL) {
        if(demo->args->no_depth)
            BrZsSceneRender(demo->world, demo->camera, demo->colour_buffer);
        else
            BrZbSceneRender(demo->world, demo->camera, demo->colour_buffer, demo->depth_buffer);
    }

    BrRendererFrameEnd();
}

void BrDemoDefaultOnResize(br_demo *demo, br_uint_16 width, br_uint_16 height)
{
    /*
     * Update the camera's aspect ratio to match the camera.
     */
    if(demo->camera != NULL) {
        br_camera *camera_data = demo->camera->type_data;
        camera_data->aspect    = BR_DIV(BR_SCALAR(width), BR_SCALAR(height));
    }
}

void BrDemoDefaultDestroy(br_demo *demo)
{
    (void)demo;
}

const static br_demo_dispatch default_dispatch = {
    .init          = BrDemoDefaultInit,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = BrDemoDefaultUpdate,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

/*
 * Custom load function to de-CLUT a pixelmap upon loading.
 */

/* FIXME: de-global this */
static br_demo *current_demo = NULL;

static br_pixelmap *BR_CALLBACK MapFindFailedLoadDeCLUT(const char *name)
{
    br_pixelmap *pm, *pm2;
    br_pixelmap *palette = current_demo != NULL ? current_demo->palette : NULL;

    if((pm = BrPixelmapLoad(name)) != NULL) {
        if(pm->type == BR_PMT_INDEX_8 && pm->map == NULL)
            pm->map = palette;

#if 0
        if((pm2 = BrPixelmapDeCLUT(pm)) != NULL) {
            if(pm->map != palette)
                BrResFree(pm->map);

            BrResFree(pm);
            pm = pm2;
        }
#endif

        if(pm->identifier != NULL)
            BrResFree(pm->identifier);

        pm->identifier = BrResStrDup(pm, name);

        if(pm->map != NULL && pm->map != palette) {
            /*
             * If there's a non-default palette, give it a (hopefully) unique name
             * so it can be added to the registry.
             */
            if(pm->map->identifier != NULL)
                BrResFree(pm->map->identifier);

            pm->map->identifier = BrResSprintf(pm->map, "%s(palette)", pm->identifier);
            BrTableAdd(pm->map);
        }

        BrMapAdd(pm);
    }

    return pm;
}

static br_error resize_buffers(br_demo *demo, const br_demo_run_args *args)
{
    br_int_32 width  = args->backbuffer_width;
    br_int_32 height = args->backbuffer_height;

    if(width <= 0)
        width = demo->_screen->width;

    if(height <= 0)
        height = demo->_screen->height;

    if(!demo->hw_accel) {
        const br_uint_8      software_type = (args->pm_type == BR_PMT_MAX) ? BR_PMT_INDEX_8 : args->pm_type;
        const br_token_value tv[]          = {
            {.t = BRT_WIDTH_I32,      .v = {.i32 = width}       },
            {.t = BRT_HEIGHT_I32,     .v = {.i32 = height}      },
            {.t = BRT_PIXEL_TYPE_U8,  .v = {.u8 = software_type}},
            {.t = BRT_PIXEL_BITS_I32, .v = {.i32 = 16}          },
            {.t = BR_NULL_TOKEN,      .v = {}                   },
        };
        return BrPixelmapResizeBuffersTV(demo->_screen, &demo->colour_buffer, &demo->depth_buffer, tv);
    }

    /*
     * The hardware colour buffer type is the driver's own unless the caller
     * asked for one; passing BR_PMT_MAX would name no type at all.
     */
    br_token_value tv[4];
    int            n = 0;

    tv[n].t       = BRT_WIDTH_I32;
    tv[n++].v.i32 = width;
    tv[n].t       = BRT_HEIGHT_I32;
    tv[n++].v.i32 = height;

    if(args->pm_type != BR_PMT_MAX) {
        tv[n].t      = BRT_PIXEL_TYPE_U8;
        tv[n++].v.u8 = args->pm_type;
    }

    tv[n].t = BR_NULL_TOKEN;

    return BrPixelmapResizeBuffersTV(demo->_screen, &demo->colour_buffer, &demo->depth_buffer, tv);
}

static br_error create_window(br_demo *demo, const br_demo_run_args *args)
{
    br_error err;

    if(args->force_software)
        goto try_software;

#if 0
    char *devs = BrResSprintf(demo, "SDL3,WIDTH=%d,HEIGHT=%d,WINDOW_NAME=\"%s\",HIDPI=1,RESIZABLE=1,OPENGL=%d,OPENGL_DEVICE_NAME=%s",
                              args->width, args->height, args->title, !args->force_software, args->opengl_device_name);
    err = BrDevBeginVar(&demo->_screen, devs, BR_NULL_TOKEN);
#endif

    // clang-format off
    err = BrDevBeginVar(&demo->_screen,              "SDL3",
                        BRT_WIDTH_I32,               args->width,
                        BRT_HEIGHT_I32,              args->height,
                        BRT_WINDOW_NAME_CSTR,        args->title,
                        BRT_HIDPI_B,                 BR_TRUE,
                        BRT_RESIZABLE_B,             BR_TRUE,
                        BRT_OPENGL_B,                args->vulkan ? BR_FALSE : BR_TRUE,
                        BRT_VULKAN_B,                args->vulkan ? BR_TRUE : BR_FALSE,
                        BRT_OPENGL_DEVICE_NAME_CSTR, args->opengl_device_name, /* NB: this is optional */
                        BR_NULL_TOKEN);
    // clang-format on

    if(err == BRE_OK) {
        /*
         * Only the OpenGL path owns a GL context to set a swap interval on.
         */
        if(!args->vulkan)
            SDL_GL_SetSwapInterval(1);

        demo->hw_accel = BR_TRUE;
        return err;
    }

    if(args->require_accel) {
        BrLogError("DEMO", "Unable to create a hardware-accelerated window for device `%s', and this run requires one rather than a software fallback.",
                   args->opengl_device_name);
        return err;
    }

    BrLogWarn("DEMO", "Unable to create window, falling back to software.");

try_software:
    // clang-format off
    err = BrDevBeginVar(&demo->_screen,       "SDL3",
                        BRT_WIDTH_I32,        args->width,
                        BRT_HEIGHT_I32,       args->height,
                        BRT_WINDOW_NAME_CSTR, args->title,
                        BRT_HIDPI_B,          BR_FALSE,
                        BRT_RESIZABLE_B,      BR_TRUE,
                        BRT_OPENGL_B,         BR_FALSE,
                        BR_NULL_TOKEN);
    // clang-format on
    if(err != BRE_OK) {
        BrLogError("DEMO", "Unable to create window.");
    }

    demo->hw_accel = BR_FALSE;
    return err;
}

int BrDemoRunArg(const br_demo_dispatch *dispatch, const br_demo_run_args *args)
{
    int         ret           = 1;
    br_demo    *demo          = NULL;
    SDL_Window *window        = NULL;
    br_boolean  is_fullscreen = BR_FALSE;
    br_boolean  did_init      = BR_FALSE;
    br_uint_64  ticks_now;
    br_uint_64  ticks_last;

    if(dispatch == NULL)
        dispatch = &default_dispatch;

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    BrBegin();

    BrModelFindHook(BrModelFindFailedLoad);
    BrMapFindHook(MapFindFailedLoadDeCLUT);
    BrMaterialFindHook(BrMaterialFindFailedLoad);
    BrTableFindHook(BrTableFindFailedLoad);

    demo           = BrResAllocate(NULL, sizeof(br_demo), BR_MEMORY_APPLICATION);
    demo->dispatch = dispatch;
    demo->args     = args;

    /* FIXME: hack to work around no args in the find hooks. */
    current_demo = demo;

    /*
     * Create the window.
     */
    if(create_window(demo, args) != BRE_OK)
        goto cleanup;

    demo->_screen->origin_x = (br_int_16)(demo->_screen->width >> 1);
    demo->_screen->origin_y = (br_int_16)(demo->_screen->height >> 1);

    /*
     * Windowed mode initially.
     */
    window = BrSDL3UtilGetWindow(demo->_screen);
    SDL_SetWindowFullscreen(window, 0);
    is_fullscreen = BR_FALSE;

    if(resize_buffers(demo, args) != BRE_OK) {
        BrLogError("DEMO", "Error resizing window buffers.");
        goto cleanup;
    }

    /*
     * Set appropriate colour defaults.
     */
    switch(demo->colour_buffer->type) {
        case BR_PMT_INDEX_8:
            demo->clear_colour = 0;
            demo->text_colour  = 255;
            break;
        case BR_PMT_RGB_555:
            demo->clear_colour = BR_COLOUR_RGB_555(0, 0, 0);
            demo->text_colour  = BR_COLOUR_RGB_555(255, 255, 255);
            break;
        case BR_PMT_RGB_565:
            demo->clear_colour = BR_COLOUR_RGB_565(0, 0, 0);
            demo->text_colour  = BR_COLOUR_RGB_565(255, 255, 255);
            break;
        default:
            demo->clear_colour = BR_COLOUR_RGBA(0, 0, 0, 255);
            demo->text_colour  = BR_COLOUR_RGBA(255, 255, 255, 255);
            break;
    }

    BrRendererBegin(demo->colour_buffer, NULL, NULL, demo->_primitive_heap, sizeof(demo->_primitive_heap));

    demo->order_table = BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_LEAVE_BOUNDS | BR_ORDER_TABLE_BUCKET_SORT, BR_SORT_AVERAGE);

    demo->world = BrActorAllocate(BR_ACTOR_NONE, NULL);
    BrZsActorOrderTableSet(demo->world, demo->order_table);

    /*
     * The destroy callback must only run if init was reached: creating the
     * window can fail first, and a destroy that assumes its own init ran will
     * dereference state that was never set up.
     */
    did_init = BR_TRUE;

    if(demo->dispatch->init(demo) != BRE_OK) {
        BrLogError("DEMO", "Initialisation failed.");
        goto cleanup;
    }

    demo->dispatch->on_resize(demo, demo->_screen->width, demo->_screen->height);

    ticks_last = SDL_GetTicksNS();

    for(SDL_Event evt;;) {
        float dt;

        ticks_now  = SDL_GetTicksNS();
        dt         = (float)(ticks_now - ticks_last) / 1e9f;
        ticks_last = ticks_now;

        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_EVENT_QUIT:
                    ret = 0;
                    goto cleanup;

                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    /*
                     * Window event, pass it to the driver.
                     */
                    if(BrPixelmapHandleWindowEvent(demo->_screen, &evt.window) != BRE_OK) {
                        BrLogError("DEMO", "Error handling window event.");
                        goto cleanup;
                    }

                    /*
                     * The main screen should have been resized above.
                     * Update its origin and resize the framebuffer.
                     */
                    demo->_screen->origin_x = (br_int_16)(demo->_screen->width >> 1);
                    demo->_screen->origin_y = (br_int_16)(demo->_screen->height >> 1);

                    if(resize_buffers(demo, args) != BRE_OK) {
                        BrLogError("DEMO", "Error resizing window buffers.");
                        goto cleanup;
                    }

                    demo->dispatch->on_resize(demo, demo->_screen->width, demo->_screen->height);
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

            demo->dispatch->process_event(demo, &evt);
        }

        demo->dispatch->update(demo, BR_SCALAR(dt));
        demo->dispatch->render(demo);

        if(!demo->args->no_stats) {
            br_int_32 base_x = -demo->colour_buffer->origin_x + 5;
            br_int_32 base_y = -demo->colour_buffer->origin_y + 5;

            BrPixelmapTextF(demo->colour_buffer, base_x, base_y, demo->text_colour, BrFontProp7x9, "last frame delta (msec): %f", dt * 1000);
        }

        BrPixelmapDoubleBuffer(demo->_screen, demo->colour_buffer);
    }

cleanup:

    if(did_init)
        demo->dispatch->destroy(demo);

    if(BrV1dbRendererQuery() != NULL)
        BrRendererEnd();

    if(demo->depth_buffer != NULL)
        BrPixelmapFree(demo->depth_buffer);

    if(demo->colour_buffer != NULL)
        BrPixelmapFree(demo->colour_buffer);

    if(demo->_screen != NULL)
        BrPixelmapFree(demo->_screen);

    if(demo->world != NULL)
        BrActorFree(demo->world);

    BrResFree(demo);
    BrEnd();
    return ret;
}

#define ARGDEF_WIDTH              'w'
#define ARGDEF_HEIGHT             'h'
#define ARGDEF_VERBOSE            'v'
#define ARGDEF_HELP               301
#define ARGDEF_FORCE_SOFTWARE     302
#define ARGDEF_BPP                303
#define ARGDEF_BACKBUFFER_WIDTH   304
#define ARGDEF_BACKBUFFER_HEIGHT  305
#define ARGDEF_OPENGL_DEVICE_NAME 306
#define ARGDEF_NO_STATS           307
#define ARGDEF_NO_DEPTH           308
#define ARGDEF_VULKAN             309

/* Application options are numbered from here, in table order. */
#define BR_DEMO_OPT_EXTRA 1000

const static struct parg_option argdefs[] = {
    {.name = "width",              .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_WIDTH             },
    {.name = "height",             .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_HEIGHT            },
    {.name = "verbose",            .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_VERBOSE           },
    {.name = "help",               .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_HELP              },
    {.name = "force-software",     .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_FORCE_SOFTWARE    },
    {.name = "bpp",                .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_BPP               },
    {.name = "backbuffer-width",   .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_BACKBUFFER_WIDTH  },
    {.name = "backbuffer-height",  .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_BACKBUFFER_HEIGHT },
    {.name = "opengl-device-name", .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_OPENGL_DEVICE_NAME},
    {.name = "no-stats",           .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_NO_STATS          },
    {.name = "no-depth",           .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_NO_DEPTH          },
    {.name = "vulkan",             .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_VULKAN            },
    {.name = NULL,                 .has_arg = 0,           .flag = NULL, .val = 0                        },
};

static br_demo_parse parse_args(int argc, char *const argv[], br_demo_run_args *args, const br_demo_option *extra, br_size_t nextra,
                                void *ctx, int *first_positional)
{
    struct parg_state ps;
    parg_init(&ps);
    int bpp = 0;
    int optend;

    /*
     * The demo's options and the application's are parsed as one table: parg
     * needs each option's arity to know whether to consume the next argument, so
     * a table holding only half of them would misparse the other half.
     *
     * NB: Yes, alloca() is dangerous, but realistically not an issue here. Nothing
     * in here allocates, which is what lets it run before BrBegin().
     */
    br_size_t           nopts     = BR_ASIZE(argdefs) + nextra;
    struct parg_option *opts      = alloca(sizeof(*opts) * nopts);
    char               *optstring = alloca(6 + nextra * 3);
    char              **tmp_argv  = alloca(sizeof(char *) * (argc + 1));
    br_size_t           olen      = 0;

    BrMemCpy(opts, argdefs, sizeof(*opts) * (BR_ASIZE(argdefs) - 1));

    optstring[olen++] = 'w';
    optstring[olen++] = ':';
    optstring[olen++] = 'h';
    optstring[olen++] = ':';
    optstring[olen++] = 'v';

    for(br_size_t i = 0; i < nextra; ++i) {
        const br_demo_option *e = extra + i;

        if(e->name == NULL || e->apply == NULL)
            return BR_DEMO_PARSE_ERROR;

        /*
         * Refuse a name collision rather than shadow one of the demo's own.
         */
        for(const struct parg_option *a = argdefs; a->name != NULL; ++a) {
            if(BrStrCmp(a->name, e->name) == 0)
                return BR_DEMO_PARSE_ERROR;
        }

        opts[BR_ASIZE(argdefs) - 1 + i] = (struct parg_option){
            .name    = e->name,
            .has_arg = e->has_arg == BR_DEMO_ARG_REQUIRED ? PARG_REQARG : PARG_NOARG,
            .flag    = NULL,
            .val     = BR_DEMO_OPT_EXTRA + (int)i,
        };

        if(e->short_name != 0) {
            optstring[olen++] = e->short_name;
            if(e->has_arg == BR_DEMO_ARG_REQUIRED)
                optstring[olen++] = ':';
        }
    }

    optstring[olen] = '\0';
    opts[nopts - 1] = (struct parg_option){.name = NULL, .has_arg = 0, .flag = NULL, .val = 0};

    BrMemCpy(tmp_argv, argv, argc * sizeof(char *));
    tmp_argv[argc] = NULL;

    if((optend = parg_reorder(argc, tmp_argv, optstring, opts)) < 0)
        return BR_DEMO_PARSE_ERROR;

    for(int c; (c = parg_getopt_long(&ps, optend, tmp_argv, optstring, opts, NULL)) != -1;) {

        if(c >= BR_DEMO_OPT_EXTRA) {
            const br_demo_option *e = extra + (c - BR_DEMO_OPT_EXTRA);

            if(e->apply(ctx, ps.optarg) != 0)
                return BR_DEMO_PARSE_ERROR;

            continue;
        }

        switch(c) {

            case ARGDEF_WIDTH:
                args->width = BrAToI(ps.optarg);
                break;

            case ARGDEF_HEIGHT:
                args->height = BrAToI(ps.optarg);
                break;

            case ARGDEF_VERBOSE:
                ++args->verbose;
                break;

            case ARGDEF_FORCE_SOFTWARE:
                ++args->force_software;
                break;

            case ARGDEF_BPP:
                bpp = BrAToI(ps.optarg);
                break;

            case ARGDEF_BACKBUFFER_WIDTH:
                args->backbuffer_width = BrAToI(ps.optarg);
                break;

            case ARGDEF_BACKBUFFER_HEIGHT:
                args->backbuffer_height = BrAToI(ps.optarg);
                break;

            case ARGDEF_OPENGL_DEVICE_NAME:
                args->opengl_device_name = ps.optarg;
                break;

            case ARGDEF_NO_STATS:
                args->no_stats = 1;
                break;

            case ARGDEF_NO_DEPTH:
                args->no_depth = 1;
                break;

            case ARGDEF_VULKAN:
                args->vulkan = 1;
                break;

            case ARGDEF_HELP:
                return BR_DEMO_PARSE_HELP;

            case '?':
            case ':':
            default:
                return BR_DEMO_PARSE_ERROR;
        }
    }

    if(args->width < 0)
        return BR_DEMO_PARSE_ERROR;

    if(args->height < 0)
        return BR_DEMO_PARSE_ERROR;

    if(bpp != 0) {
        switch(bpp) {
            case 16:
                args->pm_type = BR_PMT_RGB_565;
                break;
            case 15:
                args->pm_type = BR_PMT_RGB_555;
                break;
            case 24:
                args->pm_type = BR_PMT_RGB_888;
                break;
            case 32:
                args->pm_type = BR_PMT_RGBA_8888;
                break;
            default:
            case 8:
                args->pm_type = BR_PMT_INDEX_8;
                break;
        }
    }

    if(args->backbuffer_width < 0)
        args->backbuffer_width = 0;

    if(args->backbuffer_height < 0)
        args->backbuffer_height = 0;

    if(args->opengl_device_name == NULL)
        args->opengl_device_name = "glrend";

    if(BrStrCmp("glrend", args->opengl_device_name) != 0 && BrStrCmp("glrend1x", args->opengl_device_name) != 0)
        return BR_DEMO_PARSE_ERROR;

    if(args->title == NULL || args->title[0] == '\0')
        args->title = "BRender Application";

    args->pos_argc = argc - optend;
    if(args->pos_argc > BR_ASIZE(args->pos_argv)) {
        args->pos_argc = BR_ASIZE(args->pos_argv);
    }

    memcpy(args->pos_argv, tmp_argv + optend, args->pos_argc * sizeof(char *));

    if(first_positional != NULL)
        *first_positional = optend;

    return BR_DEMO_PARSE_OK;
}

// clang-format off
static const char *usage_options =
    "  -w, --width          Set the initial window width (default: 1280).\n"
    "  -h, --height         Set the initial window height (default: 720).\n"
    "  -v, --verbose        Increase verbosity level. May be given multiple times.\n"
    "  --force-software     Force use of the software renderer. Only valid on x86.\n"
    "  --bpp                Colour buffer pixel type. Valid options: 8, 15, 16, 24, 32 (unset leaves the renderer's own).\n"
    "  --backbuffer-width   Width of the backbuffer, defaults to the same as the screen.\n"
    "  --backbuffer-height  Height of the backbuffer, defaults to the same as the screen.\n"
    "  --opengl-device-name Name of the OpenGL device driver to use. Valid options: glrend (default), glrend1x\n"
    "  --no-stats           Suppress the on-screen frame-time text.\n"
    "  --no-depth           Render with Z-sorting and no depth buffer.\n"
    "  --vulkan             Present the window through the Vulkan front-screen driver.\n"
    "  --help               Display this message.\n"
    "\n"
;
// clang-format on

void BrDemoDefaultArgs(br_demo_run_args *args)
{
    *args = (br_demo_run_args){
        .title              = NULL,
        .width              = 1280,
        .height             = 720,
        .verbose            = BR_LOG_INFO,
        .force_software     = 0,
        .require_accel      = 0,
        .pm_type            = BR_PMT_MAX,
        .opengl_device_name = "glrend",
        .vulkan             = 0,
        .backbuffer_width   = 0,
        .backbuffer_height  = 0,
        .no_stats           = 0,
        .no_depth           = 0,
    };
}

br_demo_parse BrDemoParseArgs(br_demo_run_args *args, int argc, char *const *argv, const br_demo_option *extra, br_size_t nextra, void *ctx,
                              int *first_positional)
{
    return parse_args(argc, argv, args, extra, nextra, ctx, first_positional);
}

void BrDemoUsage(FILE *out, const char *argv0, const br_demo_option *extra, br_size_t nextra)
{
    fprintf(out, "Usage: %s \nOptions:\n%s", argv0, usage_options);

    if(nextra == 0)
        return;

    fprintf(out, "Application options:\n");

    for(br_size_t i = 0; i < nextra; ++i) {
        const br_demo_option *e = extra + i;
        char                  left[80];

        if(e->metavar != NULL)
            snprintf(left, sizeof(left), "--%s %s", e->name, e->metavar);
        else
            snprintf(left, sizeof(left), "--%s", e->name);

        fprintf(out, "  %-24s %s\n", left, e->help != NULL ? e->help : "");
    }
}

int BrDemoRunArgv(const char *title, const br_demo_dispatch *dispatch, int argc, char *const *argv)
{
    int              r;
    br_demo_run_args args;

    BrDemoDefaultArgs(&args);
    args.title = title;

    switch(BrDemoParseArgs(&args, argc, argv, NULL, 0, NULL, NULL)) {
        case BR_DEMO_PARSE_HELP:
        case BR_DEMO_PARSE_ERROR:
            BrDemoUsage(stderr, argv[0], NULL, 0);
            return 2;

        case BR_DEMO_PARSE_OK:
            break;
    }

    BrLogSetLevel(args.verbose);

    return BrDemoRunArg(dispatch, &args);
}

int BrDemoRun(const char *title, br_uint_16 width, br_uint_16 height, const br_demo_dispatch *dispatch)
{
    br_demo_run_args args;

    BrDemoDefaultArgs(&args);
    args.title  = title;
    args.width  = width;
    args.height = height;

    return BrDemoRunArg(dispatch, &args);
}
