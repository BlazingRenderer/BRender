/*
 * softprim triangle rasteriser.
 *
 * This is the only C++ in softprim, and it is C++ purely for type safety: plain
 * value types, no virtual functions, no exceptions, no RTTI and no STL. The
 * exported entry points are C ABI functions with the brp_render3 signature.
 *
 * The block set is generated. softprim_blocks.h expands one SOFTPRIM_BLOCK line
 * per distinct axis tuple into an entry point below, and match.c expands the
 * same lines into the matcher's table - so a block cannot name a kernel that
 * does not exist, and a kernel cannot be instantiated for an axis value that
 * has no implementation. The static assertions in SoftPrimRender are what make
 * the second half of that true.
 *
 * Implemented so far: INDEX_8 output, triangles, flat or Gouraud intensity,
 * z-buffered or z-sorted, plus INDEX_8 texture mapping (affine, perspective
 * correct and the per-triangle affine fallback). Anything else in the spec is a
 * compile error rather than a silently missing kernel.
 */
extern "C" {
#include "drv.h"
}

#include "softprim_axes.h"

#include <math.h>
#include <string.h>

namespace
{

/*
 * A screen-space vertex. x and y are SX and SY (pixel-centre coordinates, so
 * pixel i has centre i+0.5). z is C_SZ, which softrend scales to the signed
 * 16-bit range; i is the scaled colour index or intensity. Both are linear in
 * screen space and so interpolate affinely.
 */
struct ScreenVertex {
    float x;
    float y;
    float z;
    float i;
};

/*
 * Edge function, positive on one side of the directed line a->b.
 *
 * The triangle is made to have positive area (see IndexedTriangle), after which
 * a point is inside when all three edge functions are non-negative.
 */
inline float Edge(const ScreenVertex& a, const ScreenVertex& b, float px, float py) noexcept
{
    return (px - a.x) * (b.y - a.y) - (py - a.y) * (b.x - a.x);
}

/*
 * Top-left fill rule. With the Edge() sign convention and a positively wound
 * triangle, an edge is "top" when it is horizontal and runs right to left, and
 * "left" when it runs downwards. Pixels exactly on such an edge are considered
 * inside; pixels on every other edge are not. This makes two triangles that
 * share an edge cover each shared pixel exactly once.
 */
inline bool IsTopLeft(const ScreenVertex& a, const ScreenVertex& b) noexcept
{
    return (b.y == a.y && b.x < a.x) || (b.y > a.y);
}

inline bool Accepts(float w, bool topLeft) noexcept
{
    return w > 0.0f || (w == 0.0f && topLeft);
}

/*
 * Fold C_SZ (range -32767..32767, computed as -32767 * (ndc z)) into the
 * unsigned 16-bit depth buffer value pentprim uses: near maps to 0 and far to
 * 0xFFFF, matching the 0xFFFFFFFF the demos clear the depth buffer with.
 */
inline br_uint_16 DepthFromScalar(float z) noexcept
{
    float d = floorf(z) + 32768.0f;

    if(d < 0.0f)
        d = 0.0f;

    if(d > 65535.0f)
        d = 65535.0f;

    return (br_uint_16)d;
}

/*
 * Clamp a float pixel bound into [lo, hi]. Non-finite input clamps to lo, which
 * safely collapses the loop.
 */
inline int ClampIndex(float v, int lo, int hi) noexcept
{
    if(!(v >= (float)lo)) /* also catches NaN */
        return lo;

    if(v > (float)hi)
        return hi;

    return (int)v;
}

inline int FloorToInt(float v) noexcept
{
    return (int)floorf(v);
}

inline int CeilToInt(float v) noexcept
{
    return (int)ceilf(v);
}

/*
 * fabs for the long doubles the x87 setup carries. djgpp's libm exports no
 * _fabsl, so GCC's builtin is used there; it inlines to the x87 fabs instruction
 * and stays exact. MSVC has no such builtin, but it does have fabsl.
 */
inline long double FabsL(long double v) noexcept
{
#if defined(__GNUC__)
    return __builtin_fabsl(v);
#else
    return fabsl(v);
#endif
}

/*
 * Depth is stored natively, so copy the 16 bits in and out rather than aliasing
 * through a different type (which C++ does not allow).
 */
inline br_uint_16 LoadDepth(const br_uint_8 *p) noexcept
{
    br_uint_16 v;
    memcpy(&v, p, sizeof(v));
    return v;
}

inline void StoreDepth(br_uint_8 *p, br_uint_16 v) noexcept
{
    memcpy(p, &v, sizeof(v));
}

/*
 * The per-pixel read-modify-write ROPs, applied in pentprim's order: the
 * fragment (a texel, or a texel already looked up in the shade table) is
 * looked up in the fog table, then blended against the destination. Both are
 * "no requirement" when the template parameters say so, and the branches fold
 * away.
 *
 * Fog's index is (depth_high_byte << 8) | fragment and blend's is
 * (destination << 8) | fragment - see fti8pizp.asm's ScanlineRender_ZPTI macro
 * and zb8awtm.asm's DRAW_ZT_I8. Decal is not here: it is a second pass over
 * the triangle, not a lookup (see DecalTriangle).
 */
template <sp_blend B, sp_fog G>
inline br_uint_8 ApplyRops(br_uint_8 out, const br_uint_8 *dest, br_uint_8 zhi) noexcept
{
    if(G == SP_FOG_INDEX)
        out = SoftPrimWork.fog.base[((br_uint_32)zhi << 8) | out];

    if(B == SP_BLEND_INDEX)
        out = SoftPrimWork.blend.base[((br_uint_32)*dest << 8) | out];

    return out;
}

/* The high byte of a 16-bit depth value, which is the fog table's row. */
inline br_uint_8 DepthHighByte(br_uint_32 z) noexcept
{
    return (br_uint_8)(z >> 24);
}

/*
 * ---------------------------------------------------------------------------
 * Shared screen-space setup.
 *
 * Every INDEX_8 triangle kernel below - untextured, affine textured and
 * perspective textured - is driven by the same asm setup: pfpsetup.asm's
 * SETUP_FLOAT (the same macro fpsetup.asm defines for the untextured
 * rasterisers) for the trapezium geometry, and SETUP_FLOAT_PARAM for the
 * interpolated parameters. It is ported once, here, so the kernels cannot drift
 * apart.
 *
 * One thing pentprim splits differently and this port keeps: the x87 runs in
 * 80-bit extended precision. The conversions, the geometry and the parameter
 * gradients are computed in long double to match it, so exactness is specific
 * to targets where long double is 80-bit (see BuildTriSetup, BuildWalkGeom,
 * SetupFloatParam and the perspective setup below).
 * ---------------------------------------------------------------------------
 */

struct TexVert {
    br_fixed_ls x, y, z, u, v, w, i;
    float       rx, ry, rz, rw, ru, rv, ri;

    /* RGB output only: the per-vertex colour, and its integer form. */
    br_fixed_ls r, g, b;
    float       rr, rg, rb;

    /* The alpha the screendoor bodies scale their level from. */
    float ra;
};

inline void SwapTexVert(TexVert& a, TexVert& b) noexcept
{
    TexVert t = a;
    a         = b;
    b         = t;
}

/*
 * The vertices' sort key for SY. perspzi.h:78-96 compares `comp_x[C_SY]`, the
 * vertex union's alias of the float as a fixed word, so the comparison is on
 * the float's bits - which is what gsetuptf.asm's `cmp ecx,eax` on the raw
 * `[eax]._y` dword does too (one of the two is loaded with `mov`, the other
 * with `fld`, but both hold the float). Two vertices whose SY differs by less
 * than one 16.16 unit therefore compare equal as converted values and unequal
 * here, and that decides which of the two edges is taken as the major one.
 */
inline br_int_32 SortY(const TexVert& v) noexcept
{
    br_int_32 bits;

    memcpy(&bits, &v.ry, sizeof(bits));

    return bits;
}

/* asm `ror reg,16`: exchange the two 16-bit halves. */
inline br_int_32 Ror16(br_int_32 v) noexcept
{
    return (br_int_32)(((br_uint_32)v >> 16) | ((br_uint_32)v << 16));
}

/* `sar r,16`: the 16.16 integer part. */
inline br_int_32 Sar16(br_int_32 v) noexcept
{
    return v >> 16; /* sar eax,16 */
}

/*
 * The asm's `cmp r,0x80000000; adc r,-1` applied before a ror: for negative
 * values it subtracts a 16.16 unit, which is what makes the fraction carry the
 * trapezium walk reads out of the byte-swapped add come out right.
 */
inline br_int_32 NegAdjust(br_int_32 v) noexcept
{
    return (v < 0) ? v - 1 : v;
}

/*
 * The x87 magic-number conversions pfpsetup.asm performs by adding fp_conv_d16
 * / fp_conv_d and reading the low 32 bits of the stored double
 * (pfpsetup.asm:3133-3142): round to nearest even, the FPU's default mode.
 * The asm keeps the whole setup on the x87 stack in 80-bit extended precision,
 * so the arguments here are long double to match its rounding.
 */
inline br_int_32 Conv16(long double v) noexcept
{
    return (br_int_32)llrintl(v * 65536.0L);
}

/* fp_conv_d24 (pfpsetup.asm:2104): the same trick with 2^24, i.e. 8.24. The
 * arbitrary-width setup needs the fraction as its own quantity, and the macro
 * (ARBITRARY_SETUP below) strips the integer part back off. */
inline br_int_32 Conv24(long double v) noexcept
{
    return (br_int_32)llrintl(v * 16777216.0L);
}

/* fp_conv_d12 (setupdat.asm:40): 20.12, the MMX family's texture coordinate.
 * The same magic-number trick at a third fractional width. */
inline br_int_32 Conv12(long double v) noexcept
{
    return (br_int_32)llrintl(v * 4096.0L);
}

inline br_int_32 ConvInt(long double v) noexcept
{
    return (br_int_32)llrintl(v);
}

/* SETUP_FLOAT_PARAM's `conv` argument: fp_conv_d16 for z and i, fp_conv_d12 for
 * the MMX family's u/v and fp_conv_d24 for the arbitrary-width u/v and the MMX
 * family's r/g/b. */
template <int FRAC_BITS>
inline br_int_32 ConvParam(long double v) noexcept
{
    return (FRAC_BITS == 24) ? Conv24(v) : (FRAC_BITS == 12) ? Conv12(v) : Conv16(v);
}

/*
 * The asm's `and r,-stride` / `sub r,-stride`: the pixel-stride alignment
 * gsetuptf.asm applies to the trapezium's x origin and the major-edge gradient
 * before the parameter walk is set up. The stride is the block's pixel_stride
 * (1 for everything but the MMX family, which sets 4), and it is also the
 * boundary the major edge's carry is taken from - so a parameter that starts at
 * pixel p is the walk's origin and every stride pixels the walk advances one
 * block. For stride 1 the alignment is the identity.
 */
inline br_int_32 AlignStride(br_int_32 v, br_int_32 stride) noexcept
{
    return v & -stride;
}

/*
 * Asm-style trapezium geometry (pfpsetup.asm SETUP_FLOAT, shared by the
 * untextured, affine and perspective trapezium walks).
 *
 * The major edge xm and the two minor edge intercepts x1/x2 are 16.16 screen x
 * values; xm_f is the fractional accumulator whose carry selects the
 * per-scanline parameter increments. The asm nudges each intercept by a whole
 * pixel through the fconv_d16_m / fconv_d16_12 tables, indexed by the scan
 * direction: fconv_d16_m[dir] is +1 for dir 0 and +0 for dir 1, fconv_d16_12 is
 * the other way round, which is the `1 - dir` / `dir` below. Both are added
 * before the conversion to 16.16.
 */
struct WalkGeom {
    br_int_32 xm, d_xm, xm_f, d_xm_f;
    br_int_32 x1, d_x1, x2, d_x2;
    br_int_32 topCount, bottomCount;
    br_int_32 yTop;
};

inline WalkGeom BuildWalkGeom(const TexVert *v, br_int_32 scan_dir) noexcept
{
    WalkGeom g{};

    const long double xt = v[0].rx, yt = v[0].ry;
    const long double xm_ = v[1].rx, ym = v[1].ry;
    const long double xb = v[2].rx, yb = v[2].ry;

    const int yt_i = FloorToInt((float)yt), ym_i = FloorToInt((float)ym), yb_i = FloorToInt((float)yb);

    const long double t_dy = (long double)(yt_i + 1) - yt;
    const long double m_dy = (long double)(ym_i + 1) - ym;

    const long double g1 = (ym > yt) ? (xm_ - xt) / (ym - yt) : 0.0L;
    const long double gm = (yb > yt) ? (xb - xt) / (yb - yt) : 0.0L;
    const long double g2 = (yb > ym) ? (xb - xm_) / (yb - ym) : 0.0L;

    g.xm   = (br_int_32)llrintl((xt + t_dy * gm + (long double)(1 - scan_dir)) * 65536.0L);
    g.d_xm = (br_int_32)llrintl(gm * 65536.0L);
    g.x1   = (br_int_32)llrintl((xt + t_dy * g1 + (long double)scan_dir) * 65536.0L);
    g.d_x1 = (br_int_32)llrintl(g1 * 65536.0L);
    g.x2   = (br_int_32)llrintl((xm_ + m_dy * g2 + (long double)scan_dir) * 65536.0L);
    g.d_x2 = (br_int_32)llrintl(g2 * 65536.0L);

    g.xm_f   = g.xm << 16;
    g.d_xm_f = g.d_xm << 16;

    g.yTop        = yt_i;
    g.topCount    = (ym_i - yt_i) - 1;
    g.bottomCount = (yb_i - ym_i) - 1;

    return g;
}

/*
 * Whether an intensity shade axis interpolates the intensity (as opposed to
 * broadcasting vertex 0's constant). The untextured shade-table variants
 * interpolate or broadcast exactly like their plain counterparts; only what
 * the value is used for differs.
 */
inline bool IsInterpIntensity(sp_shade s) noexcept
{
    return s == SP_SHADE_INTERP_I || s == SP_SHADE_INTERP_I_TABLE;
}

inline bool IsShadeTable(sp_shade s) noexcept
{
    return s == SP_SHADE_CONST_I_TABLE || s == SP_SHADE_INTERP_I_TABLE;
}

/*
 * Broadcast the per-vertex intensity the asm expects.
 *
 * The unlit blocks have no intensity component at all, and the constant ones
 * carry it in vertex 0's slot only (fpsetup.asm SETUP_FLOAT_CONST and
 * SETUP_FLOAT_COLOUR read that slot). Both triangle kernels need the same
 * treatment, so it is applied once, before anything else looks at the value.
 */
inline void ApplyIntensity(TexVert v[3], sp_shade s, bool interp) noexcept
{
    if(s == SP_SHADE_NONE) {
        v[0].i = v[1].i = v[2].i = 0;
        v[0].ri = v[1].ri = v[2].ri = 0.0f;
    } else if(!interp) {
        v[1].i = v[2].i = v[0].i;
        v[1].ri = v[2].ri = v[0].ri;
    }
}

/*
 * One interpolated parameter in the trapezium form fti8pizp.asm walks it.
 *
 * current is the value at the first pixel of the current scanline, grad_x the
 * per-pixel step, and d_nocarry/d_carry the two possible per-scanline steps
 * selected by the carry out of the major edge's x step. The projective
 * numerators pu/pv leave d_carry unused - the walk adds grad_x on a carry
 * instead, which is the same thing because the setup writes d_carry equal to
 * d_nocarry for them.
 *
 * For pu/pv/pq there is no binary point: pfpsetup.asm converts them with
 * fp_conv_d, so they are plain integers scaled by the maxuv normalisation and
 * only the numerator/denominator ratio is meaningful. pz and pi are 16.16
 * (fp_conv_d16).
 */
struct TrapParam {
    br_int_32 current;
    br_int_32 grad_x;
    br_int_32 d_nocarry;
    br_int_32 d_carry;
};

/*
 * Screen-space setup shared by the INDEX_8 triangle kernels.
 *
 * This is pfpsetup.asm's SETUP_FLOAT - the same macro fpsetup.asm defines for
 * the untextured rasterisers: the vertices are sorted by SY, the scan direction
 * is the sort parity folded with the sign of the original winding, and the
 * trapezium geometry and the parameter start offsets fall out of it. Both
 * kernels go through here so their geometry cannot drift apart.
 *
 * `in` carries whatever per-vertex values the kernel needs and is copied to
 * `orig` (the order SETUP_FLOAT_PARAM takes its deltas in) before `v` is sorted
 * in place. Returns false for a triangle the setup cannot process.
 *
 * ia_sorted is the single-precision reciprocal workspace_iarea reloads in
 * SETUP_FLOAT_UV_PERSPECTIVE; odx1a..dy2a are the single-precision spills
 * SETUP_FLOAT makes for SETUP_FLOAT_PARAM. Both are kept apart because the
 * perspective macro rescales the same names from the rounded reciprocal in the
 * sorted order.
 */
struct TriSetup {
    TexVert     v[3];
    TexVert     orig[3];
    br_int_32   asm_dir;
    br_int_32   flip;
    WalkGeom    walk;
    long double t_dx;
    long double t_dy;
    long double xstep_0;
    long double xstep_1;
    long double ia_sorted;
    long double odx1a, ody1a, odx2a, ody2a;
    long double sdx1a, sdy1a, sdx2a, sdy2a;
};

inline bool BuildTriSetup(const TexVert in[3], TriSetup& s, br_int_32 stride = 1) noexcept
{
    for(int k = 0; k < 3; k++) {
        s.orig[k] = in[k];
        s.v[k]    = in[k];
    }

    /*
     * Vertex sort by SY, exactly as perspzi.h:78-96 (see SortY). The parity of
     * the permutation is tracked because the asm's scan-direction flag is the
     * sort parity folded with the sign of the original winding (flip_table in
     * fpsetup.asm is that parity).
     */
    bool flip = false;

    if(SortY(s.v[0]) > SortY(s.v[1])) {
        if(SortY(s.v[1]) > SortY(s.v[2])) {
            SwapTexVert(s.v[0], s.v[2]);
            flip = !flip;
        } else {
            if(SortY(s.v[0]) > SortY(s.v[2])) {
                SwapTexVert(s.v[0], s.v[1]);
                SwapTexVert(s.v[1], s.v[2]);
            } else {
                SwapTexVert(s.v[0], s.v[1]);
                flip = !flip;
            }
        }
    } else {
        if(SortY(s.v[1]) > SortY(s.v[2])) {
            if(SortY(s.v[0]) > SortY(s.v[2])) {
                SwapTexVert(s.v[1], s.v[2]);
                SwapTexVert(s.v[0], s.v[1]);
            } else {
                SwapTexVert(s.v[1], s.v[2]);
                flip = !flip;
            }
        }
    }

    /* The asm bails out of an empty triangle before touching anything else. */
    if(Sar16(s.v[0].y) == Sar16(s.v[2].y))
        return false;

    const long double ox0 = s.orig[0].rx, oy0 = s.orig[0].ry;
    const long double ox1 = s.orig[1].rx, oy1 = s.orig[1].ry;
    const long double ox2 = s.orig[2].rx, oy2 = s.orig[2].ry;

    const long double area_orig = (ox1 - ox0) * (oy2 - oy0) - (ox2 - ox0) * (oy1 - oy0);

    if(area_orig == 0.0L)
        return false;

    /*
     * workspace_iarea is a dword, so 1/(2*area) is rounded to single
     * precision. SETUP_FLOAT_PARAM runs before SETUP_FLOAT_UV_PERSPECTIVE and
     * uses the products of the extended reciprocal; the perspective macro then
     * reloads the rounded reciprocal and rescales the whole set in the sorted
     * vertex order. The two are kept apart below.
     */
    const long double ia_orig = 1.0L / area_orig;
    const long double iarea_f = (float)ia_orig;

    s.ia_sorted = flip ? -iarea_f : iarea_f;

    s.odx1a = (float)((ox1 - ox0) * ia_orig);
    s.ody1a = (float)((oy1 - oy0) * ia_orig);
    s.odx2a = (float)((ox2 - ox0) * ia_orig);
    s.ody2a = (float)((oy2 - oy0) * ia_orig);

    /*
     * The scan-direction flag the asm's trapezium walk runs on
     * (work.tsl.direction): the sort parity XOR the sign of the original
     * winding. Every trapezium walk below is driven by this one value.
     */
    const float area_orig_f = (s.orig[1].rx - s.orig[0].rx) * (s.orig[2].ry - s.orig[0].ry) -
                              (s.orig[2].rx - s.orig[0].rx) * (s.orig[1].ry - s.orig[0].ry);

    s.asm_dir = (flip ? 1 : 0) ^ ((area_orig_f < 0.0f) ? 1 : 0);

    /*
     * The affine blocks pick the scan direction once for the whole triangle and
     * read it from workspace.flip (zb8p2unl.asm:238); a plain affine block
     * leaves that holding the setup's sort parity, and the perspective blocks'
     * cheat replaces it with work.tsl.direction (pfpsetup.asm:2669), which also
     * folds the winding in. softprim carries both, so the affine walk can take
     * the direction from ts.flip for the first case and ts.asm_dir for the
     * second. ts.flip is the complement of the asm's workspace.flip (measured
     * against pentprim, which is what fixes the polarity below).
     */
    s.flip = flip ? 1 : 0;

    /* SETUP_FLOAT_UV_PERSPECTIVE rescales the deltas in the sorted order. */
    const long double sx0 = s.v[0].rx, sy0 = s.v[0].ry;
    const long double sx1 = s.v[1].rx, sy1 = s.v[1].ry;
    const long double sx2 = s.v[2].rx, sy2 = s.v[2].ry;

    s.sdx1a = (float)((sx1 - sx0) * s.ia_sorted);
    s.sdy1a = (float)((sy1 - sy0) * s.ia_sorted);
    s.sdx2a = (float)((sx2 - sx0) * s.ia_sorted);
    s.sdy2a = (float)((sy2 - sy0) * s.ia_sorted);

    s.walk = BuildWalkGeom(s.v, s.asm_dir);

    /*
     * t_dx and t_dy live in dword slots in the asm, so each is rounded to
     * single precision where it is stored; xstep_0/xstep_1 are the integer part
     * of the major-edge gradient and are exact. The first sample is the major
     * edge's rounded integer column, aligned down to the pixel-stride boundary
     * gsetuptf.asm truncates to (for the MMX family that is the four-pixel word
     * the parameters are kept per, so t_dx is the origin of the parameter walk,
     * not the edge itself; for stride 1 it is the edge).
     */
    s.t_dy    = (float)((long double)(FloorToInt((float)s.v[0].ry) + 1) - s.v[0].ry);
    s.t_dx    = (float)((long double)AlignStride(s.walk.xm >> 16, stride) - (long double)s.v[0].rx);
    s.xstep_0 = (float)(long double)AlignStride(s.walk.d_xm >> 16, stride);
    s.xstep_1 = (float)(long double)(AlignStride(s.walk.d_xm >> 16, stride) + stride);

    return true;
}

/*
 * SETUP_FLOAT_PARAM (pfpsetup.asm:596): the affine 16.16 gradient of one
 * parameter over the triangle, the two per-scanline increments selected by the
 * major edge's carry, and the value at the first sampled pixel. The deltas are
 * taken against the original vertex 0 and the start is at the sorted top
 * vertex. `unsign` is the macro's unsigned argument, which remaps the signed
 * ramp onto the unsigned depth buffer.
 *
 * FRAC_BITS is the macro's `conv` argument. z and i convert with fp_conv_d16
 * (16.16); the arbitrary-width u/v and the MMX family's r/g/b convert with
 * fp_conv_d24 (8.24); the MMX family's u/v convert with fp_conv_d12 (20.12).
 */
template <int FRAC_BITS = 16>
inline void SetupFloatParam(TrapParam& p, const TriSetup& s, long double p0, long double p1, long double p2, long double ptop, bool unsign) noexcept
{
    const long double dp1 = p1 - p0;
    const long double dp2 = p2 - p0;
    const long double pdx = dp1 * s.ody2a - dp2 * s.ody1a;
    const long double pdy = dp2 * s.odx1a - dp1 * s.odx2a;

    p.grad_x    = ConvParam<FRAC_BITS>(pdx);
    p.d_nocarry = ConvParam<FRAC_BITS>(pdy + s.xstep_0 * pdx);
    p.d_carry   = ConvParam<FRAC_BITS>(pdy + s.xstep_1 * pdx);
    p.current   = ConvParam<FRAC_BITS>(ptop + pdx * s.t_dx + pdy * s.t_dy);

    if(unsign)
        p.current ^= (br_int_32)0x80000000;
}

enum {
    RPD_FWD = 0,
    RPD_BWD = 1
};

/*
 * The untextured INDEX_8 triangle.
 *
 * zb8.asm's DRAW_Z_I8_D16 / TriangleRender_Z_I8_D16 for the constant-shade
 * z-buffered blocks and fti8_piz.asm's TRAPEZIUM_ZI_I8_D16 /
 * TriangleRender_ZI_I8_D16 for the interpolated one; zs8.asm's DRAW_I8 /
 * TriangleRender_I8 and DRAW_I_I8 / TriangleRender_I_I8 for their z-sorted
 * counterparts. All four use the same trapezium geometry and the same
 * SETUP_FLOAT_PARAM z ramp as the textured kernels; only what the span emits
 * differs.
 *
 * The flat blocks carry one colour byte in vertex 0's intensity slot, which
 * fpsetup.asm SETUP_FLOAT_COLOUR rounds with fp_conv_d16 and reads back as the
 * integer low byte. The interpolated block stores the same byte of its
 * interpolated 16.16 value per pixel. SP_SHADE_NONE has no block in the .ifg
 * tables and the matcher never selects it; it is treated as a constant 0 so the
 * axis still has a kernel.
 */
struct IndexedWalk {
    TrapParam  pz;
    TrapParam  pi;
    br_int_32  flat;
    br_uint_8 *scan;  /* colour row start - 1 */
    br_uint_8 *zscan; /* depth row start - 2, unused without depth */
    br_int_32  main_i, main_d;
    br_uint_32 main_f, main_d_f;
    br_int_32  minor_i, minor_d;
    br_int_32  minor_count;
};

/*
 * One scanline: DRAW_Z_I8_D16 / DRAW_ZI_I8_D16's pixel loop (DRAW_I8 /
 * DRAW_I_I8 when there is no depth buffer). Depth, when present, is tested as
 * newz <= oldz and written before the colour.
 *
 * The asm keeps z and i byte-swapped so that the carry out of the add is the
 * fraction overflow it folds back into the integer part with adc; ordinary
 * 16.16 arithmetic gives the same integer part, so these are plain 16.16.
 */
template <sp_depth D, sp_shade S, sp_fog G, int DIRN>
void IndexedSpan(IndexedWalk& w, br_uint_8 *c, br_uint_8 *end, br_uint_8 *z) noexcept
{
    br_int_32 zz = w.pz.current;
    br_int_32 ii = w.pi.current;

    for(;;) {
        bool       draw = true;
        br_uint_16 newz = 0;

        if(D == SP_DEPTH_ZW) {
            newz = (br_uint_16)((br_uint_32)zz >> 16);

            if(newz > LoadDepth(z))
                draw = false;
        }

        if(draw) {
            if(D == SP_DEPTH_ZW)
                StoreDepth(z, newz);

            br_uint_8 out;

            if(IsShadeTable(S)) {
                /*
                 * The untextured Indexed Shading blocks look the intensity up
                 * in the shade table with index_base as the texel column
                 * (zb8sh.asm:47-66, fpsetup.asm's SETUP_FLOAT_COLOUR_SHADETABLE).
                 */
                const br_uint_8 idx = IsInterpIntensity(S) ? (br_uint_8)((br_uint_32)ii >> 16) : (br_uint_8)w.flat;

                out = SoftPrimWork.shade.base[((br_uint_32)idx << 8) | ((br_uint_32)SoftPrimWork.index_base & 0xff)];
            } else {
                out = IsInterpIntensity(S) ? (br_uint_8)((br_uint_32)ii >> 16) : (br_uint_8)w.flat;
            }

            *c = ApplyRops<SP_BLEND_NONE, G>(out, c, (br_uint_8)(newz >> 8));
        }

        if(DIRN == RPD_FWD) {
            c++;

            if(c > end)
                break;

            if(D == SP_DEPTH_ZW) {
                z += 2;
                zz += w.pz.grad_x;
            }

            if(IsInterpIntensity(S))
                ii += w.pi.grad_x;
        } else {
            c--;

            if(c < end)
                break;

            if(D == SP_DEPTH_ZW) {
                z -= 2;
                zz -= w.pz.grad_x;
            }

            if(IsInterpIntensity(S))
                ii -= w.pi.grad_x;
        }
    }
}

/*
 * One trapezium of an untextured triangle: the scanline loop from the asm's
 * TrapeziumRender, shared shape with the textured walks (major edge stepped to
 * the minor edge, the major-edge fraction carry selecting the per-scanline
 * increments). The two halves of a triangle call this in turn, carrying the
 * state over.
 */
template <sp_depth D, sp_shade S, sp_fog G, int DIRN>
void IndexedTrapezium(IndexedWalk& w, const softprim_buffer& colour, const softprim_buffer& depth) noexcept
{
    for(br_int_32 n = w.minor_count; n >= 0; n--) {
        const br_int_32 main_int  = (br_int_32)((br_uint_32)w.main_i >> 16);
        const br_int_32 minor_int = (br_int_32)((br_uint_32)w.minor_i >> 16);
        const bool      draw      = (DIRN == RPD_FWD) ? (main_int <= minor_int) : (main_int >= minor_int);

        if(draw)
            IndexedSpan<D, S, G, DIRN>(w, w.scan + main_int, w.scan + minor_int, w.zscan + (br_size_t)main_int * 2);

        w.main_f += w.main_d_f;

        const br_uint_32 carry = (w.main_f < w.main_d_f) ? 1u : 0u;

        w.main_i += w.main_d;
        w.minor_i += w.minor_d;

        w.pz.current += (carry != 0u) ? w.pz.d_carry : w.pz.d_nocarry;
        w.pi.current += (carry != 0u) ? w.pi.d_carry : w.pi.d_nocarry;

        w.scan += colour.stride_b;

        if(D == SP_DEPTH_ZW)
            w.zscan += depth.stride_b;
    }
}

/*
 * One untextured INDEX_8 triangle.
 *
 * D selects depth test/write and S the shade source; both are template
 * parameters so the branches fold away and the variants share one source.
 */
template <sp_depth D, sp_shade S, sp_fog G>
void IndexedTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2)
{
    (void)block;

    const softprim_buffer& colour = SoftPrimWork.colour;
    const softprim_buffer& depth  = SoftPrimWork.depth;

    /*
     * Refuse formats the span loop does not understand rather than corrupting
     * memory. match.c should already have rejected these.
     */
    if(colour.type != BR_PMT_INDEX_8 || colour.base == NULL || colour.width_p <= 0 || colour.height <= 0)
        return;

    if(D == SP_DEPTH_ZW && (depth.type != BR_PMT_DEPTH_16 || depth.base == NULL))
        return;

    if(G == SP_FOG_INDEX && SoftPrimWork.fog.base == NULL)
        return;

    if(IsShadeTable(S) && (SoftPrimWork.shade.base == NULL || SoftPrimWork.shade.type != BR_PMT_INDEX_8))
        return;

    const bool interpolated = IsInterpIntensity(S);

    brp_vertex *src[3] = {v0, v1, v2};
    TexVert     in[3];

    for(int k = 0; k < 3; k++) {
        in[k].x  = BrFloatToFixed(src[k]->comp[C_SX]);
        in[k].y  = BrFloatToFixed(src[k]->comp[C_SY]);
        in[k].z  = BrFloatToFixed(src[k]->comp[C_SZ]);
        in[k].i  = BrFloatToFixed(src[k]->comp[C_I]);
        in[k].rx = src[k]->comp[C_SX];
        in[k].ry = src[k]->comp[C_SY];
        in[k].rz = src[k]->comp[C_SZ];
        in[k].ri = src[k]->comp[C_I];

        if(!isfinite(in[k].rx) || !isfinite(in[k].ry))
            return;
    }

    ApplyIntensity(in, S, interpolated);

    TriSetup ts;

    if(!BuildTriSetup(in, ts))
        return;

    IndexedWalk w;

    SetupFloatParam(w.pz, ts, ts.orig[0].rz, ts.orig[1].rz, ts.orig[2].rz, ts.v[0].rz, true);

    if(interpolated)
        SetupFloatParam(w.pi, ts, ts.orig[0].ri, ts.orig[1].ri, ts.orig[2].ri, ts.v[0].ri, false);
    else
        w.pi = TrapParam{};

    /* SETUP_FLOAT_COLOUR: vertex 0's rounded intensity, integer low byte. */
    w.flat = (br_int_32)(br_uint_8)((br_uint_32)Conv16(ts.orig[0].ri) >> 16);

    w.main_i   = ts.walk.xm;
    w.main_d   = ts.walk.d_xm;
    w.main_f   = (br_uint_32)ts.walk.xm_f;
    w.main_d_f = (br_uint_32)ts.walk.d_xm_f;

    w.scan  = colour.base + (br_size_t)ts.walk.yTop * colour.stride_b - 1;
    w.zscan = ((D == SP_DEPTH_ZW) ? depth.base : colour.base) + (br_size_t)ts.walk.yTop * ((D == SP_DEPTH_ZW) ? depth.stride_b : 0) - 2;

    /* Top trapezium: the row below the top vertex up to the middle one. */
    w.minor_i     = ts.walk.x1;
    w.minor_d     = ts.walk.d_x1;
    w.minor_count = ts.walk.topCount;

    if(ts.asm_dir == 0)
        IndexedTrapezium<D, S, G, RPD_FWD>(w, colour, depth);
    else
        IndexedTrapezium<D, S, G, RPD_BWD>(w, colour, depth);

    /* Bottom trapezium: the row below the middle vertex to the bottom one. */
    w.minor_i     = ts.walk.x2;
    w.minor_d     = ts.walk.d_x2;
    w.minor_count = ts.walk.bottomCount;

    if(ts.asm_dir == 0)
        IndexedTrapezium<D, S, G, RPD_FWD>(w, colour, depth);
    else
        IndexedTrapezium<D, S, G, RPD_BWD>(w, colour, depth);
}

/*
 * ---------------------------------------------------------------------------
 * Perspective-correct INDEX_8 texture mapping.
 *
 * pfpsetup.asm TriangleSetup_ZPTI's SETUP_FLOAT_UV_PERSPECTIVE for the
 * projective parameters, then fti8pizp.asm's TriangleRasterise_ZPTI_I8_D16 and
 * its TrapeziumRender / ScanlineRender pair for the walk: the rounded base
 * texel, the q'i = wj*wk projective set, the power-of-two maxuv normalisation,
 * the numerators that carry no binary point, the per-scanline PDIVIDE with its
 * grad_x/d_nocarry corrections, and the five numerator-adjustment variants.
 *
 * One deliberate divergence, required by the design: map dimensions are
 * runtime. pentprim compiles a separate kernel per power-of-two size
 * (texture8x8 .. texture1024x1024); here the shift and mask come from the bound
 * map, so one SP_ADDR_SHIFT kernel serves every size.
 * ---------------------------------------------------------------------------
 */

/*
 * The per-triangle perspective walk state, built by PerspSetup from
 * pfpsetup.asm SETUP_FLOAT + SETUP_FLOAT_UV_PERSPECTIVE and then driven by
 * fti8pizp.asm TrapeziumRender_ZPTI_I8_D16.
 *
 * main_i/main_d are held in the byte-swapped form the asm keeps them in, with
 * the integer part in the low 16 bits, so the carry out of the 32-bit add that
 * steps the major edge is the fraction overflow that selects the d_carry
 * increment. minor_i/minor_d are ordinary 16.16 (the top then bottom minor
 * edge); the walk is called once per trapezium with them reloaded.
 *
 * dz/di are the pz/pi x gradients after the asm's negative-value correction.
 * The span's z and i start from pz.current/pi.current each scanline, which is
 * what the asm reloads from work_pz_current/work_pi_current per scanline - they
 * are not snapshots taken once per triangle.
 */
