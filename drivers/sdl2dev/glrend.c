#include "drv.h"
#include <brglrend.h>
#include <brsdl2dev.h>
#include <brassert.h>

#define OPENGL_DEVICE_NAME "opengl"

typedef struct sdl_gl_state {
    SDL_Window *window;
    int         in_resize_callback;

    br_device_sdl_ext_procs ext_procs;
} sdl_gl_state;

static void *sdl_gl_create_context(br_pixelmap *dev, void *user)
{
    sdl_gl_state *state = user;
    void         *ctx;

    (void)dev;

    if((ctx = SDL_GL_CreateContext(state->window)) == NULL) {
        BrLogError("SDL2", "OpenGL context creation failed: %s", SDL_GetError());
        return NULL;
    }

    return ctx;
}

static void sdl_gl_delete_context(br_pixelmap *dev, void *ctx, void *user)
{
    (void)dev;
    (void)user;
    SDL_GL_DeleteContext(ctx);
}

static br_error sdl_gl_make_current(br_pixelmap *dev, void *ctx, void *user)
{
    sdl_gl_state *state = user;

    (void)dev;

    if(SDL_GL_MakeCurrent(state->window, (SDL_GLContext)ctx) < 0) {
        BrLogError("SDL2", "OpenGL MakeCurrent failed: %s", SDL_GetError());
        return BRE_FAIL;
    }

    return BRE_OK;
}

