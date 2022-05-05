#include "drv.h"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define AX 0
#define AF BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_CullTemplateEntries[] = {
    {BRT_TYPE_T,  NULL, F(cull.type),  Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_CC},
    {BRT_SPACE_T, NULL, F(cull.space), Q | S | A, BRTV_CONV_COPY, 0, TM_PART | TM_INVALID_CC},
};

static const GLSTATE_CULL s_Default = {
    .type  = BRT_NONE,
    .space = BRT_MODEL
};

void GLSTATEI_InitCull(HGLSTATE hState)
{
    hState->templates.cull = BrTVTemplateAllocate(
        hState->resourceAnchor,
        GLSTATEI_CullTemplateEntries,
        BR_ASIZE(GLSTATEI_CullTemplateEntries)
    );

    hState->default_.cull = s_Default;
    hState->default_.valid |= GLSTATE_MASK_CULL;
}