struct PerspWalk {
    TrapParam pu, pv, pq, pz, pi;

    br_uint_32 main_i, main_d;
    br_int_32  minor_i, minor_d;
    br_int_32  minor_count;

    br_uint_8 *scan;  /* colour row start - 1, the asm's scanAddress */
    br_uint_8 *zscan; /* depth row start - 2 */
    br_uint_32 source;

    br_int_32 dz;
    br_int_32 di;
    br_int_32 ddenominator; /* pq.grad_x */

    br_int_32 direction;
};

/* incu/decu/incv/decv (perspzi.h:27-35) generalised to a runtime map size. */
inline br_int_32 IncU(br_int_32 a, br_int_32 mask) noexcept
{
    return ((a + 1) & mask) | (a & ~mask);
}

inline br_int_32 DecU(br_int_32 a, br_int_32 mask) noexcept
{
    return ((a - 1) & mask) | (a & ~mask);
}

inline br_int_32 IncV(br_int_32 a, br_int_32 vshift, br_int_32 maskh) noexcept
{
    br_int_32 u = a & ((1 << vshift) - 1);
    br_int_32 v = (a >> vshift);

    v = (v + 1) & maskh;

    return (v << vshift) | u;
}

inline br_int_32 DecV(br_int_32 a, br_int_32 vshift, br_int_32 maskh) noexcept
{
    br_int_32 u = a & ((1 << vshift) - 1);
    br_int_32 v = (a >> vshift);

    v = (v - 1) & maskh;

    return (v << vshift) | u;
}

/*
 * One affine INDEX_8 span, ported from zb8p2lit.asm DRAW_ZTI_I8_D16_POW2.
 *
 * u, v, i and z step by a constant per pixel; the texel address is rebuilt from
 * the integer parts each pixel (the pow2 mask wraps both axes). The same code
 * serves every map size - the shift and mask come from the bound map rather
 * than a per-size instantiation.
 *
 * PACKED is fti8_piz.asm TRAPEZIUM_ZTF_I8_D16_256's coordinate packing, which
 * is a different representation of the same two coordinates rather than a
 * different geometry: the macro keeps each of u and v as an eight-bit integer
 * and a twelve-bit fraction instead of 16.16, so that the fog lookup has a
 * register to live in, and it therefore picks a neighbouring texel at the
 * boundaries the full walk lands on the other side of. One pentprim entry is
 * built on that macro - the texture-only power-of-two fog block,
 * TriangleRender_ZTF_I8_D16_256 - while every other affine INDEX_8 kernel,
 * including the same file's unfogged TriangleRender_ZT_I8_D16_256 and zb8.asm's
 * interpolated TriangleRender_ZTIF_I8_D16_256, is the full walk above. It is a
 * template parameter rather than a fold for the reason the perfect-scan family
 * is ported separately: a block that samples different pixels gets its own walk,
 * and folding it would move every kernel that shares this one.
 */
template <sp_depth D, sp_shade S, sp_blend B, sp_fog G, bool PACKED = false>
void AffineDrawSpan(const softprim_buffer& texture, const softprim_buffer& shade, br_uint_8 *start, br_uint_8 *end, br_uint_8 *zstart,
                    br_int_32 u, br_int_32 ux, br_int_32 v, br_int_32 vx, br_int_32 iv, br_int_32 ix, br_int_32 z, br_int_32 zx,
                    br_int_32 mask_w, br_int_32 mask_h, br_int_32 vshift, bool l2r) noexcept
{
    const br_uint_8 *tex  = texture.base;
    const br_uint_8 *stbl = shade.base;

    /*
     * The macro's setup packing (fti8_piz.asm:1225-1250), verbatim except for
     * the last truncation: u's step loses one more unit than its truncation to
     * 2^-12 when v's gradient is negative, because the macro compares edx and
     * then adjusts ecx, and that is what it does rather than what it reads as
     * meaning. `packed` is ebx - v's twelve fraction bits at 0..11, v's eight
     * integer bits at 12..19 and u's twelve fraction bits at 20..31 - and
     * `packed_u` is eax's low byte, u's eight integer bits. The three fields
     * being adjacent is the point: a carry out of v's fraction steps v's
     * integer, and a carry out of u's fraction is the carry out of the 32-bit
     * add the macro folds into u's integer with `adc al,dl`.
     */
    br_uint_32 packed = 0, packed_d = 0, packed_u = 0, packed_du = 0;

    if(PACKED) {
        ux = (ux + ((vx < 0) ? -1 : 0)) & ~15;
        vx &= ~15;

        packed    = ((((br_uint_32)u >> 4) & 0xfff) << 20) | ((((br_uint_32)v >> 16) & 0xff) << 12) | (((br_uint_32)v >> 4) & 0xfff);
        packed_d  = ((((br_uint_32)ux >> 4) & 0xfff) << 20) | ((((br_uint_32)vx >> 16) & 0xff) << 12) | (((br_uint_32)vx >> 4) & 0xfff);
        packed_u  = ((br_uint_32)u >> 16) & 0xff;
        packed_du = ((br_uint_32)ux >> 16) & 0xff;
    }

    /*
     * The macro steps the packed word and then folds its carry into u's integer
     * with `adc al,dl`, and its backward instantiation is `sub ebx,ecx` /
     * `sbb al,dl`: the same packed step subtracted, not the packed form of a
     * negated one, which the truncation makes different.
     */
    const auto step_fwd = [&]() noexcept {
        if(PACKED) {
            const br_uint_32 sum = packed + packed_d;
            const br_uint_32 cf  = (sum < packed) ? 1u : 0u;

            packed   = sum;
            packed_u = (packed_u + packed_du + cf) & 0xff;
        } else {
            u += ux;
            v += vx;
        }
    };

    const auto step_bwd = [&]() noexcept {
        if(PACKED) {
            const br_uint_32 cf = (packed < packed_d) ? 1u : 0u;

            packed   = packed - packed_d;
            packed_u = (packed_u - packed_du - cf) & 0xff;
        } else {
            u -= ux;
            v -= vx;
        }
    };

    for(;;) {
        const br_int_32 source = PACKED ? (br_int_32)(((packed >> 12) & (br_uint_32)mask_h) << vshift) | (br_int_32)(packed_u & (br_uint_32)mask_w)
                                        : ((((br_uint_32)v >> 16) & mask_h) << vshift) | (((br_uint_32)u >> 16) & mask_w);
        const br_uint_8 texel = tex[source];
        bool            draw  = (texel != 0);
        br_uint_16      newz  = 0;

        if(D == SP_DEPTH_ZW) {
            newz = (br_uint_16)((br_uint_32)z >> 16);

            if(newz > LoadDepth(zstart))
                draw = false;
        }

        if(draw) {
            br_uint_8 out = texel;

            if(S != SP_SHADE_NONE)
                out = stbl[(((br_uint_32)iv >> 16) & 0xff) * 256 + texel];

            if(D == SP_DEPTH_ZW)
                StoreDepth(zstart, newz);

            *start = ApplyRops<B, G>(out, start, (br_uint_8)(newz >> 8));
        }

        if(l2r) {
            if(start >= end)
                break;

            start++;
            zstart += 2;
            step_fwd();
            iv += ix;
            z += zx;
        } else {
            if(start <= end)
                break;

            start--;
            zstart -= 2;
            step_bwd();
            iv -= ix;
            z -= zx;
        }
    }
}

/*
 * The asm round-up-to-a-power-of-two normalisation of maxuv
 * (pfpsetup.asm:1163-1183). It is done on the float bit pattern: clear the
 * mantissa and bump the exponent if any mantissa bits are set, then form
 * 2^(28-exponent) by negating the bits and adding the doubled bias plus 28.
 */
constexpr br_uint_32 PERSP_EXPONENT_BIAS = 127;

inline float PerspMaxuvNorm(float maxuv) noexcept
{
    br_uint_32 bits;

    memcpy(&bits, &maxuv, sizeof(bits));

    if((bits & 0x7fffffu) != 0u) {
        bits &= ~0x7fffffu;
        bits += 1u << 23;
    }

    br_uint_32 out = (br_uint_32)(-(br_int_32)bits) + ((PERSP_EXPONENT_BIAS * 2 + 28) << 23);
    float      result;

    memcpy(&result, &out, sizeof(result));

    return result;
}

/*
 * PDIVIDE in TrapeziumRender_ZPTI_I8_D16: pull each projective numerator back
 * into [0, pq.current) before the span, stepping the texel source by whole
 * texels as it goes and carrying the correction into the per-pixel steps.
 * Only the positive direction runs one of the two numerator loops per call.
 */
inline void PerspScanlineAdjust(PerspWalk& w, br_int_32 mask_w, br_int_32 mask_h, br_int_32 vshift) noexcept
{
    const br_int_32 qcur = w.pq.current;
    const br_int_32 qgx  = w.pq.grad_x;
    const br_int_32 qdn  = w.pq.d_nocarry;

    br_int_32 un = w.pu.current, ugx = w.pu.grad_x, udn = w.pu.d_nocarry;

    if(un >= qcur) {
        if(qcur > 0) {
            do {
                w.source = (br_uint_32)IncU((br_int_32)w.source, mask_w);
                ugx -= qgx;
                udn -= qdn;
                un -= qcur;
            } while(un >= qcur);
        }
    } else if(un < 0) {
        do {
            w.source = (br_uint_32)DecU((br_int_32)w.source, mask_w);
            ugx += qgx;
            udn += qdn;
            un += qcur;
        } while(un < 0);
    }

    w.pu.current   = un;
    w.pu.grad_x    = ugx;
    w.pu.d_nocarry = udn;

    br_int_32 vn = w.pv.current, vgx = w.pv.grad_x, vdn = w.pv.d_nocarry;

    if(vn >= qcur) {
        if(qcur > 0) {
            do {
                w.source = (br_uint_32)IncV((br_int_32)w.source, vshift, mask_h);
                vgx -= qgx;
                vdn -= qdn;
                vn -= qcur;
            } while(vn >= qcur);
        }
    } else if(vn < 0) {
        do {
            w.source = (br_uint_32)DecV((br_int_32)w.source, vshift, mask_h);
            vgx += qgx;
            vdn += qdn;
            vn += qcur;
        } while(vn < 0);
    }

    w.pv.current   = vn;
    w.pv.grad_x    = vgx;
    w.pv.d_nocarry = vdn;
}

/*
 * The numerator-adjustment variants fti8pizp.asm selects between. INC and DEC
 * run one correction loop and move the numerator into [-den,0) first; BOTH
 * runs whichever loop applies. RPD_FWD/BWD are the forward and backward
 * scanline instantiations of the whole span.
 */
enum {
    RPU_INC  = 0,
    RPU_DEC  = 1,
    RPU_BOTH = 2
};
/*
 * ScanlineRender_ZPTI_I8_D16 (z-buffered) / ScanlineRender_ZPT_I8_D16
 * (z-sorted): one scanline of a perspective textured INDEX_8 span.
 *
 * Texel 0 is transparent (TRANSPARENCY=1) and is tested on the raw texel; a
 * surviving texel is looked up as shade[(I << 8) | texel], depth is tested as
 * newz <= oldz and written, and the texel address is advanced by the
 * incremental division of the projective numerator against q.
 */
template <sp_depth D, sp_shade S, sp_blend B, sp_fog G, int UD, int VD, int DIRN>
void PerspSpan(PerspWalk& w, const softprim_buffer& texture, const softprim_buffer& shade, br_int_32 mask_w, br_int_32 mask_h, br_int_32 vshift) noexcept
{
    const br_uint_8 *tex  = texture.base;
    const br_uint_8 *stbl = shade.base;

    br_int_32 den    = w.pq.current;
    br_int_32 dden   = w.ddenominator;
    br_int_32 u_num  = w.pu.current;
    br_int_32 du_num = w.pu.grad_x;
    br_int_32 v_num  = w.pv.current;
    br_int_32 dv_num = w.pv.grad_x;

    if(UD == RPU_INC) {
        u_num -= den;
        du_num -= dden;
    }

    if(VD == RPU_INC) {
        v_num -= den;
        dv_num -= dden;
    }

    br_int_32  source = (br_int_32)w.source;

    br_uint_8 *c   = w.scan + (br_int_32)(w.main_i & 0xffffu);
    br_uint_8 *end = w.scan + (br_uint_32)(w.minor_i) / 65536u;
    br_uint_8 *z   = w.zscan + (br_int_32)(w.main_i & 0xffffu) * 2;
    br_int_32  iv     = w.pi.current;
    br_int_32  zz     = w.pz.current;

    for(;;) {
        br_uint_8  texel = tex[source];
        bool       draw  = (texel != 0);
        br_uint_16 newz  = 0;

        if(D == SP_DEPTH_ZW) {
            newz = (br_uint_16)((br_uint_32)zz >> 16);

            if(newz > LoadDepth(z))
                draw = false;
        }

        if(draw) {
            br_uint_8 out = texel;

            if(S != SP_SHADE_NONE)
                out = stbl[(((br_uint_32)iv >> 16) & 0xff) * 256 + texel];

            if(D == SP_DEPTH_ZW)
                StoreDepth(z, newz);

            *c = ApplyRops<B, G>(out, c, (br_uint_8)(newz >> 8));
        }

        if(DIRN == RPD_FWD) {
            c++;
            z += 2;

            if(c > end)
                break;

            zz += w.dz;
            iv += w.di;
            den += dden;
            u_num += du_num;
        } else {
            c--;
            z -= 2;

            if(c < end)
                break;

            zz -= w.dz;
            iv -= w.di;
            den -= dden;
            u_num -= du_num;
        }

        if(UD == RPU_BOTH) {
            if(u_num < 0) {
                do {
                    source = DecU(source, mask_w);
                    du_num += dden;
                    u_num += den;
                } while(u_num < 0);

            } else if(u_num >= den) {
                do {
                    source = IncU(source, mask_w);
                    du_num -= dden;
                    u_num -= den;
                } while(u_num >= den);
            }
        } else if(UD == RPU_INC) {
            /*
             * The increasing variant's loop ends in a bare `jge`, so it runs
             * while the numerator is still >= 0; there is no cmp against den.
             */
            if(u_num >= 0) {
                do {
                    source = IncU(source, mask_w);
                    du_num -= dden;
                    u_num -= den;
                } while(u_num >= 0);
            }
        } else {
            if(u_num < 0) {
                do {
                    source = DecU(source, mask_w);
                    du_num += dden;
                    const br_uint_32 before = (br_uint_32)u_num;

                    u_num += den;

                    /* `jnc` loops while the add did not carry, i.e. while the
                     * running sum is still the larger unsigned value. */
                    if((br_uint_32)u_num < before)
                        break;
                } while(true);
            }
        }

        if(DIRN == RPD_FWD)
            v_num += dv_num;
        else
            v_num -= dv_num;

        if(VD == RPU_BOTH) {
            if(v_num < 0) {
                do {
                    source = DecV(source, vshift, mask_h);
                    dv_num += dden;
                    v_num += den;
                } while(v_num < 0);

            } else if(v_num >= den) {
                do {
                    source = IncV(source, vshift, mask_h);
                    dv_num -= dden;
                    v_num -= den;
                } while(v_num >= den);
            }
        } else if(VD == RPU_INC) {
            if(v_num >= 0) {
                do {
                    source = IncV(source, vshift, mask_h);
                    dv_num -= dden;
                    v_num -= den;
                } while(v_num >= 0);
            }
        } else {
            if(v_num < 0) {
                do {
                    source = DecV(source, vshift, mask_h);
                    dv_num += dden;
                    const br_uint_32 before = (br_uint_32)v_num;

                    v_num += den;

                    if((br_uint_32)v_num < before)
                        break;
                } while(true);
            }
        }
    }
}

/*
 * Select the span variant from the signs of the projective x gradients. The
 * comparisons are the asm's; the backward instantiation flips all of them.
 */
template <sp_depth D, sp_shade S, sp_blend B, sp_fog G, int DIRN>
void PerspRunSpan(PerspWalk& w, const softprim_buffer& texture, const softprim_buffer& shade, br_int_32 mask_w, br_int_32 mask_h, br_int_32 vshift) noexcept
{
    const br_int_32 pu  = w.pu.grad_x;
    const br_int_32 pv  = w.pv.grad_x;
    const br_int_32 pq  = w.pq.grad_x;
    const bool      rev = (DIRN == RPD_BWD);

    const auto lt = [rev](br_int_32 a, br_int_32 b) { return rev ? a > b : a < b; };
    const auto le = [rev](br_int_32 a, br_int_32 b) { return rev ? a >= b : a <= b; };
    const auto gt = [rev](br_int_32 a, br_int_32 b) { return rev ? a < b : a > b; };

    /* The five instantiated variants, named after the asm's dispatch targets. */
    enum {
        V_UD_VD,
        V_UD_VI,
        V_UI_VD,
        V_UI_VI,
        V_UB_VB
    } v;

    if(lt(pq, 0)) {
        if(le(pu, 0)) {
            if(gt(pu, pq))
                v = V_UB_VB;
            else if(gt(pv, 0))
                v = V_UD_VI;
            else if(le(pv, pq))
                v = V_UD_VD;
            else
                v = V_UB_VB;
        } else if(gt(pv, 0)) {
            v = V_UI_VI;
        } else if(le(pv, pq)) {
            v = V_UI_VD;
        } else {
            v = V_UB_VB;
        }
    } else if(le(pu, 0)) {
        if(le(pv, 0))
            v = V_UD_VD;
        else
            v = V_UD_VI;
    } else if(le(pv, 0)) {
        v = V_UI_VD;
    } else {
        v = V_UI_VI;
    }

    switch(v) {
        case V_UD_VD:
            PerspSpan<D, S, B, G, RPU_DEC, RPU_DEC, DIRN>(w, texture, shade, mask_w, mask_h, vshift);
            break;
        case V_UD_VI:
            PerspSpan<D, S, B, G, RPU_DEC, RPU_INC, DIRN>(w, texture, shade, mask_w, mask_h, vshift);
            break;
        case V_UI_VD:
            PerspSpan<D, S, B, G, RPU_INC, RPU_DEC, DIRN>(w, texture, shade, mask_w, mask_h, vshift);
            break;
        case V_UI_VI:
            PerspSpan<D, S, B, G, RPU_INC, RPU_INC, DIRN>(w, texture, shade, mask_w, mask_h, vshift);
            break;
        case V_UB_VB:
            PerspSpan<D, S, B, G, RPU_BOTH, RPU_BOTH, DIRN>(w, texture, shade, mask_w, mask_h, vshift);
            break;
    }
}

/*
 * TrapeziumRender_ZPTI_I8_D16: the scanline loop for one trapezium. Everything
 * in w carries over between the two halves of a triangle, so this is called
 * twice with minor_i/minor_d/minor_count reloaded from the bottom edge.
 */
template <sp_depth D, sp_shade S, sp_blend B, sp_fog G, int DIRN>
void PerspTrapezium(PerspWalk& w, const softprim_buffer& colour, const softprim_buffer& depth, const softprim_buffer& texture,
                    const softprim_buffer& shade, br_int_32 mask_w, br_int_32 mask_h, br_int_32 vshift) noexcept
{
    for(br_int_32 n = w.minor_count; n >= 0; n--) {
        const br_int_32  main_int  = (br_int_32)(w.main_i & 0xffffu);
        const br_uint_32 minor_int = (br_uint_32)w.minor_i / 65536u;
        const bool       draw      = (DIRN == RPD_FWD) ? (main_int <= (br_int_32)minor_int) : (main_int >= (br_int_32)minor_int);

        if(draw) {
            PerspScanlineAdjust(w, mask_w, mask_h, vshift);
            PerspRunSpan<D, S, B, G, DIRN>(w, texture, shade, mask_w, mask_h, vshift);
        }

        /*
         * Step the major edge in its byte-swapped form so the carry out of the
         * 32-bit add is the fraction overflow, then spend it on the parameters.
         */
        const br_uint_32 sum   = w.main_i + w.main_d;
        const br_uint_32 carry = (sum < w.main_i) ? 1u : 0u;

        w.main_i = sum + carry;

        w.minor_i += w.minor_d;

        w.pq.current += (carry != 0u) ? w.pq.d_carry : w.pq.d_nocarry;
        w.pu.current += w.pu.d_nocarry + ((carry != 0u) ? w.pu.grad_x : 0);
        w.pv.current += w.pv.d_nocarry + ((carry != 0u) ? w.pv.grad_x : 0);
        w.pz.current += (carry != 0u) ? w.pz.d_carry : w.pz.d_nocarry;
        w.pi.current += (carry != 0u) ? w.pi.d_carry : w.pi.d_nocarry;

        w.scan += colour.stride_b;

        if(D == SP_DEPTH_ZW)
            w.zscan += depth.stride_b;
    }
}

inline br_uint_8 RgbFormatType(sp_fmt F) noexcept;

/*
 * One textured INDEX_8 triangle.
 *
 * D selects depth test/write, S the shading source and P whether the
 * perspective path is used; all three are template parameters so the branches
 * fold away.
 */
template <sp_depth D, sp_shade S, sp_persp P, sp_blend B, sp_fog G>
void TexturedIndexedTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    (void)block;

    const softprim_buffer& colour  = SoftPrimWork.colour;
    const softprim_buffer& dbuf    = SoftPrimWork.depth;
    const softprim_buffer& texture = SoftPrimWork.texture;
    const softprim_buffer& shade   = SoftPrimWork.shade;

    if(colour.type != BR_PMT_INDEX_8 || colour.base == NULL || colour.width_p <= 0 || colour.height <= 0)
        return;

    if(D == SP_DEPTH_ZW && (dbuf.type != BR_PMT_DEPTH_16 || dbuf.base == NULL))
        return;

    if(texture.base == NULL || texture.type != BR_PMT_INDEX_8 || texture.width_p == 0 || texture.height == 0)
        return;

    /* The runtime-dimension design supports only power-of-two maps. */
    if((texture.width_p & (texture.width_p - 1)) != 0 || (texture.height & (texture.height - 1)) != 0)
        return;

    if(S != SP_SHADE_NONE && (shade.base == NULL || shade.type != BR_PMT_INDEX_8))
        return;

    if(B == SP_BLEND_INDEX && (SoftPrimWork.blend.base == NULL || SoftPrimWork.blend.type != BR_PMT_INDEX_8))
        return;

    if(G == SP_FOG_INDEX && (SoftPrimWork.fog.base == NULL || SoftPrimWork.fog.type != BR_PMT_INDEX_8))
        return;

    const br_int_32 mask_w = (br_int_32)texture.width_p - 1;
    const br_int_32 mask_h = (br_int_32)texture.height - 1;
    br_int_32       vshift = 0;

    while((1u << vshift) < texture.width_p)
        vshift++;

    const bool interpolated = (S == SP_SHADE_INTERP_I);

    brp_vertex *src[3] = {v0, v1, v2};
    TexVert     in[3];

    for(int k = 0; k < 3; k++) {
        in[k].x  = BrFloatToFixed(src[k]->comp[C_SX]);
        in[k].y  = BrFloatToFixed(src[k]->comp[C_SY]);
        in[k].z  = BrFloatToFixed(src[k]->comp[C_SZ]);
        in[k].u  = BrFloatToFixed(src[k]->comp[C_U]);
        in[k].v  = BrFloatToFixed(src[k]->comp[C_V]);
        in[k].w  = BrFloatToFixed(src[k]->comp[C_W]);
        in[k].i  = BrFloatToFixed(src[k]->comp[C_I]);
        in[k].rx = src[k]->comp[C_SX];
        in[k].ry = src[k]->comp[C_SY];
        in[k].rz = src[k]->comp[C_SZ];
        in[k].rw = src[k]->comp[C_W];
        in[k].ru = src[k]->comp[C_U];
        in[k].rv = src[k]->comp[C_V];
        in[k].ri = src[k]->comp[C_I];

        if(!isfinite(in[k].rx) || !isfinite(in[k].ry) || !isfinite(in[k].rw))
            return;
    }

    ApplyIntensity(in, S, interpolated);

    TriSetup ts;

    if(!BuildTriSetup(in, ts))
        return;

    /* Sorted vertices, the pre-sort order the parameter deltas use, and the
     * asm scan direction - all from the one shared setup. */
    TexVert        *v       = ts.v;
    const TexVert  *orig    = ts.orig;
    const br_int_32 asm_dir = ts.asm_dir;

    /*
     * SETUP_FLOAT_CHECK_PERSPECTIVE_CHEAT (pfpsetup.asm:778): a perspective
     * block renders affinely for any triangle whose w range is narrow relative
     * to its screen extent. The threshold is (wmax-wmin)*srange < 4.0*wmin with
     * srange = max(sx range, sy range), evaluated on the raw float attributes.
     */
    bool affine = (P == SP_PERSP_AFFINE);

    if(P == SP_PERSP_CORRECT) {
        /*
         * SETUP_FLOAT_CHECK_PERSPECTIVE_CHEAT (pfpsetup.asm:778) runs on the x87:
         * the ranges are exact extended differences, srange is kept unrounded
         * until the product with wrange, and both products are stored as singles
         * (xr_yr/wr_sr/wmin_4 are dword data) before an unsigned compare.
         */
        long double xmin = v[0].rx, xmax = v[0].rx;
        long double wmin = v[0].rw, wmax = v[0].rw;

        for(int k = 1; k < 3; k++) {
            if((long double)v[k].rx < xmin)
                xmin = v[k].rx;
            if((long double)v[k].rx > xmax)
                xmax = v[k].rx;
            if((long double)v[k].rw < wmin)
                wmin = v[k].rw;
            if((long double)v[k].rw > wmax)
                wmax = v[k].rw;
        }

        const long double yrange = (long double)v[2].ry - (long double)v[0].ry;
        const long double xrange = xmax - xmin;

        /* `fstp xr_yr` rounds the difference to single; the sign bit selects. */
        const float xr_yr = (float)(xrange - yrange);
        br_uint_32  xr_bits;

        memcpy(&xr_bits, &xr_yr, sizeof(xr_bits));

        const long double srange = (xr_bits & 0x80000000u) ? yrange : xrange;

        const float wr_sr  = (float)((wmax - wmin) * srange);
        const float wmin_4 = (float)(wmin * 4.0f);
        br_uint_32  wr_bits, wm_bits;

        memcpy(&wr_bits, &wr_sr, sizeof(wr_bits));
        memcpy(&wm_bits, &wmin_4, sizeof(wm_bits));

        if(wr_bits < wm_bits)
            affine = true;
    }

    if(affine) {
        /*
         * Affine INDEX_8 textured triangle: zb8p2lit.asm TriangleRender_ZTI_
         * I8_D16_POW2 (built from DRAW_ZTI_I8_D16_POW2) with the setup from
         * fpsetup.asm TriangleSetup_ZTI. Used for SP_PERSP_AFFINE tuples and as
         * the cheat's fallback from a perspective triangle, which is what
         * pentprim's setup does. u/v/i/z step linearly; no projective divide.
         */
        const WalkGeom& g = ts.walk;

        /* fpsetup.asm TriangleSetup_ZTI / _ZT: SETUP_FLOAT_PARAM per parameter. */
        TrapParam au, av, ai, az;

        SetupFloatParam(au, ts, ts.orig[0].ru, ts.orig[1].ru, ts.orig[2].ru, ts.v[0].ru, false);
        SetupFloatParam(av, ts, ts.orig[0].rv, ts.orig[1].rv, ts.orig[2].rv, ts.v[0].rv, false);
        SetupFloatParam(ai, ts, ts.orig[0].ri, ts.orig[1].ri, ts.orig[2].ri, ts.v[0].ri, false);
        SetupFloatParam(az, ts, ts.orig[0].rz, ts.orig[1].rz, ts.orig[2].rz, ts.v[0].rz, true);

        br_uint_8 *row  = colour.base + (br_size_t)g.yTop * colour.stride_b - 1;
        br_uint_8 *zrow = ((D == SP_DEPTH_ZW) ? dbuf.base : colour.base) - 2 + (br_size_t)g.yTop * ((D == SP_DEPTH_ZW) ? dbuf.stride_b : 0);

        br_int_32 xmv = g.xm, xm_f = g.xm_f;

        /*
         * DRAW_ZT_I8_D16_POW2 fixes the scan direction for the whole triangle
         * (zb8p2unl.asm:238) and skips a row whose major and minor edges have
         * crossed instead of reversing the span on it. The direction is the
         * setup's sort parity for a plain affine block and work.tsl.direction
         * for the perspective blocks' cheat, which overwrites workspace.flip
         * (pfpsetup.asm:2669).
         */
        const bool l2r = (P == SP_PERSP_AFFINE) ? (ts.flip != 0) : (ts.asm_dir == 0);

        /*
         * The one entry pentprim builds on TRAPEZIUM_ZTF_I8_D16_256 is the
         * texture-only power-of-two fog block, whose map is 256x256 and whose
         * axes are the ones tested here, and it is the only affine INDEX_8
         * kernel that does not walk the coordinates in 16.16. Every other kernel
         * for this shape, including the same file's unfogged TriangleRender_ZT_
         * I8_D16_256 and zb8.asm's interpolated TriangleRender_ZTIF_I8_D16_256,
         * is the full walk above, so the packed variant is selected by the
         * entry's axes rather than by the walk's caller.
         */
        const bool packed = (P == SP_PERSP_AFFINE && S == SP_SHADE_NONE && B == SP_BLEND_NONE && G == SP_FOG_INDEX &&
                             texture.width_p == 256 && texture.height == 256);

        for(int half = 0; half < 2; half++) {
            br_int_32       minorX = (half == 0) ? g.x1 : g.x2;
            const br_int_32 dMinor = (half == 0) ? g.d_x1 : g.d_x2;
            const br_int_32 count  = (half == 0) ? g.topCount : g.bottomCount;

            for(br_int_32 n = count; n >= 0; n--) {
                const br_int_32 xm_i = xmv >> 16;
                const br_int_32 mn_i = minorX >> 16;

                if(l2r ? (xm_i <= mn_i) : (xm_i >= mn_i)) {
                    if(packed)
                        AffineDrawSpan<D, S, B, G, true>(texture, shade, row + xm_i, row + mn_i, zrow + (br_size_t)xm_i * 2, au.current,
                                                         au.grad_x, av.current, av.grad_x, ai.current, ai.grad_x, az.current, az.grad_x,
                                                         mask_w, mask_h, vshift, l2r);
                    else
                        AffineDrawSpan<D, S, B, G>(texture, shade, row + xm_i, row + mn_i, zrow + (br_size_t)xm_i * 2, au.current,
                                                   au.grad_x, av.current, av.grad_x, ai.current, ai.grad_x, az.current, az.grad_x, mask_w,
                                                   mask_h, vshift, l2r);
                }

                xm_f += g.d_xm_f;
                const bool carry = (br_uint_32)xm_f < (br_uint_32)g.d_xm_f;

                au.current += carry ? au.d_carry : au.d_nocarry;
                av.current += carry ? av.d_carry : av.d_nocarry;
                ai.current += carry ? ai.d_carry : ai.d_nocarry;
                az.current += carry ? az.d_carry : az.d_nocarry;

                xmv += g.d_xm;
                minorX += dMinor;
                row += colour.stride_b;
                zrow += (D == SP_DEPTH_ZW) ? dbuf.stride_b : 0;
            }
        }

        return;
    }

    /*
     * Perspective-correct INDEX_8 textured triangle.
     *
     * pfpsetup.asm's TriangleSetup_ZPTI (SETUP_FLOAT for the geometry and
     * SETUP_FLOAT_UV_PERSPECTIVE for the projective parameters) followed by
     * fti8pizp.asm's TriangleRasterise_ZPTI_I8_D16 and its trapezium walk: the
     * base texel is rounded, the projective set is normalised by the
     * power-of-two maxuv factor, the numerators carry no binary point, and the
     * parameter start is the sample the trapezium walk actually reaches.
     *
     * The asm's per-triangle "cheat" into the affine rasteriser for narrow w
     * ranges is decided before this point (see the affine branch above) and
     * reaches the same affine span.
     */
    PerspWalk pw;

    /* Depth and intensity are affine 16.16, started at the sorted top vertex. */
    SetupFloatParam(pw.pz, ts, orig[0].rz, orig[1].rz, orig[2].rz, v[0].rz, true);

    if(S != SP_SHADE_NONE)
        SetupFloatParam(pw.pi, ts, orig[0].ri, orig[1].ri, orig[2].ri, v[0].ri, false);
    else
        pw.pi = {0, 0, 0, 0};

    /* --- SETUP_FLOAT_UV_PERSPECTIVE: base point and projective set. --- */
    const long double u0 = v[0].ru, u1 = v[1].ru, u2 = v[2].ru;
    const long double e0 = v[0].rv, e1 = v[1].rv, e2 = v[2].rv;
    const long double w0 = v[0].rw, w1 = v[1].rw, w2 = v[2].rw;

    /* fistp/fild: the base texel is rounded, not truncated. */
    const br_int_32 ub = (br_int_32)llrintl(u0);
    const br_int_32 vb = (br_int_32)llrintl(e0);

    long double au = u0 - ub, bu = u1 - ub, cu = u2 - ub;
    long double av = e0 - vb, bv = e1 - vb, cv = e2 - vb;

    /*
     * q'i is the product of the other two vertices' homogeneous w. The asm
     * keeps the extended product on the x87 stack for the numerator scaling
     * but also stores it to a dword (q0/q1/q2); the differences that feed the q
     * gradient use the rounded copies.
     */
    const long double aw = w1 * w2, bw = w2 * w0, cw = w1 * w0;
    const long double q0 = (float)aw, q1 = (float)bw, q2 = (float)cw;

    long double maxuv = FabsL(aw) + FabsL(au) + FabsL(av);

    /* The asm spills maxuv to a float here and reloads it. */
    maxuv = (long double)(float)maxuv;

    au *= aw;
    av *= aw;
    bu *= bw;
    bv *= bw;
    cu *= cw;
    cv *= cw;

    maxuv += FabsL(au) + FabsL(av) + FabsL(bu) + FabsL(bv) + FabsL(cu) + FabsL(cv);

    const float       maxuv_f = (float)maxuv;
    const long double norm    = (long double)PerspMaxuvNorm(maxuv_f);

    const long double du1 = bu - au, du2 = cu - au;
    const long double dv1 = bv - av, dv2 = cv - av;
    const long double dq1 = q1 - q0, dq2 = q2 - q0;

    const long double udx = du1 * ts.sdy2a - du2 * ts.sdy1a;
    const long double udy = du2 * ts.sdx1a - du1 * ts.sdx2a;
    const long double vdx = dv1 * ts.sdy2a - dv2 * ts.sdy1a;
    const long double vdy = dv2 * ts.sdx1a - dv1 * ts.sdx2a;
    const long double qdx = dq1 * ts.sdy2a - dq2 * ts.sdy1a;
    const long double qdy = dq2 * ts.sdx1a - dq1 * ts.sdx2a;

    /* pstart = p0 + pdx*fdx + pdy*fdy, d_0 = pdy + xstep_0*pdx. */
    const long double udy0 = udy + ts.xstep_0 * udx;
    const long double vdy0 = vdy + ts.xstep_0 * vdx;
    const long double qdy0 = qdy + ts.xstep_0 * qdx;
    const long double qdy1 = qdy + ts.xstep_1 * qdx;

    pw.pu.current   = ConvInt((au + udx * ts.t_dx + udy * ts.t_dy) * norm);
    pw.pu.grad_x    = ConvInt(udx * norm);
    pw.pu.d_nocarry = ConvInt(udy0 * norm);
    pw.pu.d_carry   = pw.pu.d_nocarry;

    pw.pv.current   = ConvInt((av + vdx * ts.t_dx + vdy * ts.t_dy) * norm);
    pw.pv.grad_x    = ConvInt(vdx * norm);
    pw.pv.d_nocarry = ConvInt(vdy0 * norm);
    pw.pv.d_carry   = pw.pv.d_nocarry;

    pw.pq.current   = ConvInt((q0 + qdx * ts.t_dx + qdy * ts.t_dy) * norm);
    pw.pq.grad_x    = ConvInt(qdx * norm);
    pw.pq.d_nocarry = ConvInt(qdy0 * norm);
    pw.pq.d_carry   = ConvInt(qdy1 * norm);

    if(pw.pq.current == 0)
        return;

    /* --- TriangleRasterise_ZPTI_I8_D16: walk state and start address. --- */
    pw.main_i = (br_uint_32)Ror16(ts.walk.xm);
    pw.main_d = (br_uint_32)Ror16(NegAdjust(ts.walk.d_xm));

    pw.dz           = NegAdjust(pw.pz.grad_x);
    pw.di           = NegAdjust(pw.pi.grad_x);
    pw.ddenominator = pw.pq.grad_x;
    pw.direction    = asm_dir;

    /* The scanline start is the base texel, wrapped into the map. */
    pw.source = ((br_uint_32)(vb & mask_h) << vshift) | (br_uint_32)(ub & mask_w);

    pw.scan  = colour.base + (br_size_t)ts.walk.yTop * colour.stride_b - 1;
    pw.zscan = ((D == SP_DEPTH_ZW) ? dbuf.base : colour.base) + (br_size_t)ts.walk.yTop * ((D == SP_DEPTH_ZW) ? dbuf.stride_b : 0) - 2;

    /* Top trapezium: the row below the top vertex up to the middle one. */
    pw.minor_i     = ts.walk.x1;
    pw.minor_d     = ts.walk.d_x1;
    pw.minor_count = ts.walk.topCount;

    if(asm_dir == 0)
        PerspTrapezium<D, S, B, G, RPD_FWD>(pw, colour, dbuf, texture, shade, mask_w, mask_h, vshift);
    else
        PerspTrapezium<D, S, B, G, RPD_BWD>(pw, colour, dbuf, texture, shade, mask_w, mask_h, vshift);

    /* Bottom trapezium: the row below the middle vertex to the bottom one. */
    pw.minor_i     = ts.walk.x2;
    pw.minor_d     = ts.walk.d_x2;
    pw.minor_count = ts.walk.bottomCount;

    if(asm_dir == 0)
        PerspTrapezium<D, S, B, G, RPD_FWD>(pw, colour, dbuf, texture, shade, mask_w, mask_h, vshift);
    else
        PerspTrapezium<D, S, B, G, RPD_BWD>(pw, colour, dbuf, texture, shade, mask_w, mask_h, vshift);
}

