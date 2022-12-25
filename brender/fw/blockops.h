/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: blockops.h 1.1 1997/12/10 16:41:03 jon Exp $
 * $Locker: $
 *
 * Memory operations, block fill, block copy etc.
 */

#ifndef _BLOCKOPS_H_
#define _BLOCKOPS_H_

__inline void BrBlockFill(void *dest_ptr, br_int_32 value, br_int_32 dwords)
{
	memset(dest_ptr, value, dwords*4);
}

__inline void BrBlockCopy(void *dest_ptr, void *src_ptr, br_int_32 dwords)
{
	memcpy(dest_ptr, src_ptr, dwords*4);
}

#endif /* _BLOCKOPS_H_ */
