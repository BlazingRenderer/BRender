/*
 * The wireframe witness.
 *
 * Two separate facts, checked separately:
 *
 *   - BR_RSTYLE_WIREFRAME draws every triangle edge, and BR_RSTYLE_EDGES does not. The
 *     difference lives in br_face::flags, which the engine's default cube sets on each
 *     side's diagonal and which no glTF-loaded model carries, so the corpus's wireframe
 *     fixture renders exactly what its edges fixture does. Nothing in the corpus can tell
 *     the two styles apart; this does, by drawing each alone.
 *
 *   - Changing style between two models in one frame does not leave the second one with the
 *     first one's geometry-function chain. softrend picks OpTriangleToLines() or
 *     OpTriangleToWireframe() when it builds that chain (V1Faces_GeometryFnsUpdate()), not
 *     per draw, and both styles match the same SP_TOP_LINE block, so a chain reused across
 *     a style change would draw the wrong conversion. The check is that each half of a
 *     two-style frame draws what that style drew on its own.
 *
 * The subject is the engine's default model - the cube whose faces carry
 * BR_FACEF_COPLANAR_*, 12 edges for EDGES and 18 for WIREFRAME - with one material, a host
 * RGB_555 colour buffer and the software renderer, so nothing outside the engine is
 * involved.
 *
 * Failure modes and what each one looks like: no wireframe support leaves the wireframe's
 * half empty; a wireframe that drew the edge list makes the two styles equal; a chain that
 * went stale makes the two-style frame disagree with the single-style frames; a style that
 * drew faces covers an area the fill does.
 */
#include <brender.h>
#include <brddi.h>

#include "rt_demo.h"
#include "wfwitness.h"

#define WFW_WIDTH  320
#define WFW_HEIGHT 240

/*
 * The filled cube covers about 7250 texels of a half at this size and the outlines about 410
 * (12 edges) and 550 (18 edges). These bounds are relations - outline against fill, one
 * outline against the other - rather than measurements of a particular rasteriser, so they
 * are wide around those numbers.
 */
#define WFW_FILL_MIN           3000 /* a half this far from empty is a drawn cube */
#define WFW_FILL_RATIO_PCT     90   /* the two halves must agree within 10% */
#define WFW_EDGES_MIN          50   /* more than a stray pixel */
#define WFW_OUTLINE_DIVISOR    4    /* an outline is under a quarter of its fill */
#define WFW_WIREFRAME_OVER_PCT 115  /* the wireframe must beat the edges by 15% */

typedef struct wfw_counts {
    int count;
    int min_y;
    int max_y;
} wfw_counts;

/* Count set pixels in [x0,x1) of an RGB_555 map, whose clear colour is zero. */
static void wfw_count(const br_pixelmap *pm, int x0, int x1, wfw_counts *out)
{
    const br_uint_16 *pixels = pm->pixels;
    const int         stride = pm->row_bytes / 2;

    out->count = 0;
    out->min_y = pm->height;
    out->max_y = -1;

    for(int y = 0; y < pm->height; ++y) {
        for(int x = x0; x < x1; ++x) {
            if(pixels[y * stride + x] == 0)
                continue;

            out->count++;

            if(y < out->min_y)
                out->min_y = y;
            if(y > out->max_y)
                out->max_y = y;
        }
    }
}

typedef struct wfw_frames {
    wfw_counts fill_left, fill_right;
    wfw_counts edges_alone, wire_alone;
    wfw_counts edges_pair, wire_pair;
} wfw_frames;

