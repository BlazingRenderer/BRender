#include "drv.h"

/*
 * Shortcuts for template flags
 */
#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK, f)

// clang-format off
static br_tv_template_entry GLSTATEI_MatrixTemplateEntries[] = {
    {BRT_MODEL_TO_VIEW_M34_X,        NULL, F(matrix.model_to_view),            Q | S | AX, BRTV_CONV_M34_FIXED_SCALAR, 0, TM_PART | TM_M2V | TM_CLEAR_M2V_HINT | TM_INVALID_PM | TM_INVALID_V2M | TM_INVALID_M2S},
    {BRT_MODEL_TO_VIEW_M34_F,        NULL, F(matrix.model_to_view),            Q | S | AF, BRTV_CONV_M34_FLOAT_SCALAR, 0, TM_PART | TM_M2V | TM_CLEAR_M2V_HINT | TM_INVALID_PM | TM_INVALID_V2M | TM_INVALID_M2S},
    {BRT_VIEW_TO_ENVIRONMENT_M34_X,  NULL, F(matrix.view_to_environment),      Q | S | AX, BRTV_CONV_M34_FIXED_SCALAR, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_VIEW_TO_ENVIRONMENT_M34_F,  NULL, F(matrix.view_to_environment),      Q | S | AF, BRTV_CONV_M34_FLOAT_SCALAR, 0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_VIEW_TO_SCREEN_M4_X,        NULL, F(matrix.view_to_screen),           Q | S | AX, BRTV_CONV_M4_FIXED_SCALAR,  0, TM_PART | TM_V2S | TM_CLEAR_V2S_HINT | TM_V2S_HINT | TM_INVALID_PS | TM_INVALID_PM | TM_INVALID_M2S},
    {BRT_VIEW_TO_SCREEN_M4_F,        NULL, F(matrix.view_to_screen),           Q | S | AF, BRTV_CONV_M4_FLOAT_SCALAR,  0, TM_PART | TM_V2S | TM_CLEAR_V2S_HINT | TM_V2S_HINT | TM_INVALID_PS | TM_INVALID_PM | TM_INVALID_M2S},
    {BRT_MODEL_TO_VIEW_HINT_T,       NULL, F(matrix.model_to_view_hint),       Q | S | A, BRTV_CONV_COPY,              0, TM_PART | TM_INVALID_PM | TM_INVALID_V2M},
    {BRT_VIEW_TO_SCREEN_HINT_T,      NULL, F(matrix.view_to_screen_hint),      Q | S | A, BRTV_CONV_COPY,              0, TM_PART | TM_V2S_HINT | TM_INVALID_PS | TM_INVALID_PM | TM_INVALID_M2S},
    {BRT_VIEW_TO_ENVIRONMENT_HINT_T, NULL, F(matrix.view_to_environment_hint), Q | S | A, BRTV_CONV_COPY,              0, TM_PART | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_HITHER_Z_X,                 NULL, F(matrix.hither_z),                 Q | S | A,  BRTV_CONV_FIXED_SCALAR,     0, TM_PART},
    {BRT_HITHER_Z_F,                 NULL, F(matrix.hither_z),                 Q | S | A,  BRTV_CONV_FLOAT_SCALAR,     0, TM_PART},
    {BRT_YON_Z_X,                    NULL, F(matrix.yon_z),                    Q | S | A,  BRTV_CONV_FIXED_SCALAR,     0, TM_PART},
    {BRT_YON_Z_F,                    NULL, F(matrix.yon_z),                    Q | S | A,  BRTV_CONV_FLOAT_SCALAR,     0, TM_PART},
};
// clang-format on
#undef F

static GLSTATE_MATRIX s_Default = {
    .model_to_view  = {{BR_VECTOR3(1, 0, 0), BR_VECTOR3(0, 1, 0), BR_VECTOR3(0, 0, 1), BR_VECTOR3(0, 0, 0)}},
    .view_to_screen = {{BR_VECTOR4(1, 0, 0, 0), BR_VECTOR4(0, 1, 0, 0), BR_VECTOR4(0, 0, 1, 0), BR_VECTOR4(0, 0, 0, 1)}},
    .view_to_environment = {{BR_VECTOR3(1, 0, 0), BR_VECTOR3(0, 1, 0), BR_VECTOR3(0, 0, 1), BR_VECTOR3(0, 0, 0)}},
    .model_to_view_hint  = BRT_LENGTH_PRESERVING,
    .view_to_screen_hint = BRT_PARALLEL,
};

void GLSTATEI_InitMatrix(HGLSTATE hState)
{
    hState->templates.matrix = BrTVTemplateAllocate(hState->resourceAnchor, GLSTATEI_MatrixTemplateEntries,
                                                    BR_ASIZE(GLSTATEI_MatrixTemplateEntries));

    hState->default_.matrix = s_Default;
    hState->default_.valid |= GLSTATE_MASK_MATRIX;
}
