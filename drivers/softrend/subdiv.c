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

static br_uint_32 outcodeOrdinate(br_scalar lValue, br_scalar rValue, const br_uint_32 *table)
{
    // ;assumes edx contains outcode flags,esi target for discriptor edi,reg0,reg1
    // ;14 cycles - (at least 2.5 wasted cycles)
    br_uint_32 reg0, reg1;
    br_uint_32 esi, edi;

    // mov reg0,rValue
    BrMemCpy(&reg0, &rValue, sizeof(br_uint_32));

    // mov esi,080000000h
    esi = 0x80000000;

    // and esi,reg0
    esi &= reg0;

    // mov reg1,lValue
    BrMemCpy(&reg1, &lValue, sizeof(br_uint_32));

    // mov edi,080000000h
    edi = 0x80000000;
    // and reg0,07fffffffh
    reg0 &= 0x07fffffff;

    // shr esi,1
    esi >>= 1;
    // and edi,reg1
    edi &= reg1;

    // or esi,edi
    esi |= edi;

    // mov edi,reg0
    edi = reg0;

    // and reg1,07fffffffh
    reg1 &= 0x07fffffff;

    // sub edi,reg1
    edi -= reg1;
    // sub reg1,reg0
    reg1 -= reg0;

    // shr edi,3
    edi >>= 3;
    // and reg1,080000000h
    reg1 &= 0x80000000;

    // shr reg1,2
    reg1 >>= 2;
    // or esi,edi
    esi |= edi;

    // or esi,reg1
    esi |= reg1;

    return table[esi >> 28];
}

void averageVertices(const br_renderer *renderer, brp_vertex *restrict m0, brp_vertex *restrict m1,
                     brp_vertex *restrict m2, brp_vertex *restrict v0, brp_vertex *restrict v1, brp_vertex *restrict v2)
{

    /*
     * Perform averaging
     */
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_X, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_Y, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_Z, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_W, m0, m1, m2, v0, v1, v2);

    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_R, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_G, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_B, m0, m1, m2, v0, v1, v2);

    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_I, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_U, m0, m1, m2, v0, v1, v2);
    COMPUTE_COMPONENT_MIDPOINT_VALUES(C_V, m0, m1, m2, v0, v1, v2);

    /*
     * Outcoding
     */
    m0->flags = OUTCODES_NOT;
    m0->flags ^= (br_int_32)outcodeOrdinate(m0->comp[C_X], m0->comp[C_W], rightLeftTable);
    m0->flags ^= (br_int_32)outcodeOrdinate(m0->comp[C_Y], m0->comp[C_W], topBottomTable);
    m0->flags ^= (br_int_32)outcodeOrdinate(m0->comp[C_Z], m0->comp[C_W], hitherYonTable);

    m1->flags = OUTCODES_NOT;
    m1->flags ^= (br_int_32)outcodeOrdinate(m1->comp[C_X], m1->comp[C_W], rightLeftTable);
    m1->flags ^= (br_int_32)outcodeOrdinate(m1->comp[C_Y], m1->comp[C_W], topBottomTable);
    m1->flags ^= (br_int_32)outcodeOrdinate(m1->comp[C_Z], m1->comp[C_W], hitherYonTable);

    m2->flags = OUTCODES_NOT;
    m2->flags ^= (br_int_32)outcodeOrdinate(m2->comp[C_X], m2->comp[C_W], rightLeftTable);
    m2->flags ^= (br_int_32)outcodeOrdinate(m2->comp[C_Y], m2->comp[C_W], topBottomTable);
    m2->flags ^= (br_int_32)outcodeOrdinate(m2->comp[C_Z], m2->comp[C_W], hitherYonTable);

    /*
     * Perform clip plane outcoding if necessary.
     */
    if(scache.user_clip_active) {
        for(br_size_t i = 0; i < MAX_STATE_CLIP_PLANES; ++i) {
            const struct state_clip *cp = renderer->state.clip + i;
            br_vector4               v;

            if(cp->type != BRT_PLANE)
                continue;

            v = (br_vector4){
                .v = {m0->comp[C_X], m0->comp[C_Y], m0->comp[C_Z], m0->comp[C_W]}
            };
            if(BrVector4Dot(&cp->plane, &v) < S0)
                m0->flags ^= (OUTCODE_USER | OUTCODE_N_USER) << (i + 1);

            v = (br_vector4){
                .v = {m1->comp[C_X], m1->comp[C_Y], m1->comp[C_Z], m1->comp[C_W]}
            };
            if(BrVector4Dot(&cp->plane, &v) < S0)
                m1->flags ^= (OUTCODE_USER | OUTCODE_N_USER) << (i + 1);

            v = (br_vector4){
                .v = {m2->comp[C_X], m2->comp[C_Y], m2->comp[C_Z], m2->comp[C_W]}
            };
            if(BrVector4Dot(&cp->plane, &v) < S0)
                m2->flags ^= (OUTCODE_USER | OUTCODE_N_USER) << (i + 1);
        }
    }

    /*
     * Project if necessary.
     */
    if(m0->flags & OUTCODES_ALL || m1->flags & OUTCODES_ALL || m2->flags & OUTCODES_ALL) {
        if((m0->flags & OUTCODES_ALL) == 0)
            PROJECT_VERTEX_WRITE_Q(m0, m0->comp[C_X], m0->comp[C_Y], m0->comp[C_Z], m0->comp[C_W]);

        if((m1->flags & OUTCODES_ALL) == 0)
            PROJECT_VERTEX_WRITE_Q(m1, m1->comp[C_X], m1->comp[C_Y], m1->comp[C_Z], m1->comp[C_W]);

        if((m2->flags & OUTCODES_ALL) == 0)
            PROJECT_VERTEX_WRITE_Q(m2, m2->comp[C_X], m2->comp[C_Y], m2->comp[C_Z], m2->comp[C_W]);

        return;
    }

    /*
     * Complete projection
     */
    PROJECT_VERTEX_WRITE_Q(m0, m0->comp[C_X], m0->comp[C_Y], m0->comp[C_Z], m0->comp[C_W]);
    PROJECT_VERTEX_WRITE_Q(m1, m1->comp[C_X], m1->comp[C_Y], m1->comp[C_Z], m1->comp[C_W]);
    PROJECT_VERTEX_WRITE_Q(m2, m2->comp[C_X], m2->comp[C_Y], m2->comp[C_Z], m2->comp[C_W]);
}
