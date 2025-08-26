/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: host_p.h 1.1 1997/12/10 16:41:13 jon Exp $
 * $Locker: $
 *
 * Function prototypes for BRender Host library
 */
#ifndef _HOOK_P_H_
#define _HOOK_P_H_

#ifndef _NO_PROTOTYPES
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Protected mode access
 */
br_error BR_RESIDENT_ENTRY HostInterruptHook(host_interrupt_hook *h, br_uint_8 vector, br_uint_32  off,  br_uint_16  sel);
br_error BR_RESIDENT_ENTRY HostInterruptUnhook(host_interrupt_hook *h);

br_error BR_RESIDENT_ENTRY HostExceptionHook(host_exception_hook *h, br_uint_8 exception, br_uint_32  off,  br_uint_16  sel);
br_error BR_RESIDENT_ENTRY HostExceptionUnhook(host_exception_hook *h);

br_error BR_RESIDENT_ENTRY HostInterruptCall(br_uint_8 vector, union host_regs *regs);
br_error BR_RESIDENT_ENTRY HostRegistersGet(union host_regs *regs);

#ifdef __cplusplus
};
#endif
#endif /* _NO_PROTOTYPES */
#endif

