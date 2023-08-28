
#include "brender.h"

br_boolean zs_active;

/*
 * open zsort renderer
 */

void BR_PUBLIC_ENTRY BrZsBegin(br_uint_8 colour_type, void *primitive, br_uint_32 size)
{
    zs_active = BR_TRUE;
}

/*
 * close zsort renderer
 */

void BR_PUBLIC_ENTRY BrZsEnd(void)
{
    zs_active = BR_FALSE;
}
