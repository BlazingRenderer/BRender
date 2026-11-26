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
 *
 * A directional-light pass separately tests normal strength: the ambient pass cannot see
 * a bad shading normal, and the lit pass must still see a valid normal map's tilt.
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
    br_pixelmap *albedo_uv;
    br_pixelmap *height;
    br_pixelmap *flat;
    br_pixelmap *neg_z;
    br_pixelmap *degenerate;
    br_pixelmap *tilt;
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

/* The original one-axis probe identifies u by its two equal channels. */
static br_pixelmap *pxw_make_albedo(br_boolean encode_v)
{
    br_pixelmap *pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, PXW_MAP, PXW_MAP, NULL, BR_PMAF_NORMAL);

    if(pm == NULL)
        return NULL;

    for(br_int_32 ty = 0; ty < PXW_MAP; ++ty)
        for(br_int_32 tx = 0; tx < PXW_MAP; ++tx)
            BrPixelmapPixelSet(pm, tx, ty, BR_COLOUR_RGB(tx, encode_v ? ty : tx, 0));

    BrMapUpdate(pm, BR_MAPU_ALL);
    return pm;
}

static br_pixelmap *pxw_make_bump(br_colour colour)
{
    br_pixelmap *pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, PXW_MAP, PXW_MAP, NULL, BR_PMAF_NORMAL);

    if(pm == NULL)
        return NULL;

    for(br_int_32 ty = 0; ty < PXW_MAP; ++ty)
        for(br_int_32 tx = 0; tx < PXW_MAP; ++tx)
            BrPixelmapPixelSet(pm, tx, ty, colour);

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

    if((st->albedo = pxw_make_albedo(BR_FALSE)) == NULL || (st->albedo_uv = pxw_make_albedo(BR_TRUE)) == NULL)
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
    if((st->height = pxw_make_bump(BR_COLOUR_RGBA(128, 128, 255, PXW_HEIGHT))) == NULL ||
       (st->flat = pxw_make_bump(BR_COLOUR_RGBA(128, 128, 255, 0))) == NULL ||
       (st->neg_z = pxw_make_bump(BR_COLOUR_RGBA(128, 128, 0, PXW_HEIGHT))) == NULL ||
       (st->degenerate = pxw_make_bump(BR_COLOUR_RGBA(128, 128, 128, PXW_HEIGHT))) == NULL ||
       (st->tilt = pxw_make_bump(BR_COLOUR_RGBA(200, 128, 230, PXW_HEIGHT))) == NULL)
        return BRE_FAIL;
    st->material->bump_map   = st->height;
    st->material->bump_scale = BR_SCALAR(1.0);
    st->material->bump_depth = BR_UFRACTION(0.2);
    BrMatrix23Identity(&st->material->map_transform);
    BrMatrix23Identity(&st->material->bump_map_transform);

    {
        static const struct {
            br_scalar x, y, u, v;
        } corners[4] = {
            {-PXW_QUAD, -PXW_QUAD, 0, 0},
            {PXW_QUAD,  -PXW_QUAD, 1, 0},
            {PXW_QUAD,  PXW_QUAD,  1, 1},
            {-PXW_QUAD, PXW_QUAD,  0, 1},
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

    st->world               = BrActorAllocate(BR_ACTOR_NONE, NULL);
    st->camera              = BrActorAdd(st->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    st->camera->t.type      = BR_TRANSFORM_MATRIX34;
    st->camera_data         = st->camera->type_data;
    st->camera_data->aspect = BR_SCALAR(1.0);

    st->light         = BrActorAdd(st->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    st->light->t.type = BR_TRANSFORM_IDENTITY;
    {
        br_light *l      = st->light->type_data;
        l->identifier    = "rendertest-parallax-light";
        l->type          = BR_LIGHT_DIRECT;
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

static br_error pxw_draw(pxw_state *st, double depth, br_pixelmap *bump)
{
    br_rectangle whole = {.x = -st->offscreen->origin_x, .y = -st->offscreen->origin_y, .w = PXW_SIZE, .h = PXW_SIZE};
    br_point     dst   = {0, 0};
    br_rectangle src   = whole;

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

/* Render the quad with the eye at theta about the face normal, offset by depth and map. */
static br_error pxw_render(pxw_state *st, br_uint_32 theta_deg, double depth, br_pixelmap *bump)
{
    br_angle angle = BR_ANGLE_DEG(BR_SCALAR(theta_deg));

    BrMatrix34RotateY(&st->camera->t.t.mat, angle);
    st->camera->t.t.mat.m[3][0] = BR_MUL(BR_SIN(angle), PXW_CAM_DIST);
    st->camera->t.t.mat.m[3][1] = BR_SCALAR(0.0);
    st->camera->t.t.mat.m[3][2] = BR_MUL(BR_COS(angle), PXW_CAM_DIST);

    return pxw_draw(st, depth, bump);
}

/*
 * The value two of the three colour bytes agree on; the third must be zero. The albedo puts u
 * into two channels, so this is independent of the framebuffer's byte order.
 */
static br_error pxw_probe(pxw_state *st, int x_offset, int *out_value)
{
    const int        x  = PXW_SIZE / 2 + x_offset;
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

    BrLogError("RT", "parallax witness: pixel (%d, %d) read back %02x %02x %02x %02x, so its coordinate is unreadable.", x, y, px[0], px[1],
               px[2], px[3]);
    return BRE_FAIL;
}

static br_boolean pxw_same_frame(pxw_state *st, const br_pixelmap *other)
{
    return memcmp(st->frame->pixels, other->pixels, (size_t)st->frame->row_bytes * PXW_SIZE) == 0;
}

/* The largest colour-channel difference between the last frame and `other`, in colour steps. */
static int pxw_frame_delta(pxw_state *st, const br_pixelmap *other)
{
    const br_uint_8 *a     = st->frame->pixels;
    const br_uint_8 *b     = other->pixels;
    int              worst = 0;

    for(br_size_t i = 0; i < (br_size_t)st->frame->row_bytes * PXW_SIZE; ++i) {
        int d = abs((int)a[i] - (int)b[i]);

        if(d > worst)
            worst = d;
    }

    return worst;
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

#define PXW_LINEAR(a, b, c, d)                                                 \
    {                                                                          \
        {                                                                      \
            {BR_SCALAR(a), BR_SCALAR(b)}, {BR_SCALAR(c), BR_SCALAR(d)}, {0, 0} \
        }                                                                      \
    }
#define PXW_IDENTITY PXW_LINEAR(1, 0, 0, 1)

static const struct pxw_metric_case {
    const char *name;
    br_matrix23 map, bump;
    double      depth;
    br_boolean  skip;
} pxw_metric_cases[] = {
    {"identity", PXW_IDENTITY, PXW_IDENTITY, 0.2, BR_FALSE},
    {"anisotropic-4x1", PXW_IDENTITY, PXW_LINEAR(4, 0, 0, 1), 0.2, BR_FALSE},
    {"anisotropic-1x4", PXW_IDENTITY, PXW_LINEAR(1, 0, 0, 4), 0.2, BR_FALSE},
    {"shear", PXW_IDENTITY, PXW_LINEAR(1, 0.6, 0, 1), 0.2, BR_FALSE},
    {"mirrored-base", PXW_LINEAR(-1, 0, 0, 1), PXW_IDENTITY, 0.2, BR_FALSE},
    {"combined", PXW_LINEAR(0.8, 0.3, 0, 1.1), PXW_LINEAR(0.7, -0.5, 0.4, 1.2), 0.2, BR_FALSE},
    /* Reduce depth with the uniform scale so the expected offset is still 30 by 20 texels. */
    {"uniform-bump-2^-10", PXW_IDENTITY, PXW_LINEAR(1.0 / 1024.0, 0, 0, 1.0 / 1024.0), 0.2 / 1024.0, BR_FALSE},
    /* Compensating factors keep the product usable, so the UV derivative guard cannot mask a missing check. */
    {"ill-conditioned-bump", PXW_LINEAR(1, 0, 0, 1000), PXW_LINEAR(1, 0, 0, 3e-5), 0.2, BR_TRUE},
    {"ill-conditioned-base", PXW_LINEAR(1, 0, 0, 3e-5), PXW_LINEAR(1, 0, 0, 1000), 0.2, BR_TRUE},
    {"ill-conditioned-shear", PXW_LINEAR(1, 1, 1, 1.0001), PXW_LINEAR(10001, -10000, -10000, 10000), 0.2, BR_TRUE},
};

static br_boolean pxw_metric_skip(pxw_state *st, br_pixelmap *baseline, const struct pxw_metric_case *test)
{
    if(pxw_draw(st, 0.0, NULL) != BRE_OK)
        return BR_FALSE;

    /* Only u is a stable probe: the compensating v scale can traverse many texels per pixel. */
    const int u = BR_RED(BrPixelmapPixelGet(st->frame, PXW_SIZE / 2, PXW_SIZE / 2));
    if(abs(u - PXW_MAP / 2) > PXW_TOLERANCE) {
        BrLogError("RT", "parallax metric %s: the base map's probe read u %d, expected %d.", test->name, u, PXW_MAP / 2);
        return BR_FALSE;
    }
    memcpy(baseline->pixels, st->frame->pixels, (size_t)st->frame->row_bytes * PXW_SIZE);

    if(pxw_draw(st, test->depth, st->height) != BRE_OK)
        return BR_FALSE;

    const int delta = pxw_frame_delta(st, baseline);
    BrLogInfo("RT", "parallax metric %s: base u %d, maximum frame delta %d steps.", test->name, u, delta);
    if(delta != 0)
        BrLogError("RT", "parallax metric %s: an ill-conditioned transform must leave the no-bump frame unchanged.", test->name);
    return delta == 0;
}

static br_boolean pxw_metric_case_run(pxw_state *st, br_pixelmap *baseline, const struct pxw_metric_case *test, const br_vector2 *probe,
                                      const br_vector3 *eye)
{
    const double depth       = test->depth;
    const double uv_per_unit = 0.5 / BrScalarToFloat(PXW_QUAD);
    br_matrix23 *map         = &st->material->map_transform;
    br_matrix23  bump_uv;
    br_vector2   base, hit, sampled;

    *map = test->map;
    BrMatrix23ApplyP(&base, probe, map);
    map->m[2][0]                     = BR_SCALAR(0.5) - base.v[0];
    map->m[2][1]                     = BR_SCALAR(0.375) - base.v[1];
    st->material->bump_map_transform = test->bump;
    if(test->skip)
        return pxw_metric_skip(st, baseline, test);

    /* The lift is measured in bump UV: one unit of height spans 1/max(|grad u|, |grad v|) in world space. */
    BrMatrix23Mul(&bump_uv, &test->bump, map);
    const double grad_u = uv_per_unit * hypot(BrScalarToFloat(bump_uv.m[0][0]), BrScalarToFloat(bump_uv.m[1][0]));
    const double grad_v = uv_per_unit * hypot(BrScalarToFloat(bump_uv.m[0][1]), BrScalarToFloat(bump_uv.m[1][1]));
    const double h      = PXW_HEIGHT / 255.0 * depth / fmax(grad_u, grad_v);
    for(int i = 0; i < 2; ++i)
        hit.v[i] = probe->v[i] + BR_SCALAR(uv_per_unit * h * BrScalarToFloat(eye->v[i]) / BrScalarToFloat(eye->v[2]));
    BrMatrix23ApplyP(&sampled, &hit, map);
    if(!(sampled.v[0] > 0 && sampled.v[0] < BR_SCALAR(1) && sampled.v[1] > 0 && sampled.v[1] < BR_SCALAR(1))) {
        BrLogError("RT", "parallax metric %s: predicted sample leaves the gradient texture.", test->name);
        return BR_FALSE;
    }

    if(pxw_draw(st, 0.0, st->height) != BRE_OK) {
        BrLogError("RT", "parallax metric %s: could not capture the depth-zero frame.", test->name);
        return BR_FALSE;
    }
    br_colour before = BrPixelmapPixelGet(st->frame, PXW_SIZE / 2, PXW_SIZE / 2);
    memcpy(baseline->pixels, st->frame->pixels, (size_t)st->frame->row_bytes * PXW_SIZE);
    if(pxw_draw(st, 0.0, NULL) != BRE_OK || !pxw_same_frame(st, baseline)) {
        BrLogError("RT", "parallax metric %s: binding the bump map changed the depth-zero frame.", test->name);
        return BR_FALSE;
    }

    if(pxw_draw(st, depth, st->height) != BRE_OK) {
        BrLogError("RT", "parallax metric %s: could not capture the displaced frame.", test->name);
        return BR_FALSE;
    }
    const br_colour after  = BrPixelmapPixelGet(st->frame, PXW_SIZE / 2, PXW_SIZE / 2);
    const int       want_u = (int)floor(PXW_MAP * BrScalarToFloat(sampled.v[0]));
    const int       want_v = (int)floor(PXW_MAP * BrScalarToFloat(sampled.v[1]));
    const int       base_u = PXW_MAP / 2, base_v = PXW_MAP * 3 / 8;
    const int       got_u = BR_RED(after), got_v = BR_GRN(after);
    const int       before_u = BR_RED(before), before_v = BR_GRN(before);

    BrLogInfo("RT", "parallax metric %s: (%d, %d) -> (%d, %d), predicted (%d, %d) -> (%d, %d); delta (%+d, %+d), expected (%+.2f, %+.2f) texels.",
              test->name, before_u, before_v, got_u, got_v, base_u, base_v, want_u, want_v, got_u - before_u, got_v - before_v,
              PXW_MAP * BrScalarToFloat(sampled.v[0]) - base_u, PXW_MAP * BrScalarToFloat(sampled.v[1]) - base_v);

    if(abs(before_u - base_u) > PXW_TOLERANCE || abs(before_v - base_v) > PXW_TOLERANCE || abs(got_u - want_u) > PXW_TOLERANCE ||
       abs(got_v - want_v) > PXW_TOLERANCE) {
        BrLogError("RT", "parallax metric %s: the sampled UV missed the ray-plane prediction by more than %d texels.", test->name, PXW_TOLERANCE);
        return BR_FALSE;
    }

    return BR_TRUE;
}

static int pxw_metric_run(pxw_state *st, br_pixelmap *baseline)
{
    const br_vector3 eye_dir = {
        {BR_SCALAR(0.6), BR_SCALAR(0.4), BR_SCALAR(0.8)}
    };
    const br_vector3 target = {
        {BR_SCALAR(0.4), BR_SCALAR(0.2), 0}
    };
    const br_scalar  uv_per_unit = BR_DIV(BR_SCALAR(0.5), PXW_QUAD);
    const br_vector2 probe       = {
        {BR_SCALAR(0.5) + BR_MUL(target.v[0], uv_per_unit), BR_SCALAR(0.5) + BR_MUL(target.v[1], uv_per_unit)}
    };
    br_vector3    eye, path;
    br_transform *camera   = &st->camera->t;
    int           failures = 0;

    camera->type = BR_TRANSFORM_LOOK_UP;
    BrVector3Normalise(&eye, &eye_dir);
    BrVector3Scale(&path, &eye, PXW_CAM_DIST);
    BrVector3Add(&camera->t.look_up.t, &target, &path);
    BrVector3Negate(&camera->t.look_up.look, &eye);
    BrVector3Set(&camera->t.look_up.up, 0, BR_SCALAR(1), 0);
    st->camera_data->type    = BR_CAMERA_PERSPECTIVE;
    st->material->colour_map = st->albedo_uv;

    for(br_size_t i = 0; i < BR_ASIZE(pxw_metric_cases); ++i)
        if(!pxw_metric_case_run(st, baseline, &pxw_metric_cases[i], &probe, &eye_dir))
            failures++;

    return failures;
}

/* With the light along the normal and kd = 1, the unperturbed quad reads exactly its albedo. */
static int pxw_shading_run(pxw_state *st, br_pixelmap *baseline)
{
    const struct pxw_shading_case {
        const char  *what;
        br_pixelmap *map;
        br_scalar    scale;
        int          tolerance;
        br_boolean   tilted;
    } cases[] = {
        {"flat normal",       st->height,     BR_SCALAR(1), 2, BR_FALSE},
        {"negative-z normal", st->neg_z,      0,            0, BR_FALSE},
        {"tilted normal",     st->tilt,       BR_SCALAR(1), 0, BR_TRUE },
        {"tilted normal",     st->tilt,       0,            0, BR_FALSE},
        {"RGB8 zero vector",  st->degenerate, BR_SCALAR(1), 2, BR_FALSE},
    };
    int failures = 0;
    int value;

    BrLightEnable(st->light);
    st->material->ka         = BR_SCALAR(0.0);
    st->material->kd         = BR_SCALAR(1.0);
    st->material->ks         = BR_SCALAR(0.0);
    st->material->mode       = (st->material->mode & ~BR_MATM_SHADING_MODE_MASK) | BR_MATM_SHADING_MODE_PHONG;
    st->material->colour_map = st->albedo;
    BrMatrix23Identity(&st->material->map_transform);
    BrMatrix23Identity(&st->material->bump_map_transform);

    st->camera->t.type    = BR_TRANSFORM_MATRIX34;
    st->camera_data->type = BR_CAMERA_PERSPECTIVE;
    if(pxw_render(st, 0, 0.0, NULL) != BRE_OK)
        return 1;

    memcpy(baseline->pixels, st->frame->pixels, (size_t)st->frame->row_bytes * PXW_SIZE);

    if(pxw_coverage(st) == 0) {
        BrLogError("RT", "normal witness: the lit scene drew nothing.");
        return 1;
    }

    if(pxw_probe(st, 0, &value) != BRE_OK)
        return 1;

    BrLogInfo("RT", "normal witness: the lit frame covers %u pixels and its centre reads %d, where the albedo names %d.", pxw_coverage(st),
              value, PXW_CENTRE_VALUE);

    if(abs(value - PXW_CENTRE_VALUE) > PXW_TOLERANCE) {
        BrLogError("RT", "normal witness: the lit centre reads %d, not the albedo's %d, so the light is not at full diffuse.", value,
                   PXW_CENTRE_VALUE);
        failures++;
    }

    for(br_size_t i = 0; i < BR_ASIZE(cases); ++i) {
        st->material->bump_scale = cases[i].scale;

        if(pxw_render(st, 0, 0.0, cases[i].map) != BRE_OK || pxw_probe(st, 0, &value) != BRE_OK)
            return 1;

        const int delta = pxw_frame_delta(st, baseline);
        BrLogInfo("RT", "normal witness: %s at strength %g: centre %d, maximum frame delta %d steps.", cases[i].what,
                  BrScalarToFloat(cases[i].scale), value, delta);

        if(cases[i].tilted ? value >= PXW_CENTRE_VALUE - 8 : delta > cases[i].tolerance) {
            BrLogError("RT", "normal witness: %s at strength %g did not %s.", cases[i].what, BrScalarToFloat(cases[i].scale),
                       cases[i].tilted ? "darken the probe" : "preserve unperturbed lighting");
            failures++;
        }
    }

    return failures;
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

    if(pxw_probe(st, 0, &base_value) != BRE_OK) {
        BrPixelmapFree(base_frame);
        return 1;
    }

    BrLogInfo("RT", "parallax witness: at depth 0 the centre reads %d, and the map's own name for that coordinate is %d.", base_value,
              PXW_CENTRE_VALUE);

    if(abs(base_value - PXW_CENTRE_VALUE) > PXW_TOLERANCE)
        PXW_FAIL("parallax witness: at depth 0 the centre reads %d, but the map names %d.", base_value, PXW_CENTRE_VALUE);

    /* 1. At depth 0 a bump map must not displace the albedo. */
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

        if(pxw_probe(st, 0, &measured[i]) != BRE_OK) {
            BrPixelmapFree(base_frame);
            return 1;
        }

        BrLogInfo("RT", "parallax witness: %-22s depth %.1f: measured u-texel %3d, the geometry predicts %3d.", pxw_samples[i].what,
                  pxw_samples[i].depth, measured[i], want);

        if(abs(measured[i] - want) > PXW_TOLERANCE)
            PXW_FAIL("parallax witness: %s at depth %.1f sampled u-texel %d, but the geometry predicts %d.", pxw_samples[i].what,
                     pxw_samples[i].depth, measured[i], want);
    }

    /* A zero strength must not disable the height channel. */
    {
        int zero_strength = -1;

        st->material->bump_scale = BR_SCALAR(0.0);
        if(pxw_render(st, 30, 0.2, st->tilt) != BRE_OK || pxw_probe(st, 0, &zero_strength) != BRE_OK) {
            BrPixelmapFree(base_frame);
            return 1;
        }
        st->material->bump_scale = BR_SCALAR(1.0);

        BrLogInfo("RT",
                  "parallax witness: zero normal strength, depth 0.2 at 30 degrees: measured u-texel %d, full strength read %d and the "
                  "geometry predicts %d.",
                  zero_strength, measured[2], pxw_expected(30, 0.2));

        if(abs(zero_strength - measured[2]) > PXW_TOLERANCE || abs(zero_strength - base_value) <= PXW_TOLERANCE)
            PXW_FAIL(
                "parallax witness: at zero normal strength the sample read %d where the same depth and alpha read %d at full strength, so "
                "zero strength switched off the offset along with the normal.",
                zero_strength, measured[2]);
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

        if(pxw_probe(st, 0, &got) != BRE_OK) {
            BrPixelmapFree(base_frame);
            return 1;
        }

        BrLogInfo("RT", "parallax witness: %-22s depth %.1f: negated, measured u-texel %3d, the geometry predicts %3d.",
                  pxw_samples[i].what, pxw_samples[i].depth, got, want);

        if(abs(got - want) > PXW_TOLERANCE)
            PXW_FAIL("parallax witness: %s at depth %.1f sampled u-texel %d, but a negated depth predicts %d.", pxw_samples[i].what,
                     pxw_samples[i].depth, got, want);
    }

    /* A singular surface transform must still sample its translation, with or without a bump map. */
    BrMatrix23Scale(&st->material->map_transform, BR_SCALAR(0.0), BR_SCALAR(0.0));
    st->material->map_transform.m[2][0] = BR_SCALAR(0.5);
    st->material->map_transform.m[2][1] = BR_SCALAR(0.5);

    if(pxw_render(st, 30, 0.0, NULL) != BRE_OK || pxw_probe(st, 0, &base_value) != BRE_OK) {
        BrPixelmapFree(base_frame);
        return 1;
    }
    if(base_value != PXW_CENTRE_VALUE)
        PXW_FAIL("parallax witness: singular mapping without a bump map read %d instead of %d.", base_value, PXW_CENTRE_VALUE);

    memcpy(base_frame->pixels, st->frame->pixels, (size_t)st->frame->row_bytes * PXW_SIZE);
    for(int i = 0; i < 2; ++i) {
        const double depth = i * 0.2;

        if(pxw_render(st, 30, depth, st->height) != BRE_OK || !pxw_same_frame(st, base_frame))
            PXW_FAIL("parallax witness: binding a bump map at depth %.1f changed a singularly mapped frame.", depth);
    }
    BrMatrix23Identity(&st->material->map_transform);

    /* Every ray of a face-on parallel camera hits along the normal, even off-axis. */
    st->camera_data->type   = BR_CAMERA_PARALLEL;
    st->camera_data->width  = BR_SCALAR(3.0);
    st->camera_data->height = BR_SCALAR(3.0);

    int parallel_value;
    if(pxw_render(st, 0, 0.0, st->height) != BRE_OK || pxw_probe(st, 64, &base_value) != BRE_OK) {
        BrPixelmapFree(base_frame);
        return 1;
    }
    memcpy(base_frame->pixels, st->frame->pixels, (size_t)st->frame->row_bytes * PXW_SIZE);

    if(pxw_render(st, 0, 0.2, st->height) != BRE_OK || pxw_probe(st, 64, &parallel_value) != BRE_OK) {
        BrPixelmapFree(base_frame);
        return 1;
    }
    BrLogInfo("RT", "parallax witness: face-on parallel camera, off-axis u at depths 0 and 0.2: %d, %d.", base_value, parallel_value);
    if(base_value <= PXW_CENTRE_VALUE || !pxw_same_frame(st, base_frame))
        PXW_FAIL("parallax witness: depth 0.2 changed the face-on parallel frame or the off-axis probe did not hit the quad.");

    failures += pxw_metric_run(st, base_frame);
    failures += pxw_shading_run(st, base_frame);

#undef PXW_FAIL

    BrPixelmapFree(base_frame);
    return failures;
}

br_error rt_parallax_witness_run(rt_demo *demo, const char *device)
{
    pxw_state state = {0};
    br_error  r     = BRE_FAIL;
    int       failures;

    if(strcmp(device, "glrend") != 0 && strcmp(device, "vkrend") != 0) {
        BrLogError("RT", "parallax witness: %s has no shader that applies the offset.", device);
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
        BrLogInfo("RT", "parallax witness passed on %s (parallax offset and normal strength).", device);

    r = failures == 0 ? BRE_OK : BRE_FAIL;

out:
    if(state.light != NULL)
        BrLightDisable(state.light);
    BrRendererEnd();

    if(state.frame != NULL)
        BrPixelmapFree(state.frame);
    if(state.albedo != NULL)
        BrPixelmapFree(state.albedo);
    if(state.albedo_uv != NULL)
        BrPixelmapFree(state.albedo_uv);
    if(state.height != NULL)
        BrPixelmapFree(state.height);
    if(state.flat != NULL)
        BrPixelmapFree(state.flat);
    if(state.neg_z != NULL)
        BrPixelmapFree(state.neg_z);
    if(state.degenerate != NULL)
        BrPixelmapFree(state.degenerate);
    if(state.tilt != NULL)
        BrPixelmapFree(state.tilt);
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
