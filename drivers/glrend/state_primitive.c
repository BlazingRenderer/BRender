#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(state_stack, f)

static br_tv_template_entry template_entries[] = {
    {BRT(DEPTH_WRITE_B),           F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_DEPTH_WRITE,  1},
    {BRT(COLOUR_WRITE_B),          F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_COLOUR_WRITE, 1},
    {BRT(BLEND_B),                 F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_BLEND,        1},
    {BRT(MODULATE_B),              F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_MODULATE,     1},
    {BRT(COLOUR_KEY_B),            F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_COLOUR_KEY,   1},

    {BRT(COLOUR_T),                F(prim.colour_type),         Q | S | A, BRTV_CONV_COPY,       0,                  1},

    {BRT(DEPTH_TEST_T),            F(prim.depth_test),          Q | S | A, BRTV_CONV_COPY,       0,                  1},
    {BRT(BLEND_MODE_T),            F(prim.blend_mode),          Q | S | A, BRTV_CONV_COPY,       0,                  1},

    {BRT(INDEX_BASE_I32),          F(prim.index_base),          Q | S | A, BRTV_CONV_COPY,       0,                  1},
    {BRT(INDEX_RANGE_I32),         F(prim.index_range),         Q | S | A, BRTV_CONV_COPY,       0,                  1},

    {BRT(PERSPECTIVE_T),           F(prim.perspective_type),    Q | S | A, BRTV_CONV_COPY,       0,                  1},
    {BRT(PERSPECTIVE_B),           F(prim.perspective_type),    S,         BRTV_CONV_BOOL_TOKEN, BRT_DEFAULT,        1},

    {BRT(SUBDIVIDE_TOLERANCE_I32), F(prim.subdivide_tolerance), Q | S | A, BRTV_CONV_COPY,       0,                  1},

    {BRT(COLOUR_MAP_O),            F(prim.colour_map),          Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT(TEXTURE_O),               F(prim.colour_map),          Q | S,     BRTV_CONV_COPY,       0,                  0},
    {BRT(INDEX_SHADE_O),           F(prim.index_shade),         Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT(INDEX_BLEND_O),           F(prim.index_blend),         Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT(SCREEN_DOOR_O),           F(prim.screendoor),          Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT(INDEX_LIGHT_O),           F(prim.lighting),            Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT(BUMP_O),                  F(prim.bump),                Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT(INDEX_FOG_O),             F(prim.index_fog),           Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT(MAP_INTERPOLATION_T),     F(prim.filter),              Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT(MIP_INTERPOLATION_T),     F(prim.mip_filter),          Q | S | A, BRTV_CONV_COPY,       0,                  0},
};

static const state_primitive default_state = {
    .flags               = PRIMF_COLOUR_WRITE | PRIMF_DEPTH_WRITE,
    .index_base          = 0,
    .index_range         = 0,
    .colour_type         = BRT_DEFAULT,
    .depth_test          = BRT_LESS,
    .blend_mode          = BRT_NONE,
    .perspective_type    = BRT_NONE, // BRT_NONE, BRT_SUBDIVIDE
    .subdivide_tolerance = 0,
    .colour_map          = NULL,
    .index_shade         = NULL,
    .index_blend         = NULL,
    .index_fog           = NULL,
    .screendoor          = NULL,
    .lighting            = NULL,
    .bump                = NULL,
    .filter              = BRT_NONE,
    .mip_filter          = BRT_NONE,
};

void StateGLInitPrimitive(state_all *state)
{
    state->templates.prim = BrTVTemplateAllocate(state->res, template_entries, BR_ASIZE(template_entries));

    state->default_.prim = default_state;
    state->default_.valid |= MASK_STATE_PRIMITIVE;
}
