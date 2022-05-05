#include "drv_ip.hpp"
#include "glstate_ip.hpp"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define AX 0
#define AF BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_SurfaceTemplateEntries[] = {
	{BRT(COLOUR_RGB),		F(surface.colour),			Q | S | AX,	BRTV_CONV_COPY,				0,TM_PART},

	{BRT(OPACITY_X),		F(surface.opacity),			Q | S | AX,	BRTV_CONV_FIXED_SCALAR,		0,TM_PART},
	{BRT(OPACITY_F),		F(surface.opacity),			Q | S | AF,	BRTV_CONV_FLOAT_SCALAR,		0,TM_PART},
	{BRT(AMBIENT_X),		F(surface.ka),				Q | S | AX,	BRTV_CONV_FIXED_SCALAR,		0,TM_PART},
	{BRT(AMBIENT_F),		F(surface.ka),				Q | S | AF,	BRTV_CONV_FLOAT_SCALAR,		0,TM_PART},
	{BRT(DIFFUSE_X),		F(surface.kd),				Q | S | AX,	BRTV_CONV_FIXED_SCALAR,		0,TM_PART},
	{BRT(DIFFUSE_F),		F(surface.kd),				Q | S | AF,	BRTV_CONV_FLOAT_SCALAR,		0,TM_PART},
	{BRT(SPECULAR_X),		F(surface.ks),				Q | S | AX,	BRTV_CONV_FIXED_SCALAR,		0,TM_PART},
	{BRT(SPECULAR_F),		F(surface.ks),				Q | S | AF,	BRTV_CONV_FLOAT_SCALAR,		0,TM_PART},
	{BRT(SPECULAR_POWER_X),	F(surface.power),			Q | S | AX,	BRTV_CONV_FIXED_SCALAR,		0,TM_PART},
	{BRT(SPECULAR_POWER_F),	F(surface.power),			Q | S | AF,	BRTV_CONV_FLOAT_SCALAR,		0,TM_PART},
	{BRT(AMBIENT_R_X),		F(surface.ambient_r),		Q | S | AX,	BRTV_CONV_FIXED_SCALAR,		0,TM_PART},
	{BRT(AMBIENT_R_F),		F(surface.ambient_r),		Q | S | AF,	BRTV_CONV_FLOAT_SCALAR,		0,TM_PART},
	{BRT(AMBIENT_G_X),		F(surface.ambient_g),		Q | S | AX,	BRTV_CONV_FIXED_SCALAR,		0,TM_PART},
	{BRT(AMBIENT_G_F),		F(surface.ambient_g),		Q | S | AF,	BRTV_CONV_FLOAT_SCALAR,		0,TM_PART},
	{BRT(AMBIENT_B_X),		F(surface.ambient_b),		Q | S | AX,	BRTV_CONV_FIXED_SCALAR,		0,TM_PART},
	{BRT(AMBIENT_B_F),		F(surface.ambient_b),		Q | S | AF,	BRTV_CONV_FLOAT_SCALAR,		0,TM_PART},

	{BRT(LIGHTING_B),		F(surface.lighting),		Q | S | A,	BRTV_CONV_COPY,				0,TM_PART | TM_INVALID_CC},
	{BRT(FORCE_FRONT_B),	F(surface.force_front),		Q | S | A,	BRTV_CONV_COPY,				0,TM_PART | TM_INVALID_CC},
	{BRT(FORCE_BACK_B),		F(surface.force_back),		Q | S | A,	BRTV_CONV_COPY,				0,TM_PART | TM_INVALID_CC},

	{BRT(COLOUR_SOURCE_T),	F(surface.colour_source),	Q | S | A,	BRTV_CONV_COPY,				0,TM_PART | TM_INVALID_CC},
	{BRT(MAPPING_SOURCE_T),	F(surface.mapping_source),	Q | S | A,	BRTV_CONV_COPY,				0,TM_PART | TM_INVALID_CC},

	{BRT(MAP_MATRIX_M23_X),	F(surface.map_transform),	Q | S | AX,	BRTV_CONV_M23_FIXED_SCALAR,	0,TM_PART | TM_INVALID_CC},
	{BRT(MAP_MATRIX_M23_F),	F(surface.map_transform),	Q | S | AF,	BRTV_CONV_M23_FLOAT_SCALAR,	0,TM_PART | TM_INVALID_CC},
};

static const GLSTATE_SURFACE s_Default = {
	.colour			= BR_COLOUR_RGB(255,255,255),
	.opacity		= BR_SCALAR(1.0),
	.ka				= BR_SCALAR(0.0),
	.kd				= BR_SCALAR(0.75),
	.ks				= BR_SCALAR(0.0),
	.power			= BR_SCALAR(20.0),
	.ambient_r		= BR_SCALAR(0.0),
	.ambient_g		= BR_SCALAR(0.0),
	.ambient_b		= BR_SCALAR(0.0),

	.lighting		= BR_FALSE,
	.force_front	= BR_FALSE,
	.force_back		= BR_FALSE,
	.colour_source	= BRT_SURFACE,
	.mapping_source	= BRT_GEOMETRY_MAP,

	.map_transform	= {{
		BR_VECTOR2(1,0),
		BR_VECTOR2(0,1),
		BR_VECTOR2(0,0),
	}},
};

void GLSTATEI_InitSurface(HGLSTATE hState)
{
	hState->templates.surface = BrTVTemplateAllocate(
		hState->resourceAnchor,
		const_cast<br_tv_template_entry*>(GLSTATEI_SurfaceTemplateEntries),
		BR_ASIZE(GLSTATEI_SurfaceTemplateEntries)
	);

	hState->default_.surface = s_Default;
	hState->default_.valid |= GLSTATE_MASK_SURFACE;
}
