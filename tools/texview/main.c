/*
 * BRender TexView - Quick'n'dirty tool to display a pixelmap.
 */
#include <stdio.h>
#include <SDL.h>
#include <brender.h>
#include <brsdl2dev.h>

void _BrBeginHook(void)
{
    BrDevAddStatic(NULL, BrDrv1SDL2Begin, NULL);
}

void _BrEndHook(void)
{
}

int main(int argc, char **argv)
{
    br_pixelmap *screen = NULL, *colour_buffer = NULL;
    br_error     r;
    br_pixelmap *pm = NULL, *pal = NULL;

    const char *pmpath, *palpath;

    if(argc == 2) {
        pmpath  = argv[1];
        palpath = "std.pal";
    } else if(argc == 3) {
        pmpath  = argv[1];
        palpath = argv[2];
    } else {
        fprintf(stderr, "Usage: %s <file.pix> [palette.pix]\n", argv[0]);
        return 1;
    }

    BrBegin();

    BrLogSetLevel(BR_LOG_TRACE);

    BrMapFindHook(BrMapFindFailedLoad);
    BrTableFindHook(BrTableFindFailedLoad);

    if((pm = BrPixelmapLoad(pmpath)) == NULL) {
        BrLogError("APP", "Error loading %s\n", pmpath);
        return 1;
    }

    BrLogInfo("APP", "Pixelmap Info:");
    BrLogInfo("APP", "  Identifier: %s", pm->identifier);
    BrLogInfo("APP", "  Type:       %d", pm->type);
    BrLogInfo("APP", "  Width:      %d", pm->width);
    BrLogInfo("APP", "  Height:     %d", pm->height);
    BrLogInfo("APP", "  Pitch:      %d", pm->row_bytes);

    // clang-format off
    r = BrDevBeginVar(&screen, "SDL2",
                      BRT_WIDTH_I32, (br_int_32)pm->width,
                      BRT_HEIGHT_I32, (br_int_32)pm->height,
                      BR_NULL_TOKEN);
    // clang-format on

    if(r != BRE_OK) {
        BrLogError("APP", "BrDevBeginVar() failed.");
        goto done;
    }

#if 1
    if((colour_buffer = BrPixelmapMatch(screen, BR_PMMATCH_OFFSCREEN)) == NULL) {
        BrLogError("APP", "BrPixelmapMatch() failed.");
        goto done;
    }
#else
    if((colour_buffer = BrPixelmapMatchTyped(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_INDEX_8)) == NULL) {
        BrLogError("APP", "BrPixelmapMatch() failed.");
        goto done;
    }
#endif

    switch(pm->type) {
        case BR_PMT_INDEX_1:
        case BR_PMT_INDEX_2:
        case BR_PMT_INDEX_4:
        case BR_PMT_INDEX_8:
        case BR_PMT_INDEXA_44:
        case BR_PMT_INDEXA_88:
        case BR_PMT_NORMAL_INDEX_8:
            if(pm->map == NULL) {
                BrLogInfo("APP", "No embedded palette, loading external %s", palpath);

                if((pal = BrPixelmapLoad(palpath)) == NULL) {
                    BrLogError("APP", "Unable to load palette");
                    goto done;
                }

                // BrPixelmapPaletteSet(colour_buffer, pal);

                pm->map = pal;
            }
            break;
        default:
            break;
    }

    BrPixelmapCopy(colour_buffer, pm);
    BrPixelmapFree(pm);
    pm = NULL;

    BrPixelmapDoubleBuffer(screen, colour_buffer);

    for(SDL_Event evt;;) {
        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_QUIT:
                    goto done;
                case SDL_WINDOWEVENT:
                    if(BrPixelmapHandleWindowEvent(screen, &evt.window) != BRE_OK) {
                        BrLogError("APP", "Error handling window event");
                        goto done;
                    }
            }
        }
    }

done:

    if(colour_buffer != NULL)
        BrPixelmapFree(colour_buffer);

    if(screen != NULL)
        BrPixelmapFree(screen);

    if(pm != NULL)
        BrPixelmapFree(pm);

    if(pal != NULL)
        BrPixelmapFree(pal);

    BrEnd();

    return 0;
}