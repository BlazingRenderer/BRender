/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * BRender intro sequence
 *
 *	22/2/95	SE	Runs under BRender v 1.1, 8 bit indexed
 *	22/5/95 SE	Added video card timing
 *	28/5/95 SE	Fix for last frame sphere/word pixel bug thing...
 *	08/9/95	SE	Animation changed to matrix stream
 *			Fixed for all res 8 bit indexed
 *	13/9/95 SE	Error codes, clean exit
 *	14/9/95	SE	Fixed for v1.1.2
 *
 * NB. DOS 8 bit indexed only!  Artwork needs to be reworked for true colour.
 *
 * pass pointer to screen device pixelmap (th'one returned by DOSGfxBegin)
 *
 * return	
 *   		0	no error
 *		1	failed to load pixelmaps
 *		2	failed to find materials
 *		3	failed to find shade table
 *		4	failed to load models
 *		5	failed to open animation file
 *		6	failed to load animation
 *
 *		hardware palette contains all (rgb = 0,0,0)
 *		screen all index 0
 *
 * needs BrBegin and DOSGfxBegin to have already been performed
 * doesn't need (and like) BrZbBegin to have been performed yet!
 *
 * Handy hint!	Call this before you set up colour and z buffers as this
 *		will use sooo much memory because local c & z buffers
 *		are used here.
 */
 
