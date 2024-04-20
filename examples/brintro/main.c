#include <stdio.h>
#include <SDL.h>
#include <brender.h>
#include <brsdl2dev.h>
#include <brglrend.h>

#include "intro.h"

/*
 * Primitive heap - used by z-buffered renderer to defer drawing of blended primitives
 */
static uint8_t primitive_heap[1500 * 1024];

void _BrBeginHook(void)
{
    BrDevAddStatic(NULL, BrDrv1SDL2Begin, NULL);
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);
}

void _BrEndHook(void)
{
}

int main(int argc, char **argv)
{
    br_pixelmap *screen = NULL, *colour_buffer = NULL, *depth_buffer = NULL;
    int          ret = -1;
    br_error     r;
    br_uint_64   ticks_last, ticks_now;
    br_boolean   is_fullscreen = BR_FALSE;
    SDL_Window  *window;

    br_intro *intro;

    /*
     * QoL for those poor X11 users.
     */
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    BrBegin();

    // clang-format off
    r = BrDevBeginVar(&screen, "SDL2",
                      BRT_WIDTH_I32,        1024,
                      BRT_HEIGHT_I32,       768,
                      BRT_PIXEL_TYPE_U8,    BR_PMT_RGB_888,
                      BRT_WINDOW_NAME_CSTR, "BRender Intro Sequence",
                      BRT_HIDPI_B,          BR_TRUE,
                      BRT_RESIZABLE_B,      BR_TRUE,
                      BRT_OPENGL_B,         BR_TRUE,
                      BR_NULL_TOKEN);
    // clang-format on
    if(r != BRE_OK)
        goto screen_creation_failed;

    screen->origin_x = (br_int_16)(screen->width / 2);
    screen->origin_y = (br_int_16)(screen->height / 2);

    if(BrSDLUtilResizeBuffers(screen, &colour_buffer, &depth_buffer) != BRE_OK)
        goto buffer_create_failed;

    window = BrSDLUtilGetWindow(screen);
    SDL_SetWindowFullscreen(window, 0);
    is_fullscreen = BR_FALSE;

    BrRendererBegin(colour_buffer, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    intro = BrIntroCreate();
    BrIntroSetAspect(intro, BR_DIV(BR_SCALAR(screen->width), BR_SCALAR(screen->height)));

    ticks_last = SDL_GetTicks64();

    for(SDL_Event evt;;) {
        float dt;

        ticks_now  = SDL_GetTicks64();
        dt         = (float)(ticks_now - ticks_last) / 1000.0f;
        ticks_last = ticks_now;

        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_QUIT:
                    goto done;
                case SDL_WINDOWEVENT:
                    /*
                     * Window event, pass it to the driver.
                     */
                    if(BrPixelmapHandleWindowEvent(screen, &evt.window) != BRE_OK) {
                        BrLogError("APP", "Error handling window event");
                        goto buffer_create_failed;
                    }

                    switch(evt.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            /*
                             * The main screen should have been resized above.
                             * Update its origin and resize the framebuffer.
                             */
                            screen->origin_x = (br_int_16)(screen->width / 2);
                            screen->origin_y = (br_int_16)(screen->height / 2);

                            if(BrSDLUtilResizeBuffers(screen, &colour_buffer, &depth_buffer) != BRE_OK) {
                                BrLogError("APP", "Error resizing window buffers");
                                goto buffer_create_failed;
                            }

                            /*
                             * Update the camera's aspect ratio to match the camera.
                             */
                            BrIntroSetAspect(intro, BR_DIV(BR_SCALAR(screen->width), BR_SCALAR(screen->height)));
                            break;
                    }
                    break;
                case SDL_KEYDOWN: {
                    if(BrSDLUtilIsAltEnter(&evt.key)) {
                        if(is_fullscreen) {
                            SDL_SetWindowFullscreen(window, 0);
                        } else {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                        is_fullscreen = !is_fullscreen;
                        break;
                    }
                    break;
                }
            }
        }

        /*
         * Apply the animation once we've hit our accumulator.
         */
        BrIntroUpdate(intro, dt);

        BrRendererFrameBegin();
        BrPixelmapFill(colour_buffer, 0);
        BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
        BrZbSceneRender(BrIntroGetWorld(intro), BrIntroGetCamera(intro), colour_buffer, depth_buffer);
        BrRendererFrameEnd();

        BrPixelmapDoubleBuffer(screen, colour_buffer);
    }

done:
    ret = 0;

    BrIntroDestroy(intro);

    BrRendererEnd();

    BrPixelmapFree(depth_buffer);
    BrPixelmapFree(colour_buffer);

buffer_create_failed:
    BrPixelmapFree(screen);

screen_creation_failed:

    BrEnd();
    return ret;
}
