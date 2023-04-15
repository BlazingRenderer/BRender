#ifndef _BRSDL2DEV_P_H
#define _BRSDL2DEV_P_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

/*
 * Main entry point for device.
 */
struct br_device *BR_EXPORT BrDrv1SDL2Begin(const char *arguments);

/*
 * Attempt to resize the back and depth buffers to match the screen, falling
 * back to recreation upon failure. Buffers will be created if NULL.
 *
 * Convenience method.
 */
br_error BrSDLUtilResizeBuffers(br_pixelmap *screen, br_pixelmap **colour_buffer, br_pixelmap **depth_buffer);

/*
 * Does the given SDL_KeyboardEvent represent an Alt+Enter.
 */
br_boolean BrSDLUtilIsAltEnter(const struct SDL_KeyboardEvent *evt);

/*
 * Retrieve the SDL_Window* from a device.
 * With great power, comes great responsibility.
 */
struct SDL_Window *BrSDLUtilGetWindow(br_pixelmap *pm);

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif /* _BRSDL2DEV_P_H */
