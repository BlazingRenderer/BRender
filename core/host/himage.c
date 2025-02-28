/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: himage.c 2.7 1997/05/02 15:41:08 jon Exp $
 * $Locker: $
 *
 * Using images from host environment
 */
#include <SDL3/SDL.h>
#include "brender.h"

#include "host.h"

void *BR_RESIDENT_ENTRY HostImageLoad(const char *name)
{
    return SDL_LoadObject(name);
}

void BR_RESIDENT_ENTRY HostImageUnload(void *image)
{
    SDL_UnloadObject(image);
}

void *BR_RESIDENT_ENTRY HostImageLookupName(void *img, const char *name, br_uint_32 hint)
{
    (void)hint;
    return (void *)SDL_LoadFunction(img, name);
}

void *BR_RESIDENT_ENTRY HostImageLookupOrdinal(void *img, br_uint_32 ordinal)
{
    (void)img;
    (void)ordinal;
    return NULL;
}
