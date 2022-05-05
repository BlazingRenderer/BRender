#include "drv.h"

/*
 * Main entry point for device - this may get redefined by the build system.
 */
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    br_device *dev = DeviceGLAllocate("opengl", arguments);
    if(!dev)
        return NULL;

    /* List all the SDL screen modes */
    if(OutputFacilityGLEnumerate(dev) != BRE_OK)
        return NULL;

    return dev;
}
