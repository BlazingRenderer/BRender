#include "drv.h"

#define S BRTV_SET
#define Q BRTV_QUERY
#define A BRTV_ALL

#define AX 0
#define AF BRTV_ALL

#define F(f) offsetof(GLSTATE_STACK,f)

static const br_tv_template_entry GLSTATEI_PrimitiveTemplateEntries[] = {
    //{BRT_FORCE_FRONT_B,           NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_FORCE_FRONT,   1},
    //{BRT_SMOOTH_B,                NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_SMOOTH,        1},
    //{BRT_SMOOTH_OPACITY_B,        NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_SMOOTH_ALPHA,  1},
    {BRT_DECAL_B,                 NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_DECAL,         1},
    //{BRT_DITHER_COLOUR_B,         NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_DITHER_COLOUR, 1},
    //{BRT_DITHER_MAP_B,            NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_DITHER_MAP,    1},
    {BRT_DEPTH_WRITE_B,           NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_DEPTH_WRITE,   1},
    {BRT_COLOUR_WRITE_B,          NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_COLOUR_WRITE,  1},
    {BRT_BLEND_B,                 NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_BLEND,         1},
    {BRT_MODULATE_B,              NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_MODULATE,      1},
    {BRT_COLOUR_KEY_B,            NULL, F(prim.flags),               Q | S | A, BRTV_CONV_BIT,  PRIMF_COLOUR_KEY,    1},
    {BRT_FOG_T,                   NULL, F(prim.fog_type),            Q | S | A, BRTV_CONV_COPY},

    {BRT_COLOUR_T,                NULL, F(prim.colour_type),         Q | S | A, BRTV_CONV_COPY, 0,                   1},

    //{BRT_Z_WRITE_T,               NULL, F(prim.z_write),             Q | S | A, BRTV_CONV_COPY, 0,                   1},
    {BRT_DEPTH_TEST_T,            NULL, F(prim.depth_test),          Q | S | A, BRTV_CONV_COPY, 0,                   1},
    {BRT_BLEND_MODE_T,            NULL, F(prim.blend_mode),          Q | S | A, BRTV_CONV_COPY, 0,                   1},

    {BRT_INDEX_BASE_I32,          NULL, F(prim.index_base),          Q | S | A, BRTV_CONV_COPY, 0,                   1},
    {BRT_INDEX_RANGE_I32,         NULL, F(prim.index_range),         Q | S | A, BRTV_CONV_COPY, 0,                   1},

    {BRT_PERSPECTIVE_T,           NULL, F(prim.perspective_type),    Q | S | A, BRTV_CONV_COPY, 0,                   1},
    {BRT_PERSPECTIVE_B,           NULL, F(prim.perspective_type),    S, BRTV_CONV_BOOL_TOKEN, BRT_DEFAULT, 1},

    {BRT_SUBDIVIDE_TOLERANCE_I32, NULL, F(prim.subdivide_tolerance), Q | S | A, BRTV_CONV_COPY, 0,                   1},

    {BRT_COLOUR_MAP_O,            NULL, F(prim.colour_map),          Q | S | A, BRTV_CONV_COPY},
    {BRT_TEXTURE_O,               NULL, F(prim.colour_map),          Q | S,     BRTV_CONV_COPY},
    {BRT_INDEX_SHADE_O,           NULL, F(prim.index_shade),         Q | S | A, BRTV_CONV_COPY},
    {BRT_INDEX_BLEND_O,           NULL, F(prim.index_blend),         Q | S | A, BRTV_CONV_COPY},
    {BRT_SCREEN_DOOR_O,           NULL, F(prim.screendoor),          Q | S | A, BRTV_CONV_COPY},
    {BRT_INDEX_LIGHT_O,           NULL, F(prim.lighting),            Q | S | A, BRTV_CONV_COPY},
    {BRT_BUMP_O,                  NULL, F(prim.bump),                Q | S | A, BRTV_CONV_COPY},
    {BRT_INDEX_FOG_O,             NULL, F(prim.index_fog),           Q | S | A, BRTV_CONV_COPY},
    {BRT_MAP_INTERPOLATION_T,     NULL, F(prim.filter),              Q | S | A, BRTV_CONV_COPY},
    {BRT_MIP_INTERPOLATION_T,     NULL, F(prim.mip_filter),          Q | S | A, BRTV_CONV_COPY},
};

static const GLSTATE_PRIMITIVE s_Default = {
	.flags					= PRIMF_COLOUR_WRITE | PRIMF_DEPTH_WRITE,
	.index_base				= 0,
	.index_range			= 0,
	.colour_type			= BRT_DEFAULT,
	//.z_write				= BRT_Z_WRITE_CONDITIONAL,
	.depth_test				= BRT_LESS,
	.blend_mode				= BRT_NONE,
	.perspective_type		= BRT_NONE,	// BRT_NONE, BRT_SUBDIVIDE
	.subdivide_tolerance	= 0,
	.colour_map				= NULL,
	.index_shade			= NULL,
	.index_blend			= NULL,
	.index_fog				= NULL,
	.screendoor				= NULL,
	.lighting				= NULL,
	.bump					= NULL,
	.fog_type				= BRT_NONE,
	.filter					= BRT_NONE,
	.mip_filter				= BRT_NONE,
};

void GLSTATEI_InitPrimitive(HGLSTATE hState)
{
	hState->templates.prim = BrTVTemplateAllocate(
		hState->resourceAnchor,
		GLSTATEI_PrimitiveTemplateEntries,
		BR_ASIZE(GLSTATEI_PrimitiveTemplateEntries)
	);

	hState->default_.prim = s_Default;
	hState->default_.valid |= GLSTATE_MASK_PRIMITIVE;
}
