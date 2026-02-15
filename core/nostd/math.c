/*
 * Copyright (c) 1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: math.c 1.2 1998/07/20 21:52:44 jon Exp $
 * $Locker: $
 *
 * Glue to math functions
 */

#include "brender.h"

float BR_PUBLIC_ENTRY BrFloatFloor(float f)
{
    return __builtin_floorf(f);
}

float BR_PUBLIC_ENTRY BrFloatCeil(float f)
{
    return __builtin_ceilf(f);
}

float BR_PUBLIC_ENTRY BrFloatSqrt(float f)
{
    return __builtin_sqrtf(f);
}

float BR_PUBLIC_ENTRY BrFloatPow(float a, float b)
{
    return __builtin_powf(a, b);
}

float BR_PUBLIC_ENTRY BrFloatAtan2(float x, float y)
{
    return __builtin_atan2f(x, y);
}
