/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: defmem.c 1.1 1997/12/10 16:41:06 jon Exp $
 * $Locker: $
 *
 * Default memory handler that does nothing
 */

#include "brender.h"

static void *BrNullAllocate(br_size_t size, br_uint_8 type)
{
    return 0;
}

static void *BrNullReallocate(void *ptr, br_size_t size, br_uint_8 type)
{
    return NULL;
}

static void BrNullFree(void *mem)
{
}

static br_size_t BrNullInquire(br_uint_8 type)
{
    return 0;
}

/*
 * Allocator structure
 */
br_allocator BrNullAllocator = {
    "Null", BrNullAllocate, BrNullReallocate, BrNullFree, BrNullInquire,
};

// Nope, we're using std now
///*
// * Global variable that can be overridden by linking something first
// */
// br_allocator *_BrDefaultAllocator = &BrNullAllocator;