/*
 * ---------------------------------------------------------------------------
 * Arbitrary-width INDEX_8 texture mapping.
 *
 * zb8awtm.asm's TriangleRender_ZT_I8_D16 (unlit) and TriangleRender_ZTI_I8_D16
 * (lit) for the z-buffered blocks, and zs8awtm.asm's TriangleRender_T_I8 /
 * TriangleRender_TI_I8 for their z-sorted counterparts. These are the blocks
 * the bare `texture` property selects - infogen.pl maps a block whose width and
 * height are both unset, and which is not POWER2, onto SP_ADDR_DIVIDE.
 *
 * The geometry is the same SETUP_FLOAT the shift/mask triangles use, so
 * BuildTriSetup above serves both; only the parameter setup and the span are
 * specific to this path. The parameter setup is TriangleSetup_ZT_ARBITRARY:
 * SETUP_FLOAT + SETUP_FLOAT_PARAM with fp_conv_d16 for z and i and fp_conv_d24
 * (8.24) for u and v, then ARBITRARY_SETUP. The span is DRAW_ZT_I8 /
 * DRAW_ZTI_I8 (DRAW_T_I8 / DRAW_TI_I8 without depth) and the row advance is
 * PER_SCAN_ZT / PER_SCAN_ZTI.
 *
 * This is deliberately *not* awtmi.h / t_piza.asm. Those are the C RGB family
 * (TriangleRenderPIZ2TA*), a different lineage with a different setup and a
 * different addressing scheme, and mixing the two is what produced the
 * streaking this port replaces.
 * ---------------------------------------------------------------------------
 */

/*
 * ARBITRARY_SETUP (fpsetup.asm:1509) runs under fpu_cw (fpsetup.asm:2105) =
 * round-toward-zero, double precision. Every conversion it performs has an
 * integer input and an integer result, so it is reproduced exactly in integer
 * arithmetic and the FPU control word never has to be modelled:
 *
 *   REMOVE_INTEGER_PARTS_OF_PARAMETERS keeps the 8.24 fraction only - the low
 *   24 bits for a non-negative value, the low 24 bits with the top byte forced
 *   to 0xFF for a negative one.
 *   MULTIPLY_UP_PARAM_VALUES multiplies by the map dimension under
 *   fp_conv_d8r, whose magic has ULP 256: the product is exact, so the
 *   round-toward-zero left in the low 32 bits is floor(v * dimension / 256).
 *   SPLIT_INTO_INTEGER_AND_FRACTIONAL_PARTS splits each v value into a .32
 *   fraction accumulator and an integer part.
 *   MULTIPLY_UP_V_BY_STRIDE scales those integer parts to byte offsets, and
 *   CREATE_CARRY_VERSIONS adds stride to the ones a fraction carry selects.
 *   WRAP_SETUP finishes with u modulo width << 16 and v modulo size, where
 *   pentprim's size is height * stride_b (sbuffer.c SetupRenderBuffer).
 *
 * SETUP_FLAGS is not ported. It only chooses between the four jump-table
 * entries, and this source assembles all four WRAPPED - the NON_WRAPPED case is
 * behind `if 0` - so the choice has no observable effect. The jump tables
 * themselves collapse to "always wrapped".
 */

/* REMOVE_INTEGER_PARTS_OF_PARAM: the 8.24 fraction of a signed value. */
inline br_int_32 Frac824(br_int_32 v) noexcept
{
    return (v < 0) ? (v | (br_int_32)0xff000000u) : (v & 0x00ffffff);
}

/* fp_conv_d8r under the round-toward-zero control word: floor(v / 256). */
inline br_int_32 FloorDiv256(long long v) noexcept
{
    long long q = v / 256;

    if(v < 0 && q * 256 != v)
        q--;

    return (br_int_32)q;
}

/* MULTIPLY_UP_PARAM_VALUES: one 8.24 value scaled by a map dimension. */
inline br_int_32 MulUp824(br_int_32 frac824, br_int_32 dimension) noexcept
{
    return FloorDiv256((long long)frac824 * (long long)dimension);
}

/*
 * The workspaceA half of ARBITRARY_SETUP: the packed u/v walk state zb8awtm.asm
 * carries between scanlines. su is one 16.16 texel column; the v coordinate is
 * split into the byte offset sv (always a whole number of rows) and the .32
 * fraction accumulator svf, because sv is used as a pointer and so has to step
 * in rows.
 */
struct ArbitraryTex {
    br_uint_32 su;  /* c_u at the top of the triangle, 16.16 texel column */
    br_int_32  dux; /* per-pixel u step */
    br_int_32  duy1;
    br_int_32  duy0; /* per-scanline u steps, selected by the x carry */

    br_int_32 sv;   /* v row, as a byte offset from the texture base */
    br_int_32 dvx;  /* per-pixel v step */
    br_int_32 dvxc; /* ... plus stride, for when the v fraction carries */
    br_int_32 dvy1;
    br_int_32 dvy1c;
    br_int_32 dvy0;
    br_int_32 dvy0c; /* per-scanline v steps */

    br_uint_32 svf;  /* v fraction accumulator at the top of the triangle */
    br_uint_32 dvxf; /* per-pixel v fraction step */
    br_uint_32 dvy1f;
    br_uint_32 dvy0f; /* per-scanline v fraction steps */

    br_uint_32 uUpperBound; /* width << 16 */
    br_int_32  size;        /* v modulus: height * stride_b */
};

inline ArbitraryTex BuildArbitraryTex(const TrapParam& pu, const TrapParam& pv, br_int_32 width, br_int_32 height, br_int_32 stride) noexcept
{
    ArbitraryTex a{};

    a.su   = MulUp824((br_int_32)((br_uint_32)pu.current & 0x00ffffffu), width);
    a.dux  = MulUp824(Frac824(pu.grad_x), width);
    a.duy0 = MulUp824(Frac824(pu.d_nocarry), width);
    a.duy1 = MulUp824(Frac824(pu.d_carry), width);

    /* The v start is masked like u (REMOVE_INTEGER_PARTS_OF_PARAMETERS); only
     * the gradients go through the sign-preserving form. */
    const br_int_32 sv   = MulUp824((br_int_32)((br_uint_32)pv.current & 0x00ffffffu), height);
    const br_int_32 dvx  = MulUp824(Frac824(pv.grad_x), height);
    const br_int_32 dvy0 = MulUp824(Frac824(pv.d_nocarry), height);
    const br_int_32 dvy1 = MulUp824(Frac824(pv.d_carry), height);

    /* SPLIT_INTO_INTEGER_AND_FRACTIONAL_PARTS. */
    a.svf   = (br_uint_32)sv << 16;
    a.dvxf  = (br_uint_32)dvx << 16;
    a.dvy0f = (br_uint_32)dvy0 << 16;
    a.dvy1f = (br_uint_32)dvy1 << 16;

    /* MULTIPLY_UP_V_BY_STRIDE. */
    a.sv   = (sv >> 16) * stride;
    a.dvx  = (dvx >> 16) * stride;
    a.dvy0 = (dvy0 >> 16) * stride;
    a.dvy1 = (dvy1 >> 16) * stride;

    /* CREATE_CARRY_VERSIONS. */
    a.dvxc  = a.dvx + stride;
    a.dvy0c = a.dvy0 + stride;
    a.dvy1c = a.dvy1 + stride;

    /* WRAP_SETUP. */
    a.uUpperBound = (br_uint_32)width << 16;
    a.size        = height * stride;

    return a;
}

/*
 * The workspace half of the arbitrary-width walk: the same 16.16 trapezium as
 * IndexedWalk, plus the packed u/v state ARBITRARY_SETUP produced.
 */
struct DivideWalk {
    const softprim_buffer *colour;
    const softprim_buffer *depth;
    const softprim_buffer *texture;
    const softprim_buffer *shade;

    ArbitraryTex tex;

    TrapParam pz; /* depth, 16.16, remapped unsigned */
    TrapParam pi; /* intensity, 16.16; all zero when the block is unlit */

    br_uint_32 su;  /* c_u, the u column at the start of the scanline */
    br_uint_32 svf; /* c_v, the v fraction accumulator */
    br_int_32  sv;  /* the v row byte offset */

    br_int_32  main_i, main_d;
    br_uint_32 main_f, main_d_f;
    br_int_32  minor_i, minor_d;
    br_int_32  minor_count;

    br_uint_8 *scan;  /* colour row start - 1 */
    br_uint_8 *zscan; /* depth row start - 2 */
};

/*
 * The WRAPPED u/v modulo loops. u is signed 16.16 (the asm compares it with 0
 * and with uUpperBound signed) and v is a byte offset into the texture (the asm
 * compares the absolute pointer unsigned against base and upperBound, which for
 * a valid texture is the same as comparing the offset with 0 and size).
 */
inline void WrapTexU(br_uint_32& u, br_uint_32 bound) noexcept
{
    while((br_int_32)u < 0)
        u += bound;

    while((br_int_32)u >= (br_int_32)bound)
        u -= bound;
}

inline void WrapTexV(br_int_32& v, br_int_32 size) noexcept
{
    if(size <= 0)
        return;

    while(v < 0)
        v += size;

    while(v >= size)
        v -= size;
}

/*
 * DRAW_ZT_I8 (z-buffered) / DRAW_T_I8 (z-sorted) and their lit companions, the
 * WRAPPED instantiation: one scanline.
 *
 * The u column is the high 16 bits of c_u and the v row is the byte offset in
 * sv, so the texel is tex[sv + (c_u >> 16)]. Texel 0 is transparent and is
 * tested on the raw texel before the shade lookup. The v fraction accumulator
 * steps by dvxf and its carry selects the +stride version of the v step.
 */
template <sp_depth D, sp_shade S, sp_blend B, sp_fog G, int DIRN>
void DivideSpan(DivideWalk& w, br_uint_8 *c, br_uint_8 *end, br_uint_8 *z) noexcept
{
    const br_uint_8 *tex  = w.texture->base;
    const br_uint_8 *stbl = w.shade->base;

    br_uint_32 u  = w.su;
    br_uint_32 v  = w.svf;
    br_int_32  sv = w.sv;
    br_uint_32 zz = (br_uint_32)w.pz.current;
    br_uint_32 ii = (br_uint_32)w.pi.current;

    for(;;) {
        bool       draw = true;
        br_uint_16 newz = 0;

        if(D == SP_DEPTH_ZW) {
            newz = (br_uint_16)(zz >> 16);

            if(newz > LoadDepth(z))
                draw = false;
        }

        if(draw) {
            const br_uint_8 texel = tex[sv + (br_int_32)(u >> 16)];

            if(texel != 0) {
                br_uint_8 out = texel;

                if(S != SP_SHADE_NONE)
                    out = stbl[((ii >> 16) & 0xff) * 256 + texel];

                if(D == SP_DEPTH_ZW)
                    StoreDepth(z, newz);

                *c = ApplyRops<B, G>(out, c, DepthHighByte(zz));
            }
        }

        if(DIRN == RPD_FWD) {
            const br_uint_32 nv = v + w.tex.dvxf;
            const br_uint_32 vc = (nv < w.tex.dvxf) ? 1u : 0u;

            v = nv;
            sv += (vc != 0u) ? w.tex.dvxc : w.tex.dvx;
            WrapTexV(sv, w.tex.size);

            u += w.tex.dux;
            WrapTexU(u, w.tex.uUpperBound);

            zz += (br_uint_32)w.pz.grad_x;
            ii += (br_uint_32)w.pi.grad_x;

            if(D == SP_DEPTH_ZW)
                z += 2;

            c++;

            if(c > end)
                break;
        } else {
            const br_uint_32 nv = v - w.tex.dvxf;
            const br_uint_32 vc = (nv > v) ? 1u : 0u; /* the borrow */

            v = nv;
            sv -= (vc != 0u) ? w.tex.dvxc : w.tex.dvx;
            WrapTexV(sv, w.tex.size);

            u -= w.tex.dux;
            WrapTexU(u, w.tex.uUpperBound);

            zz -= (br_uint_32)w.pz.grad_x;
            ii -= (br_uint_32)w.pi.grad_x;

            if(D == SP_DEPTH_ZW)
                z -= 2;

            c--;

            if(c < end)
                break;
        }
    }
}

/*
 * PER_SCAN_ZT / PER_SCAN_ZTI: advance the trapezium by one scanline.
 *
 * The carry out of the major edge's x accumulator selects the "_y_1"
 * (carry) increments. The carry out of the v fraction add selects the
 * "+stride" version of the v step - that is the rcl into the index of the
 * dvy0/dvy0c/dvy1/dvy1c table, which the code spells as a 0/-1 x carry and a
 * 0/1 v carry.
 */
template <sp_depth D, sp_shade S, sp_blend B, sp_fog G, int DIRN>
void DivideTrapezium(DivideWalk& w) noexcept
{
    for(br_int_32 n = w.minor_count; n >= 0; n--) {
        const br_int_32 main_int  = (br_int_32)((br_uint_32)w.main_i >> 16);
        const br_int_32 minor_int = (br_int_32)((br_uint_32)w.minor_i >> 16);
        const bool      draw      = (DIRN == RPD_FWD) ? (main_int <= minor_int) : (main_int >= minor_int);

        if(draw)
            DivideSpan<D, S, B, G, DIRN>(w, w.scan + main_int, w.scan + minor_int, w.zscan + (br_size_t)main_int * 2);

        const br_uint_32 sum   = w.main_f + w.main_d_f;
        const br_uint_32 carry = (sum < w.main_d_f) ? 1u : 0u;

        w.main_f = sum;

        w.su += (carry != 0u) ? w.tex.duy1 : w.tex.duy0;

        const br_uint_32 dvyf = (carry != 0u) ? w.tex.dvy1f : w.tex.dvy0f;
        const br_uint_32 nv   = w.svf + dvyf;
        const br_uint_32 vc   = (nv < dvyf) ? 1u : 0u;

        w.svf = nv;

        w.sv += (carry != 0u) ? ((vc != 0u) ? w.tex.dvy1c : w.tex.dvy1) : ((vc != 0u) ? w.tex.dvy0c : w.tex.dvy0);
        WrapTexV(w.sv, w.tex.size);
        WrapTexU(w.su, w.tex.uUpperBound);

        w.pz.current += (carry != 0u) ? w.pz.d_carry : w.pz.d_nocarry;
        w.pi.current += (carry != 0u) ? w.pi.d_carry : w.pi.d_nocarry;

        w.main_i += w.main_d;
        w.minor_i += w.minor_d;
        w.scan += w.colour->stride_b;

        if(D == SP_DEPTH_ZW)
            w.zscan += w.depth->stride_b;
    }
}

/*
 * One arbitrary-width INDEX_8 triangle.
 *
 * D selects depth test/write and S the shade source; both are template
 * parameters so the branches fold away and the four asm kernels share one
 * source.
 */
template <sp_depth D, sp_shade S, sp_blend B, sp_fog G>
void TexturedDivideTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    (void)block;

    const softprim_buffer& colour  = SoftPrimWork.colour;
    const softprim_buffer& dbuf    = SoftPrimWork.depth;
    const softprim_buffer& texture = SoftPrimWork.texture;
    const softprim_buffer& shade   = SoftPrimWork.shade;

    if(colour.type != BR_PMT_INDEX_8 || colour.base == NULL || colour.width_p <= 0 || colour.height <= 0)
        return;

    if(D == SP_DEPTH_ZW && (dbuf.type != BR_PMT_DEPTH_16 || dbuf.base == NULL))
        return;

    if(texture.base == NULL || texture.type != BR_PMT_INDEX_8 || texture.width_p <= 0 || texture.height <= 0 || texture.stride_b <= 0)
        return;

    if(S != SP_SHADE_NONE && (shade.base == NULL || shade.type != BR_PMT_INDEX_8))
        return;

    if(B == SP_BLEND_INDEX && (SoftPrimWork.blend.base == NULL || SoftPrimWork.blend.type != BR_PMT_INDEX_8))
        return;

    if(G == SP_FOG_INDEX && (SoftPrimWork.fog.base == NULL || SoftPrimWork.fog.type != BR_PMT_INDEX_8))
        return;

    const bool interpolated = (S == SP_SHADE_INTERP_I);

    brp_vertex *src[3] = {v0, v1, v2};
    TexVert     in[3];

    for(int k = 0; k < 3; k++) {
        in[k].x  = BrFloatToFixed(src[k]->comp[C_SX]);
        in[k].y  = BrFloatToFixed(src[k]->comp[C_SY]);
        in[k].z  = BrFloatToFixed(src[k]->comp[C_SZ]);
        in[k].u  = BrFloatToFixed(src[k]->comp[C_U]);
        in[k].v  = BrFloatToFixed(src[k]->comp[C_V]);
        in[k].i  = BrFloatToFixed(src[k]->comp[C_I]);
        in[k].rx = src[k]->comp[C_SX];
        in[k].ry = src[k]->comp[C_SY];
        in[k].rz = src[k]->comp[C_SZ];
        in[k].ru = src[k]->comp[C_U];
        in[k].rv = src[k]->comp[C_V];
        in[k].ri = src[k]->comp[C_I];

        if(!isfinite(in[k].rx) || !isfinite(in[k].ry) || !isfinite(in[k].ru) || !isfinite(in[k].rv))
            return;
    }

    ApplyIntensity(in, S, interpolated);

    TriSetup ts;

    if(!BuildTriSetup(in, ts))
        return;

    /*
     * TriangleSetup_ZT_ARBITRARY / TriangleSetup_ZTI_ARBITRARY: SETUP_FLOAT is
     * inside BuildTriSetup, and the parameters convert with fp_conv_d16 for z
     * and i (i only for the lit block) and fp_conv_d24 for u and v.
     */
    TrapParam pz, pu, pv, pi;

    SetupFloatParam<16>(pz, ts, ts.orig[0].rz, ts.orig[1].rz, ts.orig[2].rz, ts.v[0].rz, true);
    SetupFloatParam<24>(pu, ts, ts.orig[0].ru, ts.orig[1].ru, ts.orig[2].ru, ts.v[0].ru, false);
    SetupFloatParam<24>(pv, ts, ts.orig[0].rv, ts.orig[1].rv, ts.orig[2].rv, ts.v[0].rv, false);

    if(S != SP_SHADE_NONE)
        SetupFloatParam<16>(pi, ts, ts.orig[0].ri, ts.orig[1].ri, ts.orig[2].ri, ts.v[0].ri, false);
    else
        pi = TrapParam{};

    DivideWalk w;

    w.colour  = &colour;
    w.depth   = &dbuf;
    w.texture = &texture;
    w.shade   = &shade;
    w.tex     = BuildArbitraryTex(pu, pv, texture.width_p, texture.height, texture.stride_b);
    w.pz      = pz;
    w.pi      = pi;

    /* ARBITRARY_SETUP's rasteriser entry: the scanline starts from workspaceA. */
    w.su  = w.tex.su;
    w.svf = w.tex.svf;
    w.sv  = w.tex.sv;

    w.main_i   = ts.walk.xm;
    w.main_d   = ts.walk.d_xm;
    w.main_f   = (br_uint_32)ts.walk.xm_f;
    w.main_d_f = (br_uint_32)ts.walk.d_xm_f;

    w.scan  = colour.base + (br_size_t)ts.walk.yTop * colour.stride_b - 1;
    w.zscan = ((D == SP_DEPTH_ZW) ? dbuf.base : colour.base) + (br_size_t)ts.walk.yTop * ((D == SP_DEPTH_ZW) ? dbuf.stride_b : 0) - 2;

    /* Top trapezium: the row of the top vertex up to the middle one. */
    w.minor_i     = ts.walk.x1;
    w.minor_d     = ts.walk.d_x1;
    w.minor_count = ts.walk.topCount;

    if(ts.asm_dir == 0)
        DivideTrapezium<D, S, B, G, RPD_FWD>(w);
    else
        DivideTrapezium<D, S, B, G, RPD_BWD>(w);

    /* Bottom trapezium: the row of the middle vertex up to the bottom one. */
    w.minor_i     = ts.walk.x2;
    w.minor_d     = ts.walk.d_x2;
    w.minor_count = ts.walk.bottomCount;

    if(ts.asm_dir == 0)
        DivideTrapezium<D, S, B, G, RPD_FWD>(w);
    else
        DivideTrapezium<D, S, B, G, RPD_BWD>(w);
}

/*
 * One INDEX_8 decal triangle.
 *
 * decal.asm's remap is not a per-pixel lookup like the other ROPs - it is two
 * passes over the same triangle with the vertex intensity rescaled twice. The
 * kernel reads the material's index band, remaps each vertex intensity with the
 * x87 sequence at decal.asm:32-98, and rasterises:
 *
 *   1. the untextured kernel with i' = i*range/256 + base/256, which writes the
 *      decal's flat colour over the whole triangle, and writes depth;
 *   2. the textured kernel with i' = i*shade_height/256, which looks the texel
 *      up as shade[(i' << 8) | texel] and writes it where the texel is
 *      non-zero. The second pass's depth test sees the first pass's identical
 *      z, so `newz <= oldz` holds and every textured pixel lands.
 *
 * The two rescalings round to single the way the asm's fstp dword does, and
 * fp_1_256 is the exact 1/256 it multiplies by.
 */
template <sp_depth D, sp_shade S, sp_addr A>
void DecalTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    const softprim_buffer& shade = SoftPrimWork.shade;

    if(shade.base == NULL || shade.type != BR_PMT_INDEX_8)
        return;

    const long double range_256 = (long double)SoftPrimWork.index_range * 0.00390625L;
    const long double base_256  = (long double)SoftPrimWork.index_base * 0.00390625L;
    const long double shade_256 = (long double)shade.height * 0.00390625L;

    brp_vertex *src[3] = {v0, v1, v2};
    brp_vertex  pass1[3];
    brp_vertex  pass2[3];

    for(int k = 0; k < 3; k++) {
        const long double vi = (long double)(float)src[k]->comp[C_I];

        pass1[k] = pass2[k] = *src[k];

        pass1[k].comp[C_I] = (br_scalar)(float)(vi * range_256 + base_256);
        pass2[k].comp[C_I] = (br_scalar)(float)(vi * shade_256);
    }

    /* Pass 1: the decal colour band, untextured. */
    IndexedTriangle<D, S, SP_FOG_NONE>(block, &pass1[0], &pass1[1], &pass1[2]);

    /* Pass 2: the texel through the shade table, where the texel is non-zero. */
    if(A == SP_ADDR_DIVIDE)
        TexturedDivideTriangle<D, S, SP_BLEND_NONE, SP_FOG_NONE>(block, &pass2[0], &pass2[1], &pass2[2]);
    else
        TexturedIndexedTriangle<D, S, SP_PERSP_AFFINE, SP_BLEND_NONE, SP_FOG_NONE>(block, &pass2[0], &pass2[1], &pass2[2]);
}

/*
 * ---------------------------------------------------------------------------
 * The "perfect scan, integer vertices" perspective mapper.
 *
 * The dithered-map blocks are the INDEX_8 perspective mappers that do not use
 * the float setup above at all. PRIMF_DITHER_MAP selects
 * TriangleRenderPIZ2TPD* / TriangleRenderPITPD* (perspz.c:63-110,
 * persp.c:63-110), whose setup is perspzi.h / perspi.h: the screen coordinates
 * are the integer parts of their 16.16 components, the edge slopes come from a
 * 16.16 reciprocal table rather than from the x87, and the span is stepped by a
 * 32-bit accumulator whose carry selects the per-scanline parameter increment.
 * That is a different sampling of the same triangle - the float walk above and
 * this one choose different pixels along the same edge - so it is ported as its
 * own setup and walk rather than folded into BuildTriSetup, which would measure
 * better while being wrong.
 *
 * The span is ScanLinePIZ2TPD (ti8_pizp.asm:1681-1758, z-buffered) or
 * ScanLinePITPD (ti8_pip.asm:2064-2141, z-sorted). Both dither the texture
 * coordinate rather than the colour: a 4x4 Bayer matrix whose row is the
 * scanline's y and whose column is the low two bits of the destination address
 * is added to the projective u and v numerators at the start of the span, and
 * maintained thereafter by the difference between adjacent matrix entries.
 *
 * awtmi.h - the arbitrary-width RGB family (TriangleRenderPIZ2TA*,
 * TriangleRenderPIZ2TIA_RGB_555/888) - and tt15_piz.asm / tt16_piz.asm /
 * tt24_piz.asm (the untextured interpolated-colour blocks) run the same
 * geometry: the same sort, the same _reciprocal slopes, the same
 * SafeFixedMac2Div parameter setup and the same carry-driven trapezium walk.
 * Only the fragment and the addressing differ. This setup is the part they
 * share; the RGB kernels do not use it yet.
 * ---------------------------------------------------------------------------
 */

/* perspzi.h:27-35 incu/decu/incv/decv. The texel address is packed as
 * v*SIZE | u, so a column step wraps inside the row and a row step wraps modulo
 * SIZE*SIZE. The asm spells the same thing as a shl by log2(SIZE), partial
 * register add/sub and a shr/mask pair, which cannot be reproduced for a
 * runtime map size and does not need to be - these are the macros perspzi.h
 * defines for the non-Watcom build of the same source. */
inline br_int_32 PizIncU(br_int_32 a, br_int_32 size) noexcept
{
    return ((a + 1) & (size - 1)) | (a & ~(size - 1));
}

inline br_int_32 PizDecU(br_int_32 a, br_int_32 size) noexcept
{
    return ((a - 1) & (size - 1)) | (a & ~(size - 1));
}

inline br_int_32 PizIncV(br_int_32 a, br_int_32 size) noexcept
{
    return (a + size) & (size * size - 1);
}

inline br_int_32 PizDecV(br_int_32 a, br_int_32 size) noexcept
{
    return (a - size) & (size * size - 1);
}

/*
 * Force a float assignment to binary32.
 *
 * The setup below is a chain of C float assignments, and the pixels depend on
 * where each one of them rounds. The casts in this file say where that is, but
 * on the i386 x87 build a cast alone does not do it: gcc's default
 * -fexcess-precision=fast lets the x87 carry its extra precision through an
 * assignment, so `(float)x` can stay extended and a value the C rounds is not
 * rounded here. pentprim's values are the C's rounded ones wherever its
 * compiler put them in memory, so an assignment that does not happen moves a
 * sample - measured, not assumed: with these assignments enforced the last two
 * differing pixels of scene-shade-p256-flat-persp at 15bpp z-sorted disappear.
 * The one place the reference does *not* round is the maxuv accumulator, which
 * is why that one is extended here instead (see its note below).
 */
inline float PizRounded(long double v) noexcept
{
    volatile float f = (float)v;

    return f;
}

/*
 * rcp.c's _reciprocal[]: 0x10000/n for n >= 1 (index 0 holds 0x10000), which
 * the table spells out for 2048 entries. Interpreting the table rather than
 * reading it keeps this source free of an 8KB constant, and the values are
 * identical for every index the walk can reach: the integer edge heights are
 * bounded by the screen height.
 */
inline br_uint_32 PizReciprocal(br_int_32 y) noexcept
{
    if(y < 1)
        return 0x10000u;

    return 0x10000u / (br_uint_32)y;
}

/*
 * frcp.c's frcp[]: 1/n for n in 1..99, the float reciprocal perspzi.h uses for
 * a small positive divisor instead of dividing. 1.0f/(float)n is correctly
 * rounded here, which is what the initialiser of the table is - and it is a
 * table entry, so the value is a float and not the extended quotient the x87
 * would otherwise carry on with.
 */
inline float PizFrcp(br_int_32 n) noexcept
{
    return PizRounded(1.0L / (long double)n);
}

/*
 * safediv.asm's SafeFixedMac2Div(pa,pb,pc,pd,pe) = (pa*pb + pc*pd)/pe in 64
 * bits, truncated toward zero, and 0 when pe is zero or the magnitude of the
 * quotient does not fit in 32 bits (the asm's `cmp edx,ecx; jae zero`, which
 * tests the high half of the magnitude against the divisor). A magnitude in
 * [2^31,2^32) is not rejected: the asm applies the sign to the low 32 bits, so
 * it wraps.
 */
inline br_int_32 SafeFixedMac2Div(br_int_32 pa, br_int_32 pb, br_int_32 pc, br_int_32 pd, br_int_32 pe) noexcept
{
    if(pe == 0)
        return 0;

    const long long num = (long long)pa * pb + (long long)pc * pd;
    const long long den = pe;
    const bool      neg = (num < 0) != (den < 0);

    const unsigned long long m = (num < 0) ? (unsigned long long)-(num + 1) + 1 : (unsigned long long)num;
    const unsigned long long d = (den < 0) ? (unsigned long long)-(den + 1) + 1 : (unsigned long long)den;
    const unsigned long long q = m / d;

    if(q >= 0x100000000ull)
        return 0;

    const br_uint_32 r = (br_uint_32)q;

    return (br_int_32)(neg ? (br_uint_32)(0u - r) : r);
}

/*
 * ti8_pizp.asm:1078-1246 / ti8_pip.asm:1492-1660's `dither` macro: a += b*frac/16
 * for frac in -15..16, built as a chain of arithmetic shifts so that each term
 * is floor(b / 2^k) and the shifts are cumulative. The term for bit k of |frac|
 * (counting 16 >> k) needs a cumulative shift of k, so the chain walks the bits
 * from 16 down to 1 and shifts by the difference from the previous one. A
 * negative frac subtracts the same terms.
 */
inline br_int_32 PizDither(br_int_32 a, br_int_32 b, br_int_32 frac) noexcept
{
    const br_int_32 mag = (frac < 0) ? -frac : frac;
    const bool      sub = (frac < 0);
    br_int_32       cum = 0;

    for(br_int_32 k = 0; k <= 4; k++) {
        if((mag & (16 >> k)) == 0)
            continue;

        b >>= (k - cum);
        cum = k;
        a += sub ? -b : b;
    }

    return a;
}

/*
 * ti8_pizp.asm:1745-1756's four Bayer rows, selected by work.tsl.y & 3. The
 * column within a row is the low two bits of the destination address, which for
 * a buffer whose rows are a multiple of four bytes is the pixel's x.
 */
constexpr br_int_32 PIZ_BAYER[4][4] = {
    {13, 1,  4,  16},
    {8,  12, 9,  5 },
    {10, 6,  7,  11},
    {3,  15, 14, 2 },
};

/* One vertex in the representation the setup works in: the fixed-point
 * components, which is what softrend's BrFloatToFixed conversion would have
 * handed perspz.c. */
struct PizVert {
    br_int_32 x, y, z, u, v, w, i;
    br_int_32 r, g, b;
    br_int_32 sy; /* comp_x[C_SY], the sort key */
};

/* perspzi.h's per-parameter walk state. grad_y is only read by the setup; the
 * walk in the span carries grad_x and the two per-scanline increments. */
struct PizParam {
    br_int_32 grad_x, grad_y;
    br_int_32 current;
    br_int_32 d_nocarry, d_carry;
};

/*
 * The walk state perspz.c keeps in `work`. start/end/zstart are byte offsets
 * from the buffer bases rather than pointers: the C compares the addresses and
 * the asm compares the registers, both of which are a signed comparison of the
 * offsets whenever the sum cannot leave the address space, and an offset keeps
 * the walk to at most one byte outside the buffer legal to compute.
 */
struct PizWalk {
    br_int_32 top_x, top_y, bot_x, bot_y, main_x, main_y;

    br_uint_32 main_f;
    br_int_32  top_d_f, bot_d_f, main_d_f;
    br_int_32  top_d_i, bot_d_i, main_d_i;
    br_int_32  top_count, bot_count;

    /* the triangle's determinant, which every parameter gradient divides by */
    br_int_32 divisor;

    br_int_32 direction; /* g_divisor >= 0: the major edge is the left one */
    br_int_32 y;

    /* The numerator of u = base_u + pu.current/pq.current, its per-pixel step
     * and its per-scanline step, and the same for v. `nocarry` is the step for
     * a scanline whose major-edge step did not carry. */
    br_int_32 pu_current, pu_grad_x, pu_d_nocarry;
    br_int_32 pv_current, pv_grad_x, pv_d_nocarry;
    br_int_32 pq_current, pq_grad_x, pq_d_nocarry;
    br_int_32 pz_current, pz_grad_x, pz_d_nocarry;

    /* The intensity is affine, not projective, so it is one more ramp like the
     * depth and is set up only for the LIGHT cells. */
    br_int_32 pi_current, pi_grad_x, pi_d_nocarry;

    /* The numerators are carried relative to the base texel, so these
     * projective steps replace the ones above once the base point has been
     * factored out. */
    br_int_32 du_nocarry, dv_nocarry;
    br_int_32 source; /* packed v*size | u */

    br_int_32 start, end;
    br_int_32 zstart;
};

/*
 * perspzi.h:225-234's PARAM_SETUP: the 16.16 x and y gradients of one vertex
 * component, the value at the sampled start point, and the two per-scanline
 * increments the major edge's carry selects. `unsign` is PARAM_SETUP_UNSIGNED's
 * xor, which remaps the signed depth ramp onto the unsigned depth buffer; the
 * z-sorted file (perspi.h:225-234) uses the plain PARAM_SETUP.
 */
