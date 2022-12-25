/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: host_ip.h 2.6 1997/06/16 19:20:15 jon Exp $
 * $Locker: $
 *
 * Prototypes for functions internal to host library
 */
#ifndef _HOST_IP_H_
#define _HOST_IP_H_

#ifndef NO_PROTOTYPES
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Windows configuration
 */
//char * BR_RESIDENT_ENTRY GetWinProfileString(char *name);
char * BR_RESIDENT_ENTRY GetWinRegistryValue(char *name);

/*
 * cpuid.asm
 */
//void BR_ASM_CALL CPUInfo(br_token *cpu_type, br_uint_32 *features);

/* CPUInfo.c
 *
 */
void CPUInfo(br_token *cpu_type, br_uint_32 *features);

#ifdef __cplusplus
};
#endif
#endif

#endif


