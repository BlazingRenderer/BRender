/*
 * Triangle/line/point render functions using GL 1.1 fixed-function
 */
#include "drv.h"
#include "priminfo.h"

static const GladGLContext      *gl;
static const br_primitive_state *st;

static void EmitVertex(const brp_vertex *v)
{
    const float a = ((st->prim.flags & PRIMF_BLEND) ? (v->comp_f[C_A] > 0.001f ? v->comp_f[C_A] : 1.0f) : 1.0f);

    if(gl->EXT_fog_coord && (st->prim.flags & PRIMF_FOG) && st->prim.fog_type != BRT_NONE)
        gl->FogCoordfEXT(v->comp_f[C_W]);

    gl->TexCoord2f(v->comp_f[C_U], v->comp_f[C_V]);
    gl->Color4f(v->comp_f[C_R], v->comp_f[C_G], v->comp_f[C_B], a);
    gl->Vertex4f(v->comp_f[C_SX], v->comp_f[C_SY], v->comp_f[C_SZ], -v->comp_f[C_SW]);
}

void BR_ASM_CALL TriangleRenderGL1x(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2)
{
    (void)block;

    EmitVertex(v0);
    EmitVertex(v1);
    EmitVertex(v2);
}

void BR_ASM_CALL LineRenderGL1x(brp_block *block, brp_vertex *v0, brp_vertex *v1)
{
    (void)block;

    EmitVertex(v0);
    EmitVertex(v1);
}

void BR_ASM_CALL PointRenderGL1x(brp_block *block, brp_vertex *v0)
{
    (void)block;

    EmitVertex(v0);
}

void SetRendfuncGL1xContext(const GladGLContext *g, const br_primitive_state *state)
{
    gl = g;
    st = state;
}
