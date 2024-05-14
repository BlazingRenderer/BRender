/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: driver.c 1.1 1997/12/10 16:51:45 jon Exp $
 * $Locker: $
 *
 * Driver interface functions
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"


#define SHORT_SUFFIX "RNDF"
#define LONG_SUFFIX "-Float"

/*
 * Driver-wide timestamp - the only static variable
 */
br_uint_32 DriverTimestamp;

/*
 * Main entry point for device - this may get redefined by the makefile
 */
BR_SOFTREND_API br_device *BR_EXPORT BrDrv1SoftRendBegin(const char *arguments)
{
	br_device *device;

	/*
	 * Setup static timestamp
	 */
    if(DriverTimestamp == 0)
    	DriverTimestamp = TIMESTAMP_START;

	/*
	 * Set up device
	 */
    device = DeviceSoftAllocate("SOFT" SHORT_SUFFIX);

    if(device == NULL)
        return NULL;

	if(RendererFacilitySoftAllocate(device, "Default-Renderer" LONG_SUFFIX) == NULL) {
        ObjectFree(device);
        return NULL;
    }

    return device;
}

#if BR_SOFTREND_BUILD_SHARED
BR_SOFTREND_API br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1SoftRendBegin(arguments);
}
#endif
