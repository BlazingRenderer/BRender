/*
 * softprim primitive matching and range reporting
 *
 * The block set is generated. Each entry's axis tuple comes from the same .ifg
 * descriptions that describe pentprim's rasterisers (see softprim_blocks.h and
 * drivers/softprim/infogen.pl), and the brp_block that softrend consumes is
 * derived from that tuple at first use - so a block cannot ask for something
 * its kernel was not instantiated for.
 *
 * Matching reproduces pentprim's match_block(): the state is reduced to the
 * flags pentprim derives from it and the types of the bound buffers, and the
 * generated blocks are walked in table order, taking the first whose
 * requirements are met. A block the state does not satisfy falls through to the
 * next - that is how a textured primitive whose map is not a power of two
 * reaches the untextured kernel: it fails the textured blocks' flag predicate
 * rather than being mismatched. A block the state does satisfy but that has no
 * kernel refuses the primitive, and when no block's requirements are met it is
 * refused too. Both are the explicit "not implemented" behaviour rather than
 * pentprim's fallback to the last table entry, which would draw something no
 * kernel was written for.
 */
#include "drv.h"
#include "shortcut.h"
#include "brassert.h"

#include "softprim_axes.h"

/*
 * The buffers the rasteriser is currently pointed at. Refilled at every
 * render begin from the primitive state's output buffers.
 */
softprim_work SoftPrimWork;

void SoftPrimWorkUpdate(struct br_primitive_state *self)
{
    SoftPrimSetupBuffer(&SoftPrimWork.colour, self->out.colour.pixelmap);
    SoftPrimSetupBuffer(&SoftPrimWork.depth, self->out.depth.pixelmap);

    /*
     * The texture and shade table are stored buffers; the cached softprim_buffer
     * for each is already what the rasteriser needs, so copy it rather than
     * re-deriving it from an object handle.
     */
    if(self->prim.colour_map.buffer != NULL) {
        SoftPrimWork.texture = self->prim.colour_map.buffer->buffer;

        /*
         * An INDEX_8 texture with no palette of its own inherits the screen
         * palette, exactly as pentprim's match.c does, so an indexed map can
         * be decoded into an RGB output.
         */
        if(SoftPrimWork.texture.palette == NULL) {
            SoftPrimWork.texture.palette          = SoftPrimWork.colour.palette;
            SoftPrimWork.texture.palette_size     = SoftPrimWork.colour.palette_size;
            SoftPrimWork.texture.palette_type     = SoftPrimWork.colour.palette_type;
            SoftPrimWork.texture.palette_entry_b  = SoftPrimWork.colour.palette_entry_b;
            SoftPrimWork.texture.palette_stride_b = SoftPrimWork.colour.palette_stride_b;
            SoftPrimWork.texture.palette_stride_p = SoftPrimWork.colour.palette_stride_p;
        }
    } else {
        BrMemSet(&SoftPrimWork.texture, 0, sizeof(SoftPrimWork.texture));
    }

    if(self->prim.index_shade.buffer != NULL) {
        SoftPrimWork.shade = self->prim.index_shade.buffer->buffer;

        /*
         * pentprim's work area takes index_range from the primitive only while
         * a shade table is bound, and keeps the last value otherwise; the
         * RANGE_ZERO match flag is tested from it. Mirror that rather than read
         * the primitive's own range, which is set regardless.
         */
        SoftPrimWork.index_range = self->prim.index_range;
        SoftPrimWork.index_base  = self->prim.index_base;
    } else {
        BrMemSet(&SoftPrimWork.shade, 0, sizeof(SoftPrimWork.shade));
    }

    if(self->prim.index_fog.buffer != NULL) {
        SoftPrimWork.fog = self->prim.index_fog.buffer->buffer;
    } else {
        BrMemSet(&SoftPrimWork.fog, 0, sizeof(SoftPrimWork.fog));
    }

    if(self->prim.index_blend.buffer != NULL) {
        SoftPrimWork.blend = self->prim.index_blend.buffer->buffer;
    } else {
        BrMemSet(&SoftPrimWork.blend, 0, sizeof(SoftPrimWork.blend));
    }
}

