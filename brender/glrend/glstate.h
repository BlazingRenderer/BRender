#ifndef _STATE_H
#define _STATE_H

#include "drv.h"

#define GLSTATE_API
#define GLSTATE_MAX_COUNT 64
#define GLSTATE_MAX_LIGHTS BR_MAX_LIGHTS

enum
{
	GLSTATE_MASK_MATRIX = BR_STATE_MATRIX,
	GLSTATE_MASK_CULL = BR_STATE_CULL,
	GLSTATE_MASK_SURFACE = BR_STATE_SURFACE,
	GLSTATE_MASK_PRIMITIVE = BR_STATE_PRIMITIVE,
	GLSTATE_MASK_OUTPUT = BR_STATE_OUTPUT,
	GLSTATE_MASK_LIGHT = BR_STATE_LIGHT,
	GLSTATE_MASK_GLOBAL = BR_STATE_GLOBAL,
};


/*
* Bits used for template masks to indicate actions when a template entry is triggered
*/
#define TM_PART				0x00001		/* set timestamp for whole part								*/
#define TM_INDEX			0x00002		/* set timestamp for indexed part							*/
#define TM_V2S_HINT			0x00004		/* set timestamp for matrix.view_to_screen_hint				*/
#define TM_M2V				0x00008		/* set timestamp for matrix.model_to_view					*/
#define TM_V2S				0x00010		/* set timestamp for matrix.view_to_screen					*/
#define TM_COPY_CACHE		0x00020		/* set timestamp for anything that affects the copied cache	*/
#define TM_CACHE			0x00040		/* set timestamp for anything that affects the static cache	*/

#define TM_CLEAR_M2V_HINT	0x00080		/* set M2V_HINT to NONE										*/
#define TM_CLEAR_V2S_HINT	0x00100		/* set V2S_HINT to NONE										*/


#define TM_INVALID_PS		0x01000		/* set Update Per Scene										*/
#define TM_INVALID_PM		0x02000		/* set Update Per Model										*/
#define TM_INVALID_V2M		0x04000		/* set Update View To Model									*/
#define TM_INVALID_M2S		0x08000		/* set Update Model To Screen								*/
#define TM_INVALID_CC		0x10000		/* set Update Copied Cache									*/

typedef struct _GLSTATE_MATRIX
{
	br_matrix34	model_to_view;
	br_matrix4	view_to_screen;
	br_matrix34	view_to_environment;

	br_token	model_to_view_hint;
	br_token	view_to_screen_hint;
	br_token	view_to_environment_hint;

	br_scalar	hither_z;
	br_scalar	yon_z;

} GLSTATE_MATRIX, *HGLSTATE_MATRIX;

typedef struct _GLSTATE_CULL
{
	br_token type;
	br_token space;
} GLSTATE_CULL, *HGLSTATE_CULL;

typedef struct _GLSTATE_SURFACE
{
	br_colour	colour;

	br_scalar	opacity;
	br_scalar	ka;
	br_scalar	kd;
	br_scalar	ks;
	br_scalar	power;

	br_boolean	lighting;
	br_boolean	force_front;
	br_boolean	force_back;
	br_scalar	depth_bias;

	br_token	colour_source;

	br_matrix23	map_transform;
} GLSTATE_SURFACE, *HGLSTATE_SURFACE;

typedef struct _GLSTATE_LIGHT
{
	br_token	type;
	br_token	lighting_space;

	br_vector3_f	position;
	br_vector3_f	direction;

	br_colour	colour;

	br_scalar	spot_outer;
	br_scalar	spot_inner;

	br_scalar	attenuation_l;
	br_scalar	attenuation_c;
	br_scalar	attenuation_q;
} GLSTATE_LIGHT, *HGLSTATE_LIGHT;

typedef void insert_cbfn(
	br_primitive *primitive,
	void *arg1, void *arg2, void *arg3,
	br_order_table *order_table, br_scalar *z);

typedef struct _GLSTATE_HIDDEN
{
	br_token			type;
	br_token			divert;
	br_order_table		*order_table;
	br_primitive_heap	*heap;
	insert_cbfn			*insert_fn;
	void				*insert_arg1;
	void				*insert_arg2;
	void				*insert_arg3;
} GLSTATE_HIDDEN, *HGLSTATE_HIDDEN;

/*
* state.prim.flags
*/
enum
{
	/*
	* The public flags
	*/
	PRIMF_DEPTH_WRITE_BIT,
	PRIMF_COLOUR_WRITE_BIT,

	PRIMF_BLEND_BIT,
	PRIMF_MODULATE_BIT,
	PRIMF_COLOUR_KEY_BIT,
};


