/*
 * softprim axis vocabulary
 *
 * A rasteriser entry is described by a tuple of these values, one per axis.
 * The tuples are generated from the .ifg block descriptions by
 * drivers/softprim/infogen.pl in its `softprim` mode, so these names must match
 * the tokens that generator emits; a mismatch is a compile error rather than a
 * silently different kernel.
 *
 * The order of the tuple - and therefore the order of the members below - is
 * the order infogen.pl emits: format, topology, depth, shade, texture, address
 * mode, perspective, blend, fog, dither.
 *
 * Only the axes that change the per-pixel instruction sequence or the state the
 * span loop must keep live are here. Map dimensions, strides, palettes, index
 * base and range, and shade/blend/fog table contents are runtime values read
 * from the bound buffers and are deliberately absent.
 */
#ifndef _SOFTPRIM_AXES_H_
#define _SOFTPRIM_AXES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Output pixel format. */
typedef enum sp_fmt {
    SP_FMT_I8,
    SP_FMT_555,
    SP_FMT_565,
    SP_FMT_888
} sp_fmt;

/* Primitive shape. */
typedef enum sp_top {
    SP_TOP_TRI,
    SP_TOP_LINE,
    SP_TOP_POINT
} sp_top;

/* Depth test and write. SP_DEPTH_NONE means the kernel does neither, which is
 * the Z-sort path. */
typedef enum sp_depth {
    SP_DEPTH_NONE,
    SP_DEPTH_ZW
} sp_depth;

/* Where the colour comes from, and whether it varies across the primitive.
 * For an indexed output the slot carries an intensity; for an RGB output it
 * carries R,G,B - except for the _I_RGB values below, where an RGB output's
 * slot carries an intensity instead. The slot set is pinned by the output
 * format, but 40 .ifg blocks (prm_t15/prm_t16/prim_t24's "Interpolated/
 * Constant Intensity, Textured" entries) do cross the two.
 *
 * The _TABLE values are the untextured "Indexed Shading" blocks
 * (prim_t8.ifg:313-315, property_shade_table): the intensity is an index into
 * the bound shade table, not a colour index in its own right, so the fragment
 * is shade[(intensity << 8) | index_base] rather than the intensity byte
 * (zb8sh.asm's DRAW_ZI_I8_D16_ShadeTable, fpsetup.asm's
 * SETUP_FLOAT_COLOUR_SHADETABLE). They are distinct kernels from the plain
 * untextured intensity blocks even though the .ifg tuple is otherwise the
 * same, which is why they need axis values of their own.
 *
 * The _I_RGB values are the RGB-output half of that shade-table family: an
 * RGB output whose intensity is the row of the bound shade table and whose
 * index-8 texel is the column (awtmi.h's LIGHT path, t_piza.asm:165-213). They
 * exist so the projection carries the declared CM_I rather than collapsing the
 * gouraud and flat entries onto the unlit tuple, whose kernel decodes the
 * palette and reads no intensity. SP_SPEC names them, but not every shape that
 * carries them has a kernel: the RGB_888 and the z-sorted arbitrary-width
 * 15/16bpp pairs are emitted, and infogen.pl refuses the rest by an explicit
 * rule (the power-of-two 15/16bpp shapes, perspi.h's separate perspective
 * mapper). See RgbAwtTriangle. */
typedef enum sp_shade {
    SP_SHADE_NONE,
    SP_SHADE_CONST_I,
    SP_SHADE_INTERP_I,
    SP_SHADE_CONST_I_TABLE,
    SP_SHADE_INTERP_I_TABLE,
    SP_SHADE_CONST_RGB,
    SP_SHADE_INTERP_RGB,
    SP_SHADE_CONST_I_RGB,
    SP_SHADE_INTERP_I_RGB
} sp_shade;

/* Texture decode. SP_TEX_NONE means the kernel samples no texture; it is not a
 * requirement that no texture is bound. */
typedef enum sp_tex {
    SP_TEX_NONE,
    SP_TEX_I8,
    SP_TEX_555,
    SP_TEX_565,
    SP_TEX_RGBX888,
    SP_TEX_RGB888
} sp_tex;

/* Texture coordinate addressing: shift and mask for a power-of-two map, or a
 * divide for an arbitrary width. */
typedef enum sp_addr {
    SP_ADDR_NONE,
    SP_ADDR_SHIFT,
    SP_ADDR_DIVIDE
} sp_addr;

/* Perspective-correct texture mapping. Note that pentprim also falls back to
 * the affine kernel per triangle when the w range is narrow
 * (pfpsetup.asm SETUP_FLOAT_CHECK_PERSPECTIVE_CHEAT), so a CORRECT entry has to
 * keep the affine path reachable. */
typedef enum sp_persp {
    SP_PERSP_AFFINE,
    SP_PERSP_CORRECT
} sp_persp;

/* How the fragment is combined with the destination. */
typedef enum sp_blend {
    SP_BLEND_NONE,
    SP_BLEND_INDEX,
    SP_BLEND_ALPHA,
    SP_BLEND_SCREENDOOR,
    SP_BLEND_DECAL
} sp_blend;

/* Fog. Indexed output only; pentprim has no fogged RGB block at all. */
typedef enum sp_fog {
    SP_FOG_NONE,
    SP_FOG_INDEX
} sp_fog;

/* Dither source. SP_DITH_COLOUR reads a table, SP_DITH_MAP computes the
 * fraction inline and selects a row from y. */
typedef enum sp_dith {
    SP_DITH_NONE,
    SP_DITH_COLOUR,
    SP_DITH_MAP
} sp_dith;

/*
 * One tuple. The field order here is the tuple order; adding an axis means
 * adding it in the same position in infogen.pl's softprim_tuple().
 */
typedef struct sp_axes {
    sp_fmt   fmt;
    sp_top   top;
    sp_depth depth;
    sp_shade shade;
    sp_tex   tex;
    sp_addr  addr;
    sp_persp persp;
    sp_blend blend;
    sp_fog   fog;
    sp_dith  dith;
} sp_axes;

#ifdef __cplusplus
}
#endif
#endif
