/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: material.h 1.12 1998/11/13 16:22:27 jon Exp $
 * $Locker: $
 *
 * Describes the appearance of a material than can be applied to a surface
    Last change:  TN    9 Apr 97    4:41 pm
 */
#ifndef _MATERIAL_H_
#define _MATERIAL_H_

/**
 * \brief A structure describing the appearance of a material that can be applied to a surface.
 */
typedef struct br_material {
    br_uintptr_t _reserved;

    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * Can be used as a handle to retrieve a pointer to the material. Not intended for intensive use.
     * Typically used to collect pointers to materials loaded using BrMaterialLoad() and added to the
     * registry using BrMaterialAdd(). Also ideal for diagnostic purposes. A non-unique string can be
     * supplied, but which of a set of materials having the same string will be matched by search
     * functions (See BrMaterialFind()), is undefined. Also in consideration of searching, it is not
     * recommended that non-alphabetic characters are used, especially Slash ('/'), Asterisk ('*'), and
     * Query ('?'), which are used for pattern matching. This member can be modified by the programmer
     * at any time. If `identifier` is set by BrMaterialLoad() or BrMaterialLoadMany() it will have been
     * constructed using BrResStrDup().
     */
    char *identifier;

    /**
     * \brief When rendering in 'true' colour, the value of this member is taken as the basic colour of
     *        the face, which may of course be affected by lighting (if BR_MATF_LIGHT is set, but not
     *        BR_MATF_PRELIT).
     */
    br_colour colour;

    br_uint_8 opacity;

    /**
     * \brief The ambient lighting contribution for lit materials.
     *
     * This is the amount of light assumed to be reflected from other objects and lighting in general,
     * i.e. not from light actors. This means that even in a scene with all lights disabled, a lit
     * material will still be visible if it has a non-zero ambient lighting contribution. Zero can
     * produce a material whose illumination is highly dependent upon light sources, whereas higher
     * values can give ever fluorescent or luminous effects. A typical sunny scene might have most
     * materials with a significant ambient contribution, whereas a dusk scene might have a much lower
     * one, and a moonlit one, probably zero.
     */
    br_ufraction ka;
    /**
     * \brief The diffuse lighting contribution for lit materials.
     *
     * This determines how much of the reflected light is made up of the component dependent upon the
     * angle of the face to the direction of the light illuminating it. The closer the face comes to
     * being perpendicular to the light source, the more light the face receives, and thus the more
     * diffuse light that can be reflected. Zero can give a shiny surface, whereas higher values can
     * give surfaces a more matt appearance.
     */
    br_ufraction kd;
    /**
     * \brief The specular lighting contribution for this lit materials.
     *
     * This determines how much reflected light is made up of the component dependent upon the angle
     * between the reflected light source and the direction of viewer (naturally, if the angle is zero,
     * the component will be at its maximum). The greater the value, the more visible highlights will
     * be.
     */
    br_ufraction ks;

    /**
     * \brief This member applies a power to the specular lighting contribution.
     *
     * The greater the value, the sharper any highlights will be. A typical value is 20.
     */
    br_scalar power;

    /**
     * \brief This member determines how faces using the material are rendered, in terms of other
     *        members and aspects of the scene.
     *
     * \li BR_MATF_LIGHT — The material is lit – affected by lights in the scene
     * \li BR_MATF_PRELIT — The material is pre-lit – colours are taken directly from models' vertex
     *     structures (see br_vertex). Any lights are ignored.
     * \li BR_MATF_SMOOTH — Any lighting is applied using Gouraud shading. Lighting levels are linearly
     *     interpolated between vertices. Otherwise, the same lighting level is used across the face
     * \li BR_MATF_DITHER — Effectively applies a filter to the texture map to soften transitions
     *     between texels – most noticeable when a texel covers several screen pixels. A carefully
     *     chosen noise component is added to the u,v texel co-ordinates (a comparable effect to
     *     bilinear interpolation). Note that transparent pixels will also be dithered.
     * \li BR_MATF_ENVIRONMENT_I — Texels are calculated by casting a ray from the viewpoint (extended
     *     to infinity) and reflecting it off the face, out to an enclosing sphere (onto which the
     *     supplied texture has been mapped)
     * \li BR_MATF_ENVIRONMENT_L — Texels are calculated by casting a ray from the (local) viewpoint and
     *     reflecting it off the face, out to an enclosing sphere (onto which the supplied texture has
     *     been mapped)
     * \li BR_MATF_PERSPECTIVE — The texture is rendered with correct perspective (as opposed to using
     *     linear interpolation)
     * \li BR_MATF_DECAL — Both the texture mapped and non-texture mapped materials are drawn, the
     *     non-texture mapped material only appearing beneath what would have been transparent elements
     *     of the texture map. For example, this could be used to add symbols or logos to a smooth
     *     shaded model
     * \li BR_MATF_BLEND — The blend table is utilised (only for indexed textures) (see index_blend)
     * \li BR_MATF_ALWAYS_VISIBLE — Faces using the material will always be visible, and so back-face
     *     culling need not be performed for such faces
     * \li BR_MATF_TWO_SIDED — The material has two sides, and lighting calculations are performed for
     *     both of them
     * \li BR_MATF_FORCE_FRONT — The material is forced to be in front of all other materials
     *
     * The effects of various combinations of the first three flags are not particularly obvious so are
     * described in the following table. The Texture column indicates whether the material is
     * effectively textured. The Colour column indicates whether the pixel map rendered to is indexed or
     * 'true' colour (it is assumed that any texture shares this property). The 'Pixels Set To' column
     * describes how each pixel of a face using the material is set. Note that 'texel' is the term used
     * to refer to the element of the texture map corresponding to a particular screen pixel.
     *
     * | No. | Texture | Colour | PRELIT | LIGHT | SMOOTH | Pixels Set To |
     * | --- | --- | --- | --- | --- | --- | --- |
     * | 1 | no | 'True' | no | no | – | colour |
     * | 2 | no | 'True' | no | yes | no | colour, uniformly illuminated by average face lighting |
     * | 3 | no | 'True' | no | yes | yes | colour, illuminated by linearly interpolated lighting between vertices |
     * | 4 | no | 'True' | yes | – | no | Average prelit vertex colour |
     * | 5 | no | 'True' | yes | – | yes | Linearly interpolated between vertex prelit colours |
     * | 6 | no | Indexed | no | no | – | index_base |
     * | 7 | no | Indexed | no | yes | no | index_base + index_range ×(average face lighting) |
     * | 8 | no | Indexed | no | yes | yes | index_base + index_range ×(linearly interpolated lighting between vertices) |
     * | 9 | no | Indexed | yes | – | no | index_base + index_range ×(average vertex prelit index ÷ 256) |
     * | 10 | no | Indexed | yes | – | yes | index_base + index_range ×(linear interpolation between vertex prelit indices ÷ 256) |
     * | 11 | yes | 'True' | no | no | – | Texel |
     * | 12 | yes | 'True' | no | yes | no | Texel uniformly illuminated by average face lighting |
     * | 13 | yes | 'True' | no | yes | yes | Texel illuminated by linearly interpolated lighting between vertices |
     * | 14 | yes | 'True' | yes | – | no | Texel illuminated by average prelit vertex colour |
     * | 15 | yes | 'True' | yes | – | yes | Texel illuminated by colour linearly interpolated between vertex prelit colours |
     * | 16 | yes | Indexed | no | no | – | Texel |
     * | 17 | yes | Indexed | no | yes | no | Shade table: column [texel], row[index_base + index_range × (average face lighting)] |
     * | 18 | yes | Indexed | no | yes | yes | Shade table: column [texel], row[index_base + index_range × (linearly interpolated lighting between vertices)] |
     * | 19 | yes | Indexed | yes | – | no | Shade table: column [texel], row[index_base + index_range × (average vertex prelit index)] |
     * | 20 | yes | Indexed | yes | – | yes | Shade table: column [texel], row[index_base + index_range × (linear interpolation between vertex prelit indices)] |
     *
     * Table showing effects of different combinations of material lighting flags
     */
    br_uint_32 flags;

    /**
     * \brief The transform to apply to texture co-ordinates.
     *
     * This enables textures to be rotated, scaled, sheared, and translated. Moreover, this transform
     * can be continuously modified, thus providing animated texture effects. See BrModelApplyMap() for
     * details of how texture maps can be applied to a model's faces.
     */
    br_matrix23 map_transform;

    /*
     * Various mode bit fields
     */
    br_uint_16 mode;

    /**
     * \brief When rendering in indexed colour, this member determines the lower value of the index
     *        range used to colour a face.
     *
     * When lit, the light level is factored with index_range to obtain an index between index_base and
     * index_base+index_range-1. Without lighting, index_base is used to set every pixel of the face.
     * This member only applies to materials without textures - with textures, this member is ignored.
     */
    br_uint_8 index_base;
    /**
     * \brief When rendering in indexed colour, this member determines the number of index values that
     *        can be selected with a given lighting level, starting at index_base.
     *
     * This member only applies to materials without textures - with textures, this member is ignored.
     */
    br_uint_8 index_range;

    /**
     * \brief A pointer to a pixel map containing a pattern with which to cover faces using this
     *        material.
     *
     * Pixels that are zero in the pixel map are not further processed for colour information, and are
     * effectively transparent. A face is not rendered where such pixels appear on its surface, nor are
     * any corresponding values written to any depth buffer. Note though, that a 2D pick function will
     * still pick a transparent face. For more sophisticated transparency effects see the description of
     * index_blend above. Note that indexed colour textures must also have a corresponding shade table.
     */
    br_pixelmap *colour_map;

    /*
     * Pointers to tables
     */
    br_pixelmap *screendoor; /* Screen door opacity  */

    /**
     * \brief Materials with indexed colour texture maps can only be lit if they are accompanied by an
     *        appropriate shade table.
     *
     * The shade table is simply a way of tabulating the output pixel given a particular texel and a
     * particular lighting level. The texel generally indexes the column and the lighting level, the
     * row. The shade table must therefore have the full complement of columns necessary for the pixel
     * size of the texture map. The shade table may have any number of rows, as the lighting level
     * directly selects the row. Thus an 8 bit indexed colour texture map requires a shade table with
     * 256 columns and two or more rows (one being redundant). A selection of shade table types for use
     * with 8 bit textures are described below.
     *
     * \li BR_PMT_INDEX_8 shade table to BR_PMT_INDEX_8 output — The shade table converts 8 bit texels
     *     into 8 bit lit pixel values, by using the pixel in the shade table at the column given by the
     *     texel and the row given by the monochrome lighting level.
     * \li BR_PMT_RGB_555 shade table to BR_PMT_RGB_555 output — The shade table converts 8 bit texels
     *     into 15 bit lit pixel values, by using the pixel in the shade table at the column given by
     *     the texel and the row given by the monochrome lighting level.
     * \li BR_PMT_RGBX_888 shade table to BR_PMT_INDEX_8 output — The shade table converts 8 bit texels
     *     into 8 bit lit pixel values, by using a single 256 column wide table, but with 24 bit colour
     *     values in order that true colour lighting can be applied. The red and green components of
     *     each value are actually column indices used to obtain a blue component which is produced as
     *     the output pixel. The table is thus read three times:
     * \li 1. X1=Red component of pixel in shade table at column(Texel), row(Red light level)
     * \li 2. X2=Green component of pixel in shade table at column(X1), row(Green light level)
     * \li 3. X3=Blue component of pixel in shade table at column(X2), row(Blue light level)
     * \li 4. Ouput pixel is X3
     */
    br_pixelmap *index_shade;
    /**
     * \brief Blending is a way of making the destination pixel depend upon the existing contents of the
     *        output buffer.
     *
     * The blend table like the shade table tabulates the output pixel given a particular texel and the
     * pixel already in the output buffer. The texel indexes the column and the existing output pixel
     * the row. The shade table must therefore have the full complement of rows as well as columns
     * necessary for the pixel size of the texture map. Thus an 8 bit indexed colour texture map blend
     * table needs 256 columns and 256 rows. The blend table is only used if BR_MATF_BLEND is set, and
     * if set this member should not be NULL. Given that the blend table tabulates an output for every
     * possible pair of input pixels, any function can be represented, e.g. exclusive-or, inverse,
     * inclusive-or, mask, average, etc. The blend table is typically used for translucence effects,
     * e.g. ghosts, flames, frosted glass, etc. Note that in the case of the depth buffer renderer,
     * while the output pixel will only be written if it is nearer than the existing pixel, it will
     * never modify the depth buffer value. This is because it is assumed to be an intangible surface.
     * Therefore, it may be necessary to render such materials in a separate stage. Note that if both a
     * blend table and shade table is used (the material has an indexed texture which is lit and
     * blended), the shade table is applied first, followed by the blend table. Blend tables, as with
     * shade tables, also need to be added to the registry before use.
     */
    br_pixelmap *index_blend;
    br_pixelmap *index_fog;   /* Index fogging        */

    br_token_value *extra_surf;
    br_token_value *extra_prim;

    br_scalar fog_min;
    br_scalar fog_max;
    br_colour fog_colour;

    br_int_32 subdivide_tolerance;

    br_scalar depth_bias;

    /**
     * \brief This member may be used by the application for its own purposes.
     *
     * It is initialised to NULL upon allocation, and not accessed by BRender thereafter.
     */
    void *user;
    void *stored;

} br_material;

