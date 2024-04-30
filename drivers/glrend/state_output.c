#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(state_stack, f)

static br_tv_template_entry template_entries[] = {
    {BRT(COLOUR_BUFFER_O), F(output.colour), Q | S | A, BRTV_CONV_COPY},
    {BRT(DEPTH_BUFFER_O),  F(output.depth),  Q | S | A, BRTV_CONV_COPY},
};

static const state_output default_state = {
    .colour = NULL,
    .depth  = NULL,
};

void StateGLInitOutput(state_all *state)
{
    state->templates.output = BrTVTemplateAllocate(state->res, template_entries, BR_ASIZE(template_entries));

    state->default_.output = default_state;
    state->default_.valid |= MASK_STATE_OUTPUT;
}
