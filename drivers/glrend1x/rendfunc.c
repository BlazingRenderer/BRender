/*
 * Triangle/line/point render functions using GL 1.1 fixed-function
 */
#include "drv.h"
#include "priminfo.h"

static const GladGLContext      *gl;
static const br_primitive_state *st;

#define EMIT_FOG(v)                                                                            \
    do {                                                                                       \
        if(gl->EXT_fog_coord && (st->prim.flags & PRIMF_FOG) && st->prim.fog_type != BRT_NONE) \
            gl->FogCoordfEXT((v)->comp_f[C_W]);                                                \
    } while(0)

#define VERTEX_ALPHA(v) ((st->prim.flags & PRIMF_BLEND) ? ((v)->comp_f[C_A] > 0.001f ? (v)->comp_f[C_A] : 1.0f) : 1.0f)

void BR_ASM_CALL TriangleRenderGL1x(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2)
{
    (void)block;

    EMIT_FOG(v0);
    gl->TexCoord2f(v0->comp_f[C_U], v0->comp_f[C_V]);
    gl->Color4f(v0->comp_f[C_R], v0->comp_f[C_G], v0->comp_f[C_B], VERTEX_ALPHA(v0));
    gl->Vertex3f(v0->comp_f[C_SX], v0->comp_f[C_SY], v0->comp_f[C_SZ]);

    EMIT_FOG(v1);
    gl->TexCoord2f(v1->comp_f[C_U], v1->comp_f[C_V]);
    gl->Color4f(v1->comp_f[C_R], v1->comp_f[C_G], v1->comp_f[C_B], VERTEX_ALPHA(v1));
    gl->Vertex3f(v1->comp_f[C_SX], v1->comp_f[C_SY], v1->comp_f[C_SZ]);

    EMIT_FOG(v2);
    gl->TexCoord2f(v2->comp_f[C_U], v2->comp_f[C_V]);
    gl->Color4f(v2->comp_f[C_R], v2->comp_f[C_G], v2->comp_f[C_B], VERTEX_ALPHA(v2));
    gl->Vertex3f(v2->comp_f[C_SX], v2->comp_f[C_SY], v2->comp_f[C_SZ]);
}

void BR_ASM_CALL LineRenderGL1x(brp_block *block, brp_vertex *v0, brp_vertex *v1)
{
    (void)block;

    EMIT_FOG(v0);
    gl->TexCoord2f(v0->comp_f[C_U], v0->comp_f[C_V]);
    gl->Color4f(v0->comp_f[C_R], v0->comp_f[C_G], v0->comp_f[C_B], VERTEX_ALPHA(v0));
    gl->Vertex3f(v0->comp_f[C_SX], v0->comp_f[C_SY], v0->comp_f[C_SZ]);

    EMIT_FOG(v1);
    gl->TexCoord2f(v1->comp_f[C_U], v1->comp_f[C_V]);
    gl->Color4f(v1->comp_f[C_R], v1->comp_f[C_G], v1->comp_f[C_B], VERTEX_ALPHA(v1));
    gl->Vertex3f(v1->comp_f[C_SX], v1->comp_f[C_SY], v1->comp_f[C_SZ]);
}

void BR_ASM_CALL PointRenderGL1x(brp_block *block, brp_vertex *v0)
{
    (void)block;

    EMIT_FOG(v0);
    gl->TexCoord2f(v0->comp_f[C_U], v0->comp_f[C_V]);
    gl->Color4f(v0->comp_f[C_R], v0->comp_f[C_G], v0->comp_f[C_B], VERTEX_ALPHA(v0));
    gl->Vertex3f(v0->comp_f[C_SX], v0->comp_f[C_SY], v0->comp_f[C_SZ]);
}

void SetRendfuncGL1xContext(const GladGLContext *g, const br_primitive_state *state)
{
    gl = g;
    st = state;
}
