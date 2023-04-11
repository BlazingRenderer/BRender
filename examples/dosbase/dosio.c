#include <inttypes.h>
#include <stdio.h>
#include "dosio.h"

#include "SDL.h"
#include "brsdl.h"

SDL_Window  *sdl_window = NULL;
br_pixelmap *screen = NULL, *colour_buffer = NULL, *_depth_buffer = NULL;
br_error     r;
br_uint_64   ticks_last, ticks_now;
br_boolean   is_fullscreen = BR_FALSE;

int screen_width, screen_height;

/*
 * Primitive heap - used by z-buffered renderer to defer drawing of blended primitives
 */
static uint8_t primitive_heap[1500 * 1024];

static void DestroySDL()
{
    if(sdl_window) {
        SDL_DestroyWindow(sdl_window);
    }

    SDL_Quit();
}

static void InitSDL()
{
    /*
     * Init SDL
     */
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        BrLogError("SDL", "Initialisation error: %s", SDL_GetError());
        DestroySDL();
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    if((sdl_window = SDL_CreateWindow("BRender Sample Application", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280,
                                      720, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE)) == NULL) {
        BrLogError("SDL", "Window creation error: %s", SDL_GetError());
        DestroySDL();
        exit(2);
    }

    SDL_SetWindowMinimumSize(sdl_window, 320, 200);
}

void BR_CALLBACK _BrBeginHook(void)
{
    BrLogSetLevel(BR_LOG_TRACE);

    InitSDL();

    BrSDLDevAddStaticGL();
}

void BR_CALLBACK _BrEndHook(void)
{
}

br_pixelmap *BR_PUBLIC_ENTRY DOSGfxBegin(const char *setup_string)
{
    SDL_GL_GetDrawableSize(sdl_window, &screen_width, &screen_height);

    r = BrSDLUtilOnResize(sdl_window, "opengl", &screen, &colour_buffer, &_depth_buffer, primitive_heap,
                          sizeof(primitive_heap), NULL);
    if(r != BR_TRUE) {
        DestroySDL();
        exit(4);
    }

    is_fullscreen = BrSDLUtilSetFullscreen(sdl_window, BR_FALSE);

    return screen;
}

void BR_PUBLIC_ENTRY DOSGfxEnd()
{
    DestroySDL();

    BrDevEndOld();
}

bool UpdateSample(br_actor *camera, float *dt)
{
    return UpdateSampleWithKeyHandler(camera, dt, NULL);
}

bool UpdateSampleWithKeyHandler(br_actor *camera, float *dt, void (*keyCallback)(const struct SDL_KeyboardEvent *evt))
{
    SDL_Event evt;

    ticks_now  = SDL_GetTicks64();
    *dt        = (float)(ticks_now - ticks_last) / 1000.0f;
    ticks_last = ticks_now;

    while(SDL_PollEvent(&evt) > 0) {
        switch(evt.type) {
            case SDL_QUIT: {
                return false;
            }
            case SDL_WINDOWEVENT: {
                switch(evt.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        SDL_GL_GetDrawableSize(sdl_window, &screen_width, &screen_height);
                        (void)BrSDLUtilOnResize(sdl_window, "opengl", &screen, &colour_buffer, &_depth_buffer,
                                                primitive_heap, sizeof(primitive_heap), camera->type_data);
                        break;
                }
                break;
            }
            case SDL_KEYDOWN: {
                if(BrSDLUtilIsAltEnter(&evt.key)) {
                    is_fullscreen = BrSDLUtilSetFullscreen(sdl_window, !is_fullscreen);
                    break;
                } else if(keyCallback != NULL) {
                    keyCallback(&evt.key);
                }
                break;
            }
        }
    }

    return true;
}

void InitializeSample(br_pixelmap **screen_buffer, br_pixelmap **back_buffer, br_pixelmap **depth_buffer,
                      void *primitive, br_uint_32 primitiveCount)
{

    /*
     * Initialise screen buffer and set up CLUT (ignored in true colour)
     */
    *screen_buffer = DOSGfxBegin(NULL);
    //	palette = BrPixelmapLoad("std.pal");
    //	if (palette)
    //		DOSGfxPaletteSet(palette);

    /*
     * Initialise z-buffer renderer
     */
    if(primitive != NULL) {
        BrZsBegin((*screen_buffer)->type, primitive, primitiveCount);
    } else {
        BrZbBegin((*screen_buffer)->type, BR_PMT_DEPTH_16);
    }

    /*
     * Allocate Back Buffer and Depth Buffer
     */
    //	*back_buffer = BrPixelmapMatchTypedSized(*screen_buffer, BR_PMMATCH_OFFSCREEN, BR_PMT_RGB_888, screen_width, screen_height);
    //	(*back_buffer)->origin_x = (*back_buffer)->width / 2;
    //	(*back_buffer)->origin_y = (*back_buffer)->height / 2;
    //	*depth_buffer = BrPixelmapMatch(*back_buffer, BR_PMMATCH_DEPTH_16);

    *back_buffer = colour_buffer;
    if(depth_buffer != NULL) {
        *depth_buffer = _depth_buffer;
    }
}

void InitializeSampleZBuffer(br_pixelmap **screen_buffer, br_pixelmap **back_buffer, br_pixelmap **depth_buffer)
{
    BrBegin();
    InitializeSample(screen_buffer, back_buffer, depth_buffer, NULL, 0);
}

void InitializeSampleZSort(br_pixelmap **screen_buffer, br_pixelmap **back_buffer, void *primitive, br_uint_32 primitiveCount)
{
    InitializeSample(screen_buffer, back_buffer, NULL, primitive, primitiveCount);
}

br_actor *CreateSampleCamera(br_actor *parent)
{
    br_actor  *camera      = BrActorAdd(parent, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    br_camera *camera_data = (br_camera *)camera->type_data;
    camera_data->aspect    = BR_DIV(BR_SCALAR(screen_width), BR_SCALAR(screen_height));

    return camera;
}
