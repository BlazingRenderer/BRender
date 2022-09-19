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

#ifdef __cplusplus
extern "C" {
#endif

float BR_PUBLIC_ENTRY BrFloatFloor(float f);
float BR_PUBLIC_ENTRY BrFloatCeil(float f);
float BR_PUBLIC_ENTRY BrFloatSqrt(float f);
float BR_PUBLIC_ENTRY BrFloatPow(float a, float b);
float BR_PUBLIC_ENTRY BrFloatAtan2(float x, float y);

#ifdef __cplusplus
};
#endif

#endif

#endif
