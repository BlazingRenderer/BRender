/*
 * The mip witness: a minified textured quad, read back, compared with the box the chain
 * ought to hold.
 *
 * A reference frame cannot do this job. The chain is produced by the driver under test, so a
 * reference row would encode that driver's filter and bless it. This asserts instead that a
 * level below 0 is reached at all, and what that level's byte must hold; the chain's own shape
 * belongs to mipw_check_geometry(), which is pure core and needs no renderer. Which level is
 * reached is deliberately left open - every level of a checkerboard is the same byte, so only
 * a map whose levels differ could name one.
 *
 * The sampled map is a 16x16 red/blue checkerboard, so every 2x2 block holds two texels of
 * each colour and every level below 0 is the same 50/50 mix. The quad covers four pixels of the
 * 64x64 offscreen at a 90-degree field of view, so the texture is minified four-to-one and the
 * LOD lands on a level below 0, whichever one that is. With
 * BR_MATF_MIP_INTERPOLATION the centre pixel must be the box: 127.5 rounds half-up to
 * 0x80, where a filter that truncated would write 0x7f. Without the flag the sampler is
 * pinned to level 0 and the centre must be a hard red or blue texel, which is what catches
 * a chain that is not reached at all.
 */
#include <brender.h>
#include <brddi.h>

#include "rt_demo.h"
#include "mipwitness.h"

#define MIPW_SIZE     64
#define MIPW_MAP      16
#define MIPW_MAP_W    32
#define MIPW_MAP_H    8
#define MIPW_HALF_EXT BR_SCALAR(1.0)
#define MIPW_CAM_DIST BR_SCALAR(16.0)

/*
 * The mix, in the order an RGBA_8888 pixelmap stores it: BR_COLOUR_RGBA() packs A,R,G,B into a
 * br_colour and the map keeps that word, so on a little-endian host its bytes are B,G,R,A and
 * px[0] is blue. The values below are compared against the channel each one names.
 */
#define MIPW_MIX_R 0x80
#define MIPW_MIX_G 0x00
#define MIPW_MIX_B 0x80

typedef struct mipw_state {
    br_pixelmap *offscreen;
    br_pixelmap *depth;
    br_pixelmap *colour_map;
    br_pixelmap *rect_map;
    br_actor    *world;
    br_actor    *camera;
    br_actor    *actor;
    br_camera   *camera_data;
    br_material *material;
    br_model    *model;
} mipw_state;

/* A red/blue checkerboard: every 2x2 block is half red and half blue. */
static br_colour mipw_texel_colour(br_int_32 x, br_int_32 y)
{
    return ((x ^ y) & 1) ? BR_COLOUR_RGB(0x00, 0x00, 0xff) : BR_COLOUR_RGB(0xff, 0x00, 0x00);
}

static br_error mipw_make_map(mipw_state *st, br_pixelmap **out, br_int_32 width, br_int_32 height)
{
    br_pixelmap *pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, width, height, NULL, BR_PMAF_NORMAL);

    if(pm == NULL)
        return BRE_FAIL;

    for(br_int_32 y = 0; y < height; ++y)
        for(br_int_32 x = 0; x < width; ++x)
            BrPixelmapPixelSet(pm, x, y, mipw_texel_colour(x, y));

    BrMapUpdate(pm, BR_MAPU_ALL);
    *out = pm;
    return BRE_OK;
}

