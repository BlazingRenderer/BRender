/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brddi.h 1.2 1998/11/12 13:14:23 johng Exp $
 * $Locker: $
 *
 * Master include file for devices
 */
#ifndef _BRDDI_H_
#define _BRDDI_H_

#define __BRENDER_DDI__ BR_VERSION(2, 0, 0)

/*
 * Pull in all the public definitions/declarations
 */
#ifndef _BRENDER_H_
#include "brender.h"
#endif

#ifndef _POOL_H_
#include "pool.h"
#endif

#ifndef _REGISTER_H_
#include "register.h"
#endif

#ifndef _TOKEN_H_
#include "token.h"
#endif

#ifndef _TOKENMAP_H_
#include "tokenmap.h"
#endif

#ifndef _LEXER_H_
#include "lexer.h"
#endif

#ifndef _BRIMAGE_H_
#include "brimage.h"
#endif

#ifndef _DEV_OBJS_H_
// FIXME: These really should go somewhere else, but I just need to shutup the compiler.
struct v11model_f;
struct v11model;
struct brp_block;
union brp_vertex;
struct fmt_vertex;
#include "dev_objs.cgh"
#endif

/*
 * The parts of renderer state that a stored state carries: what a material maps
 * itself onto, and what a driver's own stored-state mask copies. softrend's adds
 * BR_STATE_CACHE to it because its cache is derived from those parts.
 */
#define BR_STATE_STORED_PARTS_MASK (BR_STATE_SURFACE | BR_STATE_PRIMITIVE | BR_STATE_CULL)

/*
 * Pull in DDI prototypes
 */
#ifndef _NO_PROTOTYPES

#ifndef _BRDDI_P_H_
#include "brddi_p.h"
#endif

#endif
#endif