inline void PizParamSetup(PizParam& p, br_int_32 p0, br_int_32 p1, br_int_32 p2, const PizWalk& w, bool unsign) noexcept
{
    const br_int_32 dp1 = (br_int_32)((br_uint_32)p1 - (br_uint_32)p0);
    const br_int_32 dp2 = (br_int_32)((br_uint_32)p2 - (br_uint_32)p0);

    p.grad_x = SafeFixedMac2Div(dp1, w.main_y, -dp2, w.top_y, w.divisor);
    p.grad_y = SafeFixedMac2Div(dp2, w.top_x, -dp1, w.main_x, w.divisor);

    const br_int_32 half = (w.divisor >= 0) ? p.grad_x / 2 : -p.grad_x / 2;

    p.current = (br_int_32)((br_uint_32)p0 + (br_uint_32)half);

    p.d_nocarry = w.main_d_i * p.grad_x + p.grad_y;
    p.d_carry   = p.d_nocarry + p.grad_x;

    if(unsign)
        p.current = (br_int_32)((br_uint_32)p.current ^ 0x80000000u);
}

/*
 * The two PDIVIDE loops of one numerator (perspzi.h:294-320). Both need a
 * positive denominator to terminate - the C's `while` would spin on a
 * non-positive one and the asm's `jge`/`jnc` forms would too, since subtracting
 * a negative denominator moves the numerator the wrong way - and pq.current is
 * neither guaranteed positive nor constant along the walk, so the correction is
 * skipped rather than hung.
 *
 * Each step's two corrections are the denominator's own per-pixel gradient and
 * its per-scanline step: the numerator's gradient moves as the numerator is
 * re-based, and its scanline step moves as the denominator does.
 */
inline void PizDivideU(br_int_32& num, br_int_32& grad, br_int_32& nocarry, br_int_32& source, br_int_32 den, br_int_32 d_grad,
                       br_int_32 d_nocarry, br_int_32 size) noexcept
{
    while(num >= den) {
        num -= den;
        source = PizIncU(source, size);
        grad += d_grad;
        nocarry += d_nocarry;
    }

    while(num < 0) {
        num += den;
        source = PizDecU(source, size);
        grad -= d_grad;
        nocarry -= d_nocarry;
    }
}

inline void PizDivideV(br_int_32& num, br_int_32& grad, br_int_32& nocarry, br_int_32& source, br_int_32 den, br_int_32 d_grad,
                       br_int_32 d_nocarry, br_int_32 size) noexcept
{
    while(num >= den) {
        num -= den;
        source = PizIncV(source, size);
        grad += d_grad;
        nocarry += d_nocarry;
    }

    while(num < 0) {
        num += den;
        source = PizDecV(source, size);
        grad -= d_grad;
        nocarry -= d_nocarry;
    }
}

/*
 * perspzi.h's PDIVIDE: both numerators, against the same denominator, with the
 * denominator's gradient and scanline step as the corrections. perspzi.h
 * applies the u loops and then the v loops, each a pair of whiles; the span
 * repeats the same correction per pixel against the asm's if/else structure.
 */
inline void PizDivide(PizWalk& w, br_int_32 size) noexcept
{
    if(w.pq_current <= 0)
        return;

    PizDivideU(w.pu_current, w.pu_grad_x, w.du_nocarry, w.source, w.pq_current, w.pq_grad_x, w.pq_d_nocarry, size);
    PizDivideV(w.pv_current, w.pv_grad_x, w.dv_nocarry, w.source, w.pq_current, w.pq_grad_x, w.pq_d_nocarry, size);
}

/*
 * One scanline of a dithered INDEX_8 perspective span: ScanLinePIZ2TPD
 * (ti8_pizp.asm:1251-1758) with D selecting the z-buffered body, and
 * ScanLinePITPD (ti8_pip.asm:1490-2141) the z-sorted one.
 *
 * The asm keeps its own copies of the shared walk state for the duration of the
 * scanline and writes none of them back except the texel address, which it does
 * not write back either - work.tsl.source is what the caller's PDIVIDE left. So
 * the numerator and the texel column advance between scanlines only through the
 * per-scanline step and that PDIVIDE, and the span's interior is derived from
 * the scanline's start each time.
 */
template <sp_depth D>
void PizDitherSpan(PizWalk& w, const softprim_buffer& colour, const softprim_buffer& depth, const softprim_buffer& texture, br_int_32 size) noexcept
{
    const br_uint_8 *tex = texture.base;
    const br_int_32 *mrow;

    br_int_32  u_num  = w.pu_current;
    br_int_32  v_num  = w.pv_current;
    br_int_32  du_num = w.pu_grad_x;
    br_int_32  dv_num = w.pv_grad_x;
    br_int_32  den    = w.pq_current;
    br_uint_32 z      = (br_uint_32)w.pz_current;
    br_int_32  source = w.source;
    br_int_32  dest   = w.start;
    br_int_32  end    = w.end;
    br_int_32  zdest  = w.zstart;

    if(dest == end)
        return;

    const bool forward = (dest < end);

    /*
     * The matrix row is work.tsl.y & 3 and the column is the low two bits of
     * the destination address, not of x: the asm masks the real pointer, and a
     * buffer whose base is not four-aligned shifts the pattern with it.
     */
    mrow = PIZ_BAYER[(br_uint_32)w.y & 3];

    br_int_32 col = (br_int_32)(((br_uintptr_t)colour.base + (br_uintptr_t)(br_uint_32)dest) & 3);

    u_num = PizDither(u_num, den, mrow[col]);
    v_num = PizDither(v_num, den, mrow[col]);

    /* Bounds for the destination stores. The walk's start/end can sit one byte
     * outside the visible region for a primitive clipped hard against an edge -
     * the asm writes there and the byte is invisible either way - so it is
     * skipped rather than written into whatever the pixelmap was allocated
     * with. */
    const br_size_t limit = (br_size_t)colour.stride_b * (br_size_t)colour.height;

    for(;;) {
        /*
         * The in-span PDIVIDE, against the same denominator the C-level one
         * used. ti8_pizp.asm:1332-1408 tests u before v and v's negative case
         * before its positive one; the conditions are disjoint, so the order
         * only decides which pair of register writes happens first.
         */
        if(den > 0) {
            if(u_num >= den) {
                do {
                    source = PizIncU(source, size);
                    du_num += w.pq_grad_x;
                    u_num -= den;
                } while(u_num >= den);
            } else if(u_num < 0) {
                do {
                    source = PizDecU(source, size);
                    du_num -= w.pq_grad_x;
                    u_num += den;
                } while(u_num < 0);
            }

            if(v_num < 0) {
                do {
                    source = PizDecV(source, size);
                    dv_num -= w.pq_grad_x;
                    v_num += den;
                } while(v_num < 0);
            } else if(v_num >= den) {
                do {
                    source = PizIncV(source, size);
                    dv_num += w.pq_grad_x;
                    v_num -= den;
                } while(v_num >= den);
            }
        }

        /*
         * The per-pixel numerator step and the dither (ti8_pizp.asm:1410-1440,
         * ti8_pip.asm:1827-1860). A right-to-left span steps both numerators up
         * and the denominator down - ti8_pizp.asm:1443-1450 is `add esi,ebp;
         * add ebx,edi; sub edx,ecx` - so the direction is in the step, not only
         * in which end of the pixel the walk stops.
         *
         * The span's column advances with its address, so the matrix entry it
         * is moving to differs from the one it is leaving by a whole number of
         * sixteenths, and that difference is what the shift chain applies. The
         * bodies are unrolled four ways in the asm, which is a no-op once the
         * difference is computed.
         */
        const br_int_32 next  = forward ? ((col + 1) & 3) : ((col + 3) & 3);
        const br_int_32 delta = mrow[next] - mrow[col];

        if(forward) {
            u_num -= du_num;
            v_num -= dv_num;
            den += w.pq_grad_x;
        } else {
            u_num += du_num;
            v_num += dv_num;
            den -= w.pq_grad_x;
        }

        u_num = PizDither(u_num, den, delta);
        v_num = PizDither(v_num, den, delta);

        if(forward) {
            if(dest >= end)
                break;
        } else if(dest <= end) {
            break;
        }

        /*
         * The pixel. The asm tests depth first and the texel second, and both
         * gate both stores, so a transparent texel or a failed depth test
         * leaves the depth buffer alone.
         *
         * The two addresses are checked against their buffers first: the walk's
         * destination can sit one byte either side of the visible region for a
         * primitive clipped hard against an edge, which the asm writes to and
         * which is invisible either way. Skipping it keeps the write out of
         * whatever the pixelmap was allocated with, and the colour and depth
         * rows are parallel so the two checks agree.
         */
        const bool in_colour = (dest >= 0) && ((br_size_t)dest < limit);
        const bool in_depth = (D != SP_DEPTH_ZW) || (zdest >= 0 && (br_size_t)zdest + 1 < (br_size_t)depth.stride_b * (br_size_t)depth.height);

        if(in_colour && in_depth) {
            const br_uint_8 texel = tex[source];

            if(texel != 0) {
                bool draw = true;

                if(D == SP_DEPTH_ZW) {
                    const br_uint_16 newz = (br_uint_16)(z >> 16);

                    /* `ror`/`cmp bx,dx; ja`: draw while the new value is not
                     * greater than the stored one. */
                    if(newz > LoadDepth(depth.base + zdest))
                        draw = false;
                    else
                        StoreDepth(depth.base + zdest, newz);
                }

                if(draw)
                    colour.base[dest] = texel;
            }
        }

        if(D == SP_DEPTH_ZW) {
            if(forward)
                z += (br_uint_32)w.pz_grad_x;
            else
                z -= (br_uint_32)w.pz_grad_x;

            zdest += forward ? 2 : -2;
        }

        dest += forward ? 1 : -1;
        col = next;
    }
}

/*
 * One scanline of the RGB shade-table cell: t15_pip.asm's ScanLinePITIP with
 * its RGB fragment - the LIGHT instantiation of perspi.h, whose ScanLinePITIP
 * is the SNAME for persp.c's BPP 2 / SIZE 256 / LIGHT 1 cells (the 555/565
 * PITIP256 and PITIPB256 blocks).
 *
 * The walk is the dither span's - the same perspi.h setup, its own copies of
 * the walk state for the scanline's duration, and no write-back - but the order
 * of the asm's loop is this file's own: the pixel at the scanline's start is
 * stored as the C-level PDIVIDE left it, and only then does the loop step and
 * adjust for the next pixel (t15_pip.asm:66-169). Both orders place the same
 * sample at the same pixel - measured, not assumed: the dithered span's order
 * was built first and the frames are identical either way - so neither is a
 * correction of the other.
 *
 * The fragment is the shade table's own word: the intensity is the row and the
 * texel is the column, the entry is read as a 16-bit value, and a texel of 0 is
 * transparent. The intensity steps by pi.grad_x a pixel, in the direction of
 * the walk, and never carries between pixels - only the trapezium's per-row
 * step moves it down.
 *
 * D is deliberately absent: the two cells this serves are z-sorted only
 * (SP_DEPTH_NONE), because a 555/565 output is taken by the MMX table before
 * the general one whenever a depth buffer is present, so the z-buffered
 * PIZ2TIP256 shapes are unreachable and this span has no depth handling.
 *
 * The destination advances two bytes a pixel - this is a BPP 2 cell, not the
 * dither span's one - and the blend is pentprim's destination blend: half of
 * each, through the mask that clears every channel's low bit so no carry
 * crosses into its neighbour.
 */
template <sp_fmt F, sp_blend B>
void PizShadeSpan(PizWalk& w, const softprim_buffer& colour, const softprim_buffer& texture, const softprim_buffer& shade,
                  br_int_32 size) noexcept
{
    const br_uint_8 *tex  = texture.base;
    const br_uint_8 *stbl = shade.base;

    br_int_32  u_num  = w.pu_current;
    br_int_32  v_num  = w.pv_current;
    br_int_32  du_num = w.pu_grad_x;
    br_int_32  dv_num = w.pv_grad_x;
    br_int_32  den    = w.pq_current;
    br_int_32  intensity = w.pi_current;
    br_int_32  source = w.source;
    br_int_32  dest   = w.start;
    br_int_32  end    = w.end;

    if(dest == end)
        return;

    const bool forward = (dest < end);

    /* Bounds for the destination and for the shade table's flat read. The
     * destination can sit one pixel outside the visible region for a primitive
     * clipped hard against an edge, which the asm writes and which is invisible
     * either way, so it is skipped rather than written into whatever the
     * pixelmap was allocated with. The table is read at 2*index, the flat
     * offset the asm's `lea ecx,[ecx*2+eax]` forms. */
    const br_size_t limit       = (br_size_t)colour.stride_b * (br_size_t)colour.height;
    const br_size_t shade_limit = (br_size_t)shade.stride_b * (br_size_t)shade.height;

    const br_uint_16 mask = (F == SP_FMT_555) ? 0x7bde : 0xf7de;

    for(;;) {
        if(dest >= 0 && (br_size_t)dest + 1 < limit) {
            const br_uint_8 texel = tex[source];

            if(texel != 0) {
                const br_size_t off = (br_size_t)(((((br_uint_32)intensity >> 16) & 0xffu) << 8) | texel) * 2;

                if(off + 2 <= shade_limit) {
                    br_uint_16 v;

                    memcpy(&v, stbl + off, sizeof(v));

                    if(B == SP_BLEND_ALPHA) {
                        br_uint_16 dst;

                        memcpy(&dst, colour.base + dest, sizeof(dst));
                        v = (br_uint_16)(((v & mask) >> 1) + ((dst & mask) >> 1));
                    }

                    memcpy(colour.base + dest, &v, sizeof(v));
                }
            }
        }

        /* t15_pip.asm:99-105: the intensity steps a pixel in the walk's own
         * direction, and the destination by BPP, before the end test. */
        intensity += forward ? w.pi_grad_x : -w.pi_grad_x;
        dest += forward ? 2 : -2;

        if(forward ? (dest >= end) : (dest <= end))
            break;

        /*
         * The per-pixel step and the PDIVIDE, in the asm's order: the u
         * numerator and the denominator first, then the u correction loops,
         * then v (t15_pip.asm:115-163). The v step uses the denominator the u
         * step just produced, and du_num/dv_num are corrected in the same
         * place, so the next pixel's step uses them.
         */
        if(forward) {
            u_num -= du_num;
            v_num -= dv_num;
            den += w.pq_grad_x;
        } else {
            u_num += du_num;
            v_num += dv_num;
            den -= w.pq_grad_x;
        }

        /* The denominator is neither guaranteed positive nor constant along
         * the walk, and the asm's loops would spin on a non-positive one
         * (subtracting it moves the numerator the wrong way); the correction is
         * skipped instead of hung, as in the dither span's in-span PDIVIDE. */
        if(den > 0) {
            if(u_num >= den) {
                do {
                    source = PizIncU(source, size);
                    du_num += w.pq_grad_x;
                    u_num -= den;
                } while(u_num >= den);
            } else if(u_num < 0) {
                do {
                    source = PizDecU(source, size);
                    du_num -= w.pq_grad_x;
                    u_num += den;
                } while(u_num < 0);
            }

            if(v_num < 0) {
                do {
                    source = PizDecV(source, size);
                    dv_num -= w.pq_grad_x;
                    v_num += den;
                } while(v_num < 0);
            } else if(v_num >= den) {
                do {
                    source = PizIncV(source, size);
                    dv_num += w.pq_grad_x;
                    v_num -= den;
                } while(v_num >= den);
            }
        }
    }
}

/*
 * perspi.h's setup and trapezium walk, shared by the two cells that reach
 * persp.c through it. D and F select the cell: F == SP_FMT_I8 is the dithered
 * INDEX_8 map (BPP 1, no intensity, ScanLinePITPD), and F == SP_FMT_555/565 is
 * the 256x256 RGB shade-table cell (BPP 2, an intensity parameter and the
 * shade table, ScanLinePITIP). S and B are the shade-table cell's own two
 * variants, the intensity source and pentprim's destination blend; the
 * dithered cell takes no intensity and no blend. PizDitherTriangle and
 * PizShadeTriangle below name the two cells and fix those axes.
 *
 * The setup works entirely in the fixed components, and the ring of long double
 * values below is the x87: persp.c is a chain of float assignments, evaluated
 * on the i386 build in the x87's extended precision and rounded once per
 * assignment, and the results are truncated to integers later, so the roundings
 * are observable. Each assignment here is one extended expression rounded to
 * float, which is that rule.
 */
template <sp_depth D, sp_fmt F, sp_shade S, sp_blend B>
void PizTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    /* F is the destination's own format: the indexed map is the only one-byte
     * destination, and the shade-table cell's two formats are both two bytes.
     * LIGHT is whether perspi.h's PARAM_SETUP(work.pi) is instantiated, which
     * for these blocks is exactly the shade-table cell. */
    constexpr int  BPP   = (F == SP_FMT_I8) ? 1 : 2;
    constexpr bool LIGHT = (F != SP_FMT_I8);

    (void)block;

    const softprim_buffer& colour  = SoftPrimWork.colour;
    const softprim_buffer& dbuf    = SoftPrimWork.depth;
    const softprim_buffer& texture = SoftPrimWork.texture;
    const softprim_buffer& shade   = SoftPrimWork.shade;

    if(colour.type != (F == SP_FMT_I8 ? BR_PMT_INDEX_8 : RgbFormatType(F)) || colour.base == NULL || colour.width_p <= 0 ||
       colour.height <= 0)
        return;

    if(D == SP_DEPTH_ZW && (dbuf.type != BR_PMT_DEPTH_16 || dbuf.base == NULL))
        return;

    if(texture.base == NULL || texture.type != BR_PMT_INDEX_8)
        return;

    /* The shade table the LIGHT cells read is typed to the destination: the
     * texel is the column and the intensity the row, and the entry is the pixel
     * itself, so the two formats have to agree. */
    if(LIGHT && (shade.base == NULL || shade.type != RgbFormatType(F)))
        return;

    /*
     * The blocks are the four powers of two 64, 128, 256 and 1024, each
     * requiring a square map of that size, and the texel address the walk
     * builds is the flat offset v*SIZE + u, so a map with a wider row than its
     * texture would be sampled across the padding. The matcher has already
     * required the dimensions; this is the same requirement for the stride.
     */
    const br_int_32 size = texture.width_p;

    if(size != texture.height || texture.stride_b != size)
        return;

    if(size != 64 && size != 128 && size != 256 && size != 1024)
        return;

    brp_vertex *src[3] = {v0, v1, v2};
    PizVert     pv[3];

    for(int k = 0; k < 3; k++) {
        if(!isfinite(src[k]->comp[C_SX]) || !isfinite(src[k]->comp[C_SY]) || !isfinite(src[k]->comp[C_W]))
            return;

        if(LIGHT && !isfinite(src[k]->comp[C_I]))
            return;

        pv[k].x = BrFloatToFixed(src[k]->comp[C_SX]);
        pv[k].y = BrFloatToFixed(src[k]->comp[C_SY]);
        pv[k].z = BrFloatToFixed(src[k]->comp[C_SZ]);
        pv[k].u = BrFloatToFixed(src[k]->comp[C_U]);
        pv[k].v = BrFloatToFixed(src[k]->comp[C_V]);
        pv[k].w = BrFloatToFixed(src[k]->comp[C_W]);

        if(LIGHT)
            pv[k].i = BrFloatToFixed(src[k]->comp[C_I]);

        pv[k].sy = pv[k].y;
    }

    /*
     * A constant-intensity block's intensity is vertex 0's for the whole
     * triangle. pentprim's table marks those blocks `duplicate`, which softrend
     * honours as BR_PRIMF_CONST_DUPLICATE, but the constant surface op only
     * writes vertex 0 - so replicate here, as RgbAwtTriangle and
     * ApplyIntensity() do for their own constant cells.
     */
    if(S == SP_SHADE_CONST_I_RGB)
        pv[1].i = pv[2].i = pv[0].i;

    /* perspzi.h:99-132: sort by the SY component itself, not by its integer
     * part, which is what makes the parity of the sort irrelevant here. */
    {
        const auto SwapVert = [](PizVert& l, PizVert& r) {
            const PizVert t = l;
            l               = r;
            r               = t;
        };

        if(pv[0].sy > pv[1].sy) {
            if(pv[1].sy > pv[2].sy) {
                SwapVert(pv[0], pv[2]);
            } else if(pv[0].sy > pv[2].sy) {
                SwapVert(pv[0], pv[1]);
                SwapVert(pv[1], pv[2]);
            } else {
                SwapVert(pv[0], pv[1]);
            }
        } else if(pv[1].sy > pv[2].sy) {
            if(pv[0].sy > pv[2].sy) {
                SwapVert(pv[1], pv[2]);
                SwapVert(pv[0], pv[1]);
            } else {
                SwapVert(pv[1], pv[2]);
            }
        }
    }

    PizWalk w{};

    /* perspzi.h:137-141: the integer screen coordinates. */
    const br_int_32 sxa = pv[0].x >> 16, sya = pv[0].y >> 16;
    const br_int_32 sxb = pv[1].x >> 16, syb = pv[1].y >> 16;
    const br_int_32 sxc = pv[2].x >> 16, syc = pv[2].y >> 16;

    if(sya == syc)
        return;

    w.top_x  = sxb - sxa;
    w.top_y  = syb - sya;
    w.bot_x  = sxc - sxb;
    w.bot_y  = syc - syb;
    w.main_x = sxc - sxa;
    w.main_y = syc - sya;

    /* perspzi.h:143-149: the triangle's determinant and the scan direction. */
    w.divisor = (br_int_32)((br_uint_32)(w.top_x * w.main_y) - (br_uint_32)(w.main_x * w.top_y));

    if(w.divisor == 0)
        return;

    w.direction = (w.divisor >= 0) ? 1 : 0;

    /* Only a small positive divisor takes the table; the second arm of the C's
     * test is unreachable, since a zero divisor has already been rejected. */
    const float g_inverse = (w.divisor > 0 && w.divisor < 100) ? PizFrcp(w.divisor) : PizRounded(1.0L / (long double)w.divisor);

    /* perspzi.h:155-171: the base texel, rounded down to a whole texel with the
     * fraction carried separately. u and v are the C's integer quotients and
     * are zero by construction, because the difference the C takes them from
     * is the low 16 bits of a fixed value; the *float* neighbours au..cv are
     * the same difference before the division, so they are the base point's
     * fractional texel coordinate within the texel the walk starts on. The
     * distinction matters: taking the fraction from u/v loses it - the ramp
     * would start on the texel's corner instead of the point it was given. */
    const br_int_32 u_base16 = pv[0].u & (br_int_32)0xffff0000;
    const br_int_32 v_base16 = pv[0].v & (br_int_32)0xffff0000;
    const br_int_32 u_int    = (pv[0].u - u_base16) / 0x10000;
    const br_int_32 v_int    = (pv[0].v - v_base16) / 0x10000;

    float au = (float)(long double)(pv[0].u - u_base16) / 65536.0f;
    float bu = (float)(long double)(pv[1].u - u_base16) / 65536.0f;
    float cu = (float)(long double)(pv[2].u - u_base16) / 65536.0f;
    float av = (float)(long double)(pv[0].v - v_base16) / 65536.0f;
    float bv = (float)(long double)(pv[1].v - v_base16) / 65536.0f;
    float cv = (float)(long double)(pv[2].v - v_base16) / 65536.0f;

    const br_int_32 u_base = u_base16 / 0x10000;
    const br_int_32 v_base = v_base16 / 0x10000;

    /* perspzi.h:176-178: q'i is the product of the other two vertices' w. The
     * C converts each fixed component to float first, then multiplies. */
    const float wf0 = (float)pv[0].w;
    const float wf1 = (float)pv[1].w;
    const float wf2 = (float)pv[2].w;

    float aw = PizRounded((long double)wf1 * (long double)wf2);
    float bw = PizRounded((long double)wf2 * (long double)wf0);
    float cw = PizRounded((long double)wf1 * (long double)wf0);

    /*
     * perspzi.h:184-215: the maxuv normalisation, accumulated in the order the
     * C writes it - each product is folded in as it is formed.
     *
     * The accumulator itself is extended, not float. The C assigns every `+=`
     * back to the float variable, but on the i386 x87 build the accumulator
     * stays in a register for the whole chain and is never rounded until it is
     * used, and the pixels are that register's: the terms are of order 1e13,
     * where an f32 ulp is 8, so a float accumulator is one ulp out. That ulp
     * moves `norm`, then one scaled component, then a gradient by one unit, and
     * at a texel boundary that is a different texel. Measured against
     * pentprim's own x87 state on scene-shade-p256-flat-persp: its accumulator
     * is the exact sum 81280910385152 while the float one is 81280913899520,
     * and only the exact one reproduces pentprim's pixels. This accumulator is
     * the one value of this setup the reference keeps in a register; every other
     * assignment in it is a float it stores, so those are rounded here through
     * PizRounded above, and the two together are what closes the last differing
     * pixel of that fixture.
     */
    long double maxuv = (aw > 0.0f) ? (long double)aw : (long double)(-aw);

    maxuv += (long double)((au > 0.0f) ? au : -au);
    maxuv += (long double)((av > 0.0f) ? av : -av);

    au = PizRounded((long double)au * (long double)aw);
    maxuv += (long double)((au > 0.0f) ? au : -au);
    av = PizRounded((long double)av * (long double)aw);
    maxuv += (long double)((av > 0.0f) ? av : -av);
    bu = PizRounded((long double)bu * (long double)bw);
    maxuv += (long double)((bu > 0.0f) ? bu : -bu);
    bv = PizRounded((long double)bv * (long double)bw);
    maxuv += (long double)((bv > 0.0f) ? bv : -bv);
    cu = PizRounded((long double)cu * (long double)cw);
    maxuv += (long double)((cu > 0.0f) ? cu : -cu);
    cv = PizRounded((long double)cv * (long double)cw);
    maxuv += (long double)((cv > 0.0f) ? cv : -cv);

    const float norm = PizRounded(268435456.0L / maxuv);

    au = PizRounded((long double)au * (long double)norm);
    av = PizRounded((long double)av * (long double)norm);
    aw = PizRounded((long double)aw * (long double)norm);
    bu = PizRounded((long double)bu * (long double)norm);
    bv = PizRounded((long double)bv * (long double)norm);
    bw = PizRounded((long double)bw * (long double)norm);
    cu = PizRounded((long double)cu * (long double)norm);
    cv = PizRounded((long double)cv * (long double)norm);
    cw = PizRounded((long double)cw * (long double)norm);

    /* perspzi.h:218-253: the projective x and y gradients. Every expression
     * here is one float assignment in the C, so it is one extended-precision
     * expression rounded once to float, and the final truncation to br_int_32
     * is the C's cast. */
    const float zmx = PizRounded((long double)w.main_x * (long double)g_inverse);
    const float zmy = PizRounded((long double)w.main_y * (long double)g_inverse);
    const float ztx = PizRounded((long double)w.top_x * (long double)g_inverse);
    const float zty = PizRounded((long double)w.top_y * (long double)g_inverse);

    const float du1 = PizRounded((long double)bu - (long double)au);
    const float du2 = PizRounded((long double)cu - (long double)au);
    const float dv1 = PizRounded((long double)bv - (long double)av);
    const float dv2 = PizRounded((long double)cv - (long double)av);
    const float dq1 = PizRounded((long double)bw - (long double)aw);
    const float dq2 = PizRounded((long double)cw - (long double)aw);

    br_int_32 pu_grad_x = (br_int_32)((long double)du1 * (long double)zmy - (long double)du2 * (long double)zty);
    br_int_32 pu_grad_y = (br_int_32)((long double)du2 * (long double)ztx - (long double)du1 * (long double)zmx);
    br_int_32 pv_grad_x = (br_int_32)((long double)dv1 * (long double)zmy - (long double)dv2 * (long double)zty);
    br_int_32 pv_grad_y = (br_int_32)((long double)dv2 * (long double)ztx - (long double)dv1 * (long double)zmx);
    br_int_32 pq_grad_x = (br_int_32)((long double)dq1 * (long double)zmy - (long double)dq2 * (long double)zty);
    br_int_32 pq_grad_y = (br_int_32)((long double)dq2 * (long double)ztx - (long double)dq1 * (long double)zmx);

    w.pu_current = (br_int_32)au;
    w.pv_current = (br_int_32)av;
    w.pq_current = (br_int_32)aw;

    if(w.pq_current == 0)
        return;

    /* perspzi.h:258-274: the carry accumulators. main.grad is the major edge's
     * x slope in 16.16 out of the reciprocal table, d_f is that slope's fraction
     * in a 32-bit accumulator and d_i its integer part - the carry the walk
     * takes out of f is the fraction overflow that adds one to d_i. */
    const br_int_32 main_grad = (br_int_32)((br_uint_32)w.main_x * PizReciprocal(w.main_y));

    w.main_d_f = (br_int_32)((br_uint_32)main_grad << 16);
    w.main_d_i = main_grad >> 16;

    w.pu_d_nocarry = (br_int_32)((br_uint_32)(w.main_d_i * pu_grad_x) + (br_uint_32)pu_grad_y);
    w.pv_d_nocarry = (br_int_32)((br_uint_32)(w.main_d_i * pv_grad_x) + (br_uint_32)pv_grad_y);
    w.pq_d_nocarry = (br_int_32)((br_uint_32)(w.main_d_i * pq_grad_x) + (br_uint_32)pq_grad_y);

    const br_int_32 top_grad = w.top_y ? (br_int_32)((br_uint_32)w.top_x * PizReciprocal(w.top_y)) : 0;
    const br_int_32 bot_grad = w.bot_y ? (br_int_32)((br_uint_32)w.bot_x * PizReciprocal(w.bot_y)) : 0;

    w.top_d_f   = (br_int_32)((br_uint_32)top_grad << 16);
    w.top_d_i   = top_grad >> 16;
    w.top_count = syb - sya;

    w.bot_d_f   = (br_int_32)((br_uint_32)bot_grad << 16);
    w.bot_d_i   = bot_grad >> 16;
    w.bot_count = syc - syb;

    /* perspzi.h:226-234: the depth ramp, through PARAM_SETUP_UNSIGNED for the
     * z-buffered file and the plain PARAM_SETUP for the z-sorted one, whose
     * span does not read it. */
    {
        PizParam pz;

        PizParamSetup(pz, pv[0].z, pv[1].z, pv[2].z, w, D == SP_DEPTH_ZW);

        w.pz_grad_x    = pz.grad_x;
        w.pz_d_nocarry = pz.d_nocarry;
        w.pz_current   = pz.current;
    }

    /* perspi.h:225-234 with LIGHT: the intensity ramp, set up exactly as the
     * depth one and next to it. Its d_carry is unused here - the trapezium adds
     * grad_x on the major edge's carry instead, which is the same value. */
    if(LIGHT) {
        PizParam pi;

        PizParamSetup(pi, pv[0].i, pv[1].i, pv[2].i, w, false);

        w.pi_grad_x    = pi.grad_x;
        w.pi_d_nocarry = pi.d_nocarry;
        w.pi_current   = pi.current;
    }

    /* perspzi.h:275-296: the accumulator starts half a pixel in, and the two
     * projective numerators are re-based onto the integer texel the source
     * names. u and v are the C's integer parts of that base point, and are zero
     * by construction - the difference the C takes them from is a value's low
     * 16 bits, which cannot reach 0x10000 - but they are kept because every
     * term below is written through them. */
    w.main_f     = 0x80000000u;
    w.du_nocarry = (br_int_32)((br_uint_32)(u_int * w.pq_d_nocarry) - (br_uint_32)w.pu_d_nocarry);
    w.pu_grad_x  = (br_int_32)((br_uint_32)(u_int * pq_grad_x) - (br_uint_32)pu_grad_x);

    w.dv_nocarry = (br_int_32)((br_uint_32)(v_int * w.pq_d_nocarry) - (br_uint_32)w.pv_d_nocarry);
    w.pv_grad_x  = (br_int_32)((br_uint_32)(v_int * pq_grad_x) - (br_uint_32)pv_grad_x);

    w.pq_grad_x = pq_grad_x;

    w.pu_current = (br_int_32)((br_uint_32)w.pu_current - (br_uint_32)(u_int * w.pq_current));
    w.pv_current = (br_int_32)((br_uint_32)w.pv_current - (br_uint_32)(v_int * w.pq_current));

    /* perspzi.h:300-318: the packed texel address of the base point, and the
     * start of the span - the top vertex, one pixel to its left when the scan
     * runs right to left. */
    const br_int_32 back = (w.direction == 0) ? 1 : 0;

    w.source = (((v_int + v_base) & (size - 1)) * size) | ((u_int + u_base) & (size - 1));

    w.start  = sxa * BPP + sya * colour.stride_b - BPP * back;
    w.end    = w.start;
    w.zstart = sxa * 2 + ((D == SP_DEPTH_ZW) ? sya * dbuf.stride_b : 0) - back * 2;

    /* perspzi.h:307-323: shift the projective start by half a pixel. */
    if(w.divisor >= 0) {
        w.pq_current += w.pq_grad_x / 2;
        w.pu_current += -w.pu_grad_x / 2;
        w.pv_current += -w.pv_grad_x / 2;
    } else {
        w.pq_current -= w.pq_grad_x / 2;
        w.pu_current -= -w.pu_grad_x / 2;
        w.pv_current -= -w.pv_grad_x / 2;
    }

    /*
     * perspzi.h:329-397: the two trapezium halves.
     *
     * `start` is the major edge (a to c) and `end` the minor one (a to b, then
     * b to c); the span is entered when the major edge is strictly on the side
     * the direction says, which is why the top vertex's row is never drawn - at
     * that row both are initialised to the same pixel - and why the `end` of
     * the second half is reset to the middle vertex rather than carried.
     *
     * The counters and the two accumulators are the C's work.top.f/work.main.f;
     * the minor edge's is the one whose carry moves `end`, the major edge's the
     * one that moves `start`, the parameter walk and the depth.
     */
    w.y = sya;

    for(int half = 0; half < 2; half++) {
        const br_int_32  minor_i   = (half == 0) ? w.top_d_i : w.bot_d_i;
        const br_uint_32 minor_d_f = (half == 0) ? (br_uint_32)w.top_d_f : (br_uint_32)w.bot_d_f;
        const br_int_32  count     = (half == 0) ? w.top_count : w.bot_count;

        br_uint_32 minor_f = 0x80000000u;

        /* The second half's minor edge restarts at the middle vertex. */
        if(half == 1)
            w.end = sxb * BPP + syb * colour.stride_b - BPP * back;

        for(br_int_32 n = count; n > 0; n--) {
            if((w.direction != 0) ? (w.start < w.end) : (w.start > w.end)) {
                PizDivide(w, size);

                if(LIGHT)
                    PizShadeSpan<F, B>(w, colour, texture, shade, size);
                else
                    PizDitherSpan<D>(w, colour, dbuf, texture, size);
            }

            w.y++;

            minor_f += minor_d_f;

            const br_uint_32 minor_carry = (minor_f < minor_d_f) ? 1u : 0u;

            w.end += (minor_i + (br_int_32)minor_carry) * BPP + colour.stride_b;

            w.main_f += w.main_d_f;

            const br_uint_32 carry = (w.main_f < w.main_d_f) ? 1u : 0u;

            w.pq_current += w.pq_d_nocarry;
            w.pu_current += -w.du_nocarry;
            w.pv_current += -w.dv_nocarry;
            w.start += (w.main_d_i + (br_int_32)carry) * BPP + colour.stride_b;

            if(D == SP_DEPTH_ZW) {
                w.zstart += w.main_d_i * 2 + dbuf.stride_b + (br_int_32)carry * 2;
                w.pz_current += w.pz_d_nocarry;
            }

            if(LIGHT)
                w.pi_current += w.pi_d_nocarry;

            if(carry != 0u) {
                w.pq_current += w.pq_grad_x;
                w.pu_current += -w.pu_grad_x;
                w.pv_current += -w.pv_grad_x;

                if(LIGHT)
                    w.pi_current += w.pi_grad_x;

                if(D == SP_DEPTH_ZW)
                    w.pz_current += w.pz_grad_x;
            }
        }
    }
}

