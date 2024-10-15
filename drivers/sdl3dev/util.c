#include "drv.h"

br_boolean BrSDLUtilIsAltEnter(const SDL_KeyboardEvent *evt)
{
    if(evt->repeat)
        return BR_FALSE;

    if(evt->state != SDL_PRESSED)
        return BR_FALSE;

    if(!(evt->keysym.mod & KMOD_ALT))
        return BR_FALSE;

    return evt->keysym.scancode == SDL_SCANCODE_RETURN;
}

SDL_Window *BrSDLUtilGetWindow(br_pixelmap *pm)
{
    if(pm == NULL)
        return NULL;

    if(DevicePixelmapSDL2IsOurs(pm))
        return ((br_device_pixelmap *)pm)->window;

    return DevicePixelmapSDL2GetWindowGL(pm);
}
