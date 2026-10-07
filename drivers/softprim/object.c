/*
 * softprim object methods
 */
#include <stddef.h>
#include <string.h>

#include "drv.h"
#include "brassert.h"

const char *BR_CMETHOD_DECL(br_object_softprim, identifier)(br_object *self)
{
    return self->identifier;
}

br_device *BR_CMETHOD_DECL(br_object_softprim, device)(br_object *self)
{
    return self->device;
}
