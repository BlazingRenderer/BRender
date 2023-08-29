
#include "brender.h"
#include "v1compat.h"

br_boolean zs_active = BR_FALSE;

/*
 * open zsort renderer
 */

void BR_PUBLIC_ENTRY BrZsBegin(br_uint_8 colour_type, void *primitive, br_uint_32 size)
{
    if(!zs_active && !zb_active)
        if(BrV1dbRendererBegin(BrDevLastBeginQuery(), NULL) != BRE_OK)
            BR_FAILURE("Failed to load renderer");

    zb_active = BR_TRUE;
}

/*
 * close zsort renderer
 */

void BR_PUBLIC_ENTRY BrZsEnd(void)
{
    zs_active = BR_FALSE;

    if(!zb_active && !zs_active)
        BrRendererEnd();
}
