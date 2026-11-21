/*
 * The parallax witness.
 *
 * The corpus asserts that the parallax pair differs from the depth-0 pair on glrend, that the
 * two software paths bless their pairs equal, and that a bound bump map at depth 0 changes
 * nothing. Those are relations, and a relation is satisfied by a shader whose offset has the
 * wrong sign, the wrong magnitude or the wrong axis - it differs either way.
 *
 * This witnesses the offset itself. The scene is one unlit quad with a gradient albedo, so the
 * pixel *is* the sampled coordinate and no shading can be mistaken for an offset; its bump map
 * is a flat normal with a constant height in alpha, so the offset is the only thing the bump
 * map contributes. Unlit also keeps the map's channel order out of the answer: the tangent
 * frame reads that normal, and this witness does not need to say which byte is which. The
 * camera always looks at the quad's centre, so the frame-centre pixel is the probe and its
 * fragment-to-eye direction is the camera's own, known here.
 *
 * The build, the render and the readback are the environment witness's, call for call, because
 * that is the one hand-built scene in this tool known to draw. Everything beyond it was added
 * one step at a time against a frame that was known to cover the centre.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <brender.h>
#include <brddi.h>

#include "rt_demo.h"
#include "parallaxwitness.h"

#define PXW_SIZE      512
#define PXW_MAP       256
#define PXW_QUAD      BR_SCALAR(2.0)
#define PXW_CAM_DIST  BR_SCALAR(8.0)
#define PXW_HEIGHT    200 /* alpha of the height map: 200/255 of a unit */
#define PXW_SAMPLES   4
#define PXW_TOLERANCE 2 /* colour steps, for the projection's half-pixel skew */

static const struct pxw_sample {
    br_uint_32  theta_deg;
    double      depth;
    const char *what;
} pxw_samples[PXW_SAMPLES] = {
    {0,  0.2, "normal incidence"     },
    {30, 0.1, "30 degrees, depth 0.1"},
    {30, 0.2, "30 degrees, depth 0.2"},
    {60, 0.2, "60 degrees, depth 0.2"},
};

typedef struct pxw_state {
    br_pixelmap *offscreen;
    br_pixelmap *depth;
    br_pixelmap *albedo;
    br_pixelmap *height;
    br_pixelmap *flat;
    br_pixelmap *frame;
    br_actor    *world;
    br_actor    *camera;
    br_actor    *actor;
    br_actor    *light;
    br_camera   *camera_data;
    br_material *material;
    br_model    *model;
} pxw_state;

/* The unoffset coordinate's texel: u = 0.5 is texel 128 of a 256-wide NEAREST map. */
#define PXW_CENTRE_VALUE 128

/*
 * A two-axis gradient written through BrPixelmapPixelSet, as the environment witness writes
 * its texel names: the accessor is the one the renderers' samplers agree with, so the decode
 * does not depend on the map's byte order. u goes into both R and G and B is zero, so the
 * readback is independent of the framebuffer's channel order too.
 */
static br_error pxw_make_albedo(pxw_state *st)
{
    br_pixelmap *pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, PXW_MAP, PXW_MAP, NULL, BR_PMAF_NORMAL);

    if(pm == NULL)
        return BRE_FAIL;

    for(br_int_32 ty = 0; ty < PXW_MAP; ++ty) {
        for(br_int_32 tx = 0; tx < PXW_MAP; ++tx) {
            br_uint_8 u = (br_uint_8)tx;

            BrPixelmapPixelSet(pm, tx, ty, BR_COLOUR_RGB(u, u, 0));
        }
    }

    BrMapUpdate(pm, BR_MAPU_ALL);
    st->albedo = pm;
    return BRE_OK;
}

/* A flat tangent-space normal in RGB and the height in alpha, written through the accessor. */
static br_pixelmap *pxw_make_bump(br_uint_8 alpha)
{
    br_pixelmap *pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, PXW_MAP, PXW_MAP, NULL, BR_PMAF_NORMAL);

    if(pm == NULL)
        return NULL;

    /*
     * A flat tangent-space normal and a constant height, written through the accessor: a packed
     * RGBA colour lands in memory as B,G,R,A, which is the order the sampler reads back as
     * R,G,B. Writing the bytes by hand here had x and z swapped, which the ambient-only rig
     * could not see.
     */
    for(br_int_32 ty = 0; ty < PXW_MAP; ++ty)
        for(br_int_32 tx = 0; tx < PXW_MAP; ++tx)
            BrPixelmapPixelSet(pm, tx, ty, BR_COLOUR_RGBA(128, 128, 255, alpha));

    BrMapUpdate(pm, BR_MAPU_ALL);
    return pm;
}

