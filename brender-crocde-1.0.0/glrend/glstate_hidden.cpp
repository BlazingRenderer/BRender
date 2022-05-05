#include "drv_ip.hpp"
#include "glstate_ip.hpp"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define AX 0
#define AF BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_HiddenTemplateEntries[] = {
	{BRT(TYPE_T),				F(hidden.type), 	  	Q | S | A, 	BRTV_CONV_COPY,	0,TM_PART | TM_INVALID_PS | TM_INVALID_PM},

	{BRT(V1ORDER_TABLE_P),		F(hidden.order_table),	Q | S | A, 	BRTV_CONV_COPY,	0,TM_PART},
	{BRT(V1PRIMITIVE_HEAP_P),	F(hidden.heap), 		Q | S | A, 	BRTV_CONV_COPY,	0,TM_PART | TM_INVALID_PS | TM_INVALID_PM},

	{BRT(V1INSERT_FUNCTION_P),	F(hidden.insert_fn),	Q | S | A, 	BRTV_CONV_COPY,	0,TM_PART},
	{BRT(V1INSERT_ARG1_P),		F(hidden.insert_arg1),	Q | S | A, 	BRTV_CONV_COPY,	0,TM_PART},
	{BRT(V1INSERT_ARG2_P),		F(hidden.insert_arg2),	Q | S | A, 	BRTV_CONV_COPY,	0,TM_PART},
	{BRT(V1INSERT_ARG3_P),		F(hidden.insert_arg3),	Q | S | A, 	BRTV_CONV_COPY,	0,TM_PART},
};

static const GLSTATE_HIDDEN s_Default = {
	BRT_NONE,		/* type			*/
	nullptr,		/* order_table	*/
	nullptr,		/* heap			*/
	nullptr,		/* insert_fn	*/
	nullptr,		/* insert_arg1	*/
	nullptr,		/* insert_arg2	*/
	nullptr,		/* insert_arg3	*/
};

void GLSTATEI_InitHidden(HGLSTATE hState)
{
	hState->templates.hidden = BrTVTemplateAllocate(
		hState->resourceAnchor,
		const_cast<br_tv_template_entry*>(GLSTATEI_HiddenTemplateEntries),
		BR_ASIZE(GLSTATEI_HiddenTemplateEntries)
	);

	hState->default.hidden = s_Default;
	//hState->default.valid |= GLSTATE_MASK_HIDDEN;
}