/*
 * One generated block. The axes name the kernel - the identifier softrend sees
 * is that symbol name, which is the tuple spelled out, so a frame's provenance
 * can be read off the name. The rest is what pentprim's match_block() tests:
 * the flag predicate and the per-map type and dimension requirements.
 * need_subdivide is the .ifg's RF_NEED_SUBDIVIDE, which the tuple cannot
 * express; renderBegin turns it into the block's BR_PRIMF_SUBDIVIDE flag.
 * unscaled is RF_UNSCALED_TEXTURE_COORDS, and colour7bit is the .ifg property
 * of the same name; both change updateRanges and neither is in the tuple.
 *
 * The list is not deduplicated: two blocks can name the same kernel while
 * disagreeing on their requirements (the MMX and general tables describe the
 * same tuples with different predicates), and the list order is what decides
 * which of them the walk reaches first.
 */
struct sp_match {
    br_uint_32 flags_mask;
    br_uint_32 flags_cmp;
    br_uint_32 need_subdivide;
    br_uint_32 unscaled;
    br_uint_32 offset_y;
    br_uint_32 colour7bit;
    br_scalar  colour_scale[3];
    br_uint_32 map_size;
    br_uint_8  depth_type;
    br_uint_8  texture_type;
    br_uint_8  shade_type;
    br_uint_8  blend_type;
    br_uint_8  fog_type;
    sp_axes    axes;
    brp_block  block;
    br_boolean refused;
    br_boolean filled;
};

/*
 * Declare the kernels first. The generated list is expanded twice - here for
 * the declarations and below for the table - so a table entry can only ever
 * reference a kernel the generator emitted, and an emitted kernel that is not
 * defined in raster.cpp is a link error. The list carries one line per block,
 * so a repeated declaration is expected and harmless.
 *
 * The declaration's arity is the topology's: softrend calls a triangle with
 * three vertices, a line with two and a point with one (drivers/softrend
 * faceops.c calls brp_render2 for a line and brp_render1 for a point), so the
 * generated macro name carries the topology and each name is declared with the
 * signature its kernel is called through. Declaring a line kernel with three
 * arguments would type the `.render` cast against a function that is never
 * called that way.
 *
 * SOFTPRIM_REFUSED is the other half of the list: a block whose shape is
 * outside the generator's implemented axis spec, which has no kernel. It takes
 * no declaration - there is no function - but it stays in the table below,
 * because it has to take part in the walk (see spFindMatch).
 */
#define SOFTPRIM_BLOCK_TRI(name, ...)   extern void BR_ASM_CALL name(brp_block *, brp_vertex *, brp_vertex *, brp_vertex *);
#define SOFTPRIM_BLOCK_LINE(name, ...)  extern void BR_ASM_CALL name(brp_block *, brp_vertex *, brp_vertex *);
#define SOFTPRIM_BLOCK_POINT(name, ...) extern void BR_ASM_CALL name(brp_block *, brp_vertex *);
#define SOFTPRIM_REFUSED(name, ...)

#include "softprim_matchers.inc"

#undef SOFTPRIM_BLOCK_TRI
#undef SOFTPRIM_BLOCK_LINE
#undef SOFTPRIM_BLOCK_POINT
#undef SOFTPRIM_REFUSED

/*
 * One table entry. `kernel` is the kernel to call, or NULL for a refused block,
 * and `refuse` says which; `ident` is the tuple spelled out, which is the
 * provenance string softrend sees. The parameters are named so that none of
 * them is the identifier a struct designator is made of - a parameter called
 * `render` would replace the `render` in `.render`.
 */
