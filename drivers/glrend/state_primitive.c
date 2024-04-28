#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(state_stack, f)

static br_tv_template_entry template_entries[] = {
    {BRT_DEPTH_WRITE_B,           NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_DEPTH_WRITE,  1},
    {BRT_COLOUR_WRITE_B,          NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_COLOUR_WRITE, 1},
    {BRT_BLEND_B,                 NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_BLEND,        1},
    {BRT_MODULATE_B,              NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_MODULATE,     1},
    {BRT_COLOUR_KEY_B,            NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,        PRIMF_COLOUR_KEY,   1},

    {BRT_COLOUR_T,                NULL, F(prim.colour_type),         Q | S | A, BRTV_CONV_COPY,       0,                  1},

    {BRT_DEPTH_TEST_T,            NULL, F(prim.depth_test),          Q | S | A, BRTV_CONV_COPY,       0,                  1},
    {BRT_BLEND_MODE_T,            NULL, F(prim.blend_mode),          Q | S | A, BRTV_CONV_COPY,       0,                  1},

    {BRT_INDEX_BASE_I32,          NULL, F(prim.index_base),          Q | S | A, BRTV_CONV_COPY,       0,                  1},
    {BRT_INDEX_RANGE_I32,         NULL, F(prim.index_range),         Q | S | A, BRTV_CONV_COPY,       0,                  1},

    {BRT_PERSPECTIVE_T,           NULL, F(prim.perspective_type),    Q | S | A, BRTV_CONV_COPY,       0,                  1},
    {BRT_PERSPECTIVE_B,           NULL, F(prim.perspective_type),    S,         BRTV_CONV_BOOL_TOKEN, BRT_DEFAULT,        1},

    {BRT_SUBDIVIDE_TOLERANCE_I32, NULL, F(prim.subdivide_tolerance), Q | S | A, BRTV_CONV_COPY,       0,                  1},

    {BRT_COLOUR_MAP_O,            NULL, F(prim.colour_map),          Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT_TEXTURE_O,               NULL, F(prim.colour_map),          Q | S,     BRTV_CONV_COPY,       0,                  0},
    {BRT_INDEX_SHADE_O,           NULL, F(prim.index_shade),         Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT_INDEX_BLEND_O,           NULL, F(prim.index_blend),         Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT_SCREEN_DOOR_O,           NULL, F(prim.screendoor),          Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT_INDEX_LIGHT_O,           NULL, F(prim.lighting),            Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT_BUMP_O,                  NULL, F(prim.bump),                Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT_INDEX_FOG_O,             NULL, F(prim.index_fog),           Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT_MAP_INTERPOLATION_T,     NULL, F(prim.filter),              Q | S | A, BRTV_CONV_COPY,       0,                  0},
    {BRT_MIP_INTERPOLATION_T,     NULL, F(prim.mip_filter),          Q | S | A, BRTV_CONV_COPY,       0,                  0},
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
