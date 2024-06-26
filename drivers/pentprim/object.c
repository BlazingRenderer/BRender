/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: object.c 1.1 1997/12/10 16:47:28 jon Exp $
 * $Locker: $
 *
 * Object methods
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "shortcut.h"
#include "brassert.h"


/*
 * Methods for default object object
 */
const char * BR_CMETHOD_DECL(br_object_softprim, identifier)(br_object *self)
{
	return self->identifier;
}

br_device *	BR_CMETHOD_DECL(br_object_softprim, device)(br_object *self)
{
	return self->device;
}
