/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stdmem.c 1.1 1997/12/10 16:41:29 jon Exp $
 * $Locker: $
 *
 * Default memory handler that uses malloc()/free() from C library
 */
#include "brender.h"

/*
 * Glue functions for malloc()/free()
 */
static void *BR_CALLBACK BrDreamcastAllocate(br_size_t size, br_uint_8 type)
{
    void *m;

    m = malloc(size);

    if(m == NULL)
        BR_ERROR2("BrDreamcastAllocate: failed with size=%d, type=%d", size, type);

    return m;
}

static void *BR_CALLBACK BrDreamcastReallocate(void *ptr, br_size_t size, br_uint_8 type)
{
    void *m;

    m = realloc(ptr, size);

    if(m == NULL)
        BR_ERROR2("BrDreamcastReallocate: failed with size=%d, type=%d", size, type);

    return m;
}

static void BR_CALLBACK BrDreamcastFree(void *mem)
{
    free(mem);
}

static br_size_t BR_CALLBACK BrDreamcastInquire(br_uint_8 type)
{
    /* XXX */
    return 0;
}

static br_uint_32 BR_CALLBACK BrDreamcastAlign(br_uint_8 type)
{
    /* This is 4 on 32-bit systems. I'm assuming this is 8 on 64-bit. */
    return sizeof(void *);
}

/*
 * Allocator structure
 */
br_allocator BrDreamcastAllocator = {
    .identifier = "malloc",
    .allocate   = BrDreamcastAllocate,
    .reallocate = BrDreamcastReallocate,
    .free       = BrDreamcastFree,
    .inquire    = BrDreamcastInquire,
    .align      = BrDreamcastAlign,
};

/*
 * Override global variable s.t. this is the default allocator
 */
br_allocator *BR_ASM_DATA _BrDefaultAllocator = &BrDreamcastAllocator;