/*
 * Bits for br_material->flags
 */
enum {
    BR_MATF_LIGHT  = 0x00000001,
    BR_MATF_PRELIT = 0x00000002,

    BR_MATF_SMOOTH = 0x00000004,

    BR_MATF_ENVIRONMENT_I = 0x00000008,
    BR_MATF_ENVIRONMENT_L = 0x00000010,
    BR_MATF_PERSPECTIVE   = 0x00000020,
    BR_MATF_DECAL         = 0x00000040,

    BR_MATF_I_FROM_U = 0x00000080,
    BR_MATF_I_FROM_V = 0x00000100,
    BR_MATF_U_FROM_I = 0x00000200,
    BR_MATF_V_FROM_I = 0x00000400,

    BR_MATF_ALWAYS_VISIBLE = 0x00000800,
    BR_MATF_TWO_SIDED      = 0x00001000,

    BR_MATF_FORCE_FRONT = 0x00002000,

    BR_MATF_DITHER = 0x00004000,
#if 0
	BR_MATF_CUSTOM			= 0x00008000
#endif

    BR_MATF_MAP_ANTIALIASING  = 0x00010000,
    BR_MATF_MAP_INTERPOLATION = 0x00020000,
    BR_MATF_MIP_INTERPOLATION = 0x00040000,

