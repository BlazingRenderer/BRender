#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(state_stack, f)

static br_tv_template_entry template_entries[] = {
    {BRT(TYPE_T),  F(cull.type),  Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_CC},
    {BRT(SPACE_T), F(cull.space), Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_CC},
};

static const state_cull default_state = {
    .type  = BRT_NONE,
    .space = BRT_MODEL,
};

void StateGLInitCull(state_all *state)
{
    state->templates.cull = BrTVTemplateAllocate(state->res, template_entries, BR_ASIZE(template_entries));

    state->default_.cull = default_state;
    state->default_.valid |= MASK_STATE_CULL;
}
