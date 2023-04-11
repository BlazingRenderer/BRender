#include <brender.h>
#include <brglrend.h>
#include <SDL_video.h>

static void *sdl_gl_create_context(br_pixelmap *pm, void *user)
{
    void *ctx;

    (void)pm;

    if((ctx = SDL_GL_CreateContext((SDL_Window *)user)) == NULL) {
        BrLogError("SDL", "OpenGL context creation failed: %s", SDL_GetError());
        return NULL;
    }

    return ctx;
}

static void sdl_gl_delete_context(br_pixelmap *pm, void *ctx, void *user)
{
    (void)pm;
    (void)user;
    SDL_GL_DeleteContext(ctx);
}

static br_error sdl_gl_make_current(br_pixelmap *pm, void *ctx, void *user)
{
    (void)pm;

    if(SDL_GL_MakeCurrent((SDL_Window *)user, (SDL_GLContext)ctx) < 0) {
        BrLogError("SDL", "OpenGL MakeCurrent failed: %s", SDL_GetError());
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
    (void)pm;
    SDL_GL_SwapWindow((SDL_Window *)user);
}

br_device_gl_ext_procs BrSDLMakeGLProcs(SDL_Window *window)
{
    return (br_device_gl_ext_procs){
        .create_context   = sdl_gl_create_context,
        .delete_context   = sdl_gl_delete_context,
        .make_current     = sdl_gl_make_current,
        .get_proc_address = sdl_gl_get_proc_address,
        .swap_buffers     = sdl_gl_swap_buffers,
        .preswap_hook     = NULL,
        .user             = window,
    };
}

br_error BrSDLDevAddStaticGL(void)
{
    return BrDevAddStatic(NULL, BrDrvGLBegin, NULL);
}
