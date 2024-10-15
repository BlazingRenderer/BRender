#include "drv.h"

br_boolean BrSDL3UtilIsAltEnter(const SDL_KeyboardEvent *evt)
{
    if(evt->repeat)
        return BR_FALSE;

    if(!evt->down)
        return BR_FALSE;

    if(!(evt->mod & SDL_KMOD_ALT))
        return BR_FALSE;

    return evt->scancode == SDL_SCANCODE_RETURN;
}

SDL_Window *BrSDL3UtilGetWindow(br_pixelmap *pm)
{
    if(pm == NULL)
        return NULL;

    if(DevicePixelmapSDL3IsOurs(pm))
        return ((br_device_pixelmap *)pm)->window;

    return DevicePixelmapSDL3GetWindowGL(pm);
}