#define SOFTPRIM_ENTRY(kernel, ident, refuse, fmask, fcmp, subdiv, unscaled_f, offset_y_f, colour7bit_f, cs_r, cs_g, cs_b, msize, dtype, \
                      ttype, stype, btype, ftype, ...)                                                                                  \
    {                                                                                                                                   \
        .flags_mask     = (fmask),                                                                                                      \
        .flags_cmp      = (fcmp),                                                                                                       \
        .need_subdivide = (subdiv),                                                                                                     \
        .unscaled       = (unscaled_f),                                                                                                 \
        .offset_y       = (offset_y_f),                                                                                                 \
        .colour7bit     = (colour7bit_f),                                                                                               \
        .colour_scale   = {cs_r, cs_g, cs_b},                                                                                           \
        .map_size       = (msize),                                                                                                      \
        .depth_type     = (dtype),                                                                                                      \
        .texture_type   = (ttype),                                                                                                      \
        .shade_type     = (stype),                                                                                                      \
        .blend_type     = (btype),                                                                                                      \
        .fog_type       = (ftype),                                                                                                      \
        .axes           = {__VA_ARGS__},                                                                                                \
        .refused        = (refuse),                                                                                                     \
        .block          = {.render              = (brp_render_fn *)(kernel),                                                            \
                           .chain               = NULL,                                                                                 \
                           .identifier          = (ident),                                                                              \
                           ._reserved0          = NULL,                                                                                 \
                           .type                = 0,                                                                                    \
                           .flags               = 0,                                                                                    \
                           .constant_components = 0,                                                                                    \
                           .vertex_components   = 0,                                                                                    \
                           .convert_mask_f      = 0,                                                                                    \
                           .convert_mask_x      = 0,                                                                                    \
                           .convert_mask_i      = 0,                                                                                    \
                           .constant_mask       = 0,                                                                                    \
                           .subdivide_tolerance = 0,                                                                                    \
                           ._reserved_0         = 0,                                                                                    \
                           ._reserved_1         = 0,                                                                                    \
                           ._reserved_2         = 0},                                                                                           \
        .filled         = BR_FALSE,                                                                                                     \
},

#define SOFTPRIM_BLOCK_TRI(name, ...)   SOFTPRIM_ENTRY(name, #name, BR_FALSE, __VA_ARGS__)
#define SOFTPRIM_BLOCK_LINE(name, ...)  SOFTPRIM_ENTRY(name, #name, BR_FALSE, __VA_ARGS__)
#define SOFTPRIM_BLOCK_POINT(name, ...) SOFTPRIM_ENTRY(name, #name, BR_FALSE, __VA_ARGS__)
#define SOFTPRIM_REFUSED(name, ...)     SOFTPRIM_ENTRY(NULL, "refused: " #name, BR_TRUE, __VA_ARGS__)

/*
 * The ordered block list the walk searches, in the order the tables are tried:
 * pentprim's table order, with the refused blocks left where pentprim put them.
 */
static struct sp_match refPrimBlocks[] = {
#include "softprim_matchers.inc"
};

#undef SOFTPRIM_BLOCK_TRI
#undef SOFTPRIM_BLOCK_LINE
#undef SOFTPRIM_BLOCK_POINT
#undef SOFTPRIM_REFUSED
#undef SOFTPRIM_ENTRY

/*
 * Derive the brp_block softrend consumes from the axis tuple.
 *
 * Everything downstream of this reads the derived fields, and they are derived
 * from the tuple rather than written out beside it, which is what stops a block
 * claiming a depth type or a component its kernel does not implement.
 */
