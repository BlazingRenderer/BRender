#ifndef BRSDL_H
#define BRSDL_H

#ifndef _BRENDER_H_
#   error Please include brender.h first
#endif

#include <SDL_video.h>
#include <SDL_events.h>

#ifdef __cplusplus
extern "C" {
#endif

void BrSDLUtilCleanupScreen(br_pixelmap **screen, br_pixelmap **colour_buffer,
                            br_pixelmap **depth_buffer);


/*
 * Attempt to resize the screen, falling back to (re)creation if we can't.
 * Will re-initialise the renderer.
 */
br_boolean BrSDLUtilResizeScreen(const char *driver, br_pixelmap **screen,
                                 br_pixelmap **colour_buffer, br_pixelmap **depth_buffer,
                                 void *primitive_heap, br_size_t primitive_heap_size,
                                 br_int_16 width, br_int_16 height, br_int_16 initial_bpp);

br_boolean BrSDLUtilSetFullscreen(SDL_Window *sdl_window, br_boolean fullscreen);

br_boolean BrSDLUtilIsAltEnter(const SDL_KeyboardEvent *evt);

br_boolean BrSDLUtilOnResize(SDL_Window *window, const char *driver, br_pixelmap **screen,
                             br_pixelmap **colour_buffer, br_pixelmap **depth_buffer,
                             void *primitive_heap, br_size_t primitive_heap_size,
                             br_camera *camera);

/* glrend.c */
br_error BrSDLDevAddStaticGL(SDL_Window *window);

#ifdef __cplusplus
};
#endif

#endif /* BRSDL_H */