static br_error mipw_build(mipw_state *st, br_pixelmap *screen)
{
    static char primitive_heap[256 * 1024];

    st->offscreen = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBA_8888, MIPW_SIZE, MIPW_SIZE);
    if(st->offscreen == NULL) {
        BrLogError("RT", "mip witness: could not match a %dx%d RGBA_8888 offscreen.", MIPW_SIZE, MIPW_SIZE);
        return BRE_FAIL;
    }

    st->offscreen->origin_x = st->offscreen->origin_y = (br_int_16)(MIPW_SIZE / 2);

    if((st->depth = BrPixelmapMatch(st->offscreen, BR_PMMATCH_DEPTH_16)) == NULL) {
        BrLogError("RT", "mip witness: could not match a depth buffer for the offscreen.");
        return BRE_FAIL;
    }

    /*
     * The witness gets a renderer of its own, bound to the offscreen it reads back from: the
     * material's stored state is built against the renderer's output, so it has to be this
     * one and not the demo's.
     */
    BrRendererBegin(st->offscreen, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    if(mipw_make_map(st, &st->colour_map, MIPW_MAP, MIPW_MAP) != BRE_OK) {
        BrLogError("RT", "mip witness: could not build the %dx%d map.", MIPW_MAP, MIPW_MAP);
        return BRE_FAIL;
    }

    if(mipw_make_map(st, &st->rect_map, MIPW_MAP_W, MIPW_MAP_H) != BRE_OK) {
        BrLogError("RT", "mip witness: could not build the %dx%d map.", MIPW_MAP_W, MIPW_MAP_H);
        return BRE_FAIL;
    }

    st->material = BrMaterialAllocate("rendertest-mip-witness");
    st->model    = BrModelAllocate("rendertest-mip-witness", 4, 2);
    if(st->material == NULL || st->model == NULL) {
        BrLogError("RT", "mip witness: could not allocate the material or model.");
        return BRE_FAIL;
    }

    st->material->colour     = BR_COLOUR_RGB(255, 255, 255);
    st->material->opacity    = 255;
    st->material->colour_map = st->colour_map;
    st->material->flags &= ~BR_MATF_LIGHT;
    BrMatrix23Identity(&st->material->map_transform);

    BrVector3Set(&st->model->vertices[0].p, -MIPW_HALF_EXT, -MIPW_HALF_EXT, BR_SCALAR(0.0));
    BrVector3Set(&st->model->vertices[1].p, MIPW_HALF_EXT, -MIPW_HALF_EXT, BR_SCALAR(0.0));
    BrVector3Set(&st->model->vertices[2].p, MIPW_HALF_EXT, MIPW_HALF_EXT, BR_SCALAR(0.0));
    BrVector3Set(&st->model->vertices[3].p, -MIPW_HALF_EXT, MIPW_HALF_EXT, BR_SCALAR(0.0));

    st->model->flags |= BR_MODF_CUSTOM_NORMALS;
    for(int i = 0; i < 4; ++i) {
        st->model->vertices[i].n.v[0] = 0;
        st->model->vertices[i].n.v[1] = 0;
        st->model->vertices[i].n.v[2] = BrScalarToFraction(BR_SCALAR(1.0));

        /* The map's own co-ordinate, quarter-texel in, so no fragment sits on a seam. */
        st->model->vertices[i].map.v[0] = BrScalarToFraction((i == 1 || i == 2) ? BR_SCALAR(1.0) : BR_SCALAR(0.0));
        st->model->vertices[i].map.v[1] = BrScalarToFraction((i >= 2) ? BR_SCALAR(1.0) : BR_SCALAR(0.0));
    }

    st->model->faces[0].vertices[0] = 0;
    st->model->faces[0].vertices[1] = 1;
    st->model->faces[0].vertices[2] = 2;
    st->model->faces[0].material    = st->material;

    st->model->faces[1].vertices[0] = 0;
    st->model->faces[1].vertices[1] = 2;
    st->model->faces[1].vertices[2] = 3;
    st->model->faces[1].material    = st->material;

    BrMaterialUpdate(st->material, BR_MATU_ALL);
    BrModelUpdate(st->model, BR_MODU_ALL);

    st->world          = BrActorAllocate(BR_ACTOR_NONE, NULL);
    st->camera         = BrActorAdd(st->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    st->camera->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&st->camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), MIPW_CAM_DIST);
    /*
     * The quad sits MIPW_CAM_DIST away, past the default camera's yon of 10, and a fixed
     * 90-degree field of view makes it cover MIPW_SIZE / (2 * MIPW_CAM_DIST) pixels.
     */
    st->camera_data                = st->camera->type_data;
    st->camera_data->field_of_view = BR_ANGLE_DEG(90);
    st->camera_data->hither_z      = BR_SCALAR(1.0);
    st->camera_data->yon_z         = BR_SCALAR(100.0);
    st->camera_data->aspect        = BR_SCALAR(1.0);

    st->actor           = BrActorAdd(st->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    st->actor->t.type   = BR_TRANSFORM_IDENTITY;
    st->actor->model    = st->model;
    st->actor->material = st->material;

    return BRE_OK;
}

/*
 * Render the quad with `flags` and return the centre pixel. The quad spans
 * MIPW_SIZE/(2*MIPW_CAM_DIST) pixels at a 90-degree field of view, so it covers four.
 */
