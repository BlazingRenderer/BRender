#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK, f)

static const br_tv_template_entry GLSTATEI_OutputTemplateEntries[] = {
    {BRT_COLOUR_BUFFER_O, NULL, F(output.colour), Q | S | A, BRTV_CONV_COPY},
    {BRT_DEPTH_BUFFER_O,  NULL, F(output.depth),  Q | S | A, BRTV_CONV_COPY},
};

static const GLSTATE_OUTPUT s_Default = {
    .colour = NULL,
    .depth  = NULL,
};

void GLSTATEI_InitOutput(HGLSTATE hState)
{
    hState->templates.output = BrTVTemplateAllocate(hState->resourceAnchor, GLSTATEI_OutputTemplateEntries,
                                                    BR_ASIZE(GLSTATEI_OutputTemplateEntries));

    hState->default_.output = s_Default;
    hState->default_.valid |= GLSTATE_MASK_OUTPUT;
}
