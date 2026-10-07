/*
 * softprim - a portably written replacement for the pentprim rasteriser.
 *
 * This header is the C ABI between the DDI plumbing (C) and the triangle
 * rasteriser (C++). It must never expose C++ types: it is included by C
 * translation units.
 *
 * Stage 1 renders flat-shaded, z-buffered INDEX_8 triangles only.
 */
#ifndef _SOFTPRIM_H_
#define _SOFTPRIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _BRDDI_H_
#include "brddi.h"
#endif

/*
 * A match type of "no requirement", so a block's required type never collides
 * with a real BR_PMT_*, whose first value is zero. It is pentprim's PMT_NONE by
 * another name: the generated blocks (drivers/softprim/infogen.pl) carry it
 * wherever pentprim's table carries PMT_NONE.
 */
#define SP_PMT_NONE 255

/*
 * A minimal description of a destination buffer, derived from a
 * br_device_pixelmap once per render begin.
 *
 * The fields here are the ones pstate.c needs to compare input buffers,
 * plus the direct pixel access the rasteriser needs.
 */
typedef struct softprim_buffer {
    br_uint_8 *base;     /* pixel (0,0) of the visible region */
    br_int_32  stride_b; /* bytes between the same column of adjacent rows */
    br_int_32  width_p;  /* visible width, in pixels */
    br_int_32  height;   /* visible height, in scanlines */
    br_uint_8  type;     /* BR_PMT_* */
    br_int_32  bpp;      /* bytes per pixel, from the type */

    /* Colour map attached to this buffer, if any. Only a texture needs it:
     * sampling an INDEX_8 texture into an RGB output decodes through the
     * palette. entry_b is the bytes per entry and entry_stride_b the bytes
     * between entries, both taken from the map pixelmap. */
    const br_uint_8 *palette;
    br_int_32        palette_size;
    br_uint_8        palette_type;
    br_int_32        palette_entry_b;
    br_int_32        palette_stride_b;
    br_int_32        palette_stride_p;
} softprim_buffer;

/*
 * Buffers the stage-1 rasteriser is currently pointed at.
 *
 * Updated by SoftPrimWorkUpdate() from the primitive state at every
 * render begin, so the rasteriser never reaches into the state itself.
 */
typedef struct softprim_work {
    softprim_buffer colour;
    softprim_buffer depth;
    /* Bound INDEX_8 colour map, sampled by the textured kernels. Zero when no
     * map is bound. */
    softprim_buffer texture;
    /* Bound INDEX_8 shade table, indexed by (intensity << 8) | texel. Zero when
     * no table is bound. */
    softprim_buffer shade;
    /* Bound INDEX_8 fog table, indexed by (depth_high_byte << 8) | fragment.
     * Zero when no table is bound. */
    softprim_buffer fog;
    /* Bound INDEX_8 blend table, indexed by (destination << 8) | fragment.
     * Zero when no table is bound. */
    softprim_buffer blend;
    /* The primitive's index range, taken while a shade table is bound and kept
     * otherwise. pentprim's work area behaves the same way, and the RANGE_ZERO
     * match flag is tested from it. */
    br_int_32 index_range;
    /* The primitive's index base, taken at the same point. Decal needs both to
     * remap the vertex intensity into the destination band. */
    br_int_32 index_base;
} softprim_work;

extern softprim_work SoftPrimWork;

/*
 * Fill a softprim_buffer from a device pixelmap, honouring base_x/base_y.
 */
void SoftPrimSetupBuffer(softprim_buffer *rb, struct br_device_pixelmap *pm);

/*
 * Stage-1 triangle renderer: INDEX_8, flat/Gouraud, optional texture.
 *
 * Called through the primitive block's render pointer, i.e. with the same
 * (block, v0, v1, v2) signature as brp_render3.
 */
void BR_ASM_CALL SoftPrimTriangleFlat(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2);

#ifdef __cplusplus
}
#endif
#endif
