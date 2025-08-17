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

#if defined(__DEVKITA64__)
#include <switch.h>
#include <unistd.h>

static int s_nxlinkSock = -1;

static void NxBegin(void)
{
    if(R_FAILED(socketInitializeDefault()))
        return;

    s_nxlinkSock = nxlinkStdio();
    if(s_nxlinkSock >= 0)
        BrLogTrace("PLAT", "printf output now goes to nxlink server");
    else
        socketExit();
}

static void NxEnd(void)
{
    if(s_nxlinkSock >= 0) {
        close(s_nxlinkSock);
        socketExit();
        s_nxlinkSock = -1;
    }
}
#endif

br_error BR_PUBLIC_ENTRY BrBegin(void)
{
    if(active)
        return BRE_ALLREADY_ACTIVE;

    /*
     * Fire up other libraries
     */

#if defined(__DEVKITA64__)
    NxBegin();
#endif

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
    br_device *dev;

    if(!active)
        return BRE_NOT_ACTIVE;

    _BrEndHook();

    active = BR_FALSE;

    /*
     * Close down loaded devices
     */
    while(BrDevFind(&dev, NULL) == BRE_OK)
        if(dev)
            BrDevRemove(dev);

    BrPixelmapEnd();
    HostEnd();
    BrFwEnd();


#if defined(__DEVKITA64__)
    NxEnd();
#endif
    return BRE_OK;
}
