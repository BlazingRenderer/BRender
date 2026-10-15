/*
 * vkwitness - the vkrend witness, run under rendertest as --vk-witness.
 *
 * This is the witness devpmtest used to carry (its --vulkan mode); it was moved
 * here because devpmtest is an interactive text-drawing tool and vkrend's
 * pixelmaps do not support that, while rendertest is already a non-interactive,
 * multi-mode render-test binary with a device witness of its own (--env-witness).
 *
 * The front screen comes from brdemo's --vulkan path: the SDL3 device hands out a
 * vkrend front screen when BRT_VULKAN_B is set, so the invocation is
 * `rendertest --vulkan --vk-witness`. rt_vk_witness_run() drives the two cases
 * below against that screen and returns their combined result.
 */
#include <brender.h>
#include <brddi.h>

#include "brdemo.h"
#include "vkwitness.h"

/*
 * The vkrend front screen and its offscreen path.
 *
 * vkrend's frame ring, fences and retire lists live on the front screen, and an
 * offscreen records into that screen's current frame, so a front screen has to
 * exist before an offscreen can be touched. This case creates one from the
 * screen main() asked for (BRT_VULKAN_B under --vulkan) and drives the offscreen
 * path: match, fill, a non-indexed host upload, an indexed host upload with a
 * palette, resize and free. Both uploads' source types differ from the
 * destination's, so the copy has to convert - through the engine's converter
 * when the CPU path serves it, and through the sampler and the colour-attachment
 * write when the GPU rect draw does. The indexed upload carries a map palette,
 * which is the rect draw's indexed variant: an index texture plus a CLUT one.
 *
 * Every step's result is checked; a clean run and an error-free log are not
 * evidence on their own. The checks live in init() because that is how this
 * harness reports a failure today - see issue #26.
 *
 * After the offscreen work it presents that offscreen VKREND_PRESENT_COUNT
 * times through the front screen's doubleBuffer. The frame ring is four slots,
 * but its index advances modulo the swapchain's image count, so the presents
 * wrap it and the per-slot vkWaitForFences + RetireDrain run on reuse. The
 * offscreen is refilled with a different colour each frame so the sequence is
 * real: on a windowed run a person can watch the colour change on each present.
 *
 * Halfway through those presents the front screen itself is resized, which makes
 * the driver rebuild its swapchain on the next frame while the offscreen is still
 * live. That is the window-resize path, and it is the only thing that exercises the
 * front pixelmap's dependent-pixelmap count against a swapchain recreation.
 */
#define VKREND_OFFSCREEN_WIDTH      64
#define VKREND_OFFSCREEN_HEIGHT     64
#define VKREND_RESIZED_WIDTH        48
#define VKREND_RESIZED_HEIGHT       96
#define VKREND_FRONT_RESIZED_WIDTH  96
#define VKREND_FRONT_RESIZED_HEIGHT 192
#define VKREND_PRESENT_COUNT        12

typedef struct vkrend_offscreen_state {
    br_pixelmap *offscreen;
    br_pixelmap *host;
    br_pixelmap *indexed;
    br_pixelmap *indexed_pal;
    br_pixelmap *device_source;
    br_pixelmap *device_index;
    br_pixelmap *device_pal;
    br_pixelmap *device_dst;
} vkrend_offscreen_state;

/*
 * One bright colour per presented frame, so consecutive presents are
 * distinguishable by eye (the presented frames are not read back).
 */
static const br_colour vkrend_present_colours[] = {
    BR_COLOUR_RGB(255, 0, 0),   BR_COLOUR_RGB(0, 255, 0),     BR_COLOUR_RGB(0, 0, 255),     BR_COLOUR_RGB(255, 255, 0),
    BR_COLOUR_RGB(0, 255, 255), BR_COLOUR_RGB(255, 0, 255),   BR_COLOUR_RGB(255, 128, 0),   BR_COLOUR_RGB(128, 0, 255),
    BR_COLOUR_RGB(0, 255, 128), BR_COLOUR_RGB(255, 255, 255), BR_COLOUR_RGB(128, 128, 128), BR_COLOUR_RGB(0, 0, 0),
};

static br_error vkrend_offscreen_present(br_pixelmap *screen, br_pixelmap *offscreen)
{
    br_rectangle rect = {
        .x = 0,
        .y = 0,
        .w = offscreen->width,
        .h = offscreen->height,
    };

    for(int i = 0; i < VKREND_PRESENT_COUNT; ++i) {
        br_colour colour = vkrend_present_colours[i % BR_ASIZE(vkrend_present_colours)];

        /*
         * Resize the front screen while the offscreen is live. The driver rebuilds its
         * swapchain for the new size on the next frame; the offscreen and its retire-list
         * entries have to survive that, and the front's dependent-pixelmap count must not
         * be disturbed by it.
         */
        if(i == VKREND_PRESENT_COUNT / 2) {
            if(BrPixelmapResize(screen, VKREND_FRONT_RESIZED_WIDTH, VKREND_FRONT_RESIZED_HEIGHT) == NULL) {
                BrLogError("RT", "vkrend: BrPixelmapResize() on the front screen failed.");
                return BRE_FAIL;
            }

            if(screen->width != VKREND_FRONT_RESIZED_WIDTH || screen->height != VKREND_FRONT_RESIZED_HEIGHT) {
                BrLogError("RT", "vkrend: front screen resize left the pixelmap at %dx%d, expected %dx%d.", screen->width, screen->height,
                           VKREND_FRONT_RESIZED_WIDTH, VKREND_FRONT_RESIZED_HEIGHT);
                return BRE_FAIL;
            }
        }

        /* Recolour the whole offscreen so each present shows a different frame. */
        if(DevicePixelmapRectangleFill(offscreen, &rect, colour) != BRE_OK) {
            BrLogError("RT", "vkrend: BrPixelmapRectangleFill() failed before present %d.", i);
            return BRE_FAIL;
        }

        /*
         * The DDI entry behind BrPixelmapDoubleBuffer(), used directly so the
         * present path's br_error is visible - BrPixelmapDoubleBuffer() itself
         * returns void.
         */
        if(DevicePixelmapDoubleBuffer(screen, (br_device_pixelmap *)offscreen) != BRE_OK) {
            BrLogError("RT", "vkrend: BrPixelmapDoubleBuffer() failed on present %d.", i);
            return BRE_FAIL;
        }
    }

    return BRE_OK;
}

static void vkrend_offscreen_fini(void *user)
{
    vkrend_offscreen_state *state = user;

    if(state->device_dst != NULL)
        BrPixelmapFree(state->device_dst);

    if(state->device_pal != NULL)
        BrPixelmapFree(state->device_pal);

    if(state->device_index != NULL)
        BrPixelmapFree(state->device_index);

    if(state->device_source != NULL)
        BrPixelmapFree(state->device_source);

    if(state->indexed_pal != NULL)
        BrPixelmapFree(state->indexed_pal);

    if(state->indexed != NULL)
        BrPixelmapFree(state->indexed);

    if(state->host != NULL)
        BrPixelmapFree(state->host);

    if(state->offscreen != NULL)
        BrPixelmapFree(state->offscreen);

    state->device_dst    = NULL;
    state->device_pal    = NULL;
    state->device_index  = NULL;
    state->device_source = NULL;
    state->indexed_pal   = NULL;
    state->indexed       = NULL;
    state->host          = NULL;
    state->offscreen     = NULL;
}

/*
 * Read a region of a device offscreen back into a freshly allocated 4-byte host pixelmap
 * and assert every one of its pixels against expected[4] (in the memory order B, G, R, A).
 * The pixelmap is allocated in read_type; when that differs from the offscreen's type the
 * driver converts the readback, so the same bytes must come back either way. Logs the
 * first mismatch, with both byte sequences, and returns BRE_FAIL. On success logs the
 * first pixel's bytes so a passing run states exactly what it read.
 */
static br_error vkrend_readback_assert(br_pixelmap *offscreen, br_uint_8 read_type, br_rectangle *read_rect, br_point *read_dst,
                                       const br_uint_8 expected[4], const char *what)
{
    br_pixelmap *readback;
    br_int_32    x, y;

    if((readback = BrPixelmapAllocate(read_type, read_rect->w, read_rect->h, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the readback pixelmap for %s.", what);
        return BRE_FAIL;
    }

    if(DevicePixelmapRectangleCopyFrom((br_device_pixelmap *)offscreen, read_dst, (br_device_pixelmap *)readback, read_rect) != BRE_OK) {
        BrLogError("RT", "vkrend: DevicePixelmapRectangleCopyFrom() failed for %s.", what);
        BrPixelmapFree(readback);
        return BRE_FAIL;
    }

    for(y = 0; y < readback->height; ++y) {
        for(x = 0; x < readback->width; ++x) {
            const br_uint_8 *got = (const br_uint_8 *)readback->pixels + y * readback->row_bytes + x * 4;

            if(got[0] != expected[0] || got[1] != expected[1] || got[2] != expected[2] || got[3] != expected[3]) {
                BrLogError("RT", "vkrend: %s readback mismatch at (%d,%d): expected %02x %02x %02x %02x (B G R A), got %02x %02x %02x %02x.",
                           what, x, y, expected[0], expected[1], expected[2], expected[3], got[0], got[1], got[2], got[3]);
                BrPixelmapFree(readback);
                return BRE_FAIL;
            }
        }
    }

    BrLogInfo("RT", "vkrend: %s readback ok: %02x %02x %02x %02x (B G R A).", what, ((const br_uint_8 *)readback->pixels)[0],
              ((const br_uint_8 *)readback->pixels)[1], ((const br_uint_8 *)readback->pixels)[2], ((const br_uint_8 *)readback->pixels)[3]);

    BrPixelmapFree(readback);
    return BRE_OK;
}

static br_error vkrend_offscreen_init(void *user, br_pixelmap *screen, br_pixelmap *backbuffer, void *arg)
{
    vkrend_offscreen_state *state = user;
    br_rectangle            rect;
    br_point                point;
    br_uint_8               host_type;
    br_int_32               x, y;

    (void)backbuffer;
    (void)arg;

    /* The offscreen inherits the front screen's type and dimensions. */
    if((state->offscreen = BrPixelmapMatch(screen, BR_PMMATCH_OFFSCREEN)) == NULL) {
        BrLogError("RT", "vkrend: BrPixelmapMatch() could not make an offscreen pixelmap.");
        goto fail;
    }

    rect.x = 0;
    rect.y = 0;
    rect.w = state->offscreen->width;
    rect.h = state->offscreen->height;

    if(DevicePixelmapRectangleFill(state->offscreen, &rect, BR_COLOUR_RGB(0, 128, 255)) != BRE_OK) {
        BrLogError("RT", "vkrend: BrPixelmapRectangleFill() failed.");
        goto fail;
    }

    /*
     * Upload from host memory. The source type differs from the destination's
     * so rectangleCopyTo() has to convert it through the engine.
     */
    host_type = (screen->type == BR_PMT_RGBX_888) ? BR_PMT_RGBA_8888 : BR_PMT_RGBX_888;

    if((state->host = BrPixelmapAllocate(host_type, VKREND_OFFSCREEN_WIDTH, VKREND_OFFSCREEN_HEIGHT, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the host pixelmap.");
        goto fail;
    }

    for(y = 0; y < state->host->height; ++y)
        for(x = 0; x < state->host->width; ++x)
            BrPixelmapPixelSet(state->host, x, y, BR_COLOUR_RGB((br_uint_8)(x * 4), (br_uint_8)(y * 4), 0));

    point.x = 0;
    point.y = 0;
    rect.x  = 0;
    rect.y  = 0;
    rect.w  = state->host->width;
    rect.h  = state->host->height;

    if(DevicePixelmapRectangleCopyTo(state->offscreen, &point, (br_device_pixelmap *)state->host, &rect) != BRE_OK) {
        BrLogError("RT", "vkrend: BrPixelmapRectangleCopyTo() failed.");
        goto fail;
    }

    /*
     * Upload from an indexed host pixelmap with a palette, which is the rect draw's
     * indexed variant: one texture for the indices and a second for the palette. The
     * palette ramps all three channels and the indices are a pattern, so a wrong channel
     * order or a wrong CLUT row would move or recolour the picture rather than leave it
     * looking plausible. This upload's own pixels are not asserted below - the readback
     * there covers a filled region and a pixelSet point - so it is checked by construction
     * only.
     */
    if((state->indexed_pal = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the indexed source's palette.");
        goto fail;
    }

    for(y = 0; y < state->indexed_pal->height; ++y)
        BrPixelmapPixelSet(state->indexed_pal, 0, y, BR_COLOUR_RGB((br_uint_8)y, (br_uint_8)(255 - y), (br_uint_8)(y * 3)));

    if((state->indexed = BrPixelmapAllocate(BR_PMT_INDEX_8, VKREND_OFFSCREEN_WIDTH, VKREND_OFFSCREEN_HEIGHT, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the indexed host pixelmap.");
        goto fail;
    }

    for(y = 0; y < state->indexed->height; ++y)
        for(x = 0; x < state->indexed->width; ++x)
            BrPixelmapPixelSet(state->indexed, x, y, (br_uint_8)((x * 3 + y * 7 + (x ^ y)) & 0xff));

    /* The source's palette is the pixelmap's map; the draw uploads it from there. */
    state->indexed->map = state->indexed_pal;

    rect.w = state->indexed->width;
    rect.h = state->indexed->height;

    if(DevicePixelmapRectangleCopyTo(state->offscreen, &point, (br_device_pixelmap *)state->indexed, &rect) != BRE_OK) {
        BrLogError("RT", "vkrend: BrPixelmapRectangleCopyTo() failed for the indexed source.");
        goto fail;
    }

    /*
     * The readback - the tree's first check of what a device actually rendered, and the
     * device->host copy of rectangleCopyFrom(). Fill the whole image with a known colour and
     * a sub-rectangle well inside it with another, then read both back into host memory,
     * asserting every byte.
     *
     * vkrend's BR_PMT_RGBX_888 image is VK_FORMAT_B8G8R8A8_UNORM, so a pixel in memory is
     * B, G, R, A, not R, G, B, A, and the X channel is written 0xff. Asserting all four
     * bytes at every pixel of the region means a wrong colour, a swapped channel or a
     * stray y-flip fails rather than only moving a checksum nobody looks at.
     *
     * The inside and outside reads bracket the sub-rectangle: a rectangleFill() that ignored
     * its rectangle and cleared the whole image would paint the outside read with the
     * sub-rectangle's colour. The fill rectangles and the read regions do not overlap.
     */
    {
        const br_uint_8 fill_red   = 0x12;
        const br_uint_8 fill_green = 0x34;
        const br_uint_8 fill_blue  = 0x56;
        const br_uint_8 sub_red    = 0xa1;
        const br_uint_8 sub_green  = 0xb2;
        const br_uint_8 sub_blue   = 0xc3;

        /* Whole-image fill A, read at a pixel the sub-rectangle below will not cover. */
        br_rectangle outside_rect = {.x = 0, .y = 0, .w = 2, .h = 2};
        br_rectangle whole_rect   = {
              .x = -state->offscreen->origin_x, .y = -state->offscreen->origin_y, .w = state->offscreen->width, .h = state->offscreen->height};
        br_point        read_dst   = {.x = 0, .y = 0};
        const br_uint_8 outside[4] = {fill_blue, fill_green, fill_red, 0xff};

        /* Sub-rect fill B, well inside the image, read at a region strictly inside it. */
        br_rectangle    sub_rect    = {.x = 8, .y = 8, .w = 16, .h = 16};
        br_rectangle    inside_rect = {.x = 12, .y = 12, .w = 4, .h = 4};
        const br_uint_8 inside[4]   = {sub_blue, sub_green, sub_red, 0xff};

        if(DevicePixelmapRectangleFill(state->offscreen, &whole_rect, BR_COLOUR_RGB(fill_red, fill_green, fill_blue)) != BRE_OK) {
            BrLogError("RT", "vkrend: whole-image fill for the readback failed.");
            goto fail;
        }

        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &outside_rect, &read_dst, outside, "the whole-image fill") != BRE_OK)
            goto fail;

        if(DevicePixelmapRectangleFill(state->offscreen, &sub_rect, BR_COLOUR_RGB(sub_red, sub_green, sub_blue)) != BRE_OK) {
            BrLogError("RT", "vkrend: sub-rect fill for the readback failed.");
            goto fail;
        }

        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &inside_rect, &read_dst, inside, "the sub-rect fill") != BRE_OK)
            goto fail;

        /* The same region into a different host type drives the conversion path. */
        if(vkrend_readback_assert(state->offscreen, BR_PMT_RGBA_8888, &inside_rect, &read_dst, inside, "the converted sub-rect fill") != BRE_OK)
            goto fail;

        /* The whole-image colour must survive the sub-rect fill outside its rectangle. */
        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &outside_rect, &read_dst, outside, "the sub-rect fill's outside") != BRE_OK)
            goto fail;
    }

    /*
     * The rect draw's orientation. rectangleCopyTo() uploads the source top-down, so the
     * viewport has to put source row 0 at the destination's top row. rect.vert.glsl once
     * mapped it to the bottom - a flip no uniform fill or centre-of-image readback can
     * see - so copy a two-row source and read each row back at its own coordinate. With
     * the flip, source row 0 lands on the bottom row and both assertions below fail.
     */
    {
        br_pixelmap    *rows;
        br_rectangle    rows_src = {.x = 0, .y = 0, .w = 4, .h = 2};
        br_point        rows_dst = {.x = 48, .y = 4};
        br_rectangle    top_row  = {.x = 48, .y = 4, .w = 4, .h = 1};
        br_rectangle    bot_row  = {.x = 48, .y = 5, .w = 4, .h = 1};
        br_point        read_dst = {.x = 0, .y = 0};
        const br_uint_8 red[4]   = {0x00, 0x00, 0xff, 0xff};
        const br_uint_8 blue[4]  = {0xff, 0x00, 0x00, 0xff};
        br_error        err;

        if((rows = BrPixelmapAllocate(BR_PMT_RGBX_888, 4, 2, NULL, BR_PMAF_NORMAL)) == NULL) {
            BrLogError("RT", "vkrend: could not allocate the orientation source pixelmap.");
            goto fail;
        }

        for(br_int_32 i = 0; i < rows->width; ++i) {
            BrPixelmapPixelSet(rows, i, 0, BR_COLOUR_RGB(255, 0, 0));
            BrPixelmapPixelSet(rows, i, 1, BR_COLOUR_RGB(0, 0, 255));
        }

        err = DevicePixelmapRectangleCopyTo(state->offscreen, &rows_dst, (br_device_pixelmap *)rows, &rows_src);
        BrPixelmapFree(rows);

        if(err != BRE_OK) {
            BrLogError("RT", "vkrend: DevicePixelmapRectangleCopyTo() failed for the orientation check.");
            goto fail;
        }

        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &top_row, &read_dst, red,
                                  "the copy's source row 0 at the destination top") != BRE_OK)
            goto fail;

        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &bot_row, &read_dst, blue,
                                  "the copy's source row 1 at the destination bottom") != BRE_OK)
            goto fail;
    }

    /*
     * The line and point draws, through the same offscreen. Both are clipped to the
     * offscreen and drawn on the GPU over the fill above; the pixelSet's colour is
     * asserted at its exact coordinate by the readback below. The line crosses the picture
     * diagonally and the point sits at its centre.
     */
    {
        br_point line_start = {.x = 2, .y = 2};
        br_point line_end   = {.x = VKREND_OFFSCREEN_WIDTH - 3, .y = VKREND_OFFSCREEN_HEIGHT - 3};
        br_point line_point = {.x = VKREND_OFFSCREEN_WIDTH / 2, .y = VKREND_OFFSCREEN_HEIGHT / 2};

        if(DevicePixelmapLine(state->offscreen, &line_start, &line_end, BR_COLOUR_RGB(255, 255, 0)) != BRE_OK) {
            BrLogError("RT", "vkrend: DevicePixelmapLine() failed.");
            goto fail;
        }

        if(DevicePixelmapPixelSet(state->offscreen, &line_point, BR_COLOUR_RGB(0, 255, 0)) != BRE_OK) {
            BrLogError("RT", "vkrend: DevicePixelmapPixelSet() failed.");
            goto fail;
        }
    }

    /*
     * Read back the exact point the pixelSet above drew, so the point draw is asserted on
     * its pixel's colour and coordinate, not just on its return value.
     */
    {
        br_point        point_dst         = {.x = 0, .y = 0};
        br_rectangle    point_rect        = {.x = VKREND_OFFSCREEN_WIDTH / 2, .y = VKREND_OFFSCREEN_HEIGHT / 2, .w = 1, .h = 1};
        const br_uint_8 point_expected[4] = {0x00, 0xff, 0x00, 0xff}; /* BR_COLOUR_RGB(0, 255, 0) in B, G, R, A. */

        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &point_rect, &point_dst, point_expected, "the pixelSet point") != BRE_OK)
            goto fail;
    }

    /*
     * A sub-pixelmap of the offscreen, the allocateSub path (drivers/vkrend/devpmsub.c).
     * Its rectangleFill delegates back to the offscreen with the coordinates mapped into
     * the parent's space, so a successful fill exercises both the allocation and the
     * delegation. It has to be freed before the offscreen: the offscreen's free asserts
     * that every sub has released it.
     */
    {
        br_rectangle sub_rect = {
            .x = 8,
            .y = 8,
            .w = 16,
            .h = 16,
        };
        br_pixelmap *sub = BrPixelmapAllocateSub(state->offscreen, sub_rect.x, sub_rect.y, sub_rect.w, sub_rect.h);

        if(sub == NULL) {
            BrLogError("RT", "vkrend: BrPixelmapAllocateSub() on the offscreen failed.");
            goto fail;
        }

        if(DevicePixelmapRectangleFill((br_device_pixelmap *)sub, &sub_rect, BR_COLOUR_RGB(255, 64, 0)) != BRE_OK) {
            BrLogError("RT", "vkrend: BrPixelmapRectangleFill() on the sub-pixelmap failed.");
            BrPixelmapFree(sub);
            goto fail;
        }

        BrPixelmapFree(sub);
    }

    /*
     * A stretch blit: a 2x2 host source scaled 3x into a 6x6 destination rectangle with
     * rectangleStretchCopyTo(), so the viewport - not a same-size copy - has to do the
     * scaling. An unscaled copy would write the source's 2x2 into the destination's top-
     * left corner and leave the rest of the rectangle at the whole-image fill colour, so
     * every one of the four corner blocks below would fail, not just the far ones. The
     * readbacks assert the nearest-neighbour geometry, including a pixel either side of a
     * block boundary.
     */
    {
        const br_int_32 scale = 3;
        br_pixelmap    *stretch_src;
        br_rectangle    dst_rect = {.x = 40, .y = 40, .w = 2 * scale, .h = 2 * scale};
        br_rectangle    src_rect = {.x = 0, .y = 0, .w = 2, .h = 2};
        const br_uint_8 red[4]   = {0x00, 0x00, 0xff, 0xff};
        const br_uint_8 green[4] = {0x00, 0xff, 0x00, 0xff};
        const br_uint_8 blue[4]  = {0xff, 0x00, 0x00, 0xff};
        const br_uint_8 white[4] = {0xff, 0xff, 0xff, 0xff};
        br_point        read_dst = {.x = 0, .y = 0};
        br_rectangle    block;
        br_error        err;

        if((stretch_src = BrPixelmapAllocate(BR_PMT_RGBX_888, 2, 2, NULL, BR_PMAF_NORMAL)) == NULL) {
            BrLogError("RT", "vkrend: could not allocate the stretch source pixelmap.");
            goto fail;
        }

        BrPixelmapPixelSet(stretch_src, 0, 0, BR_COLOUR_RGB(255, 0, 0));
        BrPixelmapPixelSet(stretch_src, 1, 0, BR_COLOUR_RGB(0, 255, 0));
        BrPixelmapPixelSet(stretch_src, 0, 1, BR_COLOUR_RGB(0, 0, 255));
        BrPixelmapPixelSet(stretch_src, 1, 1, BR_COLOUR_RGB(255, 255, 255));

        err = DevicePixelmapRectangleStretchCopyTo(state->offscreen, &dst_rect, (br_device_pixelmap *)stretch_src, &src_rect);
        BrPixelmapFree(stretch_src);

        if(err != BRE_OK) {
            BrLogError("RT", "vkrend: DevicePixelmapRectangleStretchCopyTo() failed.");
            goto fail;
        }

        /* Each source pixel becomes a scale x scale block; assert all four corner blocks. */
        block = (br_rectangle){.x = dst_rect.x, .y = dst_rect.y, .w = scale, .h = scale};
        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &block, &read_dst, red, "the stretch blit's top-left block") != BRE_OK)
            goto fail;

        block.x = dst_rect.x + scale;
        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &block, &read_dst, green, "the stretch blit's top-right block") != BRE_OK)
            goto fail;

        block.x = dst_rect.x;
        block.y = dst_rect.y + scale;
        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &block, &read_dst, blue, "the stretch blit's bottom-left block") != BRE_OK)
            goto fail;

        block.x = dst_rect.x + scale;
        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &block, &read_dst, white, "the stretch blit's bottom-right block") != BRE_OK)
            goto fail;

        /* One pixel either side of the vertical block boundary, in the bottom row. */
        block = (br_rectangle){.x = dst_rect.x + scale - 1, .y = dst_rect.y + scale + 1, .w = 1, .h = 1};
        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &block, &read_dst, blue, "the stretch blit's left of the boundary") != BRE_OK)
            goto fail;

        block.x = dst_rect.x + scale;
        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &block, &read_dst, white,
                                  "the stretch blit's right of the boundary") != BRE_OK)
            goto fail;
    }

    /*
     * The device-source rect draw. rectangleCopy/rectangleStretchCopy keep vkCmdBlitImage
     * for a same-format pair, so a mismatched pair - here an INDEX_8 source that has to be
     * resolved through its palette into a colour destination - falls to the rect draw,
     * which samples the source image where it lies instead of uploading host memory. The
     * source is filled through an indexed host upload (whose destination is indexed, so
     * the CPU staging path leaves the index bytes in the device image), then copied
     * device-to-device and read back. Four palette entries are laid out one per quadrant,
     * so a draw that skipped the device image, or that sampled the wrong region or the
     * wrong row, fails a quadrant assertion rather than passing on a uniform fill.
     */
    {
        const br_int_32 width    = 8;
        const br_int_32 height   = 8;
        const br_int_32 half     = width / 2;
        br_point        copy_dst = {.x = 0, .y = 0};
        br_point        idx_dst  = {.x = 0, .y = 0};
        br_rectangle    copy_src = {.x = 0, .y = 0, .w = width, .h = height};
        br_point        read_dst = {.x = 0, .y = 0};
        br_rectangle    quadrant = {.x = 0, .y = 0, .w = half, .h = half};
        const br_uint_8 red[4]   = {0x00, 0x00, 0xff, 0xff};
        const br_uint_8 green[4] = {0x00, 0xff, 0x00, 0xff};
        const br_uint_8 blue[4]  = {0xff, 0x00, 0x00, 0xff};
        const br_uint_8 white[4] = {0xff, 0xff, 0xff, 0xff};

        if((state->device_source = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_INDEX_8, width, height)) == NULL) {
            BrLogError("RT", "vkrend: could not create the INDEX_8 device source offscreen.");
            goto fail;
        }

        if((state->device_dst = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBA_8888, width, height)) == NULL) {
            BrLogError("RT", "vkrend: could not create the RGBA_8888 device destination offscreen.");
            goto fail;
        }

        /*
         * BrPixelmapMatch() copies the front screen's origin (the centre of a 1280x720
         * screen), which would put image (0,0) at (640,360) and outside these 8x8
         * pixelmaps. The device->device path addresses the images by the coordinates it is
         * handed, so give these offscreens their own top-left and address them from there.
         */
        state->device_source->origin_x = 0;
        state->device_source->origin_y = 0;
        state->device_dst->origin_x    = 0;
        state->device_dst->origin_y    = 0;

        if((state->device_pal = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL)) == NULL) {
            BrLogError("RT", "vkrend: could not allocate the device source's palette.");
            goto fail;
        }

        for(y = 0; y < state->device_pal->height; ++y) {
            br_colour colour = 0;

            /* Entries 1..4 are the four quadrants; no other entry is ever indexed. */
            if(y == 1)
                colour = BR_COLOUR_RGB(255, 0, 0);
            else if(y == 2)
                colour = BR_COLOUR_RGB(0, 255, 0);
            else if(y == 3)
                colour = BR_COLOUR_RGB(0, 0, 255);
            else if(y == 4)
                colour = BR_COLOUR_RGB(255, 255, 255);

            BrPixelmapPixelSet(state->device_pal, 0, y, colour);
        }

        if((state->device_index = BrPixelmapAllocate(BR_PMT_INDEX_8, width, height, NULL, BR_PMAF_NORMAL)) == NULL) {
            BrLogError("RT", "vkrend: could not allocate the device source's host index plane.");
            goto fail;
        }

        for(y = 0; y < height; ++y) {
            for(x = 0; x < width; ++x) {
                br_uint_8 index;

                if(x < half)
                    index = (y < half) ? 1 : 3;
                else
                    index = (y < half) ? 2 : 4;

                BrPixelmapPixelSet(state->device_index, x, y, index);
            }
        }

        /* The device source draws from this palette; the index plane only carries it. */
        state->device_source->map = state->device_pal;

        if(DevicePixelmapRectangleCopyTo(state->device_source, &idx_dst, (br_device_pixelmap *)state->device_index, &copy_src) != BRE_OK) {
            BrLogError("RT", "vkrend: the indexed host upload into the device source failed.");
            goto fail;
        }

        if(DevicePixelmapRectangleCopy(state->device_dst, &copy_dst, (br_device_pixelmap *)state->device_source, &copy_src) != BRE_OK) {
            BrLogError("RT", "vkrend: the device -> device rectangleCopy failed.");
            goto fail;
        }

        if(vkrend_readback_assert(state->device_dst, state->device_dst->type, &quadrant, &read_dst, red,
                                  "the device source's top-left quadrant") != BRE_OK)
            goto fail;

        quadrant.x = half;
        if(vkrend_readback_assert(state->device_dst, state->device_dst->type, &quadrant, &read_dst, green,
                                  "the device source's top-right quadrant") != BRE_OK)
            goto fail;

        quadrant.x = 0;
        quadrant.y = half;
        if(vkrend_readback_assert(state->device_dst, state->device_dst->type, &quadrant, &read_dst, blue,
                                  "the device source's bottom-left quadrant") != BRE_OK)
            goto fail;

        quadrant.x = half;
        if(vkrend_readback_assert(state->device_dst, state->device_dst->type, &quadrant, &read_dst, white,
                                  "the device source's bottom-right quadrant") != BRE_OK)
            goto fail;
    }

    /*
     * A sub-pixelmap source. The witness above only ever makes a sub a destination; this
     * makes the INDEX_8 device source's bottom-right quadrant the source of a device-to-
     * device copy. A sub's as_sub.parent aliases its as_back (drivers/vkrend/devpixmp.h),
     * and its image sits at its base inside the owner, so the rect draw has to resolve the
     * owner and sample it at (base + source rect) - a draw that sampled the sub's as_back
     * as the image, or the owner at (0,0), would crash or read the wrong quadrant. The
     * copy overwrites device_dst's top-left, whose colour the whole-image copy above
     * already asserted, so the new quadrant assertion is the only thing that changes.
     */
    {
        const br_int_32 half     = 4;
        br_rectangle    sub_rect = {.x = half, .y = half, .w = half, .h = half};
        br_point        sub_dst  = {.x = 0, .y = 0};
        br_rectangle    copy_src = {.x = 0, .y = 0, .w = half, .h = half};
        br_point        read_dst = {.x = 0, .y = 0};
        br_rectangle    region   = {.x = 0, .y = 0, .w = half, .h = half};
        const br_uint_8 white[4] = {0xff, 0xff, 0xff, 0xff};
        br_pixelmap    *sub;

        if((sub = BrPixelmapAllocateSub(state->device_source, sub_rect.x, sub_rect.y, sub_rect.w, sub_rect.h)) == NULL) {
            BrLogError("RT", "vkrend: BrPixelmapAllocateSub() on the INDEX_8 device source failed.");
            goto fail;
        }

        if(DevicePixelmapRectangleCopy(state->device_dst, &sub_dst, (br_device_pixelmap *)sub, &copy_src) != BRE_OK) {
            BrLogError("RT", "vkrend: the sub-pixelmap INDEX_8 source copy failed.");
            BrPixelmapFree(sub);
            goto fail;
        }

        BrPixelmapFree(sub);

        if(vkrend_readback_assert(state->device_dst, state->device_dst->type, &region, &read_dst, white, "the sub-pixelmap INDEX_8 source") != BRE_OK)
            goto fail;
    }

    /*
     * The same sub-source shape through the blit path. device_dst is RGBA_8888, so a
     * device-to-device copy of one of its subs is a same-format pair and takes
     * vkCmdBlitImage rather than the rect draw; it must still resolve the owner and offset
     * the source rect by the sub's base. A fresh destination keeps this off device_dst,
     * whose quadrants the checks above assert.
     */
    {
        const br_int_32 half     = 4;
        br_rectangle    sub_rect = {.x = half, .y = half, .w = half, .h = half};
        br_point        blit_dst = {.x = 0, .y = 0};
        br_rectangle    copy_src = {.x = 0, .y = 0, .w = half, .h = half};
        br_point        read_dst = {.x = 0, .y = 0};
        br_rectangle    region   = {.x = 0, .y = 0, .w = half, .h = half};
        const br_uint_8 white[4] = {0xff, 0xff, 0xff, 0xff};
        br_pixelmap    *sub, *blit_pm;

        if((sub = BrPixelmapAllocateSub(state->device_dst, sub_rect.x, sub_rect.y, sub_rect.w, sub_rect.h)) == NULL) {
            BrLogError("RT", "vkrend: BrPixelmapAllocateSub() on the RGBA_8888 device destination failed.");
            goto fail;
        }

        if((blit_pm = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBA_8888, half, half)) == NULL) {
            BrLogError("RT", "vkrend: could not create the sub-source blit destination offscreen.");
            BrPixelmapFree(sub);
            goto fail;
        }

        blit_pm->origin_x = 0;
        blit_pm->origin_y = 0;

        if(DevicePixelmapRectangleCopy(blit_pm, &blit_dst, (br_device_pixelmap *)sub, &copy_src) != BRE_OK) {
            BrLogError("RT", "vkrend: the sub-pixelmap RGBA_8888 blit failed.");
            BrPixelmapFree(blit_pm);
            BrPixelmapFree(sub);
            goto fail;
        }

        BrPixelmapFree(sub);

        if(vkrend_readback_assert(blit_pm, blit_pm->type, &region, &read_dst, white, "the sub-pixelmap RGBA_8888 blit") != BRE_OK) {
            BrPixelmapFree(blit_pm);
            goto fail;
        }

        BrPixelmapFree(blit_pm);
    }

    /*
     * The text draw. A fixed-pitch font's 'L' has a known bitmap - a full-height stroke
     * in column 0 and a three-wide foot in the last row - so the witness paints a known
     * background, draws one glyph in a known colour and asserts the exact pixels the
     * bitmap implies, plus a pixel inside the glyph's cell that the glyph does not cover
     * and one just outside it. A text() that did nothing, an atlas that sampled the
     * wrong layer, a flipped glyph or swapped axes fails the stroke or foot assertion
     * rather than passing on an unchanged background.
     *
     * The offscreen is screen-sized and its origin is the screen centre, so a rectangle
     * is in user coordinates: the glyph cell at base (744,414) is at user (104,54).
     */
    {
        br_rectangle    background = {.x = 100, .y = 50, .w = 16, .h = 12};
        br_point        text_point = {.x = 104, .y = 54};
        br_rectangle    stroke     = {.x = 104, .y = 54, .w = 1, .h = 4};
        br_rectangle    foot       = {.x = 104, .y = 58, .w = 3, .h = 1};
        br_rectangle    uncovered  = {.x = 105, .y = 54, .w = 1, .h = 1};
        br_rectangle    outside    = {.x = 107, .y = 54, .w = 1, .h = 1};
        br_point        read_dst   = {.x = 0, .y = 0};
        const br_uint_8 red[4]     = {0x00, 0x00, 0xff, 0xff}; /* BR_COLOUR_RGB(255, 0, 0) in B, G, R, A. */
        const br_uint_8 bg[4]      = {0x30, 0x20, 0x10, 0xff}; /* BR_COLOUR_RGB(0x10, 0x20, 0x30) in B, G, R, A. */

        if(DevicePixelmapRectangleFill(state->offscreen, &background, BR_COLOUR_RGB(0x10, 0x20, 0x30)) != BRE_OK) {
            BrLogError("RT", "vkrend: the text background fill failed.");
            goto fail;
        }

        if(DevicePixelmapText(state->offscreen, &text_point, BrFontFixed3x5, "L", BR_COLOUR_RGB(255, 0, 0)) != BRE_OK) {
            BrLogError("RT", "vkrend: DevicePixelmapText() failed.");
            goto fail;
        }

        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &stroke, &read_dst, red, "the text 'L' stroke") != BRE_OK)
            goto fail;

        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &foot, &read_dst, red, "the text 'L' foot") != BRE_OK)
            goto fail;

        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &uncovered, &read_dst, bg, "the text cell's uncovered pixel") != BRE_OK)
            goto fail;

        if(vkrend_readback_assert(state->offscreen, state->offscreen->type, &outside, &read_dst, bg, "the text cell's outside") != BRE_OK)
            goto fail;
    }

    if(BrPixelmapResize(state->offscreen, VKREND_RESIZED_WIDTH, VKREND_RESIZED_HEIGHT) == NULL) {
        BrLogError("RT", "vkrend: BrPixelmapResize() failed.");
        goto fail;
    }

    if(state->offscreen->width != VKREND_RESIZED_WIDTH || state->offscreen->height != VKREND_RESIZED_HEIGHT) {
        BrLogError("RT", "vkrend: BrPixelmapResize() left the pixelmap at %dx%d, expected %dx%d.", state->offscreen->width,
                   state->offscreen->height, VKREND_RESIZED_WIDTH, VKREND_RESIZED_HEIGHT);
        goto fail;
    }

    if(vkrend_offscreen_present(screen, state->offscreen) != BRE_OK)
        goto fail;

    BrLogInfo("RT", "vkrend: offscreen witness passed (match, fill, %dx%d host upload, %dx%d indexed upload, a line and a pixelSet, a readback round-trip, a source-orientation check, a sub-rect fill, a stretch blit, an INDEX_8 device source through the rect draw, a sub-pixelmap INDEX_8 source and a sub-pixelmap blit source, a text glyph, a sub-pixelmap fill, resize to %dx%d, %d presents with a front-screen resize to %dx%d, free).",
              VKREND_OFFSCREEN_WIDTH, VKREND_OFFSCREEN_HEIGHT, VKREND_OFFSCREEN_WIDTH, VKREND_OFFSCREEN_HEIGHT, state->offscreen->width,
              state->offscreen->height, VKREND_PRESENT_COUNT, VKREND_FRONT_RESIZED_WIDTH, VKREND_FRONT_RESIZED_HEIGHT);

    vkrend_offscreen_fini(state);
    return BRE_OK;

