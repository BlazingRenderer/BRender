#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(state_stack, f)

static br_tv_template_entry template_entries[] = {
    {BRT(TYPE_T),              F(hidden.type),        Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},
    {BRT(DIVERT_T),            F(hidden.divert),      Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(V1ORDER_TABLE_P),     F(hidden.order_table), Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
    {BRT(V1PRIMITIVE_HEAP_P),  F(hidden.heap),        Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},

    {BRT(V1INSERT_FUNCTION_P), F(hidden.insert_fn),   Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
    {BRT(V1INSERT_ARG1_P),     F(hidden.insert_arg1), Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
    {BRT(V1INSERT_ARG2_P),     F(hidden.insert_arg2), Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
    {BRT(V1INSERT_ARG3_P),     F(hidden.insert_arg3), Q | S | A, BRTV_CONV_COPY, 0, TM_PART                                },
};

static const state_hidden default_state = {
    .type        = BRT_NONE,
    .divert      = BRT_NONE,
    .order_table = NULL,
    .heap        = NULL,
    .insert_fn   = NULL,
    .insert_arg1 = NULL,
    .insert_arg2 = NULL,
    .insert_arg3 = NULL,
};

void StateGLInitHidden(state_all *state)
{
    state->templates.hidden = BrTVTemplateAllocate(state->res, template_entries, BR_ASIZE(template_entries));

    state->default_.hidden = default_state;
    /* state->default_.valid |= GLSTATE_MASK_HIDDEN; */
}
