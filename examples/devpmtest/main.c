/*
 * BRender Device Pixelmap Testing Tool
 */
#include <SDL.h>
#include <brender.h>
#include <brddi.h> /* To poke the device's CLUT. */
#include <brglrend.h>
#include <brsdl2dev.h>

/*
 * HSL -> RGB conversion code modified from https://gist.github.com/ciembor/1494530
 */

/*
 * Converts an HUE to r, g or b.
 * returns float in the set [0, 1].
 */
float hue2rgb(float p, float q, float t)
{

    if(t < 0.0f)
        t += 1.0f;
    if(t > 1.0f)
        t -= 1.0f;
    if(t < 1.0f / 6.0f)
        return p + (q - p) * 6.0f * t;
    if(t < 1.0f / 2.0f)
        return q;
    if(t < 2.0f / 3.0f)
        return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;

    return p;
}

/*
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns RGB in the set [0, 255].
 */
br_colour hsl2rgb(float h, float s, float l)
{
    float p, q;

    if(0 == s) {
        /* achromatic */
        return BR_COLOUR_RGBA((br_colour)(l * 255.0f), (br_colour)(l * 255.0f), (br_colour)(l * 255.0f), 255);
    }

    q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    p = 2 * l - q;
    return BR_COLOUR_RGBA((br_colour)(hue2rgb(p, q, h + 1.0f / 3.0f) * 255.0f), (br_colour)(hue2rgb(p, q, h) * 255.0f),
                          (br_colour)(hue2rgb(p, q, h - 1.0f / 3.0f) * 255.0f), 255);
}

/*
 * The line & point test.
 * Tests:
 *  - Rectangle fill
 *  - Memory->device non-stretch copy
 *  - Line drawing
 */
static void draw_linepoint_test(br_pixelmap *dest, br_int_32 base_x, br_int_32 base_y, float dt, float *h, float *s, float *l)
{
    br_colour       col;
    const br_int_32 width = 512;
    const int       nrows = 64;
    const int       ncols = 64;

    *h  = fmodf(*h + dt * 0.1f, 1.0f);
    col = hsl2rgb(*h, *s, *l);

    BrPixelmapRectangleFill(dest, base_x, base_y, width, width, col);

    /*
     * Draw a grid over the rainbow rect using the opposite hue.
     */
    col = hsl2rgb(1.0f - *h, *s, *l);

    for(int i = 0; i < nrows + 1; ++i) {
        int x1 = base_x;
        int x2 = x1 + width;
        int y1 = base_y + (i * 8);
        int y2 = base_y + (i * 8);
        BrPixelmapLine(dest, x1, y1, x2, y2, col);
    }

    for(int i = 0; i < ncols + 1; ++i) {
        int x1 = base_x + (i * 8);
        int x2 = base_x + (i * 8);
        int y1 = base_y;
        int y2 = y1 + width;
        BrPixelmapLine(dest, x1, y1, x2, y2, col);
    }

    /*
     * Put a single dot inside each box.
     */
    for(int i = 0; i < nrows; ++i) {
        for(int j = 0; j < ncols; ++j) {
            BrPixelmapPixelSet(dest, base_x + (i * 8) + 4, base_y + (j * 8) + 4, col);
        }
    }
}

typedef struct linepoint_state {
    float h, s, l;
} linepoint_state;

static br_error linepoint_init(void *user, br_pixelmap *screen, br_pixelmap *backbuffer, void *arg)
{
    linepoint_state *state = user;

    *state = (linepoint_state){
        .h = 0.0f,
        .s = 1.0f,
        .l = 0.5f,
    };
    return BRE_OK;
}

static void linepoint_draw(br_pixelmap *dest, float dt, void *user)
{
    linepoint_state *state = user;
    br_colour        col;
    const br_int_32  width  = 512;
    const int        nrows  = 64;
    const int        ncols  = 64;
    int              base_x = -width / 2;
    int              base_y = -width / 2;

    state->h = fmodf(state->h + dt * 0.1f, 1.0f);
    col      = hsl2rgb(state->h, state->s, state->l);

    BrPixelmapRectangleFill(dest, base_x, base_y, width, width, col);

    /*
     * Draw a grid over the rainbow rect using the opposite hue.
     */
    col = hsl2rgb(1.0f - state->h, state->s, state->l);

    for(int i = 0; i < nrows + 1; ++i) {
        int x1 = base_x;
        int x2 = x1 + width;
        int y1 = base_y + (i * 8);
        int y2 = base_y + (i * 8);
        BrPixelmapLine(dest, x1, y1, x2, y2, col);
    }

    for(int i = 0; i < ncols + 1; ++i) {
        int x1 = base_x + (i * 8);
        int x2 = base_x + (i * 8);
        int y1 = base_y;
        int y2 = y1 + width;
        BrPixelmapLine(dest, x1, y1, x2, y2, col);
    }

    /*
     * Put a single dot inside each box.
     */
    for(int i = 0; i < nrows; ++i) {
        for(int j = 0; j < ncols; ++j) {
            BrPixelmapPixelSet(dest, base_x + (i * 8) + 4, base_y + (j * 8) + 4, col);
        }
    }

    // draw_linepoint_test(dest, -dest->origin_x + 20, -200, dt, &state->h, &state->s, &state->l);
}

