/*
 * The pixelmap witness.
 *
 * A 24-bit true-colour map is byte-ordered B,G,R in memory - the least significant byte of
 * the colour first, as pixelmap.h describes `pixels`, as BrPixelmapPixelSet()/PixelGet()
 * write and read it, and as every renderer that samples one assumes. Nothing checked that
 * the writers agreed, and they did not: BrPixelmapFill(), the rectangle fill and the
 * copyBits path that text's glyph background goes through wrote R,G,B. A caller could fill a
 * map, read a pixel back and get red and blue swapped; the corpus could not see it because
 * every clear is grey and the fixtures are built a pixel at a time.
 *
 * So this writes one colour into one map every way the engine offers and asserts that all of
 * them land the same bytes and read back the same value. Nothing to bless, no device needed;
 * the checks are relations the witness computes itself.
 */
#include <brender.h>

#include "rt_demo.h"
#include "pmwitness.h"

static const br_colour pmw_colour = BR_COLOUR_RGB(0x11, 0x22, 0x33);

/*
 * The three bytes of a 24-bit pixel, and what PixelGet makes of them. A writer that used the
 * other order shows up in both, in opposite directions, which is why the message says both.
 */
static br_error pmw_check_pixel(br_pixelmap *pm, int x, int y, const char *what)
{
    const br_uint_8 *p    = (const br_uint_8 *)pm->pixels + (br_size_t)y * pm->row_bytes + (br_size_t)x * 3;
    const br_colour  got  = BrPixelmapPixelGet(pm, x, y) & 0xFFFFFF; /* no alpha byte to carry */
    const br_colour  want = pmw_colour & 0xFFFFFF;

    if(p[0] != BR_BLU(pmw_colour) || p[1] != BR_GRN(pmw_colour) || p[2] != BR_RED(pmw_colour)) {
        BrLogError("RT", "pixelmap witness: %s: bytes are %02x %02x %02x, expected %02x %02x %02x (B,G,R)", what, p[0], p[1], p[2],
                   BR_BLU(pmw_colour), BR_GRN(pmw_colour), BR_RED(pmw_colour));
        return BRE_FAIL;
    }

    if(got != want) {
        BrLogError("RT", "pixelmap witness: %s: PixelGet is %06x, expected %06x", what, (unsigned)got, (unsigned)want);
        return BRE_FAIL;
    }

    return BRE_OK;
}

/*
 * Every way the engine writes a colour into a host map, on the same map, at the same colour.
 * They have to agree: a caller that fills a map and then reads a pixel back has no way to
 * know which writer produced it.
 */
