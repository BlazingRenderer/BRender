/*
 * Copyright (c) 2025 Zane van Iperen.
 *
 * SPDX-License-Identifier: MIT
 *
 * Basic Hash Functions.
 */
#include "fw.h"

#ifndef __DJGPP__
#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "xxhash.h"
#endif

br_hash BR_RESIDENT_ENTRY BrHash(const void *data, size_t size)
{
#ifdef __DJGPP__
    // FIXME: implement this with some other backend
    return BR_INVALID_HASH;
#else
    if(data == NULL && size != 0)
        return BR_INVALID_HASH;

    /*
     * Use whichever one matches the machine's native word type.
     * See https://stackoverflow.com/a/67223865
     */
#if defined(_WIN64) || LONG_MAX == 9223372036854775807L
    return XXH3_64bits_withSeed(data, size, 0);
#else /* LONG_MAX == 2147483647L */
    return XXH32(data, size, 0);
#endif
#endif
}

br_hash BR_RESIDENT_ENTRY BrHashString(const char *s)
{
    if(s == NULL)
        return BR_INVALID_HASH;

    return BrHash(s, BrStrLen(s));
}