fail:
    /* A failed test never runs its fini, so free here. */
    vkrend_offscreen_fini(state);
    return BRE_FAIL;
}

/*
 * The br_renderer witness. It talks to a state-only renderer through BrRendererBegin(),
 * draws hand-built br_models through BrZsSceneRender() (and one step through the Zb path,
 * BrZbSceneRender(), with a depth buffer) and reads the offscreen back, asserting on bytes.
 *
 * Step 1 (stage 2) draws one flat-shaded triangle from an identity actor, given a stored
 * GPU buffer pair by BrModelUpdate() (GeometryV1ModelStoredNew goes to vkrend's
 * gv1model.c). BrZsSceneRender defers it into the primitive heap and the bucket geometry
 * draws it, all into the front screen's current frame command buffer; the readback's flush
 * submits it. The centre pixel must be the surface colour and a corner the clear colour.
 *
 * Step 2 (stage 3) moves the triangle with a non-identity actor transform, and then moves
 * the camera, asserting the surface pixels shift by exactly the number of texels the
 * camera's perspective projection predicts. That is positional: an MVP that dropped the
 * model matrix or the projection would shift by zero, or by the wrong amount, and fail.
 *
 * Step 3 (stage 4) draws a two-group model whose faces carry two distinct materials. The
 * deferred path stores one primitive per prepared group (see gstored.c), so both groups
 * must be drawn: each group's colour is asserted where that group is, and the surface
 * pixels must cover the union of the two groups' predicted screen bounds, which a wrong
 * per-group vertex or index offset would move away.
 *
 * Step 4 (stage 5) lights a white triangle with a white ambient light and a white
 * directional light, and asserts the centre pixel against the value the BRender model
 * predicts, computed here independently of the renderer: (ka * ambient + dot(N, L) * kd *
 * light) * surfaceColour, rounded to the destination's 8-bit channel. Rotating the light
 * moves the value as the dot product predicts, changing ka/kd moves it as the coefficients
 * predict, a light pointing away leaves only the ambient term, and a point light the core
 * culls for the model (its radius cannot reach it) contributes nothing.
 */
typedef struct vkrend_renderer_state {
    br_pixelmap *screen;
    br_pixelmap *offscreen;
    br_actor    *world;
    br_model    *model;
    br_material *material;

    br_model    *multi_model;
    br_material *multi_left;
    br_material *multi_right;

    br_model    *lit_model;
    br_material *lit_material;
    br_model    *dim_model;
    br_material *dim_material;
    br_model    *unlit_model;
    br_material *unlit_material;

    br_actor *ambient_light;
    br_actor *direct_light;
    br_actor *point_light;
    br_actor *volume_light;

    br_pixelmap *texture;
    br_material *texture_material;
    br_model    *texture_model;

    br_pixelmap *mip_texture;
    br_material *mip_material;
    br_model    *mip_model;

    br_pixelmap *filter_texture;
    br_material *filter_material;
    br_model    *filter_model;

    br_pixelmap *indexed_texture;
    br_pixelmap *indexed_palette;
    br_material *indexed_material;
    br_model    *indexed_model;

    br_pixelmap *rt_texture;
    br_material *rt_material;
    br_model    *rt_model;

    br_model    *vc_model;
    br_material *vc_material;
    br_model    *prelit_model;
    br_material *prelit_material;

    br_model    *smooth_model;
    br_material *smooth_material;
    br_vector3   smooth_positions[3];
    br_vector3   smooth_normals[3];

    br_model       *occlusion_model;
    br_material    *occlusion_far;
    br_material    *occlusion_near;
    br_order_table *occlusion_order_table;

    br_model    *depth_rev_model;
    br_pixelmap *depth;

    br_actor *clip_plane;

    br_pixelmap *env_texture;
    br_material *env_material;
    br_model    *env_model;
    br_actor    *env_anchor;

    br_pixelmap *aspect_texture;
    br_model    *aspect_model;
    br_material *aspect_material;
} vkrend_renderer_state;

static const br_uint_8 vkrend_renderer_clear[4]         = {0x30, 0x20, 0x10, 0xff}; /* B G R A */
static const br_uint_8 vkrend_renderer_surface[4]       = {0xcc, 0xbb, 0xaa, 0xff}; /* B G R A */
static const br_uint_8 vkrend_renderer_surface_right[4] = {0x22, 0x44, 0x88, 0xff}; /* B G R A */
static const br_uint_8 vkrend_renderer_white[4]         = {0xff, 0xff, 0xff, 0xff}; /* B G R A */

/*
 * The bytes the vertex-colour, pre-lit and blend steps assert: the even red/blue mix, the
 * two ends of the same triangle, the shaded surface plus the pre-lit vertex colour, and
 * the half-opaque white over black.
 */
static const br_uint_8 vkrend_renderer_vc_centre[4] = {0x80, 0x00, 0x80, 0xff};
static const br_uint_8 vkrend_renderer_vc_blue[4]   = {0xcc, 0x00, 0x33, 0xff};
static const br_uint_8 vkrend_renderer_vc_red[4]    = {0x33, 0x00, 0xcc, 0xff};
static const br_uint_8 vkrend_renderer_prelit[4]    = {0xbf, 0xbf, 0xff, 0xff};
static const br_uint_8 vkrend_renderer_blend[4]     = {0x80, 0x80, 0x80, 0x80};

/*
 * The occlusion step's two surface colours (B G R A): the far triangle is red, the near one
 * blue, so which face the depth sort leaves on top is readable from the centre pixel.
 */
static const br_uint_8 vkrend_renderer_occlusion_far[4]  = {0x00, 0x00, 0xff, 0xff};
static const br_uint_8 vkrend_renderer_occlusion_near[4] = {0xff, 0x00, 0x00, 0xff};

/*
 * The four texels of the witness texture, and the readback bytes (B G R A) they produce.
 * The indexed texture's palette uses the same three colours, so its quadrants assert the
 * same bytes; palette entry 0, which the colour key discards, is a fourth distinct colour.
 */
static const br_uint_8 vkrend_renderer_tex_red[4]    = {0x00, 0x00, 0xff, 0xff};
static const br_uint_8 vkrend_renderer_tex_green[4]  = {0x00, 0xff, 0x00, 0xff};
static const br_uint_8 vkrend_renderer_tex_blue[4]   = {0xff, 0x00, 0x00, 0xff};
static const br_uint_8 vkrend_renderer_tex_black[4]  = {0x00, 0x00, 0x00, 0xff};
static const br_uint_8 vkrend_renderer_index_zero[4] = {0x33, 0x22, 0x11, 0xff};

/*
 * The environment-map step's map is 16x16 with a distinct colour per texel, encoding its
 * coordinates - texel (tx, ty) is RGB(tx * 16, ty * 16, 0x80) - so the readback's green and
 * red bytes name the texel the environment fragment stage fetched. The step reads a pixel
 * VKREND_ENV_READ_X world units off the view axis, where the local and infinite eyes
 * reflect into different texels; the environment anchor is rotated 60 degrees about Y, so
 * the environment matrix the shader rotates by is that rotation's inverse.
 */
#define VKREND_ENV_TEXTURE_SIZE 16
#define VKREND_ENV_READ_X       BR_SCALAR(1.035)
#define VKREND_ENV_ROT_DEG      BR_SCALAR(60.0)

/*
 * The camera every step uses: at +z looking down -z with the default 45 degree
 * perspective. The distance is chosen only so a translated triangle stays inside the
 * viewport.
 */
#define VKREND_RENDERER_CAMERA_DISTANCE BR_SCALAR(8.0)

/*
 * The per-draw material buffer growth witness draws this many one-triangle actors in a
 * single scene. RendererVkMaterialReserve() bump-allocates one block per draw from a buffer
 * vkrend's sceneBegin() creates with BR_VKREND_MATERIAL_INITIAL_BLOCKS (16) blocks, so the
 * count must exceed 16 for the scene to grow it. The actors are scaled down and laid out in
 * a grid that keeps every triangle apart and inside the viewport, so each one's projected
 * centre can be asserted.
 */
#define VKREND_MATERIAL_GROWTH_DRAWS   24
#define VKREND_MATERIAL_GROWTH_COLUMNS 6
#define VKREND_MATERIAL_GROWTH_SPACING BR_SCALAR(0.6)
#define VKREND_MATERIAL_GROWTH_SCALE   BR_SCALAR(0.15)

/* Fill the whole offscreen with the clear colour. */
static br_error vkrend_renderer_fill_clear(br_pixelmap *offscreen)
{
    br_rectangle whole = {
        .x = -offscreen->origin_x,
        .y = -offscreen->origin_y,
        .w = offscreen->width,
        .h = offscreen->height,
    };

    if(DevicePixelmapRectangleFill(offscreen, &whole, BR_COLOUR_RGB(0x10, 0x20, 0x30)) != BRE_OK) {
        BrLogError("RT", "vkrend: clearing the renderer offscreen failed.");
        return BRE_FAIL;
    }

    return BRE_OK;
}

/*
 * Assert one image pixel against expected[4] (B G R A). The offscreen's origin is its
 * centre, so image pixel (x, y) is origin-relative (x - origin_x, y - origin_y).
 */
static br_error vkrend_renderer_assert_pixel(br_pixelmap *offscreen, br_int_32 x, br_int_32 y, const br_uint_8 expected[4], const char *what)
{
    br_rectangle rect = {
        .x = (br_int_16)(x - offscreen->origin_x),
        .y = (br_int_16)(y - offscreen->origin_y),
        .w = 1,
        .h = 1,
    };
    br_point dst = {0, 0};

    return vkrend_readback_assert(offscreen, offscreen->type, &rect, &dst, expected, what);
}

