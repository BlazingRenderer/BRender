#include "drv_ip.hpp"
#include "glstate_ip.hpp"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define AX 0
#define AF BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_OutputTemplateEntries[] = {
	{BRT(COLOUR_BUFFER_O),	F(output.colour),			Q | S | A,		BRTV_CONV_COPY},
	{BRT(DEPTH_BUFFER_O),	F(output.depth), 			Q | S | A,		BRTV_CONV_COPY},
};

static const GLSTATE_OUTPUT s_Default = {
	.colour	= nullptr,
	.depth	= nullptr
};

void GLSTATEI_InitOutput(HGLSTATE hState)
{
	hState->templates.output = BrTVTemplateAllocate(hState->resourceAnchor, const_cast<br_tv_template_entry*>(GLSTATEI_OutputTemplateEntries), BR_ASIZE(GLSTATEI_OutputTemplateEntries));
	hState->default_.output = s_Default;
	hState->default_.valid |= GLSTATE_MASK_OUTPUT;
}