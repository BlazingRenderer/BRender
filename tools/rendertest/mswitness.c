/*
 * The multisample witness: one scene drawn into a single-sampled offscreen and into a
 * multisampled one, then compared pixel by pixel.
 *
 * A stored reference cannot carry this. The resolve is the driver's own arithmetic, so a
 * blessed row would encode whatever it produced and compare the driver with itself. What is
 * asserted instead is the property multisampling exists for: a shape edge that crosses a
 * pixel is covered in proportion, so the multisampled frame holds levels between the shape's
 * colour and the background where the single-sampled frame holds only those two - and away
 * from the edges the two frames must agree, because a renderer that changes anything else has
 * changed the picture rather than its edges.
 *
 * The shape is a white quad on a black background, unlit and untextured, so a pixel's red
 * channel is its coverage and nothing else contributes to it. It is rotated so its edges cut
 * across the sample grid diagonally rather than landing on it.
 */
#include <brender.h>
#include <brddi.h>

#include "rt_demo.h"
#include "mswitness.h"

#define MSW_SIZE     64
#define MSW_SAMPLES  4
#define MSW_HALF_EXT BR_SCALAR(6.0)
#define MSW_CAM_DIST BR_SCALAR(16.0)
#define MSW_ROTATION BR_ANGLE_DEG(17)
#define MSW_BG       0x00
#define MSW_SHAPE    0xff
#define MSW_MID      0x40
#define MSW_COPY     0x80
#define MSW_SUB      0xc0
#define MSW_LINE_Y   4

typedef struct msw_state {
    br_pixelmap *colour;
    br_pixelmap *depth;
    br_pixelmap *colour_ms;
    br_pixelmap *depth_ms;
    br_actor    *world;
    br_actor    *camera;
    br_actor    *actor;
    br_camera   *camera_data;
    br_material *material;
    br_model    *model;
    br_int_32    samples;
    br_int_32    device_max;
} msw_state;

static br_error msw_match_colour(br_pixelmap *screen, br_int_32 samples, br_pixelmap **out)
{
    br_token_value tv[] = {
        {.t = BRT_WIDTH_I32,        .v = {.i32 = MSW_SIZE}                  },
        {.t = BRT_HEIGHT_I32,       .v = {.i32 = MSW_SIZE}                  },
        {.t = BRT_PIXEL_TYPE_U8,    .v = {.u8 = (br_uint_8)BR_PMT_RGBA_8888}},
        {.t = BRT_USE_T,            .v = {.t = BRT_OFFSCREEN}               },
        {.t = BRT_MSAA_SAMPLES_I32, .v = {.i32 = samples}                   },
        {.t = BR_NULL_TOKEN,        .v = {0}                                },
    };

    if((*out = BrPixelmapMatchTV(screen, tv)) == NULL)
        return BRE_FAIL;

    /* The core projects the scene about this origin, so it is the frame's centre. */
    (*out)->origin_x = (*out)->origin_y = (br_int_16)(MSW_SIZE / 2);
    return BRE_OK;
}

static br_error msw_match_depth(br_pixelmap *colour, br_pixelmap **out)
{
    br_token_value tv[] = {
        {.t = BRT_USE_T,          .v = {.t = BRT_DEPTH}},
        {.t = BRT_PIXEL_BITS_I32, .v = {.i32 = 16}     },
        {.t = BR_NULL_TOKEN,      .v = {0}             },
    };

    if((*out = BrPixelmapMatchTV(colour, tv)) == NULL)
        return BRE_FAIL;

    (*out)->origin_x = (*out)->origin_y = (br_int_16)(MSW_SIZE / 2);
    return BRE_OK;
}

