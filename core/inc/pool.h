/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: pool.h 1.1 1997/12/10 16:41:19 jon Exp $
 * $Locker: $
 *
 * Fixed size block pools
 */
#ifndef _POOL_H_
#define _POOL_H_

#define BR_POOL_DEBUG 0
#define BR_POOL_ALIGN 7

/**
 * \brief This structure is simply used as a means of 'spiking' freed pool blocks until they can be
 *        reused.
 */
typedef struct br_pool_block {
    /**
     * \brief A pointer to the next free pool block.
     *
     * This is really an area of memory with size as specified in the pool descriptor (br_pool).
     */
    struct br_pool_block *next;
} br_pool_block;

/**
 * \brief This structure holds details of a memory block pool (an optimised memory allocation scheme
 *        on a per data structure basis).
 *
 * This is ideal for cases where a particular data structure has intensive dynamic storage, e.g.
 * matrices.
 *
 * Note that currently, there is no automatic contraction of Pools, nor are there functions to
 * achieve this (with the current implementation it would have a relatively high processing
 * overhead). Therefore, it may be wise to destroy Pools when possible, if they have irregular or
 * sporadic use, or memory is at a premium.
 */
typedef struct br_pool {
    /**
     * \brief This is a pointer to the next available block of memory (of size block_size).
     *
     * When freed, blocks are cast as br_pool_block, and inserted at the head of the list to keep track
     * of them. An allocation simply removes the item at the head from the list. It may then be
     * initialised as required.
     */
    br_pool_block *free;
    /**
     * \brief This is the size of each item, or memory block.
     *
     * Its value must be greater than or equal to sizeof(br_pool_block), if not, memory corruption will
     * ensue. This is typically 4 bytes, but given that pools are intended for larger structures, this
     * is not a significant restriction. See BrPoolAllocate().
     */
    br_uint_32     block_size;
    /**
     * \brief When all the blocks in a pool are allocated (or the pool is empty), the value of this
     *        member determines the number of blocks that will be added to the pool to enable further
     *        block allocations.
     */
    br_uint_32     chunk_size;
    /**
     * \brief This is the memory class from which the blocks should be allocated.
     *
     * Any number of pools may exist for a given memory class.
     */
    int            mem_type;
#if BR_POOL_DEBUG
    br_uint_32 max_count;
    br_uint_32 count;
#endif
} br_pool;

/*
 * Speedup macros
 */
#if 0
#if !POOL_DEBUG
br_pool_block *__bp; /* Hmm, this global is not optimizer friendly */

#define BrPoolAllocate(pool) (void *)(((pool)->free ? 0 : BrPoolAddChunk(pool)), (__bp = (pool)->free), ((pool)->free = __bp->next, __bp))

#define BrPoolFree(pool, bp) ((bp)->next = (pool)->free, (pool)->free = (bp))
#endif
#endif

#endif
