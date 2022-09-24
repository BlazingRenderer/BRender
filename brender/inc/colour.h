/*
* Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
*
* $Id: colour.h 2.5 1997/02/06 14:49:53 Johng Exp $
* $Locker: $
*
* Colour type and macros
*
*/
#ifndef _COLOUR_H_
#define _COLOUR_H_

#include "compiler.h"
typedef br_uint_32 br_colour;

/*
 * N.B. Colour ranges are specified in the format of the pixelmap to which
 * they refer.  For indexed pixelmaps they specify a range of indices.
 * For RGB and YUV pixelmaps they specify a cube of colours in the
 * colourspace bounded by the planes obtained by separating out each
 * component from the low and high bounds.
 *
 * Note that the range is always specified inclusively.
 */
typedef struct br_colour_range
{
	br_colour low;
	br_colour high;
} br_colour_range;

#define BR_COLOUR_RGB(r,g,b) \
		((((br_colour)(r))<<16) |\
		(((br_colour)(g))<<8) |\
		((br_colour)(b)))

#define BR_COLOUR_RGBA(r,g,b,a) \
		((((br_colour)(a))<<24) |\
		(((br_colour)(r))<<16) |\
		(((br_colour)(g))<<8) |\
		((br_colour)(b)))

#define BR_COLOUR_ARGB(a,r,g,b) \
		((((br_colour)(a))<<24) |\
		(((br_colour)(r))<<16) |\
		(((br_colour)(g))<<8) |\
		((br_colour)(b)))

#define BR_COLOUR_565(r,g,b) \
		((((br_colour)(r))<<11) |\
		(((br_colour)(g))<<5) |\
		((br_colour)(b)))

#define BR_ALPHA(c) (((c) >> 24) & 0xFF)
#define BR_RED(c) (((c) >> 16) & 0xFF)
#define BR_GRN(c) (((c) >> 8) & 0xFF)
#define BR_BLU(c) ((c) & 0xFF)

#define BR_RED_565(c) (((c) >> 11 & 31))
#define BR_GRN_565(c) (((c) >> 5) & 63)
#define BR_BLU_565(c) ((c) & 31)

#endif

