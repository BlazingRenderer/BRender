#include "drv_ip.hpp"
#include "glstate_ip.hpp"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define AX 0
#define AF BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_CullTemplateEntries[] = {
	{BRT(TYPE_T),			F(cull.type),			Q | S | A,	BRTV_CONV_COPY,				0,TM_PART | TM_INVALID_CC},
	{BRT(SPACE_T),			F(cull.space),			Q | S | A,	BRTV_CONV_COPY,				0,TM_PART | TM_INVALID_CC},
};

static const GLSTATE_CULL s_Default = {
	BRT_NONE,
	BRT_MODEL,
};

void GLSTATEI_InitCull(HGLSTATE hState)
{
	hState->templates.cull = BrTVTemplateAllocate(
		hState->resourceAnchor,
		const_cast<br_tv_template_entry*>(GLSTATEI_CullTemplateEntries),
		BR_ASIZE(GLSTATEI_CullTemplateEntries)
	);

	hState->default.cull = s_Default;
	hState->default.valid |= GLSTATE_MASK_CULL;
}