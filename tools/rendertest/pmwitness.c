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

br_error rt_pm_witness_run(rt_demo *demo)
{
    (void)demo;

    if(pmw_check_writers() != BRE_OK)
        return BRE_FAIL;

    if(pmw_check_writers32() != BRE_OK)
        return BRE_FAIL;

    BrLogInfo("RT", "pixelmap witness passed (every writer of a 24-bit pixel agrees with the others).");

    return BRE_OK;
}