static br_error wfw_check(const wfw_frames *f)
{
    /*
     * The fill is the baseline the outlines are compared to: without it, a renderer that
     * drew nothing would satisfy "an outline is much smaller than a fill".
     */
    if(f->fill_left.count < WFW_FILL_MIN || f->fill_right.count < WFW_FILL_MIN) {
        BrLogError("RT", "wireframe witness: the filled cube covered %d and %d texels, expected at least %d in each half; nothing drew.",
                   f->fill_left.count, f->fill_right.count, WFW_FILL_MIN);
        return BRE_FAIL;
    }

    if(f->fill_left.count * 100 < f->fill_right.count * WFW_FILL_RATIO_PCT || f->fill_right.count * 100 < f->fill_left.count * WFW_FILL_RATIO_PCT) {
        BrLogError("RT", "wireframe witness: the two halves of the same filled cube cover %d and %d texels, more than 10%% apart.",
                   f->fill_left.count, f->fill_right.count);
        return BRE_FAIL;
    }

    if(f->edges_alone.count < WFW_EDGES_MIN || f->wire_alone.count < WFW_EDGES_MIN) {
        BrLogError("RT",
                   "wireframe witness: BR_RSTYLE_EDGES covered %d texels and BR_RSTYLE_WIREFRAME %d on their own, expected at "
                   "least %d each.",
                   f->edges_alone.count, f->wire_alone.count, WFW_EDGES_MIN);
        return BRE_FAIL;
    }

    if(f->edges_alone.count >= f->fill_left.count / WFW_OUTLINE_DIVISOR || f->wire_alone.count >= f->fill_right.count / WFW_OUTLINE_DIVISOR) {
        BrLogError("RT", "wireframe witness: the outlines covered %d and %d texels against fills of %d and %d; that is a fill, not an edge.",
                   f->edges_alone.count, f->wire_alone.count, f->fill_left.count, f->fill_right.count);
        return BRE_FAIL;
    }

    /* The diagonals: the one thing the two styles must not agree on. */
    if(f->wire_alone.count * 100 < f->edges_alone.count * WFW_WIREFRAME_OVER_PCT) {
        BrLogError("RT",
                   "wireframe witness: BR_RSTYLE_WIREFRAME covered %d texels and BR_RSTYLE_EDGES %d on their own; the wireframe must "
                   "cover at least %d%% of the edges, or it drew the edge list.",
                   f->wire_alone.count, f->edges_alone.count, WFW_WIREFRAME_OVER_PCT);
        return BRE_FAIL;
    }

    /*
     * The chain. Each half of the two-style frame must draw what that same style drew alone;
     * a chain left over from the other actor moves one half to the other's count.
     */
    if(f->edges_pair.count != f->edges_alone.count || f->wire_pair.count != f->wire_alone.count) {
        BrLogError("RT",
                   "wireframe witness: in one frame BR_RSTYLE_EDGES covered %d texels and BR_RSTYLE_WIREFRAME %d, but on their own %d and "
                   "%d; the second actor reused the first one's geometry-function chain.",
                   f->edges_pair.count, f->wire_pair.count, f->edges_alone.count, f->wire_alone.count);
        return BRE_FAIL;
    }

    BrLogInfo("RT",
              "wireframe witness passed (filled cube %d and %d texels; alone, BR_RSTYLE_EDGES %d texels rows %d..%d and "
              "BR_RSTYLE_WIREFRAME %d texels rows %d..%d, %d%% of the edges; together, %d and %d).",
              f->fill_left.count, f->fill_right.count, f->edges_alone.count, f->edges_alone.min_y, f->edges_alone.max_y, f->wire_alone.count,
              f->wire_alone.min_y, f->wire_alone.max_y, f->wire_alone.count * 100 / f->edges_alone.count, f->edges_pair.count, f->wire_pair.count);

    return BRE_OK;
}