/*
 * The dithered-map INDEX_8 perspective cells: perspz.c's
 * TriangleRenderPIZ2TPD* (z-buffered) or persp.c's TriangleRenderPITPD*
 * (z-sorted), each of which is perspzi.h / perspi.h instantiated for a map
 * size. The dither itself is in PizDitherSpan; this only names the cell.
 */
template <sp_depth D>
void PizDitherTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    PizTriangle<D, SP_FMT_I8, SP_SHADE_NONE, SP_BLEND_NONE>(block, v0, v1, v2);
}

/*
 * The 256x256 RGB shade-table cells: persp.c's TriangleRenderPITIP256_RGB_* and
 * TriangleRenderPITIPB256_RGB_* (prm_t15.ifg:33-36, prm_t16.ifg:33-36), which
 * are perspi.h at BPP 2, SIZE 256, LIGHT 1. They are z-sorted only - the
 * z-buffered PIZ2TIP256 twins are taken by the MMX table before the general
 * one, so no depth parameter reaches PizShadeSpan.
 */
template <sp_fmt F, sp_shade S, sp_blend B>
void PizShadeTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    PizTriangle<SP_DEPTH_NONE, F, S, B>(block, v0, v1, v2);
}

/*
 * ---------------------------------------------------------------------------
 * RGB output.
 *
 * The 555/565/888 outputs share every part of the walk that the INDEX_8 kernels
 * use: the same SETUP_FLOAT geometry (BuildTriSetup, BuildWalkGeom), the same
 * SETUP_FLOAT_PARAM z ramp, and the same trapezium shape. What differs is the
 * fragment - a texel or an interpolated colour is packed into the destination
 * format instead of written as an index.
 *
 * Every textured RGB block in the tables is an *affine* rasteriser. The blocks
 * whose identifier says "Perspective Correct" are the `perspective_subdivide`
 * ones: pentprim subdivides in softrend (BR_PRIMF_SUBDIVIDE) and rasterises the
 * small triangles affinely, and softprim's matcher carries the same subdivide
 * flag. So there is no projective divide here - the perspective option only
 * decides whether softrend splits the triangle first.
 *
 * This is the general (non-MMX) path. The 15/16bpp ground truth in
 * scratch/gltfview-baseline.txt is pentprim's MMX family, which is a different
 * textured-RGB implementation with its own packing; softprim's general path
 * draws textured RGB correctly but does not reproduce those checksums, and the
 * MMX family is not yet ported. The MMX variants, when they land, select the
 * same tuples and slot in beside these kernels rather than replacing them.
 * ---------------------------------------------------------------------------
 */

inline br_uint_8 RgbFormatType(sp_fmt F) noexcept
{
    if(F == SP_FMT_555)
        return BR_PMT_RGB_555;

    if(F == SP_FMT_565)
        return BR_PMT_RGB_565;

    return BR_PMT_RGB_888;
}

/* Bytes per output pixel. */
constexpr inline br_int_32 RgbPixelBytes(sp_fmt F) noexcept
{
    return (F == SP_FMT_888) ? 3 : 2;
}

/* Floor-modulus into [0, n), used to wrap a texel coordinate a non-power-of-two
 * map (the power-of-two shift/mask blocks go through the same path, which is
 * equivalent for a stride == width map). */
inline br_int_32 WrapIndex(br_int_32 v, br_int_32 n) noexcept
{
    if(n <= 0)
        return 0;

    br_int_32 m = v % n;

    if(m < 0)
        m += n;

    return m;
}

/*
 * Decode one palette entry into 8-bit RGB. The palette's own type decides the
 * entry layout, so an INDEX_8 texture sampled into a 555 screen decodes through
 * a 555 palette and into a 888 screen through a 888 one.
 *
 * The 888-family pixelmaps are byte-ordered B,G,R(,A/X) in memory (see
 * core/pixelmap/pmclone.c's rgb_888_read/rgbx_888_read/rgba_8888_read), where
 * the 555/565 word types carry R in the high bits. The three byte formats are
 * not the same layout as the two word formats and must not be read as if they
 * were.
 */
inline bool PaletteEntry(const softprim_buffer& tex, br_uint_32 idx, br_uint_8& r, br_uint_8& g, br_uint_8& b) noexcept
{
    if(tex.palette == NULL || (br_int_32)idx >= tex.palette_size || (br_int_32)idx < 0)
        return false;

    const br_uint_8 *p = tex.palette + (br_size_t)idx * tex.palette_stride_b;

    switch(tex.palette_type) {
        case BR_PMT_RGB_555: {
            br_uint_16 v = (br_uint_16)(p[0] | ((br_uint_16)p[1] << 8));
            r            = (br_uint_8)(((v >> 10) & 0x1f) << 3);
            g            = (br_uint_8)(((v >> 5) & 0x1f) << 3);
            b            = (br_uint_8)((v & 0x1f) << 3);
            return true;
        }
        case BR_PMT_RGB_565: {
            br_uint_16 v = (br_uint_16)(p[0] | ((br_uint_16)p[1] << 8));
            r            = (br_uint_8)(((v >> 11) & 0x1f) << 3);
            g            = (br_uint_8)(((v >> 5) & 0x3f) << 2);
            b            = (br_uint_8)((v & 0x1f) << 3);
            return true;
        }
        case BR_PMT_RGB_888:
        case BR_PMT_RGBX_888:
        case BR_PMT_RGBA_8888:
            r = p[2];
            g = p[1];
            b = p[0];
            return true;
        default:
            return false;
    }
}

/* Pack 8-bit RGB into the destination format. */
template <sp_fmt F>
inline void RgbStore(br_uint_8 *p, br_uint_8 r, br_uint_8 g, br_uint_8 b) noexcept
{
    if(F == SP_FMT_555) {
        br_uint_16 v = (br_uint_16)(((br_uint_16)(r >> 3) << 10) | ((br_uint_16)(g >> 3) << 5) | (br_uint_16)(b >> 3));
        p[0]         = (br_uint_8)v;
        p[1]         = (br_uint_8)(v >> 8);
    } else if(F == SP_FMT_565) {
        br_uint_16 v = (br_uint_16)(((br_uint_16)(r >> 3) << 11) | ((br_uint_16)(g >> 2) << 5) | (br_uint_16)(b >> 3));
        p[0]         = (br_uint_8)v;
        p[1]         = (br_uint_8)(v >> 8);
    } else {
        /* BR_PMT_RGB_888 is byte-ordered B,G,R in memory. */
        p[0] = b;
        p[1] = g;
        p[2] = r;
    }
}

/*
 * ---------------------------------------------------------------------------
 * The arbitrary-width RGB_888 family: awtmi.h and t_piza.asm.
 *
 * TriangleRenderPIZ2TA24 (a bare `texture` block, so an RGB_888 map) and
 * TriangleRenderPIZ2TIA_RGB_888 (a `texture_index8` block, read through the
 * shade table) are awtmz.c instantiations of awtmi.h; their scanlines are
 * t_piza.asm's TriangleRenderZ2 macro. They are a second lineage over the same
 * perfect-scan geometry the PIZ dither mapper above uses:
 *
 *   - the setup sorts by SY and reduces the vertices to sar16 integers, takes
 *     its edge slopes from _reciprocal and its parameter gradients from
 *     SafeFixedMac2Div, and carries the same main/top/bot trapezium. It is
 *     affine - the perspective blocks are subdivided in softrend before they
 *     reach the rasteriser - so there is no projective divide, and the scan
 *     direction comes from a comparison of the top and bottom edge slopes
 *     rather than from the sign of the determinant;
 *
 *   - the addressing is work.awsl, which walks the texel coordinate rather than
 *     a packed v*SIZE|u index. The 16.16 fraction is carried in the high word
 *     of u_current/v_current, the wrapped texel column in u_int and the texel
 *     address in a byte pointer stepped by a byte delta, so a map of any width
 *     wraps without the power-of-two masking perspzi.h needs;
 *
 *   - the fragment is three bytes copied from the map (SBPP=3) or one byte
 *     looked up in the shade table (SBPP=1, LIGHT).
 *
 * This is deliberately not folded into RgbTriangle's SETUP_FLOAT geometry: the
 * two sample the same triangle differently, and mixing them would measure
 * better while being wrong.
 * ---------------------------------------------------------------------------
 */

/* awtmi.h's `powerof2`. */
inline bool AwtPowerOf2(br_int_32 v) noexcept
{
    return !(v & (v - 1));
}

/* awtmi.h's wrapped integer texel step: always in [-dim, -1]. The power-of-two
 * form masks and the general form takes `%`, which agree; the remainder is then
 * pulled negative whenever it came out non-negative. */
inline br_int_32 AwtStepInt(br_int_32 v, br_int_32 dim) noexcept
{
    const br_int_32 r = AwtPowerOf2(dim) ? (Sar16(v) & (dim - 1)) : (Sar16(v) % dim);

    return (r >= 0) ? r - dim : r;
}

/* awtmi.h's wrapped integer texel index: always in [0, dim). */
inline br_int_32 AwtIndex(br_int_32 v, br_int_32 dim) noexcept
{
    const br_int_32 r = AwtPowerOf2(dim) ? (Sar16(v) & (dim - 1)) : (Sar16(v) % dim);

    return (r < 0) ? r + dim : r;
}

/*
 * awtmi.h's work.awsl. u_current and v_current carry the low 16 bits of the
 * 16.16 texel coordinate - its fraction - in their high word, so a whole-word
 * add of the gradient accumulates the fraction and the carry out is the
 * whole-texel step. u_int carries the wrapped texel column biased by 0x8000,
 * whose bit 15 is the "the column has not wrapped" flag the asm tests, and
 * source is the byte offset of the current texel from the map base.
 */
struct RgbAwt {
    br_uint_32 u_current, v_current;
    br_uint_32 du, dv;

    br_uint_32 du_nocarry, du_carry, dv_nocarry, dv_carry;
    br_int_32  du_int_nocarry, du_int_carry, dv_int_nocarry, dv_int_carry;

    br_int_32 du_int, dv_int;
    br_int_32 dsource, dsource_nocarry, dsource_carry;

    br_uint_32 u_int;
    br_int_32  source;

    br_int_32 width, height, stride, size, sbpp;
};

/* One edge of the trapezium (awtmi.h's scan_edge). f is the fraction
 * accumulator, d_f its per-scanline step, d_i the whole-pixel step with the row
 * stride folded in, and count the scanlines left. */
struct AwtEdge {
    br_uint_32 f, d_f;
    br_int_32  d_i;
    br_int_32  count;
};

struct RgbAwtWalk {
    const softprim_buffer *colour;
    const softprim_buffer *depth;
    const softprim_buffer *texture;
    const softprim_buffer *shade;

    RgbAwt tex;

    PizParam pz; /* depth, 16.16, remapped unsigned */
    PizParam pi; /* intensity, 16.16; only the shade-table variant reads it */

    br_int_32 direction; /* 1: the per-pixel u/v step runs with the gradient */

    br_int_32 start, end, zstart;
    AwtEdge   top, bot;

    br_uint_32 main_f, main_d_f;
    br_int_32  main_d_i;

    br_int_32 pix_b; /* DBPP */
};

/*
 * awtmi.h's setup, without its four TNAME() calls. It is passed the sorted
 * vertices and returns false for a triangle the setup cannot process.
 */
inline bool RgbAwtSetup(RgbAwtWalk& w, const PizVert vx[3], const softprim_buffer& colour, const softprim_buffer& texture, br_int_32 sbpp,
                        bool light) noexcept
{
    const br_int_32 sxa = vx[0].x >> 16, sya = vx[0].y >> 16;
    const br_int_32 sxb = vx[1].x >> 16, syb = vx[1].y >> 16;
    const br_int_32 sxc = vx[2].x >> 16, syc = vx[2].y >> 16;

    const br_int_32 top_x = sxb - sxa, top_y = syb - sya;
    const br_int_32 bot_x = sxc - sxb, bot_y = syc - syb;
    const br_int_32 main_x = sxc - sxa, main_y = syc - sya;

    const br_int_32 stride_p = colour.stride_b / colour.bpp;

    const br_int_32 main_grad = (br_int_32)((br_uint_32)main_x * PizReciprocal(main_y));
    const br_int_32 top_grad  = top_y ? (br_int_32)((br_uint_32)top_x * PizReciprocal(top_y)) : 0;
    const br_int_32 bot_grad  = bot_y ? (br_int_32)((br_uint_32)bot_x * PizReciprocal(bot_y)) : 0;

    const br_int_32 top_count = syb - sya;
    const br_int_32 bot_count = syc - syb;

    const br_int_32 divisor = top_x * main_y - main_x * top_y;

    if(divisor == 0)
        return false;

    w.direction = ((top_count != 0 && top_grad >= main_grad) || (bot_count != 0 && bot_grad < main_grad)) ? 1 : 0;

    /* awtmi.h's PARAM_SETUP / PARAM_SETUP_UNSIGNED. The half-pixel start is
     * always +grad_x/2 here - only perspzi.h makes it depend on the sign of the
     * determinant. */
    const auto ParamSetup = [&](PizParam& p, br_int_32 p0, br_int_32 p1, br_int_32 p2, bool unsign) {
        const br_int_32 dp1 = p1 - p0;
        const br_int_32 dp2 = p2 - p0;

        p.grad_x = SafeFixedMac2Div(dp1, main_y, (br_int_32)(0u - (br_uint_32)dp2), top_y, divisor);
        p.grad_y = SafeFixedMac2Div(dp2, top_x, (br_int_32)(0u - (br_uint_32)dp1), main_x, divisor);

        const br_int_32 cur = (br_int_32)((br_uint_32)p0 + (br_uint_32)(p.grad_x / 2));

        p.current   = unsign ? (br_int_32)((br_uint_32)cur ^ 0x80000000u) : cur;
        p.d_nocarry = (br_int_32)((br_uint_32)(Sar16(main_grad) * p.grad_x) + (br_uint_32)p.grad_y);
        p.d_carry   = (br_int_32)((br_uint_32)p.d_nocarry + (br_uint_32)p.grad_x);
    };

    PizParam par_u, par_v, par_z, par_i{};

    ParamSetup(par_u, vx[0].u, vx[1].u, vx[2].u, false);
    ParamSetup(par_v, vx[0].v, vx[1].v, vx[2].v, false);
    ParamSetup(par_z, vx[0].z, vx[1].z, vx[2].z, true);

    if(light)
        ParamSetup(par_i, vx[0].i, vx[1].i, vx[2].i, false);

    w.pz = par_z;
    w.pi = par_i;

    RgbAwt& t = w.tex;

    t.width  = texture.width_p;
    t.height = texture.height;
    t.stride = texture.stride_b;
    t.size   = texture.height * texture.stride_b;
    t.sbpp   = sbpp;

    t.u_current  = (br_uint_32)par_u.current << 16;
    t.v_current  = (br_uint_32)par_v.current << 16;
    t.du_nocarry = (br_uint_32)par_u.d_nocarry << 16;
    t.du_carry   = (br_uint_32)par_u.d_carry << 16;
    t.dv_nocarry = (br_uint_32)par_v.d_nocarry << 16;
    t.dv_carry   = (br_uint_32)par_v.d_carry << 16;

    t.du_int_nocarry = AwtStepInt(par_u.d_nocarry, t.width);
    t.du_int_carry   = AwtStepInt(par_u.d_carry, t.width);
    t.u_int          = (br_uint_32)AwtIndex(par_u.current, t.width);

    const br_int_32 v_int = AwtIndex(par_v.current, t.height);

    t.dv_int_nocarry = AwtStepInt(par_v.d_nocarry, t.height);
    t.dv_int_carry   = AwtStepInt(par_v.d_carry, t.height);

    t.dsource_nocarry = t.du_int_nocarry * t.sbpp + t.stride * t.dv_int_nocarry;
    t.dsource_carry   = t.du_int_carry * t.sbpp + t.stride * t.dv_int_carry;
    t.source          = t.u_int * (br_uint_32)t.sbpp + (br_uint_32)(t.stride * v_int);
    t.u_int += 0x8000;

    /* The per-pixel step: the sign follows `direction`, the u column step is a
     * `%`-wrapped one whatever the map width, and the v row step is the same
     * value the per-scanline walk uses. */
    const br_int_32 du_grad = w.direction ? par_u.grad_x : (br_int_32)(0u - (br_uint_32)par_u.grad_x);
    const br_int_32 dv_grad = w.direction ? par_v.grad_x : (br_int_32)(0u - (br_uint_32)par_v.grad_x);

    t.du     = (br_uint_32)du_grad << 16;
    t.dv     = (br_uint_32)dv_grad << 16;
    t.du_int = AwtStepInt(du_grad, t.width);
    t.dv_int = AwtStepInt(dv_grad, t.height);

    t.dsource = t.du_int * t.sbpp + t.stride * t.dv_int;

    /* awtmi.h's `offset`: the top vertex's screen position as a pixel offset.
     * high16 is an unsigned 16-bit read of the fixed component's top half. */
    const br_int_32 offset = (br_int_32)(br_uint_16)sxa + (br_int_32)(br_uint_16)sya * stride_p;

    w.pix_b = colour.bpp;
    w.start = w.end = offset * w.pix_b;
    w.zstart        = offset * 2;

    w.top.f     = 0x80000000u;
    w.top.d_f   = (br_uint_32)top_grad << 16;
    w.top.d_i   = Sar16(top_grad) + stride_p;
    w.top.count = top_count;

    w.bot.f     = 0x80000000u;
    w.bot.d_f   = (br_uint_32)bot_grad << 16;
    w.bot.d_i   = Sar16(bot_grad) + stride_p;
    w.bot.count = bot_count;

    w.main_f   = 0x80000000u;
    w.main_d_f = (br_uint_32)main_grad << 16;
    w.main_d_i = Sar16(main_grad) + stride_p;

    return true;
}

/* t_piza.asm's per-pixel u/v/source step (`contf` / the backward loop's copy):
 * the fraction's carry is the whole-texel step, the wrapped column step is
 * added, the v fraction's carry adds a row, a column that has wrapped adds the
 * map width back, and the byte pointer is pulled into the map. */
inline void RgbAwtStep(const RgbAwt& t, br_uint_32& ua, br_uint_32& vc, br_int_32& src) noexcept
{
    const br_uint_32 usum   = ua + t.du;
    const br_uint_32 ucarry = (usum < t.du) ? 1u : 0u;

    ua = usum;
    src += t.dsource;
    src += (br_int_32)ucarry * t.sbpp;
    ua += ucarry;
    ua += (br_uint_32)t.du_int;

    const br_uint_32 vsum = vc + t.dv;

    if(vsum < t.dv)
        src += t.stride;

    vc = vsum;

    if((ua & 0x8000u) == 0) {
        ua += (br_uint_32)t.width;
        src += t.width * t.sbpp;
    }

    if(src < 0)
        src += t.size;
}

/*
 * The fragment of one drawn pixel: t_piza.asm's SBPP/DBPP/LIGHT arms. The texel
 * is transparent when all three of its bytes (SBPP=3), its two bytes (SBPP=2)
 * or its one byte (SBPP=1) are zero, and the depth test draws while the new
 * 16-bit value is not greater than the stored one, which is the asm's
 * `cmp dl,bl; sbb dh,bh; jb`.
 *
 * SBPP=2 is the 555/565-typed colour map: the texel is already the output
 * pixel's type, so the fragment is the two bytes copied - t_pia.asm's
 * `mov cl,[esi] / mov ch,1[esi] / mov [edi],cl / mov 1[edi],ch`. The byte order
 * is the map's, which for both word types is the native one.
 */
template <sp_fmt F, sp_tex X, sp_depth D>
inline void RgbAwtPixel(const RgbAwtWalk& w, const br_uint_8 *tex, br_size_t tex_size, br_int_32 sp, br_int_32 src, br_int_32 zp,
                        br_int_32 pzp, br_int_32 pip) noexcept
{
    const softprim_buffer& colour = *w.colour;
    const softprim_buffer& dbuf   = *w.depth;
    constexpr br_int_32   pix_b  = RgbPixelBytes(F);

    if(sp < 0 || (br_size_t)sp + (br_size_t)pix_b > (br_size_t)colour.stride_b * (br_size_t)colour.height)
        return;

    if(D == SP_DEPTH_ZW && (zp < 0 || (br_size_t)zp + 2 > (br_size_t)dbuf.stride_b * (br_size_t)dbuf.height))
        return;

    if(src < 0)
        return;

    bool transparent;

    if(X == SP_TEX_RGB888) {
        if((br_size_t)src + 3 > tex_size)
            return;

        const br_uint_8 *tp = tex + src;

        transparent = (tp[0] | tp[1] | tp[2]) == 0;
    } else if(X == SP_TEX_555 || X == SP_TEX_565) {
        if((br_size_t)src + 2 > tex_size)
            return;

        const br_uint_8 *tp = tex + src;

        transparent = (tp[0] | tp[1]) == 0;
    } else {
        if((br_size_t)src >= tex_size)
            return;

        transparent = tex[src] == 0;
    }

    if(transparent)
        return;

    if(D == SP_DEPTH_ZW) {
        const br_uint_16 newz = (br_uint_16)((br_uint_32)pzp >> 16);

        if(LoadDepth(dbuf.base + zp) < newz)
            return;

        StoreDepth(dbuf.base + zp, newz);
    }

    if(X == SP_TEX_RGB888) {
        const br_uint_8 *tp = tex + src;

        colour.base[sp + 0] = tp[0];
        colour.base[sp + 1] = tp[1];
        colour.base[sp + 2] = tp[2];
    } else if(X == SP_TEX_555 || X == SP_TEX_565) {
        const br_uint_8 *tp = tex + src;

        colour.base[sp + 0] = tp[0];
        colour.base[sp + 1] = tp[1];
    } else {
        /*
         * The shade-table arm: the index-8 texel is the column and the
         * interpolated intensity is the row, and the sample is the output
         * pixel itself. t_piza.asm indexes the table at the entry's own width -
         * `[edx+ecx*2]` for the 15/16bpp kernels (DBPP=2), whose table is a
         * 256-wide RGB_555/565 map, and `lea ecx,[ecx+ecx*2]` with
         * `[edx+ecx]`/`2[edx+ecx]` for the 888 one (DBPP=3), whose entries are
         * three bytes. x86 has no scale-3 addressing mode, which is why the
         * reference reaches the entry through the lea rather than a scaled
         * load. With the stride in bytes the largest index the packing can
         * form, (255 << 8) | 255, ends on the byte after a 256x256 RGB_888
         * table, so no table the packing can index runs the check below out of
         * bounds; it is kept because it is free and because a caller may bind a
         * table of another size, which its type check would not catch.
         */
        const softprim_buffer& shade  = *w.shade;
        const br_uint_32      idx    = ((((br_uint_32)pip >> 16) & 0xffu) << 8) | tex[src];
        constexpr br_size_t   stride = (F == SP_FMT_888) ? 3u : 2u;
        const br_size_t       off    = (br_size_t)idx * stride;

        if(off + (br_size_t)pix_b <= (br_size_t)shade.stride_b * (br_size_t)shade.height) {
            for(br_int_32 k = 0; k < pix_b; k++)
                colour.base[sp + k] = shade.base[off + k];
        }
    }
}

/*
 * t_piza.asm's TriangleRenderZ2, one trapezium half. `end` starts equal to
 * `start` for the top half and is reset to the middle vertex for the bottom
 * one; each row draws from `start` (or, scanning backwards, to one pixel left
 * of it), then the parameters and the two edges advance.
 *
 * The direction of a scanline is decided here from the two edge pointers, as
 * the asm does, not from w.direction - that only set the sign of the per-pixel
 * u/v step.
 */
template <sp_fmt F, sp_tex X, sp_depth D>
void RgbAwtTrapezium(RgbAwtWalk& w, AwtEdge& edge) noexcept
{
    const softprim_buffer& texture  = *w.texture;
    const br_uint_8      *tex      = texture.base;
    const br_size_t       tex_size = (br_size_t)texture.stride_b * (br_size_t)texture.height;
    const br_int_32       dbpp     = w.pix_b;

    while(edge.count != 0) {
        edge.count--;

        br_int_32       sp  = w.start;
        const br_int_32 end = w.end;
        br_int_32       src = w.tex.source;
        br_int_32       zp  = w.zstart;
        br_uint_32      ua  = (w.tex.u_current & 0xffff0000u) | (w.tex.u_int & 0xffffu);
        br_uint_32      vc  = w.tex.v_current;
        br_int_32       pzp = w.pz.current;
        br_int_32       pip = w.pi.current;

        if(sp < end) {
            for(;;) {
                if(sp >= end)
                    break;

                RgbAwtPixel<F, X, D>(w, tex, tex_size, sp, src, zp, pzp, pip);

                pzp += w.pz.grad_x;

                if(X == SP_TEX_I8)
                    pip += w.pi.grad_x;

                zp += 2;
                sp += dbpp;

                RgbAwtStep(w.tex, ua, vc, src);
            }
        } else {
            for(;;) {
                if(sp <= end)
                    break;

                pzp -= w.pz.grad_x;

                if(X == SP_TEX_I8)
                    pip -= w.pi.grad_x;

                RgbAwtStep(w.tex, ua, vc, src);

                zp -= 2;
                sp -= dbpp;

                RgbAwtPixel<F, X, D>(w, tex, tex_size, sp, src, zp, pzp, pip);
            }
        }

        /* `next:` - the minor edge carries, the major edge selects the
         * parameter increments, and scan_inc steps the packed u/v state. */
        edge.f += edge.d_f;
        w.end += (edge.d_i + (br_int_32)((edge.f < edge.d_f) ? 1u : 0u)) * dbpp;

        w.main_f += w.main_d_f;

        const bool mcarry = (w.main_f < w.main_d_f);

        w.start += (w.main_d_i + (mcarry ? 1 : 0)) * dbpp;
        w.zstart += 2 * (w.main_d_i + (mcarry ? 1 : 0));
        w.pz.current += mcarry ? w.pz.d_carry : w.pz.d_nocarry;

        /*
         * t_piza.asm's scan_inc steps the intensity alongside the depth, and
         * only for the LIGHT instantiations. Without it the intensity walks
         * along the scanline (the per-pixel step below) but never down a
         * scanline, so an interpolated shade-table block reads the top row's
         * intensity on every row.
         */
        if(X == SP_TEX_I8)
            w.pi.current += mcarry ? w.pi.d_carry : w.pi.d_nocarry;

        {
            const br_uint_32 du_ = mcarry ? w.tex.du_carry : w.tex.du_nocarry;
            const br_uint_32 pre = w.tex.u_current;

            w.tex.u_current         = pre + du_;
            const br_uint_32 ucarry = (w.tex.u_current < pre) ? 1u : 0u;

            w.tex.source += mcarry ? w.tex.dsource_carry : w.tex.dsource_nocarry;
            w.tex.u_int += (br_uint_32)((mcarry ? w.tex.du_int_carry : w.tex.du_int_nocarry) + (br_int_32)ucarry);
            w.tex.source += (br_int_32)ucarry * w.tex.sbpp;

            const br_uint_32 dv_  = mcarry ? w.tex.dv_carry : w.tex.dv_nocarry;
            const br_uint_32 vpre = w.tex.v_current;

            w.tex.v_current = vpre + dv_;

            if(w.tex.v_current < vpre)
                w.tex.source += w.tex.stride;
        }

        /* `cont:` - the column that has wrapped and the map underrun. */
        if((w.tex.u_int & 0x8000u) == 0) {
            w.tex.u_int += (br_uint_32)w.tex.width;
            w.tex.source += w.tex.width * w.tex.sbpp;
        }

        if(w.tex.source < 0)
            w.tex.source += w.tex.size;
    }
}

/*
 * One arbitrary-width triangle, for any of the three RGB outputs. X selects the
 * texture decode: RGB888 copies three bytes from the map, 555/565 copies the
 * map's own two-byte word (the output type equals the map type for those, so
 * the sample *is* the pixel), and I8 looks the texel up in the shade table with
 * the interpolated intensity as the row and writes the sample straight to the
 * colour buffer (awtmi.h's LIGHT path). Only the decode and the source bytes
 * per pixel differ - awtmi.h's span, the setup and the packing are the map's
 * runtime size, so the 555/565 map is the same addressing as the RGB_888 one
 * with SBPP=2 instead of 3.
 *
 * The I8 arm is the RGB-output shade-table family. Its 888 half is reachable -
 * the general prim_t24 walk reaches it, as there is no MMX table for an 888
 * output - and its 15/16bpp half only in the z-sorted mode: the 555/565
 * z-buffered shapes are taken by the MMX table (the MMX blocks are tried first
 * and one of their untextured rows always matches). The colour slot carries an
 * intensity rather than a colour for both, which is what SP_SHADE_*_I_RGB says.
 */
template <sp_fmt F, sp_depth D, sp_tex X, sp_shade S>
void RgbAwtTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    (void)block;

    const softprim_buffer& colour  = SoftPrimWork.colour;
    const softprim_buffer& dbuf    = SoftPrimWork.depth;
    const softprim_buffer& texture = SoftPrimWork.texture;
    const softprim_buffer& shade   = SoftPrimWork.shade;

    if(colour.type != RgbFormatType(F) || colour.base == NULL || colour.width_p <= 0 || colour.height <= 0)
        return;

    if(D == SP_DEPTH_ZW && (dbuf.type != BR_PMT_DEPTH_16 || dbuf.base == NULL))
        return;

    if(texture.base == NULL || texture.width_p <= 0 || texture.height <= 0 || texture.stride_b <= 0)
        return;

    if(X == SP_TEX_RGB888 && texture.type != BR_PMT_RGB_888)
        return;

    if(X == SP_TEX_555 && texture.type != BR_PMT_RGB_555)
        return;

    if(X == SP_TEX_565 && texture.type != BR_PMT_RGB_565)
        return;

    if(X == SP_TEX_I8 && (texture.type != BR_PMT_INDEX_8 || shade.base == NULL))
        return;

    brp_vertex *src[3] = {v0, v1, v2};
    PizVert     pv[3];

    for(int k = 0; k < 3; k++) {
        if(!isfinite(src[k]->comp[C_SX]) || !isfinite(src[k]->comp[C_SY]) || !isfinite(src[k]->comp[C_U]) || !isfinite(src[k]->comp[C_V]))
            return;

        pv[k].x  = BrFloatToFixed(src[k]->comp[C_SX]);
        pv[k].y  = BrFloatToFixed(src[k]->comp[C_SY]);
        pv[k].z  = BrFloatToFixed(src[k]->comp[C_SZ]);
        pv[k].u  = BrFloatToFixed(src[k]->comp[C_U]);
        pv[k].v  = BrFloatToFixed(src[k]->comp[C_V]);
        pv[k].i  = BrFloatToFixed(src[k]->comp[C_I]);
        pv[k].sy = pv[k].y;
    }

    /*
     * A constant-intensity block's intensity is vertex 0's for the whole
     * triangle. pentprim's table marks those blocks `duplicate`, which softrend
     * honours as BR_PRIMF_CONST_DUPLICATE, but softprim does not set that on
     * triangles and the constant-surface op only writes vertex 0 - so replicate
     * here, as ApplyIntensity() does for the indexed kernels. Without it the
     * intensity gradient comes from two stale slots.
     */
    if(S == SP_SHADE_CONST_I_RGB)
        pv[1].i = pv[2].i = pv[0].i;

    /* awtmi.h:28-57: the same three-element sort as perspzi.h, on the SY
     * component itself. */
    {
        const auto SwapVert = [](PizVert& l, PizVert& r) {
            const PizVert t = l;
            l               = r;
            r               = t;
        };

        if(pv[0].sy > pv[1].sy) {
            if(pv[1].sy > pv[2].sy) {
                SwapVert(pv[0], pv[2]);
            } else if(pv[0].sy > pv[2].sy) {
                SwapVert(pv[0], pv[1]);
                SwapVert(pv[1], pv[2]);
            } else {
                SwapVert(pv[0], pv[1]);
            }
        } else if(pv[1].sy > pv[2].sy) {
            if(pv[0].sy > pv[2].sy) {
                SwapVert(pv[1], pv[2]);
                SwapVert(pv[0], pv[1]);
            } else {
                SwapVert(pv[1], pv[2]);
            }
        }
    }

    RgbAwtWalk w;

    w.colour  = &colour;
    w.depth   = &dbuf;
    w.texture = &texture;
    w.shade   = &shade;

    const br_int_32 sbpp = (X == SP_TEX_RGB888) ? 3 : (X == SP_TEX_I8) ? 1 : 2;

    if(!RgbAwtSetup(w, pv, colour, texture, sbpp, X == SP_TEX_I8))
        return;

    RgbAwtTrapezium<F, X, D>(w, w.top);

    /* awtmi.h:180-183: the bottom half's minor edge restarts at the middle
     * vertex, while start and zstart carry over from the top half. */
    w.end = (br_int_32)(br_uint_16)(pv[1].x >> 16) * w.pix_b + (br_int_32)(br_uint_16)(pv[1].y >> 16) * colour.stride_b;

    RgbAwtTrapezium<F, X, D>(w, w.bot);
}

/*
 * ---------------------------------------------------------------------------
 * The untextured perfect-scan family: tt24_piz.asm and tt15_pi.asm/tt16_pi.asm.
 *
 * TriangleRenderPIZ2_RGB_888 (flat) and TriangleRenderPIZ2I_RGB_888 (Gouraud)
 * are the 888 blocks with no texture, and TriangleRenderPI_RGB_555/565 (flat)
 * and TriangleRenderPII_RGB_555/565 (Gouraud) are the 15/16bpp z-sorted pair.
 * All of them are the same lineage over the "perfect scan" idea, with its own
 * setup: the vertices are sorted by SY and then reduced to integer pixels, and
 * each edge gradient is an exact 32-bit division (FIX_DIV + idiv) split into a
 * whole-pixel step and a 16.16 fraction accumulator. There is no _reciprocal
 * table and no SafeFixedMac2Div - the parameter gradients come from a 64-bit
 * numerator over the determinant. SETUP_PI and PARAM_PI_DIRN are the same
 * instruction sequence in all four files; only the fragment's store width and
 * the GDIVIDE overflow arm differ, so F selects the store and the overflow arm.
 *
 * The trapezium is a pixel offset plus the fraction accumulator. Its depth test
 * is this family's own: `cmp ax,[edi]; jae` draws only while the new 16-bit
 * depth is *strictly* less than the stored one, which is one off the other
 * paths' `new <= old`.
 *
 * The z-sorted blocks (tt24_pi.asm's TriangleRenderPII_RGB_888 /
 * TriangleRenderPI_RGB_888 and tt15_pi.asm/tt16_pi.asm's TriangleRenderPII_RGB_555
 * / TriangleRenderPI_RGB_555) are the same setup and trapezium without the
 * depth test and store, so D selects them. Only the 888 pair has a z-buffered
 * twin here; the 15/16bpp z-buffered shapes are the MMX family (tt15_piz.asm is
 * not reached, because the MMX table is tried first for those tuples).
 * ---------------------------------------------------------------------------
 */

/* tt24_piz.asm's FIX_DIV + idiv: (v << 16) / d, truncated toward zero. */
inline br_int_32 PiGrad(br_int_32 v, br_int_32 d) noexcept
{
    if(d == 0)
        return 0;

    return (br_int_32)(((long long)v * 65536LL) / (long long)d);
}

/*
 * tt24_piz.asm's GDIVIDE for a positive divisor (the caller negates a negative
 * determinant before it gets here). The macro's `mov ebx,g_divisor` does not
 * touch the flags, so the `js` that follows tests the *numerator*: a negative
 * numerator is negated in 64 bits and the quotient negated back for the forward
 * half. The overflow guard compares the divisor with the numerator's high word
 * as unsigned.
 *
 * The two lineages differ in that guard's arm, which is why it is a parameter
 * rather than folded in. tt24_piz.asm/tt24_pi.asm and ti8_pi/ti8_piz put the
 * `overflow:` label immediately before `done:`, so the arm falls through and
 * leaves the low word of the (possibly negated) numerator as the result.
 * tt15_pi.asm/tt16_pi.asm put `overflow:` *before* `negative:` with a body of
 * `xor eax,eax; jmp done`, so both arms (the positive one jumps forward to it,
 * the negative one back) yield zero.
 */
