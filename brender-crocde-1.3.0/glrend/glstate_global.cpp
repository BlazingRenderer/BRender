#include "drv_ip.hpp"
#include "glstate_ip.hpp"
#include "glstate.hpp"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_GlobalTemplateEntries[] = {
	{BRT(GLOBAL_AMBIENT_RGB), F(global.ambient_colour), Q | S | A, BRTV_CONV_COPY, 0, TM_PART},
};


static const GLSTATE_GLOBAL s_Default = {
	.ambient_colour = BR_COLOUR_RGB(0, 0, 0),
};

void GLSTATEI_InitGlobal(HGLSTATE hState)
{
	hState->templates.global = BrTVTemplateAllocate(
		hState->resourceAnchor,
		const_cast<br_tv_template_entry*>(GLSTATEI_GlobalTemplateEntries),
		BR_ASIZE(GLSTATEI_GlobalTemplateEntries)
	);

	hState->default_.global = s_Default;
	hState->default_.valid |= GLSTATE_MASK_GLOBAL;
}