/* Read the whole offscreen into a fresh host pixelmap so a step can scan it. */
static br_pixelmap *vkrend_renderer_read(br_pixelmap *offscreen)
{
    br_pixelmap *readback;
    br_rectangle rect = {
        .x = -offscreen->origin_x,
        .y = -offscreen->origin_y,
        .w = offscreen->width,
        .h = offscreen->height,
    };
    br_point dst = {0, 0};

    if((readback = BrPixelmapAllocate(offscreen->type, offscreen->width, offscreen->height, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the whole-image readback pixelmap.");
        return NULL;
    }

    if(DevicePixelmapRectangleCopyFrom((br_device_pixelmap *)offscreen, &dst, (br_device_pixelmap *)readback, &rect) != BRE_OK) {
        BrLogError("RT", "vkrend: whole-image readback failed.");
        BrPixelmapFree(readback);
        return NULL;
    }

    return readback;
}

static br_boolean vkrend_renderer_pixel_is(const br_pixelmap *readback, br_int_32 x, br_int_32 y, const br_uint_8 expected[4])
{
    const br_uint_8 *pixel = (const br_uint_8 *)readback->pixels + y * readback->row_bytes + x * 4;

    return pixel[0] == expected[0] && pixel[1] == expected[1] && pixel[2] == expected[2] && pixel[3] == expected[3];
}

/* Read one image pixel's four bytes (B G R A) into out[4]. */
static br_error vkrend_renderer_read_pixel(br_pixelmap *offscreen, br_int_32 x, br_int_32 y, br_uint_8 out[4])
{
    br_pixelmap     *readback;
    const br_uint_8 *pixel;

    if((readback = vkrend_renderer_read(offscreen)) == NULL)
        return BRE_FAIL;

    pixel = (const br_uint_8 *)readback->pixels + y * readback->row_bytes + x * 4;

    out[0] = pixel[0];
    out[1] = pixel[1];
    out[2] = pixel[2];
    out[3] = pixel[3];

    BrPixelmapFree(readback);
    return BRE_OK;
}

/*
 * Assert one image pixel against expected[4], allowing each channel to differ by at most
 * `tolerance`. Used where the fragment value is an interpolated vertex colour or a sum the
 * rasteriser rounds, so an exact byte would be a claim about the rounding mode rather than
 * about the renderer. Coordinates are image (top-left) coordinates, as vkrend_renderer_read()
 * and vkrend_renderer_pixel_is() use.
 */
static br_error vkrend_renderer_assert_pixel_near(br_pixelmap *offscreen, br_int_32 x, br_int_32 y, const br_uint_8 expected[4],
                                                  br_int_32 tolerance, const char *what)
{
    br_pixelmap     *readback;
    const br_uint_8 *got;

    if((readback = vkrend_renderer_read(offscreen)) == NULL)
        return BRE_FAIL;

    got = (const br_uint_8 *)readback->pixels + y * readback->row_bytes + x * 4;

    for(int i = 0; i < 4; ++i) {
        br_int_32 diff = (br_int_32)got[i] - (br_int_32)expected[i];

        if(diff < 0)
            diff = -diff;

        if(diff > tolerance) {
            BrLogError("RT", "vkrend: %s pixel (%d,%d) channel %d: expected %02x +/- %d, got %02x (read %02x %02x %02x %02x B G R A).",
                       what, x, y, i, expected[i], tolerance, got[i], got[0], got[1], got[2], got[3]);
            BrPixelmapFree(readback);
            return BRE_FAIL;
        }
    }

    BrLogInfo("RT", "vkrend: %s pixel (%d,%d) readback ok: %02x %02x %02x %02x (B G R A).", what, x, y, got[0], got[1], got[2], got[3]);
    BrPixelmapFree(readback);
    return BRE_OK;
}

typedef struct vkrend_renderer_bounds {
    br_int_32 min_x, min_y, max_x, max_y;
    br_int_32 count;
} vkrend_renderer_bounds;

/* Find the bounding box and pixel count of every pixel matching expected[4]. */
static void vkrend_renderer_bounds_find(const br_pixelmap *readback, const br_uint_8 expected[4], vkrend_renderer_bounds *bounds)
{
    bounds->min_x = readback->width;
    bounds->min_y = readback->height;
    bounds->max_x = -1;
    bounds->max_y = -1;
    bounds->count = 0;

    for(br_int_32 y = 0; y < readback->height; ++y) {
        for(br_int_32 x = 0; x < readback->width; ++x) {
            if(!vkrend_renderer_pixel_is(readback, x, y, expected))
                continue;

            if(x < bounds->min_x)
                bounds->min_x = x;
            if(y < bounds->min_y)
                bounds->min_y = y;
            if(x > bounds->max_x)
                bounds->max_x = x;
            if(y > bounds->max_y)
                bounds->max_y = y;
            bounds->count++;
        }
    }
}

static br_error vkrend_renderer_assert_near(br_int_32 got, br_int_32 want, br_int_32 tolerance, const char *what)
{
    if(got < want - tolerance || got > want + tolerance) {
        BrLogError("RT", "vkrend: %s: expected %d +/- %d, got %d.", what, want, tolerance, got);
        return BRE_FAIL;
    }

    BrLogInfo("RT", "vkrend: %s: %d (expected %d +/- %d).", what, got, want, tolerance);
    return BRE_OK;
}

/*
 * Texels per world unit at a plane `distance` in front of a perspective camera, using the
 * same scale = cot(fov / 2) that BrMatrix4Perspective builds. This is the independent
 * oracle the transform steps compare against; it does not call the renderer.
 */
static br_scalar vkrend_renderer_pixels_per_unit(const br_pixelmap *offscreen, br_scalar fov, br_scalar distance)
{
    br_scalar scale = BR_DIV(BR_COS(fov / 2), BR_SIN(fov / 2));

    return BR_DIV(BR_MUL(scale, BR_SCALAR(offscreen->height)), BR_MUL(BR_SCALAR(2.0), distance));
}

/* The image pixel a view-space point (x, y, 0) projects to. */
static void vkrend_renderer_project(const br_pixelmap *offscreen, br_scalar fov, br_scalar distance, br_scalar x, br_scalar y,
                                    br_int_32 *out_x, br_int_32 *out_y)
{
    br_scalar pixels = vkrend_renderer_pixels_per_unit(offscreen, fov, distance);

    *out_x = (br_int_32)BR_ADD(BR_SCALAR(offscreen->width / 2), BR_MUL(x, pixels));
    *out_y = (br_int_32)BR_ADD(BR_SCALAR(offscreen->height / 2), BR_MUL(y, pixels));
}

/*
 * Build the one-triangle model of steps 1 and 2. Its single face carries the surface
 * material, so the prepared group has a stored state of its own as well as the actor
 * providing one.
 */
static br_error vkrend_renderer_build_triangle(vkrend_renderer_state *state)
{
    if((state->material = BrMaterialAllocate("vkrend-renderer")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the triangle material.");
        return BRE_FAIL;
    }

    state->material->colour  = BR_COLOUR_RGB(0xaa, 0xbb, 0xcc);
    state->material->opacity = 255;
    state->material->flags &= ~BR_MATF_LIGHT;
    BrMaterialUpdate(state->material, BR_MATU_ALL);

    /*
     * Wound counter-clockwise so its primary side faces the camera at +z.
     */
    if((state->model = BrModelAllocate("vkrend-renderer", 3, 1)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the triangle model.");
        return BRE_FAIL;
    }

    BrVector3Set(&state->model->vertices[0].p, BR_SCALAR(-1.0), BR_SCALAR(-1.0), BR_SCALAR(0.0));
    BrVector3Set(&state->model->vertices[1].p, BR_SCALAR(1.0), BR_SCALAR(-1.0), BR_SCALAR(0.0));
    BrVector3Set(&state->model->vertices[2].p, BR_SCALAR(0.0), BR_SCALAR(1.0), BR_SCALAR(0.0));

    state->model->faces[0].vertices[0] = 0;
    state->model->faces[0].vertices[1] = 1;
    state->model->faces[0].vertices[2] = 2;
    state->model->faces[0].material    = state->material;

    BrModelUpdate(state->model, BR_MODU_ALL);

    if(state->model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the triangle.");
        return BRE_FAIL;
    }

    return BRE_OK;
}

/*
 * Build the two-group model of step 3: two quads either side of the model origin, the
 * left two faces carrying the left material and the right two the right material. The
 * prepared model then has two groups with distinct stored surface states, and a group
 * drawn with the wrong vertex or index offset lands away from its asserted pixels.
 */
static br_error vkrend_renderer_build_multigroup(vkrend_renderer_state *state)
{
    static const br_scalar vertices[8][3] = {
        {-1.2, -0.6, 0.0},
        {-0.3, -0.6, 0.0},
        {-0.3, 0.6,  0.0},
        {-1.2, 0.6,  0.0},
        {0.3,  -0.6, 0.0},
        {1.2,  -0.6, 0.0},
        {1.2,  0.6,  0.0},
        {0.3,  0.6,  0.0},
    };
    static const br_uint_16 faces[4][3] = {
        {0, 1, 2},
        {0, 2, 3},
        {4, 5, 6},
        {4, 6, 7},
    };

    if((state->multi_left = BrMaterialAllocate("vkrend-renderer-left")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the left multi-group material.");
        return BRE_FAIL;
    }

    state->multi_left->colour  = BR_COLOUR_RGB(0xaa, 0xbb, 0xcc);
    state->multi_left->opacity = 255;
    state->multi_left->flags &= ~BR_MATF_LIGHT;
    BrMaterialUpdate(state->multi_left, BR_MATU_ALL);

    if((state->multi_right = BrMaterialAllocate("vkrend-renderer-right")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the right multi-group material.");
        return BRE_FAIL;
    }

    state->multi_right->colour  = BR_COLOUR_RGB(0x88, 0x44, 0x22);
    state->multi_right->opacity = 255;
    state->multi_right->flags &= ~BR_MATF_LIGHT;
    BrMaterialUpdate(state->multi_right, BR_MATU_ALL);

    if((state->multi_model = BrModelAllocate("vkrend-renderer-multigroup", 8, 4)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the multi-group model.");
        return BRE_FAIL;
    }

    for(int i = 0; i < 8; ++i)
        BrVector3Set(&state->multi_model->vertices[i].p, BR_SCALAR(vertices[i][0]), BR_SCALAR(vertices[i][1]), BR_SCALAR(vertices[i][2]));

    for(int i = 0; i < 4; ++i) {
        state->multi_model->faces[i].vertices[0] = faces[i][0];
        state->multi_model->faces[i].vertices[1] = faces[i][1];
        state->multi_model->faces[i].vertices[2] = faces[i][2];
        state->multi_model->faces[i].material    = i < 2 ? state->multi_left : state->multi_right;
    }

    BrModelUpdate(state->multi_model, BR_MODU_ALL);

    if(state->multi_model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the multi-group model.");
        return BRE_FAIL;
    }

    return BRE_OK;
}

/*
 * Build the two overlapping triangles of the Z-sort occlusion step: a far one at z = -1
 * whose face carries occlusion_far and a near one at z = +1 whose face carries
 * occlusion_near, both centred on the view axis so the image centre is inside each. The
 * prepared model groups faces by material, keyed by the first face to use it, so the far
 * triangle is group 0 and the near one group 1.
 *
 * The order table is BUCKET_SORT, the flag that makes BrZsOrderTablePrimitiveInsert sort a
 * bucket by primitive depth; without it the per-face split would have nothing to sort by.
 */
static br_error vkrend_renderer_build_occlusion(vkrend_renderer_state *state)
{
    static const br_scalar vertices[6][3] = {
        {-1.0, -1.0, -1.0},
        {1.0,  -1.0, -1.0},
        {0.0,  1.0,  -1.0},
        {-0.8, -0.8, 1.0 },
        {0.8,  -0.8, 1.0 },
        {0.0,  0.8,  1.0 },
    };
    static const br_uint_16 faces[2][3] = {
        {0, 1, 2},
        {3, 4, 5},
    };

    if((state->occlusion_far = BrMaterialAllocate("vkrend-renderer-occlusion-far")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the far occlusion material.");
        return BRE_FAIL;
    }

    state->occlusion_far->colour  = BR_COLOUR_RGB(0xff, 0x00, 0x00);
    state->occlusion_far->opacity = 255;
    state->occlusion_far->flags &= ~BR_MATF_LIGHT;
    BrMaterialUpdate(state->occlusion_far, BR_MATU_ALL);

    if((state->occlusion_near = BrMaterialAllocate("vkrend-renderer-occlusion-near")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the near occlusion material.");
        return BRE_FAIL;
    }

    state->occlusion_near->colour  = BR_COLOUR_RGB(0x00, 0x00, 0xff);
    state->occlusion_near->opacity = 255;
    state->occlusion_near->flags &= ~BR_MATF_LIGHT;
    BrMaterialUpdate(state->occlusion_near, BR_MATU_ALL);

    if((state->occlusion_model = BrModelAllocate("vkrend-renderer-occlusion", 6, 2)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the occlusion model.");
        return BRE_FAIL;
    }

    for(int i = 0; i < 6; ++i)
        BrVector3Set(&state->occlusion_model->vertices[i].p, BR_SCALAR(vertices[i][0]), BR_SCALAR(vertices[i][1]), BR_SCALAR(vertices[i][2]));

    for(int i = 0; i < 2; ++i) {
        state->occlusion_model->faces[i].vertices[0] = faces[i][0];
        state->occlusion_model->faces[i].vertices[1] = faces[i][1];
        state->occlusion_model->faces[i].vertices[2] = faces[i][2];
        state->occlusion_model->faces[i].material    = i == 0 ? state->occlusion_far : state->occlusion_near;
    }

    BrModelUpdate(state->occlusion_model, BR_MODU_ALL);

    if(state->occlusion_model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the occlusion model.");
        return BRE_FAIL;
    }

    /*
     * The same two triangles with the group order reversed: the near face carries the near
     * material first, so the prepared model's group 0 is the near one. The Zb depth step
     * renders both models and must read the near face either way, which is what makes the
     * result depth rather than painter order.
     */
    if((state->depth_rev_model = BrModelAllocate("vkrend-renderer-depth-rev", 6, 2)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the reversed depth model.");
        return BRE_FAIL;
    }

    for(int i = 0; i < 6; ++i)
        BrVector3Set(&state->depth_rev_model->vertices[i].p, BR_SCALAR(vertices[i][0]), BR_SCALAR(vertices[i][1]), BR_SCALAR(vertices[i][2]));

    for(int i = 0; i < 2; ++i) {
        /*
         * The reversed model carries the near triangle - geometry and material together - as
         * face 0 and the far triangle as face 1, so the prepared model's group 0 is the near
         * one. Swapping only the materials would leave the far geometry under the near
         * colour, which is not the same scene in a different draw order.
         */
        const br_uint_16 *fv = faces[1 - i];

        state->depth_rev_model->faces[i].vertices[0] = fv[0];
        state->depth_rev_model->faces[i].vertices[1] = fv[1];
        state->depth_rev_model->faces[i].vertices[2] = fv[2];
        state->depth_rev_model->faces[i].material    = i == 0 ? state->occlusion_near : state->occlusion_far;
    }

    BrModelUpdate(state->depth_rev_model, BR_MODU_ALL);

    if(state->depth_rev_model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the reversed depth model.");
        return BRE_FAIL;
    }

    if((state->occlusion_order_table = BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_LEAVE_BOUNDS | BR_ORDER_TABLE_BUCKET_SORT, BR_SORT_AVERAGE)) ==
       NULL) {
        BrLogError("RT", "vkrend: could not allocate the occlusion order table.");
        return BRE_FAIL;
    }

    return BRE_OK;
}

/*
 * Build a one-triangle model whose single face carries a white surface with the given
 * ambient/diffuse coefficients, lit or unlit. It is the same triangle steps 1 and 2 use,
 * wound so its face normal is +z, which is what the lighting steps' predictions assume.
 */
static br_error vkrend_renderer_build_surface(const char *name, br_scalar ka, br_scalar kd, br_boolean lit, br_material **out_material,
                                              br_model **out_model)
{
    br_material *material;
    br_model    *model;

    if((material = BrMaterialAllocate(name)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the %s material.", name);
        return BRE_FAIL;
    }

    material->colour  = BR_COLOUR_RGB(255, 255, 255);
    material->opacity = 255;
    material->ka      = ka;
    material->kd      = kd;
    material->ks      = BR_SCALAR(0.0);

    if(!lit)
        material->flags &= ~BR_MATF_LIGHT;

    BrMaterialUpdate(material, BR_MATU_ALL);

    if((model = BrModelAllocate(name, 3, 1)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the %s model.", name);
        BrMaterialFree(material);
        return BRE_FAIL;
    }

    BrVector3Set(&model->vertices[0].p, BR_SCALAR(-1.0), BR_SCALAR(-1.0), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[1].p, BR_SCALAR(1.0), BR_SCALAR(-1.0), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[2].p, BR_SCALAR(0.0), BR_SCALAR(1.0), BR_SCALAR(0.0));

    model->faces[0].vertices[0] = 0;
    model->faces[0].vertices[1] = 1;
    model->faces[0].vertices[2] = 2;
    model->faces[0].material    = material;

    BrModelUpdate(model, BR_MODU_ALL);

    if(model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the %s model.", name);
        BrModelFree(model);
        BrMaterialFree(material);
        return BRE_FAIL;
    }

    *out_material = material;
    *out_model    = model;
    return BRE_OK;
}

/*
 * Build the textured quad both texture steps draw: four vertices spanning model
 * (-1,-1)..(1,1), wound so each screen quadrant samples one texel of a 2x2 map. Both faces
 * carry `material`.
 */
static br_error vkrend_renderer_build_textured_quad(const char *name, br_material *material, br_model **out_model)
{
    static const br_scalar positions[4][2] = {
        {-1.0, 1.0 },
        {-1.0, -1.0},
        {1.0,  1.0 },
        {1.0,  -1.0}
    };
    static const br_scalar maps[4][2] = {
        {0.0, 0.0},
        {0.0, 1.0},
        {1.0, 0.0},
        {1.0, 1.0}
    };
    static const br_uint_16 faces[2][3] = {
        {0, 1, 2},
        {2, 1, 3}
    };
    br_model *model;

    if((model = BrModelAllocate(name, 4, 2)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the %s model.", name);
        return BRE_FAIL;
    }

    for(int i = 0; i < 4; ++i) {
        BrVector3Set(&model->vertices[i].p, BR_SCALAR(positions[i][0]), BR_SCALAR(positions[i][1]), BR_SCALAR(0.0));
        BrVector2Set(&model->vertices[i].map, BR_SCALAR(maps[i][0]), BR_SCALAR(maps[i][1]));
    }

    for(int i = 0; i < 2; ++i) {
        model->faces[i].vertices[0] = faces[i][0];
        model->faces[i].vertices[1] = faces[i][1];
        model->faces[i].vertices[2] = faces[i][2];
        model->faces[i].material    = material;
    }

    BrModelUpdate(model, BR_MODU_ALL);

    if(model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the %s model.", name);
        BrModelFree(model);
        return BRE_FAIL;
    }

    *out_model = model;
    return BRE_OK;
}

/*
 * Build the textured quad of step 5 (stage 6). Its colour map is 2x2 with four distinct
 * corner colours - red top-left, green top-right, blue bottom-left, black bottom-right -
 * and each corner's map coordinate names that corner, so the interpolated UVs pick a texel
 * per quadrant. The material is unlit white, so an untextured draw would be a flat white
 * quad and any assertion below would fail on the colour alone. The black corner is what the
 * colour-key case discards.
 */
static br_error vkrend_renderer_build_texture(vkrend_renderer_state *state)
{
    if((state->texture = BrPixelmapAllocate(BR_PMT_RGBA_8888, 2, 2, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the texture map.");
        return BRE_FAIL;
    }

    BrPixelmapPixelSet(state->texture, 0, 0, BR_COLOUR_RGB(255, 0, 0));
    BrPixelmapPixelSet(state->texture, 1, 0, BR_COLOUR_RGB(0, 255, 0));
    BrPixelmapPixelSet(state->texture, 0, 1, BR_COLOUR_RGB(0, 0, 255));
    BrPixelmapPixelSet(state->texture, 1, 1, BR_COLOUR_RGB(0, 0, 0));

    /*
     * The map's stored buffer is what the material's primitive state points at, so it has
     * to exist before BrMaterialUpdate() reads it.
     */
    BrMapUpdate(state->texture, BR_MAPU_ALL);

    if(state->texture->stored == NULL) {
        BrLogError("RT", "vkrend: BrMapUpdate() produced no stored buffer for the texture map.");
        return BRE_FAIL;
    }

    if((state->texture_material = BrMaterialAllocate("vkrend-renderer-texture")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the textured material.");
        return BRE_FAIL;
    }

    state->texture_material->colour     = BR_COLOUR_RGB(255, 255, 255);
    state->texture_material->opacity    = 255;
    state->texture_material->colour_map = state->texture;
    state->texture_material->flags &= ~BR_MATF_LIGHT;
    BrMatrix23Identity(&state->texture_material->map_transform);
    BrMaterialUpdate(state->texture_material, BR_MATU_ALL);

    return vkrend_renderer_build_textured_quad("vkrend-renderer-texture", state->texture_material, &state->texture_model);
}

/*
 * How many map repeats the mip witness spans across the quad. The quad's map coordinate runs
 * 0..1, so a scale of 128 makes a screen pixel span about eight texels of the 16x16 map and
 * puts the LOD near 3 - deep enough into the chain that every selected level is the map's
 * purple average, at any offscreen up to several thousand pixels tall.
 */
#define VKREND_MIP_MAP_SCALE 128

/*
 * Build the mip/filter witness's colour map and material.
 *
 * The map is a 16x16 one-texel red/blue checkerboard. Every mip level above 0 averages the
 * level above it, and each 2x2 block of a checkerboard holds two red and two blue texels, so
 * every level below 0 is the same 50/50 purple (0x80, 0x00, 0x80). A sample that reaches the
 * chain therefore reads purple, while a sample held at level 0 reads exactly one of the two
 * hard texels whatever the sub-pixel phase the nearest sampler lands on. The material is
 * unlit white with the colour key off, so the texel reaches the readback unmodulated.
 */
static br_error vkrend_renderer_build_mipmap(vkrend_renderer_state *state)
{
    const br_uint_32 size = 16;

    if((state->mip_texture = BrPixelmapAllocate(BR_PMT_RGBA_8888, size, size, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the mip witness map.");
        return BRE_FAIL;
    }

    for(br_uint_32 y = 0; y < size; ++y) {
        for(br_uint_32 x = 0; x < size; ++x)
            BrPixelmapPixelSet(state->mip_texture, x, y, ((x + y) & 1) ? BR_COLOUR_RGB(255, 0, 0) : BR_COLOUR_RGB(0, 0, 255));
    }

    BrMapUpdate(state->mip_texture, BR_MAPU_ALL);

    if(state->mip_texture->stored == NULL) {
        BrLogError("RT", "vkrend: BrMapUpdate() produced no stored buffer for the mip witness map.");
        return BRE_FAIL;
    }

    if((state->mip_material = BrMaterialAllocate("vkrend-renderer-mip")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the mip witness material.");
        return BRE_FAIL;
    }

    state->mip_material->colour     = BR_COLOUR_RGB(255, 255, 255);
    state->mip_material->opacity    = 255;
    state->mip_material->colour_map = state->mip_texture;
    state->mip_material->flags &= ~BR_MATF_LIGHT;
    state->mip_material->flags |= BR_MATF_DISABLE_COLOUR_KEY;

    BrMatrix23Identity(&state->mip_material->map_transform);
    state->mip_material->map_transform.m[0][0] = BR_SCALAR(VKREND_MIP_MAP_SCALE);
    state->mip_material->map_transform.m[1][1] = BR_SCALAR(VKREND_MIP_MAP_SCALE);
    BrMaterialUpdate(state->mip_material, BR_MATU_ALL);

    /*
     * The quad's faces carry their material, so the mip map needs its own model rather than
     * reusing the step-5 quad (whose faces name the 2x2 texture).
     */
    return vkrend_renderer_build_textured_quad("vkrend-renderer-mip", state->mip_material, &state->mip_model);
}

/*
 * Build the filter witness's colour map and material.
 *
 * The map is 2x2 with a red left column and a green right column, identical in both rows, so
 * a bilinear sample across the column boundary blends the two colours whatever its vertical
 * phase - the value is a function of u alone, and the boundary is the image column through
 * the quad centre. The material is unlit white with the colour key off and an identity map
 * transform, so the quad magnifies the map and the nearest path reads one hard texel.
 */
static br_error vkrend_renderer_build_filter_texture(vkrend_renderer_state *state)
{
    if((state->filter_texture = BrPixelmapAllocate(BR_PMT_RGBA_8888, 2, 2, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the filter witness map.");
        return BRE_FAIL;
    }

    BrPixelmapPixelSet(state->filter_texture, 0, 0, BR_COLOUR_RGB(255, 0, 0));
    BrPixelmapPixelSet(state->filter_texture, 1, 0, BR_COLOUR_RGB(0, 255, 0));
    BrPixelmapPixelSet(state->filter_texture, 0, 1, BR_COLOUR_RGB(255, 0, 0));
    BrPixelmapPixelSet(state->filter_texture, 1, 1, BR_COLOUR_RGB(0, 255, 0));

    BrMapUpdate(state->filter_texture, BR_MAPU_ALL);

    if(state->filter_texture->stored == NULL) {
        BrLogError("RT", "vkrend: BrMapUpdate() produced no stored buffer for the filter witness map.");
        return BRE_FAIL;
    }

    if((state->filter_material = BrMaterialAllocate("vkrend-renderer-filter")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the filter witness material.");
        return BRE_FAIL;
    }

    state->filter_material->colour     = BR_COLOUR_RGB(255, 255, 255);
    state->filter_material->opacity    = 255;
    state->filter_material->colour_map = state->filter_texture;
    state->filter_material->flags &= ~BR_MATF_LIGHT;
    state->filter_material->flags |= BR_MATF_DISABLE_COLOUR_KEY;
    BrMatrix23Identity(&state->filter_material->map_transform);
    BrMaterialUpdate(state->filter_material, BR_MATU_ALL);

    return vkrend_renderer_build_textured_quad("vkrend-renderer-filter", state->filter_material, &state->filter_model);
}

/*
 * Build the environment-map step's map, material and model. The map is 16x16 with a distinct
 * colour per texel that encodes its coordinates - texel (tx, ty) is RGB(tx * 16, ty * 16,
 * 0x80) - so the byte the step reads back names the texel a reflection sampled. The model is
 * one large triangle in the model z = 0 plane, wound so its face is +z, with a constant
 * custom normal of (1, 1, 1)/sqrt(3) on all three vertices: a constant normal keeps the
 * infinite eye's reflection the same across the triangle, and the custom value is what the
 * environment fragment stage reflects.
 *
 * The one material has an unlit white surface, so the texel reaches the readback unmodulated.
 * The step sets BR_MATF_ENVIRONMENT_I / _L on it and clears both again, which the core
 * publishes as the surface's mapping source (core/v1db/prepmatl.c) each time the material is
 * updated; with neither flag the source stays BRT_GEOMETRY_MAP and the plain vertex map
 * coordinate is sampled.
 */
static br_error vkrend_renderer_build_environment(vkrend_renderer_state *state)
{
    br_vector3 normal;
    br_model  *model;

    if((state->env_texture = BrPixelmapAllocate(BR_PMT_RGBA_8888, VKREND_ENV_TEXTURE_SIZE, VKREND_ENV_TEXTURE_SIZE, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the environment map.");
        return BRE_FAIL;
    }

    for(int ty = 0; ty < VKREND_ENV_TEXTURE_SIZE; ++ty) {
        for(int tx = 0; tx < VKREND_ENV_TEXTURE_SIZE; ++tx)
            BrPixelmapPixelSet(state->env_texture, tx, ty, BR_COLOUR_RGB((br_uint_8)(tx * 16), (br_uint_8)(ty * 16), 0x80));
    }

    /* The stored buffer is what the material's primitive state points at, so it precedes the update. */
    BrMapUpdate(state->env_texture, BR_MAPU_ALL);

    if(state->env_texture->stored == NULL) {
        BrLogError("RT", "vkrend: BrMapUpdate() produced no stored buffer for the environment map.");
        return BRE_FAIL;
    }

    if((state->env_material = BrMaterialAllocate("vkrend-renderer-env")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the environment material.");
        return BRE_FAIL;
    }

    state->env_material->colour     = BR_COLOUR_RGB(255, 255, 255);
    state->env_material->opacity    = 255;
    state->env_material->colour_map = state->env_texture;
    state->env_material->flags &= ~BR_MATF_LIGHT;
    BrMatrix23Identity(&state->env_material->map_transform);
    BrMaterialUpdate(state->env_material, BR_MATU_ALL);

    if((model = BrModelAllocate("vkrend-renderer-env", 3, 1)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the environment model.");
        return BRE_FAIL;
    }

    BrVector3Set(&model->vertices[0].p, BR_SCALAR(-3.0), BR_SCALAR(-2.0), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[1].p, BR_SCALAR(3.0), BR_SCALAR(-2.0), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[2].p, BR_SCALAR(0.0), BR_SCALAR(4.0), BR_SCALAR(0.0));

    /*
     * The custom normal replaces the face normal the core would otherwise generate; the
     * oracle reflects the same authored value, so the two agree by construction.
     */
    model->flags |= BR_MODF_CUSTOM_NORMALS;

    BrVector3Set(&normal, BR_SCALAR(1.0), BR_SCALAR(1.0), BR_SCALAR(1.0));
    BrVector3Normalise(&normal, &normal);

    for(int i = 0; i < 3; ++i) {
        model->vertices[i].n.v[0] = BrScalarToFraction(normal.v[0]);
        model->vertices[i].n.v[1] = BrScalarToFraction(normal.v[1]);
        model->vertices[i].n.v[2] = BrScalarToFraction(normal.v[2]);
    }

    model->faces[0].vertices[0] = 0;
    model->faces[0].vertices[1] = 1;
    model->faces[0].vertices[2] = 2;
    model->faces[0].material    = state->env_material;

    BrModelUpdate(model, BR_MODU_ALL);

    if(model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the environment model.");
        BrModelFree(model);
        return BRE_FAIL;
    }

    state->env_model = model;
    return BRE_OK;
}

/*
 * The independent oracle for the environment-map step: the texel a reflection samples, from
 * the same inputs without the renderer. The incident vector is the view-space eye direction -
 * the fragment for the local eye and the model origin (the camera at +z, so (0, 0, -distance))
 * for the infinite eye - and the reflected vector is GLSL's reflect() against the normalised
 * custom normal. The environment anchor's VKREND_ENV_ROT_DEG Y rotation carries the reflection
 * into the environment frame as the transpose of that rotation (view_to_environment is the
 * length-preserving inverse), and the sphere map is softrend's:
 *
 *     cu = atan2(wr.x, -wr.z) / (2*PI)      cv = 0.5 - wr.y / 2
 *
 * (drivers/softrend/mapping.c SurfaceMapEnvironment). BrAngleToScalar() is the identity, so cu
 * carries no 0.5 offset - only cv does. The incident points from the eye towards the fragment,
 * i.e. -E, which is what makes reflect() produce softrend's r; glrend's shader passes E and
 * adds 0.5 to cu to compensate the pi that flips the ray through the origin, so the 0.5 is
 * glrend's, not softrend's. The result names the 16x16 texel a NEAREST fetch would choose, and
 * the map's texel (tx, ty) is RGB(tx * 16, ty * 16, 0x80), so the readback is
 * (0x80, ty * 16, tx * 16, 0xff).
 */
static void vkrend_renderer_environment_oracle(br_scalar pixels, br_scalar distance, br_int_32 origin_x, br_int_32 origin_y, br_int_32 x,
                                               br_int_32 y, br_boolean infinite, br_uint_8 out_colour[4])
{
    const br_scalar angle = BR_ANGLE_DEG(VKREND_ENV_ROT_DEG);
    const br_scalar c     = BR_COS(angle);
    const br_scalar s     = BR_SIN(angle);
    br_vector3      normal, incident, r, wr;
    br_scalar       len, d, u, v;
    br_int_32       tx, ty;

    BrVector3Set(&normal, BR_SCALAR(1.0), BR_SCALAR(1.0), BR_SCALAR(1.0));
    BrVector3Normalise(&normal, &normal);

    if(infinite) {
        /* The model origin in view space, normalised: the camera is at (0, 0, distance) looking down -z. */
        BrVector3Set(&incident, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(-1.0));
    } else {
        /* The fragment's view position from the pixel centre, at the model z = 0 plane's view depth. */
        BrVector3Set(&incident, BR_DIV(BR_SUB(BR_ADD(BR_SCALAR(x), BR_SCALAR(0.5)), BR_SCALAR(origin_x)), pixels),
                     BR_NEG(BR_DIV(BR_SUB(BR_ADD(BR_SCALAR(y), BR_SCALAR(0.5)), BR_SCALAR(origin_y)), pixels)), BR_NEG(distance));
        BrVector3Normalise(&incident, &incident);
    }

    /* GLSL's reflect(incident, normal) = incident - 2 * dot(normal, incident) * normal. */
    d = BR_MAC3(normal.v[0], incident.v[0], normal.v[1], incident.v[1], normal.v[2], incident.v[2]);
    BrVector3Set(&r, BR_SUB(incident.v[0], BR_MUL(BR_MUL(BR_SCALAR(2.0), d), normal.v[0])),
                 BR_SUB(incident.v[1], BR_MUL(BR_MUL(BR_SCALAR(2.0), d), normal.v[1])),
                 BR_SUB(incident.v[2], BR_MUL(BR_MUL(BR_SCALAR(2.0), d), normal.v[2])));

    /* r * transpose(rotateY(angle)): view_to_environment is the rotation's length-preserving inverse. */
    BrVector3Set(&wr, BR_SUB(BR_MUL(c, r.v[0]), BR_MUL(s, r.v[2])), r.v[1], BR_ADD(BR_MUL(s, r.v[0]), BR_MUL(c, r.v[2])));
    BrVector3Normalise(&wr, &wr);

    len = BR_ATAN2(wr.v[0], BR_NEG(wr.v[2]));
    u   = len;
    v   = BR_SUB(BR_SCALAR(0.5), BR_MUL(BR_SCALAR(0.5), wr.v[1]));

    tx = (br_int_32)BR_MUL(u, BR_SCALAR(VKREND_ENV_TEXTURE_SIZE));
    ty = (br_int_32)BR_MUL(v, BR_SCALAR(VKREND_ENV_TEXTURE_SIZE));

    out_colour[0] = 0x80;
    out_colour[1] = (br_uint_8)(ty * 16);
    out_colour[2] = (br_uint_8)(tx * 16);
    out_colour[3] = 0xff;
}

/*
 * Build the indexed texture the indexed-colour-map step draws. It is a 2x2 INDEX_8 map
 * whose four texels name palette rows 1, 2, 3 and 0: three distinct colours and the
 * transparent entry 0. The palette is a 1x256 RGBX_888 plane, so an index lookup that used
 * the wrong axis, the wrong row or a wrong channel order would recolour or move a quadrant.
 */
static br_error vkrend_renderer_build_indexed_texture(vkrend_renderer_state *state)
{
    if((state->indexed_palette = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the indexed texture's palette.");
        return BRE_FAIL;
    }

    for(int i = 0; i < state->indexed_palette->height; ++i)
        BrPixelmapPixelSet(state->indexed_palette, 0, i, BR_COLOUR_RGB((br_uint_8)(i * 7), (br_uint_8)(i * 13), (br_uint_8)(i * 29)));

    /* The entries the step asserts: 1, 2 and 3 are the coloured quadrants, 0 the keyed one. */
    BrPixelmapPixelSet(state->indexed_palette, 0, 0, BR_COLOUR_RGB(0x11, 0x22, 0x33));
    BrPixelmapPixelSet(state->indexed_palette, 0, 1, BR_COLOUR_RGB(0xff, 0x00, 0x00));
    BrPixelmapPixelSet(state->indexed_palette, 0, 2, BR_COLOUR_RGB(0x00, 0xff, 0x00));
    BrPixelmapPixelSet(state->indexed_palette, 0, 3, BR_COLOUR_RGB(0x00, 0x00, 0xff));

    if((state->indexed_texture = BrPixelmapAllocate(BR_PMT_INDEX_8, 2, 2, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the indexed texture map.");
        return BRE_FAIL;
    }

    BrPixelmapPixelSet(state->indexed_texture, 0, 0, 1);
    BrPixelmapPixelSet(state->indexed_texture, 1, 0, 2);
    BrPixelmapPixelSet(state->indexed_texture, 0, 1, 3);
    BrPixelmapPixelSet(state->indexed_texture, 1, 1, 0);

    /* The palette is the index map's map; BrMapUpdate uploads both planes. */
    state->indexed_texture->map = state->indexed_palette;
    BrMapUpdate(state->indexed_texture, BR_MAPU_ALL);

    if(state->indexed_texture->stored == NULL) {
        BrLogError("RT", "vkrend: BrMapUpdate() produced no stored buffer for the indexed texture map.");
        return BRE_FAIL;
    }

    if((state->indexed_material = BrMaterialAllocate("vkrend-renderer-indexed-texture")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the indexed textured material.");
        return BRE_FAIL;
    }

    state->indexed_material->colour     = BR_COLOUR_RGB(255, 255, 255);
    state->indexed_material->opacity    = 255;
    state->indexed_material->colour_map = state->indexed_texture;
    state->indexed_material->flags &= ~BR_MATF_LIGHT;
    BrMatrix23Identity(&state->indexed_material->map_transform);
    BrMaterialUpdate(state->indexed_material, BR_MATU_ALL);

    return vkrend_renderer_build_textured_quad("vkrend-renderer-indexed-texture", state->indexed_material, &state->indexed_model);
}

/*
 * Build the render-to-texture material the device-colour-map step draws. A 2x2 RGBA_8888
 * offscreen is filled with the same red/green/blue/black quadrants as the texture step and
 * registered as a map, so its stored buffer samples the device image in place rather than
 * uploading a host copy. A render-to-texture that fell back - a host upload of an offscreen
 * with no pixels, or BRE_FAIL - would leave the quad untextured and fail the assertions.
 */
static br_error vkrend_renderer_build_render_to_texture(vkrend_renderer_state *state)
{
    br_rectangle rect = {.x = 0, .y = 0, .w = 1, .h = 1};

    if((state->rt_texture = BrPixelmapMatchTypedSized(state->screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBA_8888, 2, 2)) == NULL) {
        BrLogError("RT", "vkrend: could not create the render-to-texture offscreen.");
        return BRE_FAIL;
    }

    /* BrPixelmapMatch() copies the front screen's origin, so give this offscreen its own. */
    state->rt_texture->origin_x = 0;
    state->rt_texture->origin_y = 0;

    if(DevicePixelmapRectangleFill(state->rt_texture, &rect, BR_COLOUR_RGB(255, 0, 0)) != BRE_OK)
        return BRE_FAIL;

    rect.x = 1;
    if(DevicePixelmapRectangleFill(state->rt_texture, &rect, BR_COLOUR_RGB(0, 255, 0)) != BRE_OK)
        return BRE_FAIL;

    rect.x = 0;
    rect.y = 1;
    if(DevicePixelmapRectangleFill(state->rt_texture, &rect, BR_COLOUR_RGB(0, 0, 255)) != BRE_OK)
        return BRE_FAIL;

    rect.x = 1;
    if(DevicePixelmapRectangleFill(state->rt_texture, &rect, BR_COLOUR_RGB(0, 0, 0)) != BRE_OK)
        return BRE_FAIL;

    /* Register the offscreen as a map: the stored buffer samples its device image in place. */
    BrMapUpdate(state->rt_texture, BR_MAPU_ALL);

    if(state->rt_texture->stored == NULL) {
        BrLogError("RT", "vkrend: BrMapUpdate() produced no stored buffer for the render-to-texture offscreen.");
        return BRE_FAIL;
    }

    if((state->rt_material = BrMaterialAllocate("vkrend-renderer-render-to-texture")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the render-to-texture material.");
        return BRE_FAIL;
    }

    state->rt_material->colour     = BR_COLOUR_RGB(255, 255, 255);
    state->rt_material->opacity    = 255;
    state->rt_material->colour_map = state->rt_texture;
    state->rt_material->flags &= ~BR_MATF_LIGHT;
    BrMatrix23Identity(&state->rt_material->map_transform);
    BrMaterialUpdate(state->rt_material, BR_MATU_ALL);

    return vkrend_renderer_build_textured_quad("vkrend-renderer-render-to-texture", state->rt_material, &state->rt_model);
}

/*
 * Build one triangle from the step 1 positions with the given per-vertex colours, each a
 * red, grn, blu and index quad - the index is what the prepared group carries as the
 * vertex colour's alpha (core/v1db/prepmesh.c) - and face material.
 */
static br_error vkrend_renderer_build_coloured_triangle(const char *name, const br_uint_8 colours[3][4], br_material *material, br_model **out_model)
{
    br_model *model;

    if((model = BrModelAllocate(name, 3, 1)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the %s model.", name);
        return BRE_FAIL;
    }

    BrVector3Set(&model->vertices[0].p, BR_SCALAR(-1.0), BR_SCALAR(-1.0), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[1].p, BR_SCALAR(1.0), BR_SCALAR(-1.0), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[2].p, BR_SCALAR(0.0), BR_SCALAR(1.0), BR_SCALAR(0.0));

    for(int i = 0; i < 3; ++i) {
        model->vertices[i].red   = colours[i][0];
        model->vertices[i].grn   = colours[i][1];
        model->vertices[i].blu   = colours[i][2];
        model->vertices[i].index = colours[i][3];
    }

    model->faces[0].vertices[0] = 0;
    model->faces[0].vertices[1] = 1;
    model->faces[0].vertices[2] = 2;
    model->faces[0].material    = material;

    BrModelUpdate(model, BR_MODU_ALL);

    if(model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the %s model.", name);
        BrModelFree(model);
        return BRE_FAIL;
    }

    *out_model = model;
    return BRE_OK;
}

/*
 * Build the two triangles the vertex-colour steps use. Both are the step 1 triangle. The
 * first carries distinct red and blue vertex colours and a BR_MATF_PRELIT-only material,
 * whose colour_source the core sets to BRT_GEOMETRY so the fragments take the interpolated
 * vertex colour rather than the surface colour. The second carries a uniform pre-lit
 * colour and a BR_MATF_PRELIT|BR_MATF_LIGHT material, for which the core writes a lighting
 * result into the vertex colours and the fragment stage adds it to the shaded surface.
 */
static br_error vkrend_renderer_build_vertex_colours(vkrend_renderer_state *state)
{
    static const br_uint_8 vc_colours[3][4] = {
        {255, 0, 0,   255},
        {255, 0, 0,   255},
        {0,   0, 255, 255},
    };
    static const br_uint_8 prelit_colours[3][4] = {
        {64, 0, 0, 255},
        {64, 0, 0, 255},
        {64, 0, 0, 255},
    };

    if((state->vc_material = BrMaterialAllocate("vkrend-renderer-vertex-colour")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the vertex-colour material.");
        return BRE_FAIL;
    }

    state->vc_material->colour  = BR_COLOUR_RGB(0, 255, 0);
    state->vc_material->opacity = 255;
    state->vc_material->flags |= BR_MATF_PRELIT;
    state->vc_material->flags &= ~BR_MATF_LIGHT;
    BrMaterialUpdate(state->vc_material, BR_MATU_ALL);

    if(vkrend_renderer_build_coloured_triangle("vkrend-renderer-vertex-colour", vc_colours, state->vc_material, &state->vc_model) != BRE_OK)
        return BRE_FAIL;

    if((state->prelit_material = BrMaterialAllocate("vkrend-renderer-prelit")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the pre-lit material.");
        return BRE_FAIL;
    }

    state->prelit_material->colour  = BR_COLOUR_RGB(255, 255, 255);
    state->prelit_material->opacity = 255;
    state->prelit_material->ka      = BR_SCALAR(0.25);
    state->prelit_material->kd      = BR_SCALAR(0.5);
    state->prelit_material->ks      = BR_SCALAR(0.0);
    state->prelit_material->flags |= BR_MATF_PRELIT;
    BrMaterialUpdate(state->prelit_material, BR_MATU_ALL);

    return vkrend_renderer_build_coloured_triangle("vkrend-renderer-prelit", prelit_colours, state->prelit_material, &state->prelit_model);
}

/*
 * Build the smooth-lit triangle the shading-mode step draws. It is the step 1 triangle,
 * wound so its face normal is +z, with BR_MODF_CUSTOM_NORMALS and a distinct normal per
 * vertex: the two bottom vertices lean left and right of the +z light and the top one
 * points along +y, so a vertex is lit differently from its neighbours. The material is a
 * lit white surface; the step sets its shading mode before each render.
 */
static br_error vkrend_renderer_build_smooth_lit(vkrend_renderer_state *state)
{
    br_vector3   normals[3];
    br_material *material;
    br_model    *model;

    BrVector3Set(&normals[0], BR_SCALAR(-0.4), BR_SCALAR(0.0), BR_SCALAR(1.0));
    BrVector3Set(&normals[1], BR_SCALAR(0.6), BR_SCALAR(0.0), BR_SCALAR(1.0));
    BrVector3Set(&normals[2], BR_SCALAR(0.0), BR_SCALAR(1.0), BR_SCALAR(0.0));

    if((material = BrMaterialAllocate("vkrend-renderer-smooth")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the smooth-lit material.");
        return BRE_FAIL;
    }

    material->colour  = BR_COLOUR_RGB(255, 255, 255);
    material->opacity = 255;
    material->ka      = BR_SCALAR(0.25);
    material->kd      = BR_SCALAR(0.5);
    material->ks      = BR_SCALAR(0.0);
    BrMaterialUpdate(material, BR_MATU_ALL);

    if((model = BrModelAllocate("vkrend-renderer-smooth", 3, 1)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the smooth-lit model.");
        BrMaterialFree(material);
        return BRE_FAIL;
    }

    BrVector3Set(&model->vertices[0].p, BR_SCALAR(-1.0), BR_SCALAR(-1.0), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[1].p, BR_SCALAR(1.0), BR_SCALAR(-1.0), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[2].p, BR_SCALAR(0.0), BR_SCALAR(1.0), BR_SCALAR(0.0));

    /*
     * The custom normals replace the face normal the core would otherwise generate, and the
     * shading-mode oracle reads the authored values back out of the state, so the two agree
     * by construction and survive BrModelUpdate() freeing the model's CPU vertices (a
     * stored-only model keeps only the prepared copy). The vertex normal is a fraction
     * vector, so the normalised scalar vector is converted component-wise.
     */
    model->flags |= BR_MODF_CUSTOM_NORMALS;

    for(int i = 0; i < 3; ++i) {
        br_vector3 unit;

        BrVector3Normalise(&unit, &normals[i]);
        model->vertices[i].n.v[0] = BrScalarToFraction(unit.v[0]);
        model->vertices[i].n.v[1] = BrScalarToFraction(unit.v[1]);
        model->vertices[i].n.v[2] = BrScalarToFraction(unit.v[2]);

        state->smooth_positions[i] = model->vertices[i].p;
        state->smooth_normals[i]   = unit;
    }

    model->faces[0].vertices[0] = 0;
    model->faces[0].vertices[1] = 1;
    model->faces[0].vertices[2] = 2;
    model->faces[0].material    = material;

    BrModelUpdate(model, BR_MODU_ALL);

    if(model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the smooth-lit model.");
        BrModelFree(model);
        BrMaterialFree(material);
        return BRE_FAIL;
    }

    state->smooth_model    = model;
    state->smooth_material = material;
    return BRE_OK;
}

/*
 * The volume witness's cutoff volume: a single region bounded by the plane x <= 0, whose
 * normal (-1, 0, 0) points inwards. The light actor sits at identity, so the core's
 * this_to_view transform - a translation along z - leaves the plane exactly as written.
 * Fragments left of the triangle's centre (view x < 0) are inside and lit in full; the
 * contribution falls linearly to nothing over VKREND_VOLUME_FALLOFF units to the right.
 */
#define VKREND_VOLUME_FALLOFF BR_SCALAR(0.5)
static br_vector4       vkrend_renderer_volume_planes[1]  = {{{BR_SCALAR(-1.0), BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(0.0)}}};
static br_convex_region vkrend_renderer_volume_regions[1] = {
    {vkrend_renderer_volume_planes, 1}
};

/*
 * Add the lights the lighting step uses: a white ambient light, a white directional light
 * pointing down +z in view space, a white point light 10 world units in front of the origin
 * with a 20-unit radius, and a second white directional light carrying a cutoff volume for
 * the volume witness. Only the ambient and directional lights start enabled.
 */
static br_error vkrend_renderer_build_lights(vkrend_renderer_state *state)
{
    br_light *light;

    if((state->ambient_light = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL))) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the ambient light.");
        return BRE_FAIL;
    }

    light                = state->ambient_light->type_data;
    light->type          = BR_LIGHT_AMBIENT;
    light->colour        = BR_COLOUR_RGB(255, 255, 255);
    light->attenuation_c = BR_SCALAR(1.0);
    BrLightEnable(state->ambient_light);

    if((state->direct_light = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL))) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the directional light.");
        return BRE_FAIL;
    }

    light                = state->direct_light->type_data;
    light->type          = BR_LIGHT_DIRECT;
    light->colour        = BR_COLOUR_RGB(255, 255, 255);
    light->attenuation_c = BR_SCALAR(1.0);
    BrLightEnable(state->direct_light);

    if((state->point_light = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL))) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the point light.");
        return BRE_FAIL;
    }

    light                = state->point_light->type_data;
    light->type          = BR_LIGHT_POINT;
    light->colour        = BR_COLOUR_RGB(255, 255, 255);
    light->attenuation_c = BR_SCALAR(1.0);
    light->radius_inner  = BR_SCALAR(0.0);
    light->radius_outer  = BR_SCALAR(20.0);

    state->point_light->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&state->point_light->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(10.0));

    /*
     * The volume witness's light is the same white direct light as the one above; only the
     * cutoff volume distinguishes it. It starts disabled so no earlier step sees it. The
     * volume is set once here - the core re-reads and re-transforms it every frame the
     * light is enabled.
     */
    if((state->volume_light = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL))) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the volume light.");
        return BRE_FAIL;
    }

    light                          = state->volume_light->type_data;
    light->type                    = BR_LIGHT_DIRECT;
    light->colour                  = BR_COLOUR_RGB(255, 255, 255);
    light->attenuation_c           = BR_SCALAR(1.0);
    light->volume.falloff_distance = VKREND_VOLUME_FALLOFF;
    light->volume.regions          = vkrend_renderer_volume_regions;
    light->volume.nregions         = 1;

    return BRE_OK;
}

/*
 * Add the clip-plane actor the clip step enables. Its plane is in the actor's space, and
 * the actor is a direct child of the world at identity, so (1, 0, 0, 0) is the world-space
 * half-space x >= 0; the camera is a pure translation along z, so world and view x agree
 * and enabling the plane cuts the triangle's left half. It starts disabled, so the step's
 * first render must draw the whole triangle.
 */
static br_error vkrend_renderer_build_clip_plane(vkrend_renderer_state *state)
{
    if((state->clip_plane = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_CLIP_PLANE, NULL))) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the clip plane actor.");
        return BRE_FAIL;
    }

    BrVector4Set((br_vector4 *)state->clip_plane->type_data, BR_SCALAR(1.0), BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(0.0));

    return BRE_OK;
}

/*
 * The 8-bit value the BRender lighting model predicts for one surface channel, computed
 * here without the renderer: (ka * ambient + dot(N, L) * kd * light) * surface, clamped
 * and rounded to the destination's channel width. `ambient` and `light` are the light
 * colours in [0,1] already scaled by their intensity, `surface` the surface colour. The
 * dot product is clamped the way the shader's max(dot, 0) does.
 */
static br_uint_8 vkrend_renderer_lit_channel(br_scalar ka, br_scalar ambient, br_scalar kd, br_scalar dot, br_scalar light, br_scalar surface)
{
    br_scalar value;

    if(dot < 0.0f)
        dot = 0.0f;

    value = (ka * ambient + dot * kd * light) * surface;

    if(value < 0.0f)
        value = 0.0f;
    if(value > 1.0f)
        value = 1.0f;

    return (br_uint_8)(value * 255.0f + 0.5f);
}

/* Fill expected[4] with the lit value of a white surface under a white light. */
static void vkrend_renderer_lit_expected(br_scalar ka, br_scalar kd, br_scalar dot, br_uint_8 expected[4])
{
    br_uint_8 value = vkrend_renderer_lit_channel(ka, BR_SCALAR(1.0), kd, dot, BR_SCALAR(1.0), BR_SCALAR(1.0));

    expected[0] = expected[1] = expected[2] = value;
    expected[3]                             = 0xff;
}

/*
 * glrend's fog, evaluated here as the fog witness's oracle
 * (drivers/glrend/brender.frag.glsl, applyFog()): the 8-bit fragment channel `surface`
 * fogged toward the 8-bit `fog` over [fog_min, fog_max], with glrend's 0.001 guard on
 * fog_max and its clamp-then-mix order, then rounded to the 8-bit channel the destination's
 * UNORM conversion produces. `dist` is the fragment's view-space distance, glrend's
 * viewDistance, which vkrend takes as -vViewPosition.z.
 */
static br_uint_8 vkrend_renderer_fog_channel(br_uint_8 surface, br_uint_8 fog, float dist, float fog_min, float fog_max)
{
    float fog_max_guarded = fog_max + 0.001f;
    float fog_factor;
    float value;

    if(fog_max_guarded < fog_max)
        fog_max_guarded = fog_max;

    fog_factor = (fog_max_guarded - dist) / (fog_max_guarded - fog_min);

    if(fog_factor < 0.0f)
        fog_factor = 0.0f;
    if(fog_factor > 1.0f)
        fog_factor = 1.0f;

    value = ((float)fog / 255.0f) * (1.0f - fog_factor) + ((float)surface / 255.0f) * fog_factor;

    if(value < 0.0f)
        value = 0.0f;
    if(value > 1.0f)
        value = 1.0f;

    return (br_uint_8)(value * 255.0f + 0.5f);
}

/* Render `model`/`material` and assert the image centre against expected[4]. */
static br_error vkrend_renderer_render_centre(vkrend_renderer_state *state, br_actor *camera, br_actor *actor, br_model *model,
                                              br_material *material, const br_uint_8 expected[4], const char *what)
{
    actor->model    = model;
    actor->material = material;

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        return BRE_FAIL;

    BrZsSceneRender(state->world, camera, state->offscreen);

    return vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 2, expected, what);
}

/* As vkrend_renderer_render_centre(), but allow each centre channel `tolerance`. */
static br_error vkrend_renderer_render_centre_near(vkrend_renderer_state *state, br_actor *camera, br_actor *actor, br_model *model,
                                                   br_material *material, const br_uint_8 expected[4], br_int_32 tolerance, const char *what)
{
    actor->model    = model;
    actor->material = material;

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        return BRE_FAIL;

    BrZsSceneRender(state->world, camera, state->offscreen);

    return vkrend_renderer_assert_pixel_near(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 2, expected, tolerance, what);
}

/* Whether the four bytes (B G R A) are one of the mip witness map's hard checkerboard texels. */
static br_boolean vkrend_renderer_is_checkerboard_texel(const br_uint_8 pixel[4])
{
    return pixel[1] == 0x00 && pixel[3] == 0xff && ((pixel[0] == 0x00 && pixel[2] == 0xff) || (pixel[0] == 0xff && pixel[2] == 0x00));
}

/* Whether the four bytes (B G R A) are one of the step-5 2x2 map's hard red/green texels. */
static br_boolean vkrend_renderer_is_red_or_green_texel(const br_uint_8 pixel[4])
{
    return pixel[0] == 0x00 && pixel[3] == 0xff && ((pixel[1] == 0x00 && pixel[2] == 0xff) || (pixel[1] == 0xff && pixel[2] == 0x00));
}

/*
 * The mip and filter witness.
 *
 * The mip half uses the 16x16 red/blue checkerboard, minified by VKREND_MIP_MAP_SCALE so the
 * LOD sits near 3. Every mip level below 0 is the map's 50/50 purple (each 2x2 block of a
 * checkerboard holds two red and two blue texels), so a sample that reaches the chain reads
 * purple while a sample pinned to level 0 reads one hard texel. With neither interpolation
 * flag (filter BRT_NONE, mip_filter BRT_NONE) the centre must be a hard red or blue texel.
 * Setting BR_MATF_MIP_INTERPOLATION alone (filter BRT_NONE, mip_filter BRT_LINEAR) must read
 * purple: the minification is past level 0, so a sampler whose LOD reaches the chain reads
 * the average, while one whose maxLod pins it to level 0 - or a map uploaded without a chain
 * - reads a hard texel instead. That difference is what this half exists to catch. Setting
 * both flags keeps purple, the mipmap mode's linear blend between two already-average levels.
 *
 * The filter half uses a 2x2 red/green map (both rows identical), magnified and sampled
 * across its column boundary: BR_MATF_MAP_INTERPOLATION must blend the two columns to
 * (0x00, 0x80, 0x80), while the default nearest filtering must read one hard red or green
 * texel. Together the two halves pin every row of glrend's filter/mip filter table
 * (drivers/glrend/v1model.c).
 *
 * The mip and filter materials and the actor are left as the neighbouring steps expect them.
 */
static br_error vkrend_renderer_mipmap_step(vkrend_renderer_state *state, br_actor *camera, br_actor *actor)
{
    const br_int_32 cx = state->offscreen->width / 2;
    const br_int_32 cy = state->offscreen->height / 2;

    const br_uint_8 purple[4] = {0x80, 0x00, 0x80, 0xff};
    const br_uint_8 blend[4]  = {0x00, 0x80, 0x80, 0xff};

    br_uint_32 saved_flags;
    br_uint_8  pixel[4];

    /* (filter BRT_NONE, mip_filter BRT_NONE): level 0, a hard checkerboard texel. */
    actor->model    = state->mip_model;
    actor->material = state->mip_material;

    state->mip_material->flags &= ~(BR_MATF_MAP_INTERPOLATION | BR_MATF_MIP_INTERPOLATION);
    BrMaterialUpdate(state->mip_material, BR_MATU_ALL);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        return BRE_FAIL;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_read_pixel(state->offscreen, cx, cy, pixel) != BRE_OK)
        return BRE_FAIL;

    if(!vkrend_renderer_is_checkerboard_texel(pixel)) {
        BrLogError("RT", "vkrend: the unfiltered mip sample read %02x %02x %02x %02x, expected a hard red or blue checkerboard texel.",
                   pixel[0], pixel[1], pixel[2], pixel[3]);
        return BRE_FAIL;
    }

    BrLogInfo("RT", "vkrend: mip witness (nearest, no mip) read the hard texel %02x %02x %02x %02x.", pixel[0], pixel[1], pixel[2], pixel[3]);

    /* (filter BRT_NONE, mip_filter BRT_LINEAR): the chain is reached, so the sample is purple. */
    state->mip_material->flags |= BR_MATF_MIP_INTERPOLATION;
    BrMaterialUpdate(state->mip_material, BR_MATU_ALL);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        return BRE_FAIL;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, cy, purple, 6, "the mip-filtered minified sample") != BRE_OK)
        return BRE_FAIL;

    /* (filter BRT_LINEAR, mip_filter BRT_LINEAR): the mipmap mode blends two average levels, still purple. */
    state->mip_material->flags |= BR_MATF_MAP_INTERPOLATION;
    BrMaterialUpdate(state->mip_material, BR_MATU_ALL);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        return BRE_FAIL;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, cy, purple, 6, "the trilinear minified sample") != BRE_OK)
        return BRE_FAIL;

    /*
     * The filter half, on the red/green map magnified and sampled across its column boundary,
     * which runs through the image column at the quad centre.
     */
    actor->model    = state->filter_model;
    actor->material = state->filter_material;
    saved_flags     = state->filter_material->flags;

    state->filter_material->flags &= ~(BR_MATF_MAP_INTERPOLATION | BR_MATF_MIP_INTERPOLATION);
    BrMaterialUpdate(state->filter_material, BR_MATU_ALL);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto fail_restore;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_read_pixel(state->offscreen, cx, cy, pixel) != BRE_OK)
        goto fail_restore;

    if(!vkrend_renderer_is_red_or_green_texel(pixel)) {
        BrLogError("RT", "vkrend: the unfiltered filter-witness sample read %02x %02x %02x %02x, expected a hard red or green texel.",
                   pixel[0], pixel[1], pixel[2], pixel[3]);
        goto fail_restore;
    }

    BrLogInfo("RT", "vkrend: filter witness (nearest) read the hard texel %02x %02x %02x %02x.", pixel[0], pixel[1], pixel[2], pixel[3]);

    state->filter_material->flags |= BR_MATF_MAP_INTERPOLATION;
    BrMaterialUpdate(state->filter_material, BR_MATU_ALL);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto fail_restore;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, cy, blend, 12, "the bilinear filter-witness boundary sample") != BRE_OK)
        goto fail_restore;

    state->filter_material->flags = saved_flags;
    BrMaterialUpdate(state->filter_material, BR_MATU_ALL);

    BrLogInfo("RT",
              "vkrend: mip and filter witness passed (a 16x16 red/blue checkerboard minified: nearest with no mip a hard texel, nearest "
              "with a mip chain %02x %02x "
              "%02x %02x, trilinear the same, and a magnified 2x2 map blending to %02x %02x %02x %02x across a texel boundary).",
              purple[0], purple[1], purple[2], purple[3], blend[0], blend[1], blend[2], blend[3]);

    return BRE_OK;

fail_restore:
    state->filter_material->flags = saved_flags;
    BrMaterialUpdate(state->filter_material, BR_MATU_ALL);
    return BRE_FAIL;
}

/*
 * The fog witness. A triangle at view distance 8 - the camera sits at +z 8 looking down -z,
 * the actor is at identity - is drawn with BR_MATF_FOG_LOCAL, fog_min 4, fog_max 10 and
 * fog_colour (0x20, 0x40, 0x60). glrend's applyFog() gives fog_factor = (10.001 - 8) /
 * (10.001 - 4) = 0.33344, so each channel is mix(fog, surface, 0.33344); for the surface
 * colour cc bb aa (B G R) the expected bytes are B 0x84, G 0x69, R 0x4e, A 0xff, computed by
 * the oracle above rather than read from the renderer. A control with fog off must show the
 * plain surface colour cc bb aa ff, so a renderer that fogged unconditionally, ignored the
 * material's fog state, or misread fog_min/fog_max fails one of the two.
 *
 * The material and the actor are restored before returning, so the steps after this one see
 * the triangle material and actor they expect.
 */
static br_error vkrend_renderer_fog_step(vkrend_renderer_state *state, br_actor *camera, br_actor *actor)
{
    const float     dist    = 8.0f; /* VKREND_RENDERER_CAMERA_DISTANCE as a float. */
    const float     fog_min = 4.0f;
    const float     fog_max = 10.0f;
    const br_colour fog_rgb = BR_COLOUR_RGB(0x20, 0x40, 0x60);
    br_uint_32      saved_flags;
    br_scalar       saved_min;
    br_scalar       saved_max;
    br_colour       saved_colour;
    br_uint_8       fogged[4];
    br_error        result = BRE_FAIL;

    /*
     * The oracle over the surface bytes in memory order (B, G, R, A): index 0 is the
     * surface's blue channel, matched with the fog colour's blue byte, and so on. The alpha
     * is glrend's fog_colour alpha of 1 mixed with the opaque surface's 1.
     */
    fogged[0] = vkrend_renderer_fog_channel(vkrend_renderer_surface[0], (br_uint_8)BR_BLU(fog_rgb), dist, fog_min, fog_max);
    fogged[1] = vkrend_renderer_fog_channel(vkrend_renderer_surface[1], (br_uint_8)BR_GRN(fog_rgb), dist, fog_min, fog_max);
    fogged[2] = vkrend_renderer_fog_channel(vkrend_renderer_surface[2], (br_uint_8)BR_RED(fog_rgb), dist, fog_min, fog_max);
    fogged[3] = 0xff;

    actor->model    = state->model;
    actor->material = state->material;
    actor->t.type   = BR_TRANSFORM_IDENTITY;
    BrMatrix34Identity(&actor->t.t.mat);

    camera->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), VKREND_RENDERER_CAMERA_DISTANCE);

    saved_flags  = state->material->flags;
    saved_min    = state->material->fog_min;
    saved_max    = state->material->fog_max;
    saved_colour = state->material->fog_colour;

    state->material->flags |= BR_MATF_FOG_LOCAL;
    state->material->fog_min    = BR_SCALAR(4.0);
    state->material->fog_max    = BR_SCALAR(10.0);
    state->material->fog_colour = fog_rgb;
    BrMaterialUpdate(state->material, BR_MATU_ALL);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto out;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 2, fogged, "the fogged surface") != BRE_OK)
        goto out;

    /* Control: fog off must draw the plain surface colour. */
    state->material->flags &= ~BR_MATF_FOG_LOCAL;
    BrMaterialUpdate(state->material, BR_MATU_ALL);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto out;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 2, vkrend_renderer_surface,
                                    "the unfogged control") != BRE_OK)
        goto out;

    BrLogInfo("RT",
              "vkrend: fog witness passed (fog_min %.0f, fog_max %.0f, fog_colour %02x %02x %02x at distance %.0f: surface %02x %02x %02x "
              "fogged to %02x %02x %02x, fog off %02x %02x %02x).",
              fog_min, fog_max, (br_uint_8)BR_RED(fog_rgb), (br_uint_8)BR_GRN(fog_rgb), (br_uint_8)BR_BLU(fog_rgb), dist,
              vkrend_renderer_surface[2], vkrend_renderer_surface[1], vkrend_renderer_surface[0], fogged[2], fogged[1], fogged[0],
              vkrend_renderer_surface[2], vkrend_renderer_surface[1], vkrend_renderer_surface[0]);

    result = BRE_OK;

out:
    state->material->flags      = saved_flags;
    state->material->fog_min    = saved_min;
    state->material->fog_max    = saved_max;
    state->material->fog_colour = saved_colour;
    BrMaterialUpdate(state->material, BR_MATU_ALL);
    return result;
}

/*
 * The independent oracle for the shading-mode step. The camera is a pure translation at +z
 * looking down -z and the actor is at identity, so a model-space normal is already the
 * view-space normal and the identity direct light's view-space direction is +z; the lights
 * are the white ambient and the white identity direct light, each of intensity one. Every
 * vertex lies on the z = 0 plane, so all three share a view depth and the rasteriser
 * interpolates the varyings linearly - the perspective correction cancels.
 *
 * The pixel centre (col + 0.5, row + 0.5) in the continuous framebuffer coordinates the
 * viewport maps the scene into gets barycentric weights against the three projected
 * vertices. GOURAUD interpolates the per-vertex `(ka + kd * max(dot(n, L), 0))` term;
 * PHONG normalises the interpolated normal and evaluates the same term there. Both are
 * returned as the 8-bit channel the destination's UNORM conversion of the white surface
 * produces, so neither calls the renderer.
 */
static br_uint_8 vkrend_renderer_shading_oracle(const vkrend_renderer_state *state, br_scalar pixels, br_int_32 col, br_int_32 row, br_boolean gouraud)
{
    const br_scalar ka = state->smooth_material->ka;
    const br_scalar kd = state->smooth_material->kd;
    br_scalar       sx[3], sy[3];
    br_scalar       qx = col + 0.5f;
    br_scalar       qy = row + 0.5f;
    br_scalar       v0x, v0y, v1x, v1y, v2x, v2y;
    br_scalar       d00, d01, d11, d20, d21, denom, b0, b1, b2;
    br_scalar       value;

    for(int i = 0; i < 3; ++i) {
        sx[i] = (br_scalar)(state->offscreen->width / 2) + state->smooth_positions[i].v[0] * pixels;
        sy[i] = (br_scalar)(state->offscreen->height / 2) - state->smooth_positions[i].v[1] * pixels;
    }

    v0x = sx[1] - sx[0];
    v0y = sy[1] - sy[0];
    v1x = sx[2] - sx[0];
    v1y = sy[2] - sy[0];
    v2x = qx - sx[0];
    v2y = qy - sy[0];

    d00   = v0x * v0x + v0y * v0y;
    d01   = v0x * v1x + v0y * v1y;
    d11   = v1x * v1x + v1y * v1y;
    d20   = v2x * v0x + v2y * v0y;
    d21   = v2x * v1x + v2y * v1y;
    denom = d00 * d11 - d01 * d01;

    b1 = (d11 * d20 - d01 * d21) / denom;
    b2 = (d00 * d21 - d01 * d20) / denom;
    b0 = 1.0f - b1 - b2;

    if(gouraud) {
        br_scalar l[3];

        for(int i = 0; i < 3; ++i) {
            br_scalar dot = state->smooth_normals[i].v[2]; /* The light's view-space direction is +z. */

            l[i] = ka + kd * (dot > 0.0f ? dot : 0.0f);
        }

        value = b0 * l[0] + b1 * l[1] + b2 * l[2];
    } else {
        br_scalar nx  = b0 * state->smooth_normals[0].v[0] + b1 * state->smooth_normals[1].v[0] + b2 * state->smooth_normals[2].v[0];
        br_scalar ny  = b0 * state->smooth_normals[0].v[1] + b1 * state->smooth_normals[1].v[1] + b2 * state->smooth_normals[2].v[1];
        br_scalar nz  = b0 * state->smooth_normals[0].v[2] + b1 * state->smooth_normals[1].v[2] + b2 * state->smooth_normals[2].v[2];
        br_scalar len = BR_SQRT(nx * nx + ny * ny + nz * nz);
        br_scalar dot = nz / len;

        value = ka + kd * (dot > 0.0f ? dot : 0.0f);
    }

    if(value < 0.0f)
        value = 0.0f;
    if(value > 1.0f)
        value = 1.0f;

    return (br_uint_8)(value * 255.0f + 0.5f);
}

/*
 * The aspect step: a sphere rendered at a square aspect and at two extremes.
 *
 * The projection's horizontal scale is scale/aspect and its vertical scale is scale, so a
 * correct perspective renders a sphere as a circle whatever the framebuffer's aspect is -
 * the horizontal pixels-per-unit, (width / 2) * (scale / aspect) / distance, and the
 * vertical, (height / 2) * scale / distance, are equal when aspect = width / height. A
 * stale or mismatched aspect stretches the circle into an ellipse; a clip volume or depth
 * range that depends on the aspect cuts it into a disc segment. Both leave the silhouette
 * wrong, so the step measures the silhouette's bounding box and compares its width and
 * height against the diameter the projection predicts, independently of the renderer.
 *
 * The colour map is unlit-bright so every lit sphere pixel is far from the clear colour and
 * the silhouette is unambiguous; the material is lit by the step's ambient light so the
 * whole front hemisphere shows rather than a terminator. The scene goes through the Zb
 * path with a depth buffer, which is what brtutor6 uses.
 */
#define VKREND_ASPECT_SEGMENTS 48
#define VKREND_ASPECT_RINGS    24

/* The projected diameter every aspect must render, in pixels. */
#define VKREND_ASPECT_DIAMETER 80

/* How far the measured diameter may differ from the projected one. */
#define VKREND_ASPECT_TOLERANCE 4

typedef struct vkrend_renderer_aspect_metric {
    br_int_32 min_x, max_x, min_y, max_y;
    br_int_32 count;
} vkrend_renderer_aspect_metric;

/*
 * Build the aspect step's sphere: a lat/long tessellation of the unit sphere whose normals
 * are the positions and whose map coordinates wrap it, carrying a bright 4x4 colour map.
 * The step scales it per aspect, so the model itself is fixed.
 */
static br_error vkrend_renderer_build_sphere(vkrend_renderer_state *state)
{
    const int nverts = (VKREND_ASPECT_RINGS + 1) * (VKREND_ASPECT_SEGMENTS + 1);
    const int nfaces = VKREND_ASPECT_RINGS * VKREND_ASPECT_SEGMENTS * 2;
    br_model *model;
    int       v = 0, f = 0;

    if((state->aspect_texture = BrPixelmapAllocate(BR_PMT_RGBA_8888, 4, 4, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the aspect sphere's colour map.");
        return BRE_FAIL;
    }

    for(int y = 0; y < 4; ++y)
        for(int x = 0; x < 4; ++x)
            BrPixelmapPixelSet(state->aspect_texture, x, y,
                               BR_COLOUR_RGB((br_uint_8)(0xc0 + x * 8), (br_uint_8)(0x80 + y * 16), (br_uint_8)(0xff - x * 16)));

    BrMapUpdate(state->aspect_texture, BR_MAPU_ALL);

    if(state->aspect_texture->stored == NULL) {
        BrLogError("RT", "vkrend: BrMapUpdate() produced no stored buffer for the aspect sphere's colour map.");
        return BRE_FAIL;
    }

    if((state->aspect_material = BrMaterialAllocate("vkrend-aspect-sphere")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the aspect sphere's material.");
        return BRE_FAIL;
    }

    state->aspect_material->colour     = BR_COLOUR_RGB(255, 255, 255);
    state->aspect_material->opacity    = 255;
    state->aspect_material->ka         = BR_SCALAR(1.0);
    state->aspect_material->kd         = BR_SCALAR(0.0);
    state->aspect_material->ks         = BR_SCALAR(0.0);
    state->aspect_material->colour_map = state->aspect_texture;
    state->aspect_material->flags |= BR_MATF_LIGHT | BR_MATF_TWO_SIDED;
    BrMaterialUpdate(state->aspect_material, BR_MATU_ALL);

    if((model = BrModelAllocate("vkrend-aspect-sphere", nverts, nfaces)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the aspect sphere's model.");
        return BRE_FAIL;
    }

    for(int i = 0; i <= VKREND_ASPECT_RINGS; ++i) {
        br_scalar theta = BR_MUL(BR_SCALAR(3.14159265358979323846), BR_DIV(BR_SCALAR(i), BR_SCALAR(VKREND_ASPECT_RINGS)));
        br_scalar st    = (br_scalar)sinf(theta);
        br_scalar ct    = (br_scalar)cosf(theta);

        for(int j = 0; j <= VKREND_ASPECT_SEGMENTS; ++j) {
            br_scalar phi = BR_MUL(BR_SCALAR(6.28318530717958647692), BR_DIV(BR_SCALAR(j), BR_SCALAR(VKREND_ASPECT_SEGMENTS)));
            br_scalar x   = BR_MUL(st, (br_scalar)cosf(phi));
            br_scalar z   = BR_MUL(st, (br_scalar)sinf(phi));

            BrVector3Set(&model->vertices[v].p, x, ct, z);
            model->vertices[v].n.v[0] = x;
            model->vertices[v].n.v[1] = ct;
            model->vertices[v].n.v[2] = z;
            BrVector2Set(&model->vertices[v].map, BR_DIV(BR_SCALAR(j), BR_SCALAR(VKREND_ASPECT_SEGMENTS)),
                         BR_DIV(BR_SCALAR(i), BR_SCALAR(VKREND_ASPECT_RINGS)));
            ++v;
        }
    }

    for(int i = 0; i < VKREND_ASPECT_RINGS; ++i) {
        for(int j = 0; j < VKREND_ASPECT_SEGMENTS; ++j) {
            const int a = i * (VKREND_ASPECT_SEGMENTS + 1) + j;
            const int b = a + 1;
            const int c = a + (VKREND_ASPECT_SEGMENTS + 1);
            const int d = c + 1;

            model->faces[f].vertices[0] = (br_uint_16)a;
            model->faces[f].vertices[1] = (br_uint_16)c;
            model->faces[f].vertices[2] = (br_uint_16)b;
            model->faces[f].material    = state->aspect_material;
            ++f;

            model->faces[f].vertices[0] = (br_uint_16)b;
            model->faces[f].vertices[1] = (br_uint_16)c;
            model->faces[f].vertices[2] = (br_uint_16)d;
            model->faces[f].material    = state->aspect_material;
            ++f;
        }
    }

    BrModelUpdate(model, BR_MODU_ALL);

    if(model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the aspect sphere.");
        BrModelFree(model);
        return BRE_FAIL;
    }

    state->aspect_model = model;
    return BRE_OK;
}

/* Whether a readback pixel is a sphere pixel rather than the clear colour. */
static br_boolean vkrend_renderer_aspect_is_surface(const br_pixelmap *readback, br_int_32 x, br_int_32 y)
{
    const br_uint_8 *pixel = (const br_uint_8 *)readback->pixels + y * readback->row_bytes + x * 4;

    return pixel[0] > 0x60 || pixel[1] > 0x60 || pixel[2] > 0x60;
}

/* Find the bounding box and pixel count of the sphere's silhouette in `colour`. */
static br_error vkrend_renderer_aspect_measure(br_pixelmap *colour, vkrend_renderer_aspect_metric *metric)
{
    br_pixelmap *readback;

    if((readback = vkrend_renderer_read(colour)) == NULL)
        return BRE_FAIL;

    metric->min_x = colour->width;
    metric->min_y = colour->height;
    metric->max_x = -1;
    metric->max_y = -1;
    metric->count = 0;

    for(br_int_32 y = 0; y < readback->height; ++y) {
        for(br_int_32 x = 0; x < readback->width; ++x) {
            if(!vkrend_renderer_aspect_is_surface(readback, x, y))
                continue;

            if(x < metric->min_x)
                metric->min_x = x;
            if(y < metric->min_y)
                metric->min_y = y;
            if(x > metric->max_x)
                metric->max_x = x;
            if(y > metric->max_y)
                metric->max_y = y;
            ++metric->count;
        }
    }

    BrPixelmapFree(readback);
    return BRE_OK;
}

/* Render the sphere into `colour`/`depth` at offscreen's aspect and measure its silhouette. */
static br_error vkrend_renderer_aspect_render(vkrend_renderer_state *state, br_actor *camera, br_actor *actor, br_camera *camera_data,
                                              br_pixelmap *colour, br_pixelmap *depth, const char *what, vkrend_renderer_aspect_metric *metric)
{
    const br_scalar fov      = camera_data->field_of_view;
    const br_scalar distance = VKREND_RENDERER_CAMERA_DISTANCE;
    br_scalar       pixels   = vkrend_renderer_pixels_per_unit(colour, fov, distance);
    br_scalar       scale    = BR_DIV(BR_SCALAR(VKREND_ASPECT_DIAMETER), BR_MUL(BR_SCALAR(2.0), pixels));
    br_int_32       got_w, got_h;

    camera_data->aspect = BR_DIV(BR_SCALAR(colour->width), BR_SCALAR(colour->height));

    actor->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Scale(&actor->t.t.mat, scale, scale, scale);
    actor->model    = state->aspect_model;
    actor->material = state->aspect_material;

    if(vkrend_renderer_fill_clear(colour) != BRE_OK)
        return BRE_FAIL;

    BrZbSceneRender(state->world, camera, colour, depth);

    if(vkrend_renderer_aspect_measure(colour, metric) != BRE_OK)
        return BRE_FAIL;

    got_w = metric->count ? metric->max_x - metric->min_x + 1 : 0;
    got_h = metric->count ? metric->max_y - metric->min_y + 1 : 0;

    BrLogInfo("RT", "vkrend: %s (%dx%d, aspect %g): silhouette %d x %d, x %d..%d, y %d..%d, %d pixels.", what, colour->width,
              colour->height, (double)BR_DIV(BR_SCALAR(colour->width), BR_SCALAR(colour->height)), got_w, got_h, metric->min_x,
              metric->max_x, metric->min_y, metric->max_y, metric->count);

    if(metric->count == 0) {
        BrLogError("RT", "vkrend: %s: the sphere did not render at all.", what);
        return BRE_FAIL;
    }

    if(got_w < VKREND_ASPECT_DIAMETER - VKREND_ASPECT_TOLERANCE || got_w > VKREND_ASPECT_DIAMETER + VKREND_ASPECT_TOLERANCE ||
       got_h < VKREND_ASPECT_DIAMETER - VKREND_ASPECT_TOLERANCE || got_h > VKREND_ASPECT_DIAMETER + VKREND_ASPECT_TOLERANCE) {
        BrLogError("RT",
                   "vkrend: %s: the sphere's silhouette is %d x %d, not the projected %d x %d - the aspect or the clip volume is wrong at "
                   "this aspect.",
                   what, got_w, got_h, VKREND_ASPECT_DIAMETER, VKREND_ASPECT_DIAMETER);
        return BRE_FAIL;
    }

    return BRE_OK;
}

/*
 * The aspect step's driver: a square framebuffer and two extremes, each with the camera's
 * aspect set from the framebuffer exactly as BrDemoDefaultOnResize() does.
 */
static br_error vkrend_renderer_aspect_step(vkrend_renderer_state *state, br_actor *camera, br_actor *actor, br_camera *camera_data)
{
    static const struct {
        br_int_32 width, height;
    } sizes[] = {
        {512,  512}, /* square                     */
        {2048, 512}, /* 4:1, the recording's wide  */
        {128,  512}, /* 1:4, the other extreme     */
    };

    for(size_t i = 0; i < BR_ASIZE(sizes); ++i) {
        br_pixelmap                  *colour = NULL, *depth = NULL;
        vkrend_renderer_aspect_metric metric;
        br_error                      result;

        if((colour = BrPixelmapMatchTypedSized(state->screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBA_8888, sizes[i].width, sizes[i].height)) == NULL) {
            BrLogError("RT", "vkrend: could not create the %dx%d aspect offscreen.", sizes[i].width, sizes[i].height);
            return BRE_FAIL;
        }

        colour->origin_x = (br_int_16)(colour->width / 2);
        colour->origin_y = (br_int_16)(colour->height / 2);

        if((depth = BrPixelmapMatch(colour, BR_PMMATCH_DEPTH_16)) == NULL) {
            BrLogError("RT", "vkrend: could not create the %dx%d aspect depth buffer.", sizes[i].width, sizes[i].height);
            BrPixelmapFree(colour);
            return BRE_FAIL;
        }

        result = vkrend_renderer_aspect_render(state, camera, actor, camera_data, colour, depth, "the aspect sphere", &metric);

        BrPixelmapFree(depth);
        BrPixelmapFree(colour);

        if(result != BRE_OK)
            return BRE_FAIL;
    }

    return BRE_OK;
}

static void vkrend_renderer_fini(void *user)
{
    vkrend_renderer_state *state = user;

    if(state->world != NULL)
        BrActorFree(state->world);

    if(state->occlusion_order_table != NULL)
        BrZsOrderTableFree(state->occlusion_order_table);

    if(state->model != NULL)
        BrModelFree(state->model);

    if(state->multi_model != NULL)
        BrModelFree(state->multi_model);

    if(state->lit_model != NULL)
        BrModelFree(state->lit_model);

    if(state->dim_model != NULL)
        BrModelFree(state->dim_model);

    if(state->unlit_model != NULL)
        BrModelFree(state->unlit_model);

    if(state->texture_model != NULL)
        BrModelFree(state->texture_model);

    if(state->indexed_model != NULL)
        BrModelFree(state->indexed_model);

    if(state->rt_model != NULL)
        BrModelFree(state->rt_model);

    if(state->vc_model != NULL)
        BrModelFree(state->vc_model);

    if(state->prelit_model != NULL)
        BrModelFree(state->prelit_model);

    if(state->smooth_model != NULL)
        BrModelFree(state->smooth_model);

    if(state->occlusion_model != NULL)
        BrModelFree(state->occlusion_model);

    if(state->depth_rev_model != NULL)
        BrModelFree(state->depth_rev_model);

    if(state->material != NULL)
        BrMaterialFree(state->material);

    if(state->multi_left != NULL)
        BrMaterialFree(state->multi_left);

    if(state->multi_right != NULL)
        BrMaterialFree(state->multi_right);

    if(state->lit_material != NULL)
        BrMaterialFree(state->lit_material);

    if(state->dim_material != NULL)
        BrMaterialFree(state->dim_material);

    if(state->unlit_material != NULL)
        BrMaterialFree(state->unlit_material);

    if(state->texture_material != NULL)
        BrMaterialFree(state->texture_material);

    if(state->indexed_material != NULL)
        BrMaterialFree(state->indexed_material);

    if(state->rt_material != NULL)
        BrMaterialFree(state->rt_material);

    if(state->vc_material != NULL)
        BrMaterialFree(state->vc_material);

    if(state->prelit_material != NULL)
        BrMaterialFree(state->prelit_material);

    if(state->smooth_material != NULL)
        BrMaterialFree(state->smooth_material);

    if(state->occlusion_far != NULL)
        BrMaterialFree(state->occlusion_far);

    if(state->occlusion_near != NULL)
        BrMaterialFree(state->occlusion_near);

    /*
     * The texture's stored buffer is a child of the renderer, which BrRendererEnd() has
     * already freed; BrBufferClear() would double free it, so free the pixelmap alone.
     */
    if(state->texture != NULL)
        BrPixelmapFree(state->texture);

    /*
     * As with the colour texture, the indexed map's and its palette's stored buffers are
     * children of the renderer, which BrRendererEnd() has already freed, so free the
     * pixelmaps alone rather than BrBufferClear().
     */
    if(state->indexed_texture != NULL)
        BrPixelmapFree(state->indexed_texture);

    if(state->indexed_palette != NULL)
        BrPixelmapFree(state->indexed_palette);

    if(state->rt_texture != NULL)
        BrPixelmapFree(state->rt_texture);

    if(state->aspect_material != NULL)
        BrMaterialFree(state->aspect_material);

    if(state->aspect_model != NULL)
        BrModelFree(state->aspect_model);

    /*
     * As with the colour texture, the aspect map's stored buffer is a child of the renderer,
     * so free the pixelmap alone.
     */
    if(state->aspect_texture != NULL)
        BrPixelmapFree(state->aspect_texture);

    if(state->depth != NULL)
        BrPixelmapFree(state->depth);

    if(state->offscreen != NULL)
        BrPixelmapFree(state->offscreen);

    state->world                 = NULL;
    state->model                 = NULL;
    state->multi_model           = NULL;
    state->lit_model             = NULL;
    state->dim_model             = NULL;
    state->unlit_model           = NULL;
    state->texture_model         = NULL;
    state->indexed_model         = NULL;
    state->rt_model              = NULL;
    state->vc_model              = NULL;
    state->prelit_model          = NULL;
    state->smooth_model          = NULL;
    state->occlusion_model       = NULL;
    state->depth_rev_model       = NULL;
    state->material              = NULL;
    state->multi_left            = NULL;
    state->multi_right           = NULL;
    state->lit_material          = NULL;
    state->dim_material          = NULL;
    state->unlit_material        = NULL;
    state->texture_material      = NULL;
    state->indexed_material      = NULL;
    state->rt_material           = NULL;
    state->vc_material           = NULL;
    state->prelit_material       = NULL;
    state->smooth_material       = NULL;
    state->occlusion_far         = NULL;
    state->occlusion_near        = NULL;
    state->occlusion_order_table = NULL;
    state->ambient_light         = NULL;
    state->direct_light          = NULL;
    state->point_light           = NULL;
    state->clip_plane            = NULL;
    state->texture               = NULL;
    state->indexed_texture       = NULL;
    state->indexed_palette       = NULL;
    state->rt_texture            = NULL;
    state->aspect_texture        = NULL;
    state->aspect_model          = NULL;
    state->aspect_material       = NULL;
    state->depth                 = NULL;
    state->offscreen             = NULL;
}

/*
 * The scene-draw-into-offscreen orientation witness. A pre-lit quad is red at its world
 * +y vertices and blue at its -y vertices, so a scene draw that puts world +y at the top
 * of the image (BRender's image y grows downward) shows red on the top row and blue on
 * the bottom. A scene draw into the offscreen that was vertically inverted would swap
 * them, and a renderer that samples that offscreen as an environment map would then
 * reflect a vertically mirrored world through either mapping convention. This is the
 * orientation rdemo's BrZbSceneRender of the earth into mirror_pm produces, and it is
 * independent of the environment-map U,V the step above checks.
 */
static br_error vkrend_renderer_offscreen_orientation(vkrend_renderer_state *state, br_actor *camera, br_actor *actor, br_camera *camera_data)
{
    static const br_scalar positions[8][2] = {
        {-3.0, 0.5 },
        {3.0,  0.5 },
        {3.0,  3.0 },
        {-3.0, 3.0 },
        {-3.0, -3.0},
        {3.0,  -3.0},
        {3.0,  -0.5},
        {-3.0, -0.5}
    };
    static const br_uint_8 colours[8][3] = {
        {255, 0, 0  },
        {255, 0, 0  },
        {255, 0, 0  },
        {255, 0, 0  },
        {0,   0, 255},
        {0,   0, 255},
        {0,   0, 255},
        {0,   0, 255}
    };
    static const br_uint_16 faces[4][3] = {
        {0, 1, 2},
        {0, 2, 3},
        {4, 5, 6},
        {4, 6, 7}
    };
    static const br_uint_8 red[4]  = {0x00, 0x00, 0xff, 0xff}; /* BR_COLOUR_RGB(255, 0, 0) in B G R A. */
    static const br_uint_8 blue[4] = {0xff, 0x00, 0x00, 0xff}; /* BR_COLOUR_RGB(0, 0, 255) in B G R A. */
    br_material           *material;
    br_model              *model;
    br_scalar              old_aspect = camera_data->aspect;
    br_error               result     = BRE_FAIL;

    if((material = BrMaterialAllocate("vkrend-renderer-orientation")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the orientation material.");
        return BRE_FAIL;
    }

    material->colour  = BR_COLOUR_RGB(0, 255, 0);
    material->opacity = 255;
    material->flags |= BR_MATF_PRELIT;
    material->flags &= ~BR_MATF_LIGHT;
    BrMaterialUpdate(material, BR_MATU_ALL);

    if((model = BrModelAllocate("vkrend-renderer-orientation", 8, 4)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the orientation model.");
        BrMaterialFree(material);
        return BRE_FAIL;
    }

    for(int i = 0; i < 8; ++i) {
        BrVector3Set(&model->vertices[i].p, BR_SCALAR(positions[i][0]), BR_SCALAR(positions[i][1]), BR_SCALAR(0.0));
        model->vertices[i].red   = colours[i][0];
        model->vertices[i].grn   = colours[i][1];
        model->vertices[i].blu   = colours[i][2];
        model->vertices[i].index = 255;
    }

    for(int i = 0; i < 4; ++i) {
        model->faces[i].vertices[0] = faces[i][0];
        model->faces[i].vertices[1] = faces[i][1];
        model->faces[i].vertices[2] = faces[i][2];
        model->faces[i].material    = material;
    }

    BrModelUpdate(model, BR_MODU_ALL);

    actor->t.type = BR_TRANSFORM_IDENTITY;
    BrMatrix34Identity(&actor->t.t.mat);
    actor->model    = model;
    actor->material = material;

    camera_data->aspect = BR_DIV(BR_SCALAR(state->offscreen->width), BR_SCALAR(state->offscreen->height));
    BrEnvironmentSet(NULL);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto out;

    BrPixelmapFill(state->depth, 0xffffffff);
    BrZbSceneRender(state->world, camera, state->offscreen, state->depth);

    if(vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 4, red,
                                    "the offscreen orientation's top row") != BRE_OK)
        goto out;

    if(vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height * 3 / 4, blue,
                                    "the offscreen orientation's bottom row") != BRE_OK)
        goto out;

    BrLogInfo("RT", "vkrend: offscreen orientation witness passed (a scene-drawn quad's world +y is red at the image top and its world -y "
                    "blue at the image bottom).");
    result = BRE_OK;

out:
    actor->model    = NULL;
    actor->material = NULL;
    BrModelFree(model);
    BrMaterialFree(material);
    camera_data->aspect = old_aspect;
    return result;
}

/*
 * The winding step. glrend flips the front face when the model-view reflects: a negative
 * upper-3x3 determinant of model_to_view means the transformation changes handedness, so
 * glrend calls glFrontFace(GL_CW) (drivers/glrend/v1model.c). vkrend hardcoded
 * VK_FRONT_FACE_COUNTER_CLOCKWISE, which culls the wrong side of a mirrored model.
 *
 * The witness uses the one-sided material default and an asymmetric triangle whose front
 * face is +z, spanning model x in [0.5, 2.0], so mirroring x moves it from the right half
 * of the image to the left instead of onto itself. It draws the triangle twice - identity
 * (determinant > 0) and an x scale of -1 (determinant < 0) - and asserts opposite bytes at
 * the same two pixels, so neither the transform nor the winding alone can pass both:
 *
 *   view x = +1.25  identity surface, mirrored clear
 *   view x = -1.25  identity clear,   mirrored surface
 *
 * A mirrored draw whose front face is left at CCW is back-facing and culled, so its
 * "surface" read is clear; that is the assertion the fix turns over. Both bytes of each
 * pair differ, so a renderer that ignored the mirror (drawing the identity geometry both
 * times) also fails.
 */
static br_error vkrend_renderer_winding_step(vkrend_renderer_state *state, br_actor *camera, br_actor *actor, br_camera *camera_data)
{
    br_material *material;
    br_model    *model;
    br_scalar    fov      = camera_data->field_of_view;
    br_scalar    distance = VKREND_RENDERER_CAMERA_DISTANCE;
    br_int_32    right_x, left_x, pixel_y;
    br_error     result = BRE_FAIL;

    if((material = BrMaterialAllocate("vkrend-renderer-winding")) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the winding material.");
        return BRE_FAIL;
    }

    material->colour  = BR_COLOUR_RGB(0xaa, 0xbb, 0xcc);
    material->opacity = 255;
    material->flags &= ~(BR_MATF_LIGHT | BR_MATF_TWO_SIDED | BR_MATF_ALWAYS_VISIBLE);
    BrMaterialUpdate(material, BR_MATU_ALL);

    if((model = BrModelAllocate("vkrend-renderer-winding", 3, 1)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the winding model.");
        BrMaterialFree(material);
        return BRE_FAIL;
    }

    /*
     * Wound counter-clockwise from +z, as the step-1 triangle is, so its front face is +z;
     * the x range [0.5, 2.0] is what makes the x mirror land on the other side of the
     * screen rather than on top of the identity case.
     */
    BrVector3Set(&model->vertices[0].p, BR_SCALAR(0.5), BR_SCALAR(-0.5), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[1].p, BR_SCALAR(2.0), BR_SCALAR(-0.5), BR_SCALAR(0.0));
    BrVector3Set(&model->vertices[2].p, BR_SCALAR(1.25), BR_SCALAR(0.5), BR_SCALAR(0.0));

    model->faces[0].vertices[0] = 0;
    model->faces[0].vertices[1] = 1;
    model->faces[0].vertices[2] = 2;
    model->faces[0].material    = material;

    BrModelUpdate(model, BR_MODU_ALL);

    if(model->stored == NULL) {
        BrLogError("RT", "vkrend: BrModelUpdate() produced no stored geometry for the winding model.");
        goto out;
    }

    actor->model    = model;
    actor->material = material;

    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(1.25), BR_SCALAR(0.0), &right_x, &pixel_y);
    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(-1.25), BR_SCALAR(0.0), &left_x, &pixel_y);

    /* Positive determinant: the front face is +z and the triangle sits at view +x. */
    actor->t.type = BR_TRANSFORM_IDENTITY;
    BrMatrix34Identity(&actor->t.t.mat);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto out;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_assert_pixel(state->offscreen, right_x, pixel_y, vkrend_renderer_surface, "the identity front face") != BRE_OK)
        goto out;

    if(vkrend_renderer_assert_pixel(state->offscreen, left_x, pixel_y, vkrend_renderer_clear, "the identity back position") != BRE_OK)
        goto out;

    /* Negative determinant: the x mirror moves the front face to view -x. */
    actor->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Scale(&actor->t.t.mat, BR_SCALAR(-1.0), BR_SCALAR(1.0), BR_SCALAR(1.0));

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto out;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_assert_pixel(state->offscreen, left_x, pixel_y, vkrend_renderer_surface, "the mirrored front face") != BRE_OK)
        goto out;

    if(vkrend_renderer_assert_pixel(state->offscreen, right_x, pixel_y, vkrend_renderer_clear, "the mirrored back position") != BRE_OK)
        goto out;

    BrLogInfo("RT",
              "vkrend: winding witness passed (view x=+1.25 identity %02x %02x %02x %02x, mirrored %02x %02x %02x %02x; view x=-1.25 "
              "identity %02x "
              "%02x %02x %02x, mirrored %02x %02x %02x %02x).",
              vkrend_renderer_surface[0], vkrend_renderer_surface[1], vkrend_renderer_surface[2], vkrend_renderer_surface[3],
              vkrend_renderer_clear[0], vkrend_renderer_clear[1], vkrend_renderer_clear[2], vkrend_renderer_clear[3],
              vkrend_renderer_clear[0], vkrend_renderer_clear[1], vkrend_renderer_clear[2], vkrend_renderer_clear[3],
              vkrend_renderer_surface[0], vkrend_renderer_surface[1], vkrend_renderer_surface[2], vkrend_renderer_surface[3]);

    result = BRE_OK;

out:
    actor->model    = NULL;
    actor->material = NULL;
    actor->t.type   = BR_TRANSFORM_IDENTITY;
    BrMatrix34Identity(&actor->t.t.mat);
    BrModelFree(model);
    BrMaterialFree(material);
    return result;
}

static br_error vkrend_renderer_init(void *user, br_pixelmap *screen, br_pixelmap *backbuffer, void *arg)
{
    vkrend_renderer_state *state = user;
    static char            primitive_heap[256 * 1024];
    br_actor              *camera, *actor;
    br_camera             *camera_data;
    br_pixelmap           *readback;
    br_scalar              fov, distance, pixels, shift, dot;
    br_int_32              expected_x, expected_y;
    br_uint_8              lit_expected[4];
    br_uint_8              gouraud_expected[4], phong_expected[4];
    br_uint_8              gouraud_pixel[4], phong_pixel[4];
    br_uint_8              env_infinite_expected[4], env_local_expected[4];
    vkrend_renderer_bounds identity_bounds, moved_bounds, left_bounds, right_bounds;

    (void)backbuffer;
    (void)arg;

    state->screen = screen;

    if((state->offscreen = BrPixelmapMatch(screen, BR_PMMATCH_OFFSCREEN)) == NULL) {
        BrLogError("RT", "vkrend: BrPixelmapMatch() could not make a renderer offscreen.");
        goto fail;
    }

    /*
     * The offscreen inherits the front screen's origin, which is not the new image's centre
     * once the front has been resized. The origin defines the centre of projection (see
     * SetOrigin() in core/v1db/render.c) and the readback coordinates, so set it to the
     * offscreen's own centre.
     */
    state->offscreen->origin_x = (br_int_16)(state->offscreen->width / 2);
    state->offscreen->origin_y = (br_int_16)(state->offscreen->height / 2);

    /*
     * The Zb step's depth buffer, matched from the offscreen the way
     * BrPixelmapResizeBuffersTV() and the demos ask for one (BR_PMMATCH_DEPTH_16). It is a
     * vkrend depth pixelmap and is attached only by BrZbSceneRender().
     */
    if((state->depth = BrPixelmapMatch(state->offscreen, BR_PMMATCH_DEPTH_16)) == NULL) {
        BrLogError("RT", "vkrend: could not match a depth buffer for the Zb step.");
        goto fail;
    }

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto fail;

    /*
     * BrRendererBegin() resolves the renderer facility for the offscreen and the geometry
     * formats BrV1dbRendererBegin() looks up (v1model, buckets and lighting), all created
     * with the facility. Its destination must be a vkrend offscreen.
     */
    BrRendererBegin(state->offscreen, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    /*
     * A sub-pixelmap is a valid renderer destination: RendererVkAllocate() resolves its
     * owner, and the scope is opened on the owner's image with the viewport and scissor set
     * to the sub's rect. Croc does not create a renderer on the sub directly - it begins the
     * renderer on the owner and targets the sub from the scene - but rendererNew() on a sub
     * is accepted too, and checked here before that shape is exercised below.
     */
    {
        br_pixelmap          *sub          = NULL;
        br_output_facility   *out          = NULL;
        br_renderer_facility *fac          = NULL;
        br_renderer          *sub_renderer = NULL;
        br_token_value        tv[2]        = {
            {.t = BRT_DESTINATION_O, .v.o = NULL},
            {.t = BR_NULL_TOKEN},
        };

        if((sub = BrPixelmapAllocateSub(state->offscreen, -8, -8, 16, 16)) == NULL) {
            BrLogError("RT", "vkrend: could not allocate the sub-pixelmap destination.");
            goto fail;
        }

        ObjectQuery(state->offscreen, &out, BRT_OUTPUT_FACILITY_O);
        if(out != NULL)
            ObjectQuery(out, &fac, BRT_RENDERER_FACILITY_O);

        if(fac == NULL) {
            BrLogError("RT", "vkrend: could not find the renderer facility for the sub-destination test.");
            BrPixelmapFree(sub);
            goto fail;
        }

        tv[0].v.o = (br_object *)sub;

        if(RendererFacilityRendererNew(fac, &sub_renderer, tv) != BRE_OK || sub_renderer == NULL) {
            BrLogError("RT", "vkrend: a sub-pixelmap was rejected as a renderer destination.");
            BrPixelmapFree(sub);
            goto fail;
        }

        ObjectFree((br_object *)sub_renderer);
        BrPixelmapFree(sub);
        BrLogInfo("RT", "vkrend: sub-pixelmap renderer destination accepted.");
    }

    /*
     * The unlit materials have to be updated after BrRendererBegin() so their stored state
     * exists; the prepared model's groups take their stored states from them.
     */
    if(vkrend_renderer_build_triangle(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_multigroup(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_surface("vkrend-renderer-lit", BR_SCALAR(0.25), BR_SCALAR(0.5), BR_TRUE, &state->lit_material, &state->lit_model) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_surface("vkrend-renderer-dim", BR_SCALAR(0.5), BR_SCALAR(0.5), BR_TRUE, &state->dim_material, &state->dim_model) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_surface("vkrend-renderer-unlit", BR_SCALAR(0.25), BR_SCALAR(0.5), BR_FALSE, &state->unlit_material,
                                     &state->unlit_model) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_texture(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_mipmap(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_filter_texture(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_environment(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_indexed_texture(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_render_to_texture(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_vertex_colours(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_smooth_lit(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_occlusion(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_sphere(state) != BRE_OK)
        goto fail;

    if((state->world = BrActorAllocate(BR_ACTOR_NONE, NULL)) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the world.");
        goto fail;
    }

    if(vkrend_renderer_build_lights(state) != BRE_OK)
        goto fail;

    if(vkrend_renderer_build_clip_plane(state) != BRE_OK)
        goto fail;

    if((camera = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL))) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the camera.");
        goto fail;
    }

    camera->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), VKREND_RENDERER_CAMERA_DISTANCE);

    camera_data         = camera->type_data;
    camera_data->aspect = BR_DIV(BR_SCALAR(state->offscreen->width), BR_SCALAR(state->offscreen->height));

    fov      = camera_data->field_of_view;
    distance = VKREND_RENDERER_CAMERA_DISTANCE;
    pixels   = vkrend_renderer_pixels_per_unit(state->offscreen, fov, distance);

    if((actor = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_MODEL, NULL))) == NULL) {
        BrLogError("RT", "vkrend: could not allocate the model actor.");
        goto fail;
    }

    actor->model    = state->model;
    actor->material = state->material;

    /*
     * Step 1: an identity actor. The centre pixel is the surface colour and the top-left
     * pixel is the clear colour; the surface bounds of this image are the baseline step 2
     * measures its shifts against.
     */
    BrZsSceneRender(state->world, camera, state->offscreen);

    if(vkrend_renderer_assert_pixel(state->offscreen, 0, 0, vkrend_renderer_clear, "the cleared corner") != BRE_OK)
        goto fail;

    if(vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 2, vkrend_renderer_surface,
                                    "the triangle centre") != BRE_OK)
        goto fail;

    if((readback = vkrend_renderer_read(state->offscreen)) == NULL)
        goto fail;

    vkrend_renderer_bounds_find(readback, vkrend_renderer_surface, &identity_bounds);
    BrPixelmapFree(readback);

    /*
     * Step 1b: the sub-pixelmap scene destination, Croc's shape. The renderer was begun on
     * the owner offscreen above, but the scene's colour target is a sub of that owner -
     * BrPixelmapAllocateSub(cbuf, -cbuf->origin_x + rect.x, -cbuf->origin_y + rect.y, w, h)
     * with the sub's origin set to its own centre, exactly as Croc's crocmain.c builds
     * sub_buffer. The scene must land in the sub's rect of the owner's image and nowhere
     * else: the sub's centre must be the surface colour while the owner's centre - outside
     * the sub - keeps the clear colour. A renderer that rejected the sub draws nothing (the
     * sub centre stays clear); one that ignored the sub's rect draws at the owner's centre
     * (the sub centre stays clear and the owner's centre no longer is).
     */
    {
        /*
         * The owner is half the front screen's size (the offscreen witness above resized the
         * front), so take a quadrant of it: the owner's centre then falls on the sub's
         * bottom-right corner and the scene drawn at the sub's centre is well inside.
         */
        const br_rectangle sub_rect = {.x = 0, .y = 0, .w = state->offscreen->width / 2, .h = state->offscreen->height / 2};
        br_pixelmap       *sub;
        br_int_32          sub_x, sub_y, sub_cx, sub_cy;

        /*
         * The owner's user origin is its centre, so subtracting it places the sub's image
         * rect at sub_rect; the sub's pm_base_x/pm_base_y are then sub_rect's top-left in
         * the owner's image coordinates, which is what the viewport and this readback use.
         */
        if((sub = BrPixelmapAllocateSub(state->offscreen, sub_rect.x - state->offscreen->origin_x, sub_rect.y - state->offscreen->origin_y,
                                        sub_rect.w, sub_rect.h)) == NULL) {
            BrLogError("RT", "vkrend: could not allocate the sub-pixelmap scene destination.");
            goto fail;
        }

        /* Croc gives the sub its own centre as its origin, so the projection centres on it. */
        sub->origin_x = (br_int_16)(sub_rect.w / 2);
        sub->origin_y = (br_int_16)(sub_rect.h / 2);

        sub_x  = sub->base_x;
        sub_y  = sub->base_y;
        sub_cx = sub_x + sub_rect.w / 2;
        sub_cy = sub_y + sub_rect.h / 2;

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK) {
            BrPixelmapFree(sub);
            goto fail;
        }

        BrZbSceneRender(state->world, camera, sub, state->depth);

        if(vkrend_renderer_assert_pixel(state->offscreen, sub_cx, sub_cy, vkrend_renderer_surface, "the sub-destination centre") != BRE_OK) {
            BrPixelmapFree(sub);
            goto fail;
        }

        if(vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 2, vkrend_renderer_clear,
                                        "the owner centre outside the sub") != BRE_OK) {
            BrPixelmapFree(sub);
            goto fail;
        }

        if(vkrend_renderer_assert_pixel(state->offscreen, sub_x + sub_rect.w + 1, sub_cy, vkrend_renderer_clear,
                                        "the pixel just outside the sub") != BRE_OK) {
            BrPixelmapFree(sub);
            goto fail;
        }

        BrPixelmapFree(sub);
        BrLogInfo("RT", "vkrend: sub-pixelmap scene destination witness passed (sub image rect %d,%d %dx%d: its centre %02x %02x %02x %02x is the surface, while the owner centre and the pixel just outside the sub keep the clear %02x %02x %02x %02x).",
                  sub_x, sub_y, sub_rect.w, sub_rect.h, vkrend_renderer_surface[0], vkrend_renderer_surface[1], vkrend_renderer_surface[2],
                  vkrend_renderer_surface[3], vkrend_renderer_clear[0], vkrend_renderer_clear[1], vkrend_renderer_clear[2],
                  vkrend_renderer_clear[3]);
    }

    /*
     * Step 2: a non-identity actor transform. A model translation of +0.25 world units must
     * move every surface pixel right by the projected 0.25 units, and nothing vertically. A
     * zero shift means the MVP dropped the model matrix; a wrong one means it dropped the
     * projection.
     */
    shift = BR_MUL(BR_SCALAR(0.25), pixels);

    actor->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&actor->t.t.mat, BR_SCALAR(0.25), BR_SCALAR(0.0), BR_SCALAR(0.0));

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto fail;

    BrZsSceneRender(state->world, camera, state->offscreen);

    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(0.0), BR_SCALAR(0.0), &expected_x, &expected_y);

    if(vkrend_renderer_assert_pixel(state->offscreen, expected_x, expected_y, vkrend_renderer_surface, "the moved triangle centre") != BRE_OK)
        goto fail;

    if((readback = vkrend_renderer_read(state->offscreen)) == NULL)
        goto fail;

    vkrend_renderer_bounds_find(readback, vkrend_renderer_surface, &moved_bounds);
    BrPixelmapFree(readback);

    if(vkrend_renderer_assert_near(moved_bounds.min_x - identity_bounds.min_x, (br_int_32)shift, 1, "the model translation's min_x shift") != BRE_OK)
        goto fail;

    if(vkrend_renderer_assert_near(moved_bounds.max_x - identity_bounds.max_x, (br_int_32)shift, 1, "the model translation's max_x shift") != BRE_OK)
        goto fail;

    if(vkrend_renderer_assert_near(moved_bounds.min_y - identity_bounds.min_y, 0, 1, "the model translation's min_y shift") != BRE_OK)
        goto fail;

    if(vkrend_renderer_assert_near(moved_bounds.max_y - identity_bounds.max_y, 0, 1, "the model translation's max_y shift") != BRE_OK)
        goto fail;

    /*
     * The camera is in the model-to-view chain too: move it the same way and the triangle
     * must move the other way by the same number of texels. The actor goes back to identity.
     */
    actor->t.type = BR_TRANSFORM_IDENTITY;
    BrMatrix34Identity(&actor->t.t.mat);
    BrMatrix34Translate(&camera->t.t.mat, BR_SCALAR(0.25), BR_SCALAR(0.0), VKREND_RENDERER_CAMERA_DISTANCE);

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto fail;

    BrZsSceneRender(state->world, camera, state->offscreen);

    if((readback = vkrend_renderer_read(state->offscreen)) == NULL)
        goto fail;

    vkrend_renderer_bounds_find(readback, vkrend_renderer_surface, &moved_bounds);
    BrPixelmapFree(readback);

    if(vkrend_renderer_assert_near(moved_bounds.min_x - identity_bounds.min_x, -(br_int_32)shift, 1, "the camera translation's min_x shift") != BRE_OK)
        goto fail;

    if(vkrend_renderer_assert_near(moved_bounds.max_x - identity_bounds.max_x, -(br_int_32)shift, 1, "the camera translation's max_x shift") != BRE_OK)
        goto fail;

    /*
     * Step 3: the two-group model. Each group must draw its own colour: the left quad's
     * centre projects to model (-0.75, 0) and the right quad's to (0.75, 0), and the gap
     * between them stays clear, so a draw that only rendered the first group fails.
     */
    BrMatrix34Translate(&camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), VKREND_RENDERER_CAMERA_DISTANCE);

    actor->model    = state->multi_model;
    actor->material = state->multi_left;

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto fail;

    BrZsSceneRender(state->world, camera, state->offscreen);

    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(-0.75), BR_SCALAR(0.0), &expected_x, &expected_y);

    if(vkrend_renderer_assert_pixel(state->offscreen, expected_x, expected_y, vkrend_renderer_surface, "the left group") != BRE_OK)
        goto fail;

    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(0.75), BR_SCALAR(0.0), &expected_x, &expected_y);

    if(vkrend_renderer_assert_pixel(state->offscreen, expected_x, expected_y, vkrend_renderer_surface_right, "the right group") != BRE_OK)
        goto fail;

    if(vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 2, vkrend_renderer_clear,
                                    "the gap between the groups") != BRE_OK)
        goto fail;

    if((readback = vkrend_renderer_read(state->offscreen)) == NULL)
        goto fail;

    vkrend_renderer_bounds_find(readback, vkrend_renderer_surface, &left_bounds);
    vkrend_renderer_bounds_find(readback, vkrend_renderer_surface_right, &right_bounds);
    BrPixelmapFree(readback);

    if(left_bounds.count < 100 || right_bounds.count < 100) {
        BrLogError("RT", "vkrend: the multi-group model drew too little: %d left pixels, %d right pixels.", left_bounds.count, right_bounds.count);
        goto fail;
    }

    /*
     * The surface pixels must cover the union of the two quads' predicted screen bounds. A
     * wrong per-group vertex or index offset moves a group, so a bound escapes the window.
     */
    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(-1.2), BR_SCALAR(0.0), &expected_x, &expected_y);

    if(vkrend_renderer_assert_near(left_bounds.min_x, expected_x, 2, "the left group's min_x") != BRE_OK)
        goto fail;

    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(1.2), BR_SCALAR(0.0), &expected_x, &expected_y);

    if(vkrend_renderer_assert_near(right_bounds.max_x, expected_x, 2, "the right group's max_x") != BRE_OK)
        goto fail;

    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(0.0), BR_SCALAR(-0.6), &expected_x, &expected_y);

    if(vkrend_renderer_assert_near(left_bounds.min_y, expected_y, 2, "the multi-group min_y") != BRE_OK)
        goto fail;

    if(vkrend_renderer_assert_near(right_bounds.min_y, expected_y, 2, "the multi-group min_y") != BRE_OK)
        goto fail;

    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(0.0), BR_SCALAR(0.6), &expected_x, &expected_y);

    if(vkrend_renderer_assert_near(left_bounds.max_y, expected_y, 2, "the multi-group max_y") != BRE_OK)
        goto fail;

    if(vkrend_renderer_assert_near(right_bounds.max_y, expected_y, 2, "the multi-group max_y") != BRE_OK)
        goto fail;

    /*
     * Step 4: lighting. The camera is still at +z and the actor at identity, so the
     * triangle's normal is +z in model and view space, and the directional light's
     * view-space direction is the light transform's third column (the camera transform is a
     * pure translation, so it does not rotate the direction). dot(N, L) is that column's z
     * component, mat.m[2][2].
     */

    /* An unlit surface is the flat colour, whatever the lights are. */
    if(vkrend_renderer_render_centre(state, camera, actor, state->unlit_model, state->unlit_material, vkrend_renderer_white,
                                     "the unlit surface") != BRE_OK)
        goto fail;

    /* Ambient 1, a +z directional light, dot 1: (0.25 * 1 + 1 * 0.5) = 0.75 -> 0xbf. */
    vkrend_renderer_lit_expected(BR_SCALAR(0.25), BR_SCALAR(0.5), BR_SCALAR(1.0), lit_expected);

    if(vkrend_renderer_render_centre(state, camera, actor, state->lit_model, state->lit_material, lit_expected, "the lit surface") != BRE_OK)
        goto fail;

    /* Turn the light to +x: dot 0, so only the ambient term is left. */
    state->direct_light->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34RotateY(&state->direct_light->t.t.mat, BR_ANGLE_DEG(90.0));

    dot = state->direct_light->t.t.mat.m[2][2];
    vkrend_renderer_lit_expected(BR_SCALAR(0.25), BR_SCALAR(0.5), dot, lit_expected);

    if(vkrend_renderer_render_centre(state, camera, actor, state->lit_model, state->lit_material, lit_expected, "the moved light") != BRE_OK)
        goto fail;

    /* Back to +z, and raise ka/kd: (0.5 * 1 + 1 * 0.5) = 1.0 -> 0xff. */
    BrMatrix34Identity(&state->direct_light->t.t.mat);
    vkrend_renderer_lit_expected(BR_SCALAR(0.5), BR_SCALAR(0.5), BR_SCALAR(1.0), lit_expected);

    if(vkrend_renderer_render_centre(state, camera, actor, state->dim_model, state->dim_material, lit_expected, "the raised ka/kd") != BRE_OK)
        goto fail;

    /* A light pointing the other way: dot -1 clamps to 0, so ambient alone again. */
    BrMatrix34RotateY(&state->direct_light->t.t.mat, BR_ANGLE_DEG(180.0));

    dot = state->direct_light->t.t.mat.m[2][2];
    vkrend_renderer_lit_expected(BR_SCALAR(0.25), BR_SCALAR(0.5), dot, lit_expected);

    if(vkrend_renderer_render_centre(state, camera, actor, state->lit_model, state->lit_material, lit_expected, "the surface behind the light") != BRE_OK)
        goto fail;

    /*
     * The core culls a light whose radius cannot reach the model (BrLightCullReset()). Swap
     * the directional light for the point light: 10 units away with a 20-unit radius it is
     * not culled and adds dot 1 * kd 0.5, and moved to 100 units it is culled and adds
     * nothing. The point light's attenuation is constant (c=1, l=q=0), so distance alone
     * would not dim it - only the cull can.
     */
    BrLightDisable(state->direct_light);
    BrLightEnable(state->point_light);

    vkrend_renderer_lit_expected(BR_SCALAR(0.25), BR_SCALAR(0.5), BR_SCALAR(1.0), lit_expected);

    if(vkrend_renderer_render_centre(state, camera, actor, state->lit_model, state->lit_material, lit_expected, "the point light") != BRE_OK)
        goto fail;

    BrMatrix34Translate(&state->point_light->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(100.0));
    vkrend_renderer_lit_expected(BR_SCALAR(0.25), BR_SCALAR(0.5), BR_SCALAR(0.0), lit_expected);

    if(vkrend_renderer_render_centre(state, camera, actor, state->lit_model, state->lit_material, lit_expected, "the culled point light") != BRE_OK)
        goto fail;

    /*
     * Step 4c: the cutoff volume. The volume light is the same white direct light as step
     * 4, plus a single region whose plane is the view-space half-space x <= 0, with a
     * falloff of 0.5. Reading the lit triangle along view y = -0.8, where it spans x in
     * [-0.9, 0.9], shows the volume rule with the ambient term as the floor:
     *
     *   x = -0.5  inside the region, atten 1.0 -> (0.25*1 + 1*0.5*1.0) = 0.75 -> 0xbf
     *   x =  0.25 outside, 0.25 into the falloff, atten 1 - 0.25/0.5 = 0.5
     *                    -> (0.25*1 + 1*0.5*0.5) = 0.5 -> 0x80
     *   x =  0.6  outside and past the falloff, atten 0.0 -> ambient only 0.25 -> 0x40
     *
     * The middle expectation comes from glrend's lightVolumeAttenuation(): the fragment is
     * outside the plane by less than the falloff, so the attenuation is 1 - dist/0.5. A
     * renderer that ignored the volume lights all three points to 0xbf; one that applied the
     * falloff backwards darkens the left instead. The readback samples the pixel centre, half
     * a texel right of the projected point, so the falloff value is evaluated from that
     * position rather than the one projected.
     *
     * vkrend_renderer_project() computes its row as height/2 + y*pixels, while the scene
     * draw flips y so that view +y is the image top (see the offscreen orientation witness);
     * the pixel for view y is therefore the one project() names for -y. The volume's geometry
     * is written in view space, so only the reads negate.
     */
    const br_scalar read_y         = BR_SCALAR(-0.8);
    br_uint_8       volume_falloff = 0;

    BrLightDisable(state->point_light);
    BrLightEnable(state->volume_light);

    actor->model    = state->lit_model;
    actor->material = state->lit_material;

    if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
        goto fail;

    BrZsSceneRender(state->world, camera, state->offscreen);

    /* x = -0.5: inside the region, so the direct light contributes in full. */
    vkrend_renderer_lit_expected(BR_SCALAR(0.25), BR_SCALAR(0.5), BR_SCALAR(1.0), lit_expected);

    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(-0.5), BR_NEG(read_y), &expected_x, &expected_y);

    if(vkrend_renderer_assert_pixel(state->offscreen, expected_x, expected_y, lit_expected, "the volume light inside the region") != BRE_OK)
        goto fail;

    /*
     * x = 0.25: outside, half way into the falloff, so the direct light is halved. The
     * readback samples the pixel centre, which is half a texel right of the projected point
     * and so slightly deeper into the falloff; evaluate glrend's formula at the view x the
     * sampled pixel centre maps to, not the x that was projected.
     */
    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(0.25), BR_NEG(read_y), &expected_x, &expected_y);

    {
        br_scalar pixels = vkrend_renderer_pixels_per_unit(state->offscreen, fov, distance);
        br_scalar x_frag = BR_DIV(BR_SUB(BR_ADD(BR_SCALAR(expected_x), BR_SCALAR(0.5)), BR_SCALAR(state->offscreen->width / 2)), pixels);
        br_scalar atten  = BR_SUB(BR_SCALAR(1.0), BR_DIV(x_frag, VKREND_VOLUME_FALLOFF));

        volume_falloff = vkrend_renderer_lit_channel(BR_SCALAR(0.25), BR_SCALAR(1.0), BR_SCALAR(0.5), BR_SCALAR(1.0), atten, BR_SCALAR(1.0));
        lit_expected[0] = lit_expected[1] = lit_expected[2] = volume_falloff;
        lit_expected[3]                                     = 0xff;
    }

    if(vkrend_renderer_assert_pixel_near(state->offscreen, expected_x, expected_y, lit_expected, 1, "the volume light in the falloff") != BRE_OK)
        goto fail;

    /* x = 0.6: outside and past the falloff, so only the ambient term is left. */
    vkrend_renderer_lit_expected(BR_SCALAR(0.25), BR_SCALAR(0.5), BR_SCALAR(0.0), lit_expected);

    vkrend_renderer_project(state->offscreen, fov, distance, BR_SCALAR(0.6), BR_NEG(read_y), &expected_x, &expected_y);

    if(vkrend_renderer_assert_pixel(state->offscreen, expected_x, expected_y, lit_expected, "the volume light outside the region") != BRE_OK)
        goto fail;

    BrLogInfo("RT", "vkrend: volume witness passed (inside 0xbf, falloff 0x%02x, outside 0x40 along y = -0.8).", volume_falloff);

    /* Restore the lights step 4 left behind: the volume light off, the point light on. */
    BrLightDisable(state->volume_light);
    BrLightEnable(state->point_light);

    /*
     * Step 4b: fog. The triangle material is unlit and the triangle sits at view distance 8,
     * so the step renders it with fog on and asserts the fogged centre pixel against glrend's
     * formula, then with fog off and asserts the plain surface. See the step for the bytes.
     */
    if(vkrend_renderer_fog_step(state, camera, actor) != BRE_OK)
        goto fail;

    /*
     * Step 5 (stage 6): textures. The camera is still at +z and the actor at identity, so a
     * quad spanning model (-1, -1)..(1, 1) projects to a centred screen quadrilateral. Its
     * four corners carry the map coordinates (0,0), (0,1), (1,0) and (1,1), and the map is
     * 2x2, so each quadrant samples one texel.
     *
     * The image's row 0 is the top of the offscreen (the scene vertex shader flips y), and a
     * BRender map's v = 0 is the top row of the map, so model +y (up) with v = 0 lands at the
     * image top and samples the map's top row: red top-left, green top-right, blue
     * bottom-left and black bottom-right.
     *
     * A material turns the colour key on by default (core/v1db/prepmatl.c sets BRT_COLOUR_KEY_B
     * from BR_MATF_DISABLE_COLOUR_KEY), so the black corner is discarded and shows the cleared
     * background; BR_MATF_DISABLE_COLOUR_KEY turns the key off and the same corner draws black.
     * A flipped UV, a missing sample or a map transform applied with the wrong sign moves one of
     * the coloured quadrants.
     */
    {
        const br_int_32 cx = state->offscreen->width / 2;
        const br_int_32 cy = state->offscreen->height / 2;
        const br_int_32 q  = (br_int_32)(pixels * BR_SCALAR(0.5));

        if(q < 4) {
            BrLogError("RT", "vkrend: the renderer offscreen is too small for the texture step (%dx%d, %d texels per quadrant).",
                       state->offscreen->width, state->offscreen->height, q);
            goto fail;
        }

        actor->model    = state->texture_model;
        actor->material = state->texture_material;

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy - q, vkrend_renderer_tex_red, "the texture's top-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy - q, vkrend_renderer_tex_green, "the texture's top-right quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy + q, vkrend_renderer_tex_blue, "the texture's bottom-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy + q, vkrend_renderer_clear,
                                        "the texture's colour-keyed bottom-right quadrant") != BRE_OK)
            goto fail;

        /*
         * A non-identity map transform: dv = 1 - v flips the map vertically. Every quadrant
         * must now show the texel from the opposite row, which a renderer that ignored the
         * transform (or applied it backwards) cannot produce.
         */
        BrMatrix23Identity(&state->texture_material->map_transform);
        state->texture_material->map_transform.m[1][1] = BR_SCALAR(-1.0);
        state->texture_material->map_transform.m[2][1] = BR_SCALAR(1.0);
        BrMaterialUpdate(state->texture_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy - q, vkrend_renderer_tex_blue, "the flipped texture's top-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy - q, vkrend_renderer_clear,
                                        "the flipped texture's colour-keyed top-right quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy + q, vkrend_renderer_tex_red,
                                        "the flipped texture's bottom-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy + q, vkrend_renderer_tex_green,
                                        "the flipped texture's bottom-right quadrant") != BRE_OK)
            goto fail;

        /* Restore the identity transform so the log line's geometry is the unflipped quad. */
        BrMatrix23Identity(&state->texture_material->map_transform);
        BrMaterialUpdate(state->texture_material, BR_MATU_ALL);

        /*
         * Disabling the colour key makes the black corner draw black instead of discarding,
         * while the other three corners are unchanged. That is the other side of the discard
         * the assertions above rely on.
         */
        state->texture_material->flags |= BR_MATF_DISABLE_COLOUR_KEY;
        BrMaterialUpdate(state->texture_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy - q, vkrend_renderer_tex_red,
                                        "the colour-key-disabled texture's top-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy - q, vkrend_renderer_tex_green,
                                        "the colour-key-disabled texture's top-right quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy + q, vkrend_renderer_tex_blue,
                                        "the colour-key-disabled texture's bottom-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy + q, vkrend_renderer_tex_black,
                                        "the colour-key-disabled texture's black bottom-right quadrant") != BRE_OK)
            goto fail;

        state->texture_material->flags &= ~BR_MATF_DISABLE_COLOUR_KEY;
        BrMaterialUpdate(state->texture_material, BR_MATU_ALL);

        /*
         * The lit textured pipeline. The same quad, lit by the ambient and the +z directional
         * light from step 4, must show `(ka * ambient + dot(N, L) * kd) * texel` per channel:
         * dot is 1, so a white surface would be 0xbf and each texel channel scales from there.
         * The black corner is still discarded by the colour key.
         */
        BrLightDisable(state->point_light);
        BrMatrix34Identity(&state->direct_light->t.t.mat);
        BrLightEnable(state->direct_light);

        state->texture_material->flags |= BR_MATF_LIGHT;
        state->texture_material->ka = BR_SCALAR(0.25);
        state->texture_material->kd = BR_SCALAR(0.5);
        state->texture_material->ks = BR_SCALAR(0.0);
        BrMaterialUpdate(state->texture_material, BR_MATU_ALL);

        /*
         * One lit surface channel: `(ka * ambient + dot * kd * light) * surface`, the same
         * vkrend_renderer_lit_channel() the untextured lit step uses, with the texel channel
         * as `surface`.
         */
        const br_uint_8 lit_red[4] = {
            0x00, 0x00,
            vkrend_renderer_lit_channel(BR_SCALAR(0.25), BR_SCALAR(1.0), BR_SCALAR(0.5), BR_SCALAR(1.0), BR_SCALAR(1.0), BR_SCALAR(1.0)), 0xff};
        const br_uint_8 lit_green[4] = {
            0x00, vkrend_renderer_lit_channel(BR_SCALAR(0.25), BR_SCALAR(1.0), BR_SCALAR(0.5), BR_SCALAR(1.0), BR_SCALAR(1.0), BR_SCALAR(1.0)),
            0x00, 0xff};
        const br_uint_8 lit_blue[4] = {
            vkrend_renderer_lit_channel(BR_SCALAR(0.25), BR_SCALAR(1.0), BR_SCALAR(0.5), BR_SCALAR(1.0), BR_SCALAR(1.0), BR_SCALAR(1.0)),
            0x00, 0x00, 0xff};

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy - q, lit_red, "the lit textured top-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy - q, lit_green, "the lit textured top-right quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy + q, lit_blue, "the lit textured bottom-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy + q, vkrend_renderer_clear,
                                        "the lit textured colour-keyed bottom-right quadrant") != BRE_OK)
            goto fail;

        state->texture_material->flags &= ~BR_MATF_LIGHT;
        BrMaterialUpdate(state->texture_material, BR_MATU_ALL);

        /*
         * The same quad with an INDEX_8 colour map: each quadrant samples one index, so the
         * index fetch, the palette lookup, its row order and its channel swizzle are all read
         * back. Indices 1, 2 and 3 name the palette's red, green and blue rows; index 0 is
         * the transparent entry, so the bottom-right quadrant is discarded. A wrong index
         * lookup, a CLUT addressed by column rather than row, or a swapped channel would
         * recolour or move a quadrant.
         */
        actor->model    = state->indexed_model;
        actor->material = state->indexed_material;

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy - q, vkrend_renderer_tex_red, "the indexed texture's top-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy - q, vkrend_renderer_tex_green,
                                        "the indexed texture's top-right quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy + q, vkrend_renderer_tex_blue,
                                        "the indexed texture's bottom-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy + q, vkrend_renderer_clear,
                                        "the indexed texture's colour-keyed bottom-right quadrant") != BRE_OK)
            goto fail;

        /*
         * Disabling the colour key draws palette entry 0 (33 22 11 ff) at the bottom-right
         * quadrant instead of discarding it, which is the other side of the index-0 key.
         */
        state->indexed_material->flags |= BR_MATF_DISABLE_COLOUR_KEY;
        BrMaterialUpdate(state->indexed_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy + q, vkrend_renderer_index_zero, "the indexed texture's palette entry 0") != BRE_OK)
            goto fail;

        state->indexed_material->flags &= ~BR_MATF_DISABLE_COLOUR_KEY;
        BrMaterialUpdate(state->indexed_material, BR_MATU_ALL);

        /*
         * The render-to-texture path: the same quad sampling an offscreen a fill/draw has
         * written, rather than a host pixelmap. The colour map is a device pixelmap sampled
         * where its image lies, so a fallback to a host upload or BRE_FAIL leaves the quad
         * untextured and every quadrant below fails. The bottom-right texel is black and is
         * discarded by the colour key.
         */
        actor->model    = state->rt_model;
        actor->material = state->rt_material;

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy - q, vkrend_renderer_tex_red, "the render-to-texture's top-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy - q, vkrend_renderer_tex_green,
                                        "the render-to-texture's top-right quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy + q, vkrend_renderer_tex_blue,
                                        "the render-to-texture's bottom-left quadrant") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy + q, vkrend_renderer_clear,
                                        "the render-to-texture's colour-keyed bottom-right quadrant") != BRE_OK)
            goto fail;

        state->rt_material->flags |= BR_MATF_DISABLE_COLOUR_KEY;
        BrMaterialUpdate(state->rt_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy + q, vkrend_renderer_tex_black,
                                        "the render-to-texture's black bottom-right quadrant") != BRE_OK)
            goto fail;

        state->rt_material->flags &= ~BR_MATF_DISABLE_COLOUR_KEY;
        BrMaterialUpdate(state->rt_material, BR_MATU_ALL);
    }

    /*
     * Step 5b: mip and filter selection. The same quad samples a 16x16 checkerboard minified
     * into the mip chain and the step-5 2x2 map across a texel boundary; see the step for
     * what each assertion pins down.
     */
    if(vkrend_renderer_mipmap_step(state, camera, actor) != BRE_OK)
        goto fail;

    /*
     * Step 6: culling. The triangle's front face is +z, so an actor rotated 180 degrees
     * about y presents its back to the camera at +z. A one-sided material - the default -
     * culls back faces, so the centre pixel must read the clear colour. Marking the same
     * material two-sided draws it, so the cull state is what decides. Culling the wrong
     * side would show the surface in the first case, and no cull at all would too.
     */
    actor->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34RotateY(&actor->t.t.mat, BR_ANGLE_DEG(180.0));

    if(vkrend_renderer_render_centre(state, camera, actor, state->model, state->material, vkrend_renderer_clear, "the one-sided back face") != BRE_OK)
        goto fail;

    state->material->flags |= BR_MATF_TWO_SIDED;
    BrMaterialUpdate(state->material, BR_MATU_ALL);

    if(vkrend_renderer_render_centre(state, camera, actor, state->model, state->material, vkrend_renderer_surface, "the two-sided back face") != BRE_OK)
        goto fail;

    state->material->flags &= ~BR_MATF_TWO_SIDED;
    BrMaterialUpdate(state->material, BR_MATU_ALL);
    actor->t.type = BR_TRANSFORM_IDENTITY;
    BrMatrix34Identity(&actor->t.t.mat);

    /*
     * Step 6b: the front face a reflecting model-view selects. An x mirror reverses the
     * screen winding, so the draw must flip its front face as glrend does; see the step
     * for why the two draws assert opposite bytes at the same pixels.
     */
    if(vkrend_renderer_winding_step(state, camera, actor, camera_data) != BRE_OK)
        goto fail;

    /*
     * Step 7: vertex colours. A BR_MATF_PRELIT-only material makes the core set
     * colour_source to BRT_GEOMETRY, so the fragment colour is the interpolated vertex
     * colour, not the surface colour. The triangle's bottom two vertices are red and its
     * top vertex blue, so the image centre - half way between them - is an even mix, the
     * point towards the top vertex is predominantly blue and the point towards the bottom
     * edge predominantly red. The surface colour is green, so a renderer that ignored the
     * attribute would show green at all three points.
     */
    {
        const br_int_32 cx   = state->offscreen->width / 2;
        const br_int_32 cy   = state->offscreen->height / 2;
        const br_int_32 half = (br_int_32)BR_MUL(BR_SCALAR(0.6), pixels);

        actor->model    = state->vc_model;
        actor->material = state->vc_material;

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        /* Model +y is the image top (see the texture step), so the blue vertex sits above the centre. */
        if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, cy, vkrend_renderer_vc_centre, 4, "the vertex-colour centroid") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, cy - half, vkrend_renderer_vc_blue, 4, "the vertex-colour blue end") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, cy + half, vkrend_renderer_vc_red, 4, "the vertex-colour red end") != BRE_OK)
            goto fail;
    }

    /*
     * Step 8: pre-lighting. A BR_MATF_PRELIT|BR_MATF_LIGHT material keeps the surface
     * colour for the shaded term - the same white surface, ka 0.25 and kd 0.5, and the
     * white ambient and +z directional light as step 4, giving 0.75 a channel - and the
     * fragment stage then adds the vertex colour, which for a pre-lit model is the lighting
     * result the core wrote into group->vertex_colours. With a uniform vertex red of 64 the
     * red channel adds 0.251 and clips to full, while green and blue stay at 0.75 (0xbf). A
     * renderer that dropped the pre-lighting term would show 0xbf in red.
     */
    {
        if(vkrend_renderer_render_centre_near(state, camera, actor, state->prelit_model, state->prelit_material, vkrend_renderer_prelit, 2,
                                              "the pre-lit surface") != BRE_OK)
            goto fail;
    }

    /*
     * Step 9: blending. An opacity of 128 makes the core set PRIMF_BLEND and bucket the
     * surface transparent. Drawn over a black background its white surface must blend as
     * srcAlpha*src + (1 - srcAlpha)*dst, so each channel is 128/255 of white and rounds to
     * 0x80; the alpha blend factor is one, so it is 0x80 too. A renderer that never enabled
     * blending would write the surface colour opaque and read 0xff in red, green and blue.
     * The triangle's face carries state->material, which is the state the draw restores.
     */
    {
        br_rectangle whole = {
            .x = -state->offscreen->origin_x,
            .y = -state->offscreen->origin_y,
            .w = state->offscreen->width,
            .h = state->offscreen->height,
        };

        state->material->colour  = BR_COLOUR_RGB(255, 255, 255);
        state->material->opacity = 128;
        state->material->flags &= ~(BR_MATF_LIGHT | BR_MATF_TWO_SIDED);
        BrMaterialUpdate(state->material, BR_MATU_ALL);

        actor->model    = state->model;
        actor->material = state->material;

        if(DevicePixelmapRectangleFill(state->offscreen, &whole, BR_COLOUR_RGB(0, 0, 0)) != BRE_OK) {
            BrLogError("RT", "vkrend: clearing the offscreen to black for the blend step failed.");
            goto fail;
        }

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 2, vkrend_renderer_blend,
                                        "the blended surface") != BRE_OK)
            goto fail;
    }

    /*
     * Step 10: Z-sort occlusion. Two triangles overlap the image centre, the far one red and
     * the near one blue. They carry different materials, so the prepared model splits them
     * into two groups. With no depth buffer the order table must order them by the face
     * centroid depth. Given a BUCKET_SORT table, the per-group path gives both groups the
     * same depth (the model origin) and draws the group listed first last, so the far red
     * triangle wins; the per-face path sorts the near blue face last. The centre pixel must
     * therefore be blue, and a per-group draw reads red instead.
     */
    {
        state->occlusion_order_table->min_z = camera_data->hither_z;
        state->occlusion_order_table->max_z = camera_data->yon_z;

        actor->t.type = BR_TRANSFORM_IDENTITY;
        BrMatrix34Identity(&actor->t.t.mat);
        actor->model    = state->occlusion_model;
        actor->material = state->occlusion_far;

        BrZsActorOrderTableSet(actor, state->occlusion_order_table);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, state->offscreen->width / 2, state->offscreen->height / 2,
                                        vkrend_renderer_occlusion_near, "the Z-sorted near face") != BRE_OK)
            goto fail;

        BrZsActorOrderTableSet(actor, NULL);
    }

    /*
     * Step 11: clip planes. BRT_CLIP is stored and advertised and the core fills it from the
     * enabled clip-plane actors, but it only reaches the rasteriser if the vertex stage emits
     * gl_ClipDistance. The clip plane is the world-space half-space x >= 0, so with it disabled
     * both halves of the triangle draw, and with it enabled the left half must read the clear
     * colour while the right half keeps the surface colour. A renderer that ignored the planes
     * would draw the left half in both cases, so the first pair of assertions cannot pass for
     * the wrong reason.
     */
    {
        const br_int_32 cx = state->offscreen->width / 2;
        const br_int_32 cy = state->offscreen->height / 2;
        const br_int_32 q  = (br_int_32)(pixels * BR_SCALAR(0.25));

        /* The blend step left the surface white and half-opaque; restore the triangle's. */
        state->material->colour  = BR_COLOUR_RGB(0xaa, 0xbb, 0xcc);
        state->material->opacity = 255;
        state->material->flags &= ~(BR_MATF_LIGHT | BR_MATF_TWO_SIDED);
        BrMaterialUpdate(state->material, BR_MATU_ALL);

        actor->t.type = BR_TRANSFORM_IDENTITY;
        BrMatrix34Identity(&actor->t.t.mat);
        actor->model    = state->model;
        actor->material = state->material;

        /* Disabled: the whole triangle draws, both sides of x = 0. */
        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy, vkrend_renderer_surface, "the unclipped left half") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy, vkrend_renderer_surface, "the unclipped right half") != BRE_OK)
            goto fail;

        /* Enabled: the plane cuts the left half. */
        BrClipPlaneEnable(state->clip_plane);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx - q, cy, vkrend_renderer_clear, "the clipped left half") != BRE_OK)
            goto fail;

        if(vkrend_renderer_assert_pixel(state->offscreen, cx + q, cy, vkrend_renderer_surface, "the surviving right half") != BRE_OK)
            goto fail;

        BrClipPlaneDisable(state->clip_plane);
    }

    /*
     * Step 12 (Zb): the depth buffer. Two overlapping triangles - the far one red (group 0),
     * the near one blue - rendered through BrZbSceneRender() with a depth buffer. Under Zs
     * the order table's per-face split resolves the overlap; under Zb the groups are
     * deferred whole and share the model origin's depth, so the order table's tie-break
     * draws the far group last and only the depth test can put the near one on top. The
     * reversed-order model, whose group 0 is the near triangle, must read the near colour
     * at the centre too, so neither draw order decides. A renderer without a working depth
     * test shows the far red face in the first case.
     */
    {
        const br_int_32 cx = state->offscreen->width / 2;
        const br_int_32 cy = state->offscreen->height / 2;

        actor->t.type = BR_TRANSFORM_IDENTITY;
        BrMatrix34Identity(&actor->t.t.mat);
        actor->material = state->occlusion_far;

        /* The far group is group 0, so the order-table tie-break draws it last. */
        actor->model = state->occlusion_model;

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZbSceneRender(state->world, camera, state->offscreen, state->depth);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx, cy, vkrend_renderer_occlusion_near, "the Z-buffered near face") != BRE_OK)
            goto fail;

        /* The near group is group 0 now; the depth test must still leave the near face. */
        actor->model = state->depth_rev_model;

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZbSceneRender(state->world, camera, state->offscreen, state->depth);

        if(vkrend_renderer_assert_pixel(state->offscreen, cx, cy, vkrend_renderer_occlusion_near, "the Z-buffered near face, reversed order") != BRE_OK)
            goto fail;
    }

    /*
     * Step 13: shading modes. The material's BRT_SHADING_MODE_T names where the lighting is
     * evaluated. The triangle's three vertices carry distinct normals, so a vertex tilted
     * away from the +z light contributes a dimmer value to an interpolated per-vertex
     * result than the normal it contributes to a per-fragment evaluation, and the two
     * renders must differ. The pixel nearest the centroid is asserted against
     * vkrend_renderer_shading_oracle() for each mode, computed here without the renderer,
     * and the two reads must differ.
     *
     * The lights are as step 5 left them - the white ambient on, the white direct light at
     * identity (+z) and the point light off - which is what the oracle assumes.
     */
    {
        const br_int_32 cx  = state->offscreen->width / 2;
        const br_int_32 cy  = state->offscreen->height / 2;
        const br_int_32 row = cy + (br_int_32)BR_DIV(pixels, BR_SCALAR(3.0));

        BrLightDisable(state->point_light);
        BrMatrix34Identity(&state->direct_light->t.t.mat);
        BrLightEnable(state->direct_light);

        actor->t.type = BR_TRANSFORM_IDENTITY;
        BrMatrix34Identity(&actor->t.t.mat);
        actor->model    = state->smooth_model;
        actor->material = state->smooth_material;

        gouraud_expected[0] = gouraud_expected[1] = gouraud_expected[2] = vkrend_renderer_shading_oracle(state, pixels, cx, row, BR_TRUE);
        phong_expected[0] = phong_expected[1] = phong_expected[2] = vkrend_renderer_shading_oracle(state, pixels, cx, row, BR_FALSE);
        gouraud_expected[3] = phong_expected[3] = 0xff;

        if(gouraud_expected[0] == phong_expected[0]) {
            BrLogError("RT", "vkrend: the GOURAUD and PHONG oracles coincide at %02x, so the step cannot tell the modes apart.",
                       gouraud_expected[0]);
            goto fail;
        }

        state->smooth_material->mode = (state->smooth_material->mode & ~BR_MATM_SHADING_MODE_MASK) | BR_MATM_SHADING_MODE_GOURAUD;
        BrMaterialUpdate(state->smooth_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, row, gouraud_expected, 3, "the GOURAUD surface") != BRE_OK)
            goto fail;

        if(vkrend_renderer_read_pixel(state->offscreen, cx, row, gouraud_pixel) != BRE_OK)
            goto fail;

        state->smooth_material->mode = (state->smooth_material->mode & ~BR_MATM_SHADING_MODE_MASK) | BR_MATM_SHADING_MODE_PHONG;
        BrMaterialUpdate(state->smooth_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, row, phong_expected, 3, "the PHONG surface") != BRE_OK)
            goto fail;

        if(vkrend_renderer_read_pixel(state->offscreen, cx, row, phong_pixel) != BRE_OK)
            goto fail;

        if(gouraud_pixel[0] == phong_pixel[0] && gouraud_pixel[1] == phong_pixel[1] && gouraud_pixel[2] == phong_pixel[2] &&
           gouraud_pixel[3] == phong_pixel[3]) {
            BrLogError("RT", "vkrend: GOURAUD and PHONG rendered the same pixel %02x %02x %02x %02x.", gouraud_pixel[0], gouraud_pixel[1],
                       gouraud_pixel[2], gouraud_pixel[3]);
            goto fail;
        }

        /*
         * The lit-textured pipeline honours the same split. The smooth model's map
         * coordinates are all (0,0), so the witness texture's red texel modulates the white
         * surface and each mode must render (0, 0, light, 0xff) in the readback's B G R A
         * order, from the same oracle the untextured path asserted.
         */
        state->smooth_material->colour_map = state->texture;
        BrMaterialUpdate(state->smooth_material, BR_MATU_ALL);

        const br_uint_8 tex_gouraud[4] = {0x00, 0x00, gouraud_expected[0], 0xff};
        const br_uint_8 tex_phong[4]   = {0x00, 0x00, phong_expected[0], 0xff};

        state->smooth_material->mode = (state->smooth_material->mode & ~BR_MATM_SHADING_MODE_MASK) | BR_MATM_SHADING_MODE_GOURAUD;
        BrMaterialUpdate(state->smooth_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, row, tex_gouraud, 3, "the textured GOURAUD surface") != BRE_OK)
            goto fail;

        state->smooth_material->mode = (state->smooth_material->mode & ~BR_MATM_SHADING_MODE_MASK) | BR_MATM_SHADING_MODE_PHONG;
        BrMaterialUpdate(state->smooth_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel_near(state->offscreen, cx, row, tex_phong, 3, "the textured PHONG surface") != BRE_OK)
            goto fail;

        state->smooth_material->colour_map = NULL;
    }

    /*
     * Step 14: environment mapping. A material with BR_MATF_ENVIRONMENT_I or _L makes the
     * core publish BRT_ENVIRONMENT_INFINITE or _LOCAL as its mapping source
     * (core/v1db/prepmatl.c), which the fragment stage turns into a sphere-map U,V from the
     * reflection of the eye and the surface normal instead of the vertex's map coordinate.
     * The model is one large triangle whose constant custom normal is (1, 1, 1)/sqrt(3), so
     * the infinite eye reflects the same direction everywhere, and the environment anchor is
     * rotated VKREND_ENV_ROT_DEG about Y so the scene's environment matrix is not the
     * identity: a renderer that dropped it samples a different texel. The read pixel sits
     * VKREND_ENV_READ_X world units off the view axis, where the two eyes reflect into
     * different texels of the 16x16 map. Each is asserted against
     * vkrend_renderer_environment_oracle(); with the flags cleared the same triangle samples
     * its plain map coordinate (0, 0) instead, the texel the falsification turns on.
     */
    {
        const br_int_32        cx           = state->offscreen->width / 2;
        const br_int_32        cy           = state->offscreen->height / 2;
        const br_int_32        offset       = (br_int_32)BR_MUL(VKREND_ENV_READ_X, pixels);
        const br_int_32        rx           = cx + offset;
        static const br_uint_8 env_plain[4] = {0x80, 0x00, 0x00, 0xff};

        if(offset < 1 || rx >= state->offscreen->width) {
            BrLogError("RT", "vkrend: the renderer offscreen is too small for the environment step (%dx%d, offset %d).",
                       state->offscreen->width, state->offscreen->height, offset);
            goto fail;
        }

        if(state->env_anchor == NULL) {
            if((state->env_anchor = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_NONE, NULL))) == NULL) {
                BrLogError("RT", "vkrend: could not allocate the environment anchor.");
                goto fail;
            }

            state->env_anchor->t.type = BR_TRANSFORM_MATRIX34;
            BrMatrix34RotateY(&state->env_anchor->t.t.mat, BR_ANGLE_DEG(VKREND_ENV_ROT_DEG));
        }

        actor->t.type = BR_TRANSFORM_IDENTITY;
        BrMatrix34Identity(&actor->t.t.mat);
        actor->model    = state->env_model;
        actor->material = state->env_material;

        vkrend_renderer_environment_oracle(pixels, distance, state->offscreen->origin_x, state->offscreen->origin_y, rx, cy, BR_TRUE,
                                           env_infinite_expected);
        vkrend_renderer_environment_oracle(pixels, distance, state->offscreen->origin_x, state->offscreen->origin_y, rx, cy, BR_FALSE,
                                           env_local_expected);

        if(env_infinite_expected[1] == env_local_expected[1] && env_infinite_expected[2] == env_local_expected[2]) {
            BrLogError("RT", "vkrend: the local and infinite eyes reflect into the same texel at (%d,%d), so the step cannot tell them apart.",
                       rx, cy);
            goto fail;
        }

        BrEnvironmentSet(state->env_anchor);

        state->env_material->flags |= BR_MATF_ENVIRONMENT_I;
        BrMaterialUpdate(state->env_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, rx, cy, env_infinite_expected, "the environment infinite-eye texel") != BRE_OK)
            goto fail;

        state->env_material->flags &= ~BR_MATF_ENVIRONMENT_I;
        state->env_material->flags |= BR_MATF_ENVIRONMENT_L;
        BrMaterialUpdate(state->env_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, rx, cy, env_local_expected, "the environment local-eye texel") != BRE_OK)
            goto fail;

        state->env_material->flags &= ~BR_MATF_ENVIRONMENT_L;
        BrMaterialUpdate(state->env_material, BR_MATU_ALL);

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if(vkrend_renderer_assert_pixel(state->offscreen, rx, cy, env_plain, "the environment plain-UV texel") != BRE_OK)
            goto fail;

        BrEnvironmentSet(NULL);

        BrLogInfo("RT", "vkrend: environment witness passed (infinite eye %02x %02x %02x %02x, local eye %02x %02x %02x %02x, plain map UV %02x %02x %02x %02x).",
                  env_infinite_expected[0], env_infinite_expected[1], env_infinite_expected[2], env_infinite_expected[3], env_local_expected[0],
                  env_local_expected[1], env_local_expected[2], env_local_expected[3], env_plain[0], env_plain[1], env_plain[2], env_plain[3]);
    }

    /*
     * Step 14b: the scene-draw-into-offscreen orientation, which the environment-map step
     * above cannot see because it samples a host map. BrZbSceneRender draws a quad whose
     * world +y vertices are red and -y vertices blue into the renderer offscreen. Image y
     * grows downward, so world +y is the top of the image: the top row must read red and
     * the bottom row blue. rdemo draws the earth into its mirror offscreen the same way and
     * then samples it, so a scene draw that put +y at the bottom would sample a vertically
     * mirrored environment without moving either U,V. See the step for what the two rows
     * prove.
     */
    if(vkrend_renderer_offscreen_orientation(state, camera, actor, camera_data) != BRE_OK)
        goto fail;

    /*
     * Step 15: the window-resize path, driven the way brdemo drives it. On a window
     * resize brdemo calls BrPixelmapResizeBuffersTV(), which resizes the colour and depth
     * buffers to match the screen. A vkrend depth pixelmap is a BRT_DEPTH use_type, so its
     * resize has to work in place; when it is rejected the fallback frees the colour and
     * depth, presents the half-built buffer and recreates both from tokens, changing the
     * depth format to the caller's default in the process. Render the Zb scene at the
     * original size, resize the front screen and both buffers much larger through that same
     * call, then render the same scene again and assert the depth test still leaves the near
     * face at the new size.
     */
    {
        const br_int_32 old_w          = state->offscreen->width;
        const br_int_32 old_h          = state->offscreen->height;
        const br_int_32 new_w          = 1920;
        const br_int_32 new_h          = 1080;
        const br_uint_8 old_depth_type = state->depth->type;
        br_pixelmap    *colour         = state->offscreen;
        br_pixelmap    *depth          = state->depth;

        actor->t.type = BR_TRANSFORM_IDENTITY;
        BrMatrix34Identity(&actor->t.t.mat);
        actor->model    = state->occlusion_model;
        actor->material = state->occlusion_far;

        if(vkrend_renderer_fill_clear(colour) != BRE_OK)
            goto fail;

        BrZbSceneRender(state->world, camera, colour, depth);

        if(vkrend_renderer_assert_pixel(colour, colour->width / 2, colour->height / 2, vkrend_renderer_occlusion_near,
                                        "the Z-buffered near face before the resize") != BRE_OK)
            goto fail;

        if(BrPixelmapResize(state->screen, new_w, new_h) == NULL) {
            BrLogError("RT", "vkrend: BrPixelmapResize() on the front screen to %dx%d failed.", new_w, new_h);
            goto fail;
        }

        if(state->screen->width != new_w || state->screen->height != new_h) {
            BrLogError("RT", "vkrend: the front screen resize left the pixelmap at %dx%d, expected %dx%d.", state->screen->width,
                       state->screen->height, new_w, new_h);
            goto fail;
        }

        state->screen->origin_x = (br_int_16)(state->screen->width / 2);
        state->screen->origin_y = (br_int_16)(state->screen->height / 2);

        /*
         * The same call brdemo's resize_buffers() makes, with the size the window settled
         * on rather than a requested one, so a platform that clamps the resize still gets
         * buffers that match the screen.
         */
        {
            const br_token_value tv[] = {
                {.t = BRT_WIDTH_I32,  .v = {.i32 = state->screen->width} },
                {.t = BRT_HEIGHT_I32, .v = {.i32 = state->screen->height}},
                {.t = BR_NULL_TOKEN,  .v = {}                            },
            };

            if(BrPixelmapResizeBuffersTV(state->screen, &colour, &depth, tv) != BRE_OK) {
                BrLogError("RT", "vkrend: BrPixelmapResizeBuffersTV() failed after the front screen resize.");
                goto fail;
            }
        }

        /*
         * A resize changes the buffers' size, not their format. A depth pixelmap whose
         * resize is unsupported sends the whole call through the recreation fallback, which
         * rebuilds the depth from the default pixel_bits and so hands back a different
         * format than the one the caller matched.
         */
        if(depth->type != old_depth_type) {
            BrLogError("RT",
                       "vkrend: the depth buffer's type changed from %d to %d across the resize; the depth resize is unsupported and "
                       "BrPixelmapResizeBuffersTV() recreated the buffers instead of resizing them in place.",
                       old_depth_type, depth->type);
            goto fail;
        }

        colour->origin_x = (br_int_16)(colour->width / 2);
        colour->origin_y = (br_int_16)(colour->height / 2);

        state->offscreen = colour;
        state->depth     = depth;

        camera_data->aspect = BR_DIV(BR_SCALAR(colour->width), BR_SCALAR(colour->height));

        if(vkrend_renderer_fill_clear(colour) != BRE_OK)
            goto fail;

        BrZbSceneRender(state->world, camera, colour, depth);

        if(vkrend_renderer_assert_pixel(colour, colour->width / 2, colour->height / 2, vkrend_renderer_occlusion_near,
                                        "the Z-buffered near face after the resize") != BRE_OK)
            goto fail;

        BrLogInfo("RT", "vkrend: depth resize witness passed (%dx%d Zb near face %02x %02x %02x %02x before and after an in-place resize to %dx%d, depth type %d kept).",
                  old_w, old_h, vkrend_renderer_occlusion_near[0], vkrend_renderer_occlusion_near[1], vkrend_renderer_occlusion_near[2],
                  vkrend_renderer_occlusion_near[3], colour->width, colour->height, depth->type);
    }

    if(vkrend_renderer_aspect_step(state, camera, actor, camera_data) != BRE_OK)
        goto fail;

    /*
     * Step 16: the per-draw material buffer's growth. RendererVkMaterialReserve() is called
     * once per draw (drivers/vkrend/gv1buckt.c) and bump-allocates from a buffer sceneBegin()
     * creates with BR_VKREND_MATERIAL_INITIAL_BLOCKS (16) blocks, growing it by doubling when a
     * scene draws more. This step draws VKREND_MATERIAL_GROWTH_DRAWS (24) one-triangle actors -
     * more than the 16 the initial buffer holds - so the scene grows it once, then reads the
     * image back. The readback flushes the frame and drains its retire list; a growth path that
     * retired the buffer it replaced - already retired when sceneBegin() created it - rather
     * than the replacement destroys the initial buffer twice there. Each triangle's projected
     * centre must also be the surface colour, so a growth that declined a draw rather than
     * reallocating leaves its triangle clear.
     */
    {
        br_actor  *growth[VKREND_MATERIAL_GROWTH_DRAWS];
        br_uint_32 i;

        for(i = 0; i < VKREND_MATERIAL_GROWTH_DRAWS; ++i) {
            const br_int_32 col = (br_int_32)(i % VKREND_MATERIAL_GROWTH_COLUMNS);
            const br_int_32 row = (br_int_32)(i / VKREND_MATERIAL_GROWTH_COLUMNS);
            const br_scalar x   = BR_MUL(BR_SUB(BR_SCALAR(col), BR_SCALAR(2.5)), VKREND_MATERIAL_GROWTH_SPACING);
            const br_scalar y   = BR_MUL(BR_SUB(BR_SCALAR(row), BR_SCALAR(1.5)), VKREND_MATERIAL_GROWTH_SPACING);

            if((growth[i] = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_MODEL, NULL))) == NULL) {
                BrLogError("RT", "vkrend: could not allocate the material-growth actor %u.", i);
                goto fail;
            }

            growth[i]->model    = state->model;
            growth[i]->material = state->material;
            growth[i]->t.type   = BR_TRANSFORM_MATRIX34;
            BrMatrix34Scale(&growth[i]->t.t.mat, VKREND_MATERIAL_GROWTH_SCALE, VKREND_MATERIAL_GROWTH_SCALE, VKREND_MATERIAL_GROWTH_SCALE);
            BrMatrix34PostTranslate(&growth[i]->t.t.mat, x, y, BR_SCALAR(0.0));
        }

        /*
         * The aspect step left its sphere on the shared actor; detach it so this scene draws
         * exactly the growth triangles and the draw count is the one this step controls.
         */
        actor->model = NULL;

        /*
         * The aspect step left the camera on its last measurement's aspect (0.25); the scene
         * renders at the offscreen's own, like the depth resize step restores.
         */
        camera_data->aspect = BR_DIV(BR_SCALAR(state->offscreen->width), BR_SCALAR(state->offscreen->height));

        if(vkrend_renderer_fill_clear(state->offscreen) != BRE_OK)
            goto fail;

        BrZsSceneRender(state->world, camera, state->offscreen);

        if((readback = vkrend_renderer_read(state->offscreen)) == NULL)
            goto fail;

        for(i = 0; i < VKREND_MATERIAL_GROWTH_DRAWS; ++i) {
            const br_int_32 col = (br_int_32)(i % VKREND_MATERIAL_GROWTH_COLUMNS);
            const br_int_32 row = (br_int_32)(i / VKREND_MATERIAL_GROWTH_COLUMNS);
            const br_scalar x   = BR_MUL(BR_SUB(BR_SCALAR(col), BR_SCALAR(2.5)), VKREND_MATERIAL_GROWTH_SPACING);
            const br_scalar y   = BR_MUL(BR_SUB(BR_SCALAR(row), BR_SCALAR(1.5)), VKREND_MATERIAL_GROWTH_SPACING);

            /* The image's top row is view +y (see the offscreen orientation witness). */
            vkrend_renderer_project(state->offscreen, fov, distance, x, BR_NEG(y), &expected_x, &expected_y);

            if(!vkrend_renderer_pixel_is(readback, expected_x, expected_y, vkrend_renderer_surface)) {
                BrLogError("RT", "vkrend: the material-growth triangle %u at image (%d,%d) is not the surface colour; the grown buffer dropped its draw.",
                           i, expected_x, expected_y);
                BrPixelmapFree(readback);
                goto fail;
            }
        }

        BrPixelmapFree(readback);

        for(i = 0; i < VKREND_MATERIAL_GROWTH_DRAWS; ++i) {
            BrActorRemove(growth[i]);
            BrActorFree(growth[i]);
        }

        BrLogInfo("RT", "vkrend: material growth witness passed (%d one-triangle draws, more than the 16-block initial per-draw material buffer, so the scene grew it once and drained the frame's retire list; every triangle's centre is the surface %02x %02x %02x %02x).",
                  VKREND_MATERIAL_GROWTH_DRAWS, vkrend_renderer_surface[0], vkrend_renderer_surface[1], vkrend_renderer_surface[2],
                  vkrend_renderer_surface[3]);
    }

    BrRendererEnd();

    BrLogInfo(
        "RT",
        "vkrend: renderer witness passed (BrRendererBegin, a scene into a sub-pixelmap destination drawn at the sub's centre with the "
        "owner's clear colour surviving outside it, a triangle through BrZsSceneRender at %02x %02x %02x %02x, a model "
        "translation of %d texels and a camera translation of %d texels, a two-group model with %d left pixels of %02x %02x %02x "
        "%02x and %d right pixels of %02x %02x %02x %02x, a lit surface at %02x %02x %02x %02x, a 2x2 texture with quadrants %02x "
        "%02x %02x %02x / %02x %02x %02x %02x / %02x %02x %02x %02x / %02x %02x %02x %02x at %d texels and a vertically flipped map "
        "transform and a colour key, an INDEX_8 texture with palette rows 1, 2 and 3 at %02x %02x %02x %02x / %02x %02x %02x %02x / "
        "%02x %02x %02x %02x and its colour-keyed palette entry 0 at %02x %02x %02x %02x, a render-to-texture sampled from a device "
        "offscreen at %02x %02x %02x %02x / %02x %02x %02x %02x / %02x %02x %02x %02x / %02x %02x %02x %02x, clear %02x %02x %02x %02x, "
        "a one-sided "
        "back face culled and the same face two-sided drawn, a "
        "red/blue vertex-colour triangle at %02x %02x %02x %02x / %02x %02x %02x %02x / %02x %02x %02x %02x, a pre-lit surface at "
        "%02x %02x %02x %02x and a half-opaque blend at %02x %02x %02x %02x, a Z-sorted near face at %02x %02x %02x %02x, a clip "
        "plane that cuts the triangle's left half to %02x %02x %02x %02x while the right half stays %02x %02x %02x %02x, a "
        "Z-buffered near face at %02x %02x %02x %02x in both draw orders, and a smooth-lit triangle at %02x %02x %02x %02x for "
        "GOURAUD (oracle %02x %02x %02x %02x) and %02x %02x %02x %02x for PHONG (oracle %02x %02x %02x %02x), the same through "
        "the lit-textured pipeline at (0, 0, %02x, ff) and (0, 0, %02x, ff), an environment map sampling %02x %02x %02x %02x for the "
        "infinite eye and %02x %02x %02x %02x for the local eye while the plain map UV gives %02x %02x %02x %02x, and the Zb depth "
        "test leaving the near face at %02x %02x %02x %02x after an in-place resize to %dx%d, and a sphere at a square, a 4:1 "
        "and a 1:4 aspect each measuring %d x %d pixels).",
        vkrend_renderer_surface[0], vkrend_renderer_surface[1], vkrend_renderer_surface[2], vkrend_renderer_surface[3], (br_int_32)shift,
        (br_int_32)shift, left_bounds.count, vkrend_renderer_surface[0], vkrend_renderer_surface[1], vkrend_renderer_surface[2],
        vkrend_renderer_surface[3], right_bounds.count, vkrend_renderer_surface_right[0], vkrend_renderer_surface_right[1],
        vkrend_renderer_surface_right[2], vkrend_renderer_surface_right[3], lit_expected[0], lit_expected[1], lit_expected[2],
        lit_expected[3], vkrend_renderer_tex_red[0], vkrend_renderer_tex_red[1], vkrend_renderer_tex_red[2], vkrend_renderer_tex_red[3],
        vkrend_renderer_tex_green[0], vkrend_renderer_tex_green[1], vkrend_renderer_tex_green[2], vkrend_renderer_tex_green[3],
        vkrend_renderer_tex_blue[0], vkrend_renderer_tex_blue[1], vkrend_renderer_tex_blue[2], vkrend_renderer_tex_blue[3],
        vkrend_renderer_tex_black[0], vkrend_renderer_tex_black[1], vkrend_renderer_tex_black[2], vkrend_renderer_tex_black[3],
        (br_int_32)(pixels * BR_SCALAR(0.5)), vkrend_renderer_tex_red[0], vkrend_renderer_tex_red[1], vkrend_renderer_tex_red[2],
        vkrend_renderer_tex_red[3], vkrend_renderer_tex_green[0], vkrend_renderer_tex_green[1], vkrend_renderer_tex_green[2],
        vkrend_renderer_tex_green[3], vkrend_renderer_tex_blue[0], vkrend_renderer_tex_blue[1], vkrend_renderer_tex_blue[2],
        vkrend_renderer_tex_blue[3], vkrend_renderer_index_zero[0], vkrend_renderer_index_zero[1], vkrend_renderer_index_zero[2],
        vkrend_renderer_index_zero[3], vkrend_renderer_tex_red[0], vkrend_renderer_tex_red[1], vkrend_renderer_tex_red[2],
        vkrend_renderer_tex_red[3], vkrend_renderer_tex_green[0], vkrend_renderer_tex_green[1], vkrend_renderer_tex_green[2],
        vkrend_renderer_tex_green[3], vkrend_renderer_tex_blue[0], vkrend_renderer_tex_blue[1], vkrend_renderer_tex_blue[2],
        vkrend_renderer_tex_blue[3], vkrend_renderer_tex_black[0], vkrend_renderer_tex_black[1], vkrend_renderer_tex_black[2],
        vkrend_renderer_tex_black[3], vkrend_renderer_clear[0], vkrend_renderer_clear[1], vkrend_renderer_clear[2], vkrend_renderer_clear[3],
        vkrend_renderer_vc_centre[0], vkrend_renderer_vc_centre[1], vkrend_renderer_vc_centre[2], vkrend_renderer_vc_centre[3],
        vkrend_renderer_vc_blue[0], vkrend_renderer_vc_blue[1], vkrend_renderer_vc_blue[2], vkrend_renderer_vc_blue[3],
        vkrend_renderer_vc_red[0], vkrend_renderer_vc_red[1], vkrend_renderer_vc_red[2], vkrend_renderer_vc_red[3],
        vkrend_renderer_prelit[0], vkrend_renderer_prelit[1], vkrend_renderer_prelit[2], vkrend_renderer_prelit[3], vkrend_renderer_blend[0],
        vkrend_renderer_blend[1], vkrend_renderer_blend[2], vkrend_renderer_blend[3], vkrend_renderer_occlusion_near[0],
        vkrend_renderer_occlusion_near[1], vkrend_renderer_occlusion_near[2], vkrend_renderer_occlusion_near[3], vkrend_renderer_clear[0],
        vkrend_renderer_clear[1], vkrend_renderer_clear[2], vkrend_renderer_clear[3], vkrend_renderer_surface[0], vkrend_renderer_surface[1],
        vkrend_renderer_surface[2], vkrend_renderer_surface[3], vkrend_renderer_occlusion_near[0], vkrend_renderer_occlusion_near[1],
        vkrend_renderer_occlusion_near[2], vkrend_renderer_occlusion_near[3], gouraud_pixel[0], gouraud_pixel[1], gouraud_pixel[2],
        gouraud_pixel[3], gouraud_expected[0], gouraud_expected[1], gouraud_expected[2], gouraud_expected[3], phong_pixel[0],
        phong_pixel[1], phong_pixel[2], phong_pixel[3], phong_expected[0], phong_expected[1], phong_expected[2], phong_expected[3],
        gouraud_expected[0], phong_expected[0], env_infinite_expected[0], env_infinite_expected[1], env_infinite_expected[2],
        env_infinite_expected[3], env_local_expected[0], env_local_expected[1], env_local_expected[2], env_local_expected[3], 0x80, 0x00,
        0x00, 0xff, vkrend_renderer_occlusion_near[0], vkrend_renderer_occlusion_near[1], vkrend_renderer_occlusion_near[2],
        vkrend_renderer_occlusion_near[3], state->offscreen->width, state->offscreen->height, VKREND_ASPECT_DIAMETER, VKREND_ASPECT_DIAMETER);

    vkrend_renderer_fini(state);
    return BRE_OK;

fail:
    BrRendererEnd();
    vkrend_renderer_fini(state);
    return BRE_FAIL;
}

br_error rt_vk_witness_run(br_demo *demo)
{
    /*
     * The witness steps report through their init(), which frees what it built
     * on both paths, so each is called once and only its result is inspected.
     * The offscreen case runs first: it resizes the front screen on the way, and
     * the renderer case then matches its offscreen from that resized screen, as
     * the two cases did as devpmtest tests.
     */
    vkrend_offscreen_state offscreen = {0};
    vkrend_renderer_state  renderer  = {0};
    br_error               r         = BRE_OK;

    if(vkrend_offscreen_init(&offscreen, demo->_screen, NULL, NULL) != BRE_OK)
        r = BRE_FAIL;

    if(vkrend_renderer_init(&renderer, demo->_screen, NULL, NULL) != BRE_OK)
        r = BRE_FAIL;

    if(r == BRE_OK)
        BrLogInfo("RT", "vkrend witness passed (offscreen pixelmap path and renderer scene path).");

    return r;
}
