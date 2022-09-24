/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: material.h 2.17 1997/05/22 14:42:22 jon Exp ALISTAIR $
 * $Locker: ALISTAIR $
 *
 * Describes the appearance of a material than can be applied to a surface
	Last change:  TN    9 Apr 97    4:41 pm
 */
#ifndef _MATERIAL_H_
#define _MATERIAL_H_

typedef struct br_material {
	br_uintptr_t _reserved;

	char *identifier;

	br_colour colour;

	br_uint_8 opacity;

// Eight bit ambient ambient lighting level.
	br_ufraction ka;
	br_ufraction kd;
	br_ufraction ks;

	/*
	 * Specular power
	 */
	br_scalar power;

	/*
	 * Controls how texture is rendered
	 */
	br_uint_32 flags;

	/*
	 * 2x3 transform applied to model U,V
	 */
	br_matrix23 map_transform;

	/*
	 * Various mode bit fields
	 */
	br_uint_16 mode;

	/*
	 * The direct index ramp base and range
	 */
	br_uint_8 index_base;
	br_uint_8 index_range;

	/*
	 * Pointers to colour map
	 */
	br_pixelmap *colour_map;

	/*
	 * Pointers to tables
	 */
	br_pixelmap *screendoor;		/* Screen door opacity  */

	br_pixelmap *index_shade;		/* Index shading        */
	br_pixelmap *index_blend;		/* Index blending       */
	br_pixelmap *index_fog;			/* Index fogging        */

	br_token_value *extra_surf;
	br_token_value *extra_prim;

	br_scalar fog_min;
	br_scalar fog_max;
	br_colour fog_colour;

    br_uint_32 flags_ext;

    br_uint_8 alpha_mode;
    br_uint_8 zbuffer_mode;

    br_int_32 subdivide_tolerance;

	br_scalar depth_bias;

	/*
	 * Workspace fields for user and database
	 */
	void	  *user;
	void	  *stored;

} br_material;

/*
 * Bits for br_material->flags
 */
enum {
	BR_MATF_LIGHT			= 0x00000001,
	BR_MATF_PRELIT			= 0x00000002,

	BR_MATF_SMOOTH			= 0x00000004,

	BR_MATF_ENVIRONMENT_I	= 0x00000008,
	BR_MATF_ENVIRONMENT_L	= 0x00000010,
	BR_MATF_PERSPECTIVE		= 0x00000020,
	BR_MATF_DECAL			= 0x00000040,

	BR_MATF_I_FROM_U		= 0x00000080,
	BR_MATF_I_FROM_V		= 0x00000100,
	BR_MATF_U_FROM_I		= 0x00000200,
	BR_MATF_V_FROM_I		= 0x00000400,

	BR_MATF_ALWAYS_VISIBLE	= 0x00000800,
	BR_MATF_TWO_SIDED		= 0x00001000,

	BR_MATF_FORCE_FRONT		= 0x00002000,

	BR_MATF_DITHER			= 0x00004000,
#if 0
	BR_MATF_CUSTOM			= 0x00008000
#endif

	BR_MATF_MAP_ANTIALIASING	= 0x00010000,
	BR_MATF_MAP_INTERPOLATION 	= 0x00020000,
	BR_MATF_MIP_INTERPOLATION 	= 0x00040000,

	BR_MATF_FOG_LOCAL       = 0x00080000,
	BR_MATF_SUBDIVIDE       = 0x00100000,

    /* GRID extensions. */

	BR_MATF_RESERVED_0 = 0x00200000,

	BR_MATF_QUAD_MAPPING	= 0x00400000,

	BR_MATF_FORCE_BACK = 0x00800000,
};

/*
 * Backwards compatibility
 */
#define BR_MATF_GOURAUD		BR_MATF_SMOOTH
#define BR_MATF_MAP_COLOUR	0
#define BR_MATF_FORCE_Z_0	BR_MATF_FORCE_FRONT
#define BR_MATF_BLEND		0

