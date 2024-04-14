/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brintro.h 1.1 1995/02/22 21:36:54 joey Exp $
 * $Locker: $
 *
 * Function prototypes for BRender intro sequence
 */

#ifndef _BRINTRO_H_
#define _BRINTRO_H_

/*
 * NB. DOS 8 bit indexed only!
 *
 * pass pointer to screen device pixelmap (th'one returned by DOSGfxBegin)
 * return	0 everything ok,
 * 		-1 some error occurred
 */
 
int BrenderIntro(br_pixelmap *screen);


#endif

