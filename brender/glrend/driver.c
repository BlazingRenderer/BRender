#include "drv.h"

/*
 * Main entry point for device.
 */
br_device *BR_EXPORT BrDrvGLBegin(const char *arguments)
{
    br_device *dev = DeviceGLAllocate("opengl", arguments);
    if(!dev)
        return NULL;

    return dev;
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrvGLBegin(arguments);
}
#endif