static br_error pxw_build(pxw_state *st, br_pixelmap *screen)
{
    static char primitive_heap[256 * 1024];

    st->offscreen = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBA_8888, PXW_SIZE, PXW_SIZE);
    if(st->offscreen == NULL) {
        BrLogError("RT", "parallax witness: could not match a %dx%d RGBA_8888 offscreen.", PXW_SIZE, PXW_SIZE);
        return BRE_FAIL;
    }

    st->offscreen->origin_x = st->offscreen->origin_y = (br_int_16)(PXW_SIZE / 2);

    if((st->depth = BrPixelmapMatch(st->offscreen, BR_PMMATCH_DEPTH_16)) == NULL) {
        BrLogError("RT", "parallax witness: could not match a depth buffer for the offscreen.");
        return BRE_FAIL;
    }

    BrRendererBegin(st->offscreen, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    if(pxw_make_albedo(st) != BRE_OK)
        return BRE_FAIL;

    st->material = BrMaterialAllocate("rendertest-parallax-witness");
    st->model    = BrModelAllocate("rendertest-parallax-witness", 4, 2);
    if(st->material == NULL || st->model == NULL) {
        BrLogError("RT", "parallax witness: could not allocate the material or model.");
        return BRE_FAIL;
    }

    st->material->colour     = BR_COLOUR_RGB(255, 255, 255);
    st->material->opacity    = 255;
    st->material->ka         = BR_SCALAR(1.0);
    st->material->kd         = BR_SCALAR(0.0);
    st->material->ks         = BR_SCALAR(0.0);
    st->material->power      = BR_SCALAR(20.0);
    st->material->colour_map = st->albedo;
    st->material->flags |= BR_MATF_LIGHT | BR_MATF_SMOOTH;
    st->material->bump_map = st->height = pxw_make_bump(PXW_HEIGHT);
    st->flat                            = pxw_make_bump(0);
    st->material->bump_scale            = BR_SCALAR(1.0);
    st->material->bump_depth            = BR_UFRACTION(0.2);
    BrMatrix23Identity(&st->material->map_transform);
    BrMatrix23Identity(&st->material->bump_map_transform);

    {
        static const struct {
            br_scalar x, y, u, v;
        } corners[4] = {
            {-2, -2, 0, 0},
            {2,  -2, 1, 0},
            {2,  2,  1, 1},
            {-2, 2,  0, 1},
        };

        for(int i = 0; i < 4; ++i) {
            br_vertex *v = &st->model->vertices[i];

            BrVector3Set(&v->p, corners[i].x, corners[i].y, BR_SCALAR(0.0));
            v->map.v[0] = corners[i].u;
            v->map.v[1] = corners[i].v;
            v->n.v[0]   = BrScalarToFraction(BR_SCALAR(0.0));
            v->n.v[1]   = BrScalarToFraction(BR_SCALAR(0.0));
            v->n.v[2]   = BrScalarToFraction(BR_SCALAR(1.0));
        }
    }

    st->model->flags |= BR_MODF_CUSTOM_NORMALS;

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
    {
        const br_angle a = BR_ANGLE_DEG(BR_SCALAR(30.0));

        BrMatrix34RotateY(&st->camera->t.t.mat, a);
        st->camera->t.t.mat.m[3][0] = BR_MUL(BR_SIN(a), PXW_CAM_DIST);
        st->camera->t.t.mat.m[3][1] = BR_SCALAR(0.0);
        st->camera->t.t.mat.m[3][2] = BR_MUL(BR_COS(a), PXW_CAM_DIST);
        st->camera->t.t.mat.m[3][3] = BR_SCALAR(1.0);
    }
    st->camera_data         = st->camera->type_data;
    st->camera_data->aspect = BR_SCALAR(1.0);

    /*
     * An ambient light and a fully ambient material: the modulation is ka times the light's
     * colour at every fragment, by construction, with no direction or distance to get wrong and
     * no dependence on the map's channel order. The material is still flagged BR_MATF_LIGHT,
     * which is the switch that decides whether a bump map is read at all.
     */
    st->light         = BrActorAdd(st->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    st->light->t.type = BR_TRANSFORM_IDENTITY;
    {
        br_light *l      = st->light->type_data;
        l->identifier    = "rendertest-parallax-light";
        l->type          = BR_LIGHT_AMBIENT;
        l->colour        = BR_COLOUR_RGB(255, 255, 255);
        l->attenuation_c = BR_SCALAR(1.0);
    }

    st->actor           = BrActorAdd(st->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    st->actor->t.type   = BR_TRANSFORM_IDENTITY;
    st->actor->model    = st->model;
    st->actor->material = st->material;

    return BRE_OK;
}

/*
 * The expected albedo value at the frame centre, in the map's own units: the shader samples
 * `u - offset` with a NEAREST 256-wide map, so the texel is floor(256 * (0.5 - offset)) and the
 * value written there is the texel index itself. offset is in bump-UV units - both maps'
 * transforms are identity - and is (vTS.x / vTS.z) * height * depth, where vTS is the eye
 * direction at the frame centre in the tangent frame, which for a camera that looks at the
 * quad's centre is the camera's own direction.
 */
static int pxw_expected(br_uint_32 theta_deg, double depth)
{
    const double rad    = (double)theta_deg * 3.14159265358979323846 / 180.0;
    const double height = (double)PXW_HEIGHT / 255.0;
    const double offset = tan(rad) * height * depth;
    int          texel  = (int)floor((double)PXW_MAP * (0.5 + offset));

    if(texel < 0)
        texel = 0;
    if(texel > 255)
        texel = 255;

    return texel;
}

/* Render the quad with the eye at theta about the face normal, offset by depth and map. */
static br_error pxw_render(pxw_state *st, br_uint_32 theta_deg, double depth, br_pixelmap *bump)
{
    br_rectangle whole = {.x = -st->offscreen->origin_x, .y = -st->offscreen->origin_y, .w = PXW_SIZE, .h = PXW_SIZE};
    br_point     dst   = {0, 0};
    br_rectangle src   = whole;
    br_angle     angle = BR_ANGLE_DEG(BR_SCALAR(theta_deg));

    BrMatrix34RotateY(&st->camera->t.t.mat, angle);
    st->camera->t.t.mat.m[3][0] = BR_MUL(BR_SIN(angle), PXW_CAM_DIST);
    st->camera->t.t.mat.m[3][1] = BR_SCALAR(0.0);
    st->camera->t.t.mat.m[3][2] = BR_MUL(BR_COS(angle), PXW_CAM_DIST);
    st->camera->t.t.mat.m[3][3] = BR_SCALAR(1.0);

    st->material->bump_map   = bump;
    st->material->bump_depth = (br_fraction)depth;
    BrMaterialUpdate(st->material, BR_MATU_ALL);

    if(DevicePixelmapRectangleFill(st->offscreen, &whole, BR_COLOUR_RGB(0, 0, 0)) != BRE_OK)
        return BRE_FAIL;

    BrPixelmapFill(st->depth, 0xffffffff);
    BrZbSceneRender(st->world, st->camera, st->offscreen, st->depth);

    return DevicePixelmapRectangleCopyFrom((br_device_pixelmap *)st->offscreen, &dst, (br_device_pixelmap *)st->frame, &src) == BRE_OK
               ? BRE_OK
               : BRE_FAIL;
}

/*
 * The value two of the three colour bytes agree on; the third must be zero. The albedo puts u
 * into two channels, so this is independent of the framebuffer's byte order.
 */
static br_error pxw_probe(pxw_state *st, int *out_value)
{
    const int        x  = PXW_SIZE / 2;
    const int        y  = PXW_SIZE / 2;
    const br_uint_8 *px = (const br_uint_8 *)st->frame->pixels + (br_size_t)y * st->frame->row_bytes + (br_size_t)x * 4;

    for(int i = 0; i < 3; ++i) {
        for(int j = i + 1; j < 3; ++j) {
            int k = 3 - i - j;

            if(px[i] != px[j])
                continue;

            if(px[k] != 0)
                break;

            *out_value = px[i];
            return BRE_OK;
        }
    }

    BrLogError("RT", "parallax witness: the centre read back %02x %02x %02x %02x, so its coordinate is unreadable.", px[0], px[1], px[2], px[3]);
    return BRE_FAIL;
}

static br_boolean pxw_same_frame(pxw_state *st, const br_pixelmap *other)
{
    return memcmp(st->frame->pixels, other->pixels, (size_t)st->frame->row_bytes * PXW_SIZE) == 0;
}

/* How many pixels the last render covered: a black frame is the scene not drawing. */
static br_uint_32 pxw_coverage(pxw_state *st)
{
    br_uint_32 covered = 0;

    for(br_int_32 y = 0; y < PXW_SIZE; ++y) {
        const br_uint_8 *row = (const br_uint_8 *)st->frame->pixels + (br_size_t)y * st->frame->row_bytes;

        for(br_int_32 x = 0; x < PXW_SIZE; ++x) {
            const br_uint_8 *px = row + (br_size_t)x * 4;

            if(px[0] || px[1] || px[2])
                ++covered;
        }
    }

    return covered;
}

static int pxw_run(pxw_state *st)
{
    int          failures   = 0;
    int          base_value = -1;
    int          measured[PXW_SAMPLES];
    br_pixelmap *base_frame;

    base_frame = BrPixelmapAllocate(BR_PMT_RGBA_8888, PXW_SIZE, PXW_SIZE, NULL, BR_PMAF_NORMAL);
    if(base_frame == NULL)
        return 1;
    base_frame->origin_x = base_frame->origin_y = 0;

#define PXW_FAIL(...)                  \
    do {                               \
        BrLogError("RT", __VA_ARGS__); \
        failures++;                    \
    } while(0)

    /* The depth-0 frame with the height map bound: the reference every offset is measured from. */
    if(pxw_render(st, 30, 0.0, st->height) != BRE_OK) {
        BrPixelmapFree(base_frame);
        return 1;
    }
    memcpy(base_frame->pixels, st->frame->pixels, (size_t)st->frame->row_bytes * PXW_SIZE);

    if(pxw_coverage(st) == 0) {
        PXW_FAIL("parallax witness: the scene drew nothing, so no offset can be measured.");
        BrPixelmapFree(base_frame);
        return failures;
    }

    if(pxw_probe(st, &base_value) != BRE_OK) {
        BrPixelmapFree(base_frame);
        return 1;
    }

    BrLogInfo("RT", "parallax witness: at depth 0 the centre reads %d, and the map's own name for that coordinate is %d.", base_value,
              PXW_CENTRE_VALUE);

    if(abs(base_value - PXW_CENTRE_VALUE) > PXW_TOLERANCE)
        PXW_FAIL("parallax witness: at depth 0 the centre reads %d, but the map names %d.", base_value, PXW_CENTRE_VALUE);

    /* 1. A bump map at depth 0 is not a bump map at all. */
    if(pxw_render(st, 30, 0.0, NULL) != BRE_OK) {
        BrPixelmapFree(base_frame);
        return 1;
    }
    if(!pxw_same_frame(st, base_frame))
        PXW_FAIL("parallax witness: binding a bump map at depth 0 changed the frame.");

    /* 2. A height of zero drives nothing, at any depth. */
    if(pxw_render(st, 30, 0.2, st->flat) != BRE_OK) {
        BrPixelmapFree(base_frame);
        return 1;
    }
    if(!pxw_same_frame(st, base_frame))
        PXW_FAIL("parallax witness: a zero-height bump map at depth 0.2 changed the frame.");

    /* 3. The four measurements: the offset itself, against the value the geometry names. */
    for(int i = 0; i < PXW_SAMPLES; ++i) {
        int want = pxw_expected(pxw_samples[i].theta_deg, pxw_samples[i].depth);

        measured[i] = -1;

        if(pxw_render(st, pxw_samples[i].theta_deg, pxw_samples[i].depth, st->height) != BRE_OK) {
            BrPixelmapFree(base_frame);
            return 1;
        }

        if(pxw_probe(st, &measured[i]) != BRE_OK) {
            BrPixelmapFree(base_frame);
            return 1;
        }

        BrLogInfo("RT", "parallax witness: %-22s depth %.1f: measured u-texel %3d, the geometry predicts %3d.", pxw_samples[i].what,
                  pxw_samples[i].depth, measured[i], want);

        if(abs(measured[i] - want) > PXW_TOLERANCE)
            PXW_FAIL("parallax witness: %s at depth %.1f sampled u-texel %d, but the geometry predicts %d.", pxw_samples[i].what,
                     pxw_samples[i].depth, measured[i], want);
    }

    {
        const int delta_30_1 = measured[1] - PXW_CENTRE_VALUE;
        const int delta_30_2 = measured[2] - PXW_CENTRE_VALUE;
        const int delta_60_2 = measured[3] - PXW_CENTRE_VALUE;
        const int delta_0_2  = measured[0] - PXW_CENTRE_VALUE;

        /*
         * 4. The sign. The alpha is a height, so a raised texel is seen at a coordinate
         * nearer the eye: with the eye at +X the sample moves towards +u and the frame is
         * brighter than the unoffset 128. A shader that offsets the other way is dark here,
         * which is the defect this witness exists for.
         */
        if(delta_30_2 <= 0)
            PXW_FAIL("parallax witness: the eye is at +X but the offset did not move the sample towards +u (read %d, unoffset %d).",
                     measured[2], PXW_CENTRE_VALUE);

        /* 5. Linear in depth: twice the depth is twice the offset, to a texel. */
        if(abs(delta_30_2 - 2 * delta_30_1) > 1)
            PXW_FAIL("parallax witness: depth 0.2 moved the sample by %d texels and depth 0.1 by %d, so the offset is not linear in "
                     "depth.",
                     delta_30_2, delta_30_1);

        /* 6. Zero on the axis, and monotonic in obliquity. */
        if(abs(delta_0_2) > PXW_TOLERANCE)
            PXW_FAIL("parallax witness: at normal incidence depth 0.2 moved the sample by %d texels.", delta_0_2);

        if(delta_60_2 <= delta_30_2)
            PXW_FAIL("parallax witness: 60 degrees moved the sample by %d texels and 30 degrees by %d, so obliquity does not "
                     "increase the offset.",
                     delta_60_2, delta_30_2);
    }

    /*
     * 7. The same map with a negated depth, which is how a depth-authored map is described: the
     * offset must mirror about the unoffset co-ordinate, because that is the whole difference
     * between a map whose 1 is nearest and one whose 1 is furthest.
     */
    for(int i = 0; i < PXW_SAMPLES; ++i) {
        int want = pxw_expected(pxw_samples[i].theta_deg, -pxw_samples[i].depth);
        int got  = -1;

        if(pxw_render(st, pxw_samples[i].theta_deg, -pxw_samples[i].depth, st->height) != BRE_OK) {
            BrPixelmapFree(base_frame);
            return 1;
        }

        if(pxw_probe(st, &got) != BRE_OK) {
            BrPixelmapFree(base_frame);
            return 1;
        }

        BrLogInfo("RT", "parallax witness: %-22s depth %.1f: negated, measured u-texel %3d, the geometry predicts %3d.",
                  pxw_samples[i].what, pxw_samples[i].depth, got, want);

        if(abs(got - want) > PXW_TOLERANCE)
            PXW_FAIL("parallax witness: %s at depth %.1f sampled u-texel %d, but a negated depth predicts %d.", pxw_samples[i].what,
                     pxw_samples[i].depth, got, want);
    }

#undef PXW_FAIL

    BrPixelmapFree(base_frame);
    return failures;
}

br_error rt_parallax_witness_run(rt_demo *demo, const char *device)
{
    pxw_state  state    = {0};
    br_boolean hardware = (strcmp(device, "glrend") == 0);
    br_error   r        = BRE_FAIL;
    int        failures;

    /*
     * glrend only, for now. It is the reference device and the one whose shader the offset
     * lives in; softrend cannot shade a bump map at all and glrend1x borrows softrend's
     * transform and lighting, so neither has an offset to measure. Registered for glrend
     * alone in CMakeLists.txt, and this refuses rather than passing vacuously elsewhere.
     */
    if(!hardware) {
        BrLogError("RT", "parallax witness: --parallax-witness asserts the offset on glrend only; %s has none to measure.", device);
        return BRE_FAIL;
    }

    if(pxw_build(&state, demo->_screen) != BRE_OK) {
        BrLogError("RT", "parallax witness: could not build the scene.");
        goto out;
    }

    state.frame = BrPixelmapAllocate(BR_PMT_RGBA_8888, PXW_SIZE, PXW_SIZE, NULL, BR_PMAF_NORMAL);
    if(state.frame == NULL)
        goto out;
    state.frame->origin_x = state.frame->origin_y = 0;

    failures = pxw_run(&state);

    if(failures == 0)
        BrLogInfo("RT", "parallax witness passed (the offset's sign, magnitude, depth linearity and obliquity on glrend).");

    r = failures == 0 ? BRE_OK : BRE_FAIL;

out:
    BrRendererEnd();

    if(state.frame != NULL)
        BrPixelmapFree(state.frame);
    if(state.albedo != NULL)
        BrPixelmapFree(state.albedo);
    if(state.height != NULL)
        BrPixelmapFree(state.height);
    if(state.flat != NULL)
        BrPixelmapFree(state.flat);
    if(state.depth != NULL)
        BrPixelmapFree(state.depth);
    if(state.offscreen != NULL)
        BrPixelmapFree(state.offscreen);
    if(state.material != NULL)
        BrMaterialFree(state.material);
    if(state.model != NULL)
        BrModelFree(state.model);
    if(state.world != NULL)
        BrActorFree(state.world);

    return r;
}
