/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: host_p.h 2.13 1997/06/16 19:20:18 jon Exp $
 * $Locker: $
 *
 * Function prototypes for BRender Host library
 */
#ifndef _HOST_P_H_
#define _HOST_P_H_

#ifndef _NO_PROTOTYPES
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Setup
 */
void BR_PUBLIC_ENTRY HostBegin(void);
void BR_PUBLIC_ENTRY HostEnd(void);

br_error BR_RESIDENT_ENTRY HostInfo(struct host_info *buffer, br_size_t buffersize);

/*
 * Native loadable modules
 */
void *BR_RESIDENT_ENTRY HostImageLoad(const char *name);
void BR_RESIDENT_ENTRY  HostImageUnload(void *image);
void *BR_RESIDENT_ENTRY HostImageLookupName(void *img, const char *name, br_uint_32 hint);
void *BR_RESIDENT_ENTRY HostImageLookupOrdinal(void *img, br_uint_32 ordinal);

/*
 * Configuration strings
 */
br_error BR_RESIDENT_ENTRY HostConfigStringSize(br_size_t *size, br_int_32 type);
br_error BR_RESIDENT_ENTRY HostConfigStringQuery(char *buffer, br_size_t buffer_size, br_int_32 type);

// INI File

br_error BR_RESIDENT_ENTRY HostGetIniEntry(const char *ini_file, const char *section_name, char *entry, char *Buffer, int max);

br_error BR_RESIDENT_ENTRY HostIniReadSection(const char *filename, const char *section, char *buffer, br_size_t size);

br_error BR_RESIDENT_ENTRY HostIniWriteSection(const char *filename, const char *section, char *buffer, br_size_t size);

// Registry entry query

br_error BR_RESIDENT_ENTRY HostRegistryQuery(const void *hKey, const char *Path, const char *entry, char *Buffer, br_uint_16 max, br_uint_16 *size);

br_error BR_RESIDENT_ENTRY HostRegistryReadKey(const void *key, const char *subkey, char *buffer, br_size_t size);

br_error BR_RESIDENT_ENTRY HostRegistryWriteKey(const void *key, const char *subkey, char *buffer, br_size_t size);

// INI file query

br_error BR_RESIDENT_ENTRY HostIniQuery(const char *ini_file, const char *section_name, const char *entry, char *Buffer, int max, br_uint_16 *size);

// Environment variable query.

br_boolean BR_RESIDENT_ENTRY HostIniSectionExists(const char *ini_file, const char *section_name);

char *BR_RESIDENT_ENTRY HostDefaultDevice();

#ifdef __cplusplus
};
#endif
#endif /* _NO_PROTOTYPES */
#endif