static br_error mipw_read_centre(mipw_state *st, br_pixelmap *map, br_uint_32 flags, br_uint_8 px[4], const char *label)
{
    const br_int_32 cx = st->offscreen->width / 2;
    const br_int_32 cy = st->offscreen->height / 2;
    br_rectangle whole = {.x = -st->offscreen->origin_x, .y = -st->offscreen->origin_y, .w = st->offscreen->width, .h = st->offscreen->height};
    br_pixelmap *readback;
    br_error     r = BRE_FAIL;

    st->material->colour_map = map;
    st->material->flags &= ~(BR_MATF_MAP_INTERPOLATION | BR_MATF_MIP_INTERPOLATION);
    st->material->flags |= flags;
    BrMaterialUpdate(st->material, BR_MATU_ALL);

    if(DevicePixelmapRectangleFill(st->offscreen, &whole, BR_COLOUR_RGB(0, 0, 0)) != BRE_OK)
        return BRE_FAIL;

    BrPixelmapFill(st->depth, 0xffffffff);
    BrZbSceneRender(st->world, st->camera, st->offscreen, st->depth);

    readback = BrPixelmapAllocate(BR_PMT_RGBA_8888, st->offscreen->width, st->offscreen->height, NULL, BR_PMAF_NORMAL);
    if(readback == NULL)
        return BRE_FAIL;

    {
        br_rectangle src = whole;
        br_point     dst = {0, 0};

        if(DevicePixelmapRectangleCopyFrom((br_device_pixelmap *)st->offscreen, &dst, (br_device_pixelmap *)readback, &src) != BRE_OK)
            goto out;
    }

    BrMemCpy(px, (const br_uint_8 *)readback->pixels + cy * readback->row_bytes + cx * 4, 4);

    BrLogInfo("RT", "mip witness: %s read %02x %02x %02x %02x.", label, px[0], px[1], px[2], px[3]);
    r = BRE_OK;

out:
    BrPixelmapFree(readback);
    return r;
}

static br_error mipw_check_map(mipw_state *st, br_pixelmap *map, const char *what)
{
    br_uint_8 px[4];

    /* Pinned to level 0: a hard checkerboard texel, never the mix. */
    if(mipw_read_centre(st, map, 0, px, "nearest with no chain") != BRE_OK)
        return BRE_FAIL;

    if(!((px[0] == 0x00 && px[1] == 0x00 && px[2] == 0xff) || (px[0] == 0xff && px[1] == 0x00 && px[2] == 0x00))) {
        BrLogError("RT", "mip witness: the unfiltered sample of %s read %02x %02x %02x %02x, expected a hard red or blue texel.", what,
                   px[0], px[1], px[2], px[3]);
        return BRE_FAIL;
    }

    /*
     * The chain is reached: the mix, to the byte. 0x7f is what a truncating filter writes,
     * so this is also the assertion that pins the tie-break the reference rows carry.
     */
    if(mipw_read_centre(st, map, BR_MATF_MIP_INTERPOLATION, px, "nearest with the chain") != BRE_OK)
        return BRE_FAIL;

    if(px[0] != MIPW_MIX_B || px[1] != MIPW_MIX_G || px[2] != MIPW_MIX_R || px[3] != 0xff) {
        BrLogError("RT",
                   "mip witness: the minified sample of %s read %02x %02x %02x %02x, expected %02x %02x %02x ff - the 50/50 mix rounded "
                   "half up.",
                   what, px[0], px[1], px[2], px[3], MIPW_MIX_B, MIPW_MIX_G, MIPW_MIX_R);
        return BRE_FAIL;
    }

    /* Both flags: the mipmap mode blends two levels that are already the mix. */
    if(mipw_read_centre(st, map, BR_MATF_MIP_INTERPOLATION | BR_MATF_MAP_INTERPOLATION, px, "linear with the chain") != BRE_OK)
        return BRE_FAIL;

    if(px[0] != MIPW_MIX_B || px[1] != MIPW_MIX_G || px[2] != MIPW_MIX_R || px[3] != 0xff) {
        BrLogError("RT", "mip witness: the trilinear sample of %s read %02x %02x %02x %02x, expected %02x %02x %02x ff.", what, px[0],
                   px[1], px[2], px[3], MIPW_MIX_B, MIPW_MIX_G, MIPW_MIX_R);
        return BRE_FAIL;
    }

    return BRE_OK;
}

/*
 * The chain's own geometry, read through the public accessor rather than a renderer: the
 * levels of a 32x8 map halve each axis separately, so level i is
 * max(1, 32 >> i) x max(1, 8 >> i), each begins where the one above ends, and each level
 * below 0 has a tight row. A rule that used the width for both axes passes on a square map
 * and fails here, which is what the rendered case above cannot see - every level of a
 * checkerboard is the same mix, so it can only say a chain was reached.
 */
