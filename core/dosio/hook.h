/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: host.h 1.1 1997/12/10 16:41:13 jon Exp $
 * $Locker: $
 */
#ifndef _HOOK_H_
#define _HOOK_H_

#ifndef _BRENDER_H_
#include "brender.h"
#endif

/*
 * Structures use to hook interrupts and exceptions
 */
typedef struct host_interrupt_hook {
		br_uint_8	vector;
		br_boolean	active;

		br_uint_32	old_offset;
		br_uint_16	old_sel;
} host_interrupt_hook ;

typedef struct host_exception_hook {
		br_uint_8	exception;
		br_boolean	active;

		br_uint_32	old_offset;
		br_uint_16	old_sel;

		br_uint_8	scratch[256];
} host_exception_hook ;

#ifndef _HOOK_P_H_
#include "hook_p.h"
#endif

#endif

