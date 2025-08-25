/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: bios.c 1.1 1997/12/10 16:53:21 jon Exp $
 * $Locker: $
 *
 * BIOS interface functions for drivers
 */
#include <dpmi.h>
#include "drv.h"

br_uint_16 BIOSVideoGetMode(void)
{
    __dpmi_regs regs = {.h = {.ah = 0x0F}};
    __dpmi_int(0x10, &regs);
    return regs.h.al;
}

br_error BIOSVideoSetMode(br_uint_16 mode)
{
    enum {
        HOST_FLAG_CARRY = 1
    };
    __dpmi_regs regs = {.x = {.ax = mode}};
    __dpmi_int(0x10, &regs);

    return (regs.x.flags & HOST_FLAG_CARRY) ? BRE_FAIL : BRE_OK;
}