static void spFillBlock(struct sp_match *e)
{
    br_uint_32 vertex_components   = CM_SX | CM_SY;
    br_uint_32 constant_components = 0;
    br_uint_32 constant_mask       = 0;

    if(e->filled)
        return;

    switch(e->axes.top) {
        case SP_TOP_TRI:
            e->block.type = BRT_TRIANGLE;
            break;
        case SP_TOP_LINE:
            e->block.type = BRT_LINE;
            break;
        case SP_TOP_POINT:
            e->block.type = BRT_POINT;
            break;
    }

    if(e->axes.depth == SP_DEPTH_ZW)
        vertex_components |= CM_SZ;

    /*
     * A texture needs U and V; the projective path additionally needs the
     * homogeneous W. Without these in the component mask softrend generates no
     * surface function to fill them in and the kernel would read stale slots.
     */
    if(e->axes.tex != SP_TEX_NONE)
        vertex_components |= CM_U | CM_V;

    if(e->axes.persp == SP_PERSP_CORRECT)
        vertex_components |= CM_W;

    switch(e->axes.shade) {
        case SP_SHADE_CONST_I:
        case SP_SHADE_CONST_I_TABLE:
        case SP_SHADE_CONST_I_RGB:
            constant_components = CM_I;
            constant_mask       = (1u << C_I);
            break;
        case SP_SHADE_INTERP_I:
        case SP_SHADE_INTERP_I_TABLE:
        case SP_SHADE_INTERP_I_RGB:
            vertex_components |= CM_I;
            break;
        case SP_SHADE_CONST_RGB:
            constant_components = CM_R | CM_G | CM_B;
            constant_mask       = (1u << C_R) | (1u << C_G) | (1u << C_B);
            break;
        case SP_SHADE_INTERP_RGB:
            vertex_components |= CM_R | CM_G | CM_B;
            break;
        default:
            break;
    }

    /*
     * The 32 screendoor blocks declare constant_components = CM_A
     * (infogen.pl's property_constant_alpha, which no screendoor block omits
     * and no non-screendoor block uses), and pentprim's _S/_SD kernels take
     * the screendoor level from the alpha byte of the constant colour word
     * (rastrise.asm's UNPACK_SCREENDOOR_ALPHA). Without A in the mask the
     * level is read from a slot softrend never filled. SP_SPEC does not name
     * SP_BLEND_SCREENDOOR, so these blocks are refused and nothing runs - but
     * the derivation carries the declaration, so adding the kernel is the
     * whole of the change that would make them draw.
     */
    if(e->axes.blend == SP_BLEND_SCREENDOOR) {
        constant_components |= CM_A;
        constant_mask |= (1u << C_A);
    }

    e->block.constant_components = constant_components;
    e->block.vertex_components   = vertex_components;

    /*
     * constant_mask is a mask of slots, one bit per brp_vertex member, and it
     * is a different mask from constant_components: the latter is the CM_*
     * component mask, in which C_I and C_A share a slot. softrend's constant
     * replication walks constant_mask (faceops.c OpTriangleReplicateConstant,
     * :253-270), and the line and point paths add that replication
     * unconditionally whenever the block has constant functions
     * (v1model.c:620, :634). The triangle path only adds it for
     * BR_PRIMF_CONST_DUPLICATE, which softprim never sets, so this mask is
     * unused for triangles and cannot move them.
     */
    e->block.constant_mask = constant_mask;

    /*
     * pentprim's line table sets BR_PRIMF_CONST_DUPLICATE on every constant
     * line (the `duplicate` property in prim_l*.ifg) and on no point - and the
     * flag is what makes softrend not share the non-zero vertices of a line in
     * the primitive heap (v1model.c:522), which is the behaviour pentprim's
     * frames were produced with. Refprim draws the constant into every vertex
     * through softrend's replication either way, so only the heap sharing
     * differs; mirror pentprim so the two agree.
     */
    if(e->axes.top == SP_TOP_LINE && constant_components != 0)
        e->block.flags |= BR_PRIMF_CONST_DUPLICATE;

    /*
     * pentprim marks a blend-table block BR_PRIMF_BLENDED (infogen.pl's
     * property_blend_table, :1048), which softrend uses to decide whether a
     * primitive must be deferred until the opaque geometry behind it has been
     * drawn (v1model.c:791, :37). Without it a blend is drawn in tree order and
     * reads whatever happens to be in the buffer, which is a different frame
     * whenever two blended models overlap. Decal is not a blend: it neither
     * reads the destination nor carries the flag.
     */
    if(e->axes.blend == SP_BLEND_INDEX || e->axes.blend == SP_BLEND_ALPHA || e->axes.blend == SP_BLEND_SCREENDOOR)
        e->block.flags |= BR_PRIMF_BLENDED;

    /*
     * Stage 1 asks softrend for everything as float, which is what the
     * rasteriser reads. The fixed and integer masks stay empty.
     */
    e->block.convert_mask_f = constant_components | vertex_components;
    e->block.convert_mask_x = 0;
    e->block.convert_mask_i = 0;

    e->filled = BR_TRUE;
}

/*
 * The state a block is matched against: pentprim's `flags` word, the types of
 * the bound buffers, and the bound map's dimensions. The output format and the
 * topology are how pentprim picks the array to search before calling
 * match_block, so they are carried here too and compared against the block's
 * tuple.
 *
 * An unbound buffer is SP_PMT_NONE, a type no block requires, which is how
 * "no requirement" stays distinct from BR_PMT_INDEX_1, the zero-valued type.
 */
struct sp_want {
    sp_fmt     fmt;
    sp_top     top;
    br_uint_32 flags;
    br_uint_32 map_width;
    br_uint_32 map_height;
    br_uint_8  depth_type;
    br_uint_8  texture_type;
    br_uint_8  shade_type;
    br_uint_8  blend_type;
    br_uint_8  fog_type;
};

