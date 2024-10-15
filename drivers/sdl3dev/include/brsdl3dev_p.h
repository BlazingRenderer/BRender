#ifndef _BRSDL3DEV_P_H
#define _BRSDL3DEV_P_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

/*
 * Main entry point for device.
 */
struct br_device *BR_EXPORT BrDrv1SDL3Begin(const char *arguments);

/*
 * Does the given SDL_KeyboardEvent represent an Alt+Enter.
 */
br_boolean BrSDL3UtilIsAltEnter(const struct SDL_KeyboardEvent *evt);

/*
 * Retrieve the SDL_Window* from a device.
 * With great power, comes great responsibility.
 */
struct SDL_Window *BrSDL3UtilGetWindow(br_pixelmap *pm);

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif /* _BRSDL3DEV_P_H */