    BR_MATF_FOG_LOCAL = 0x00080000,
    BR_MATF_SUBDIVIDE = 0x00100000,

    /* GRID extensions. */

    BR_MATF_RESERVED_0 = 0x00200000,

    BR_MATF_QUAD_MAPPING = 0x00400000,

    BR_MATF_FORCE_BACK = 0x00800000,

    BR_MATF_INHIBIT_DEPTH_WRITE = 0x01000000,

    BR_MATF_BLEND = 0x02000000,

    BR_MATF_PREALPHA = 0x04000000,

    BR_MATF_SEPARATE_SPECULAR = 0x08000000,

    BR_MATF_MODULATE_ALPHA = 0x10000000,

    BR_MATF_DISABLE_COLOUR_KEY = 0x20000000,

    BR_MATF_SMOOTH_ALPHA = 0x40000000,

    /*
     * Backwards compatibility
     */
    BR_MATF_GOURAUD    = BR_MATF_SMOOTH,
    BR_MATF_MAP_COLOUR = 0,
    BR_MATF_FORCE_Z_0  = BR_MATF_FORCE_FRONT,
};

/*
 * Bits for br_material->mode
 */
enum {
    BR_MATM_DEPTH_TEST_MASK = 0x0007,
    BR_MATM_DEPTH_TEST_GT   = 0x0000,
    BR_MATM_DEPTH_TEST_GE   = 0x0001,
    BR_MATM_DEPTH_TEST_EQ   = 0x0002,
    BR_MATM_DEPTH_TEST_NE   = 0x0003,
    BR_MATM_DEPTH_TEST_LE   = 0x0004,
    BR_MATM_DEPTH_TEST_LT   = 0x0005,
    BR_MATM_DEPTH_TEST_NV   = 0x0006,
    BR_MATM_DEPTH_TEST_AL   = 0x0007,