typedef struct br_smpte_state {
    br_pixelmap *pm;
} br_smpte_state;

static void smpte_fini(void *user)
{
    br_smpte_state *state = user;
    BrPixelmapFree(state->pm);
}

static br_error smpte_init(void *user, br_pixelmap *screen, br_pixelmap *backbuffer, void *arg)
{
    br_smpte_state *state = user;

    if((state->pm = BrPixelmapLoad(arg)) == NULL) {
        BrLogError("APP", "Error loading %s", arg);
        return BRE_FAIL;
    }

    if(ObjectDevice(backbuffer) == NULL && state->pm->type != backbuffer->type) {
        BrLogError("APP", "Disabling SMPTE type %d due to memory device format %d", state->pm->type, backbuffer->type);
        smpte_fini(state);
        return BRE_FAIL;
    }

    return BRE_OK;
}

static void smpte_draw(br_pixelmap *dest, float dt, void *user)
{
    br_smpte_state *state  = user;
    br_pixelmap    *pm     = state->pm;
    int             base_x = -pm->width / 2;
    int             base_y = -pm->height / 2;

    BrPixelmapRectangleCopy(dest, base_x, base_y, pm, -pm->origin_x, -pm->origin_y, pm->width, pm->height);
}

typedef struct br_index8nopal_state {
    br_pixelmap *earth;
    br_pixelmap *pal_std;
    br_pixelmap *pal_grey;
    br_pixelmap *target_nopal;
    br_pixelmap *target_pal;
} br_earth8_state;

static void earth8_fini(void *user)
{
    br_earth8_state *state = user;

    if(state->target_pal != NULL)
        BrPixelmapFree(state->target_pal);

    if(state->target_nopal != NULL)
        BrPixelmapFree(state->target_nopal);

    if(state->pal_grey != NULL)
        BrPixelmapFree(state->pal_grey);

    if(state->pal_std != NULL)
        BrPixelmapFree(state->pal_std);

    if(state->earth != NULL)
        BrPixelmapFree(state->earth);
}

static br_error earth8_init(void *user, br_pixelmap *screen, br_pixelmap *backbuffer, void *arg)
{
    br_earth8_state *state = user;

    if((state->earth = BrPixelmapLoad("earth8.pix")) == NULL) {
        BrLogError("APP", "Error loading earth8.pix");
        earth8_fini(user);
        return BRE_FAIL;
    }

    if((state->pal_std = BrPixelmapLoad("std.pal")) == NULL) {
        BrLogError("APP", "Error loading std.pal");
        earth8_fini(user);
        return BRE_FAIL;
    }

    if((state->pal_grey = BrPixelmapAllocate(BR_PMT_RGBX_888, state->pal_std->width, state->pal_std->height, NULL,
                                             BR_PMAF_NORMAL)) == NULL) {
        BrLogError("APP", "Error allocating greyscale palette");
        earth8_fini(user);
        return BRE_FAIL;
    }

    for(br_int_32 j = 0; j < state->pal_grey->height; ++j) {
        for(br_int_32 i = 0; i < state->pal_grey->width; ++i) {
            br_colour col = BrPixelmapPixelGet(state->pal_std, i, j);
            br_uint_8 val = (br_uint_8)(0.299f * BR_RED(col) + 0.587f * BR_GRN(col) + 0.114f * BR_BLU(col));
            BrPixelmapPixelSet(state->pal_grey, i, j, BR_COLOUR_RGBA(val, val, val, 255));
        }
    }

    if((state->target_nopal = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBA_8888,
                                                        state->earth->width, state->earth->height)) == NULL) {
        BrLogError("APP", "Error creating BR_PMT_RGBA_8888 device pixelmap.");
        return BRE_FAIL;
    }

    state->target_nopal->origin_x = (br_int_16)(state->target_nopal->width / 2);
    state->target_nopal->origin_y = (br_int_16)(state->target_nopal->height / 2);

    if((state->target_pal = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_INDEX_8, state->earth->width,
                                                      state->earth->height)) == NULL) {
        BrLogError("APP", "Error creating BR_PMT_INDEX_8 device pixelmap.");
        earth8_fini(user);
        return BRE_FAIL;
    }

    state->target_pal->origin_x = (br_int_16)(state->target_pal->width / 2);
    state->target_pal->origin_y = (br_int_16)(state->target_pal->height / 2);

    return BRE_OK;
}