static br_error mipw_check_geometry(void)
{
    br_pixelmap *src = BrPixelmapAllocate(BR_PMT_RGBA_8888, MIPW_MAP_W, MIPW_MAP_H, NULL, BR_PMAF_NORMAL);
    br_pixelmap *chain;
    br_int_32    want_w = MIPW_MAP_W, want_h = MIPW_MAP_H;
    br_size_t    want_at = 0;
    br_uint_32   levels;
    br_error     r = BRE_FAIL;

    if(src == NULL)
        return BRE_FAIL;

    src->identifier = BrResStrDup(src, "mip-geometry");

    for(br_int_32 y = 0; y < MIPW_MAP_H; ++y)
        for(br_int_32 x = 0; x < MIPW_MAP_W; ++x)
            BrPixelmapPixelSet(src, x, y, mipw_texel_colour(x, y));

    if((chain = BrPixelmapMakeMipMap(src, BR_PMT_RGBA_8888, NULL, 0, 0, BR_QUANTIZE_RGB)) == NULL) {
        BrLogError("RT", "mip witness: could not build a %dx%d chain.", MIPW_MAP_W, MIPW_MAP_H);
        goto out;
    }

    levels = BrPixelmapMipLevelCount(chain);

    if(levels != 6) {
        BrLogError("RT", "mip witness: a %dx%d chain has %u levels, expected 6.", MIPW_MAP_W, MIPW_MAP_H, levels);
        goto out_chain;
    }

    for(br_uint_32 level = 0; level < levels; ++level) {
        br_pixelmap *view = BrPixelmapMipLevel(chain, level);
        br_size_t    at;

        if(view == NULL) {
            BrLogError("RT", "mip witness: level %u of a %dx%d chain is unreachable.", level, MIPW_MAP_W, MIPW_MAP_H);
            goto out_chain;
        }

        at = (br_size_t)((br_uint_8 *)view->pixels - (br_uint_8 *)chain->pixels);

        if(view->width != want_w || view->height != want_h || at != want_at || view->row_bytes != want_w * 4) {
            BrLogError("RT", "mip witness: level %u of a %dx%d chain is %dx%d at %lld with a %d-byte row, expected %dx%d at %lld with %d.",
                       level, MIPW_MAP_W, MIPW_MAP_H, view->width, view->height, (long long)at, view->row_bytes, want_w, want_h,
                       (long long)want_at, want_w * 4);
            BrPixelmapFree(view);
            goto out_chain;
        }

        BrPixelmapFree(view);

        want_at += (br_size_t)want_w * want_h * 4;
        want_w = (want_w > 1) ? (want_w >> 1) : 1;
        want_h = (want_h > 1) ? (want_h >> 1) : 1;
    }

    {
        br_pixelmap *past = BrPixelmapMipLevel(chain, levels);

        if(past != NULL) {
            BrLogError("RT", "mip witness: level %u of a %dx%d chain exists and must not.", levels, MIPW_MAP_W, MIPW_MAP_H);
            BrPixelmapFree(past);
            goto out_chain;
        }
    }

    BrLogInfo("RT", "mip witness: the %dx%d chain's six levels halve each axis and pack tight.", MIPW_MAP_W, MIPW_MAP_H);
    r = BRE_OK;

out_chain:
    BrPixelmapFree(chain);
out:
    BrPixelmapFree(src);
    return r;
}

/*
 * A conversion, which is the path that does not copy: level 0 is written a pixel at a time
 * into a destination whose type differs from the source's. The chain is a 2x2 RGBA_8888
 * result, so level 0 is eight bytes a row and level 1 begins at sixteen - a row written at
 * the wrong offset lands in the level below, which makes a broken stride visible as a wrong
 * colour rather than as a quiet shift, and only this path takes it. Both the drivers pass
 * the map's own type as the destination, so nothing else in the suite reaches it.
 */
