
#include "brender.h"

br_boolean zb_active;

/*
 * open zbuffered renderer
 */

void BR_PUBLIC_ENTRY BrZbBegin(br_uint_8 colour_type, br_uint_8 depth_type)
{
    zb_active = BR_TRUE;
}

/*
 * close zbuffered renderer
 */

void BR_PUBLIC_ENTRY BrZbEnd(void)
{
    zb_active = BR_FALSE;
}
