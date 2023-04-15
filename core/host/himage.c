/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: himage.c 2.7 1997/05/02 15:41:08 jon Exp $
 * $Locker: $
 *
 * Using images from host environment
 */
#include "brender.h"

#include "host.h"

#ifdef __WIN_32__

#include <windows.h>

void *BR_RESIDENT_ENTRY HostImageLoad(const char *name)
{
    return (void *)LoadLibraryA(name);
}

void BR_RESIDENT_ENTRY HostImageUnload(void *image)
{
    FreeLibrary((HMODULE)image);
}

void *BR_RESIDENT_ENTRY HostImageLookupName(void *img, const char *name, br_uint_32 hint)
{
    return (void *)GetProcAddress((HMODULE)img, name);
}

void *BR_RESIDENT_ENTRY HostImageLookupOrdinal(void *img, br_uint_32 ordinal)
{
    return NULL;
}
#elif defined(__unix__)

#include <dlfcn.h>

void *BR_RESIDENT_ENTRY HostImageLoad(const char *name)
{
    return dlopen(name, RTLD_NOW);
}

void BR_RESIDENT_ENTRY HostImageUnload(void *image)
{
    dlclose(image);
}

void *BR_RESIDENT_ENTRY HostImageLookupName(void *img, const char *name, br_uint_32 hint)
{
    (void)hint;
    return dlsym(img, name);
}

void *BR_RESIDENT_ENTRY HostImageLookupOrdinal(void *img, br_uint_32 ordinal)
{
    (void)img;
    (void)ordinal;
    return NULL;
}
#else
void *BR_RESIDENT_ENTRY HostImageLoad(const char *name)
{
    return NULL;
}

void BR_RESIDENT_ENTRY HostImageUnload(void *image)
{
}

void *BR_RESIDENT_ENTRY HostImageLookupName(void *img, const char *name, br_uint_32 hint)
{
    return NULL;
}

void *BR_RESIDENT_ENTRY HostImageLookupOrdinal(void *img, br_uint_32 ordinal)
{
    return NULL;
}
#endif