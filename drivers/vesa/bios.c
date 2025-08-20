/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: bios.c 1.1 1997/12/10 16:53:21 jon Exp $
 * $Locker: $
 *
 * BIOS interface functions for drivers
 */
#include "drv.h"
#include "host.h"

#include <dos.h>

static union REGS regs;

br_uint_16 BIOSVideoGetMode(void)
{
	regs.h.ah = 0x0F;
	int86(0x10, &regs, &regs);
	return (br_uint_16)regs.h.al;
}

br_error BIOSVideoSetMode(br_uint_16 mode)
{
	regs.w.ax = mode;
	int86(0x10, &regs, &regs);
	
	return (regs.x.flags & 1) ? BRE_FAIL : BRE_OK;
}