/*
 * Bits for br_material->mode
 */
enum {
	BR_MATM_DEPTH_TEST_MASK	= 0x0007,
	BR_MATM_DEPTH_TEST_GT	= 0x0000,
	BR_MATM_DEPTH_TEST_GE	= 0x0001,
	BR_MATM_DEPTH_TEST_EQ	= 0x0002,
	BR_MATM_DEPTH_TEST_NE	= 0x0003,
	BR_MATM_DEPTH_TEST_LE	= 0x0004,
	BR_MATM_DEPTH_TEST_LT	= 0x0005,
	BR_MATM_DEPTH_TEST_NV	= 0x0006,
	BR_MATM_DEPTH_TEST_AL	= 0x0007,
};

/*
 * Bits for material->flags_ext
 */

enum {
	BR_MATFX_TEXTURE0_MIRRORED_U = 0x00000001,
	BR_MATFX_TEXTURE0_CLAMPED_U  = 0x00000002,
	BR_MATFX_TEXTURE0_MIRRORED_V = 0x00000004,
	BR_MATFX_TEXTURE0_CLAMPED_V  = 0x00000008,
	BR_MATFX_TEXTURE1_MIRRORED_U = 0x00000010,
	BR_MATFX_TEXTURE1_CLAMPED_U  = 0x00000020,
	BR_MATFX_TEXTURE1_MIRRORED_V = 0x00000040,
	BR_MATFX_TEXTURE1_CLAMPED_V  = 0x00000080,
	BR_MATFX_EDGE_ANTIALIASING   = 0x00000100,

    BR_MATFX_DOUBLE_TEXTURE      = 0x00000200,
    BR_MATFX_CONSTANT_FOG        = 0x00000400,
    BR_MATFX_CONSTANT_ALPHA      = 0x00000800,
    BR_MATFX_BACKGROUND_ALPHA    = 0x00001000,
    BR_MATFX_MIP_MOD_ABSOLUTE    = 0x00002000,

    BR_MATFX_SMOOTH_1            = 0x00004000
};


/*
 * Bits for screen alpha-blend mode
 */

enum {
    BR_MATB_NONE                        = 0x00,
    BR_MATB_INTENSITY_REDUCTION         = 0x01,
    BR_MATB_COMPOSITING_WITH_REDUCTION  = 0x02,
    BR_MATB_STANDARD_BLEND              = 0x03,
    BR_MATB_COMPOSITING                 = 0x04,
    BR_MATB_COMPOSITING_WITH_INCREASE   = 0x05
};


/* Bits for z buffer mode */

enum {
    BR_MATZM_STANDARD     = 0x00,
    BR_MATZM_ALWAYS_WRITE = 0x01,
    BR_MATZM_DONT_WRITE   = 0x02,
    BR_MATZM_OFF          = 0x03
};

/*
 * Flags to BrMaterialUpdate()
 */
enum {
	BR_MATU_MAP_TRANSFORM	= 0x0001,
	BR_MATU_RENDERING		= 0x0002,
	BR_MATU_LIGHTING		= 0x0004,
	BR_MATU_COLOURMAP		= 0x0008,
	BR_MATU_SCREENDOOR		= 0x0010,
	BR_MATU_EXTRA_SURF		= 0x0020,
	BR_MATU_EXTRA_PRIM		= 0x0040,
	BR_MATU_ALL				= 0x7fff
};

/*
 * Backwards compatibility
 */

/*
 * Flags to BrMapUpdate()
 */
enum {
	BR_MAPU_DATA			= 0x0001, // If this flag is ommitted the driver gets a chance to free the original (assuming KEEP_ORIGINAL not set) without doing any extra work.
	BR_MAPU_ALL				= 0x0fff,
	BR_MAPU_SHARED			= 0x8000
};

/*
 * Flags to BrTableUpdate()
 */
enum {
	BR_TABU_ALL				= 0x7fff
};

#endif