static void earth8_draw(br_pixelmap *dest, float dt, void *user)
{
    br_earth8_state *state       = user;
    br_uint_16       text_height = BrPixelmapTextHeight(dest, BrFontProp7x9);
    int              base_x      = -(256 + text_height * 3);
    int              base_y      = -(256 + text_height * 3);
    int              white_len;

    state->earth->map = NULL;

    /*
     * Test 1 - Blit from indexed texture w/o palette to non-indexed destination.
     * Expected failure.
     */
    BrPixelmapText(dest, base_x, base_y - (text_height * 2), BR_COLOUR_RGBA(255, 255, 255, 255), BrFontProp7x9,
                   "Indexed w/o palette -> non-indexed");
    BrPixelmapFill(state->target_nopal, BR_COLOUR_RGBA(255, 255, 255, 255));
    BrPixelmapCopy(state->target_nopal, state->earth);

    white_len = BrPixelmapTextWidth(state->target_nopal, BrFontProp7x9, "This square should be white.");
    BrPixelmapText(state->target_nopal, -white_len / 2, -text_height / 2, BR_COLOUR_RGBA(0, 0, 0, 255), BrFontProp7x9,
                   "This square should be white.");
    BrPixelmapRectangleCopy(dest, base_x, base_y, state->target_nopal, -state->target_nopal->origin_x,
                            -state->target_nopal->origin_y, state->target_nopal->width, state->target_nopal->height);

    base_y += 256 + text_height * 3;

    /*
     * Test 2 - Blit from indexed texture w/o palette to indexed destination.
     */
    BrPixelmapText(dest, base_x, base_y - (text_height * 2), BR_COLOUR_RGBA(255, 255, 255, 255), BrFontProp7x9,
                   "Indexed w/o palette -> indexed w/ palette");
    BrPixelmapPaletteSet(state->target_pal, state->pal_std);
    BrPixelmapFill(state->target_pal, BR_COLOUR_RGBA(0, 0, 0, 255));
    BrPixelmapCopy(state->target_pal, state->earth);
    BrPixelmapRectangleCopy(dest, base_x, base_y, state->target_pal, -state->target_pal->origin_x,
                            -state->target_pal->origin_y, state->target_pal->width, state->target_pal->height);

    base_x += 256 + text_height * 3;
    base_y -= 256 + text_height * 3;

    /*
     * Test 3 - Blit from indexed texture w/ palette to non-indexed destination.
     */
    state->earth->map = state->pal_std;
    BrPixelmapText(dest, base_x, base_y - (text_height * 2), BR_COLOUR_RGBA(255, 255, 255, 255), BrFontProp7x9,
                   "Indexed w/ palette -> non-indexed");
    BrPixelmapFill(state->target_nopal, BR_COLOUR_RGBA(0, 0, 0, 255));
    BrPixelmapCopy(state->target_nopal, state->earth);
    BrPixelmapRectangleCopy(dest, base_x, base_y, state->target_nopal, -state->target_nopal->origin_x,
                            -state->target_nopal->origin_y, state->target_nopal->width, state->target_nopal->height);

    base_y += 256 + text_height * 3;

    /*
     * Test 4 - Blit from indexed texture w/ palette to indexed destination.
     * Should use the destination surface's CLUT (a greyscale one, in this case).
     */
    state->earth->map = state->pal_std;
    BrPixelmapText(dest, base_x, base_y - (text_height * 2), BR_COLOUR_RGBA(255, 255, 255, 255), BrFontProp7x9,
                   "Indexed w/ palette -> index w/ palette (grayscale)");
    BrPixelmapPaletteSet(state->target_pal, state->pal_grey);
    BrPixelmapFill(state->target_pal, BR_COLOUR_RGBA(0, 0, 0, 255));
    BrPixelmapCopy(state->target_pal, state->earth);
    BrPixelmapRectangleCopy(dest, base_x, base_y, state->target_pal, -state->target_pal->origin_x,
                            -state->target_pal->origin_y, state->target_pal->width, state->target_pal->height);
}

typedef struct br_device2mem_state {
    br_pixelmap *last_frame_memory;
    br_pixelmap *last_frame_device;
    br_pixelmap *last_frame_hxw;
    br_pixelmap *checkerboard;
} br_device2mem_state;

static void device2mem_fini(void *user)
{
    br_device2mem_state *state = user;

    if(state->checkerboard != NULL)
        BrPixelmapFree(state->checkerboard);

    if(state->last_frame_hxw != NULL)
        BrPixelmapFree(state->last_frame_hxw);

    if(state->last_frame_device != NULL)
        BrPixelmapFree(state->last_frame_device);

    if(state->last_frame_memory != NULL)
        BrPixelmapFree(state->last_frame_memory);
}

