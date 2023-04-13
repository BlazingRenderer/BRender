#include <inttypes.h>
#include "dosio.h"
#include <SDL.h>
#include <brglrend.h>
#include <brsdl2dev.h>

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

void BR_CALLBACK _BrBeginHook(void)
{
    BrLogSetLevel(BR_LOG_TRACE);
    BrDevAddStatic(NULL, BrDrvGLBegin, NULL);
    BrDevAddStatic(NULL, BrDrv1SDL2Begin, NULL);
}

void BR_CALLBACK _BrEndHook(void)
{
}

br_pixelmap *BR_PUBLIC_ENTRY DOSGfxBegin(const char *setup_string)
{
    // clang-format off
    r = BrDevBeginVar(&screen, "SDL2",
                      BRT_WIDTH_I32,     1280,
                      BRT_HEIGHT_I32,    720,
                      BRT_PIXEL_TYPE_U8, BR_PMT_RGB_888,
                      BRT_HIDPI_B,       BR_TRUE,
                      BRT_RESIZABLE_B,   BR_TRUE,
                      BRT_OPENGL_B,      BR_TRUE,
                      BR_NULL_TOKEN);
    // clang-format on

    if(r != BRE_OK) {
        return NULL;
    }

    sdl_window = BrSDLUtilGetWindow(screen);

    screen->origin_x = (br_int_16)(screen->width / 2);
    screen->origin_y = (br_int_16)(screen->height / 2);

    r = BrSDLUtilResizeBuffers(screen, &colour_buffer, &_depth_buffer);
    if(r != BRE_OK) {
        BrPixelmapFree(screen);
        return NULL;
    }

    screen_width  = screen->width;
    screen_height = screen->height;
    is_fullscreen = BR_FALSE;
    SDL_SetWindowFullscreen(sdl_window, 0);

    BrRendererBegin(colour_buffer, NULL, NULL, primitive_heap, sizeof(primitive_heap));
    return screen;
}

void BR_PUBLIC_ENTRY DOSGfxEnd()
{
    BrRendererEnd();

    if(_depth_buffer != NULL)
        BrPixelmapFree(_depth_buffer);

    _depth_buffer = NULL;

    if(colour_buffer != NULL)
        BrPixelmapFree(colour_buffer);

    colour_buffer = NULL;

    if(screen != NULL)
        BrPixelmapFree(screen);

    screen = NULL;
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
                if(BrPixelmapHandleWindowEvent(screen, &evt.window) != BRE_OK) {
                    BrLogError("APP", "Error handling window event");
                    return false;
                }

                switch(evt.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        screen->origin_x = (br_int_16)(screen->width / 2);
                        screen->origin_y = (br_int_16)(screen->height / 2);

                        if(BrSDLUtilResizeBuffers(screen, &colour_buffer, &_depth_buffer) != BRE_OK) {
                            BrLogError("APP", "Error resizing window buffers");
                            return false;
                        }

                        ((br_camera *)camera->type_data)->aspect = BR_DIV(BR_SCALAR(screen->width), BR_SCALAR(screen->height));
                        break;
                }
                break;
            }
            case SDL_KEYDOWN: {
                if(BrSDLUtilIsAltEnter(&evt.key)) {
                    if(is_fullscreen) {
                        SDL_SetWindowFullscreen(sdl_window, 0);
                    } else {
                        SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    is_fullscreen = !is_fullscreen;
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