/*
 * pentprim's isPowerof2(), which is true for zero as well - it is a plain
 * !((x-1) & x), not an (x & (x-1)) == 0 test.
 */
static br_boolean spIsPowerOf2(br_int_32 x)
{
    return !((x - 1) & x);
}

/*
 * The palette entry count a map of this type needs before pentprim will set
 * PRIMF_PALETTE; anything else can never satisfy a palette requirement.
 */
static br_int_32 spPaletteEntries(br_uint_8 type)
{
    switch(type) {
        case BR_PMT_INDEX_1:
            return 2;
        case BR_PMT_INDEX_2:
            return 4;
        case BR_PMT_INDEX_4:
            return 16;
        case BR_PMT_INDEX_8:
            return 256;
        default:
            return 0;
    }
}

/*
 * The flags pentprim's renderBegin derives from the bound state on top of the
 * primitive's own flags: the map's shape (pentprim/match.c:576-597) and whether
 * the caller asked for perspective correction (match.c:619-627).
 *
 * A block's flags_mask/flags_cmp are predicates on this word - "requires
 * POWER2", "forbids SMOOTH" - and the walk tests them. This is what makes a
 * textured primitive with a non-power-of-two map fail the textured blocks'
 * predicates and reach the untextured one.
 */
static br_uint_32 spDeriveFlags(struct br_primitive_state *self)
{
    const softprim_buffer *t     = &SoftPrimWork.texture;
    br_uint_32            flags = self->prim.flags;

    /*
     * RANGE_ZERO is pentprim's "index_range == 0" flag, tested from the work
     * area it fills only while a shade table is bound (match.c:212-215, :577).
     * SoftPrimWorkUpdate mirrors that, so an unshaded primitive sees the
     * zero-initialised value, exactly as pentprim does.
     */
    if(SoftPrimWork.index_range == 0)
        flags |= PRIMF_RANGE_ZERO;

    if(t->type != SP_PMT_NONE) {
        if(t->width_p * t->bpp == t->stride_b)
            flags |= PRIMF_NO_SKIP;

        if(t->stride_b > 0)
            flags |= PRIMF_STRIDE_POSITIVE;

        if(spIsPowerOf2(t->width_p) && spIsPowerOf2(t->height) && t->width_p <= 1024 && t->height <= 1024)
            flags |= PRIMF_POWER2;

        /*
         * pentprim only counts a palette it can use: an RGBX_888 one of at
         * least the entry count the map's index width implies.
         */
        if(t->palette != NULL && t->palette_type == BR_PMT_RGBX_888 && t->palette_size >= spPaletteEntries(t->type))
            flags |= PRIMF_PALETTE;
    }

    if(self->prim.perspective_type != BRT_NONE && self->prim.perspective_type != BRT_SUBDIVIDE)
        flags |= PRIMF_PERSPECTIVE;

    return flags;
}

/*
 * Reduce the bound state to what the walk tests.
 *
 * Returns BR_FALSE for a state that cannot be matched at all (unknown output
 * format or shape, or a depth buffer that is neither 16-bit nor absent), which
 * is refused outright rather than approximated.
 */