static br_error device2mem_init(void *user, br_pixelmap *screen, br_pixelmap *backbuffer, void *arg)
{
    br_device2mem_state *state = user;

    state->last_frame_memory = BrPixelmapAllocate(backbuffer->type, backbuffer->width, backbuffer->height, NULL, BR_PMAF_NORMAL);
    state->last_frame_device = BrPixelmapMatch(backbuffer, BR_PMMATCH_OFFSCREEN);
    state->last_frame_hxw = BrPixelmapAllocate(BR_PMT_RGBX_888, backbuffer->height, backbuffer->width, NULL, BR_PMAF_NORMAL);

    if((state->checkerboard = BrPixelmapLoad("checkerboard.pix")) == NULL) {
        device2mem_fini(user);
        return BRE_FAIL;
    }

    return BRE_OK;
}

static void device2mem_draw(br_pixelmap *dest, float dt, void *user)
{
    br_device2mem_state *state       = user;
    int                  width       = dest->width / 3;
    int                  height      = dest->height / 3;
    int                  base_x      = -width;
    int                  base_y      = -height;
    int                  text_height = BrPixelmapTextHeight(dest, BrFontProp7x9);

    for(int i = 0; i < 5; ++i) {
        BrPixelmapLine(dest, -dest->origin_x, -dest->origin_y + i, dest->origin_x, -dest->origin_y + i, 0xFFFFFFFF);
        BrPixelmapLine(dest, -dest->origin_x, dest->origin_y - i, dest->origin_x, dest->origin_y - i, 0xFFFFFFFF);

        BrPixelmapLine(dest, -dest->origin_x + i, -dest->origin_y, -dest->origin_x + i, dest->origin_y, 0xFFFFFFFF);
        BrPixelmapLine(dest, dest->origin_x - i, -dest->origin_y, dest->origin_x - i, dest->origin_y, 0xFFFFFFFF);
    }

    /*
     * Draw the previous frame (black initially) in the top-right corner.
     * Tests memory->device stretch copy.
     */
    BrPixelmapRectangleStretchCopy(dest, base_x, base_y, width, height, state->last_frame_memory, 0, 0,
                                   state->last_frame_memory->width, state->last_frame_memory->height);
    BrPixelmapText(dest, base_x, base_y - (text_height * 2), BR_COLOUR_RGBA(255, 255, 255, 255), BrFontProp7x9,
                   "Memory->device stretch copy");

    BrPixelmapRectangleStretchCopy(dest, base_x + (width / 2) - (128 / 2), 128 / 2, 128, 128, state->checkerboard,
                                   -state->checkerboard->origin_x, -state->checkerboard->origin_y,
                                   state->checkerboard->width, state->checkerboard->height);

    base_x += width + text_height;
    /*
     * Draw the previous frame (black initially) underneath it.
     * Tests device->device stretch copy.
     */
    BrPixelmapRectangleStretchCopy(dest, base_x, base_y, width, height, state->last_frame_device,
                                   -state->last_frame_device->origin_x, -state->last_frame_device->origin_y,
                                   state->last_frame_device->width, state->last_frame_device->height);
    BrPixelmapText(dest, base_x, base_y - (text_height * 2), BR_COLOUR_RGBA(255, 255, 255, 255), BrFontProp7x9,
                   "Device->device stretch copy");

    BrPixelmapRectangleStretchCopy(dest, base_x + (width / 2) - (128 / 2), 128 / 2, 128, 128, state->checkerboard,
                                   -state->checkerboard->origin_x, -state->checkerboard->origin_y,
                                   state->checkerboard->width, state->checkerboard->height);
    /*
     * Tests a device->memory non-stretch copy, with device memory non-addressable.
     * Hack to invoke rectangleStretchCopyFrom().
     */
    {
        int ww = state->last_frame_hxw->width / 6;
        int hh = state->last_frame_hxw->height / 6;

        state->last_frame_device->flags |= BR_PMF_NO_ACCESS;
        BrPixelmapRectangleStretchCopy(state->last_frame_hxw, -state->last_frame_hxw->origin_x,
                                       -state->last_frame_hxw->origin_y, state->last_frame_hxw->width,
                                       state->last_frame_hxw->height, state->last_frame_device,
                                       -state->last_frame_device->origin_x, -state->last_frame_device->origin_y,
                                       state->last_frame_device->width, state->last_frame_device->height);
        state->last_frame_device->flags &= ~BR_PMF_NO_ACCESS;

        BrPixelmapRectangleStretchCopy(dest, -ww + (ww / 2), 128 / 2, state->last_frame_hxw->width / 6,
                                       state->last_frame_hxw->height / 6, state->last_frame_hxw,
                                       -state->last_frame_hxw->origin_x, -state->last_frame_hxw->origin_y,
                                       state->last_frame_hxw->width, state->last_frame_hxw->height);
    }
}

static void device2mem_swap(br_pixelmap *screen, br_pixelmap *backbuffer, void *user)
{
    br_device2mem_state *state = user;

    /* Capture the last frame, using a device->memory non-stretch copy. */
    BrPixelmapCopy(state->last_frame_memory, backbuffer);

    /* Capture the last frame, using a device->device non-stretch copy. */
    BrPixelmapCopy(state->last_frame_device, backbuffer);
}