    BR_MATM_BLEND_MODE_MASK          = 0x0018,
    BR_MATM_BLEND_MODE_STANDARD      = 0x0000,
    BR_MATM_BLEND_MODE_SUMMED        = 0x0008,
    BR_MATM_BLEND_MODE_DIMMED        = 0x0010,
    BR_MATM_BLEND_MODE_PREMULTIPLIED = 0x0018,

    BR_MATM_MAP_WIDTH_LIMIT_MASK   = 0x0060,
    BR_MATM_MAP_WIDTH_LIMIT_WRAP   = 0x0000,
    BR_MATM_MAP_WIDTH_LIMIT_CLAMP  = 0x0020,
    BR_MATM_MAP_WIDTH_LIMIT_MIRROR = 0x0040,

    BR_MATM_MAP_HEIGHT_LIMIT_MASK   = 0x0180,
    BR_MATM_MAP_HEIGHT_LIMIT_WRAP   = 0x0000,
    BR_MATM_MAP_HEIGHT_LIMIT_CLAMP  = 0x0080,
    BR_MATM_MAP_HEIGHT_LIMIT_MIRROR = 0x0100,

    BR_MATM_SHADING_MODE_MASK    = 0x0600,
    BR_MATM_SHADING_MODE_FLAT    = 0x0000,
    BR_MATM_SHADING_MODE_GOURAUD = 0x0200,
    BR_MATM_SHADING_MODE_PHONG   = 0x0400,
};

/*
 * Flags to BrMaterialUpdate()
 */
enum {
    BR_MATU_MAP_TRANSFORM = 0x0001,
    BR_MATU_RENDERING     = 0x0002,
    BR_MATU_LIGHTING      = 0x0004,
    BR_MATU_COLOURMAP     = 0x0008,
    BR_MATU_SCREENDOOR    = 0x0010,
    BR_MATU_EXTRA_SURF    = 0x0020,
    BR_MATU_EXTRA_PRIM    = 0x0040,
    BR_MATU_ALL           = 0x7fff
};

/*
 * Backwards compatibility
 */

/*
 * Flags to BrMapUpdate()
 */
enum {
    BR_MAPU_DATA = 0x0001, // If this flag is ommitted the driver gets a chance to free the original (assuming
                           // KEEP_ORIGINAL not set) without doing any extra work.
    BR_MAPU_ALL    = 0x0fff,
    BR_MAPU_SHARED = 0x8000,
};

/*
 * Flags to BrTableUpdate()
 */
enum {
    BR_TABU_ALL = 0x7fff
};

#endif