inline br_int_32 PiDivide(br_int_32 numLo, br_int_32 numHi, br_int_32 divisor, bool reversed, bool overflow_zero) noexcept
{
    const br_uint_32 ebx = (br_uint_32)divisor;
    br_uint_32       lo  = (br_uint_32)numLo;
    br_uint_32       hi  = (br_uint_32)numHi;

    if((br_int_32)hi < 0) {
        /* `neg edx; neg eax; sbb edx,0`: the 64-bit negation. */
        const br_uint_32 nlo = 0u - lo;

        hi = (0u - hi) - ((lo != 0u) ? 1u : 0u);
        lo = nlo;

        if(ebx <= hi)
            return overflow_zero ? 0 : (br_int_32)lo;

        const br_uint_32 q = (br_uint_32)((((br_uint_64)hi << 32) | lo) / (br_uint_64)ebx);

        return (br_int_32)(reversed ? q : (0u - q));
    }

    if(ebx <= hi)
        return overflow_zero ? 0 : numLo;

    const br_uint_32 q = (br_uint_32)((((br_uint_64)hi << 32) | lo) / (br_uint_64)ebx);

    return (br_int_32)(reversed ? (0u - q) : q);
}

/* The integer-vertex geometry RgbPiSetup and the parameter walk share. The
 * determinant is always positive: the caller negates it on the reversed half. */
struct PiGeom {
    br_int_32 top_x, top_y, bot_x, bot_y, main_x, main_y;
    br_int_32 main_grad;
    br_int_32 divisor;
    bool      reversed;
};

/*
 * tt24_piz.asm's PARAM_PI_DIRN. grad_x and grad_y are 64-bit numerators over
 * the determinant; `current` starts half a pixel in, by ceil(grad_x/2) - the
 * asm's `sar ebx,1; adc eax,ebx` - and pz has that remapped unsigned for the
 * depth buffer.
 */
inline void PiParamSetup(PizParam& p, br_int_32 p0, br_int_32 p1, br_int_32 p2, const PiGeom& g, bool unsign, bool overflow_zero) noexcept
{
    const long long d1 = (long long)(br_int_32)((br_uint_32)p1 - (br_uint_32)p0);
    const long long d2 = (long long)(br_int_32)((br_uint_32)p2 - (br_uint_32)p0);

    const long long num_x = d1 * (long long)g.main_y - d2 * (long long)g.top_y;
    const long long num_y = d2 * (long long)g.top_x - d1 * (long long)g.main_x;

    p.grad_x = PiDivide((br_int_32)num_x, (br_int_32)(num_x >> 32), g.divisor, g.reversed, overflow_zero);
    p.grad_y = PiDivide((br_int_32)num_y, (br_int_32)(num_y >> 32), g.divisor, g.reversed, overflow_zero);

    /* `sar ebx,1` followed by `adc eax,ebx`: floor(grad_x/2) plus the shifted-out
     * bit, i.e. the gradient rounded toward +infinity. */
    const br_int_32 half = (p.grad_x >> 1) + (p.grad_x & 1);
    br_int_32       cur  = (br_int_32)((br_uint_32)p0 + (br_uint_32)half);

    if(unsign)
        cur = (br_int_32)((br_uint_32)cur ^ 0x80000000u);

    p.current = cur;

    const br_int_32 mi = Sar16(g.main_grad);

    p.d_nocarry = (br_int_32)((br_uint_32)(mi * p.grad_x) + (br_uint_32)p.grad_y);
    p.d_carry   = (br_int_32)((br_uint_32)p.d_nocarry + (br_uint_32)p.grad_x);
}

/* One edge of the tt24_piz trapezium: the 16.16 fraction accumulator, its
 * gradient's fraction part as the per-scanline step, the whole-pixel step with
 * the row stride folded in, and the scanlines left. */
struct PiEdge {
    br_uint_32 f, d_f;
    br_int_32  d_i;
    br_int_32  count;
    br_int_32  i; /* the edge's pixel offset at the top of the half */
};

struct RgbPiWalk {
    const softprim_buffer *colour;
    const softprim_buffer *depth;

    PizParam pz;
    PizParam pr, pg, pb;

    br_int_32  main_i, main_d_i;
    br_uint_32 main_f, main_d_f;

    PiEdge top, bot;
};

/*
 * tt24_piz.asm's SETUP_PI. The top and bottom edge gradients are only computed
 * when the edge has height; a zero-height edge's fields are left alone, and its
 * count is what stops the trapezium from reading them.
 */
inline bool RgbPiSetup(RgbPiWalk& w, const PizVert vx[3], const softprim_buffer& colour, bool interp, bool overflow_zero) noexcept
{
    const br_int_32 x0 = vx[0].x >> 16, y0 = vx[0].y >> 16;
    const br_int_32 x1 = vx[1].x >> 16, y1 = vx[1].y >> 16;
    const br_int_32 x2 = vx[2].x >> 16, y2 = vx[2].y >> 16;

    if(y0 == y2)
        return false;

    const br_int_32 stride_p = colour.stride_b / colour.bpp;

    const PiGeom g{(br_int_32)(x1 - x0),
                   (br_int_32)(y1 - y0),
                   (br_int_32)(x2 - x1),
                   (br_int_32)(y2 - y1),
                   (br_int_32)(x2 - x0),
                   (br_int_32)(y2 - y0),
                   PiGrad((br_int_32)(x2 - x0), (br_int_32)(y2 - y0)),
                   0,
                   false};

    const br_int_32 raw_divisor = (br_int_32)((br_uint_32)(g.top_x * g.main_y) - (br_uint_32)(g.main_x * g.top_y));

    if(raw_divisor == 0)
        return false;

    const bool reversed = raw_divisor < 0;

    PiGeom gg = g;

    gg.divisor  = reversed ? (br_int_32)(0u - (br_uint_32)raw_divisor) : raw_divisor;
    gg.reversed = reversed;

    const br_int_32 top_grad = g.top_y ? PiGrad(g.top_x, g.top_y) : 0;
    const br_int_32 bot_grad = g.bot_y ? PiGrad(g.bot_x, g.bot_y) : 0;

    /* `mov word ptr d_f+2, ax`: the fraction half of the 16.16 gradient. The
     * asm leaves the low half of the 32-bit field alone, which is the stale
     * value from whatever wrote it last; zero is what the walk needs. */
    w.top.f     = 0x80000000u;
    w.top.d_f   = (br_uint_32)top_grad << 16;
    w.top.d_i   = g.top_y ? (Sar16(top_grad) + stride_p) : 0;
    w.top.count = g.top_y;

    w.bot.f     = 0x80000000u;
    w.bot.d_f   = (br_uint_32)bot_grad << 16;
    w.bot.d_i   = g.bot_y ? (Sar16(bot_grad) + stride_p) : 0;
    w.bot.count = g.bot_y;

    /* The edge offsets: y*stride_p + x at the top vertex (which both `main.i`
     * and `top.i` take) and at the middle one. */
    w.main_i = (br_int_32)((br_uint_32)(y0 * stride_p) + (br_uint_32)x0);
    w.top.i  = w.main_i;
    w.bot.i  = (br_int_32)((br_uint_32)(y1 * stride_p) + (br_uint_32)x1);

    w.main_d_i = Sar16(g.main_grad) + stride_p;
    w.main_f   = 0x80000000u;
    w.main_d_f = (br_uint_32)g.main_grad << 16;

    PiParamSetup(w.pz, vx[0].z, vx[1].z, vx[2].z, gg, true, overflow_zero);
    if(interp) {
        PiParamSetup(w.pr, vx[0].r, vx[1].r, vx[2].r, gg, false, overflow_zero);
        PiParamSetup(w.pg, vx[0].g, vx[1].g, vx[2].g, gg, false, overflow_zero);
        PiParamSetup(w.pb, vx[0].b, vx[1].b, vx[2].b, gg, false, overflow_zero);
    } else {
        /* The flat variant's colour is the whole vertex colour, constant along
         * the triangle - a zero gradient leaves the trapezium's copy alone. */
        w.pr = w.pg = w.pb = PizParam{};

        w.pr.current = vx[0].r;
        w.pg.current = vx[0].g;
        w.pb.current = vx[0].b;
    }

    return true;
}

/* The fragment of one drawn pixel: the new 16-bit depth must be strictly less
 * than the stored one for the pixel to land. The colour is three bytes for
 * RGB_888 and one 15/16-bit word for RGB_555/565; the 15/16bpp blocks are
 * z-sorted only, so D is SP_DEPTH_NONE for them in practice. */
template <sp_fmt F, sp_depth D>
inline void PiPixel(const softprim_buffer& colour, const softprim_buffer& dbuf, br_int_32 sp, br_int_32 zz, br_int_32 cr, br_int_32 cg,
                    br_int_32 cb) noexcept
{
    const br_int_32 pix_b = RgbPixelBytes(F);

    if(sp < 0 || (br_size_t)sp * (br_size_t)pix_b + (br_size_t)pix_b > (br_size_t)colour.stride_b * (br_size_t)colour.height)
        return;

    const br_uint_16 newz = (br_uint_16)((br_uint_32)zz >> 16);

    if(D == SP_DEPTH_ZW) {
        if((br_size_t)sp * 2 + 2 > (br_size_t)dbuf.stride_b * (br_size_t)dbuf.height)
            return;

        if(newz >= LoadDepth(dbuf.base + (br_size_t)sp * 2))
            return;

        StoreDepth(dbuf.base + (br_size_t)sp * 2, newz);
    }

    /* The asm takes `byte ptr temp+2` (the integer byte of the 16.16 value) and
     * shifts the top five bits of it into the packed word, which is the same
     * truncation RgbStore<F> makes from a byte channel. */
    RgbStore<F>(colour.base + (br_size_t)sp * (br_size_t)pix_b, (br_uint_8)(((br_uint_32)cr >> 16) & 0xffu),
                (br_uint_8)(((br_uint_32)cg >> 16) & 0xffu), (br_uint_8)(((br_uint_32)cb >> 16) & 0xffu));
}

/*
 * tt24_piz.asm's TRAPEZOID_PIZ2{RGB,I}_RGB_888 and tt15_pi.asm/tt16_pi.asm's
 * TRAPEZOID_PI{PII,PI}_RGB_555, one trapezium half. `edge.i` carries the half's
 * end offset; the middle vertex's is restored by the caller for the bottom
 * half, as SETUP_PI left it.
 */
template <sp_fmt F, sp_depth D, bool INTERP>
void RgbPiTrapezium(RgbPiWalk& w, PiEdge& edge) noexcept
{
    const softprim_buffer& colour = *w.colour;
    const softprim_buffer& dbuf   = *w.depth;

    while(edge.count != 0) {
        edge.count--;

        br_int_32       sp  = w.main_i;
        const br_int_32 end = edge.i;
        br_int_32       zz  = w.pz.current;
        br_int_32       cr  = w.pr.current;
        br_int_32       cg  = w.pg.current;
        br_int_32       cb  = w.pb.current;

        if(sp < end) {
            for(;;) {
                if(sp >= end)
                    break;

                PiPixel<F, D>(colour, dbuf, sp, zz, cr, cg, cb);

                zz += w.pz.grad_x;
                cr += w.pr.grad_x;
                cg += w.pg.grad_x;
                cb += w.pb.grad_x;
                sp++;
            }
        } else {
            for(;;) {
                if(sp <= end)
                    break;

                /* The backward loop steps before it draws, so its first pixel
                 * is the one to the left of the major edge. */
                zz -= w.pz.grad_x;
                cr -= w.pr.grad_x;
                cg -= w.pg.grad_x;
                cb -= w.pb.grad_x;
                sp--;

                PiPixel<F, D>(colour, dbuf, sp, zz, cr, cg, cb);
            }
        }

        /* `next:`: the minor edge's fraction carry moves its end offset, the
         * major edge's selects the parameter increments. */
        edge.f += edge.d_f;
        edge.i += edge.d_i + (br_int_32)((edge.f < edge.d_f) ? 1u : 0u);

        w.main_f += w.main_d_f;

        const bool mcarry = (w.main_f < w.main_d_f);

        w.main_i += w.main_d_i + (mcarry ? 1 : 0);
        w.pz.current += mcarry ? w.pz.d_carry : w.pz.d_nocarry;

        if(INTERP) {
            w.pr.current += mcarry ? w.pr.d_carry : w.pr.d_nocarry;
            w.pg.current += mcarry ? w.pg.d_carry : w.pg.d_nocarry;
            w.pb.current += mcarry ? w.pb.d_carry : w.pb.d_nocarry;
        }
    }
}

/* One untextured perfect-scan triangle. INTERP selects the Gouraud variant. The
 * 15/16bpp blocks are the z-sorted pair, so D is SP_DEPTH_NONE for them. */
template <sp_fmt F, sp_depth D, bool INTERP>
void RgbPiTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    (void)block;

    const softprim_buffer& colour = SoftPrimWork.colour;
    const softprim_buffer& dbuf   = SoftPrimWork.depth;

    if(colour.type != RgbFormatType(F) || colour.base == NULL || colour.width_p <= 0 || colour.height <= 0)
        return;

    if(D == SP_DEPTH_ZW && (dbuf.type != BR_PMT_DEPTH_16 || dbuf.base == NULL))
        return;

    brp_vertex *src[3] = {v0, v1, v2};
    PizVert     pv[3];

    for(int k = 0; k < 3; k++) {
        if(!isfinite(src[k]->comp[C_SX]) || !isfinite(src[k]->comp[C_SY]))
            return;

        pv[k].x  = BrFloatToFixed(src[k]->comp[C_SX]);
        pv[k].y  = BrFloatToFixed(src[k]->comp[C_SY]);
        pv[k].z  = BrFloatToFixed(src[k]->comp[C_SZ]);
        pv[k].r  = BrFloatToFixed(src[k]->comp[C_R]);
        pv[k].g  = BrFloatToFixed(src[k]->comp[C_G]);
        pv[k].b  = BrFloatToFixed(src[k]->comp[C_B]);
        pv[k].sy = pv[k].y;
    }

    /* The flat variant's colour is vertex 0's, read before the sort - which is
     * what the asm does, and the block duplicates it, so the order is moot. */
    if(!INTERP) {
        pv[1].r = pv[2].r = pv[0].r;
        pv[1].g = pv[2].g = pv[0].g;
        pv[1].b = pv[2].b = pv[0].b;
    }

    /*
     * SETUP_PI's three-element sort, on the SY component.
     *
     * tt24_piz.asm's tie-break is not perspzi.h's: the asm's sort_3 falls to
     * `cba` when the bottom two vertices are equal (`cmp ecx,ebx; jg sort_4`
     * jumps only on v2.y > v1.y), where the C sorts v1.y > v2.y. The two differ
     * on one arrangement of equal Y values and pick a different long edge, and
     * a different long edge is a different depth walk. tt15_pi.asm and
     * tt16_pi.asm carry the same sort_3 body, so the same broken tie-break is
     * modelled here for them too.
     */
    {
        const auto SwapVert = [](PizVert& l, PizVert& r) {
            const PizVert t = l;
            l               = r;
            r               = t;
        };

        if(pv[0].sy > pv[1].sy) {
            if(pv[2].sy > pv[1].sy) {
                if(pv[0].sy > pv[2].sy) {
                    SwapVert(pv[0], pv[1]);
                    SwapVert(pv[1], pv[2]);
                } else {
                    SwapVert(pv[0], pv[1]);
                }
            } else {
                SwapVert(pv[0], pv[2]);
            }
        } else if(pv[1].sy > pv[2].sy) {
            if(pv[0].sy > pv[2].sy) {
                SwapVert(pv[1], pv[2]);
                SwapVert(pv[0], pv[1]);
            } else {
                SwapVert(pv[1], pv[2]);
            }
        }
    }

    RgbPiWalk w;

    w.colour = &colour;
    w.depth  = &dbuf;

    if(!RgbPiSetup(w, pv, colour, INTERP, F != SP_FMT_888))
        return;

    RgbPiTrapezium<F, D, INTERP>(w, w.top);
    RgbPiTrapezium<F, D, INTERP>(w, w.bot);
}

struct RgbWalk {
    const softprim_buffer *colour;
    const softprim_buffer *depth;
    const softprim_buffer *texture;

    br_int_32 pix_b;   /* output bytes per pixel */
    br_int_32 texel_b; /* texture bytes per texel */

    TrapParam pz;         /* depth, 16.16, remapped unsigned */
    TrapParam pu, pv;     /* texture coordinate, 16.16 texel units */
    TrapParam pr, pg, pb; /* colour, 16.16; integer part is the channel byte */

    br_int_32  main_i, main_d;
    br_uint_32 main_f, main_d_f;
    br_int_32  minor_i, minor_d;
    br_int_32  minor_count;

    br_uint_8 *scan;  /* colour row start - pix_b */
    br_uint_8 *zscan; /* depth row start - 2 */
};

/*
 * One RGB span. Depth is tested as newz <= oldz and written before the colour,
 * matching the INDEX_8 span and pentprim's 16-bit z test. A texel of zero (or
 * an all-zero RGB texel) is transparent and skips both the colour and the depth
 * store, which is pentprim's TRANSP behaviour for the RGB texture mappers.
 */
template <sp_fmt F, sp_depth D, sp_shade S, sp_tex X, bool C7, int DIRN>
void RgbSpan(RgbWalk& w, br_uint_8 *c, br_uint_8 *end, br_uint_8 *z) noexcept
{
    const softprim_buffer& tex = *w.texture;

    br_int_32 zz = w.pz.current;
    br_int_32 u  = w.pu.current;
    br_int_32 v  = w.pv.current;
    br_int_32 r  = w.pr.current;
    br_int_32 g  = w.pg.current;
    br_int_32 b  = w.pb.current;

    for(;;) {
        bool       draw = true;
        br_uint_16 newz = 0;
        br_uint_8  orr = 0, ogg = 0, obb = 0;

        if(D == SP_DEPTH_ZW) {
            newz = (br_uint_16)((br_uint_32)zz >> 16);

            if(newz > LoadDepth(z))
                draw = false;
        }

        if(draw) {
            if(X == SP_TEX_NONE) {
                orr = (br_uint_8)((br_uint_32)r >> 16);
                ogg = (br_uint_8)((br_uint_32)g >> 16);
                obb = (br_uint_8)((br_uint_32)b >> 16);
            } else {
                const br_int_32  tx = WrapIndex(u >> 16, tex.width_p);
                const br_int_32  ty = WrapIndex(v >> 16, tex.height);
                const br_uint_8 *tp = tex.base + (br_size_t)ty * tex.stride_b + (br_size_t)tx * w.texel_b;

                if(X == SP_TEX_I8) {
                    const br_uint_32 idx = tp[0];

                    if(idx == 0)
                        draw = false;
                    else if(!PaletteEntry(tex, idx, orr, ogg, obb))
                        draw = false;
                } else {
                    /* An RGB_888 texture is byte-ordered B,G,R in memory. */
                    orr = tp[2];
                    ogg = tp[1];
                    obb = tp[0];

                    if(orr == 0 && ogg == 0 && obb == 0)
                        draw = false;
                }

                if(draw && S != SP_SHADE_NONE) {
                    /*
                     * The block's colour scale is either 254 or, for a
                     * colour7bit block, 126: pentprim hands its MMX kernel the
                     * 7-bit value and restores the scale inside the kernel
                     * (unpack.inc's psrlw 1 / psllw 1). The same compensation
                     * here is one bit less of shift.
                     */
                    constexpr int shift = C7 ? 7 : 8;

                    orr = (br_uint_8)(((br_uint_32)orr * (br_uint_32)((br_uint_8)((br_uint_32)r >> 16))) >> shift);
                    ogg = (br_uint_8)(((br_uint_32)ogg * (br_uint_32)((br_uint_8)((br_uint_32)g >> 16))) >> shift);
                    obb = (br_uint_8)(((br_uint_32)obb * (br_uint_32)((br_uint_8)((br_uint_32)b >> 16))) >> shift);
                }
            }
        }

        if(draw) {
            if(D == SP_DEPTH_ZW)
                StoreDepth(z, newz);

            RgbStore<F>(c, orr, ogg, obb);
        }

        if(DIRN == RPD_FWD) {
            c += w.pix_b;

            if(c > end)
                break;

            if(D == SP_DEPTH_ZW)
                z += 2;

            zz += w.pz.grad_x;
            u += w.pu.grad_x;
            v += w.pv.grad_x;
            r += w.pr.grad_x;
            g += w.pg.grad_x;
            b += w.pb.grad_x;
        } else {
            c -= w.pix_b;

            if(c < end)
                break;

            if(D == SP_DEPTH_ZW)
                z -= 2;

            zz -= w.pz.grad_x;
            u -= w.pu.grad_x;
            v -= w.pv.grad_x;
            r -= w.pr.grad_x;
            g -= w.pg.grad_x;
            b -= w.pb.grad_x;
        }
    }
}

template <sp_fmt F, sp_depth D, sp_shade S, sp_tex X, bool C7, int DIRN>
void RgbTrapezium(RgbWalk& w) noexcept
{
    for(br_int_32 n = w.minor_count; n >= 0; n--) {
        const br_int_32 main_int  = (br_int_32)((br_uint_32)w.main_i >> 16);
        const br_int_32 minor_int = (br_int_32)((br_uint_32)w.minor_i >> 16);
        const bool      draw      = (DIRN == RPD_FWD) ? (main_int <= minor_int) : (main_int >= minor_int);

        if(draw)
            RgbSpan<F, D, S, X, C7, DIRN>(w, w.scan + (br_size_t)main_int * w.pix_b, w.scan + (br_size_t)minor_int * w.pix_b,
                                          w.zscan + (br_size_t)main_int * 2);

        w.main_f += w.main_d_f;

        const br_uint_32 carry = (w.main_f < w.main_d_f) ? 1u : 0u;

        w.main_i += w.main_d;
        w.minor_i += w.minor_d;

        w.pz.current += (carry != 0u) ? w.pz.d_carry : w.pz.d_nocarry;
        w.pu.current += (carry != 0u) ? w.pu.d_carry : w.pu.d_nocarry;
        w.pv.current += (carry != 0u) ? w.pv.d_carry : w.pv.d_nocarry;
        w.pr.current += (carry != 0u) ? w.pr.d_carry : w.pr.d_nocarry;
        w.pg.current += (carry != 0u) ? w.pg.d_carry : w.pg.d_nocarry;
        w.pb.current += (carry != 0u) ? w.pb.d_carry : w.pb.d_nocarry;

        w.scan += w.colour->stride_b;

        if(D == SP_DEPTH_ZW)
            w.zscan += w.depth->stride_b;
    }
}

/*
 * One RGB triangle. F is the output format, S the shade, X the texture decode
 * and D the depth mode; A and P do not change the fragment (every textured RGB
 * block is affine - see the note above) but are carried so the dispatch is one
 * line per tuple.
 *
 * This is the general SETUP_FLOAT RGB path (fpsetup.asm's SETUP_FLOAT and
 * SETUP_FLOAT_PARAM) and no generated block reaches it: the dispatch in
 * SoftPrimRender routes every emitted tuple to a kernel above it. The two static
 * assertions at the dispatch's tail are what keep that true - they name the two
 * axis combinations that would otherwise fall through to here, so a widened
 * SP_SPEC cannot quietly start drawing a shape through this kernel. The kernel
 * itself is complete and deliberately kept; removing it is a separate change.
 */
template <sp_fmt F, sp_depth D, sp_shade S, sp_tex X, sp_addr A, sp_persp P, bool C7>
void RgbTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    (void)block;
    (void)A;
    (void)P;

    const softprim_buffer& colour  = SoftPrimWork.colour;
    const softprim_buffer& dbuf    = SoftPrimWork.depth;
    const softprim_buffer& texture = SoftPrimWork.texture;

    if(colour.type != RgbFormatType(F) || colour.base == NULL || colour.width_p <= 0 || colour.height <= 0)
        return;

    if(D == SP_DEPTH_ZW && (dbuf.type != BR_PMT_DEPTH_16 || dbuf.base == NULL))
        return;

    if(X != SP_TEX_NONE) {
        if(texture.base == NULL || texture.width_p <= 0 || texture.height <= 0 || texture.stride_b <= 0)
            return;

        if(X == SP_TEX_I8 && (texture.type != BR_PMT_INDEX_8 || texture.palette == NULL || texture.palette_size <= 0))
            return;

        if(X == SP_TEX_RGB888 && texture.type != BR_PMT_RGB_888)
            return;
    }

    brp_vertex *src[3] = {v0, v1, v2};
    TexVert     in[3];

    for(int k = 0; k < 3; k++) {
        in[k].x  = BrFloatToFixed(src[k]->comp[C_SX]);
        in[k].y  = BrFloatToFixed(src[k]->comp[C_SY]);
        in[k].z  = BrFloatToFixed(src[k]->comp[C_SZ]);
        in[k].u  = BrFloatToFixed(src[k]->comp[C_U]);
        in[k].v  = BrFloatToFixed(src[k]->comp[C_V]);
        in[k].r  = BrFloatToFixed(src[k]->comp[C_R]);
        in[k].g  = BrFloatToFixed(src[k]->comp[C_G]);
        in[k].b  = BrFloatToFixed(src[k]->comp[C_B]);
        in[k].rx = src[k]->comp[C_SX];
        in[k].ry = src[k]->comp[C_SY];
        in[k].rz = src[k]->comp[C_SZ];
        in[k].ru = src[k]->comp[C_U];
        in[k].rv = src[k]->comp[C_V];
        in[k].rr = src[k]->comp[C_R];
        in[k].rg = src[k]->comp[C_G];
        in[k].rb = src[k]->comp[C_B];

        if(!isfinite(in[k].rx) || !isfinite(in[k].ry) || !isfinite(in[k].rz))
            return;

        if(X != SP_TEX_NONE && (!isfinite(in[k].ru) || !isfinite(in[k].rv)))
            return;

        if(S != SP_SHADE_NONE && (!isfinite(in[k].rr) || !isfinite(in[k].rg) || !isfinite(in[k].rb)))
            return;
    }

    /* A constant-colour block duplicates vertex 0's colour before the sort. */
    if(S == SP_SHADE_CONST_RGB) {
        in[1].r = in[2].r = in[0].r;
        in[1].g = in[2].g = in[0].g;
        in[1].b = in[2].b = in[0].b;
        in[1].rr = in[2].rr = in[0].rr;
        in[1].rg = in[2].rg = in[0].rg;
        in[1].rb = in[2].rb = in[0].rb;
    }

    TriSetup ts;

    if(!BuildTriSetup(in, ts))
        return;

    RgbWalk w;

    w.colour  = &colour;
    w.depth   = &dbuf;
    w.texture = &texture;
    w.pix_b   = RgbPixelBytes(F);
    w.texel_b = (X == SP_TEX_RGB888) ? 3 : 1;

    SetupFloatParam<16>(w.pz, ts, ts.orig[0].rz, ts.orig[1].rz, ts.orig[2].rz, ts.v[0].rz, true);

    if(X != SP_TEX_NONE) {
        SetupFloatParam<16>(w.pu, ts, ts.orig[0].ru, ts.orig[1].ru, ts.orig[2].ru, ts.v[0].ru, false);
        SetupFloatParam<16>(w.pv, ts, ts.orig[0].rv, ts.orig[1].rv, ts.orig[2].rv, ts.v[0].rv, false);
    } else {
        w.pu = w.pv = TrapParam{};
    }

    if(S != SP_SHADE_NONE) {
        SetupFloatParam<16>(w.pr, ts, ts.orig[0].rr, ts.orig[1].rr, ts.orig[2].rr, ts.v[0].rr, false);
        SetupFloatParam<16>(w.pg, ts, ts.orig[0].rg, ts.orig[1].rg, ts.orig[2].rg, ts.v[0].rg, false);
        SetupFloatParam<16>(w.pb, ts, ts.orig[0].rb, ts.orig[1].rb, ts.orig[2].rb, ts.v[0].rb, false);
    } else {
        w.pr = w.pg = w.pb = TrapParam{};
    }

    w.main_i   = ts.walk.xm;
    w.main_d   = ts.walk.d_xm;
    w.main_f   = (br_uint_32)ts.walk.xm_f;
    w.main_d_f = (br_uint_32)ts.walk.d_xm_f;

    w.scan  = colour.base + (br_size_t)ts.walk.yTop * colour.stride_b - w.pix_b;
    w.zscan = ((D == SP_DEPTH_ZW) ? dbuf.base : colour.base) + (br_size_t)ts.walk.yTop * ((D == SP_DEPTH_ZW) ? dbuf.stride_b : 0) - 2;

    w.minor_i     = ts.walk.x1;
    w.minor_d     = ts.walk.d_x1;
    w.minor_count = ts.walk.topCount;

    if(ts.asm_dir == 0)
        RgbTrapezium<F, D, S, X, C7, RPD_FWD>(w);
    else
        RgbTrapezium<F, D, S, X, C7, RPD_BWD>(w);

    w.minor_i     = ts.walk.x2;
    w.minor_d     = ts.walk.d_x2;
    w.minor_count = ts.walk.bottomCount;

    if(ts.asm_dir == 0)
        RgbTrapezium<F, D, S, X, C7, RPD_FWD>(w);
    else
        RgbTrapezium<F, D, S, X, C7, RPD_BWD>(w);
}

/*
 * ---------------------------------------------------------------------------
 * The MMX 15/16bpp family.
 *
 * pentprim's only textured path into an RGB_555/565 output is the MMX family
 * (mmx_t15.ifg / mmx_t16.ifg, bodies xzrgb.inc, xzuvc.inc, xzuvrgb.inc and
 * xzuv1.inc). The general tables' textured 555/565 blocks want an intensity
 * slot (awtmi.h's LIGHT path) and have no kernel here, so every textured
 * 15/16bpp tuple in the spec is an MMX one and this is where they live.
 *
 * Four things separate these kernels from the general RGB kernel above, and
 * each one shows in the pixels:
 *
 *  - the setup is gsetuptf.asm's SETUP_FLOAT rather than fpsetup.asm's, so the
 *    block's pixel_stride (4) aligns the walk's x origin and the major-edge
 *    gradient to a four-pixel word, and the carry the per-scanline increments
 *    are selected by is the four-pixel boundary rather than the pixel one;
 *  - u and v arrive as 20.12 and are packed into two 32-bit accumulators, each
 *    with a bridge bit that turns a carry out of the fraction into a carry into
 *    the integer field, and the texel address is (u + v) >> 12;
 *  - r, g and b arrive as 8.24 and live in four sixteen-bit lanes, one per pixel
 *    of a word, with the per-word step truncated when the lane is built;
 *  - the depth store is gated on the depth test alone and not on the texel being
 *    opaque, so a transparent texel still consumes the depth.
 *
 * Each body is a different fragment over the same walk: xzrgb.inc packs the
 * interpolated colour words straight into the destination (no texture);
 * xzuv1.inc packs the palette bytes straight (texture only); xzuvc.inc modulates
 * them by a constant colour; xzuvrgb.inc modulates them by the interpolated
 * colour, with the palette byte halved before the multiply. The three texture
 * bodies halve the palette byte at different points - before the wide multiply,
 * after it, or not at all - so the halving is per body rather than shared.
 * ---------------------------------------------------------------------------
 */

/* findShift (pentprim/sbuffer.c:35): floor(log2(x)), and -1 for zero, which is
 * how a bound map's width_s and height_s are derived. */
inline br_int_32 MmxFindShift(br_int_32 x) noexcept
{
    br_int_32 b = -1;

    while(x) {
        x /= 2;
        b++;
    }

    return b;
}

/* pmulhw: the signed high half of a sixteen-bit lane multiply. */
inline br_uint_16 MmxPmulhw(br_uint_16 a, br_uint_16 b) noexcept
{
    return (br_uint_16)(br_uint_32)(((br_int_32)(br_int_16)a * (br_int_32)(br_int_16)b) >> 16);
}

/* packssdw's per-lane saturation. */
inline br_uint_16 MmxSat16(br_int_32 v) noexcept
{
    if(v > 32767)
        return (br_uint_16)0x7fff;

    if(v < -32768)
        return (br_uint_16)0x8000;

    return (br_uint_16)v;
}

/*
 * One colour parameter in the shape UNPACK_PARAM_RGB leaves it: four sixteen bit
 * lanes, one per pixel of a four-pixel word, and the three steps the walk
 * applies. The lanes are `psrad 16` followed by `packssdw`, so the eight
 * fractional bits of the 8.24 value become the lane's high byte - which is why
 * the modulates below read as signed and take the top bit of the input word as
 * a sign.
 *
 * The per-word step is the same conversion applied to four pixels' worth of the
 * gradient, so it is truncated when it is built and the fraction below it never
 * accumulates; that is how pentprim's colour words drift from the true
 * gradient, and reproducing it matters as much as reproducing the modulate.
 */
struct MmxLane {
    br_uint_16 w[4];
    br_uint_16 step;
    br_uint_16 dy0, dy1;
};

inline MmxLane MmxMakeLane(const TrapParam& p) noexcept
{
    MmxLane l;

    for(int k = 0; k < 4; k++)
        l.w[k] = MmxSat16((br_int_32)((br_uint_32)p.current + (br_uint_32)p.grad_x * (br_uint_32)k) >> 16);

    l.step = MmxSat16((br_int_32)((br_uint_32)p.grad_x << 2) >> 16);
    l.dy0  = MmxSat16((br_int_32)((br_uint_32)p.d_nocarry) >> 16);
    l.dy1  = MmxSat16((br_int_32)((br_uint_32)p.d_carry) >> 16);

    return l;
}

/* The lane's value `g` words past the walk's origin. The per-word step is added
 * to every lane of a word at once (`paddw`), so it is the same modular
 * sixteen-bit add for each of them. */
inline br_uint_16 MmxLaneAt(const MmxLane& l, br_int_32 k, br_int_32 g) noexcept
{
    return (br_uint_16)(l.w[k] + (br_uint_16)(g * l.step));
}

/* The packed texture accumulator's fraction width, its mask, and the bridge bit
 * that carries a fraction overflow into the integer field. */
constexpr br_int_32  MMX_FRACTION_BITS    = 11;
constexpr br_uint_32 MMX_UV_FRACTION_MASK = (1u << MMX_FRACTION_BITS) - 1u;
constexpr br_uint_32 MMX_UV_BRIDGE        = 1u << MMX_FRACTION_BITS;

/*
 * UNPACK_UV_32's packing of one 20.12 texture coordinate into the accumulator:
 * the fraction keeps its top eleven bits, the integer part is shifted up by the
 * tile shift and masked into the axis's own field, and `fill` is ORed in on top.
 * `fill` is what makes the carry work - it holds ones in the bits between the
 * fraction and this axis's integer, so a carry out of the fraction ripples
 * through them - and it belongs on the delta rather than the start because a
 * ripple needs both operands to have ones there while the start is masked.
 *
 * The tile field is empty for every map this build can be handed: sbuffer.c
 * pins tile_s to zero, so the tile mask is zero and the low integer bits of an
 * unpacked coordinate carry nothing.
 */
inline br_uint_32 MmxPackCoord(br_int_32 s, br_int_32 shift, br_uint_32 field, br_uint_32 fill) noexcept
{
    br_uint_32 r = ((br_uint_32)((br_int_32)s >> 1)) & MMX_UV_FRACTION_MASK;

    r |= ((br_uint_32)s << shift) & field;
    r |= fill;

    return r;
}

/*
 * The MMX walk: the same trapezium shape as the general RGB kernel, plus the
 * packed texture accumulators and the sixteen-bit colour lanes. `u`, `v`, the
 * lanes, `pz` and the two edge values all hold the state for the scanline about
 * to be drawn; the scanline address pointers are advanced as the walk descends.
 */