typedef struct br_submap_state {
    br_pixelmap    *checkerboard_memory;
    br_pixelmap    *checkerboard_device;
    br_pixelmap    *middle_square;
    br_pixelmap    *left_square;
    br_pixelmap    *left_square2;
    linepoint_state linepoint;
} br_submap_state;

void submap_fini(void *user)
{
    br_submap_state *state = user;

    if(state->left_square2 != NULL)
        BrPixelmapFree(state->left_square2);

    if(state->left_square != NULL)
        BrPixelmapFree(state->left_square);

    if(state->middle_square != NULL)
        BrPixelmapFree(state->middle_square);

    if(state->checkerboard_device != NULL)
        BrPixelmapFree(state->checkerboard_device);

    if(state->checkerboard_memory != NULL)
        BrPixelmapFree(state->checkerboard_memory);
}

static br_error submap_init(void *user, br_pixelmap *screen, br_pixelmap *backbuffer, void *arg)
{
    br_submap_state *state = user;

    if((state->checkerboard_memory = BrPixelmapLoad("checkerboard.pix")) == NULL)
        return BRE_FAIL;

    if((state->checkerboard_device = BrPixelmapMatchSized(backbuffer, BR_PMMATCH_OFFSCREEN, state->checkerboard_memory->width,
                                                          state->checkerboard_memory->height)) == NULL) {
        submap_fini(user);
        return BRE_FAIL;
    }

    /*
     * Memory pixelmaps don't support copying different types.
     */
    if(ObjectDevice(state->checkerboard_device) == NULL) {
        BrLogWarn("APP", "Disable Sub-pixelmap test due to memory device.");
        submap_fini(user);
        return BRE_FAIL;
    }

    BrPixelmapCopy(state->checkerboard_device, state->checkerboard_memory);

    if((state->middle_square = BrPixelmapAllocateSub(backbuffer, -300, -300, 600, 600)) == NULL) {
        submap_fini(user);
        return BRE_FAIL;
    }

    state->middle_square->origin_x = (br_int_16)(state->middle_square->width / 2);
    state->middle_square->origin_y = (br_int_16)(state->middle_square->height / 2);

    if((state->left_square = BrPixelmapAllocateSub(backbuffer, -backbuffer->origin_x + 20, 0, state->checkerboard_memory->width,
                                                   state->checkerboard_memory->height)) == NULL) {
        submap_fini(user);
        return BRE_FAIL;
    }

    state->left_square->origin_x = (br_int_16)(state->left_square->width / 2);
    state->left_square->origin_y = (br_int_16)(state->left_square->height / 2);

    if((state->left_square2 = BrPixelmapAllocateSub(
            backbuffer, -backbuffer->origin_x + 20 + state->checkerboard_memory->width + 20, 0,
            state->checkerboard_memory->width, state->checkerboard_memory->height)) == NULL) {
        submap_fini(user);
        return BRE_FAIL;
    }

    (void)linepoint_init(&state->linepoint, NULL, state->middle_square, arg);

    return BRE_OK;
}

void submap_draw(br_pixelmap *dest, float dt, void *user)
{
    br_submap_state *state = user;

    BrPixelmapFill(state->middle_square, 0xFFFFFFFF);
    linepoint_draw(state->middle_square, dt, &state->linepoint);

    BrPixelmapCopy(state->left_square, state->checkerboard_memory);
    BrPixelmapCopy(state->left_square2, state->checkerboard_device);
    // BrPixelmapFill(state->left_square2, 0xFFFFFFFF);
}

typedef struct pixelquery_state {
    br_pixelmap *tex8;
    br_pixelmap *tex16;
    br_pixelmap *tex24;
    br_pixelmap *tex32;
} pixelquery_state;

static void pixelquery_fini(void *user)
{
    pixelquery_state *state = user;

    if(state->tex32 != NULL) {
        BrPixelmapFree(state->tex32);
    }
    state->tex32 = NULL;

    if(state->tex24 != NULL) {
        BrPixelmapFree(state->tex24);
    }
    state->tex24 = NULL;

    if(state->tex16 != NULL) {
        BrPixelmapFree(state->tex16);
    }
    state->tex16 = NULL;

    if(state->tex8 != NULL) {
        BrPixelmapFree(state->tex8);
    }
    state->tex8 = NULL;
}

static br_pixelmap *pm2dev(br_pixelmap *dst, br_pixelmap *pm)
{
    br_pixelmap *new_pm;

    if((new_pm = BrPixelmapMatchTypedSized(dst, BR_PMMATCH_OFFSCREEN, pm->type, pm->width, pm->height)) == NULL)
        return NULL;

    if(pm->map != NULL)
        BrPixelmapPaletteSet(new_pm, pm->map);

    BrPixelmapCopy(new_pm, pm);
    return new_pm;
}