static br_error msw_build(msw_state *st, br_pixelmap *screen)
{
    st->world          = BrActorAllocate(BR_ACTOR_NONE, NULL);
    st->camera         = BrActorAdd(st->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    st->camera->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&st->camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), MSW_CAM_DIST);

    /*
     * A 90-degree field of view makes the quad cover MSW_SIZE/(2 * MSW_CAM_DIST) of the
     * frame per unit of its own half extent.
     */
    st->camera_data                = st->camera->type_data;
    st->camera_data->field_of_view = BR_ANGLE_DEG(90);
    st->camera_data->hither_z      = BR_SCALAR(1.0);
    st->camera_data->yon_z         = BR_SCALAR(64.0);
    st->camera_data->aspect        = BR_SCALAR(1.0);

    st->material = BrMaterialAllocate("rendertest-ms-witness");
    st->model    = BrModelAllocate("rendertest-ms-witness", 4, 2);

    if(st->material == NULL || st->model == NULL) {
        BrLogError("RT", "ms witness: could not allocate the material or the model.");
        return BRE_FAIL;
    }

    st->material->colour  = BR_COLOUR_RGB(MSW_SHAPE, MSW_SHAPE, MSW_SHAPE);
    st->material->opacity = 255;
    st->material->flags &= ~BR_MATF_LIGHT;
    BrMatrix23Identity(&st->material->map_transform);
    BrMaterialUpdate(st->material, BR_MATU_ALL);

    BrVector3Set(&st->model->vertices[0].p, -MSW_HALF_EXT, -MSW_HALF_EXT, BR_SCALAR(0.0));
    BrVector3Set(&st->model->vertices[1].p, MSW_HALF_EXT, -MSW_HALF_EXT, BR_SCALAR(0.0));
    BrVector3Set(&st->model->vertices[2].p, MSW_HALF_EXT, MSW_HALF_EXT, BR_SCALAR(0.0));
    BrVector3Set(&st->model->vertices[3].p, -MSW_HALF_EXT, MSW_HALF_EXT, BR_SCALAR(0.0));

    st->model->flags |= BR_MODF_CUSTOM_NORMALS;
    for(int i = 0; i < 4; ++i) {
        st->model->vertices[i].n.v[0] = 0;
        st->model->vertices[i].n.v[1] = 0;
        st->model->vertices[i].n.v[2] = BrScalarToFraction(BR_SCALAR(1.0));
    }

    st->model->faces[0].vertices[0] = 0;
    st->model->faces[0].vertices[1] = 1;
    st->model->faces[0].vertices[2] = 2;
    st->model->faces[0].material    = st->material;

    st->model->faces[1].vertices[0] = 0;
    st->model->faces[1].vertices[1] = 2;
    st->model->faces[1].vertices[2] = 3;
    st->model->faces[1].material    = st->material;

    BrModelUpdate(st->model, BR_MODU_ALL);

    st->actor         = BrActorAdd(st->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    st->actor->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34RotateZ(&st->actor->t.t.mat, MSW_ROTATION);
    st->actor->model    = st->model;
    st->actor->material = st->material;

    /*
     * Both pairs are matched, the second with the token in it, so what the device reports for
     * the count is what it really allocated rather than what was asked for.
     */
    if(msw_match_colour(screen, 0, &st->colour) != BRE_OK || msw_match_depth(st->colour, &st->depth) != BRE_OK) {
        BrLogError("RT", "ms witness: could not match the single-sampled %dx%d RGBA_8888 pair.", MSW_SIZE, MSW_SIZE);
        return BRE_FAIL;
    }

    if(msw_match_colour(screen, MSW_SAMPLES, &st->colour_ms) != BRE_OK || msw_match_depth(st->colour_ms, &st->depth_ms) != BRE_OK) {
        BrLogError("RT", "ms witness: could not match the multisampled %dx%d RGBA_8888 pair.", MSW_SIZE, MSW_SIZE);
        return BRE_FAIL;
    }

    {
        br_int_32  samples    = 0;
        br_object *facility   = NULL;
        br_int_32  device_max = 0;

        if(ObjectQuery(st->colour_ms, &samples, BRT_MSAA_SAMPLES_I32) != BRE_OK) {
            BrLogError("RT", "ms witness: the matched pixelmap does not answer BRT_MSAA_SAMPLES_I32.");
            return BRE_FAIL;
        }

        if(ObjectQuery(screen, &facility, BRT_OUTPUT_FACILITY_O) != BRE_OK || facility == NULL ||
           ObjectQuery(facility, &device_max, BRT_MSAA_SAMPLES_I32) != BRE_OK) {
            BrLogError("RT", "ms witness: the output facility does not report what the device can do.");
            return BRE_FAIL;
        }

        /*
         * The match clamps to what the device and the format can do, so a count below what
         * was asked for is legitimate only when the device cannot do better. Asking the
         * facility is what separates the two: without it a driver that ignored the token and
         * a machine with no multisample support would both report the same number.
         */
        if(device_max >= MSW_SAMPLES && samples < MSW_SAMPLES) {
            BrLogError("RT", "ms witness: the device offers %d samples but the match returned %d.", device_max, samples);
            return BRE_FAIL;
        }

        st->samples    = samples;
        st->device_max = device_max;
    }

    return BRE_OK;
}

