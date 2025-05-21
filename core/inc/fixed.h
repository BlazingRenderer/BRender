/*
 * Copyright (c) 1992,1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: fixed.h 1.1 1997/12/10 16:41:17 jon Exp $
 * $Locker: $
 */
#ifndef _FIXED_H_
#define _FIXED_H_

#ifndef INLINE_FIXED
#define INLINE_FIXED 1
#endif

/*
 * Fixed point types
 */
typedef br_int_32 br_fixed_ls;  /* Long Signed fixed             15.16 */
typedef br_int_16 br_fixed_lsf; /* Long Signed fraction           0.15 */

typedef br_uint_32 br_fixed_lu;  /* Long unsigned fixed	         16.16 */
typedef br_uint_16 br_fixed_luf; /* Long unsigned fixed fraction  0.16 */

typedef br_int_16 br_fixed_ss; /* Short Signed fixed              7.8  */

typedef br_int_8 br_fixed_ssf; /* Short Signed fraction           0.7  */

typedef br_uint_16 br_fixed_su;  /* Short unsigned fixed          8.8  */
typedef br_uint_8  br_fixed_suf; /* Short unsigned fixed fraction 0.8  */

/*
 * 0 in various fixed-point forms
 */
#define BR_ZERO_LS  0
#define BR_ZERO_LSF 0
#define BR_ZERO_LU  0
#define BR_ZERO_LUF 0

/*
 * 1 in various fixed point forms
 */
#define BR_ONE_LS  65536 /* (1<<16) */
#define BR_ONE_LSF 32768 /* (1<<15) */
#define BR_ONE_LU  65536 /* (1<<16) */
#define BR_ONE_LUF 65536 /* (1<<16) */

#define BR_ONE_SS  256 /* (1<<8)  */
#define BR_ONE_SSF 128 /* (1<<7)  */
#define BR_ONE_SU  256 /* (1<<8)  */
#define BR_ONE_SUF 256 /* (1<<8)  */

/*
 * Radians in 0.16 forms.
 */
#define BR_PI_4_LUF          0x2000 /* 0.125 turns = pi/4 rad     */
#define BR_PI_2_LUF          0x4000 /* 0.250 turns = pi/2 rad     */
#define BR_3_PI_4_LUF        0x6000 /* 0.375 turns = (3*pi)/4 rad */
#define BR_PI_LUF            0x8000 /* 0.500 turns = pi rad       */
#define BR_5_PI_4_LUF        0xA000 /* 0.625 turns = (5*pi)/4 rad */
#define BR_3_PI_2_LUF        0xC000 /* 0.750 turns = (3*pi)/2 rad */
#define BR_7_PI_4_LUF        0xE000 /* 0.875 turns = (7*pi)/4 rad */

#define BrIntToFixed(i)      ((i) << 16)
#define BrFixedToInt(i)      ((i) >> 16)
#define BrFloatToFixed(f)    ((br_fixed_ls)((f) * (float)BR_ONE_LS))
#define BrFixedToFloat(s)    ((float)((s) * (1.0f / (float)BR_ONE_LS)))

#define BrFloatToFixedLS(f)  ((br_fixed_ls)((f) * (float)BR_ONE_LS))
#define BrFixedLSToFloat(s)  ((float)((s) * (1.0f / (float)BR_ONE_LS)))

#define BrFixedLSFToFloat(s) ((float)((s) * (1.0f / (float)BR_ONE_LSF)))
#define BrFixedLUFToFloat(s) ((float)((s) * (1.0f / (float)BR_ONE_LUF)))

#define BrFloatToFixedLSF(f) ((br_fixed_lsf)((f) * (float)BR_ONE_LSF))
#define BrFloatToFixedLUF(f) ((br_fixed_luf)((f) * (float)BR_ONE_LUF))

#endif