static br_pixelmap *load2dev(br_pixelmap *dst, const char *name)
{
    br_pixelmap *pm;
    br_pixelmap *devpm;

    if((pm = BrPixelmapLoad(name)) == NULL) {
        BrLogError("APP", "Unable to load %s", name);
        return NULL;
    }

    devpm = pm2dev(dst, pm);
    BrPixelmapFree(pm);

    if(devpm == NULL) {
        BrLogError("APP", "Unable to convert memory pixelmap to device pixelmap");
        return NULL;
    }

    return devpm;
}

static br_error pixelquery_init(void *user, br_pixelmap *screen, br_pixelmap *backbuffer, void *arg)
{
    pixelquery_state *state = user;

    if((state->tex8 = load2dev(backbuffer, "smpte_type03_index_8_small.pix")) == NULL) {
        pixelquery_fini(state);
        return BRE_FAIL;
    }

    if((state->tex16 = load2dev(backbuffer, "smpte_type05_rgb_565_small.pix")) == NULL) {
        pixelquery_fini(state);
        return BRE_FAIL;
    }

    if((state->tex24 = load2dev(backbuffer, "smpte_type06_rgb_888_small.pix")) == NULL) {
        pixelquery_fini(state);
        return BRE_FAIL;
    }

    if((state->tex32 = load2dev(backbuffer, "smpte_type07_rgbx_888_small.pix")) == NULL) {
        pixelquery_fini(state);
        return BRE_FAIL;
    }
    return BRE_OK;
}

static void draw_pixelbypixel(br_pixelmap *dest, br_pixelmap *pm, br_int_32 base_x, br_int_32 base_y)
{
    br_colour       col;
    br_device_clut *clut;

    base_x -= pm->width / 2;
    base_y -= pm->height / 2;

    clut = NULL;
    ObjectQuery(pm, &clut, BRT_CLUT_O);

    for(br_int_32 y = 0; y < pm->height; ++y) {
        for(br_int_32 x = 0; x < pm->width; ++x) {
            br_uint_8 r, g, b;

            col = BrPixelmapPixelGet(pm, -pm->origin_x + x, -pm->origin_y + y);

            if(clut != NULL)
                DeviceClutEntryQuery(clut, &col, col);

            if(pm->type == BR_PMT_RGB_565) {
                r = BR_RED_565(col) << 3;
                g = BR_GRN_565(col) << 2;
                b = BR_BLU_565(col) << 3;
            } else {
                r = BR_RED(col);
                g = BR_GRN(col);
                b = BR_BLU(col);
            }

            BrPixelmapPixelSet(dest, base_x + x, base_y + y, BR_COLOUR_RGBA(r, g, b, 0xFF));
        }
    }
}

static void pixelquery_draw(br_pixelmap *dest, float dt, void *user)
{
    pixelquery_state *state = user;

    draw_pixelbypixel(dest, state->tex8, -84, -64);
    draw_pixelbypixel(dest, state->tex16, 84, -64);
    draw_pixelbypixel(dest, state->tex24, -84, 64);
    draw_pixelbypixel(dest, state->tex32, 84, 64);
}

typedef br_error(br_drawtest_init_cbfn)(void *user, br_pixelmap *screen, br_pixelmap *backbuffer, void *arg);
typedef void(br_drawtest_draw_cbfn)(br_pixelmap *dest, float dt, void *user);
typedef void(br_drawtest_swap_cbfn)(br_pixelmap *screen, br_pixelmap *backbuffer, void *user);
typedef void(br_drawtest_fini_cbfn)(void *user);

typedef struct br_drawtest {
    const char            *name;
    br_drawtest_init_cbfn *init;
    br_drawtest_draw_cbfn *draw;
    br_drawtest_swap_cbfn *post;
    br_drawtest_fini_cbfn *fini;
    size_t                 user_size;
    void                  *arg;

    void      *_user;
    br_boolean _failed;
} br_drawtest;

#define MAKE_SMPTE_TEST(filename, type, suffix)                                                                \
    {                                                                                                          \
        .name = "SMPTE (" BR_STR(type) suffix ")", .init = smpte_init, .draw = smpte_draw, .fini = smpte_fini, \
        .arg = filename, .user_size = sizeof(br_smpte_state),                                                  \
    }

