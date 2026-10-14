/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brbegin.c 1.1 1997/12/10 16:41:03 jon Exp $
 * $Locker: $
 *
 * Wrapper functions for all the various subsystem begins
 */
#include "fw.h"
#include "host.h"
#include <brassert.h>

static br_boolean active = BR_FALSE;

br_error BR_PUBLIC_ENTRY BrBegin(void)
{
    if(active)
        return BRE_ALLREADY_ACTIVE;

    /*
     * Fire up other libraries
     */
    BrFwBegin();
    HostBegin();
    BrPixelmapBegin();

    active = BR_TRUE;

    /*
     * Call a hook that allows static devices at link time
     */

    _BrBeginHook();

    return BRE_OK;
}

br_error BR_PUBLIC_ENTRY BrEnd(void)
{
    int i;

    if(!active)
        return BRE_NOT_ACTIVE;

    _BrEndHook();

    active = BR_FALSE;

    /*
     * Close down loaded devices, most recently loaded first. A device added
     * later may depend on a resource (e.g. a loaded library) provided by one
     * added earlier, so teardown has to run against the loading order. Slots
     * are filled in ascending order, hence the reverse scan.
     */
    for(i = fw.ndev_slots - 1; i >= 0; i--)
        if(fw.dev_slots[i].dev)
            BrDevRemove(fw.dev_slots[i].dev);

    BrPixelmapEnd();
    HostEnd();
    BrFwEnd();

    return BRE_OK;
}