struct MmxWalk {
    TrapParam pz;

    br_uint_32 u, v;
    br_uint_32 d_u_x, d_v_x;               /* packed per-word steps */
    br_uint_32 du_y0, du_y1, dv_y0, dv_y1; /* packed per-scanline steps */
    br_uint_32 u_mask, v_mask;

    /*
     * xzuv1.inc's four accumulators, one per lane of the four-pixel word, plus
     * the packed per-word step. The texture-only body does not carry a single
     * coordinate advanced per pixel the way xzuvc.inc and xzuvrgb.inc do: it
     * builds a coordinate per word lane from the exact 20.12 start - pack(s + 0
     * du) .. pack(s + 3 du) - and advances the whole word by pack(4 du), so the
     * sub-fraction bit the pack drops is dropped once rather than accumulated.
     * Used when X == SP_TEX_I8 && S == SP_SHADE_NONE; see MmxScan.
     */
    br_uint_32 lane_u[4], lane_v[4];
    br_uint_32 d_u_word, d_v_word;

    MmxLane col[3]; /* r, g, b; empty for the texture-only bodies */

    br_int_32 main_i, main_d;
    br_int_32 minor_i, minor_d;
    br_int_32 minor_count;

    br_uint_8 *scan;
    br_uint_8 *zscan;

    br_int_32 scan_stride_b;
    br_int_32 zscan_stride_b;
};

/* The conversion masks the colour pass shreds the fragment with. mask_5 and
 * mask_6 are for a word whose channel is the eight-bit value shifted up by
 * eight (the palette read straight and the interpolated colour); mask_5d and
 * mask_6d are for the modulated word, which sits one bit lower because the
 * modulate halves one of its operands. */
constexpr br_uint_16 MMX_MASK5  = 0xf800;
constexpr br_uint_16 MMX_MASK6  = 0xfc00;
constexpr br_uint_16 MMX_MASK5D = 0x3e00;
constexpr br_uint_16 MMX_MASK6D = 0x3f00;

/* xzrgb.inc / xzuvc.inc / xzuv1.inc's `Convert to RGB 565`. */
template <sp_fmt F>
inline br_uint_16 MmxPackDirect(br_uint_16 r, br_uint_16 g, br_uint_16 b) noexcept
{
    if(F == SP_FMT_565)
        return (br_uint_16)((r & MMX_MASK5) | ((g & MMX_MASK6) >> 5) | (b >> 11));

    return (br_uint_16)(((r & MMX_MASK5) >> 1) | ((g & MMX_MASK5) >> 6) | (b >> 11));
}

/*
 * xzuv1.inc's MANY-WORD dither fragment's `Convert to RGB 565`. The two
 * `pand mask_5`/`pand mask_6` that clear the green channel sit inside
 * `if COLOUR_TYPE_15`, so the sixteen-bit many-word dither builds its green from
 * the whole green word and the channel's low five bits land in the blue field
 * instead of being shifted out. The fifteen-bit shape is masked exactly as the
 * non-dither pack is, so this is the non-dither pack with one lane moved.
 */
template <sp_fmt F>
inline br_uint_16 MmxPackDirectDither(br_uint_16 r, br_uint_16 g, br_uint_16 b) noexcept
{
    if(F == SP_FMT_565)
        return (br_uint_16)((r & MMX_MASK5) | (g >> 5) | (b >> 11));

    return (br_uint_16)(((r & MMX_MASK5) >> 1) | ((g & MMX_MASK5) >> 6) | (b >> 11));
}

/* xzuvrgb.inc's `Convert to RGB 565`. */
template <sp_fmt F>
inline br_uint_16 MmxPackMod(br_uint_16 r, br_uint_16 g, br_uint_16 b) noexcept
{
    if(F == SP_FMT_565)
        return (br_uint_16)(((r & MMX_MASK5D) << 2) | ((g & MMX_MASK6D) >> 3) | ((b & MMX_MASK5D) >> 9));

    return (br_uint_16)(((r & MMX_MASK5D) << 1) | ((g & MMX_MASK5D) >> 4) | ((b & MMX_MASK5D) >> 9));
}

/*
 * rastrise.asm's four magic squares, indexed by (scanline & 3) and then by the
 * pixel's column within its word. xzrgb/xzuvc/xzuv1 use the eleven/ten bit rows
 * - their fragment's channel sits at bits 11..15, so the square's eleven bits
 * are the fraction below it; xzuvrgb uses the nine/eight bit rows, because the
 * modulate leaves the channel at bits 9..13.
 */
constexpr br_uint_16 MMX_DITHER11[4][4] = {
    {0x07f, 0x37f, 0x4ff, 0x7ff},
    {0x5ff, 0x6ff, 0x17f, 0x27f},
    {0x3ff, 0x0ff, 0x77f, 0x47f},
    {0x67f, 0x57f, 0x2ff, 0x1ff}
};
constexpr br_uint_16 MMX_DITHER10[4][4] = {
    {0x03f, 0x1bf, 0x27f, 0x3ff},
    {0x2ff, 0x37f, 0x0bf, 0x13f},
    {0x1ff, 0x07f, 0x3bf, 0x23f},
    {0x33f, 0x2bf, 0x17f, 0x0ff}
};
constexpr br_uint_16 MMX_DITHER9[4][4] = {
    {0x01f, 0x0df, 0x13f, 0x1ff},
    {0x17f, 0x1bf, 0x05f, 0x09f},
    {0x0ff, 0x03f, 0x1df, 0x11f},
    {0x19f, 0x15f, 0x0bf, 0x07f}
};
constexpr br_uint_16 MMX_DITHER8[4][4] = {
    {0x00f, 0x06f, 0x09f, 0x0ff},
    {0x0bf, 0x0df, 0x02f, 0x04f},
    {0x07f, 0x01f, 0x0ef, 0x08f},
    {0x0cf, 0x0af, 0x05f, 0x03f}
};

/*
 * rastrise.asm's screendoor_masks: sixteen levels, four scanline rows each, one
 * word per pixel of the four-pixel word. The row the mask is looked up with is
 * (alpha nibble << 2) | (scanline & 3) and the pattern is aligned to the word,
 * so pixel p tests bit (p & 3).
 */
constexpr br_uint_16 MMX_SCREENDOOR[16][4][4] = {
    {{0, 0, 0, 0},                     {0, 0, 0, 0},                     {0, 0, 0, 0},                     {0, 0, 0, 0}               },
    {{0xffff, 0, 0, 0},                {0, 0, 0, 0},                     {0, 0, 0, 0},                     {0, 0, 0, 0}               },
    {{0xffff, 0, 0, 0},                {0, 0, 0, 0},                     {0, 0, 0xffff, 0},                {0, 0, 0, 0}               },
    {{0xffff, 0, 0xffff, 0},           {0, 0, 0, 0},                     {0, 0, 0xffff, 0},                {0, 0, 0, 0}               },
    {{0xffff, 0, 0xffff, 0},           {0, 0, 0, 0},                     {0xffff, 0, 0xffff, 0},           {0, 0, 0, 0}               },
    {{0xffff, 0, 0xffff, 0},           {0, 0xffff, 0, 0},                {0xffff, 0, 0xffff, 0},           {0, 0, 0, 0}               },
    {{0xffff, 0, 0xffff, 0},           {0, 0xffff, 0, 0},                {0xffff, 0, 0xffff, 0},           {0, 0, 0, 0xffff}          },
    {{0xffff, 0, 0xffff, 0},           {0, 0xffff, 0, 0xffff},           {0xffff, 0, 0xffff, 0},           {0, 0, 0, 0xffff}          },
    {{0xffff, 0, 0xffff, 0},           {0, 0xffff, 0, 0xffff},           {0xffff, 0, 0xffff, 0},           {0, 0xffff, 0, 0xffff}     },
    {{0xffff, 0xffff, 0xffff, 0},      {0, 0xffff, 0, 0xffff},           {0xffff, 0, 0xffff, 0},           {0, 0xffff, 0, 0xffff}     },
    {{0xffff, 0xffff, 0xffff, 0},      {0, 0xffff, 0, 0xffff},           {0xffff, 0, 0xffff, 0xffff},      {0, 0xffff, 0, 0xffff}     },
    {{0xffff, 0xffff, 0xffff, 0xffff}, {0, 0xffff, 0, 0xffff},           {0xffff, 0, 0xffff, 0xffff},      {0, 0xffff, 0, 0xffff}     },
    {{0xffff, 0xffff, 0xffff, 0xffff}, {0, 0xffff, 0, 0xffff},           {0xffff, 0xffff, 0xffff, 0xffff}, {0, 0xffff, 0, 0xffff}     },
    {{0xffff, 0xffff, 0xffff, 0xffff}, {0xffff, 0xffff, 0, 0xffff},      {0xffff, 0xffff, 0xffff, 0xffff}, {0, 0xffff, 0, 0xffff}     },
    {{0xffff, 0xffff, 0xffff, 0xffff}, {0xffff, 0xffff, 0, 0xffff},      {0xffff, 0xffff, 0xffff, 0xffff}, {0, 0xffff, 0xffff, 0xffff}},
    {{0xffff, 0xffff, 0xffff, 0xffff}, {0xffff, 0xffff, 0xffff, 0xffff}, {0xffff, 0xffff, 0xffff, 0xffff}, {0, 0xffff, 0xffff, 0xffff}},
};

/* UNPACK_CONSTANT_COLOUR's lanes: each byte duplicated into a sixteen-bit lane
 * and then the *word* halved - `punpcklbw` then `psrlw 1`, which is not the same
 * as halving the byte before doubling it. */
inline br_uint_16 MmxConstantLane(br_uint_32 word, br_int_32 shift) noexcept
{
    return (br_uint_16)(((br_uint_16)(br_uint_8)(word >> shift) * 257u) >> 1);
}

/*
 * One scanline of an MMX span. The asm walks word-aligned groups from the word
 * holding the major edge to the word holding the minor one, four pixels at a
 * time, and clips the two partial words with START_MASKS/END_MASKS; a per-pixel
 * range test is exactly that clip, and the four-pixel group survives in the lane
 * index the colour words are taken from and in the fact that the texture
 * accumulator steps for every pixel of the group, drawn or not.
 *
 * The depth test and store run for every pixel the z mask passes, before the
 * texel is looked at; the colour store additionally needs an opaque texel. Both
 * are gated by the screendoor when there is one.
 */
template <sp_fmt F, sp_depth D, sp_shade S, sp_tex X, sp_blend B, sp_dith H, int DIRN>
void MmxScan(MmxWalk& w, const softprim_buffer& dbuf, const softprim_buffer& tex, const br_uint_16 clane[3], const br_uint_16 *sdmask, br_int_32 y) noexcept
{
    const br_int_32 xmaj = w.main_i >> 16;
    const br_int_32 xmin = w.minor_i >> 16;

    /* J_EMPTY: the asm tests the two edge pixels and skips the scanline. */
    if(DIRN == RPD_FWD ? (xmaj > xmin) : (xmaj < xmin))
        return;

    const br_int_32 origin = xmaj & ~3;
    const br_int_32 lo     = (xmaj < xmin) ? xmaj : xmin;
    const br_int_32 hi     = (xmaj < xmin) ? xmin : xmaj;

    /* The asm's `and eax,not 3; and ebx,not 3; sub eax,ebx; je one_word`: a
     * span whose two edge pixels share a word is rendered by the one-word
     * colour pass rather than the many-word one, and the two differ (see
     * MmxPackDirectDither). */
    const bool one_word = (xmaj & ~3) == (xmin & ~3);

    /* The walk covers the words from the major edge's to the minor edge's. For
     * the right-to-left blocks the first pixel of the first word is the word's
     * last one, which is where UNPACK_UV_32's advance-and-negate put the
     * texture accumulator. */
    br_int_32       p     = (DIRN == RPD_FWD) ? origin : origin + 3;
    const br_int_32 pend  = (DIRN == RPD_FWD) ? ((xmin & ~3) + 3) : (xmin & ~3);
    const br_int_32 pstep = (DIRN == RPD_FWD) ? 1 : -1;

    br_uint_32 u = w.u;
    br_uint_32 v = w.v;

    /*
     * xzuv1.inc walks its texture coordinate a word at a time from four
     * per-lane accumulators (see MmxWalk) where the other textured bodies
     * accumulate one coordinate per pixel. The lane values are taken from the
     * word's own lane and the whole word advances by w.d_u_word when the walk
     * crosses into the next one.
     */
    constexpr bool lane_model = (X == SP_TEX_I8 && S == SP_SHADE_NONE);

    br_uint_32 lu[4] = {0, 0, 0, 0};
    br_uint_32 lv[4] = {0, 0, 0, 0};

    if(lane_model) {
        for(int k = 0; k < 4; k++) {
            lu[k] = w.lane_u[k];
            lv[k] = w.lane_v[k];
        }
    }

    br_int_32 cur_word = p >> 2;

    (void)dbuf;
    (void)tex;
    (void)clane;
    (void)y;

    for(; DIRN == RPD_FWD ? (p <= pend) : (p >= pend); p += pstep) {
        if(lane_model) {
            const br_int_32 pw = p >> 2;

            if(pw != cur_word) {
                for(int k = 0; k < 4; k++) {
                    lu[k] = (lu[k] + w.d_u_word) & w.u_mask;
                    lv[k] = (lv[k] + w.d_v_word) & w.v_mask;
                }

                cur_word = pw;
            }

            u = lu[p & 3];
            v = lv[p & 3];
        }

        if(p >= lo && p <= hi) {
            const br_int_32  rel  = p - origin;
            const br_int_32  lane = rel & 3;
            const br_int_32  grp  = rel >> 2;
            const br_uint_16 sdb  = (sdmask != NULL) ? sdmask[p & 3] : (br_uint_16)0xffff;

            bool vis = sdb != 0;

            const br_uint_16 newz = (br_uint_16)((br_uint_32)((br_uint_32)w.pz.current + (br_uint_32)w.pz.grad_x * (br_uint_32)rel) >> 16);
            br_uint_8       *zp   = w.zscan + (br_size_t)p * 2;

            if(vis && D == SP_DEPTH_ZW && newz > LoadDepth(zp))
                vis = false;

            if(vis && D == SP_DEPTH_ZW)
                StoreDepth(zp, newz);

            if(vis) {
                bool       opaque = true;
                br_uint_16 r = 0, g = 0, b = 0;

                if(X == SP_TEX_I8) {
                    const br_uint_8 texel = tex.base[(u + v) >> 12];

                    if(texel == 0) {
                        opaque = false;
                    } else {
                        const br_uint_8 *pe = tex.palette + (br_size_t)texel * tex.palette_stride_b;
                        const br_uint_16 tb = pe[0], tg = pe[1], tr = pe[2];

                        if(S == SP_SHADE_NONE) {
                            /* xzuv1.inc: the palette bytes go straight through. */
                            r = (br_uint_16)(tr * 257u);
                            g = (br_uint_16)(tg * 257u);
                            b = (br_uint_16)(tb * 257u);
                        } else if(S == SP_SHADE_CONST_RGB) {
                            /* xzuvc.inc: two seven-bit operands, and the product
                             * comes out one bit low so it is shifted back up. */
                            r = (br_uint_16)(MmxPmulhw((br_uint_16)(((br_uint_16)(tr * 257u)) >> 1), clane[0]) << 2);
                            g = (br_uint_16)(MmxPmulhw((br_uint_16)(((br_uint_16)(tg * 257u)) >> 1), clane[1]) << 2);
                            b = (br_uint_16)(MmxPmulhw((br_uint_16)(((br_uint_16)(tb * 257u)) >> 1), clane[2]) << 2);
                        } else {
                            /* xzuvrgb.inc: the palette byte is halved before it is
                             * doubled, and the interpolated colour is the other
                             * operand. */
                            r = MmxPmulhw((br_uint_16)((br_uint_16)(tr >> 1) * 257u), MmxLaneAt(w.col[0], lane, grp));
                            g = MmxPmulhw((br_uint_16)((br_uint_16)(tg >> 1) * 257u), MmxLaneAt(w.col[1], lane, grp));
                            b = MmxPmulhw((br_uint_16)((br_uint_16)(tb >> 1) * 257u), MmxLaneAt(w.col[2], lane, grp));
                        }
                    }
                } else {
                    r = MmxLaneAt(w.col[0], lane, grp);
                    g = MmxLaneAt(w.col[1], lane, grp);
                    b = MmxLaneAt(w.col[2], lane, grp);
                }

                if(opaque) {
                    if(H == SP_DITH_COLOUR) {
                        const int row = y & 3, col = p & 3;

                        if(X == SP_TEX_I8 && S == SP_SHADE_INTERP_RGB) {
                            r = (br_uint_16)(r + MMX_DITHER9[row][col]);
                            g = (br_uint_16)(g + MMX_DITHER8[row][col]);
                            b = (br_uint_16)(b + MMX_DITHER9[row][col]);
                        } else if(X == SP_TEX_I8 && S == SP_SHADE_NONE) {
                            /* xzuv1.inc is the one body that dithers with a
                             * saturating add. */
                            r = (br_uint_16)(((br_uint_32)r + MMX_DITHER11[row][col] > 0xffffu) ? 0xffffu : (br_uint_32)r + MMX_DITHER11[row][col]);
                            g = (br_uint_16)(((br_uint_32)g + MMX_DITHER10[row][col] > 0xffffu) ? 0xffffu : (br_uint_32)g + MMX_DITHER10[row][col]);
                            b = (br_uint_16)(((br_uint_32)b + MMX_DITHER11[row][col] > 0xffffu) ? 0xffffu : (br_uint_32)b + MMX_DITHER11[row][col]);
                        } else {
                            r = (br_uint_16)(r + MMX_DITHER11[row][col]);
                            g = (br_uint_16)(g + MMX_DITHER10[row][col]);
                            b = (br_uint_16)(b + MMX_DITHER11[row][col]);
                        }
                    }

                    const br_uint_16 out = (X == SP_TEX_I8 && S == SP_SHADE_INTERP_RGB) ? MmxPackMod<F>(r, g, b)
                                           : (X == SP_TEX_I8 && S == SP_SHADE_NONE && H == SP_DITH_COLOUR && F == SP_FMT_565 && !one_word)
                                               ? MmxPackDirectDither<F>(r, g, b)
                                               : MmxPackDirect<F>(r, g, b);
                    br_uint_8       *cp  = w.scan + (br_size_t)p * 2;

                    cp[0] = (br_uint_8)out;
                    cp[1] = (br_uint_8)(out >> 8);
                }
            }
        }

        if(!lane_model) {
            u = (u + w.d_u_x) & w.u_mask;
            v = (v + w.d_v_x) & w.v_mask;
        }
    }
}

/*
 * pentprim's rasteriser buffer.
 *
 * The MMX tables are the only ones that draw through pentprim's generic setup
 * code (gsetuptf.asm), and that code does not rasterise a triangle: it stacks
 * the triangle's parameter block in rasteriseBuffer and rasterises it later,
 * when the buffer is full or the frame ends (RasteriseBufferFlush). The stack
 * grows downwards from rasteriseBufferLast and the flush starts at its top, so
 * the triangles of one batch are rasterised in the reverse of the order they
 * were submitted. The source calls the ordering out itself - "XXX should
 * rearrange this so that stack is built bottom to top, and then flushed - means
 * that prims are rasterised in order" (rastbuff.asm).
 *
 * That order is visible wherever two primitives meet at the same depth: the
 * depth test passes on equality (xzrgb.inc's psubusw/pcmpeqw mask), so the pixel
 * keeps whichever of the two was rasterised last, and the reversal within a
 * batch decides which that is. Reproducing the batch is therefore part of
 * reproducing the kernel, not an ordering the rasteriser is free to choose.
 *
 * A batch holds as many parameter blocks as fit in rasteriseBuffer's 200 qwords.
 * The setup subtracts the block's parameter_struct size from the top and flushes
 * when the result would fall below the buffer (gsetuptf.asm:291-316), so a frame
 * is admitted while the queued total plus it stays within the buffer. The check
 * is made before the triangle is looked at - an empty triangle has already
 * flushed, and then releases its frame again (gsetuptf.asm's empty_triangle).
 *
 * The parameter structs are rastparm.h's, and the MMX tables use three of them
 * (infogen.pl's parameter_struct, mmx_t15.ifg/mmx_t16.ifg): param_zrgbuv for the
 * interpolated-colour textured family, and param_zuv/param_zrgb for the rest.
 * Each is the 56-byte tsb_header plus four dwords per parameter, and a textured
 * parameter set carries the extra 16-byte texture_info - 168 bytes for
 * param_zrgbuv against 120 for the other two.
 */
constexpr br_size_t MMX_RASTER_BUFFER_BYTES = 200 * 8;

/* The block's parameter_struct size, keyed by the axes that select the family.
 * Only the three MMX triangle families occur, and each has one size. */
template <sp_shade S, sp_tex X>
constexpr br_size_t MmxFrameSize() noexcept
{
    return (X == SP_TEX_I8 && S == SP_SHADE_INTERP_RGB) ? 168 : 120;
}

/*
 * One trapezium: the asm's TrapeziumRender scanline loop. The per-scanline
 * increments are selected by the carry out of the four-pixel boundary of the
 * major edge - the asm's `xor edx,ecx; xor edx,eax; shl edx,14; sbb edx,edx`,
 * whose xored bit is bit 18 of the sum's carry vector, which is the boundary the
 * pixel stride puts the parameters on.
 */
template <sp_fmt F, sp_depth D, sp_shade S, sp_tex X, sp_blend B, sp_dith H, int DIRN>
void MmxTrapezium(MmxWalk& w, const softprim_buffer& dbuf, const softprim_buffer& tex, const br_uint_16 clane[3], const br_uint_16 (*sd)[4],
                  br_int_32& y) noexcept
{
    (void)dbuf;
    (void)tex;
    (void)clane;

    for(br_int_32 n = w.minor_count; n >= 0; n--) {
        MmxScan<F, D, S, X, B, H, DIRN>(w, dbuf, tex, clane, (sd != NULL) ? sd[y & 3] : NULL, y);

        const br_uint_32 carry = (((br_uint_32)w.main_i & 0x3ffffu) + ((br_uint_32)w.main_d & 0x3ffffu)) >= 0x40000u ? 1u : 0u;

        w.main_i += w.main_d;
        w.minor_i += w.minor_d;

        w.pz.current += carry ? w.pz.d_carry : w.pz.d_nocarry;

        for(int c = 0; c < 3; c++) {
            const br_uint_16 add = carry ? w.col[c].dy1 : w.col[c].dy0;

            for(int k = 0; k < 4; k++)
                w.col[c].w[k] = (br_uint_16)(w.col[c].w[k] + add);
        }

        w.u = (w.u + (carry ? w.du_y1 : w.du_y0)) & w.u_mask;
        w.v = (w.v + (carry ? w.dv_y1 : w.dv_y0)) & w.v_mask;

        /* xzuv1.inc advances each of its four word-lane accumulators by the
         * same packed per-scanline delta (see MmxWalk). */
        if constexpr(X == SP_TEX_I8 && S == SP_SHADE_NONE) {
            const br_uint_32 duy = carry ? w.du_y1 : w.du_y0;
            const br_uint_32 dvy = carry ? w.dv_y1 : w.dv_y0;

            for(int k = 0; k < 4; k++) {
                w.lane_u[k] = (w.lane_u[k] + duy) & w.u_mask;
                w.lane_v[k] = (w.lane_v[k] + dvy) & w.v_mask;
            }
        }

        w.scan += w.scan_stride_b;
        w.zscan += w.zscan_stride_b;

        y++;
    }
}

/*
 * A queued triangle: everything the two trapezium walks read, captured at setup
 * time the way pentprim's parameter block captures it (the texture address is
 * part of the frame there too, so a frame flushed later samples the map that
 * was bound when it was set up). The walk itself stays as setup left it - the
 * second trapezium's geometry and the direction are carried beside it.
 */
struct MmxFrame {
    void (*run)(const MmxFrame&) noexcept;
    MmxWalk        w;
    softprim_buffer dbuf, tex;
    const br_uint_16 (*sd)[4];
    br_uint_16 clane[3];
    br_int_32  y;
    br_int_32  x2, d_x2, bottom_count;
    br_int_32  dir;
    br_size_t  bytes;
};

/* The bound the byte budget puts on a batch: the smallest frame the MMX tables
 * queue is 120 bytes (MmxFrameSize, above), so no batch can hold more. */
constexpr int MMX_FRAME_MAX = MMX_RASTER_BUFFER_BYTES / 120;

MmxFrame  mmxFrames[MMX_FRAME_MAX];
int       mmxFrameCount = 0;
br_size_t mmxFrameBytes = 0;

/*
 * Rasterise the queued frames, newest first, which is the order the asm's
 * RASTERISE_EXIT chain walks the stack in (rastmacs.inc:20-28).
 */
void MmxFlush() noexcept
{
    for(int i = mmxFrameCount - 1; i >= 0; i--)
        mmxFrames[i].run(mmxFrames[i]);

    mmxFrameCount = 0;
    mmxFrameBytes = 0;
}

/*
 * The two trapeziums of one queued triangle. The walk is copied out of the frame
 * because the trapezium loop advances it.
 */
template <sp_fmt F, sp_depth D, sp_shade S, sp_tex X, sp_blend B, sp_dith H>
void MmxRun(const MmxFrame& f) noexcept
{
    MmxWalk   w = f.w;
    br_int_32 y = f.y;

    if(f.dir == 0)
        MmxTrapezium<F, D, S, X, B, H, RPD_FWD>(w, f.dbuf, f.tex, f.clane, f.sd, y);
    else
        MmxTrapezium<F, D, S, X, B, H, RPD_BWD>(w, f.dbuf, f.tex, f.clane, f.sd, y);

    w.minor_i     = f.x2;
    w.minor_d     = f.d_x2;
    w.minor_count = f.bottom_count;

    if(f.dir == 0)
        MmxTrapezium<F, D, S, X, B, H, RPD_FWD>(w, f.dbuf, f.tex, f.clane, f.sd, y);
    else
        MmxTrapezium<F, D, S, X, B, H, RPD_BWD>(w, f.dbuf, f.tex, f.clane, f.sd, y);
}

/*
 * One MMX triangle. F is the output format, D the depth mode, S the shading
 * source and X the texture decode; B and H are carried so the dispatch is one
 * line per tuple.
 */
template <sp_fmt F, sp_depth D, sp_shade S, sp_tex X, sp_blend B, sp_dith H>
void MmxTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) noexcept
{
    (void)block;

    /*
     * The buffer-full flush comes first, before the triangle is looked at: the
     * asm makes the same check before its setup starts (gsetuptf.asm:291).
     */
    if(mmxFrameBytes + MmxFrameSize<S, X>() > MMX_RASTER_BUFFER_BYTES)
        MmxFlush();

    const softprim_buffer& cbuf = SoftPrimWork.colour;
    const softprim_buffer& dbuf = SoftPrimWork.depth;
    const softprim_buffer& tex  = SoftPrimWork.texture;

    if(cbuf.type != ((F == SP_FMT_565) ? (br_uint_8)BR_PMT_RGB_565 : (br_uint_8)BR_PMT_RGB_555) || cbuf.base == NULL || cbuf.width_p <= 0 ||
       cbuf.height <= 0)
        return;

    if(D == SP_DEPTH_ZW && (dbuf.type != BR_PMT_DEPTH_16 || dbuf.base == NULL))
        return;

    if(X == SP_TEX_I8) {
        if(tex.base == NULL || tex.type != BR_PMT_INDEX_8 || tex.width_p <= 0 || tex.height <= 0 || tex.stride_b <= 0 || tex.palette == NULL)
            return;
    }

    brp_vertex *src[3] = {v0, v1, v2};
    TexVert     in[3];

    for(int k = 0; k < 3; k++) {
        in[k].x  = BrFloatToFixed(src[k]->comp[C_SX]);
        in[k].y  = BrFloatToFixed(src[k]->comp[C_SY]);
        in[k].z  = BrFloatToFixed(src[k]->comp[C_SZ]);
        in[k].u  = BrFloatToFixed(src[k]->comp[C_U]);
        in[k].v  = BrFloatToFixed(src[k]->comp[C_V]);
        in[k].rx = src[k]->comp[C_SX];
        in[k].ry = src[k]->comp[C_SY];
        in[k].rz = src[k]->comp[C_SZ];
        in[k].ru = src[k]->comp[C_U];
        in[k].rv = src[k]->comp[C_V];
        in[k].rr = src[k]->comp[C_R];
        in[k].rg = src[k]->comp[C_G];
        in[k].rb = src[k]->comp[C_B];
        in[k].ra = src[k]->comp[C_A];

        if(!isfinite(in[k].rx) || !isfinite(in[k].ry) || !isfinite(in[k].rz))
            return;

        if(X == SP_TEX_I8 && (!isfinite(in[k].ru) || !isfinite(in[k].rv)))
            return;
    }

    /*
     * An untextured constant-colour block's colour is vertex 0's, and softrend
     * leaves the other vertices' slots alone: the block does not carry
     * BR_PRIMF_CONST_DUPLICATE, which pentprim's own table sets on exactly
     * these rows and which is what makes softrend copy the constant into every
     * vertex. Without the copy the setup would take the colour gradient from
     * three unrelated numbers and the fragment would be an interpolation of
     * vertex 0's colour with two zeroes. The general RGB kernel above does the
     * same for the same reason.
     */
    if(S == SP_SHADE_CONST_RGB && X == SP_TEX_NONE) {
        in[1].rr = in[2].rr = in[0].rr;
        in[1].rg = in[2].rg = in[0].rg;
        in[1].rb = in[2].rb = in[0].rb;
    }

    TriSetup ts;

    if(!BuildTriSetup(in, ts, 4))
        return;

    MmxWalk w;

    for(int c = 0; c < 3; c++)
        w.col[c] = MmxLane{};

    SetupFloatParam<16>(w.pz, ts, ts.orig[0].rz, ts.orig[1].rz, ts.orig[2].rz, ts.v[0].rz, true);

    /* The interpolated colour is 8.24; the constant-colour body takes its
     * fragment from the block's `c` word instead. */
    if(S != SP_SHADE_NONE && !(S == SP_SHADE_CONST_RGB && X == SP_TEX_I8)) {
        TrapParam pr, pg, pb;

        SetupFloatParam<24>(pr, ts, ts.orig[0].rr, ts.orig[1].rr, ts.orig[2].rr, ts.v[0].rr, false);
        SetupFloatParam<24>(pg, ts, ts.orig[0].rg, ts.orig[1].rg, ts.orig[2].rg, ts.v[0].rg, false);
        SetupFloatParam<24>(pb, ts, ts.orig[0].rb, ts.orig[1].rb, ts.orig[2].rb, ts.v[0].rb, false);

        w.col[0] = MmxMakeLane(pr);
        w.col[1] = MmxMakeLane(pg);
        w.col[2] = MmxMakeLane(pb);
    }

    /* SETUP_FLOAT_CONSTANT_COLOUR / SETUP_FLOAT_CONSTANT_ALPHA: vertex 0's
     * rounded components, the colour packed as ARGB. Only the alpha byte is read
     * back out by the screendoor bodies, but the constant-colour body reads the
     * R,G,B bytes too. */
    br_uint_32 cword    = 0;
    br_uint_16 clane[3] = {0, 0, 0};

    if(S == SP_SHADE_CONST_RGB && X == SP_TEX_I8) {
        cword = ((br_uint_32)(br_uint_8)llrintl((long double)ts.orig[0].rr) << 16) |
                ((br_uint_32)(br_uint_8)llrintl((long double)ts.orig[0].rg) << 8) | (br_uint_32)(br_uint_8)llrintl((long double)ts.orig[0].rb);
    }

    const br_uint_16(*sdtable)[4] = NULL;

    if(B == SP_BLEND_SCREENDOOR) {
        const br_uint_32 alevel = (br_uint_8)llrintl((long double)ts.orig[0].ra * 256.0L);

        cword |= (alevel & 0xffu) << 24;
        sdtable = MMX_SCREENDOOR[(alevel >> 4) & 0x0fu];
    }

    if(S == SP_SHADE_CONST_RGB && X == SP_TEX_I8) {
        clane[0] = MmxConstantLane(cword, 16);
        clane[1] = MmxConstantLane(cword, 8);
        clane[2] = MmxConstantLane(cword, 0);
    }

    if(X == SP_TEX_I8) {
        TrapParam pu, pv;

        SetupFloatParam<12>(pu, ts, ts.orig[0].ru, ts.orig[1].ru, ts.orig[2].ru, ts.v[0].ru, false);
        SetupFloatParam<12>(pv, ts, ts.orig[0].rv, ts.orig[1].rv, ts.orig[2].rv, ts.v[0].rv, false);

        const br_int_32  ws      = MmxFindShift(tex.width_p);
        const br_int_32  hs      = MmxFindShift(tex.height);
        const br_uint_32 u_field = ((1u << ws) - 1u) << (MMX_FRACTION_BITS + 1);
        const br_uint_32 v_field = ((1u << (ws + hs)) - 1u) << (MMX_FRACTION_BITS + 1);
        const br_uint_32 v_high  = v_field & ~u_field;

        w.u_mask = u_field | MMX_UV_FRACTION_MASK;
        w.v_mask = v_high | MMX_UV_FRACTION_MASK;

        br_int_32 su = pu.current, du = pu.grad_x;
        br_int_32 sv = pv.current, dv = pv.grad_x;

        if(ts.asm_dir != 0) {
            /* UNPACK_UV_32's right-to-left adjustment: advance three pixels so
             * the accumulator starts at the end of the four-pixel word, and
             * negate the step so the walk runs the other way. The per-scanline
             * increments are left alone - they step the parameter down a
             * scanline, which is the same either way. */
            su = (br_int_32)((br_uint_32)su + 3u * (br_uint_32)du);
            sv = (br_int_32)((br_uint_32)sv + 3u * (br_uint_32)dv);
            du = (br_int_32)(0u - (br_uint_32)du);
            dv = (br_int_32)(0u - (br_uint_32)dv);
        }

        w.u     = MmxPackCoord(su, 0, u_field, 0) & w.u_mask;
        w.d_u_x = MmxPackCoord(du, 0, u_field, MMX_UV_BRIDGE);

        w.v     = MmxPackCoord(sv, ws, v_high, 0) & w.v_mask;
        w.d_v_x = MmxPackCoord(dv, ws, v_high, MMX_UV_BRIDGE | u_field);

        /*
         * The per-scanline steps are packed the same way, and for the same
         * reason: the walk adds one of them to the accumulator and then masks
         * the result, so a step that carried the 20.12 fraction as ordinary
         * bits would have its carry land in the fraction field instead of the
         * integer one. UNPACK_UV_32 packs d_u_y1/d_u_y0 out of the same quad
         * words as d_u_x - they are the high halves of the two `movq`s - and
         * so they keep the bridge too.
         */
        w.du_y0 = MmxPackCoord(pu.d_nocarry, 0, u_field, MMX_UV_BRIDGE);
        w.du_y1 = MmxPackCoord(pu.d_carry, 0, u_field, MMX_UV_BRIDGE);
        w.dv_y0 = MmxPackCoord(pv.d_nocarry, ws, v_high, MMX_UV_BRIDGE | u_field);
        w.dv_y1 = MmxPackCoord(pv.d_carry, ws, v_high, MMX_UV_BRIDGE | u_field);

        /*
         * xzuv1.inc's four accumulators (see MmxWalk). Lane k is the coordinate
         * of the word's pixel k, packed from the exact 20.12 sum pu.current +
         * k*pu.grad_x - the unadjusted start, so the right-to-left adjustment
         * above is already undone by the asm's own lane mapping. The word step
         * is the same pack applied to the whole four-pixel delta, so it keeps
         * the bridge the per-pixel steps above carry.
         */
        if(S == SP_SHADE_NONE) {
            for(int k = 0; k < 4; k++) {
                w.lane_u[k] = MmxPackCoord((br_int_32)((br_uint_32)pu.current + (br_uint_32)k * (br_uint_32)pu.grad_x), 0, u_field, 0);
                w.lane_v[k] = MmxPackCoord((br_int_32)((br_uint_32)pv.current + (br_uint_32)k * (br_uint_32)pv.grad_x), ws, v_high, 0);
            }

            w.d_u_word = MmxPackCoord((br_int_32)((br_uint_32)du << 2), 0, u_field, MMX_UV_BRIDGE);
            w.d_v_word = MmxPackCoord((br_int_32)((br_uint_32)dv << 2), ws, v_high, MMX_UV_BRIDGE | u_field);
        } else {
            for(int k = 0; k < 4; k++)
                w.lane_u[k] = w.lane_v[k] = 0;

            w.d_u_word = w.d_v_word = 0;
        }
    } else {
        w.u = w.v = 0;
        w.d_u_x = w.d_v_x = 0;
        w.du_y0 = w.du_y1 = w.dv_y0 = w.dv_y1 = 0;
        w.u_mask = w.v_mask = ~(br_uint_32)0;

        for(int k = 0; k < 4; k++)
            w.lane_u[k] = w.lane_v[k] = 0;

        w.d_u_word = w.d_v_word = 0;
    }

    w.main_i = ts.walk.xm;
    w.main_d = ts.walk.d_xm;
    w.scan   = cbuf.base + (br_size_t)ts.walk.yTop * cbuf.stride_b;
    w.zscan  = ((D == SP_DEPTH_ZW) ? dbuf.base : cbuf.base) + (br_size_t)ts.walk.yTop * ((D == SP_DEPTH_ZW) ? dbuf.stride_b : 0);

    w.scan_stride_b  = cbuf.stride_b;
    w.zscan_stride_b = (D == SP_DEPTH_ZW) ? dbuf.stride_b : 0;

    w.minor_i     = ts.walk.x1;
    w.minor_d     = ts.walk.d_x1;
    w.minor_count = ts.walk.topCount;

    MmxFrame& f = mmxFrames[mmxFrameCount++];

    f.run          = &MmxRun<F, D, S, X, B, H>;
    f.w            = w;
    f.dbuf         = dbuf;
    f.tex          = tex;
    f.sd           = sdtable;
    f.clane[0]     = clane[0];
    f.clane[1]     = clane[1];
    f.clane[2]     = clane[2];
    f.y            = ts.walk.yTop;
    f.x2           = ts.walk.x2;
    f.d_x2         = ts.walk.d_x2;
    f.bottom_count = ts.walk.bottomCount;
    f.dir          = ts.asm_dir;
    f.bytes        = MmxFrameSize<S, X>();

    mmxFrameBytes += f.bytes;
}

} // namespace