// clang-format off
static br_drawtest tests[] = {
    {
        .name      = "Line & Point Test",
        .init      = linepoint_init,
        .draw      = linepoint_draw,
        .user_size = sizeof(linepoint_state),
        .arg       = NULL
    },
    MAKE_SMPTE_TEST("smpte_type03_index_8.pix",   BR_PMT_INDEX_8,   ", embedded palette"),
    MAKE_SMPTE_TEST("smpte_type04_rgb_555.pix",   BR_PMT_RGB_555,   ""),
    MAKE_SMPTE_TEST("smpte_type05_rgb_565.pix",   BR_PMT_RGB_565,   ""),
    MAKE_SMPTE_TEST("smpte_type06_rgb_888.pix",   BR_PMT_RGB_888,   ""),
    MAKE_SMPTE_TEST("smpte_type07_rgbx_888.pix",  BR_PMT_RGBX_888,  ""),
    MAKE_SMPTE_TEST("smpte_type08_rgba_8888.pix", BR_PMT_RGBA_8888, ""),
    MAKE_SMPTE_TEST("smpte_type17_bgr_555.pix",   BR_PMT_BGR_555,   ""),
    MAKE_SMPTE_TEST("smpte_type32_argb_4444.pix", BR_PMT_ARGB_4444, ""),
    {
        .name      = "Earth 8-bit",
        .init      = earth8_init,
        .draw      = earth8_draw,
        .fini      = earth8_fini,
        .user_size = sizeof(br_earth8_state),
        .arg       = NULL,
    },
    {
        .name      = "Device <-> Memory Copies",
        .init      = device2mem_init,
        .draw      = device2mem_draw,
        .post      = device2mem_swap,
        .fini      = device2mem_fini,
        .user_size = sizeof(br_device2mem_state),
        .arg       = NULL,
    },
    {
        .name      = "Sub-pixelmap Test",
        .init      = submap_init,
        .draw      = submap_draw,
        .fini      = submap_fini,
        .user_size = sizeof(br_submap_state),
        .arg       = NULL,
    },
    {
        .name      = "pixelQuery() Test",
        .init      = pixelquery_init,
        .draw      = pixelquery_draw,
        .fini      = pixelquery_fini,
        .user_size = sizeof(pixelquery_state),
        .arg       = NULL,
    },
};
// clang-format on

void BR_CALLBACK _BrBeginHook(void)
{
    BrDevAddStatic(NULL, BrDrv1SDL2Begin, NULL);
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);
}

void BR_CALLBACK _BrEndHook(void)
{
}

static void init_tests(void *anchor, br_pixelmap *screen, br_pixelmap *colour_buffer)
{
    for(size_t i = 0; i < BR_ASIZE(tests); ++i) {
        br_drawtest *test = tests + i;
        br_error     r;

        test->_user = BrResAllocate(anchor, test->user_size, BR_MEMORY_APPLICATION);

        r = BRE_OK;
        if(test->init != NULL)
            r = test->init(test->_user, screen, colour_buffer, test->arg);

        if(r != BRE_OK) {
            test->_failed = BR_TRUE;
            BrLogError("APP", "Error initialising test %s", test->name);
        }
    }
}

static void cleanup_tests(void)
{
    for(size_t i = 0; i < BR_ASIZE(tests); ++i) {
        br_drawtest *test = tests + i;
        if(test->fini != NULL && !test->_failed)
            test->fini(test->_user);

        if(test->_user != NULL)
            memset(test->_user, 0, test->user_size);

        test->_failed = BR_FALSE;
    }
}

