#include <string.h>
#include "drv.h"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define AX 0
#define AF BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_LightTemplateEntries[] = {
    {BRT_TYPE_T,          NULL, F(light[0].type),           Q | S | A,  BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_SPACE_T,         NULL, F(light[0].lighting_space), Q | S | A,  BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_POSITION_V3_X,   NULL, F(light[0].position),       Q | S | AX, BRTV_CONV_V3_FIXED_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_POSITION_V3_F,   NULL, F(light[0].position),       Q | S | AF, BRTV_CONV_V3_FLOAT_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_DIRECTION_V3_X,  NULL, F(light[0].direction),      Q | S | AX, BRTV_CONV_V3_FIXED_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_DIRECTION_V3_F,  NULL, F(light[0].direction),      Q | S | AF, BRTV_CONV_V3_FLOAT_SCALAR, 0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_SPOT_INNER_X,    NULL, F(light[0].spot_inner),     Q | S | AX, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_SPOT_INNER_F,    NULL, F(light[0].spot_inner),     Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_SPOT_OUTER_X,    NULL, F(light[0].spot_outer),     Q | S | AX, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_SPOT_OUTER_F,    NULL, F(light[0].spot_outer),     Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT_COLOUR_RGB,      NULL, F(light[0].colour),         Q | S | AF, BRTV_CONV_COPY,            0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},

    {BRT_ATTENUATION_C_F, NULL, F(light[0].attenuation_c),  Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_ATTENUATION_C_X, NULL, F(light[0].attenuation_c),  Q | S | AF, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_ATTENUATION_L_F, NULL, F(light[0].attenuation_l),  Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_ATTENUATION_L_X, NULL, F(light[0].attenuation_l),  Q | S | AF, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_ATTENUATION_Q_F, NULL, F(light[0].attenuation_q),  Q | S | AF, BRTV_CONV_FLOAT_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
    {BRT_ATTENUATION_Q_X, NULL, F(light[0].attenuation_q),  Q | S | AF, BRTV_CONV_FIXED_SCALAR,    0, TM_PART | TM_INDEX | TM_INVALID_PS | TM_INVALID_PM},
};

static const GLSTATE_LIGHT s_Default = {
	.type			= BRT_NONE,
	.lighting_space	= BRT_MODEL,
	.position		= BR_VECTOR3(0,0,0),
	.direction		= BR_VECTOR3(0,0,0),
	.colour			= BR_COLOUR_RGB(255, 255, 255),
	.spot_outer		= BR_ANGLE_DEG(30.0),
	.spot_inner		= BR_ANGLE_DEG(20.0)
};

static br_tv_template_entry s_LightStates[GLSTATE_MAX_LIGHTS][BR_ASIZE(GLSTATEI_LightTemplateEntries)];

void GLSTATEI_InitLight(HGLSTATE hState)
{

	/* Create a different template list for each light. */
	for(int i = 0; i < GLSTATE_MAX_LIGHTS; ++i)
	{
		memcpy(&s_LightStates[i], GLSTATEI_LightTemplateEntries, sizeof(GLSTATEI_LightTemplateEntries));

		for(int j = 0; j < BR_ASIZE(GLSTATEI_LightTemplateEntries); ++j)
			s_LightStates[i][j].offset += sizeof(GLSTATE_LIGHT) * i;

		hState->templates.light[i] = BrTVTemplateAllocate(
			hState->resourceAnchor,
			s_LightStates[i],
			BR_ASIZE(s_LightStates[i])
		);
		hState->default_.light[i] = s_Default;
	}

	hState->default_.valid |= GLSTATE_MASK_LIGHT;
}
