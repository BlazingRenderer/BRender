#include <stdio.h>

#include <SDL3/SDL.h>
#include <brender.h>
#include <brglrend.h>
#include <brsdl3dev.h>
#include "parg.h"

#include "brdemo.h"

struct br_device *BR_EXPORT BrDrv1SoftPrimBegin(const char *arguments);
struct br_device *BR_EXPORT BrDrv1SoftRendBegin(const char *arguments);

/* begin hook */
void _BrBeginHook(void) // NOLINT(*-reserved-identifier)
{
    BrDevAddStatic(NULL, BrDrv1SDL3Begin, NULL);
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);

#if HAVE_SOFTPRIM
    BrDevAddStatic(NULL, BrDrv1SoftPrimBegin, NULL);
#endif
    BrDevAddStatic(NULL, BrDrv1SoftRendBegin, NULL);
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

    if(demo->camera != NULL)
        BrZbSceneRender(demo->world, demo->camera, demo->colour_buffer, demo->depth_buffer);

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

typedef struct br_demo_run_args {
    const char *title;
    br_int_32   width;
    br_int_32   height;
    int         verbose;
    int         force_software;
    br_uint_8   software_pm_type;
    br_int_32   backbuffer_width;
    br_int_32   backbuffer_height;
} br_demo_run_args;

static br_error resize_buffers(br_demo *demo, const br_demo_run_args *args)
{
    br_int_32 width  = args->backbuffer_width;
    br_int_32 height = args->backbuffer_height;

    if(width <= 0)
        width = demo->_screen->width;

    if(height <= 0)
        height = demo->_screen->height;

    if(!demo->hw_accel) {
        const br_token_value tv[] = {
            {.t = BRT_WIDTH_I32,      .v = {.i32 = width}                },
            {.t = BRT_HEIGHT_I32,     .v = {.i32 = height}               },
            {.t = BRT_PIXEL_TYPE_U8,  .v = {.u8 = args->software_pm_type}},
            {.t = BRT_PIXEL_BITS_I32, .v = {.i32 = 16}                   },
            {.t = BR_NULL_TOKEN,      .v = {}                            },
        };
        return BrPixelmapResizeBuffersTV(demo->_screen, &demo->colour_buffer, &demo->depth_buffer, tv);
    }

    const br_token_value tv[] = {
        {.t = BRT_WIDTH_I32,  .v = {.i32 = width} },
        {.t = BRT_HEIGHT_I32, .v = {.i32 = height}},
        {.t = BR_NULL_TOKEN,  .v = {}             },
    };
    return BrPixelmapResizeBuffersTV(demo->_screen, &demo->colour_buffer, &demo->depth_buffer, tv);
}