/*
 * Rasterise the frames the MMX tables have queued but not yet drawn. Called
 * where pentprim's primitive library flushes its rasteriser buffer, which is
 * the end of a scene render (softrend's RendererFlush) - see plib.c.
 */
extern "C" void SoftPrimMmxFlush(void)
{
    MmxFlush();
}

/*
 * Instantiate one kernel per generated block.
 *
 * The assertions are the contract with the generator: widening the axis spec in
 * CMakeLists.txt without writing a kernel is a compile error naming the axis,
 * rather than a block that matches and draws nothing.
 */
template <sp_fmt F, sp_top T, sp_depth D, sp_shade S, sp_tex X, sp_addr A, sp_persp P, sp_blend B, sp_fog G, sp_dith H, bool C7>
static void SoftPrimRender(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2)
{
    static_assert(T == SP_TOP_TRI, "softprim: no kernel for this primitive topology");
    static_assert(S == SP_SHADE_NONE || S == SP_SHADE_CONST_I || S == SP_SHADE_INTERP_I || S == SP_SHADE_CONST_I_TABLE ||
                      S == SP_SHADE_INTERP_I_TABLE || S == SP_SHADE_CONST_RGB || S == SP_SHADE_INTERP_RGB || S == SP_SHADE_CONST_I_RGB ||
                      S == SP_SHADE_INTERP_I_RGB,
                  "softprim: no kernel for this shading mode");
    static_assert(X == SP_TEX_NONE || X == SP_TEX_I8 || X == SP_TEX_555 || X == SP_TEX_565 || X == SP_TEX_RGB888,
                  "softprim: no kernel for this texture mode");
    static_assert(A == SP_ADDR_NONE || A == SP_ADDR_SHIFT || A == SP_ADDR_DIVIDE, "softprim: no kernel for this texture addressing mode");
    static_assert(X != SP_TEX_NONE || A == SP_ADDR_NONE, "softprim: texture addressing mode without a texture");
    static_assert(X != SP_TEX_RGB888 || F == SP_FMT_888, "softprim: an RGB_888 texture is only implemented for RGB_888 output");
    static_assert(X != SP_TEX_555 || F == SP_FMT_555, "softprim: an RGB_555 texture is only implemented for RGB_555 output");
    static_assert(X != SP_TEX_565 || F == SP_FMT_565, "softprim: an RGB_565 texture is only implemented for RGB_565 output");
    static_assert(P == SP_PERSP_AFFINE || P == SP_PERSP_CORRECT, "softprim: no kernel for this perspective mode");
    static_assert((S != SP_SHADE_CONST_I && S != SP_SHADE_INTERP_I && S != SP_SHADE_CONST_I_TABLE && S != SP_SHADE_INTERP_I_TABLE) || F == SP_FMT_I8,
                  "softprim: intensity shading is INDEX_8 only");
    static_assert((S != SP_SHADE_CONST_RGB && S != SP_SHADE_INTERP_RGB) || F != SP_FMT_I8, "softprim: RGB shading is not an INDEX_8 mode");
    static_assert(P == SP_PERSP_AFFINE || X != SP_TEX_NONE, "softprim: an untextured kernel cannot be perspective-correct");
    static_assert(B == SP_BLEND_NONE || B == SP_BLEND_INDEX || B == SP_BLEND_DECAL || B == SP_BLEND_SCREENDOOR ||
                      (B == SP_BLEND_ALPHA && X == SP_TEX_I8 && A == SP_ADDR_SHIFT &&
                       (S == SP_SHADE_CONST_I_RGB || S == SP_SHADE_INTERP_I_RGB)),
                  "softprim: no kernel for this blend mode");
    static_assert(H == SP_DITH_NONE || H == SP_DITH_MAP || H == SP_DITH_COLOUR, "softprim: no kernel for this dithering mode");
    static_assert(H != SP_DITH_MAP || (F == SP_FMT_I8 && S == SP_SHADE_NONE && X == SP_TEX_I8 && A == SP_ADDR_SHIFT &&
                                       P == SP_PERSP_CORRECT && B == SP_BLEND_NONE && G == SP_FOG_NONE),
                  "softprim: the dithered map is implemented for the INDEX_8 perspective-shift path only");
    static_assert((B != SP_BLEND_SCREENDOOR && H != SP_DITH_COLOUR) || (F != SP_FMT_I8 && F != SP_FMT_888),
                  "softprim: the screendoor and dithered-colour ROPs are RGB_555/565 only");
    static_assert(B != SP_BLEND_SCREENDOOR || H == SP_DITH_NONE || H == SP_DITH_COLOUR, "softprim: screendoor with an unknown dither");

    if(F == SP_FMT_I8) {
        if(X == SP_TEX_I8) {
            if(B == SP_BLEND_DECAL)
                DecalTriangle<D, S, A>(block, v0, v1, v2);
            else if(H == SP_DITH_MAP)
                PizDitherTriangle<D>(block, v0, v1, v2);
            else if(A == SP_ADDR_DIVIDE)
                TexturedDivideTriangle<D, S, B, G>(block, v0, v1, v2);
            else
                TexturedIndexedTriangle<D, S, P, B, G>(block, v0, v1, v2);
        } else
            IndexedTriangle<D, S, G>(block, v0, v1, v2);
    } else if(X == SP_TEX_I8 && A == SP_ADDR_SHIFT && D == SP_DEPTH_NONE &&
              (S == SP_SHADE_CONST_I_RGB || S == SP_SHADE_INTERP_I_RGB)) {
        /*
         * The 256x256 RGB shade-table cells: persp.c's PITIP256/PITIPB256, which
         * are perspi.h at BPP 2, SIZE 256 and LIGHT 1. Their setup, addressing
         * and trapezium walk are the dithered-map cell's (PizTriangle), and their
         * span is t15_pip.asm's ScanLinePITIP - not the pfpsetup.asm family the
         * indexed texture path above ports. ADDR_SHIFT is the map-size
         * instantiation - the block declares 256x256 and the matcher requires
         * width and height to equal it - which is why a map of any other size
         * reaches the arbitrary-width cell instead. They are z-sorted only: at
         * 15/16bpp the MMX table is tried first and takes the z-buffered twins,
         * so no depth reaches this kernel. The span reads the destination only
         * for the blend, which is the one ROP the cell's blocks carry.
         */
        static_assert(!(X == SP_TEX_I8 && A == SP_ADDR_SHIFT && D == SP_DEPTH_NONE &&
                        (S == SP_SHADE_CONST_I_RGB || S == SP_SHADE_INTERP_I_RGB)) ||
                          B == SP_BLEND_NONE || B == SP_BLEND_ALPHA,
                      "softprim: no kernel for this blend mode on the 256x256 RGB shade-table cell");

        PizShadeTriangle<F, S, B>(block, v0, v1, v2);
    } else if(X != SP_TEX_NONE && A == SP_ADDR_DIVIDE &&
              (F == SP_FMT_888 || X == SP_TEX_555 || X == SP_TEX_565 || S == SP_SHADE_CONST_I_RGB || S == SP_SHADE_INTERP_I_RGB)) {
        /*
         * awtmi.h's arbitrary-width family, which has its own setup and its own
         * addressing (see RgbAwtTriangle). Three shapes reach it: the RGB_888
         * textured blocks, whose sample is the map itself or a shade-table
         * lookup; the z-sorted RGB_555/565 shade-table blocks, whose fragment is
         * the same LIGHT lookup into a table typed to the output; and the
         * z-sorted RGB_555/565 colour-map blocks, whose fragment is the map's
         * own texel word copied - the same code with SBPP=2 rather than 3. The
         * power-of-two 15/16bpp shapes, the z-buffered 555/565 ones and the
         * line/point ones the generator refuses, so they never reach here.
         */
        if(X == SP_TEX_I8)
            RgbAwtTriangle<F, D, SP_TEX_I8, S>(block, v0, v1, v2);
        else if(X == SP_TEX_555)
            RgbAwtTriangle<F, D, SP_TEX_555, S>(block, v0, v1, v2);
        else if(X == SP_TEX_565)
            RgbAwtTriangle<F, D, SP_TEX_565, S>(block, v0, v1, v2);
        else
            RgbAwtTriangle<F, D, SP_TEX_RGB888, S>(block, v0, v1, v2);
    } else if(X == SP_TEX_NONE && (F == SP_FMT_888 || D == SP_DEPTH_NONE)) {
        /*
         * The untextured perfect-scan blocks, not the SETUP_FLOAT geometry and
         * not the MMX family: RGB_888 always (tt24_piz.asm/tt24_pi.asm) and the
         * z-sorted 15/16bpp pair (tt15_pi.asm/tt16_pi.asm). The two sample the
         * same triangle at different pixels, so they are separate kernels, and
         * the 15/16bpp z-buffered shapes are the MMX table's and are caught by
         * the branch below.
         */
        if(S == SP_SHADE_INTERP_RGB)
            RgbPiTriangle<F, D, true>(block, v0, v1, v2);
        else
            RgbPiTriangle<F, D, false>(block, v0, v1, v2);
    } else if(F != SP_FMT_888 && D == SP_DEPTH_ZW) {
        /*
         * Every 15/16bpp tuple with a depth buffer is the MMX family: the
         * general tables' z-buffered textured RGB_555/565 blocks carry an
         * intensity slot and are refused, and their untextured ones describe
         * the same shape the MMX table's Z_RGB blocks do, which the MMX table
         * is tried first for. tt15_piz.asm/tt16_piz.asm are therefore never
         * reached.
         */
        MmxTriangle<F, D, S, X, B, H>(block, v0, v1, v2);
    } else {
        /*
         * The general SETUP_FLOAT RGB path, reached by no emitted tuple - see
         * RgbTriangle. The assertion is the guard: it fires for the one
         * combination that would otherwise fall through, naming the axes, so a
         * tuple with it is a compile error rather than a block silently drawn
         * through this kernel. Everything the arbitrary-width clause above
         * routes has to be excluded here, or the two disagree and the routed
         * tuple trips the guard instead of running: that is the colour-map
         * (SP_TEX_555/565) and shade-table shapes at SP_ADDR_DIVIDE, and the
         * exclusion mirrors the routing predicate rather than the shapes the
         * generator happens to emit today. The power-of-two RGB shade-table cells
         * are routed to PizShadeTriangle above and are excluded here too, by the
         * second clause.
         * What remains in the region - the power-of-two 555/565 colour-map shapes
         * and the MMX/perfect-scan mappers - the generator refuses, so the guard
         * is silent until SP_SPEC (or an .ifg block) is widened into them, then
         * the build stops here.
         */
        static_assert(!((F == SP_FMT_555 || F == SP_FMT_565) && D == SP_DEPTH_NONE && X != SP_TEX_NONE &&
                        !(A == SP_ADDR_DIVIDE && (X == SP_TEX_555 || X == SP_TEX_565 || S == SP_SHADE_CONST_I_RGB || S == SP_SHADE_INTERP_I_RGB)) &&
                        !(A == SP_ADDR_SHIFT && X == SP_TEX_I8 && (S == SP_SHADE_CONST_I_RGB || S == SP_SHADE_INTERP_I_RGB))),
                      "softprim: no kernel for a z-sorted RGB_555/565 texture that is not the arbitrary-width "
                      "colour-map or shade-table shape, nor the 256x256 shade-table cell");
        static_assert(!(F == SP_FMT_888 && X != SP_TEX_NONE && A != SP_ADDR_DIVIDE),
                      "softprim: no kernel for an RGB_888 texture that is not arbitrary-width - SP_TEX_I8/SP_TEX_RGB888 "
                      "with SP_ADDR_NONE/SP_ADDR_SHIFT would fall through to the general SETUP_FLOAT RGB path");

        RgbTriangle<F, D, S, X, A, P, C7>(block, v0, v1, v2);
    }
}

/*
 * Line and point rasterisers.
 *
 * pentprim draws these with plain C (drivers/pentprim/l_pi.c, l_piz.c, p_pi.c
 * and p_piz.c): a fixed-point Bresenham walk of which only the per-pixel write
 * depends on the output format and the ROPs. These templates are that walk.
 * softrend hands softprim the components as scalars (the block's convert masks
 * are all float), so each is converted with BrFloatToFixed at the point
 * pentprim's fixed convert_mask_x would have, and the fixed-point arithmetic
 * itself is kept in fixed so the pixel sequence is identical.
 *
 * The depth value is the one the fixed kernels compare and store,
 *
 *     (unsigned short)((comp_x[SZ] ^ 0x80000000) >> 16)
 *
 * kept in this form rather than reusing the triangle path's DepthFromScalar,
 * because the two differ for a scalar in (-1, 0): the fixed form truncates
 * toward zero before the shift, DepthFromScalar floors. The colour packing and
 * the texel tests are pentprim's, including the cases where a textured kernel
 * writes a transparent texel and the cases where it does not - l_pi.c's
 * LineRenderPIT has no transparent test, its LineRenderPIZ2T does, and the
 * point kernels test in p_pi.c's PointRenderPIT but not PointRenderPITI.
 */

/*
 * core/math/fixed.c's BrFixedDiv: softprim is a DDI client and does not link
 * the engine's math library, so the one operation is spelled out here.
 */
static inline br_fixed_ls LpFixedDiv(br_fixed_ls numerator, br_fixed_ls denominator) noexcept
{
    if(denominator == 0)
        return 0;

    return (br_fixed_ls)(((br_int_64)numerator << 16) / denominator);
}

/* The 16-bit depth value a line or point kernel compares and stores. */
static inline br_uint_16 LpDepth(br_fixed_ls z) noexcept
{
    br_int_32 v = (br_int_32)((br_uint_32)z ^ 0x80000000u);

    return (br_uint_16)(v >> 16);
}

static inline br_fixed_ls LpAbs(br_fixed_ls v) noexcept
{
    return v < 0 ? -v : v;
}

/* pentprim's CLAMP_LP (drivers/pentprim/work.h): clamp to the buffer, and pull
 * a coordinate that lands on the far edge back by one pixel. */
static inline void LpClamp(int& x, int& y) noexcept
{
    if(x < 0)
        x = 0;
    if(y < 0)
        y = 0;
    if(x >= SoftPrimWork.colour.width_p)
        x--;
    if(y >= SoftPrimWork.colour.height)
        y--;
}

/* ScalarsToRGB15/16 from l_pi.c and p_pi.c. */
static inline br_uint_16 LpPack555(br_fixed_ls r, br_fixed_ls g, br_fixed_ls b) noexcept
{
    return (br_uint_16)(((b >> 19) & 0x1f) | ((g >> 14) & 0x3e0) | ((r >> 9) & 0x7c00));
}

static inline br_uint_16 LpPack565(br_fixed_ls r, br_fixed_ls g, br_fixed_ls b) noexcept
{
    return (br_uint_16)(((b >> 19) & 0x1f) | ((g >> 13) & 0x7c0) | ((r >> 8) & 0xf800));
}

template <sp_fmt F>
static inline void LpStoreRgb(br_uint_8 *p, br_fixed_ls r, br_fixed_ls g, br_fixed_ls b) noexcept
{
    if constexpr(F == SP_FMT_888) {
        p[0] = (br_uint_8)(b >> 16);
        p[1] = (br_uint_8)(g >> 16);
        p[2] = (br_uint_8)(r >> 16);
    } else {
        br_uint_16 v = (F == SP_FMT_555) ? LpPack555(r, g, b) : LpPack565(r, g, b);

        memcpy(p, &v, sizeof(v));
    }
}

template <sp_fmt F, sp_depth D, sp_shade S, sp_tex X, sp_addr A>
static void SoftPrimLine(brp_block *block, brp_vertex *v0, brp_vertex *v1)
{
    (void)block;

    static_assert(F == SP_FMT_I8 || F == SP_FMT_555 || F == SP_FMT_565 || F == SP_FMT_888, "softprim: unknown line output format");
    static_assert(X == SP_TEX_NONE || (X == SP_TEX_I8 && F == SP_FMT_I8) || (X == SP_TEX_RGB888 && F == SP_FMT_888) ||
                      (X == SP_TEX_555 && F == SP_FMT_555) || (X == SP_TEX_565 && F == SP_FMT_565),
                  "softprim: a line's texture form must match its output format");
    static_assert(X != SP_TEX_NONE || A == SP_ADDR_NONE, "softprim: a texture-less line has no address mode");
    static_assert(X == SP_TEX_NONE || A == SP_ADDR_DIVIDE, "softprim: a textured line is arbitrary-width only");
    static_assert(F == SP_FMT_I8 ? (S == SP_SHADE_NONE || S == SP_SHADE_INTERP_I || S == SP_SHADE_CONST_I)
                                 : (S == SP_SHADE_NONE || S == SP_SHADE_INTERP_RGB || S == SP_SHADE_CONST_RGB),
                  "softprim: no line kernel for this shading mode");

    const softprim_buffer& col = SoftPrimWork.colour;
    const softprim_buffer& dep = SoftPrimWork.depth;
    const softprim_buffer& tex = SoftPrimWork.texture;

    const int bpp = (F == SP_FMT_I8) ? 1 : (F == SP_FMT_888) ? 3 : 2;

    br_fixed_ls fx0 = BrFloatToFixed(v0->comp[C_SX]);
    br_fixed_ls fy0 = BrFloatToFixed(v0->comp[C_SY]);
    br_fixed_ls fx1 = BrFloatToFixed(v1->comp[C_SX]);
    br_fixed_ls fy1 = BrFloatToFixed(v1->comp[C_SY]);

    int X0 = BrFixedToInt(fx0);
    int Y0 = BrFixedToInt(fy0);
    int X1 = BrFixedToInt(fx1);
    int Y1 = BrFixedToInt(fy1);

    LpClamp(X0, Y0);
    LpClamp(X1, Y1);

    br_fixed_ls dx    = (br_fixed_ls)(X1 - X0);
    br_fixed_ls dy    = (br_fixed_ls)(Y1 - Y0);
    int         error = 0;

    int         ptr_step, ptr_minor, z_step, z_minor;
    br_fixed_ls major_delta;
    br_boolean  major_x = (LpAbs(dx) > LpAbs(dy)) ? BR_TRUE : BR_FALSE;

    if(major_x) {
        if(dx < 0) {
            brp_vertex *tv = v0;
            v0             = v1;
            v1             = tv;
            int ti         = X0;
            X0             = X1;
            X1             = ti;
            ti             = Y0;
            Y0             = Y1;
            Y1             = ti;
            dx             = -dx;
            dy             = -dy;
        }

        ptr_step    = bpp + (dy > 0 ? col.stride_b : -col.stride_b);
        z_step      = 2 + (dy > 0 ? dep.stride_b : -dep.stride_b);
        ptr_minor   = bpp;
        z_minor     = 2;
        dy          = LpAbs(dy);
        major_delta = dx;
    } else {
        if(dy < 0) {
            brp_vertex *tv = v0;
            v0             = v1;
            v1             = tv;
            int ti         = X0;
            X0             = X1;
            X1             = ti;
            ti             = Y0;
            Y0             = Y1;
            Y1             = ti;
            dx             = -dx;
            dy             = -dy;
        }

        ptr_step    = col.stride_b + (dx > 0 ? bpp : -bpp);
        z_step      = dep.stride_b + (dx > 0 ? 2 : -2);
        ptr_minor   = col.stride_b;
        z_minor     = dep.stride_b;
        dx          = LpAbs(dx);
        major_delta = dy;
    }

    int         count       = major_x ? dx : dy;
    br_fixed_ls minor_delta = major_x ? dy : dx;

    const br_fixed_ls tex_fw = BrIntToFixed(tex.width_p);
    const br_fixed_ls tex_fh = BrIntToFixed(tex.height);

    br_fixed_ls pz = 0, dz = 0;
    br_fixed_ls pi = 0, di = 0;
    br_fixed_ls pu = 0, pv = 0, du = 0, dv = 0;
    br_fixed_ls pr = 0, pg = 0, pb = 0, dr = 0, dg = 0, db = 0;

    if constexpr(D == SP_DEPTH_ZW)
        pz = BrFloatToFixed(v0->comp[C_SZ]);

    if constexpr(F == SP_FMT_I8 && (S == SP_SHADE_INTERP_I || S == SP_SHADE_CONST_I))
        pi = BrFloatToFixed(v0->comp[C_I]);
    else if constexpr(F != SP_FMT_I8 && (S == SP_SHADE_INTERP_RGB || S == SP_SHADE_CONST_RGB)) {
        pr = BrFloatToFixed(v0->comp[C_R]);
        pg = BrFloatToFixed(v0->comp[C_G]);
        pb = BrFloatToFixed(v0->comp[C_B]);
    }

    if constexpr(X != SP_TEX_NONE) {
        pu = BrFloatToFixed(v0->comp[C_U]) % tex_fw;
        if(pu < 0)
            pu += tex_fw;
        pv = BrFloatToFixed(v0->comp[C_V]) % tex_fh;
        if(pv < 0)
            pv += tex_fh;
    }

    if(count > 0) {
        const br_fixed_ls den = BrIntToFixed(count);

        if constexpr(D == SP_DEPTH_ZW)
            dz = LpFixedDiv(BrFloatToFixed(v1->comp[C_SZ]) - BrFloatToFixed(v0->comp[C_SZ]), den);

        if constexpr(F == SP_FMT_I8 && (S == SP_SHADE_INTERP_I || S == SP_SHADE_CONST_I))
            di = LpFixedDiv(BrFloatToFixed(v1->comp[C_I]) - BrFloatToFixed(v0->comp[C_I]), den);
        else if constexpr(F != SP_FMT_I8 && (S == SP_SHADE_INTERP_RGB || S == SP_SHADE_CONST_RGB)) {
            dr = LpFixedDiv(BrFloatToFixed(v1->comp[C_R]) - BrFloatToFixed(v0->comp[C_R]), den);
            dg = LpFixedDiv(BrFloatToFixed(v1->comp[C_G]) - BrFloatToFixed(v0->comp[C_G]), den);
            db = LpFixedDiv(BrFloatToFixed(v1->comp[C_B]) - BrFloatToFixed(v0->comp[C_B]), den);
        }

        if constexpr(X != SP_TEX_NONE) {
            du = LpFixedDiv(BrFloatToFixed(v1->comp[C_U]) - BrFloatToFixed(v0->comp[C_U]), den);
            du = du % tex_fw;
            if(du > 0)
                du -= tex_fw;
            dv = LpFixedDiv(BrFloatToFixed(v1->comp[C_V]) - BrFloatToFixed(v0->comp[C_V]), den);
            dv = dv % tex_fh;
            if(dv > 0)
                dv -= tex_fh;
        }
    }

    br_uint_8 *ptr  = col.base + bpp * X0 + Y0 * col.stride_b;
    br_uint_8 *zptr = nullptr;

    if constexpr(D == SP_DEPTH_ZW)
        zptr = dep.base + 2 * X0 + Y0 * dep.stride_b;

    while(count-- >= 0) {
        if constexpr(D == SP_DEPTH_ZW) {
            const br_uint_16 zp = LpDepth(pz);

            if(LoadDepth(zptr) > zp) {
                if constexpr(X != SP_TEX_NONE) {
                    if constexpr(F == SP_FMT_I8) {
                        br_uint_8 texel = tex.base[(pv >> 16) * tex.stride_b + (pu >> 16)];

                        if(texel) {
                            StoreDepth(zptr, zp);
                            if constexpr(S != SP_SHADE_NONE)
                                ptr[0] = SoftPrimWork.shade.base[((pi >> 8) & 0xff00) + texel];
                            else
                                ptr[0] = texel;
                        }
                    } else if constexpr(X == SP_TEX_RGB888) {
                        const br_uint_8 *texel = tex.base + (pv >> 16) * tex.stride_b + 3 * (pu >> 16);

                        if(texel[0] || texel[1] || texel[2]) {
                            StoreDepth(zptr, zp);
                            ptr[0] = texel[0];
                            ptr[1] = texel[1];
                            ptr[2] = texel[2];
                        }
                    } else {
                        /*
                         * A 555/565-typed map: its own two-byte word is the
                         * output pixel, so it is copied rather than decoded,
                         * and a zero word is the colour key - the same test the
                         * RGB_888 arm makes over three bytes.
                         */
                        const br_uint_8 *texel = tex.base + (pv >> 16) * tex.stride_b + 2 * (pu >> 16);

                        if(texel[0] || texel[1]) {
                            StoreDepth(zptr, zp);
                            ptr[0] = texel[0];
                            ptr[1] = texel[1];
                        }
                    }
                } else {
                    StoreDepth(zptr, zp);
                    if constexpr(F == SP_FMT_I8)
                        ptr[0] = (br_uint_8)BrFixedToInt(pi);
                    else
                        LpStoreRgb<F>(ptr, pr, pg, pb);
                }
            }
        } else {
            if constexpr(X != SP_TEX_NONE) {
                if constexpr(F == SP_FMT_I8) {
                    br_uint_8 texel = tex.base[(pv >> 16) * tex.stride_b + (pu >> 16)];

                    if constexpr(S != SP_SHADE_NONE)
                        ptr[0] = SoftPrimWork.shade.base[((pi >> 8) & 0xff00) + texel];
                    else
                        ptr[0] = texel;
                } else if constexpr(X == SP_TEX_RGB888) {
                    const br_uint_8 *texel = tex.base + (pv >> 16) * tex.stride_b + 3 * (pu >> 16);

                    if(texel[0] || texel[1] || texel[2]) {
                        ptr[0] = texel[0];
                        ptr[1] = texel[1];
                        ptr[2] = texel[2];
                    }
                } else {
                    const br_uint_8 *texel = tex.base + (pv >> 16) * tex.stride_b + 2 * (pu >> 16);

                    if(texel[0] || texel[1]) {
                        ptr[0] = texel[0];
                        ptr[1] = texel[1];
                    }
                }
            } else {
                if constexpr(F == SP_FMT_I8)
                    ptr[0] = (br_uint_8)BrFixedToInt(pi);
                else
                    LpStoreRgb<F>(ptr, pr, pg, pb);
            }
        }

        error += minor_delta;
        if(error > 0) {
            error -= major_delta;
            ptr += ptr_step;
            if constexpr(D == SP_DEPTH_ZW)
                zptr += z_step;
        } else {
            ptr += ptr_minor;
            if constexpr(D == SP_DEPTH_ZW)
                zptr += z_minor;
        }

        if constexpr(D == SP_DEPTH_ZW)
            pz += dz;

        if constexpr(F == SP_FMT_I8 && (S == SP_SHADE_INTERP_I || S == SP_SHADE_CONST_I))
            pi += di;
        else if constexpr(F != SP_FMT_I8 && (S == SP_SHADE_INTERP_RGB || S == SP_SHADE_CONST_RGB)) {
            pr += dr;
            pg += dg;
            pb += db;
        }

        if constexpr(X != SP_TEX_NONE) {
            pu += du;
            if(pu < 0)
                pu += tex_fw;
            pv += dv;
            if(pv < 0)
                pv += tex_fh;
        }
    }
}

template <sp_fmt F, sp_depth D, sp_shade S, sp_tex X>
static void SoftPrimPoint(brp_block *block, brp_vertex *tvp)
{
    (void)block;

    static_assert(F == SP_FMT_I8 || F == SP_FMT_555 || F == SP_FMT_565 || F == SP_FMT_888, "softprim: unknown point output format");
    static_assert(X == SP_TEX_NONE || (X == SP_TEX_I8 && F == SP_FMT_I8) || (X == SP_TEX_RGB888 && F == SP_FMT_888) ||
                      (X == SP_TEX_555 && F == SP_FMT_555) || (X == SP_TEX_565 && F == SP_FMT_565),
                  "softprim: a point's texture form must match its output format");
    static_assert(F == SP_FMT_I8 ? (S == SP_SHADE_NONE || S == SP_SHADE_INTERP_I || S == SP_SHADE_CONST_I)
                                 : (S == SP_SHADE_NONE || S == SP_SHADE_INTERP_RGB || S == SP_SHADE_CONST_RGB),
                  "softprim: no point kernel for this shading mode");

    const softprim_buffer& col = SoftPrimWork.colour;
    const softprim_buffer& dep = SoftPrimWork.depth;
    const softprim_buffer& tex = SoftPrimWork.texture;

    const int bpp = (F == SP_FMT_I8) ? 1 : (F == SP_FMT_888) ? 3 : 2;

    int x0 = BrFixedToInt(BrFloatToFixed(tvp->comp[C_SX]));
    int y0 = BrFixedToInt(BrFloatToFixed(tvp->comp[C_SY]));

    LpClamp(x0, y0);

    br_uint_8 *zptr = nullptr;
    br_uint_16 pz   = 0;

    if constexpr(D == SP_DEPTH_ZW) {
        pz   = LpDepth(BrFloatToFixed(tvp->comp[C_SZ]));
        zptr = dep.base + y0 * dep.stride_b + 2 * x0;

        if(!(LoadDepth(zptr) > pz))
            return;
    }

    br_uint_8 *p = col.base + y0 * col.stride_b + bpp * x0;

    if constexpr(X == SP_TEX_NONE) {
        if constexpr(F == SP_FMT_I8)
            p[0] = (br_uint_8)BrFixedToInt(BrFloatToFixed(tvp->comp[C_I]));
        else
            LpStoreRgb<F>(p, BrFloatToFixed(tvp->comp[C_R]), BrFloatToFixed(tvp->comp[C_G]), BrFloatToFixed(tvp->comp[C_B]));
    } else if constexpr(F == SP_FMT_I8) {
        const br_fixed_ls fw = BrIntToFixed(tex.width_p);
        const br_fixed_ls fh = BrIntToFixed(tex.height);
        br_fixed_ls       pu = BrFloatToFixed(tvp->comp[C_U]) % fw;

        if(pu < 0)
            pu += fw;

        br_fixed_ls pv = BrFloatToFixed(tvp->comp[C_V]) % fh;

        if(pv < 0)
            pv += fh;

        /*
         * The depth and no-depth kernels sample the texel through different
         * strides (p_pi.c's PointRenderPIT uses the map width while every other
         * index kernel, including the z-buffered one, uses the row stride), and
         * only the kernels that have no shade table test the texel for
         * transparency (PointRenderPIT and both PointRenderPIZ2* variants do,
         * PointRenderPITI does not).
         */
        const int stride = (D == SP_DEPTH_ZW || S != SP_SHADE_NONE) ? tex.stride_b : tex.width_p;

        br_uint_8 texel = tex.base[(pv >> 16) * stride + (pu >> 16)];

        if((D == SP_DEPTH_ZW || S == SP_SHADE_NONE) && texel == 0)
            return;

        if constexpr(S != SP_SHADE_NONE)
            p[0] = SoftPrimWork.shade.base[256 * (br_fixed_ls)(BrFloatToFixed(tvp->comp[C_I]) >> 16) + texel];
        else
            p[0] = texel;
    } else if constexpr(X == SP_TEX_RGB888) {
        const br_fixed_ls fw = BrIntToFixed(tex.width_p);
        const br_fixed_ls fh = BrIntToFixed(tex.height);
        br_fixed_ls       pu = BrFloatToFixed(tvp->comp[C_U]) % fw;

        if(pu < 0)
            pu += fw;

        br_fixed_ls pv = BrFloatToFixed(tvp->comp[C_V]) % fh;

        if(pv < 0)
            pv += fh;

        const br_uint_8 *texel = tex.base + (pv >> 16) * tex.stride_b + 3 * (pu >> 16);

        if(!(texel[0] || texel[1] || texel[2]))
            return;

        p[0] = texel[0];
        p[1] = texel[1];
        p[2] = texel[2];
    } else {
        const br_fixed_ls fw = BrIntToFixed(tex.width_p);
        const br_fixed_ls fh = BrIntToFixed(tex.height);
        br_fixed_ls       pu = BrFloatToFixed(tvp->comp[C_U]) % fw;

        if(pu < 0)
            pu += fw;

        br_fixed_ls pv = BrFloatToFixed(tvp->comp[C_V]) % fh;

        if(pv < 0)
            pv += fh;

        const br_uint_8 *texel = tex.base + (pv >> 16) * tex.stride_b + 2 * (pu >> 16);

        if(!(texel[0] || texel[1]))
            return;

        p[0] = texel[0];
        p[1] = texel[1];
    }

    if constexpr(D == SP_DEPTH_ZW)
        StoreDepth(zptr, pz);
}

#define SOFTPRIM_BLOCK_TRI(name, colour7bit_f, ...)                                                     \
    extern "C" void BR_ASM_CALL name(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2) \
    {                                                                                                  \
        SoftPrimRender<__VA_ARGS__, (colour7bit_f) != 0>(block, v0, v1, v2);                            \
    }

#define SOFTPRIM_BLOCK_LINE(name, colour7bit_f, F, T, D, S, X, A, P, B, G, H)           \
    extern "C" void BR_ASM_CALL name(brp_block *block, brp_vertex *v0, brp_vertex *v1) \
    {                                                                                  \
        SoftPrimLine<F, D, S, X, A>(block, v0, v1);                                     \
    }

#define SOFTPRIM_BLOCK_POINT(name, colour7bit_f, F, T, D, S, X, A, P, B, G, H) \
    extern "C" void BR_ASM_CALL name(brp_block *block, brp_vertex *tvp)       \
    {                                                                         \
        SoftPrimPoint<F, D, S, X>(block, tvp);                                 \
    }

#include "softprim_blocks.h"