static br_error pmw_check_writers(void)
{
    br_pixelmap *pm;
    br_error     r = BRE_FAIL;

    if((pm = BrPixelmapAllocate(BR_PMT_RGB_888, 64, 16, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "pixelmap witness: could not allocate a 64x16 RGB_888 map.");
        return BRE_FAIL;
    }

    BrPixelmapFill(pm, pmw_colour);

    if(pmw_check_pixel(pm, 0, 0, "fill") != BRE_OK)
        goto out;

    BrPixelmapRectangleFill(pm, 2, 0, 1, 1, pmw_colour);

    if(pmw_check_pixel(pm, 2, 0, "rectangle fill") != BRE_OK)
        goto out;

    BrPixelmapPixelSet(pm, 1, 0, pmw_colour);

    if(pmw_check_pixel(pm, 1, 0, "pixel set") != BRE_OK)
        goto out;

    BrPixelmapLine(pm, 3, 0, 3, 0, pmw_colour);

    if(pmw_check_pixel(pm, 3, 0, "line") != BRE_OK)
        goto out;

    /*
     * Text writes its glyphs through copyBits, whose background pixels take the fill path -
     * the glyph is opaque, so the sample below it is the one this is about.
     */
    BrPixelmapText(pm, 4, 0, pmw_colour, BrFontFixed3x5, "\x01");

    if(pmw_check_pixel(pm, 4, 1, "text background") != BRE_OK)
        goto out;

    r = BRE_OK;

out:
    BrPixelmapFree(pm);
    return r;
}

/*
 * An RGBX_888 map is four bytes, so its fill writes the colour as a word: included so that a
 * change which breaks 32-bit while fixing 24-bit fails here rather than in a game.
 */
static br_error pmw_check_writers32(void)
{
    br_pixelmap *pm;
    br_colour    colour = BR_COLOUR_RGBA(0x11, 0x22, 0x33, 0xFF);
    br_error     r      = BRE_FAIL;

    if((pm = BrPixelmapAllocate(BR_PMT_RGBX_888, 16, 16, NULL, BR_PMAF_NORMAL)) == NULL)
        return BRE_FAIL;

    BrPixelmapFill(pm, colour);

    if(BrPixelmapPixelGet(pm, 0, 0) != colour) {
        BrLogError("RT", "pixelmap witness: RGBX_888 fill reads back as %06x, expected %06x", (unsigned)BrPixelmapPixelGet(pm, 0, 0),
                   (unsigned)colour);
        goto out;
    }

    BrPixelmapPixelSet(pm, 1, 0, colour);

    if(BrPixelmapPixelGet(pm, 1, 0) != colour) {
        BrLogError("RT", "pixelmap witness: RGBX_888 pixel set reads back as %06x, expected %06x", (unsigned)BrPixelmapPixelGet(pm, 1, 0),
                   (unsigned)colour);
        goto out;
    }

    r = BRE_OK;

out:
    BrPixelmapFree(pm);
    return r;
}

static br_boolean pmw_layout(const br_pixelmap *pm, int w, int h, int row)
{
    return pm != NULL && pm->width == w && pm->height == h && pm->row_bytes == row;
}

static br_error pmw_check_allocate_bounds(void)
{
    static const struct {
        br_uint_8 type;
        int       w, h, row;
    } cases[] = {
        {BR_PMT_INDEX_8,  0,     0,     0    },
        {BR_PMT_RGBX_888, 1,     65535, 4    },
        {BR_PMT_RGBX_888, 8191,  1,     32764},
        {BR_PMT_INDEX_8,  32764, 1,     32764},
        {BR_PMT_RGBX_888, 8192,  1,     -1   },
        {BR_PMT_INDEX_8,  32765, 1,     -1   },
        {BR_PMT_RGB_888,  10922, 1,     -1   },
        {BR_PMT_RGBX_888, -1,    1,     -1   },
        {BR_PMT_RGBX_888, 1,     -1,    -1   },
        {BR_PMT_RGBX_888, 65536, 1,     -1   },
        {BR_PMT_RGBX_888, 1,     65536, -1   },
        {BR_PMT_MAX,      1,     1,     -1   },
    };

    for(br_size_t i = 0; i < BR_ASIZE(cases); ++i) {
        br_pixelmap *pm = BrPixelmapAllocate(cases[i].type, cases[i].w, cases[i].h, NULL, BR_PMAF_NO_PIXELS);
        br_boolean   ok = cases[i].row < 0 ? pm == NULL : pmw_layout(pm, cases[i].w, cases[i].h, cases[i].row) && pm->pixels == NULL;

        if(pm != NULL)
            BrPixelmapFree(pm);
        if(!ok) {
            BrLogError("RT", "pixelmap witness: unexpected layout for %dx%d type %u (expected stride %d, -1 means refusal).", cases[i].w,
                       cases[i].h, (unsigned)cases[i].type, cases[i].row);
            return BRE_FAIL;
        }
    }

    return BRE_OK;
}

static br_error pmw_check_resize(void)
{
    br_pixelmap *pm   = BrPixelmapAllocate(BR_PMT_RGB_888, 4, 2, NULL, BR_PMAF_INVERTED);
    br_pixelmap *view = NULL;
    br_error     r    = BRE_FAIL;
    br_colour    want = pmw_colour & 0xffffff;
    void        *pixels;
    br_uint_16   flags;

    if(!pmw_layout(pm, 4, 2, -12) || !(pm->flags & BR_PMF_LINEAR))
        goto out;
    BrPixelmapFill(pm, pmw_colour);
    pixels = pm->pixels;
    flags  = pm->flags;

    if(BrPixelmapResize(pm, 10922, 2) != NULL || BrPixelmapResize(pm, 4, -1) != NULL || !pmw_layout(pm, 4, 2, -12) ||
       pm->pixels != pixels || pm->flags != flags || (BrPixelmapPixelGet(pm, 3, 1) & 0xffffff) != want)
        goto out;

    if(BrPixelmapResize(pm, 5, 2) == NULL || !pmw_layout(pm, 5, 2, -24) || (pm->flags & BR_PMF_LINEAR))
        goto out;

    view = BrPixelmapMipLevel(pm, 0);
    if(!pmw_layout(view, 5, 2, -24) || view->pixels != pm->pixels || view->flags != pm->flags)
        goto out;
    BrPixelmapFree(view);
    view = NULL;

    BrPixelmapFill(pm, pmw_colour);
    if((BrPixelmapPixelGet(pm, 4, 1) & 0xffffff) == want)
        r = BRE_OK;

out:
    if(view != NULL)
        BrPixelmapFree(view);
    if(pm != NULL)
        BrPixelmapFree(pm);
    if(r != BRE_OK)
        BrLogError("RT", "pixelmap witness: resize did not preserve rejected geometry or describe the new padded, inverted map.");
    return r;
}

static br_pixelmap *pmw_match_depth(br_pixelmap *src, br_int_32 w)
{
    br_token_value tokens[] = {
        {.t = BRT_USE_T,          .v = {.t = BRT_DEPTH}},
        {.t = BRT_WIDTH_I32,      .v = {.i32 = w}      },
        {.t = BRT_PIXEL_BITS_I32, .v = {.i32 = 32}     },
        {.t = BR_NULL_TOKEN,      .v = {0}             },
    };

    return BrPixelmapMatchTV(src, tokens);
}

static br_error pmw_check_match(void)
{
    br_pixelmap *src = BrPixelmapAllocate(BR_PMT_RGBX_888, 4, 2, NULL, BR_PMAF_INVERTED);
    br_pixelmap *pm  = NULL;
    br_error     r   = BRE_FAIL;

    if(src == NULL)
        goto out;
    pm = BrPixelmapMatchSized(src, BR_PMMATCH_OFFSCREEN, 8192, 1);
    if(pm != NULL)
        goto out;
    pm = pmw_match_depth(src, 8192);
    if(pm != NULL)
        goto out;
    pm = pmw_match_depth(src, 6);
    if(!pmw_layout(pm, 6, 2, -24) || pm->type != BR_PMT_DEPTH_32)
        goto out;
    BrPixelmapFree(pm);
    BrPixelmapFree(src);
    pm = NULL;

    /* The matched pitch fits even though DEPTH_32's usual padded row does not. */
    src = BrPixelmapAllocate(BR_PMT_RGBX_888, 8189, 1, NULL, BR_PMAF_NO_PIXELS);
    if(src != NULL) {
        pm = pmw_match_depth(src, 8189);
        if(pmw_layout(pm, 8189, 1, 32756) && pm->type == BR_PMT_DEPTH_32)
            r = BRE_OK;
    }

out:
    if(pm != NULL)
        BrPixelmapFree(pm);
    if(src != NULL)
        BrPixelmapFree(src);
    if(r != BRE_OK)
        BrLogError("RT", "pixelmap witness: a sized match did not reject or cover its requested row.");
    return r;
}

static br_error pmw_check_mip_chain(void)
{
    br_pixelmap *src   = BrPixelmapAllocate(BR_PMT_RGB_565, 10922, 1, NULL, BR_PMAF_NORMAL);
    br_pixelmap *chain = NULL, *level = NULL;
    br_error     r = BRE_FAIL;

    if(src == NULL)
        goto out;
    BrPixelmapFill(src, 0xffff);
    chain = BrPixelmapMakeMipMap(src, BR_PMT_RGB_888, NULL, 0, 256, BR_QUANTIZE_YIQ);
    if(!pmw_layout(chain, 10922, 1, 32766) || chain->mip_offset != 1)
        goto out;

    level = BrPixelmapMipLevel(chain, 0);
    if(!pmw_layout(level, 10922, 1, 32766) || level->pixels != chain->pixels)
        goto out;
    BrPixelmapFree(level);

    level = BrPixelmapMipLevel(chain, 1);
    if(pmw_layout(level, 5461, 1, 16383) && level->pixels == (char *)chain->pixels + 32766 &&
       (BrPixelmapPixelGet(chain, 10921, 0) & 0xffffff) == 0xffffff)
        r = BRE_OK;

out:
    if(level != NULL)
        BrPixelmapFree(level);
    if(chain != NULL)
        BrPixelmapFree(chain);
    if(src != NULL)
        BrPixelmapFree(src);
    if(r != BRE_OK)
        BrLogError("RT", "pixelmap witness: a representable tight mip row or its borrowed view was lost.");
    return r;
}

br_error rt_pm_witness_run(rt_demo *demo)
{
    (void)demo;

    if(pmw_check_writers() != BRE_OK || pmw_check_writers32() != BRE_OK || pmw_check_allocate_bounds() != BRE_OK ||
       pmw_check_resize() != BRE_OK || pmw_check_match() != BRE_OK || pmw_check_mip_chain() != BRE_OK)
        return BRE_FAIL;

    BrLogInfo("RT", "pixelmap witness passed (colour writers and memory layout).");
    return BRE_OK;
}
