/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stdmem.c 1.1 1997/12/10 16:41:29 jon Exp $
 * $Locker: $
 *
 * Default memory handler that uses malloc()/free() from C library
 */
#include "brender.h"

#include "tinyalloc/tinyalloc.h"

#ifndef BR_FREESTANDING_HEAP_SIZE
#if defined(BR_FREESTANDING_HEAP_SIZE_MB)
#define BR_FREESTANDING_HEAP_SIZE (BR_FREESTANDING_HEAP_SIZE_MB * 1024 * 1024)
#elif defined(BR_FREESTANDING_HEAP_SIZE_KB)
#define BR_FREESTANDING_HEAP_SIZE (BR_FREESTANDING_HEAP_SIZE_KB * 1024)
#else
#define BR_FREESTANDING_HEAP_SIZE (4 * 1024 * 1024)
#endif
#endif

#ifndef BR_FREESTANDING_HEAP_BLOCKS
#define BR_FREESTANDING_HEAP_BLOCKS (256)
#endif

#ifndef BR_FREESTANDING_HEAP_SPLIT_THRESHOLD
#define BR_FREESTANDING_HEAP_SPLIT_THRESHOLD (16)
#endif

#ifndef BR_FREESTANDING_HEAP_ALIGNMENT
#define BR_FREESTANDING_HEAP_ALIGNMENT sizeof(void *)
#endif

static br_uint_8 br_heap[BR_FREESTANDING_HEAP_SIZE] __attribute__((aligned(4)));
static br_boolean br_heap_initialized = BR_FALSE;

static void InitializeHeap(void)
{
    if (br_heap_initialized)
        return;

    ta_init(br_heap, br_heap + sizeof(br_heap), BR_FREESTANDING_HEAP_BLOCKS, BR_FREESTANDING_HEAP_SPLIT_THRESHOLD, BR_FREESTANDING_HEAP_ALIGNMENT);
    br_heap_initialized = BR_TRUE;
}

static void *BR_CALLBACK BrFreestandingAllocate(br_size_t size, br_uint_8 type)
{
    void *m;

    InitializeHeap();

    m = ta_alloc(size);

    if(m == NULL)
        BR_ERROR2("BrFreestandingAllocate: failed with size=%d, type=%d", size, type);

    return m;
}

static void *BR_CALLBACK BrFreestandingReallocate(void *ptr, br_size_t size, br_uint_8 type)
{
    void *m;

    InitializeHeap();

    m = ta_realloc(ptr, size);

    if(m == NULL)
        BR_ERROR2("BrFreestandingReallocate: failed with size=%d, type=%d", size, type);

    return m;
}

static void BR_CALLBACK BrFreestandingFree(void *mem)
{
    InitializeHeap();
    ta_free(mem);
}

static br_size_t BR_CALLBACK BrFreestandingInquire(br_uint_8 type)
{
    InitializeHeap();
    return ta_size_free();
}

static br_uint_32 BR_CALLBACK BrFreestandingAlign(br_uint_8 type)
{
    InitializeHeap();
    return BR_FREESTANDING_HEAP_ALIGNMENT;
}

/*
 * Allocator structure
 */
br_allocator BrFreestandingAllocator = {
    .identifier = "freestanding",
    .allocate   = BrFreestandingAllocate,
    .reallocate = BrFreestandingReallocate,
    .free       = BrFreestandingFree,
    .inquire    = BrFreestandingInquire,
    .align      = BrFreestandingAlign,
};

/*
 * Override global variable s.t. this is the default allocator
 */
br_allocator *BR_ASM_DATA _BrDefaultAllocator = &BrFreestandingAllocator;
