#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK, f)

static br_tv_template_entry GLSTATEI_ClipTemplateEntries[] = {
    {BRT(TYPE_T),     F(clip[0].type),  Q | S | A,  BRTV_CONV_COPY,            0, TM_PART | TM_INVALID_PS},
    {BRT(PLANE_V4_X), F(clip[0].plane), Q | S | AX, BRTV_CONV_V4_FIXED_SCALAR, 0, TM_PART | TM_INVALID_PS},
    {BRT(PLANE_V4_F), F(clip[0].plane), Q | S | AF, BRTV_CONV_V4_FLOAT_SCALAR, 0, TM_PART | TM_INVALID_PS},
};

static const GLSTATE_CLIP s_Default = {
    .type  = BRT_NONE,
    .plane = BR_VECTOR4(0, 0, -1, 0),
};

static br_tv_template_entry s_ClipStates[GLSTATE_MAX_CLIP_PLANES][BR_ASIZE(GLSTATEI_ClipTemplateEntries)];

void GLSTATEI_InitClip(HGLSTATE hState)
{
    /* Create a different template list for each clipping plane. */
    for(int i = 0; i < GLSTATE_MAX_CLIP_PLANES; ++i) {
        BrMemCpy(&s_ClipStates[i], GLSTATEI_ClipTemplateEntries, sizeof(GLSTATEI_ClipTemplateEntries));

        for(int j = 0; j < BR_ASIZE(GLSTATEI_ClipTemplateEntries); ++j)
            s_ClipStates[i][j].offset += sizeof(GLSTATE_CLIP) * i;

        hState->templates.clip[i] = BrTVTemplateAllocate(hState->resourceAnchor, s_ClipStates[i], BR_ASIZE(s_ClipStates[i]));
        hState->default_.clip[i] = s_Default;
    }

    hState->default_.valid |= GLSTATE_MASK_CLIP;
}
