#ifndef BRDEMO_H_
#define BRDEMO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <SDL3/SDL.h>
#include <brender.h>

struct br_demo;
typedef struct br_demo br_demo;

/**
 * Demo callback function
 */
typedef br_error br_demo_init_cbfn(br_demo *demo);
typedef void     br_demo_process_event(br_demo *demo, const SDL_Event *evt);
typedef void     br_demo_update_cbfn(br_demo *demo, br_scalar dt);
typedef void     br_demo_render_cbfn(br_demo *demo);
typedef void     br_demo_on_resize(br_demo *demo, br_uint_16 width, br_uint_16 height);
typedef void     br_demo_destroy_cbfn(br_demo *demo);

typedef struct br_demo_dispatch {
    br_demo_init_cbfn     *init;
    br_demo_process_event *process_event;
    br_demo_update_cbfn   *update;
    br_demo_render_cbfn   *render;
    br_demo_on_resize     *on_resize;
    br_demo_destroy_cbfn  *destroy;
} br_demo_dispatch;

br_error BrDemoDefaultInit(br_demo *demo);
void     BrDemoDefaultProcessEvent(br_demo *demo, const SDL_Event *evt);
void     BrDemoDefaultUpdate(br_demo *demo, br_scalar dt);
void     BrDemoDefaultRender(br_demo *demo);
void     BrDemoDefaultOnResize(br_demo *demo, br_uint_16 width, br_uint_16 height);
void     BrDemoDefaultDestroy(br_demo *demo);

typedef struct br_demo_run_args {
    const char *title;
    br_int_32   width;
    br_int_32   height;
    int         verbose;
    int         force_software;
    int         require_accel;
    br_uint_8   pm_type;
    br_int_32   backbuffer_width;
    br_int_32   backbuffer_height;
    const char *opengl_device_name;
    int         vulkan;
    int         no_stats;
    int         no_depth;
    int         pos_argc;    /* Number of positional arguments found, even if only pos_argv[0..3] could be stored. */
    char       *pos_argv[4]; /* Positional arguments; BrDemoParseArgs() can store more in caller-supplied memory. */
} br_demo_run_args;

/**
 * Demo structure passed to callback functions
 */
typedef struct br_demo {
    /*
     * Dispatch table.
     */
    const br_demo_dispatch *dispatch;

    /*
     * Copy of the parsed arguments structure.
     */
    const br_demo_run_args *args;

    /*
     * The world's order table.
     * Use this update this when configuring the camera.
     */
    br_order_table *order_table;

    /*
     * The default palette to use when de-CLUT'ting pixelmaps.
     * This will only be used if the pixelmap is indexed and
     * has no palette set.
     */
    br_pixelmap *palette;

    /*
     * The root world actor - attach things here.
     */
    br_actor *world;

    /*
     * The camera actor - this must be set by the init() function.
     */
    br_actor *camera;

    /*
     * The background clear colour.
     * This is used by the default render() implementation.
     */
    br_colour clear_colour;

    /*
     * Colour in which to render text.
     */
    br_colour text_colour;

    br_pixelmap *colour_buffer;
    br_pixelmap *depth_buffer;

    /*
     * Are we using a hardware-accelerated renderer?
     */
    br_boolean hw_accel;

    void *user;

    /*
     * Private fields.
     */
    br_pixelmap *_screen;
    uint8_t      _primitive_heap[8192 * 1024];
} br_demo;

/**
 * \brief Default-initialise a br_demo_run_args structure, suitable for passing to BrDemoRunArg().
 *
 * \param args A non-NULL pointer to the target br_demo_run_args structure.
 */
void BrDemoDefaultArgs(br_demo_run_args *args);

/**
 * \brief Result of BrDemoParseArgs().
 *
 * The parse prints nothing, so a tool can show help on whichever stream and
 * exit with whichever code it treats as success; --help is a successful
 * invocation for some tools and an error for others.
 */
typedef enum br_demo_parse {
    BR_DEMO_PARSE_OK = 0,
    BR_DEMO_PARSE_HELP,  /* --help was given. */
    BR_DEMO_PARSE_ERROR, /* Bad option, bad value, missing or unknown argument. */
} br_demo_parse;

/** Whether an application option takes a value. */
typedef enum br_demo_arg {
    BR_DEMO_ARG_NONE = 0,
    BR_DEMO_ARG_REQUIRED,
} br_demo_arg;