static void *sdl_gl_get_proc_address(const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

static void sdl_gl_swap_buffers(br_pixelmap *pm, void *user)
{
    sdl_gl_state *state = user;

    (void)pm;
    SDL_GL_SwapWindow(state->window);
}

static br_error sdl_gl_resize(br_pixelmap *pm, br_int_32 w, br_int_32 h, void *user)
{
    sdl_gl_state *state = user;

    (void)pm;

    /*
     * Prevent _issues_.
     */
    if(state->in_resize_callback)
        return BRE_OK;

    SDL_SetWindowSize(state->window, w, h);
    return BRE_OK;
}

static void sdl_gl_free(br_pixelmap *pm, void *user)
{
    sdl_gl_state *state = user;

    (void)pm;

    UASSERT(state->in_resize_callback == 0);

    SDL_DestroyWindow(state->window);
}

static br_error sdl_gl_handle_window_event(br_pixelmap *pm, void *arg, void *user)
{
    SDL_WindowEvent *evt   = arg;
    sdl_gl_state    *state = user;
    int              wpix, hpix;

    (void)user;

    /*
     * We can only handle window size changes for now.
     */
    if(evt->type != SDL_WINDOWEVENT && evt->event != SDL_WINDOWEVENT_SIZE_CHANGED)
        return BRE_UNSUPPORTED;

    SDL_GL_GetDrawableSize(state->window, &wpix, &hpix);

    /*
     * glrend's resize() method is guaranteed to only update the internal state, so this is fine.
     */
    ++state->in_resize_callback;
    BrPixelmapResize(pm, wpix, hpix);
    --state->in_resize_callback;
    return BRE_OK;
}

static void sdl_gl_preswap_hook(br_pixelmap *pm, unsigned int fbo, void *user)
{
    sdl_gl_state *state = user;

    (void)pm;
    (void)fbo;

    if(state->ext_procs.preswap != NULL)
        state->ext_procs.preswap(pm, state->ext_procs.user);
}

static br_error configure_device(br_device *dev)
{
    br_int_32      count  = 0;
    br_token_value gltv[] = {
        {.t = BRT_OPENGL_PROFILE_T,        .v = 0},
        {.t = BRT_OPENGL_VERSION_MAJOR_U8, .v = 0},
        {.t = BRT_OPENGL_VERSION_MINOR_U8, .v = 0},
        {.t = BR_NULL_TOKEN,               .v = 0},
    };

    if(ObjectQueryMany(dev, gltv, NULL, 0, &count) != BRE_OK) {
        BrLogError("SDL2", "Unable to query OpenGL driver for supported version.");
        return BRE_FAIL;
    }

    if(count != BR_ASIZE(gltv) - 1) {
        BrLogError("SDL2", "OpenGL driver returned short value count.");
        return BRE_FAIL;
    }

    switch(gltv[0].v.t) {
        case BRT_OPENGL_PROFILE_CORE:
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            break;
        case BRT_OPENGL_PROFILE_COMPATIBILITY:
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
            break;
        default:
            return BRE_FAIL;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gltv[1].v.u8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gltv[2].v.u8);

    return BRE_OK;
}

static br_error create_gl_pixelmap(SDL_Window *window, br_device *gldev, br_device_sdl_ext_procs *ext_procs,
                                   br_device_pixelmap **ppmap)
{

    br_error      err;
    Uint32        format;
    int           width, height;
    br_pixelmap  *pm;
    br_object    *outfctygl;
    sdl_gl_state *state;

    br_device_gl_ext_procs gl_ext_procs = {
        .create_context      = sdl_gl_create_context,
        .delete_context      = sdl_gl_delete_context,
        .make_current        = sdl_gl_make_current,
        .get_proc_address    = sdl_gl_get_proc_address,
        .resize              = sdl_gl_resize,
        .swap_buffers        = sdl_gl_swap_buffers,
        .preswap_hook        = sdl_gl_preswap_hook,
        .free                = sdl_gl_free,
        .handle_window_event = sdl_gl_handle_window_event,
        .user                = window,
    };

    br_token_value tv[] = {
        {.t = BRT_WIDTH_I32,          .v = 0                   },
        {.t = BRT_HEIGHT_I32,         .v = 0                   },
        {.t = BRT_PIXEL_TYPE_U8,      .v = 0                   },
        {.t = BRT_OPENGL_EXT_PROCS_P, .v = {.h = &gl_ext_procs}},
        {.t = BR_NULL_TOKEN,          .v = 0                   },
    };

    SDL_GL_GetDrawableSize(window, &width, &height);
    /* FIXME: Switch to this once SDL on Ubuntu LTS is new-enough. */
    //SDL_GetWindowSizeInPixels(window, &width, &height);
    tv[0].v.i32 = (br_int_32)width;
    tv[1].v.i32 = (br_int_32)height;

    format = SDL_GetWindowPixelFormat(window);
    if((err = SDLToBRenderPixelFormat(format, NULL, &tv[2].v.u8)) != BRE_OK)
        return err;

    /*
     * Find an output facility with our given requirements.
     */
    outfctygl = NULL;
    if((err = ObjectContainerFind(gldev, &outfctygl, BRT_OUTPUT_FACILITY, NULL, tv)) != BRE_OK) {
        return err;
    }

    /*
     * Allocate our state.
     */
    state  = BrResAllocate(NULL, sizeof(sdl_gl_state), BR_MEMORY_APPLICATION);
    *state = (sdl_gl_state){
        .window             = window,
        .in_resize_callback = 0,
        .ext_procs          = (br_device_sdl_ext_procs){.preswap = NULL, .user = NULL},
    };

    if(ext_procs != NULL)
        state->ext_procs = *ext_procs;

    gl_ext_procs.user = state;

    /*
     * Create the screen pixelmap.
     */
    if((err = OutputFacilityPixelmapNew(outfctygl, (void *)&pm, tv)) != BRE_OK) {
        BrResFree(state);
        return err;
    }

    /*
     * Attach the state to the pixelmap. #notourproblemanymore
     */
    BrResAdd(pm, state);

    *ppmap = (br_device_pixelmap *)pm;
    return BRE_OK;
}

br_error DevicePixelmapSDL2CreateGL(const pixelmap_new_tokens *pt, br_device_pixelmap **ppmap)
{
    br_device  *gldev;
    SDL_Window *window;
    br_error    err;

    UASSERT(pt->flags & SDL_WINDOW_OPENGL);

    /*
     * OpenGL device pixelmaps are for new windows only.
     */
    if(pt->surface != NULL || pt->window != NULL || pt->use_type != BRT_NONE)
        return BRE_FAIL;

    /*
     * Find or load the device.
     */
    if(BrDevCheckAdd(&gldev, OPENGL_DEVICE_NAME, NULL) != BRE_OK || gldev == NULL) {
        BrLogError("SDL2", "OpenGL window requested, but driver can't be loaded.");
        return BRE_FAIL;
    }

    /*
     * Query its caps and set any SDL attributes prior to window creation.
     */
    if((err = configure_device(gldev)) != BRE_OK)
        return err;

    window = SDL_CreateWindow(pt->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pt->width, pt->height, pt->flags);
    if(window == NULL) {
        BrLogError("SDL2", "Error creating window: %s", SDL_GetError());
        return BRE_FAIL;
    }

    return create_gl_pixelmap(window, gldev, pt->ext_procs, ppmap);
}

SDL_Window *DevicePixelmapSDL2GetWindowGL(br_pixelmap *pm)
{
    const br_device_gl_ext_procs *procs;
    br_device                    *gldev;

    if(pm == NULL)
        return NULL;

    gldev = NULL;
    if(BrDevFind(&gldev, OPENGL_DEVICE_NAME) != BRE_OK || gldev == NULL)
        return NULL;

    if(ObjectDevice(pm) != gldev)
        return NULL;

    procs = NULL;
    if(ObjectQuery(pm, &procs, BRT_OPENGL_EXT_PROCS_P) != BRE_OK)
        return NULL;

    return ((sdl_gl_state *)procs->user)->window;
}
