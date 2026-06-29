#ifndef BRDEMO_H_
#define BRDEMO_H_

#ifdef __cplusplus
extern "C" {
#endif

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
    br_uint_8   software_pm_type;
    br_int_32   backbuffer_width;
    br_int_32   backbuffer_height;
    const char *opengl_device_name;
    int         pos_argc;
    char       *pos_argv[4]; /* If you need more than 4 positional args, tough. */
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
    uint8_t      _primitive_heap[1500 * 1024];
} br_demo;

/**
 * \brief Default-initialise a br_demo_run_args structure, suitable for passing to BrDemoRunArg().
 *
 * \param args A non-NULL pointer to the target br_demo_run_args structure.
 */
void BrDemoDefaultArgs(br_demo_run_args *args);

/**
 * \brief Parse the default set of arguments for a BrDemo* application.
 *
 * \param args A non-NULL pointer to the target br_demo_run_args structure.
 * \param argc argc as passed to main().
 * \param argv argv as passed to main().
 *
 * \returns If argument parsing fails, returns <0 and prints the usage to stderr.
 *          If argument parsing succeeds, returns 0.
 */
int BrDemoParseArgs(br_demo_run_args *args, int argc, char *const *argv);

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
