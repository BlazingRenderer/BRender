/*
 * The 2D witness.
 *
 * Br2D*() reaches the renderer through br_geometry_v1_model::render() - the
 * immediate path - which glrend and vkrend implement by building a stored
 * geometry from the caller's v11model and running the stored draw. Nothing in
 * the corpus takes that path: the fixtures all go through the scene renderers,
 * and Br2D's only other caller is examples/imgui-demo, which CI does not run.
 * A driver that stubs render() out therefore draws nothing and no test says so.
 *
 * This drives it and asserts the result. The checks are relations between
 * frames the witness computes itself, so there is no reference to bless:
 *
 *   blank   - the offscreen is the clear colour before anything is drawn
 *   draw    - a quad lands where it was asked for, in the colour it was given
 *   clip    - the software clipper keeps the inside of the rect and only that
 *   order   - a later opaque quad covers an earlier one, and the reverse
 *
 * "draw" is the check the stub fails: under a stubbed render() the offscreen
 * stays blank and the first sample already disagrees.
 */
#include <brender.h>
#include <brddi.h>

#include "rt_demo.h"
#include "b2dwitness.h"

#define B2DW_SIZE  64
#define B2DW_LEFT  8
#define B2DW_RIGHT 56

/*
 * The renderer's primitive heap. BrRendererBegin() wants one, and the witness
 * runs before the demo's own rendering has started.
 */
static char b2dw_heap[256 * 1024];

typedef struct b2dw_state {
    br_pixelmap *offscreen;
    br_pixelmap *white;
    br_pixelmap *readback;
    br_material *material;
} b2dw_state;

static const br_colour b2dw_red  = BR_COLOUR_RGB(255, 0, 0);
static const br_colour b2dw_blue = BR_COLOUR_RGB(0, 0, 255);

/*
 * Read a pixel through the device->host copy, not BrPixelmapPixelGet().
 * BrPixelmapPixelGet() is the addressable-pixelmap path, and glrend leaves
 * pixelQuery as the fail stub - it reads back as black whatever was drawn - so a
 * witness sampling that way reports a working draw as nothing drawn. The copy is
 * the path every driver implements, and it is what the vkrend witness reads
 * through too.
 */
static br_colour b2dw_at(b2dw_state *st, int x, int y)
{
    br_rectangle src = {.x = x, .y = y, .w = 1, .h = 1};
    br_point     dst = {.x = 0, .y = 0};

    if(DevicePixelmapRectangleCopyFrom((br_device_pixelmap *)st->offscreen, &dst, (br_device_pixelmap *)st->readback, &src) != BRE_OK) {
        BrLogError("RT", "2D witness: could not read pixel (%d,%d) back from the offscreen.", x, y);
        return 0;
    }

    return BrPixelmapPixelGet(st->readback, 0, 0);
}

/*
 * Fill the offscreen with the clear colour, so each check starts from a known
 * frame rather than from the previous check's.
 */
static void b2dw_clear(b2dw_state *st)
{
    BrPixelmapFill(st->offscreen, 0);
}

static void b2dw_quad(b2dw_state *st, const br_rectangle *clip, int x0, int y0, int x1, int y1, br_colour colour)
{
    const br_vertex_2d vertices[6] = {
        {.position = {{(br_scalar)x0, (br_scalar)y0}}, .map = {{BR_SCALAR(0), BR_SCALAR(1)}}, .colour = colour},
        {.position = {{(br_scalar)x1, (br_scalar)y0}}, .map = {{BR_SCALAR(1), BR_SCALAR(1)}}, .colour = colour},
        {.position = {{(br_scalar)x1, (br_scalar)y1}}, .map = {{BR_SCALAR(1), BR_SCALAR(0)}}, .colour = colour},
        {.position = {{(br_scalar)x0, (br_scalar)y0}}, .map = {{BR_SCALAR(0), BR_SCALAR(1)}}, .colour = colour},
        {.position = {{(br_scalar)x1, (br_scalar)y1}}, .map = {{BR_SCALAR(1), BR_SCALAR(0)}}, .colour = colour},
        {.position = {{(br_scalar)x0, (br_scalar)y1}}, .map = {{BR_SCALAR(0), BR_SCALAR(0)}}, .colour = colour},
    };

    Br2DBegin(st->offscreen, st->material, clip);
    Br2DVertices(vertices, (br_int_32)BR_ASIZE(vertices));
    Br2DEnd();
}

/*
 * The destination is RGB, so a readback carries no alpha, and a colour that goes
 * through the batch's vertex interpolation and blend arrives within a step of
 * where it started rather than exactly on it. This witness asks whether the
 * right thing landed in the right place - the corpus is where exact pixels are
 * checked - so the channels are compared with a tolerance and the alpha ignored.
 */
static br_boolean b2dw_same(br_colour got, br_colour want)
{
    const int dr = (int)BR_RED(got) - (int)BR_RED(want);
    const int dg = (int)BR_GRN(got) - (int)BR_GRN(want);
    const int db = (int)BR_BLU(got) - (int)BR_BLU(want);

    return dr >= -2 && dr <= 2 && dg >= -2 && dg <= 2 && db >= -2 && db <= 2;
}