/**
 * \brief Handle one application option as it is parsed.
 *
 * \param ctx   The value passed as BrDemoParseArgs()'s ctx.
 * \param value The option's argument, or NULL for BR_DEMO_ARG_NONE.
 *
 * \returns 0 on success, non-zero to fail the parse.
 */
typedef int br_demo_opt_fn(void *ctx, const char *value);

/**
 * \brief An application-specific option, added to the demo's own set.
 */
typedef struct br_demo_option {
    const char     *name;       /* Long name, without the leading "--". */
    char            short_name; /* 0 for none. */
    br_demo_arg     has_arg;
    const char     *metavar; /* Value name in the usage text, e.g. "DIR". */
    const char     *help;    /* One line of usage text. */
    br_demo_opt_fn *apply;   /* Non-NULL. */
} br_demo_option;

/**
 * \brief Parse the arguments for a BrDemo* application, with the application's
 *        own options added to the demo's.
 *
 * The two sets are parsed together. An application option whose name collides
 * with one of the demo's is rejected rather than silently shadowing it. An
 * application short name that collides with the demo's own (w, h, v) or with
 * another application option's short name is likewise an error.
 *
 * \param args      A non-NULL pointer to the target br_demo_run_args structure.
 * \param argc      argc as passed to main().
 * \param argv      argv as passed to main().
 * \param extra     The application's own options, or NULL.
 * \param nextra    The number of entries in extra.
 * \param ctx       Passed to every application option's apply callback, so the
 *                  table can stay static and the values stay the caller's.
 * \param pos_argv  Where to store the positional (non-option) arguments, in
 *                  order, or NULL to store them in args->pos_argv instead.
 * \param pos_cap   The number of entries pos_argv can hold. Ignored when
 *                  pos_argv is NULL, where args->pos_argv's four entries are
 *                  filled as they always have been.
 *
 * args->pos_argc always receives the true number of positional arguments
 * found, not the number stored, so a caller that supplies a small array can
 * compare the two and refuse rather than silently lose arguments. The stored
 * pointers point into the caller's argv; they are not copies.
 *
 * \returns BR_DEMO_PARSE_OK on success, BR_DEMO_PARSE_HELP if --help was given,
 *          BR_DEMO_PARSE_ERROR otherwise. Nothing is printed.
 */
br_demo_parse BrDemoParseArgs(br_demo_run_args *args, int argc, char *const *argv, const br_demo_option *extra, br_size_t nextra, void *ctx,
                              char **pos_argv, br_size_t pos_cap);

/**
 * \brief Print the usage text for a BrDemo* application.
 *
 * Prints the demo's options followed by the application's, when it has any.
 */
void BrDemoUsage(FILE *out, const char *argv0, const br_demo_option *extra, br_size_t nextra);

/**
 * \brief Execute a BrDemo*() application.
 *
 * This function automatically creates a valid BRender state and renderer with
 * the SDL3+GLRend drivers.
 *
 * \remark The intended usage is to return BrDemoRunArg() from main with the
 *         appropriate arguments.
 *
 * \param dispatch A non-NULL pointer to the dispatch table.
 * \param args     A non-NULL pointer to a br_demo_run_args structure.
 *
 * \returns 0 for success, 1 if some error occurred.
 */
int BrDemoRunArg(const br_demo_dispatch *dispatch, const br_demo_run_args *args);

/**
 * \brief Convenience BrDemo*() entrypoint.
 *
 * This is the same as calling BrDemoRunArg() after BrDemoParseArgs().
 *
 * \param title    A non-NULL pointer to the window title.
 * \param dispatch A non-NULL pointer to the dispatch table.
 * \param argc     argc as passed to main().
 * \param argv     argv as passed to main().
 *
 * \returns 0 for success, 1 if some error occurred.
 */
int BrDemoRunArgv(const char *title, const br_demo_dispatch *dispatch, int argc, char *const *argv);

/**
 * \brief Legacy BrDemo*() entrypoint.
 *
 * \param title    A non-NULL pointer to the window title.
 * \param width    The initial window width.
 * \param height   The initial window height.
 * \param dispatch A non-NULL pointer to the dispatch table.
 *
 * \returns 0 for success, 1 if some error occurred.
 *
 * \sa BrDemoRunArg().
 */
int BrDemoRun(const char *title, br_uint_16 width, br_uint_16 height, const br_demo_dispatch *dispatch);

#ifdef __cplusplus
}
#endif

#endif /* BRDEMO_H_ */
