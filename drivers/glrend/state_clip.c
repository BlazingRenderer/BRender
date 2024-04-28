#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(state_stack, f)

static br_tv_template_entry template_entries[] = {
    {BRT(TYPE_T),     F(clip[0].type),  Q | S | A,  BRTV_CONV_COPY,            0, TM_PART | TM_INVALID_PS},
    {BRT(PLANE_V4_X), F(clip[0].plane), Q | S | AX, BRTV_CONV_V4_FIXED_SCALAR, 0, TM_PART | TM_INVALID_PS},
    {BRT(PLANE_V4_F), F(clip[0].plane), Q | S | AF, BRTV_CONV_V4_FLOAT_SCALAR, 0, TM_PART | TM_INVALID_PS},
};

static const state_clip default_state = {
    .type  = BRT_NONE,
    .plane = BR_VECTOR4(0, 0, -1, 0),
};

static br_tv_template_entry clip_states[MAX_STATE_CLIP_PLANES][BR_ASIZE(template_entries)];

void StateGLInitClip(state_all *state)
{
    /* Create a different template list for each clipping plane. */
    for(int i = 0; i < MAX_STATE_CLIP_PLANES; ++i) {
        BrMemCpy(&clip_states[i], template_entries, sizeof(template_entries));

        for(int j = 0; j < BR_ASIZE(template_entries); ++j)
            clip_states[i][j].offset += sizeof(state_clip) * i;

        state->templates.clip[i] = BrTVTemplateAllocate(state->res, clip_states[i], BR_ASIZE(clip_states[i]));
        state->default_.clip[i]  = default_state;
    }

    state->default_.valid |= MASK_STATE_CLIP;
}
