/*
* Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
*
* $Id: scalar.h 2.10 1996/10/03 11:10:11 sam Exp $
* $Locker: $
*
* Scalar type - 32 bit floating point
*/
#ifndef _SCALAR_H_
#define _SCALAR_H_

/**
** Basic types - float
**/
#include <math.h>

/*
* Floating point base types
*/
typedef br_float br_scalar;
typedef br_float br_fraction;
typedef br_float br_ufraction;

#define BR_SCALAR_EPSILON	1.192092896e-7f
#define BR_SCALAR_MAX		3.402823466e+38f
#define BR_SCALAR_MIN		(-3.402823466e+38f)

#define BR_SCALAR_TOKEN		BRT_FLOAT

/*
* Macros for static initialisation
*/
#define BR_SCALAR(x)		((br_scalar)(x))
#define BR_FRACTION(x)		((br_fraction)(x))
#define BR_UFRACTION(x)		((br_ufraction)(x))

/*
* Macros for type conversion
*/
#define BrFloatToScalar(f)	(f)
#define BrScalarToFloat(c)	(c)

#define BrIntToScalar(i)	((br_scalar)(i))
#define BrScalarToInt(s)	((int)(s))

#define BrFixedToScalar(f)	((br_scalar)((f)/(br_float)BR_ONE_LS))
#define BrScalarToFixed(s)	(br_fixed_ls)((s)*(br_scalar)BR_ONE_LS)

#define BrFractionToScalar(f)	(f)
#define BrScalarToFraction(s)	(s)

#define BrUFractionToScalar(f)	(f)
#define BrScalarToUFraction(s)	(s)

/*
* Various arithmetic operations
*/
#define BR_ADD(a,b)				((a)+(b))
#define BR_SUB(a,b)				((a)-(b))
#define BR_MUL(a,b)				((a)*(b))
#define BR_SQR(a)				((a)*(a))

#define BR_ABS(a)				((br_scalar)fabs(a))
#define BR_NEG(a)				(-(a))

#define BR_DIV(a,b)				((a)/(b))
#define BR_DIVR(a,b)			((a)/(b))
#define BR_MULDIV(a,b,c)		((a)*(b)/(c))
#define BR_RCP(a)				((br_scalar)(1.0f/(a)))

#define BR_CONST_MUL(a,b)		((a)*(b))
#define BR_CONST_DIV(a,b)		((a)/(b))


#define BR_MAC2(a,b,c,d)		((a)*(b)+(c)*(d))
#define BR_MAC3(a,b,c,d,e,f)	((a)*(b)+(c)*(d)+(e)*(f))
#define BR_MAC4(a,b,c,d,e,f,g,h) ((a)*(b)+(c)*(d)+(e)*(f)+(g)*(h))

#define BR_MAC2DIV(a,b,c,d,e)		(((a)*(b)+(c)*(d))/e)
#define BR_MAC3DIV(a,b,c,d,e,f,g)	(((a)*(b)+(c)*(d)+(e)*(f))/g)
#define BR_MAC4DIV(a,b,c,d,e,f,g,h,i) (((a)*(b)+(c)*(d)+(e)*(f)+(g)*(h))/i)

#define BR_SQR2(a,b)			((a)*(a)+(b)*(b))
#define BR_SQR3(a,b,c)			((a)*(a)+(b)*(b)+(c)*(c))
#define BR_SQR4(a,b,c,d)		((a)*(a)+(b)*(b)+(c)*(c)+(d)*(d))

#define BR_FMAC2(a,b,c,d)		((a)*(b)+(c)*(d))
#define BR_FMAC3(a,b,c,d,e,f)	((a)*(b)+(c)*(d)+(e)*(f))
#define BR_FMAC4(a,b,c,d,e,f,g,h) ((a)*(b)+(c)*(d)+(e)*(f)+(g)*(h))

#define BR_LENGTH2(a,b)			((br_scalar)sqrtf((a)*(a)+(b)*(b)))
#define BR_LENGTH3(a,b,c)		((br_scalar)sqrtf((a)*(a)+(b)*(b)+(c)*(c)))
#define BR_LENGTH4(a,b,c,d)		((br_scalar)sqrtf((a)*(a)+(b)*(b)+(c)*(c)+(d)*(d)))

#define BR_RLENGTH2(a,b)		((br_scalar)(1.0f/sqrtf((a)*(a)+(b)*(b))))
#define BR_RLENGTH3(a,b,c)		((br_scalar)(1.0f/sqrtf((a)*(a)+(b)*(b)+(c)*(c))))
#define BR_RLENGTH4(a,b,c,d)	((br_scalar)(1.0f/sqrtf((a)*(a)+(b)*(b)+(c)*(c)+(d)*(d))))

#define BR_POW(a,b)				((br_scalar)powf((a),(b)))
#define BR_SQRT(a)				((br_scalar)sqrtf(a))

#define BR_FLOOR(a)				((br_scalar)floorf(a))
#define BR_CEIL(a)				((br_scalar)ceilf(a))

/*
* Make sure PI is defined
*/
#ifndef PI
#define PI		3.14159265358979323846f
#endif

/*
* Build independant types
*/
typedef br_fixed_ls		br_scalar_x;
typedef br_fixed_lsf	br_fraction_x;
typedef br_fixed_luf	br_ufraction_x;

typedef br_float br_scalar_f;
typedef br_float br_fraction_f;
typedef br_float br_ufraction_f;

/*
 * Some macros to speed up floating point comparisons
 *
 * N.B. when comparing two scalars in floating point
 * 		at least one must be non-negative
 * 		-0 < 0
 */

#define BR_SCALAR_LE_0(a) (*(br_int_32 *)&(a) <= 0)
#define BR_SCALAR_GE_1(a) (*(br_int_32 *)&(a) >= 0x3f800000)
#define BR_SCALAR_LE_2_EPSILON(a) (*(br_int_32 *)&(a) <= 0x34800000)
#define BR_SCALAR_GT_SPECULARPOW_CUTOFF(a) (*(br_int_32 *)&(a) > 0x3f1e00d2)
#define BR_SCALAR_LT(a,b) (*(br_int_32 *)&(a) < *(br_int_32 *)&(b))
#define BR_SCALAR_GT(a,b) (*(br_int_32 *)&(a) > *(br_int_32 *)&(b))

#endif