static br_error b2dw_check_colour(b2dw_state *st, const char *what, int x, int y, br_colour want)
{
    const br_colour got = b2dw_at(st, x, y);

    if(!b2dw_same(got, want)) {
        BrLogError("RT", "2D witness: %s: pixel (%d,%d) is %08x, expected about %08x", what, x, y, (unsigned)got, (unsigned)want);
        return BRE_FAIL;
    }

    return BRE_OK;
}

static br_error b2dw_check_blank(b2dw_state *st)
{
    b2dw_clear(st);

    if(b2dw_check_colour(st, "blank", 2, 2, 0) != BRE_OK)
        return BRE_FAIL;

    if(b2dw_check_colour(st, "blank", B2DW_SIZE - 2, B2DW_SIZE - 2, 0) != BRE_OK)
        return BRE_FAIL;

    return BRE_OK;
}

/*
 * The check a stubbed render() fails. A quad over the middle of the frame has
 * to arrive, in its own colour, with the frame empty outside it.
 */
static br_error b2dw_check_draw(b2dw_state *st)
{
    b2dw_clear(st);
    b2dw_quad(st, NULL, B2DW_LEFT, B2DW_LEFT, B2DW_RIGHT, B2DW_RIGHT, b2dw_red);

    if(b2dw_check_colour(st, "draw centre", B2DW_SIZE / 2, B2DW_SIZE / 2, b2dw_red) != BRE_OK)
        return BRE_FAIL;

    if(b2dw_check_colour(st, "draw corner", 2, 2, 0) != BRE_OK)
        return BRE_FAIL;

    return BRE_OK;
}

/*
 * The API puts the origin at the top left and y down. Every other check here is symmetric in
 * y - the quads run from B2DW_LEFT to B2DW_RIGHT in both axes and the samples are on the
 * centre line - so a frame drawn upside down passes all of them. This one is asymmetric: a
 * band across the top has to arrive at the top, and the matching band at the bottom of the
 * frame stays empty.
 */
static br_error b2dw_check_orientation(b2dw_state *st)
{
    /*
     * Asymmetric in both axes: a band across the top-left. A frame drawn upside down puts it
     * at the bottom, and one drawn mirrored puts it on the right, so both are caught.
     */
    b2dw_clear(st);
    b2dw_quad(st, NULL, 4, 2, 40, 10, b2dw_red);

    if(b2dw_check_colour(st, "orientation inside", 8, 6, b2dw_red) != BRE_OK)
        return BRE_FAIL;

    if(b2dw_check_colour(st, "orientation right", B2DW_SIZE - 12, 6, 0) != BRE_OK)
        return BRE_FAIL;

    if(b2dw_check_colour(st, "orientation bottom", B2DW_SIZE / 2, B2DW_SIZE - 6, 0) != BRE_OK)
        return BRE_FAIL;

    return BRE_OK;
}

/*
 * The clip rect keeps the inside and only the inside, and every edge of it has to do work.
 */
static br_error b2dw_check_clip(b2dw_state *st)
{
    /*
     * The rect is strictly inside the quad on all four sides, so every plane of the clipper
     * has to do work: an edge that coincided with the quad's would leave that plane untested.
     */
    const br_rectangle clip = {
        .x = 16,
        .y = 16,
        .w = 24,
        .h = 24,
    };

    b2dw_clear(st);
    b2dw_quad(st, &clip, B2DW_LEFT, B2DW_LEFT, B2DW_RIGHT, B2DW_RIGHT, b2dw_red);

    if(b2dw_check_colour(st, "clip inside", 28, 28, b2dw_red) != BRE_OK)
        return BRE_FAIL;

    if(b2dw_check_colour(st, "clip left", 12, 28, 0) != BRE_OK)
        return BRE_FAIL;

    if(b2dw_check_colour(st, "clip right", 44, 28, 0) != BRE_OK)
        return BRE_FAIL;

    if(b2dw_check_colour(st, "clip top", 28, 12, 0) != BRE_OK)
        return BRE_FAIL;

    if(b2dw_check_colour(st, "clip bottom", 28, 44, 0) != BRE_OK)
        return BRE_FAIL;

    return BRE_OK;
}

/*
 * A batch draws in submission order, over what is already there. A batch that
 * the renderer deferred and sorted would not be able to tell red-then-blue from
 * blue-then-red.
 */
static br_error b2dw_check_order(b2dw_state *st)
{
    b2dw_clear(st);
    b2dw_quad(st, NULL, B2DW_LEFT, B2DW_LEFT, B2DW_RIGHT, B2DW_RIGHT, b2dw_red);
    b2dw_quad(st, NULL, B2DW_LEFT, B2DW_LEFT, B2DW_RIGHT, B2DW_RIGHT, b2dw_blue);

    if(b2dw_check_colour(st, "order red-then-blue", B2DW_SIZE / 2, B2DW_SIZE / 2, b2dw_blue) != BRE_OK)
        return BRE_FAIL;

    b2dw_clear(st);
    b2dw_quad(st, NULL, B2DW_LEFT, B2DW_LEFT, B2DW_RIGHT, B2DW_RIGHT, b2dw_blue);
    b2dw_quad(st, NULL, B2DW_LEFT, B2DW_LEFT, B2DW_RIGHT, B2DW_RIGHT, b2dw_red);

    if(b2dw_check_colour(st, "order blue-then-red", B2DW_SIZE / 2, B2DW_SIZE / 2, b2dw_red) != BRE_OK)
        return BRE_FAIL;

    return BRE_OK;
}

