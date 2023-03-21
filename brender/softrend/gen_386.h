/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: gen_386.h 1.3 1998/07/20 19:52:18 jon Exp $
 * $Locker: $
 *
 * Prototypes for 386 downcoded geometry support
 */
#ifndef _GEN_386_H_
#define _GEN_386_H_


#ifdef __cplusplus
extern "C" {
#endif

br_boolean BR_ASM_CALL ModelToViewportUpdate_A(void);

#if BASED_FLOAT

#endif

#if BASED_FIXED

/*
 * Need to be converted to use separate face equations and
 * vertex indices
 */
#if 1

#define ModelToViewportUpdate() (BR_FALSE)

#else

/*
 * N.B. Not sure why these are disabled (looks like Sam did it just before
 * 1.3 was released) but if they are ever reenabled, it will be necessary
 * to make them write out C_Q (in floating point, even though it's a fixed
 * point build) if necessary
 */
#define ModelToViewportUpdate					ModelToViewportUpdate_A

#endif

#endif

#ifdef __cplusplus
};
#endif
#endif