br_error rt_wf_witness_run(rt_demo *demo)
{
    static char  primitive_heap[256 * 1024];
    br_pixelmap *colour = NULL, *depth = NULL;
    br_actor    *world = NULL, *camera = NULL, *left = NULL, *right = NULL;
    br_material *material = NULL;
    br_camera   *camera_data;
    wfw_frames   f;
    br_error     r = BRE_FAIL;

    (void)demo;

    if((colour = BrPixelmapAllocate(BR_PMT_RGB_555, WFW_WIDTH, WFW_HEIGHT, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "wireframe witness: could not allocate a %dx%d RGB_555 colour buffer.", WFW_WIDTH, WFW_HEIGHT);
        return BRE_FAIL;
    }

    /*
     * The origin is the projection centre. A host buffer starts with it at zero, which puts
     * the scene off the corner of the image.
     */
    colour->origin_x = (br_int_16)(WFW_WIDTH / 2);
    colour->origin_y = (br_int_16)(WFW_HEIGHT / 2);

    if((depth = BrPixelmapMatch(colour, BR_PMMATCH_DEPTH_16)) == NULL) {
        BrLogError("RT", "wireframe witness: could not match a depth buffer.");
        goto out;
    }

    BrRendererBegin(colour, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    if((world = BrActorAllocate(BR_ACTOR_NONE, NULL)) == NULL)
        goto out;

    BrZsActorOrderTableSet(world, BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_LEAVE_BOUNDS | BR_ORDER_TABLE_BUCKET_SORT, BR_SORT_AVERAGE));

    if((camera = BrActorAdd(world, BrActorAllocate(BR_ACTOR_CAMERA, NULL))) == NULL)
        goto out;

    camera->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&camera->t.t.mat, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(8));

    camera_data                = camera->type_data;
    camera_data->type          = BR_CAMERA_PERSPECTIVE;
    camera_data->field_of_view = BR_ANGLE_DEG(45.0);
    camera_data->aspect        = BR_DIV(BR_SCALAR(WFW_WIDTH), BR_SCALAR(WFW_HEIGHT));
    camera_data->hither_z      = BR_SCALAR(0.1);
    camera_data->yon_z         = BR_SCALAR(20);

    if((material = BrMaterialAllocate("wireframe-witness")) == NULL)
        goto out;

    material->colour  = BR_COLOUR_RGB(220, 220, 220);
    material->opacity = 255;
    BrMaterialAdd(material);
    BrMaterialUpdate(material, BR_MATU_ALL);

    /*
     * Model NULL: the actor draws v1db's default model, the cube whose faces carry
     * BR_FACEF_COPLANAR_* on each side's diagonal - the model the two styles differ on, and
     * the one the sheet demo draws as edges.
     */
    if((left = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL))) == NULL)
        goto out;

    if((right = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL))) == NULL)
        goto out;

    left->material  = material;
    right->material = material;
    left->t.type    = BR_TRANSFORM_MATRIX34;
    right->t.type   = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&left->t.t.mat, BR_SCALAR(-1.6), BR_SCALAR(0), BR_SCALAR(0));
    BrMatrix34Translate(&right->t.t.mat, BR_SCALAR(1.6), BR_SCALAR(0), BR_SCALAR(0));

#define WFW_RENDER()                                   \
    do {                                               \
        BrPixelmapFill(colour, 0);                     \
        BrPixelmapFill(depth, 0xFFFFFFFF);             \
        BrZbSceneRender(world, camera, colour, depth); \
    } while(0)

    /* The baseline: the same two cubes as faces, so the outlines can be compared to a fill. */
    left->render_style  = BR_RSTYLE_FACES;
    right->render_style = BR_RSTYLE_FACES;
    WFW_RENDER();
    wfw_count(colour, 0, WFW_WIDTH / 2, &f.fill_left);
    wfw_count(colour, WFW_WIDTH / 2, WFW_WIDTH, &f.fill_right);

    /* Each style on its own: the wireframe as such, and the edge list it has to differ from. */
    left->render_style  = BR_RSTYLE_EDGES;
    right->render_style = BR_RSTYLE_NONE;
    WFW_RENDER();
    wfw_count(colour, 0, WFW_WIDTH / 2, &f.edges_alone);

    left->render_style  = BR_RSTYLE_NONE;
    right->render_style = BR_RSTYLE_WIREFRAME;
    WFW_RENDER();
    wfw_count(colour, WFW_WIDTH / 2, WFW_WIDTH, &f.wire_alone);

    /* And in one frame, so the second actor's geometry-function chain is built after the first's. */
    left->render_style  = BR_RSTYLE_EDGES;
    right->render_style = BR_RSTYLE_WIREFRAME;
    WFW_RENDER();
    wfw_count(colour, 0, WFW_WIDTH / 2, &f.edges_pair);
    wfw_count(colour, WFW_WIDTH / 2, WFW_WIDTH, &f.wire_pair);

#undef WFW_RENDER

    r = wfw_check(&f);

out:
    if(world != NULL)
        BrActorFree(world);

    if(material != NULL)
        BrMaterialFree(material);

    BrRendererEnd();

    if(depth != NULL)
        BrPixelmapFree(depth);

    if(colour != NULL)
        BrPixelmapFree(colour);

    return r;
}
