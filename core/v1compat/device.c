
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
