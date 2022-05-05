#include "drv_ip.hpp"
#include "glstate_ip.hpp"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define AX 0
#define AF BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_PrimitiveTemplateEntries[] = {
	//{BRT(FORCE_FRONT_B),	F(prim.flags),			Q | S | A,	BRTV_CONV_BIT,	PRIMF_FORCE_FRONT,			1},
	//{BRT(SMOOTH_B),			F(prim.flags),			Q | S | A,	BRTV_CONV_BIT,	PRIMF_SMOOTH,				1},
	{BRT(DECAL_B),			F(prim.flags),			Q | S | A,	BRTV_CONV_BIT,	PRIMF_DECAL,				1},
	//{BRT(DITHER_COLOUR_B),	F(prim.flags),			Q | S | A,	BRTV_CONV_BIT,	PRIMF_DITHER_COLOUR,		1},
	//{BRT(DITHER_MAP_B),		F(prim.flags),			Q | S | A,	BRTV_CONV_BIT,	PRIMF_DITHER_MAP,			1},
	{BRT(DEPTH_WRITE_B),	F(prim.flags),			Q | S | A,	BRTV_CONV_BIT,	PRIMF_DEPTH_WRITE,			1},
	{BRT(COLOUR_WRITE_B),	F(prim.flags),			Q | S | A,	BRTV_CONV_BIT,	PRIMF_COLOUR_WRITE,			1},
	{BRT(BLEND_B),			F(prim.flags),			Q | S | A,	BRTV_CONV_BIT,	PRIMF_BLEND,				1},
	{BRT(MODULATE_B),		F(prim.flags),			Q | S | A,	BRTV_CONV_BIT,	PRIMF_MODULATE,				1},
	{BRT(FOG_T),			F(prim.fog_type),		Q | S | A,	BRTV_CONV_COPY},

	{BRT(COLOUR_T),			F(prim.colour_type),	Q | S | A,	BRTV_CONV_COPY,0,							1},

	//{BRT(Z_WRITE_T),		F(prim.z_write),		Q | S | A,	BRTV_CONV_COPY,0,							1},
	{BRT(DEPTH_TEST_T),		F(prim.depth_test),		Q | S | A,	BRTV_CONV_COPY,0,							1},
	//{BRT(BLEND_MODE_T),		F(prim.blend_mode),		Q | S | A,	BRTV_CONV_COPY,0,							1},

	{BRT(INDEX_BASE_I32),	F(prim.index_base),		Q | S | A,	BRTV_CONV_COPY,0,							1},
	{BRT(INDEX_RANGE_I32),	F(prim.index_range),	Q | S | A,	BRTV_CONV_COPY,0,							1},

	{BRT(PERSPECTIVE_T),	F(prim.perspective_type),Q | S | A,	BRTV_CONV_COPY,0,							1},
	{BRT(PERSPECTIVE_B),	F(prim.perspective_type),S,			BRTV_CONV_BOOL_TOKEN,BRT_DEFAULT,			1},

	{BRT(SUBDIVIDE_TOLERANCE_I32),	F(prim.subdivide_tolerance),Q | S | A,	BRTV_CONV_COPY,0,				1},

	{BRT(COLOUR_MAP_O),			F(prim.colour_map),		Q | S | A,	BRTV_CONV_COPY},
	{BRT(TEXTURE_O),			F(prim.colour_map),		Q | S,		BRTV_CONV_COPY},
	{BRT(INDEX_SHADE_O),		F(prim.index_shade),	Q | S | A,	BRTV_CONV_COPY},
	{BRT(INDEX_BLEND_O),		F(prim.index_blend),	Q | S | A,	BRTV_CONV_COPY},
	{BRT(SCREEN_DOOR_O),		F(prim.screendoor),		Q | S | A,	BRTV_CONV_COPY},
	{BRT(INDEX_LIGHT_O),		F(prim.lighting),		Q | S | A,	BRTV_CONV_COPY},
	{BRT(BUMP_O),				F(prim.bump),			Q | S | A,	BRTV_CONV_COPY},
	{BRT(INDEX_FOG_O),			F(prim.index_fog),		Q | S | A,	BRTV_CONV_COPY},
	{BRT(MAP_INTERPOLATION_T),	F(prim.filter),			Q | S | A,	BRTV_CONV_COPY},
};

static const GLSTATE_PRIMITIVE s_Default = {
	PRIMF_COLOUR_WRITE | PRIMF_DEPTH_WRITE,	/* flags */
	0,										/* index_base */
	0,										/* index_range */
	BRT_DEFAULT,							/* colour_type */
	//BRT_Z_WRITE_CONDITIONAL,				/* z_write */
	BRT_LESS,								/* depth_test */
	//BRT_NONE,								/* blend_mode */
	BRT_NONE,								/* perspective_type */ // BRT_NONE, BRT_SUBDIVIDE
	0,										/* subdivide_tolerance */
	nullptr,								/* colour_map */
	nullptr,								/* index_shade */
	nullptr,								/* index_blend */
	nullptr,								/* index_fog */
	nullptr,								/* screendoor */
	nullptr,								/* lighting */
	nullptr,								/* bump */
	BRT_NONE,								/* fog_type */
	BRT_NONE								/* filter */
};

void GLSTATEI_InitPrimitive(HGLSTATE hState)
{
	hState->templates.prim = BrTVTemplateAllocate(
		hState->resourceAnchor,
		const_cast<br_tv_template_entry*>(GLSTATEI_PrimitiveTemplateEntries),
		BR_ASIZE(GLSTATEI_PrimitiveTemplateEntries)
	);

	hState->default.prim = s_Default;
	hState->default.valid |= GLSTATE_MASK_PRIMITIVE;
}