static br_error mipw_check_conversion(void)
{
    static const br_colour texel[4] = {
        BR_COLOUR_RGB(0xff, 0x00, 0x00),
        BR_COLOUR_RGB(0x00, 0xff, 0x00),
        BR_COLOUR_RGB(0x00, 0x00, 0xff),
        BR_COLOUR_RGB(0xff, 0xff, 0xff),
    };
    br_pixelmap *src = BrPixelmapAllocate(BR_PMT_RGB_888, 2, 2, NULL, BR_PMAF_NORMAL);
    br_pixelmap *chain;
    br_error     r = BRE_FAIL;

    if(src == NULL)
        return BRE_FAIL;

    src->identifier = BrResStrDup(src, "mip-conversion");

    for(br_int_32 y = 0; y < 2; ++y)
        for(br_int_32 x = 0; x < 2; ++x)
            BrPixelmapPixelSet(src, x, y, texel[y * 2 + x]);

    if((chain = BrPixelmapMakeMipMap(src, BR_PMT_RGBA_8888, NULL, 0, 0, BR_QUANTIZE_RGB)) == NULL) {
        BrLogError("RT", "mip witness: could not build a chain from an RGB_888 source.");
        goto out;
    }

    for(br_int_32 y = 0; y < 2; ++y) {
        br_uint_8 *pixel = (br_uint_8 *)chain->pixels + (br_size_t)y * chain->row_bytes;

        for(br_int_32 x = 0; x < 2; ++x, pixel += 4) {
            const br_uint_8 red = BR_RED(texel[y * 2 + x]), green = BR_GRN(texel[y * 2 + x]), blue = BR_BLU(texel[y * 2 + x]);

            if(pixel[0] != blue || pixel[1] != green || pixel[2] != red || pixel[3] != 0xff) {
                BrLogError("RT", "mip witness: RGB_888 to RGBA_8888 wrote %02x %02x %02x %02x at level 0 (%d,%d), expected %02x %02x %02x ff.",
                           pixel[0], pixel[1], pixel[2], pixel[3], x, y, blue, green, red);
                goto out_chain;
            }
        }
    }

    {
        const br_uint_8 *mix = (br_uint_8 *)chain->pixels + (br_size_t)chain->mip_offset * chain->row_bytes;

        if(mix[0] != 0x80 || mix[1] != 0x80 || mix[2] != 0x80 || mix[3] != 0xff) {
            BrLogError("RT", "mip witness: the conversion's level 1 read %02x %02x %02x %02x, expected 80 80 80 ff.", mix[0], mix[1],
                       mix[2], mix[3]);
            goto out_chain;
        }
    }

    BrLogInfo("RT", "mip witness: an RGB_888 source converts to an RGBA_8888 level 0 row by row, and mixes to 80 80 80 ff.");
    r = BRE_OK;

out_chain:
    BrPixelmapFree(chain);
out:
    BrPixelmapFree(src);
    return r;
}

static br_error mipw_check(mipw_state *st)
{
    /*
     * Two chains, because the level extents halve each axis separately: a square map cannot
     * tell a per-axis rule from one that uses the width for both.
     */
    if(mipw_check_map(st, st->colour_map, "the 16x16 map") != BRE_OK)
        return BRE_FAIL;

    if(mipw_check_map(st, st->rect_map, "the 32x8 map") != BRE_OK)
        return BRE_FAIL;

    if(mipw_check_geometry() != BRE_OK)
        return BRE_FAIL;

    if(mipw_check_conversion() != BRE_OK)
        return BRE_FAIL;

    BrLogInfo("RT",
              "mip witness passed (16x16 and 32x8 red/blue checkerboards minified: pinned to level 0 a hard texel, with the chain the "
              "50/50 mix at %02x %02x %02x ff, both mipmap modes).",
              MIPW_MIX_B, MIPW_MIX_G, MIPW_MIX_R);

    return BRE_OK;
}

static void mipw_fini(mipw_state *st)
{
    BrRendererEnd();

    if(st->colour_map != NULL)
        BrPixelmapFree(st->colour_map);

    if(st->rect_map != NULL)
        BrPixelmapFree(st->rect_map);

    if(st->depth != NULL)
        BrPixelmapFree(st->depth);

    if(st->offscreen != NULL)
        BrPixelmapFree(st->offscreen);

    if(st->material != NULL)
        BrMaterialFree(st->material);

    if(st->model != NULL)
        BrModelFree(st->model);

    if(st->world != NULL)
        BrActorFree(st->world);

    BrMemSet(st, 0, sizeof(*st));
}

br_error rt_mip_witness_run(rt_demo *demo, const char *device)
{
    mipw_state state = {0};
    br_error   r     = BRE_FAIL;

    /*
     * glrend is the reference device and the one whose upload builds the chain. softrend
     * samples its maps per fragment but keeps no chain and selects no LOD, and glrend1x
     * uploads level 0 only, so neither has an average to measure - the corpus pins their
     * samplers to level 0 instead.
     */
    if(BrStrCmp(device, "glrend") != 0) {
        BrLogError("RT", "mip witness: --mip-witness asserts a chain on glrend only; %s has none to measure.", device);
        return BRE_FAIL;
    }

    if(mipw_build(&state, demo->_screen) != BRE_OK) {
        BrLogError("RT", "mip witness: could not build the scene (does the device allow a %dx%d RGBA_8888 offscreen?).", MIPW_SIZE, MIPW_SIZE);
        goto out;
    }

    if(mipw_check(&state) != BRE_OK)
        goto out;

    r = BRE_OK;

out:
    mipw_fini(&state);
    return r;
}