static br_boolean spWantState(struct br_primitive_state *self, br_token prim_type, struct sp_want *w)
{
    switch(self->out.colour.type) {
        case BR_PMT_INDEX_8:
            w->fmt = SP_FMT_I8;
            break;
        case BR_PMT_RGB_555:
            w->fmt = SP_FMT_555;
            break;
        case BR_PMT_RGB_565:
            w->fmt = SP_FMT_565;
            break;
        case BR_PMT_RGB_888:
            w->fmt = SP_FMT_888;
            break;
        default:
            return BR_FALSE;
    }

    switch(prim_type) {
        case BRT_TRIANGLE:
            w->top = SP_TOP_TRI;
            break;
        case BRT_LINE:
            w->top = SP_TOP_LINE;
            break;
        case BRT_POINT:
            w->top = SP_TOP_POINT;
            break;
        default:
            return BR_FALSE;
    }

    if(self->out.depth.pixelmap == NULL)
        w->depth_type = SP_PMT_NONE;
    else if(self->out.depth.type == BR_PMT_DEPTH_16)
        w->depth_type = BR_PMT_DEPTH_16;
    else
        return BR_FALSE;

    /*
     * The buffer types pentprim's updateWorkPrim fills, with SP_PMT_NONE for
     * "not bound" so a block requiring a type can never match an absent buffer.
     */
    w->texture_type = (self->prim.colour_map.buffer != NULL) ? self->prim.colour_map.buffer->buffer.type : SP_PMT_NONE;
    w->shade_type   = (self->prim.index_shade.buffer != NULL) ? self->prim.index_shade.buffer->buffer.type : SP_PMT_NONE;
    w->blend_type   = (self->prim.index_blend.buffer != NULL) ? self->prim.index_blend.buffer->buffer.type : SP_PMT_NONE;
    w->fog_type     = (self->prim.index_fog.buffer != NULL) ? self->prim.index_fog.buffer->buffer.type : SP_PMT_NONE;

    w->map_width  = (br_uint_32)SoftPrimWork.texture.width_p;
    w->map_height = (br_uint_32)SoftPrimWork.texture.height;

    w->flags = spDeriveFlags(self);

    return BR_TRUE;
}

/*
 * Walk the generated blocks in table order and take the first the state
 * satisfies, as pentprim's match_block() walks its table: the flag predicate
 * first, then the per-map type requirements and the required dimensions. A
 * block whose requirements are not met does not stop the search - that is the
 * fall-through - and the tuple only names the kernel once a block has won.
 *
 * A block the state satisfies but that has no kernel stops the search and
 * refuses the primitive. Those are the blocks the generator put outside its
 * implemented axis spec, and they matter because they are where pentprim's
 * tables route a shape softprim cannot draw: a variant is usually spelled as a
 * match flag on a block that sits before its twin, so dropping the variant
 * outright would not refuse the shape - the walk would reach the twin and draw
 * it as though the variant had been applied. That is a silent wrong draw, so
 * the refusal happens here instead.
 *
 * The format and topology tests reproduce pentprim picking the array by output
 * format and primitive type before searching it: the generated list carries
 * every format's tables, so a block from another format must not be considered.
 */
static struct sp_match *spFindMatch(const struct sp_want *w)
{
    int i;

    for(i = 0; i < BR_ASIZE(refPrimBlocks); i++) {
        struct sp_match *e = &refPrimBlocks[i];

        if(e->axes.fmt != w->fmt || e->axes.top != w->top)
            continue;

        if((w->flags & e->flags_mask) != e->flags_cmp)
            continue;

        if(e->depth_type != SP_PMT_NONE && e->depth_type != w->depth_type)
            continue;

        if(e->texture_type != SP_PMT_NONE && e->texture_type != w->texture_type)
            continue;

        if(e->shade_type != SP_PMT_NONE && e->shade_type != w->shade_type)
            continue;

        if(e->blend_type != SP_PMT_NONE && e->blend_type != w->blend_type)
            continue;

        if(e->fog_type != SP_PMT_NONE && e->fog_type != w->fog_type)
            continue;

        if(e->map_size != 0 && (e->map_size != w->map_width || e->map_size != w->map_height))
            continue;

        if(e->refused)
            return NULL;

        return e;
    }

    return NULL;
}

/*
 * Component ranges for the current output buffer, as consumed by
 * PROJECT_VERTEX and CLAMP_SCALE in softrend.
 */
