/*
 * softprim driver entry point
 *
 * This deliberately reuses pentprim's entry point name, device identifier and
 * primitive library object name. BrPrimitiveLibraryFind() resolves the library
 * by image/object name, so presenting the same names means the swap is purely
 * a link-time choice and nothing outside the driver needs to change.
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Driver-wide timestamp
 */
br_timestamp SoftPrimTimestamp;

/*
 * Main entry point for device - this may get redefined by the makefile
 */
br_device *BR_EXPORT BrDrv1SoftPrimBegin(const char *arguments)
{
    br_device *device;

    if(SoftPrimTimestamp == 0)
        SoftPrimTimestamp = TIMESTAMP_START;

    device = DeviceSoftPrimAllocate("SOFTPRMF");

    if(device == NULL)
        return NULL;

    if(PrimitiveLibrarySoftPrimAllocate(device, "Default-Primitives-Float", arguments) == NULL) {
        ObjectFree(device);
        return NULL;
    }

    return device;
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1SoftPrimBegin(arguments);
}
#endif
