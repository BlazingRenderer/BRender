/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: fontptrs.c 1.1 1997/12/10 16:41:24 jon Exp $
 * $Locker: $
 *
 * Public pointers to standard fonts
 */
#include "brender.h"


extern struct br_font BR_ASM_DATA _FontFixed3x5;
extern struct br_font BR_ASM_DATA _FontProp4x6;
extern struct br_font BR_ASM_DATA _FontProp7x9;

BR_API struct br_font *BR_ASM_DATA BrFontFixed3x5 = &_FontFixed3x5;
BR_API struct br_font *BR_ASM_DATA BrFontProp4x6  = &_FontProp4x6;
BR_API struct br_font *BR_ASM_DATA BrFontProp7x9  = &_FontProp7x9;