int main(int argc, char **argv)
{
    br_pixelmap *screen = NULL, *colour_buffer = NULL;
    int          width = 1280, height = 720;
    int          ret = -1;
    br_error     r;
    br_uint_64   ticks_last, ticks_now;
    int          want_screenshot;
    int          test_index = 0;/* BR_ASIZE(tests) - 1; */

    BrBegin();

    BrLogSetLevel(BR_LOG_DEBUG);

    void *anchor = BrResAllocate(NULL, 0, BR_MEMORY_ANCHOR);

    // clang-format off
    r = BrDevBeginVar(&screen, "SDL2",
                      BRT_WIDTH_I32,      (br_int_32)width,
                      BRT_HEIGHT_I32,     (br_int_32)height,
                      BRT_HIDPI_B,        BR_TRUE,
                      /* Set these if you dare... */
                      BRT_RESIZABLE_B,    BR_TRUE,
                      BRT_OPENGL_B,       BR_FALSE,
                      BR_NULL_TOKEN);
    // clang-format on

    if(r != BRE_OK) {
        BrLogError("APP", "BrDevBeginVar() failed.");
        goto screen_creation_failed;
    }

    width  = screen->width;
    height = screen->height;

    if((colour_buffer = BrPixelmapMatch(screen, BR_PMMATCH_OFFSCREEN)) == NULL) {
        BrLogError("APP", "BrPixelmapMatchTypedSized() failed.");
        goto screen_creation_failed;
    }

    screen->origin_x = colour_buffer->origin_x = (br_int_16)(width / 2);
    screen->origin_y = colour_buffer->origin_y = (br_int_16)(height / 2);

    ticks_last = SDL_GetTicks64();

    init_tests(anchor, screen, colour_buffer);

    for(SDL_Event evt;;) {
        float dt;

        ticks_now  = SDL_GetTicks64();
        dt         = (float)(ticks_now - ticks_last) / 1000.0f;
        ticks_last = ticks_now;

        want_screenshot = 0;
        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_QUIT:
                    goto done;
                case SDL_WINDOWEVENT:
                    if(BrPixelmapHandleWindowEvent(screen, &evt.window) != BRE_OK) {
                        BrLogError("APP", "Error handling window event");
                        goto done;
                    }

                    switch(evt.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED: {
                            br_pixelmap *ncol;
                            screen->origin_x = (br_int_16)(screen->width / 2);
                            screen->origin_y = (br_int_16)(screen->height / 2);

                            ncol          = BrPixelmapResize(colour_buffer, screen->width, screen->height);
                            colour_buffer = ncol;

                            cleanup_tests();
                            init_tests(anchor, screen, colour_buffer);

                            break;
                        }
                    }
                    break;
                case SDL_KEYDOWN: {
                    switch(evt.key.keysym.sym) {
                        case SDLK_F5:
                            want_screenshot = 1;
                            break;
                        case SDLK_SPACE:
                            test_index = (test_index + 1) % BR_ASIZE(tests);
                            break;
                        case 'q':
                            SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});
                            break;
                    }
                }
            }
        }

        /* Clear the screen black. */
        BrPixelmapFill(colour_buffer, BR_COLOUR_RGBA(0, 0, 0, 0xFF));

        /* Run the test */
        {
            int base_x      = -colour_buffer->origin_x + colour_buffer->width / 2;
            int base_y      = -colour_buffer->origin_y + 20;
            int text_height = BrPixelmapTextHeight(colour_buffer, BrFontProp7x9);

            int text_width = BrPixelmapTextWidth(colour_buffer, BrFontProp7x9, tests[test_index].name);

            if(tests[test_index]._failed) {
                BrPixelmapText(colour_buffer, base_x - text_width / 2, base_y + text_height, 0xFFFFFFFF, BrFontProp7x9,
                               "INIT FAILED");
            } else {
                tests[test_index].draw(colour_buffer, dt, tests[test_index]._user);
            }
            BrPixelmapText(colour_buffer, base_x - text_width / 2, base_y, 0xFFFFFFFF, BrFontProp7x9, tests[test_index].name);
        }


        /* Add in some text. */
        {
            br_int_32 base_x      = -colour_buffer->origin_x + 5;
            br_int_32 base_y      = -colour_buffer->origin_y + 5;
            br_colour col         = BR_COLOUR_RGBA(255, 255, 255, 255);
            br_int_32 text_height = BrPixelmapTextHeight(colour_buffer, BrFontProp7x9);

            static const char *usage_lines[] = {
                "Usage:",
                "  Q      = Quit",
                "  Space = Next Test",
                "  F5     = Screenshot (to devpmtest.pix)",
            };

            BrPixelmapTextF(colour_buffer, base_x, base_y, col, BrFontProp7x9, "last frame delta (msec): %f", dt * 1000);

            base_y += text_height * 2;

            for(size_t i = 0; i < BR_ASIZE(usage_lines); ++i) {
                base_y += text_height * 2;
                BrPixelmapText(colour_buffer, base_x, base_y, col, BrFontProp7x9, usage_lines[i]);
            }

            base_y += text_height * 3;
            BrPixelmapText(colour_buffer, base_x, base_y, col, BrFontProp7x9, "Tests:");
            for(int i = 0; i < BR_ASIZE(tests); ++i) {
                br_colour colour = col;

                if(test_index == i)
                    colour = BR_COLOUR_RGBA(0, 0, 255, 255);

                base_y += text_height * 2;
                BrPixelmapTextF(colour_buffer, base_x, base_y, colour, BrFontProp7x9, "%4d - %s", i, tests[i].name);
            }
        }
        BrPixelmapDoubleBuffer(screen, colour_buffer);

        if(!tests[test_index]._failed && tests[test_index].post != NULL)
            tests[test_index].post(screen, colour_buffer, tests[test_index]._user);

        /*
         * Snap a screenshot if requested.
         * It's unreasonable to expect all drivers to be able to lock the screen or backbuffer
         * for direct pixel access, so copy it to a temporary first.
         */
        if(want_screenshot) {
            br_pixelmap *pm;
            BrLogInfo("APP", "Saving screenshot to devpmtest.pix");

            pm = BrPixelmapAllocate(colour_buffer->type, colour_buffer->width, colour_buffer->height, NULL, BR_PMAF_NORMAL);
            BrPixelmapCopy(pm, colour_buffer);
            BrPixelmapSave("devpmtest.pix", pm);
            BrPixelmapFree(pm);
        }
    }

done:
    ret = 0;

    cleanup_tests();

screen_creation_failed:
    if(colour_buffer != NULL)
        BrPixelmapFree(colour_buffer);

    if(screen != NULL)
        BrPixelmapFree(screen);

    BrResFree(anchor);

    BrEnd();

    return ret;
}
