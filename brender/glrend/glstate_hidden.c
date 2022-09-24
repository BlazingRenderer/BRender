#include "drv.h"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define AX 0
#define AF BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_HiddenTemplateEntries[] = {
    {BRT_TYPE_T,              NULL, F(hidden.type),        Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_DIVERT_T,            NULL, F(hidden.divert),      Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},

    {BRT_V1ORDER_TABLE_P,     NULL, F(hidden.order_table), Q | S | A, BRTV_CONV_COPY, 0, TM_PART},
    {BRT_V1PRIMITIVE_HEAP_P,  NULL, F(hidden.heap),        Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},

    {BRT_V1INSERT_FUNCTION_P, NULL, F(hidden.insert_fn),   Q | S | A, BRTV_CONV_COPY, 0, TM_PART},
    {BRT_V1INSERT_ARG1_P,     NULL, F(hidden.insert_arg1), Q | S | A, BRTV_CONV_COPY, 0, TM_PART},
    {BRT_V1INSERT_ARG2_P,     NULL, F(hidden.insert_arg2), Q | S | A, BRTV_CONV_COPY, 0, TM_PART},
    {BRT_V1INSERT_ARG3_P,     NULL, F(hidden.insert_arg3), Q | S | A, BRTV_CONV_COPY, 0, TM_PART},
};

static const GLSTATE_HIDDEN s_Default = {
	.type			= BRT_NONE,
	.divert         = BRT_NONE,
	.order_table	= NULL,
	.heap			= NULL,
	.insert_fn		= NULL,
	.insert_arg1	= NULL,
	.insert_arg2	= NULL,
	.insert_arg3	= NULL,
};

void GLSTATEI_InitHidden(HGLSTATE hState)
{
	hState->templates.hidden = BrTVTemplateAllocate(
		hState->resourceAnchor,
		GLSTATEI_HiddenTemplateEntries,
		BR_ASIZE(GLSTATEI_HiddenTemplateEntries)
	);

	hState->default_.hidden = s_Default;
	//hState->default_.valid |= GLSTATE_MASK_HIDDEN;
}
