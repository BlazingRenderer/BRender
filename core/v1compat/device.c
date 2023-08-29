
#include "brender.h"

static br_pixelmap *last_begin_screen;

/*
 * horrible hack to give the old 1.1 entry points access to the last created screen
 */
br_pixelmap *BrDevLastBeginQuery(void)
{
    return last_begin_screen;
}

void BrDevLastBeginSet(br_pixelmap *pm)
{
    last_begin_screen = pm;
}

/*
 * old style of device begin, for compatibility with DOSGfxBegin etc
 */
br_pixelmap *BR_PUBLIC_ENTRY BrDevBeginOld(const char *setup_string)
{
    br_pixelmap *s;

    if(BrDevLastBeginQuery()) {
        BR_FAILURE("Device already active");
        return NULL;
    }

    if(BrDevBegin(&s, setup_string) != BRE_OK) {
        BR_FAILURE("Could not start driver");
        return NULL;
    }

    BrDevLastBeginSet(s);

    return s;
}

void BR_PUBLIC_ENTRY BrDevEndOld(void)
{
    if(BrDevLastBeginQuery())
        BrPixelmapFree(BrDevLastBeginQuery());

    BrDevLastBeginSet(NULL);
}

void BR_PUBLIC_ENTRY BrDevPaletteSetOld(br_pixelmap *pm)
{
    if(BrDevLastBeginQuery())
        BrPixelmapPaletteSet(BrDevLastBeginQuery(), pm);
}

void BR_PUBLIC_ENTRY BrDevPaletteSetEntryOld(int i, br_colour colour)
{
    if(BrDevLastBeginQuery())
        BrPixelmapPaletteEntrySet(BrDevLastBeginQuery(), i, colour);
}