/*
 * Render the scene into one target and read it back into a memory pixelmap. The renderer is
 * bound to the destination first: a material's stored state is built against the renderer's
 * output, and the two targets are separate pixelmaps.
 */
static void msw_begin(br_pixelmap *colour)
{
    static char primitive_heap[256 * 1024];

    BrRendererEnd();
    BrRendererBegin(colour, NULL, NULL, primitive_heap, sizeof(primitive_heap));
}

static br_pixelmap *msw_read(const br_pixelmap *colour)
{
    br_pixelmap       *readback;
    const br_rectangle src = {
        .x = -colour->origin_x,
        .y = -colour->origin_y,
        .w = colour->width,
        .h = colour->height,
    };
    const br_point dst = {0, 0};

    if((readback = BrPixelmapAllocate(BR_PMT_RGBA_8888, colour->width, colour->height, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    if(DevicePixelmapRectangleCopyFrom((br_device_pixelmap *)colour, (br_point *)&dst, (br_device_pixelmap *)readback, (br_rectangle *)&src) !=
       BRE_OK) {
        BrLogError("RT", "ms witness: could not read %s back.", colour->identifier);
        BrPixelmapFree(readback);
        return NULL;
    }

    return readback;
}

/*
 * The red channel of a pixel, which is what the witness' colours vary in.
 */
static br_uint_8 msw_pixel(const br_pixelmap *pm, br_int_32 x, br_int_32 y)
{
    return *((const br_uint_8 *)pm->pixels + (br_size_t)y * pm->row_bytes + (br_size_t)x * 4 + 2);
}

static br_pixelmap *msw_render(msw_state *st, br_pixelmap *colour, br_pixelmap *depth)
{
    const br_rectangle whole = {
        .x = -colour->origin_x,
        .y = -colour->origin_y,
        .w = colour->width,
        .h = colour->height,
    };

    msw_begin(colour);

    if(DevicePixelmapRectangleFill((br_device_pixelmap *)colour, (br_rectangle *)&whole, BR_COLOUR_RGB(MSW_BG, MSW_BG, MSW_BG)) != BRE_OK) {
        BrLogError("RT", "ms witness: could not clear %s.", colour->identifier);
        return NULL;
    }

    BrZbSceneRender(st->world, st->camera, colour, depth);
    return msw_read(colour);
}

typedef struct msw_census {
    br_int_32 background;
    br_int_32 shape;
    br_int_32 partial;
} msw_census;

static msw_census msw_count(const br_pixelmap *pm)
{
    msw_census       c      = {0};
    const br_uint_8 *pixels = pm->pixels;

    for(br_int_32 y = 0; y < pm->height; ++y) {
        const br_uint_8 *row = pixels + (br_size_t)y * pm->row_bytes;

        for(br_int_32 x = 0; x < pm->width; ++x) {
            /* The map is RGBA_8888, so px[0] is blue and px[2] red; the material is grey. */
            const br_uint_8 v = row[x * 4 + 2];

            if(v == MSW_BG)
                ++c.background;
            else if(v == MSW_SHAPE)
                ++c.shape;
            else
                ++c.partial;
        }
    }

    return c;
}

static br_error msw_check_coverage(msw_state *st)
{
    br_pixelmap *one = msw_render(st, st->colour, st->depth);
    br_pixelmap *many;
    msw_census   c1, c4;
    br_int_32    differ = 0;
    br_error     r      = BRE_FAIL;

    if(one == NULL)
        return BRE_FAIL;

    if((many = msw_render(st, st->colour_ms, st->depth_ms)) == NULL) {
        BrPixelmapFree(one);
        return BRE_FAIL;
    }

    c1 = msw_count(one);
    c4 = msw_count(many);

    BrLogInfo("RT", "ms witness: at one sample %d background, %d covered and %d partial pixels; at %d samples %d, %d and %d.",
              c1.background, c1.shape, c1.partial, st->samples, c4.background, c4.shape, c4.partial);

    /*
     * A single-sampled frame has no partial coverage at all: a pixel is inside the quad or it
     * is not. This is the assertion that fails against a driver that accepts the token and
     * draws the shape once per pixel anyway.
     */
    if(c1.partial != 0) {
        BrLogError("RT", "ms witness: the single-sampled frame has %d partial pixels, and cannot have any.", c1.partial);
        goto out;
    }

    if(c1.shape == 0) {
        BrLogError("RT", "ms witness: the single-sampled frame does not contain the quad at all; nothing was drawn.");
        goto out;
    }

    /*
     * The quad covers the same area either way, so the multisampled frame must hold some
     * pixels that are neither the shape nor the background, and its covered count must be
     * close to the single-sampled one - a frame that resolved the wrong thing would not be.
     */
    if(c4.partial == 0) {
        BrLogError("RT", "ms witness: the %d-sample frame has no partial coverage, so the samples were not resolved.", st->samples);
        goto out;
    }

    {
        const br_int_32 delta = (c4.shape > c1.shape) ? (c4.shape - c1.shape) : (c1.shape - c4.shape);
        const br_int_32 slack = (c1.shape * 10) / 100;

        if(delta > slack) {
            BrLogError("RT", "ms witness: the frames cover %d and %d pixels, more than a tenth apart.", c1.shape, c4.shape);
            goto out;
        }
    }

    /*
     * Away from the edges the two frames must be identical: multisampling changes coverage,
     * not shading. A resolved image that differs in the interior has lost the shape's colour
     * or its background, which is a different defect from the one above and worth its own
     * sentence in the log.
     */
    {
        const br_uint_8 *a      = one->pixels;
        const br_uint_8 *b      = many->pixels;
        br_int_32        inside = 0;

        for(br_int_32 y = 0; y < one->height; ++y) {
            const br_uint_8 *ra = a + (br_size_t)y * one->row_bytes;
            const br_uint_8 *rb = b + (br_size_t)y * many->row_bytes;

            for(br_int_32 x = 0; x < one->width; ++x) {
                if(ra[x * 4 + 2] != rb[x * 4 + 2])
                    ++differ;
            }
        }

        /*
         * Only edge pixels may differ, and an edge is a small part of a 64x64 frame: a
         * quarter of it is a generous ceiling that still fails a frame drawn at the wrong
         * sample positions or resolved from the wrong image.
         */
        inside = (one->width * one->height) / 4;

        BrLogInfo("RT", "ms witness: %d pixels differ between the frames.", differ);

        if(differ > inside) {
            BrLogError("RT", "ms witness: %d of %d pixels differ, more than the edges can account for.", differ, one->width * one->height);
            goto out;
        }
    }

    r = BRE_OK;

out:
    BrPixelmapFree(many);
    BrPixelmapFree(one);
    return r;
}

/*
 * The same multisampled target driven through the operations a frame is actually made of:
 * a fill, a scene, an ordinary rectangle copy and a sub-rectangle fill, each in its own
 * rendering scope. What every step asserts is that the target kept what the previous one
 * left, which a scope that cleared its samples instead of loading them, or a copy that the
 * driver refused, would show as a wrong colour here rather than as a subtle edge.
 */
static br_error msw_check_scopes(msw_state *st)
{
    const br_rectangle whole = {
        .x = -st->colour_ms->origin_x,
        .y = -st->colour_ms->origin_y,
        .w = st->colour_ms->width,
        .h = st->colour_ms->height,
    };
    const br_int_32    ox  = st->colour_ms->origin_x;
    const br_int_32    oy  = st->colour_ms->origin_y;
    const br_rectangle sub = {
        .x = 4 - ox,
        .y = 4 - oy,
        .w = 8,
        .h = 8,
    };
    br_pixelmap *readback = NULL;
    br_pixelmap *host     = NULL;
    br_error     r        = BRE_FAIL;

    /* A fill through one scope. */
    if(DevicePixelmapRectangleFill((br_device_pixelmap *)st->colour_ms, (br_rectangle *)&whole, BR_COLOUR_RGB(MSW_MID, MSW_MID, MSW_MID)) != BRE_OK) {
        BrLogError("RT", "ms witness: could not fill the multisampled target.");
        goto out;
    }

    if((readback = msw_read(st->colour_ms)) == NULL)
        goto out;

    if(msw_pixel(readback, 0, 0) != MSW_MID || msw_pixel(readback, MSW_SIZE / 2, MSW_SIZE / 2) != MSW_MID) {
        BrLogError("RT", "ms witness: the whole-image fill read %02x, expected %02x.", msw_pixel(readback, 0, 0), MSW_MID);
        goto out;
    }

    BrPixelmapFree(readback);
    readback = NULL;

    /*
     * A scene through a second scope. The quad covers the middle of the frame and leaves
     * the corners alone, so a corner still holding the fill is what says the scope loaded
     * the samples it started with rather than clearing them.
     */
    msw_begin(st->colour_ms);
    BrZbSceneRender(st->world, st->camera, st->colour_ms, st->depth_ms);

    if((readback = msw_read(st->colour_ms)) == NULL)
        goto out;

    if(msw_pixel(readback, 0, 0) != MSW_MID) {
        BrLogError("RT", "ms witness: the scene lost the previous scope's fill; the corner read %02x, expected %02x.",
                   msw_pixel(readback, 0, 0), MSW_MID);
        goto out;
    }

    if(msw_pixel(readback, MSW_SIZE / 2, MSW_SIZE / 2) != MSW_SHAPE) {
        BrLogError("RT", "ms witness: the second scope did not draw the quad; the centre read %02x.",
                   msw_pixel(readback, MSW_SIZE / 2, MSW_SIZE / 2));
        goto out;
    }

    BrPixelmapFree(readback);
    readback = NULL;

    /* An ordinary rectangle copy into the target, from host memory. */
    if((host = BrPixelmapAllocate(BR_PMT_RGBA_8888, MSW_SIZE, MSW_SIZE, NULL, BR_PMAF_NORMAL)) == NULL)
        goto out;

    for(br_int_32 y = 0; y < MSW_SIZE; ++y)
        for(br_int_32 x = 0; x < MSW_SIZE; ++x)
            BrPixelmapPixelSet(host, x, y, BR_COLOUR_RGB(MSW_COPY, MSW_COPY, MSW_COPY));

    BrPixelmapRectangleCopy(st->colour_ms, -ox, -oy, host, 0, 0, MSW_SIZE, MSW_SIZE);

    if((readback = msw_read(st->colour_ms)) == NULL)
        goto out;

    if(msw_pixel(readback, MSW_SIZE / 2, MSW_SIZE / 2) != MSW_COPY) {
        BrLogError("RT", "ms witness: the rectangle copy into the multisampled target read %02x, expected %02x.",
                   msw_pixel(readback, MSW_SIZE / 2, MSW_SIZE / 2), MSW_COPY);
        goto out;
    }

    BrPixelmapFree(readback);
    readback = NULL;

    /* A sub-rectangle fill, which must not disturb what surrounds it. */
    if(DevicePixelmapRectangleFill((br_device_pixelmap *)st->colour_ms, (br_rectangle *)&sub, BR_COLOUR_RGB(MSW_SUB, MSW_SUB, MSW_SUB)) != BRE_OK) {
        BrLogError("RT", "ms witness: could not fill a sub-rectangle of the multisampled target.");
        goto out;
    }

    if((readback = msw_read(st->colour_ms)) == NULL)
        goto out;

    if(msw_pixel(readback, 5, 5) != MSW_SUB) {
        BrLogError("RT", "ms witness: the sub-rectangle fill read %02x, expected %02x.", msw_pixel(readback, 5, 5), MSW_SUB);
        goto out;
    }

    if(msw_pixel(readback, MSW_SIZE - 1, MSW_SIZE - 1) != MSW_COPY) {
        BrLogError("RT", "ms witness: the sub-rectangle fill disturbed the rest of the target; a far corner read %02x, expected %02x.",
                   msw_pixel(readback, MSW_SIZE - 1, MSW_SIZE - 1), MSW_COPY);
        goto out;
    }

    r = BRE_OK;

out:
    if(host != NULL)
        BrPixelmapFree(host);

    if(readback != NULL)
        BrPixelmapFree(readback);

    return r;
}

/*
 * The immediate-mode paths - a line and a run of text - drawn into the multisampled target
 * and then covered by a later scene scope.
 *
 * The scene does not reach the rows they are drawn in, so those rows must come back
 * bit-identical: the same samples, resolved twice. A line or a glyph that went into the
 * resolved image instead would be erased by the second resolve, and one that went into the
 * samples but was never resolved would appear only at the second, so neither direction of
 * that mistake passes here.
 *
 * The colours are not asserted exactly. A one-pixel line through four samples is mostly
 * partial coverage, so what is compared is the frame against itself.
 */
static br_error msw_check_immediate(msw_state *st)
{
    const br_rectangle whole = {
        .x = -st->colour_ms->origin_x,
        .y = -st->colour_ms->origin_y,
        .w = st->colour_ms->width,
        .h = st->colour_ms->height,
    };
    const br_int_32  ox          = st->colour_ms->origin_x;
    const br_int_32  oy          = st->colour_ms->origin_y;
    const br_uint_32 ink         = BR_COLOUR_RGB(0x10, 0x10, 0x10);
    br_pixelmap     *before      = NULL;
    br_pixelmap     *after       = NULL;
    br_int_32        line_marked = 0;
    br_int_32        text_marked = 0;
    br_int_32        differing   = 0;
    br_error         r           = BRE_FAIL;

    if(DevicePixelmapRectangleFill((br_device_pixelmap *)st->colour_ms, (br_rectangle *)&whole, BR_COLOUR_RGB(MSW_MID, MSW_MID, MSW_MID)) != BRE_OK) {
        BrLogError("RT", "ms witness: could not refill the multisampled target.");
        return BRE_FAIL;
    }

    /* Image rows 0-10, which the quad does not reach; the text starts below the line. */
    BrPixelmapLine(st->colour_ms, -ox, MSW_LINE_Y - oy, MSW_SIZE - 1 - ox, MSW_LINE_Y - oy, ink);
    BrPixelmapText(st->colour_ms, 2 - ox, 6 - oy, ink, BrFontFixed3x5, "MSAA");

    if((before = msw_read(st->colour_ms)) == NULL)
        goto out;

    for(br_int_32 y = 0; y <= 10; ++y)
        for(br_int_32 x = 0; x < MSW_SIZE; ++x) {
            if(msw_pixel(before, x, y) != MSW_MID) {
                if(y == MSW_LINE_Y)
                    ++line_marked;
                else
                    ++text_marked;
            }
        }

    /*
     * The two are counted apart because they are drawn on different rows: a threshold on
     * their sum lets the line satisfy it alone, and a text path that drew nothing would
     * pass. The line spans the frame, so its count is the strict one; the text only has to
     * appear. Neither assertion suffices alone - the comparison below cannot see a pixel
     * that was never drawn, because it has nothing to lose.
     */
    if(line_marked < MSW_SIZE / 2) {
        BrLogError("RT", "ms witness: the line marked %d pixels of the %d in its row.", line_marked, MSW_SIZE);
        goto out;
    }

    if(text_marked == 0) {
        BrLogError("RT", "ms witness: the text drew nothing into the multisampled target.");
        goto out;
    }

    msw_begin(st->colour_ms);
    BrZbSceneRender(st->world, st->camera, st->colour_ms, st->depth_ms);

    if((after = msw_read(st->colour_ms)) == NULL)
        goto out;

    for(br_int_32 y = 0; y <= 10; ++y)
        for(br_int_32 x = 0; x < MSW_SIZE; ++x) {
            if(msw_pixel(before, x, y) != msw_pixel(after, x, y))
                ++differing;
        }

    if(differing != 0) {
        BrLogError("RT", "ms witness: a scene scope changed %d pixels it does not cover; the line and the text were not in the samples.", differing);
        goto out;
    }

    BrLogInfo("RT", "ms witness: %d line and %d glyph pixels survived a scene scope unchanged.", line_marked, text_marked);
    r = BRE_OK;

out:
    if(after != NULL)
        BrPixelmapFree(after);

    if(before != NULL)
        BrPixelmapFree(before);

    return r;
}

static void msw_fini(msw_state *st)
{
    BrRendererEnd();

    if(st->depth_ms != NULL)
        BrPixelmapFree(st->depth_ms);

    if(st->colour_ms != NULL)
        BrPixelmapFree(st->colour_ms);

    if(st->depth != NULL)
        BrPixelmapFree(st->depth);

    if(st->colour != NULL)
        BrPixelmapFree(st->colour);

    if(st->material != NULL)
        BrMaterialFree(st->material);

    if(st->model != NULL)
        BrModelFree(st->model);

    if(st->world != NULL)
        BrActorFree(st->world);

    BrMemSet(st, 0, sizeof(*st));
}

br_error rt_ms_witness_run(rt_demo *demo, const char *device)
{
    msw_state state = {0};
    br_error  r     = BRE_FAIL;

    /*
     * Only vkrend is under test: glrend renders into a multisampled texture of its own shape
     * and resolves on a blit, so it is the reference for what the feature looks like rather
     * than a second implementation this witness can hold to the same rules.
     */
    if(BrStrCmp(device, "vkrend") != 0) {
        BrLogError("RT", "ms witness: --ms-witness asserts vkrend's resolve; %s has its own.", device);
        return BRE_FAIL;
    }

    if(msw_build(&state, demo->_screen) != BRE_OK) {
        BrLogError("RT", "ms witness: could not build the scene (does the device allow a %dx%d RGBA_8888 offscreen?).", MSW_SIZE, MSW_SIZE);
        goto out;
    }

    if(state.samples < 2) {
        /*
         * Reached only when the device itself offers nothing: a count below what was asked
         * for on a device that offers it was already refused in msw_build(). There is nothing
         * to assert then, and saying so is the difference between a skip and a green run
         * that measured nothing.
         */
        BrLogInfo("RT", "ms witness: this device offers %d samples, so %d was not available; nothing to compare.", state.device_max, MSW_SAMPLES);
        r = BRE_OK;
        goto out;
    }

    if(msw_check_coverage(&state) != BRE_OK)
        goto out;

    if(msw_check_scopes(&state) != BRE_OK)
        goto out;

    if(msw_check_immediate(&state) != BRE_OK)
        goto out;

    BrLogInfo(
        "RT",
        "ms witness passed (%d samples: partial edge coverage in the resolved frame, no partial pixels without it, and fills, scenes, "
        "copies, lines and text keeping what the previous scope left).",
        state.samples);
    r = BRE_OK;

out:
    msw_fini(&state);
    return r;
}