enum
{
	PRIMF_DEPTH_WRITE = (1 << PRIMF_DEPTH_WRITE_BIT),
	PRIMF_COLOUR_WRITE = (1 << PRIMF_COLOUR_WRITE_BIT),
	PRIMF_BLEND = (1 << PRIMF_BLEND_BIT),
	PRIMF_MODULATE = (1 << PRIMF_MODULATE_BIT),
	PRIMF_COLOUR_KEY = (1 << PRIMF_COLOUR_KEY_BIT),
};

struct br_buffer_stored;

typedef struct _GLSTATE_PRIMITIVE
{
	/*
	* flags
	*/
	br_uint_32 flags;

	/*
	* User selected ramp
	*/
	br_int_32	index_base;
	br_int_32	index_range;

	/*
	* Type of colour
	*/
	br_token	colour_type;

	br_token	depth_test;

	br_token	blend_mode;

	/*
	* Type of perspective correct rendering
	*/
	br_token	perspective_type;
	br_int_32	subdivide_tolerance;

	/*
	* Current input buffers
	*/
	struct br_buffer_stored *colour_map;
	struct br_buffer_stored *index_shade;
	struct br_buffer_stored *index_blend;
	struct br_buffer_stored *index_fog;
	struct br_buffer_stored *screendoor;
	struct br_buffer_stored *lighting;
	struct br_buffer_stored *bump;

	/* Texture filtering. BRT_NONE or BRT_LINEAR */
	br_token		filter;

	/* Mipmap filtering. BRT_NONE or BRT_LINEAR */
	br_token		mip_filter;
} GLSTATE_PRIMITIVE, *HGLSTATE_PRIMITIVE;

typedef struct _GLSTATE_OUTPUT
{
	struct br_device_pixelmap *colour;
	struct br_device_pixelmap *depth;
} GLSTATE_OUTPUT, *HGLSTATE_OUTPUT;

typedef struct _GLSTATE_STACK
{
	GLSTATE_MATRIX matrix;

	/* Used for texture/materials */
	GLSTATE_CULL cull;
	GLSTATE_SURFACE surface;
	GLSTATE_PRIMITIVE prim;
	GLSTATE_OUTPUT output;
	GLSTATE_LIGHT light[GLSTATE_MAX_LIGHTS];
	GLSTATE_HIDDEN hidden;

	uint32_t valid;
} GLSTATE_STACK, *HGLSTATE_STACK;

typedef struct _GLCACHE
{
	GLuint fbo;

	alignas(16) GLSTD140_SCENE_DATA scene;

	struct
	{
		br_matrix4 p;
		br_matrix4 mv;
		br_matrix4 mvp;
		br_matrix4 normal;
	} model;

	/*
	** Not really sure what this is for.
	** Keeping it around just in case.
	** This is per-model.
	*/
	br_matrix34 model_to_environment;

} GLCACHE, *HGLCACHE;

typedef struct _GLSTATE
{
	GLSTATE_STACK default_;
	GLSTATE_STACK *current;
	GLSTATE_STACK stack[GLSTATE_MAX_COUNT];
	int top;

	GLCACHE cache;

	struct
	{
		struct br_tv_template *matrix;
		struct br_tv_template *cull;
		struct br_tv_template *surface;
		struct br_tv_template *prim;
		struct br_tv_template *output;
		struct br_tv_template *light[GLSTATE_MAX_LIGHTS];
		struct br_tv_template *hidden;
	} templates;

	void *resourceAnchor;

} GLSTATE, *HGLSTATE;

#if defined(__cplusplus)
extern "C" {
#endif

/*
** Initialise the state stack.
** - Allocates part templates
**/
GLSTATE_API void GLSTATE_Init(HGLSTATE hState, void *resAnchor);
GLSTATE_API br_boolean GLSTATE_Push(HGLSTATE hState, uint32_t mask);
GLSTATE_API br_boolean GLSTATE_Pop(HGLSTATE hState, uint32_t mask);
GLSTATE_API void GLSTATE_Default(HGLSTATE hState, uint32_t mask);
GLSTATE_API void GLSTATE_TemplateActions(HGLSTATE hState, uint32_t mask);

GLSTATE_API struct br_tv_template *GLSTATE_GetStateTemplate(HGLSTATE hState, br_token part, br_int_32 index);

GLSTATE_API void GLSTATE_Copy(HGLSTATE_STACK hDest, HGLSTATE_STACK hSrc, uint32_t mask);

GLSTATE_API void GLCACHE_UpdateScene(HGLCACHE hCache, HGLSTATE_STACK hState);
GLSTATE_API void GLCACHE_UpdateModel(HGLCACHE hCache, HGLSTATE_MATRIX hMatrix);

GLSTATE_API void GLCACHE_Reset(HGLCACHE hCache);

#if defined(__cplusplus)
}
#endif

#endif /* _STATE_H */
