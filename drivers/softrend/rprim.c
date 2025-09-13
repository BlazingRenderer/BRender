/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: rprim.c 1.1 1997/12/10 16:52:35 jon Exp $
 * $Locker: $
 *
 * Geometry format methods
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

br_error BR_CMETHOD_DECL(br_geometry_format_soft, render_primitives)(br_geometry *self, br_renderer *r)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_geometry_format_soft, storedNew_primitives)(br_geometry *self, br_renderer *r)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_geometry_format_soft, prepare_primitives)(br_geometry *self, br_renderer *r)
{
    return BRE_FAIL;
}
