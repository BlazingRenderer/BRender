
#include "brender.h"
#include "v1compat.h"

br_boolean zb_active = BR_FALSE;

/*
 * open zbuffered renderer
 */

void BR_PUBLIC_ENTRY BrZbBegin(br_uint_8 colour_type, br_uint_8 depth_type)
{
    if(!zs_active && !zb_active)
        if(BrV1dbRendererBegin(BrDevLastBeginQuery(), NULL) != BRE_OK)
            BR_FAILURE("Failed to load renderer");

    zb_active = BR_TRUE;
}

/*
 * close zbuffered renderer
 */

void BR_PUBLIC_ENTRY BrZbEnd(void)
{
    zb_active = BR_FALSE;

    if(!zb_active && !zs_active)
        BrV1dbRendererEnd();
}
