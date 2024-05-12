/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: syscn_p.h 1.1 1997/12/10 16:41:19 jon Exp $
 * $Locker: $
 *
 * Public function prototypes for BRender framework
 */
#ifndef _SYSCON_P_H_
#define _SYSCON_P_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

BR_API br_error                 BrSystemConfigBegin();
BR_API br_error BR_PUBLIC_ENTRY BrSystemConfigLoad(br_token t, const char *Param1, const void *Param2);
BR_API br_error BR_PUBLIC_ENTRY BrSystemConfigSave(br_token t, const char *Param1, const void *Param2);
BR_API br_error BR_PUBLIC_ENTRY BrSystemConfigQueryString(br_token t, char *string, int max_size);
BR_API br_error BR_PUBLIC_ENTRY BrSystemConfigSetString(br_token t, const char *string);

BR_API br_error BR_PUBLIC_ENTRY BrSystemConfigSet(br_token t, br_value v);
BR_API br_error BR_PUBLIC_ENTRY BrSystemConfigQuery(br_token t, br_value *v);

BR_API br_error BR_PUBLIC_ENTRY BrReadEnvironmentEntry(br_associative_array *a, const char *name);

BR_API br_error BR_PUBLIC_ENTRY BrReadIniSection(br_associative_array *a, const char *filename, const char *section);

BR_API br_error BR_PUBLIC_ENTRY BrWriteIniSection(br_associative_array *a, const char *filename, const char *section);

BR_API br_error BR_PUBLIC_ENTRY BrReadRegistryKey(br_associative_array *a, const void *key, const char *subkey);

BR_API br_error BR_PUBLIC_ENTRY BrWriteRegistryKey(br_associative_array *a, const void *key, const char *subkey);

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif
