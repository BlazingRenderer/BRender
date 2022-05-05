#include "drv.h"

/*
 * Main entry point for device - this may get redefined by the build system.
 */
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    (void)arguments;
    return DeviceNullAllocate("NULL");
}
