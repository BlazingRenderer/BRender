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

br_uint_32 OUTCODE_ORDINATE2(br_uint_32 lValue, br_uint_32 rValue, const br_uint_32 *table, br_uint_32 edx)
{
    br_uint_32 reg0, reg1;
    br_uint_32 esi, edi;

    reg0 = rValue;
    esi  = 0x80000000u;

    esi  = esi & reg0;
    reg1 = lValue;

    edi  = 0x80000000u;
    reg0 = reg0 & 0x7fffffffu;

    esi = esi >> 1;
    edi = edi & reg1;

    esi = esi | edi;
    edi = reg0;

    reg1 = reg1 & 0x7fffffffu;

    edi  = edi - reg1;
    reg1 = reg1 - reg0;

    edi  = edi >> 3;
    reg1 = reg1 & 0x80000000u;

    reg1 = reg1 >> 2;
    esi  = esi | edi;

    esi = esi | reg1;

    reg0 = (br_uint_32)table;

    esi >>= 28;

    reg0 = ((br_uint_32 *)reg0)[esi];

    edx = edx ^ reg0;
    return edx;
}

br_uint_32 OUTCODE_ORDINATE3(br_uint_32 leftValue, br_uint_32 rightValue, const br_uint_32 *outcodeTable, br_uint_32 currentOutcode)
{
    /*
     * NB: lValue and rValue are actually floats, but it's easier to deal with them as br_uint_32s.
     */

    // Extract the sign bit from the left and right values.
    br_uint_32 leftSignBit  = leftValue & 0x80000000u;
    br_uint_32 rightSignBit = rightValue & 0x80000000u;

    // Combine the right sign bit and left sign bit to start building the index.
    br_uint_32 combinedSignBits = (rightSignBit >> 1) | leftSignBit;

    // Get the absolute values of leftValue and rightValue as integers without the sign bit.
    br_uint_32 absLeftValue  = leftValue & 0x7fffffffu;
    br_uint_32 absRightValue = rightValue & 0x7fffffffu;

    // Calculate the difference between the absolute values of rightValue and leftValue.
    br_uint_32 diffRightLeft = (absRightValue - absLeftValue) >> 3;

    // Calculate the difference between the absolute values of leftValue and rightValue,
    // and extract the sign bit from this difference (shifting down to reduce to a smaller range).
    br_uint_32 diffLeftRightSignBit = ((absLeftValue - absRightValue) & 0x80000000u) >> 2;

    // Combine the differences into the index.
    combinedSignBits |= diffRightLeft | diffLeftRightSignBit;

    // Use the top 4 bits of combinedSignBits as an index to the outcode table (16 entries).
    br_uint_32 index            = combinedSignBits >> 28;
    br_uint_32 outcodeFromTable = outcodeTable[index];

    // XOR the calculated outcode with the current outcode to update it.
    currentOutcode ^= outcodeFromTable;

    return currentOutcode;
}
