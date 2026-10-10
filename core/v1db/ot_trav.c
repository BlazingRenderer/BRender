/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: ot_trav.c 1.1 1997/12/10 16:41:32 jon Exp $
 * $Locker: $
 *
 * Order table traversal for the Z sort renderer
 */
#include "v1db.h"
#include "brassert.h"

#include "priminfo.h"

static zs_order_table_traversal_cbfn *callback;

/*
 * Walk a bucket table from last bucket to first, invoking the traversal
 * callback for every primitive in every bucket.
 *
 * This fork is float-only: primitive vertices carry a single scalar array,
 * brp_vertex::comp[], so every arm reads comp[C_SX] and friends directly. The
 * original fixed-point idiom (comp_x[] and BrFixedToScalar(... - 0x8000)) has
 * no meaning in a float build; -0x8000 is -0.5 in 16.16, so the float arms
 * subtract BR_SCALAR(0.5) from the screen co-ordinates just as the triangle
 * arm always did. distanceZ is the depth component, comp[C_W].
 */
static inline br_error GeometryV1BucketsTraverse(br_primitive **buckets, br_int_32 nbuckets)
{
    br_primitive *p;
    ot_vertex     v[3] = {0};

    if(nbuckets <= 0)
        return BRE_OK;

    /*
     * Render bucket table from last to first
     */
    for(buckets += nbuckets - 1; nbuckets--; buckets--) {
        for(p = *buckets; p; p = p->next) {

            ASSERT((p->type == BR_PRIMITIVE_POINT) || (p->type == BR_PRIMITIVE_LINE) || (p->type == BR_PRIMITIVE_TRIANGLE));

            switch(p->type) {
                case BR_PRIMITIVE_TRIANGLE:
                    v[2].screenX   = ((brp_vertex *)p->v[2])->comp[C_SX] - BR_SCALAR(0.5);
                    v[2].screenY   = ((brp_vertex *)p->v[2])->comp[C_SY] - BR_SCALAR(0.5);
                    v[2].distanceZ = ((brp_vertex *)p->v[2])->comp[C_W];
                    v[1].screenX   = ((brp_vertex *)p->v[1])->comp[C_SX] - BR_SCALAR(0.5);
                    v[1].screenY   = ((brp_vertex *)p->v[1])->comp[C_SY] - BR_SCALAR(0.5);
                    v[1].distanceZ = ((brp_vertex *)p->v[1])->comp[C_W];
                    v[0].screenX   = ((brp_vertex *)p->v[0])->comp[C_SX] - BR_SCALAR(0.5);
                    v[0].screenY   = ((brp_vertex *)p->v[0])->comp[C_SY] - BR_SCALAR(0.5);
                    v[0].distanceZ = ((brp_vertex *)p->v[0])->comp[C_W];
                    break;
                case BR_PRIMITIVE_LINE:
                    v[1].screenX   = ((brp_vertex *)p->v[1])->comp[C_SX] - BR_SCALAR(0.5);
                    v[1].screenY   = ((brp_vertex *)p->v[1])->comp[C_SY] - BR_SCALAR(0.5);
                    v[1].distanceZ = ((brp_vertex *)p->v[1])->comp[C_W];
                    /* Falls through: a line has two endpoints, v[1] and v[0] */
                case BR_PRIMITIVE_POINT:
                    v[0].screenX   = ((brp_vertex *)p->v[0])->comp[C_SX] - BR_SCALAR(0.5);
                    v[0].screenY   = ((brp_vertex *)p->v[0])->comp[C_SY] - BR_SCALAR(0.5);
                    v[0].distanceZ = ((brp_vertex *)p->v[0])->comp[C_W];
            }
            callback(p->type, &v[0], &v[1], &v[2]);
        }
    }

    return BRE_OK;
}

static void TraverseOrderTableBuckets(br_primitive **buckets, br_int_32 nbuckets)
{
    GeometryV1BucketsTraverse(buckets, nbuckets);
}

static inline void TraverseOrderTableList(void)
{
    WalkOrderTableList(TraverseOrderTableBuckets);
}

static inline void TraversePrimaryOrderTable(void)
{
    WalkPrimaryOrderTable(TraverseOrderTableBuckets);
}

void BR_PUBLIC_ENTRY ZsOrderTableTraversal(zs_order_table_traversal_cbfn *cbfn)
{
    callback = cbfn;

    if(v1db.format_buckets == NULL)
        BR_ERROR0("Renderer does not support buckets");

    if(v1db.primary_order_table) {
        /*
         * Render primitives in the primary order table
         * and the list of order tables
         */
        TraversePrimaryOrderTable();
    } else {
        /*
         * Render primitives in the list of order table
         */
        TraverseOrderTableList();
    }
}