static void updateRanges(struct br_primitive_state *self, const sp_axes *axes, br_uint_32 m, const br_scalar scale[3], br_boolean unscaled,
                         br_boolean offset_y)
{
    int       i;
    br_scalar width  = BrIntToScalar(self->out.colour.width);
    br_scalar height = BrIntToScalar(self->out.colour.height);

    /*
     * Everything defaults to offset = 0, scale = 1
     */
    for(i = 0; i < NUM_COMPONENTS; i++) {
        self->cache.comp_offsets[i] = BR_SCALAR(0.0);
        self->cache.comp_scales[i]  = BR_SCALAR(1.0);
    }

    /*
     * Screen x/y map normalised device coordinates onto pixel centres: pixel i
     * has centre i+0.5, with x increasing to the right and y downwards.
     *
     * RF_OFFSET_Y (match.h:170) offsets x the other way by half a pixel: its
     * comment says it is for the MMX setup, and the setup's screen x is the
     * rounding of a value that has already been moved half a pixel, so the
     * whole triangle lands one pixel left of where the other blocks put it.
     * Y is deliberately left alone.
     */
    const br_scalar xoff = offset_y ? -BR_SCALAR(0.5) : BR_SCALAR(0.5);

    self->cache.comp_offsets[C_SX] = BR_CONST_DIV(width, 2) + xoff;
    self->cache.comp_scales[C_SX]  = BR_CONST_DIV(width, 2);

    self->cache.comp_offsets[C_SY] = BR_CONST_DIV(height, 2) + BR_SCALAR(0.5);
    self->cache.comp_scales[C_SY]  = -BR_CONST_DIV(height, 2);

    /*
     * Depth is scaled to the signed 16-bit range that the rasteriser folds
     * into an unsigned 16-bit z value (see raster.cpp).
     */
    self->cache.comp_offsets[C_SZ] = BR_SCALAR(0);
    self->cache.comp_scales[C_SZ]  = -BR_SCALAR(32767);

    /*
     * Indexed colour: intensity 0..1 across the current ramp. A textured shaded
     * block draws its ramp from the shade table height, exactly as pentprim's
     * updateRanges does for shade_type != PMT_NONE; every other indexed block
     * (including an untextured intensity block) uses the primitive's own index
     * range.
     *
     * The block's component mask decides whether it is set at all, as
     * pentprim's updateRanges guards each parameter group with the mask
     * (match.c:320-360). That guard is load-bearing rather than tidy: C_I and
     * C_A are the same component slot (priminfo.h:44-45) while being different
     * mask bits, so a screendoor block - which carries CM_A and no CM_I -
     * would otherwise have its alpha range overwritten with the index range and
     * its screendoor level come out as an intensity. pentprim's order is the
     * alpha group first, the intensity group second.
     */
    if(m & CM_I) {
        if(axes->blend == SP_BLEND_DECAL) {
            /*
             * RF_DECAL rescales intensity 0..1 onto 0.5..254.5 rather than across
             * the shade table, because the decal kernel is what turns it into an
             * index (pentprim/match.c:354-357).
             */
            self->cache.comp_scales[C_I]  = BR_SCALAR(254);
            self->cache.comp_offsets[C_I] = BR_SCALAR(0.5);
        } else if(axes->shade == SP_SHADE_CONST_I_TABLE || axes->shade == SP_SHADE_INTERP_I_TABLE ||
                  (axes->shade != SP_SHADE_NONE && axes->tex != SP_TEX_NONE)) {
            self->cache.comp_scales[C_I]  = BrIntToScalar(self->prim.index_shade.height - 1);
            self->cache.comp_offsets[C_I] = BR_SCALAR(0.5);
        } else {
            self->cache.comp_scales[C_I]  = BrIntToScalar(self->prim.index_range);
            self->cache.comp_offsets[C_I] = BrIntToScalar(self->prim.index_base) + BR_SCALAR(0.5);
        }
    }

    /*
     * An RGB output carries R,G,B with the block's colour base and scale, which
     * pentprim's updateRanges takes from the block's colour_scales: offset 1,
     * scale 254 for the undithered colour blocks and 126 for the colour7bit
     * ones (the MMX interpolated-colour family). softrend applies these to the
     * vertex colour before the kernel reads it, so the integer part of comp[R]
     * is the 8-bit channel src*255/255.
     */
    if(axes->fmt != SP_FMT_I8 && axes->shade != SP_SHADE_NONE) {
        self->cache.comp_offsets[C_R] = self->cache.comp_offsets[C_G] = self->cache.comp_offsets[C_B] = BR_SCALAR(1);
        self->cache.comp_scales[C_R]                                                                  = scale[0];
        self->cache.comp_scales[C_G]                                                                  = scale[1];
        self->cache.comp_scales[C_B]                                                                  = scale[2];
    }

    /*
     * Texture coordinates are in texel units: softrend folds the map size into
     * the map transform, so the rasteriser receives a texel coordinate rather
     * than a normalised one. The arbitrary-width family is declared
     * unscaled_texture_coords, so its blocks take the coordinates as given
     * (pentprim's RF_UNSCALED_TEXTURE_COORDS).
     */
    if(self->prim.colour_map.buffer != NULL) {
        if(unscaled) {
            self->cache.comp_scales[C_U] = BR_SCALAR(1);
            self->cache.comp_scales[C_V] = BR_SCALAR(1);
        } else {
            self->cache.comp_scales[C_U] = BrIntToScalar(self->prim.colour_map.width);
            self->cache.comp_scales[C_V] = BrIntToScalar(self->prim.colour_map.height);
        }
    }
}

