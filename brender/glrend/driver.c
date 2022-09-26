#include "drv.h"

/*
 * Main entry point for device - this may get redefined by the build system.
 */
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    br_device *dev = DeviceGLAllocate("opengl", arguments);
    if(!dev)
        return NULL;

    return dev;
}
