/*
 * ASM definitions ported from fpwork.inc.
 */
#ifndef _WORKSPACE_H_
#define _WORKSPACE_H_

#if defined(__H2INC__)
#define alignas(x) /* NB: This is fixed with a regex in mkdrv.pl */
#else
#include <stdalign.h>
#endif

#pragma pack(push, 4)

/*
 * Layout of workspace for floating point Pentium optimized setup for rasterisers
 */
struct _workspace {
    /* qwords start here (pair-of-dword usage) */
    br_uint_32 xm;
    br_uint_32 d_xm;
    br_uint_32 x1;
    br_uint_32 d_x1;
    br_uint_32 x2;
    br_uint_32 d_x2;

    br_uint_32 s_z;
    br_uint_32 d_z_y_1;
    br_uint_32 d_z_x;
    br_uint_32 d_z_y_0;

    br_uint_32 s_i;
    br_uint_32 d_i_y_1;
    br_uint_32 d_i_x;
    br_uint_32 d_i_y_0;

    br_uint_32 s_u;
    br_uint_32 d_u_y_1;
    br_uint_32 d_u_x;
    br_uint_32 d_u_y_0;

    br_uint_32 s_v;
    br_uint_32 d_v_y_1;
    br_uint_32 d_v_x;
    br_uint_32 d_v_y_0;

    br_uint_32 s_s;
    br_uint_32 d_s_y_1;
    br_uint_32 d_s_x;
    br_uint_32 d_s_y_0;

    br_uint_32 scanAddress;
    br_uint_32 scanAddressTrashed;
    br_uint_32 depthAddress;
    br_uint_32 depthAddressTrashed;
    /* qwords end here */

    /* input to setup code */
    brp_vertex *v0;
    brp_vertex *v1;
    brp_vertex *v2;

    /* only used within setup */
    br_uint_32 iarea;
    br_uint_32 dx1_a;
    br_uint_32 dx2_a;
    br_uint_32 dy1_a;
    br_uint_32 dy2_a;
    br_uint_32 xstep_1;
    br_uint_32 xstep_0;
    br_uint_32 t_dx;
    br_uint_32 t_dy;

    /* results from setup */
    br_uint_32 t_y;
    br_uint_32 flip;

    /* used in some rasterisers */
    br_uint_32 c_z;
    br_uint_32 c_u;
    br_uint_32 c_v;
    br_uint_32 c_i;

    /* tsbHeader-ish: counts union */
    union {
        br_uint_32 l;    /* dword */
        br_int_16  s[2]; /* two SWORDs */
    } counts;

    /* Fixed setup only */
    br_uint_32 v0_x;
    br_uint_32 v1_x;
    br_uint_32 v2_x;
    br_uint_32 v0_y;
    br_uint_32 v1_y;
    br_uint_32 v2_y;
    br_uint_32 top_vertex;

    /* my stuff, currently in here for my convenience -JohnG */
    br_uint_32 xm_f;
    br_uint_32 d_xm_f;

    br_uint_32 topCount;
    br_uint_32 bottomCount;

    br_uint_32 colour; /* align 8 */
    br_uint_32 colourExtension;
    br_uint_32 shadeTable;

    br_uint_32 _pad1; /* align 8 */
    br_uint_32 scratch0;
    br_uint_32 scratch1;
    br_uint_32 scratch2;
    br_uint_32 scratch3;
    br_uint_32 scratch4;
    br_uint_32 scratch5;
    br_uint_32 scratch6;
    br_uint_32 scratch7;
    br_uint_32 scratch8;
    br_uint_32 scratch9;
    br_uint_32 scratch10;
    br_uint_32 scratch11;
    br_uint_32 scratch12;
    br_uint_32 scratch13;
    br_uint_32 scratch14;
    br_uint_32 scratch15;
};

struct ArbitraryWidthWorkspace {
    uint32_t su;
    uint32_t _pad0;

    uint32_t dux;
    uint32_t _pad1;

    uint32_t duy1;
    uint32_t _pad2;

    uint32_t duy0;
    uint32_t _pad3;

    uint32_t sv;
    uint32_t _pad4;

    uint32_t dvy1;
    uint32_t _pad5;
    uint32_t dvy1c;
    uint32_t _pad6;

    uint32_t dvy0;
    uint32_t _pad7;
    uint32_t dvy0c;

    uint32_t dvxc;
    uint32_t _pad8;
    uint32_t dvx;
    uint32_t _pad9;

    uint32_t svf;
    uint32_t dvxf;
    uint32_t dvy1f;
    uint32_t dvy0f;

    uint32_t uUpperBound;
    uint32_t vUpperBound;

    uint32_t flags;
    uint32_t retAddress;
};

#pragma pack(pop)

extern BR_ALIGNAS(8) struct _workspace workspace;
extern BR_ALIGNAS(8) struct ArbitraryWidthWorkspace workspaceA;

#endif /* _WORKSPACE_H_ */