static br_error b2dw_build(b2dw_state *st, br_pixelmap *screen)
{
    /*
     * RGB_888: every device renders to it, the sprite and 2D blocks are
     * RGB-output only, and brdemo's software default of INDEX_8 would quantise
     * the vertex colours the checks compare against.
     */
    st->offscreen = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGB_888, B2DW_SIZE, B2DW_SIZE);

    if(st->offscreen == NULL) {
        BrLogError("RT", "2D witness: could not match a %dx%d RGB_888 offscreen.", B2DW_SIZE, B2DW_SIZE);
        return BRE_FAIL;
    }

    /*
     * A matched offscreen arrives centred on the screen, so its origin is not
     * (0,0) - and the driver adds it to a copied rectangle. The witness indexes
     * from the top-left of its own frame.
     */
    st->offscreen->origin_x = 0;
    st->offscreen->origin_y = 0;

    /*
     * The destination of the readback: one pixel of host memory, in the offscreen's
     * own type, so what is compared is the colour the frame holds rather than one
     * a conversion moved.
     */
    if((st->readback = BrPixelmapAllocate(BR_PMT_RGB_888, 1, 1, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "2D witness: could not allocate the readback pixelmap.");
        return BRE_FAIL;
    }

    BrRendererBegin(st->offscreen, NULL, NULL, b2dw_heap, sizeof(b2dw_heap));

    /*
     * What a 2D caller gives Br2D: per-vertex colour and alpha, both sides
     * drawn, no colour keying. Everything else a material owns is left at its
     * defaults; the batch sets the 2D state itself.
     */
    st->material = BrMaterialAllocate("rendertest-2d-witness");

    if(st->material == NULL) {
        BrLogError("RT", "2D witness: could not allocate the material.");
        return BRE_FAIL;
    }

    /*
     * A one-texel white map, as a UI renderer binds for a solid fill: the
     * sprite and 2D blocks are textured, and a 2D batch with no map at all is
     * not a shape any of them take.
     */
    if((st->white = BrPixelmapAllocate(BR_PMT_RGB_888, 1, 1, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "2D witness: could not allocate the white texel.");
        return BRE_FAIL;
    }

    ((br_uint_8 *)st->white->pixels)[0] = 255;
    ((br_uint_8 *)st->white->pixels)[1] = 255;
    ((br_uint_8 *)st->white->pixels)[2] = 255;

    st->material->colour_map = st->white;
    st->material->flags      = BR_MATF_PRELIT | BR_MATF_PREALPHA | BR_MATF_ALWAYS_VISIBLE | BR_MATF_DISABLE_COLOUR_KEY;
    st->material->mode       = BR_MATM_SHADING_MODE_GOURAUD | BR_MATM_DEPTH_TEST_AL | BR_MATM_BLEND_MODE_STANDARD;
    st->material->opacity    = 255;

    BrMaterialUpdate(st->material, BR_MATU_ALL);
    BrMaterialAdd(st->material);

    if(st->material->stored == NULL) {
        BrLogError("RT", "2D witness: the material was not prepared against the witness's renderer.");
        return BRE_FAIL;
    }

    return BRE_OK;
}

static void b2dw_fini(b2dw_state *st)
{
    if(st->white != NULL)
        BrPixelmapFree(st->white);

    if(st->readback != NULL)
        BrPixelmapFree(st->readback);

    BrRendererEnd();

    /*
     * BrMaterialFree() is only a BrResFree(): the material stays in the registry, and
     * BrRendererEnd() clears every entry in it - reading mat->stored, which is a child of
     * the renderer and gone by then. So the material is freed after the renderer, not before.
     */
    if(st->material != NULL)
        BrMaterialFree(st->material);

    if(st->offscreen != NULL)
        BrPixelmapFree(st->offscreen);
}

br_error rt_2d_witness_run(rt_demo *demo)
{
    b2dw_state st = {0};
    br_error   r  = BRE_FAIL;

    if(b2dw_build(&st, demo->_screen) != BRE_OK)
        goto out;

    if(b2dw_check_blank(&st) != BRE_OK)
        goto out;

    if(b2dw_check_draw(&st) != BRE_OK)
        goto out;

    if(b2dw_check_orientation(&st) != BRE_OK)
        goto out;

    if(b2dw_check_clip(&st) != BRE_OK)
        goto out;

    if(b2dw_check_order(&st) != BRE_OK)
        goto out;

    BrLogInfo("RT", "2D witness passed (Br2D draws through the immediate geometry path, clips to the rect and composites in order).");
    r = BRE_OK;

out:
    b2dw_fini(&st);
    return r;
}
