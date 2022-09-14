/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brmath.h 2.5 1997/08/13 10:53:20 JOHNG Exp $
 * $Locker: $
 *
 * Wrappers around standard C math functions
 */

#ifndef _BRMATH_H_
#define _BRMATH_H_

#include <stdarg.h>

#ifndef _NO_PROTOTYPES

#include <math.h>

#define BrFloatFloor(f) floorf(f)
#define BrFloatCeil(f) ceilf(f)
#define BrFloatSqrt(f) sqrtf(f)
#define BrFloatPow(a, b) powf(a, b)
#define BrFloatAtan2(x, y) atan2f(x, y)

#endif

#endif
