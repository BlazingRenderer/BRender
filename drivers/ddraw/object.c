/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: object.c 1.1 1997/12/10 16:45:22 jon Exp $
 * $Locker: $
 *
 * Local methods for default object
 *
 */
#include <stddef.h>
#include <string.h>
#include <windows.h>
#include <ddraw.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

/*
 * Get the identifier associated with a object
 */
const char *BR_CMETHOD_DECL(br_object_dd, identifier)(br_object *self)
{
    return self->identifier;
}

/*
 * Find the device associated with a object
 */
br_device *BR_CMETHOD_DECL(br_object_dd, device)(br_object *self)
{
    return self->device;
}
