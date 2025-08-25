/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: driver.c 1.1 1997/12/10 16:53:52 jon Exp $
 * $Locker: $
 *
 * Driver interface functions
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"

br_device *BR_EXPORT BrDrv1VESABegin(const char *arguments)
{
    br_device *device;
    int        type_count;

    /*
     * Set up device
     */
    if((device = DeviceVESAAllocate("VESA", arguments)) == NULL)
        return NULL;

    type_count = 0;

    /*
     * Setup all the built in types
     */
    if(OutputFacilityVESAInitialise(&type_count, device) != BRE_OK)
        return NULL;

    /*
     * If nothing is available, then don't admit to being a device
     */
    if(type_count == 0)
        return NULL;
    else
        return device;
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1VESABegin(arguments);
}
#endif
