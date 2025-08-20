#include "drv.h"

/*
 * Main entry point for device - this may get redefined by the build system.
 */
br_device *BR_EXPORT BrDrv1DOSBegin(const char *arguments)
{
    br_device *device;
    (void)arguments;

    /*
     * Set up device
     */
    if((device = DeviceDOSAllocate("DOS")) == NULL)
        return NULL;

    if(OutputFacilityDOSCreateGeneric(device) == NULL) {
        /*
         * If nothing is available, then don't admit to being a device
         */
        ObjectFree(device);
        return NULL;
    }

    return device;
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1DOSBegin(arguments);
}
#endif