static br_error create_window(br_demo *demo, const br_demo_run_args *args)
{
    br_error err;

    if(args->force_software)
        goto try_software;

#if 0
    char *devs = BrResSprintf(demo, "SDL3,WIDTH=%d,HEIGHT=%d,WINDOW_NAME=\"%s\",HIDPI=1,RESIZABLE=1,OPENGL=%d",
                              args->width, args->height, args->title, !args->force_software);
    err = BrDevBeginVar(&demo->_screen, devs, BR_NULL_TOKEN);
#endif

    // clang-format off
    err = BrDevBeginVar(&demo->_screen,       "SDL3",
                        BRT_WIDTH_I32,        args->width,
                        BRT_HEIGHT_I32,       args->height,
                        BRT_WINDOW_NAME_CSTR, args->title,
                        BRT_HIDPI_B,          BR_TRUE,
                        BRT_RESIZABLE_B,      BR_TRUE,
                        BRT_OPENGL_B,         BR_TRUE,
                        BR_NULL_TOKEN);
    // clang-format on
    if(err == BRE_OK) {
        demo->hw_accel = BR_TRUE;
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

static int BrDemoRunArg(const br_demo_dispatch *dispatch, const br_demo_run_args *args)
{
    int         ret           = 1;
    br_demo    *demo          = NULL;
    SDL_Window *window        = NULL;
    br_boolean  is_fullscreen = BR_FALSE;
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

        {
            br_int_32 base_x = -demo->colour_buffer->origin_x + 5;
            br_int_32 base_y = -demo->colour_buffer->origin_y + 5;

            BrPixelmapTextF(demo->colour_buffer, base_x, base_y, demo->text_colour, BrFontProp7x9, "last frame delta (msec): %f", dt * 1000);
        }

        BrPixelmapDoubleBuffer(demo->_screen, demo->colour_buffer);
    }

cleanup:

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

#define ARGDEF_WIDTH             'w'
#define ARGDEF_HEIGHT            'h'
#define ARGDEF_VERBOSE           'v'
#define ARGDEF_HELP              301
#define ARGDEF_FORCE_SOFTWARE    302
#define ARGDEF_SOFTWARE_BPP      303
#define ARGDEF_BACKBUFFER_WIDTH  304
#define ARGDEF_BACKBUFFER_HEIGHT 305

const static struct parg_option argdefs[] = {
    {.name = "width",             .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_WIDTH            },
    {.name = "height",            .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_HEIGHT           },
    {.name = "verbose",           .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_VERBOSE          },
    {.name = "help",              .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_HELP             },
    {.name = "force-software",    .has_arg = PARG_NOARG,  .flag = NULL, .val = ARGDEF_FORCE_SOFTWARE   },
    {.name = "software-bpp",      .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_SOFTWARE_BPP     },
    {.name = "backbuffer-width",  .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_BACKBUFFER_WIDTH },
    {.name = "backbuffer-height", .has_arg = PARG_REQARG, .flag = NULL, .val = ARGDEF_BACKBUFFER_HEIGHT},
    {.name = NULL,                .has_arg = 0,           .flag = NULL, .val = 0                       },
};

static int parse_args(int argc, char *const argv[], br_demo_run_args *args)
{
    struct parg_state ps;
    parg_init(&ps);
    int software_bpp = 8;

    for(int c; (c = parg_getopt_long(&ps, argc, argv, "w:h:v", argdefs, NULL)) != -1;) {
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

            case ARGDEF_SOFTWARE_BPP:
                software_bpp = BrAToI(ps.optarg);
                break;

            case ARGDEF_BACKBUFFER_WIDTH:
                args->backbuffer_width = BrAToI(ps.optarg);
                break;

            case ARGDEF_BACKBUFFER_HEIGHT:
                args->backbuffer_height = BrAToI(ps.optarg);
                break;

            case ARGDEF_HELP:
            case '?':
            case ':':
            default:
                return 2;
        }
    }

    if(args->width < 0)
        return 2;

    if(args->height < 0)
        return 2;

    switch(software_bpp) {
        case 16:
            args->software_pm_type = BR_PMT_RGB_565;
            break;
        case 15:
            args->software_pm_type = BR_PMT_RGB_555;
            break;
        case 24:
            args->software_pm_type = BR_PMT_RGB_888;
            break;
        default:
        case 8:
            args->software_pm_type = BR_PMT_INDEX_8;
            break;
    }

    if(args->backbuffer_width < 0)
        args->backbuffer_width = 0;

    if(args->backbuffer_height < 0)
        args->backbuffer_height = 0;

    if(args->title == NULL || args->title[0] == '\0')
        args->title = "BRender Application";

    return 0;
}

// clang-format off
static const char *usage_options =
    "  -w, --width          Set the initial window width (default: 1280).\n"
    "  -h, --height         Set the initial window height (default: 720).\n"
    "  -v, --verbose        Increase verbosity level. May be given multiple times.\n"
    "  --force-software     Force use of the software renderer. Only valid on x86.\n"
    "  --software-bpp       Software renderer bit depth. Valid options: 8 (default), 15, 16.\n"
    "  --backbuffer-width   Width of the backbuffer, defaults to the same as the screen.\n"
    "  --backbuffer-height  Height of the backbuffer, defaults to the same as the screen.\n"
    "  --help               Display this message.\n"
    "\n"
;
// clang-format on

int BrDemoRunArgv(const char *title, const br_demo_dispatch *dispatch, int argc, char *const *argv)
{
    int               r;
    struct parg_state ps;
    parg_init(&ps);

    br_demo_run_args args = {
        .title             = title,
        .width             = 1280,
        .height            = 720,
        .verbose           = BR_LOG_INFO,
        .force_software    = 0,
        .software_pm_type  = BR_PMT_INDEX_8,
        .backbuffer_width  = 0,
        .backbuffer_height = 0,
    };

    if((r = parse_args(argc, argv, &args)) != 0) {
        fprintf(stderr, "Usage: %s \nOptions:\n%s", argv[0], usage_options);
        return r;
    }

    BrLogSetLevel(args.verbose);

    return BrDemoRunArg(dispatch, &args);
}

int BrDemoRun(const char *title, br_uint_16 width, br_uint_16 height, const br_demo_dispatch *dispatch)
{
    br_demo_run_args args = {
        .title             = title,
        .width             = width,
        .height            = height,
        .verbose           = 0,
        .force_software    = 0,
        .software_pm_type  = BR_PMT_INDEX_8,
        .backbuffer_width  = 0,
        .backbuffer_height = 0,
    };

    return BrDemoRunArg(dispatch, &args);
}