br_error BR_CMETHOD_DECL(br_primitive_state_softprim, renderBegin)(struct br_primitive_state *self, struct brp_block **rpb, br_boolean *block_changed,
                                                                  br_boolean *ranges_changed, br_boolean no_render, br_token prim_type)
{
    struct sp_want   want;
    struct sp_match *found = NULL;

    ASSERT(rpb);
    ASSERT(self);
    ASSERT(self->plib);

    /*
     * Lock the destination pixelmap for the model, if not already locked. It is
     * unlocked again by the primitive library's flush.
     */
    if(!no_render && self->plib->colour_buffer != self->out.colour.pixelmap) {
        if(self->plib->colour_buffer != NULL)
            DevicePixelmapDirectUnlock(self->plib->colour_buffer);

        self->plib->colour_buffer = self->out.colour.pixelmap;

        if(self->plib->colour_buffer != NULL)
            DevicePixelmapDirectLock(self->plib->colour_buffer, BR_TRUE);
    }

    /*
     * The work area describes the bound buffers, and the match flags are
     * derived from it, so it is filled before the state is reduced to a match.
     */
    SoftPrimWorkUpdate(self);

    if(self->prim.custom_block != NULL || self->out.colour.pixelmap == NULL || !spWantState(self, prim_type, &want)) {
        self->cache.last_block = NULL;
        self->cache.last_type  = BR_NULL_TOKEN;

        return BRE_FAIL;
    }

    found = spFindMatch(&want);

    if(found == NULL) {
        self->cache.last_block = NULL;
        self->cache.last_type  = BR_NULL_TOKEN;

        return BRE_FAIL;
    }

    spFillBlock(found);

    /*
     * pentprim marks the matched block subdivide-able when the primitive asks
     * for subdivision or the block's own range flags require it, and clears the
     * flag otherwise. softrend turns that bit into an OpTriangleSubdivide pass
     * (v1model.c:661), which is what keeps an arbitrary-width perspective
     * primitive close to perspective correct; without it the whole quad reaches
     * the affine mapper in one piece and samples differently.
     */
    if(self->prim.perspective_type == BRT_SUBDIVIDE || found->need_subdivide) {
        found->block.flags |= BR_PRIMF_SUBDIVIDE;
        found->block.subdivide_tolerance = self->prim.subdivide_tolerance;
    } else {
        found->block.flags &= ~BR_PRIMF_SUBDIVIDE;
    }

    /*
     * The block and its ranges always describe the current output buffers,
     * which may have been resized or swapped since the last call.
     */
    *block_changed  = BR_TRUE;
    *ranges_changed = BR_TRUE;

    updateRanges(self, &found->axes, found->block.constant_components | found->block.vertex_components, found->colour_scale,
                 found->unscaled != 0, found->offset_y != 0);

    *rpb = &found->block;

    self->cache.last_block     = &found->block;
    self->cache.last_type      = prim_type;
    self->cache.timestamp_prim = self->prim.timestamp_major;
    self->cache.timestamp_out  = self->out.timestamp_major;

    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_primitive_state_softprim, renderEnd)(struct br_primitive_state *self, struct brp_block *pb)
{
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_primitive_state_softprim, rangesQuery)(struct br_primitive_state *self, br_scalar *offset, br_scalar *scale,
                                                                  br_int_32 max_comp)
{
    int i;

    /*
     * Fail if the current info is not valid
     */
    if(self->cache.timestamp_prim != self->prim.timestamp_major || self->cache.timestamp_out != self->out.timestamp_major)
        return BRE_FAIL;

    for(i = 0; i < max_comp; i++) {
        offset[i] = self->cache.comp_offsets[i];
        scale[i]  = self->cache.comp_scales[i];
    }

    return BRE_OK;
}
