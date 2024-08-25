#include "drv.h"
#include "shortcut.h"
#include "rend.h"

#define COMPUTE_COMPONENT_MIDPOINT_VALUES(C_P, m0, m1, m2, v0, v1, v2)                            \
    do {                                                                                          \
        (m0)->comp[C_P] = BR_CONST_MUL(BR_ADD((v1)->comp[C_P], (v2)->comp[C_P]), BR_SCALAR(0.5)); \
        (m1)->comp[C_P] = BR_CONST_MUL(BR_ADD((v0)->comp[C_P], (v2)->comp[C_P]), BR_SCALAR(0.5)); \
        (m2)->comp[C_P] = BR_CONST_MUL(BR_ADD((v1)->comp[C_P], (v0)->comp[C_P]), BR_SCALAR(0.5)); \
    } while(0)

void averageVerticesOnScreen(const br_renderer *renderer, brp_vertex *restrict m0, brp_vertex *restrict m1,
                             brp_vertex *restrict m2, brp_vertex *restrict v0, brp_vertex *restrict v1, brp_vertex *restrict v2)
{
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_X, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_Y, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_Z, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_W, m0, m1, m2, v0, v1, v2);

    if(rend.block->convert_mask_f & (CM_R | CM_G | CM_B)) {
        COMPUTE_COMPONENT_MIDPOINT_VALUES(C_R, m0, m1, m2, v0, v1, v2);
        COMPUTE_COMPONENT_MIDPOINT_VALUES(C_G, m0, m1, m2, v0, v1, v2);
        COMPUTE_COMPONENT_MIDPOINT_VALUES(C_B, m0, m1, m2, v0, v1, v2);
    }

    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_I, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_U, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_V, m0, m1, m2, v0, v1, v2);

    PROJECT_VERTEX_WRITE_Q(m0, m0->comp[C_X], m0->comp[C_Y], m0->comp[C_Z], m0->comp[C_W]);
    PROJECT_VERTEX_WRITE_Q(m1, m1->comp[C_X], m1->comp[C_Y], m1->comp[C_Z], m1->comp[C_W]);
    PROJECT_VERTEX_WRITE_Q(m2, m2->comp[C_X], m2->comp[C_Y], m2->comp[C_Z], m2->comp[C_W]);
}

#define OUTCODE_TABLE(name, gENameUCase, lNameUCase)                                                     \
    const br_uint_32 name##Table[16] = {                                                                 \
        OUTCODE_##gENameUCase | OUTCODE_N_##gENameUCase,                                                 \
        OUTCODE_##gENameUCase | OUTCODE_N_##gENameUCase,                                                 \
        0,                                                                                               \
                                                                                                         \
        0,                                                                                               \
                                                                                                         \
        OUTCODE_##gENameUCase | OUTCODE_N_##gENameUCase,                                                 \
        OUTCODE_##gENameUCase | OUTCODE_N_##gENameUCase,                                                 \
        OUTCODE_##gENameUCase | OUTCODE_N_##gENameUCase | OUTCODE_##lNameUCase | OUTCODE_N_##lNameUCase, \
                                                                                                         \
        0,                                                                                               \
                                                                                                         \
        0,                                                                                               \
        OUTCODE_##lNameUCase | OUTCODE_N_##lNameUCase,                                                   \
        0,                                                                                               \
                                                                                                         \
        0,                                                                                               \
                                                                                                         \
        OUTCODE_##gENameUCase | OUTCODE_N_##gENameUCase | OUTCODE_##lNameUCase | OUTCODE_N_##lNameUCase, \
        OUTCODE_##lNameUCase | OUTCODE_N_##lNameUCase,                                                   \
        OUTCODE_##gENameUCase | OUTCODE_N_##gENameUCase | OUTCODE_##lNameUCase | OUTCODE_N_##lNameUCase, \
                                                                                                         \
        0,                                                                                               \
    }

OUTCODE_TABLE(rightLeft, RIGHT, LEFT);
OUTCODE_TABLE(topBottom, TOP, BOTTOM);
OUTCODE_TABLE(hitherYon, HITHER, YON);
