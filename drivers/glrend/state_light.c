#include <string.h>
#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(state_stack, f)

// clang-format off
static br_tv_template_entry template_entries[] = {
    {BRT(TYPE_T),          F(light[0].type),           Q | S | A,  BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(SPACE_T),         F(light[0].lighting_space), Q | S | A,  BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(POSITION_V3_X),   F(light[0].position),       Q | S | AX, BRTV_CONV_V3_FIXED_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(POSITION_V3_F),   F(light[0].position),       Q | S | AF, BRTV_CONV_V3_FLOAT_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(DIRECTION_V3_X),  F(light[0].direction),      Q | S | AX, BRTV_CONV_V3_FIXED_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(DIRECTION_V3_F),  F(light[0].direction),      Q | S | AF, BRTV_CONV_V3_FLOAT_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(SPOT_INNER_X),    F(light[0].spot_inner),     Q | S | AX, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(SPOT_INNER_F),    F(light[0].spot_inner),     Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(SPOT_OUTER_X),    F(light[0].spot_outer),     Q | S | AX, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(SPOT_OUTER_F),    F(light[0].spot_outer),     Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(COLOUR_RGB),      F(light[0].colour),         Q | S | AF, BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(ATTENUATION_C_F), F(light[0].attenuation_c),  Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_C_X), F(light[0].attenuation_c),  Q | S | AF, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_L_F), F(light[0].attenuation_l),  Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_L_X), F(light[0].attenuation_l),  Q | S | AF, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_Q_F), F(light[0].attenuation_q),  Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(ATTENUATION_Q_X), F(light[0].attenuation_q),  Q | S | AF, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
};
// clang-format on

static const state_light default_state = {
    .type           = BRT_NONE,
    .lighting_space = BRT_MODEL,
    .position       = BR_VECTOR3(0, 0, 0),
    .direction      = BR_VECTOR3(0, 0, 0),
    .colour         = BR_COLOUR_RGB(255, 255, 255),
    .spot_outer     = BR_ANGLE_DEG(30.0),
    .spot_inner     = BR_ANGLE_DEG(20.0),
};

static br_tv_template_entry light_states[MAX_STATE_LIGHTS][BR_ASIZE(template_entries)];

void StateGLInitLight(state_all *state)
{
    /* Create a different template list for each light. */
    for(int i = 0; i < MAX_STATE_LIGHTS; ++i) {
        memcpy(&light_states[i], template_entries, sizeof(template_entries));

        for(int j = 0; j < BR_ASIZE(template_entries); ++j)
            light_states[i][j].offset += sizeof(state_light) * i;

        state->templates.light[i] = BrTVTemplateAllocate(state->res, light_states[i], BR_ASIZE(light_states[i]));
        state->default_.light[i] = default_state;
    }

    state->default_.valid |= MASK_STATE_LIGHT;
}
