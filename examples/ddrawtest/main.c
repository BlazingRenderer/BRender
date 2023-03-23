#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <brender.h>
#include <brddi.h> /* For Object{Device,Identifier}() */
#include <inttypes.h>
#include <SDL.h>
#include <SDL_syswm.h>

static br_boolean log_outfcty(char *identifier, br_outfcty_desc *desc, void *args)
{
    struct br_device *dev;
    (void)args;

    dev = ObjectDevice(desc->output_facility);

    BrLogInfo("APP", "  DEVICE=%s IDENTIFIER=%s WIDTH=%d HEIGHT=%d BITS=%d TYPE=%d INDEXED=%d FULLSCREEN=%d",
              ObjectIdentifier(dev), identifier, desc->width, desc->height, desc->pmbits, desc->pmtype, desc->indexed,
              desc->fullscreen);

    return BR_FALSE;
}

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
        return BR_COLOUR_RGB((br_colour)(l * 255.0f), (br_colour)(l * 255.0f), (br_colour)(l * 255.0f));
    }

    q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    p = 2 * l - q;
    return BR_COLOUR_RGB((br_colour)(hue2rgb(p, q, h + 1.0f / 3.0f) * 255.0f), (br_colour)(hue2rgb(p, q, h) * 255.0f),
                         (br_colour)(hue2rgb(p, q, h - 1.0f / 3.0f) * 255.0f));
}

int main(int argc, char **argv)
{
    SDL_Window   *sdl_window;
    SDL_SysWMinfo wm_info = {0};
    char          args[256];
    br_pixelmap  *screen = NULL, *colour_buffer = NULL;
    br_error      err   = BRE_OK;
    br_uint_64    ticks_last, ticks_now;
    float         h, s, l;

    /* Use SDL to create the window, we get its HWND for DirectDraw later. */
    sdl_window = SDL_CreateWindow("BRender DirectDraw example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

    if(sdl_window == NULL) {
        SDL_ShowSimpleMessageBox(0, "SDL_CreateWindow() failed", SDL_GetError(), NULL);
        return 1;
    }

    BrBegin();

    BrLogSetLevel(BR_LOG_INFO);

    /* Get the HWND. */
    SDL_VERSION(&wm_info.version);
    if(SDL_GetWindowWMInfo(sdl_window, &wm_info) != SDL_TRUE) {
        SDL_ShowSimpleMessageBox(0, "SDL_GetWindowWMInfo() failed", SDL_GetError(), sdl_window);
        goto done;
    }

    SDL_snprintf(args, sizeof(args) - 1, "WINDOW_HANDLE=0x%" PRIxPTR, (uintptr_t)wm_info.info.win.window);

    struct br_device *BR_EXPORT BrDrv1DirectDrawBegin(const char *arguments);
    if(BrDevAddStatic(NULL, BrDrv1DirectDrawBegin, args) != BRE_OK) {
        SDL_ShowSimpleMessageBox(0, "BrDevAddStatic() failed", "Error adding DirectDraw device", sdl_window);
        goto done;
    }

    BrLogInfo("APP", "Dumping Output Facilities:");
    BrOutputFacilityEnum("*", log_outfcty, NULL);

    // clang-format off
    /* Pick the first windowed RGB888 one. Windows 8+ should always have one of these. */
    err = BrDevBeginVar(&screen, "DDRAW",
                        BRT_WINDOW_HANDLE_H,     wm_info.info.win.window,
                        BRT_PIXEL_TYPE_U8,       BR_PMT_RGBX_888,
                        BRT_WINDOW_FULLSCREEN_B, BR_FALSE,
                        NULL);
    // clang-format on

    if(err != BRE_OK) {
        SDL_ShowSimpleMessageBox(0, "BrDevBeginVar() failed", "Error creating screen pixelmap", sdl_window);
        goto done;
    }

    if((colour_buffer = BrPixelmapMatch(screen, BR_PMMATCH_OFFSCREEN)) == NULL) {
        SDL_ShowSimpleMessageBox(0, "BrPixelmapMatch() failed", "Error creating colour pixelmap", sdl_window);
        goto done;
    }

    /* Create a rotating rainbow gradient for effect. */
    h = 0.0f;
    s = 1.0f;
    l = 0.5f;

    ticks_last = SDL_GetTicks64();
    for(SDL_Event evt;;) {
        br_colour col;
        float     dt;

        ticks_now  = SDL_GetTicks64();
        dt         = (float)(ticks_now - ticks_last) / 1000.0f;
        ticks_last = ticks_now;

        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_QUIT:
                    goto done;
            }
        }

        h   = fmodf(h + dt * 0.1f, 1.0f);
        col = hsl2rgb(h, s, l);

        BrPixelmapFill(colour_buffer, col);
        BrPixelmapTextF(colour_buffer, 0, colour_buffer->height / 2, BR_COLOUR_RGB(0, 0, 0), BrFontProp7x9,
            "This is an example of using BRender's DirectDraw driver!");

        BrPixelmapDoubleBuffer(screen, colour_buffer);
    }

done:
    if(colour_buffer != NULL)
        BrPixelmapFree(colour_buffer);

    if(screen != NULL)
        BrPixelmapFree(screen);

    BrEnd();

    if(sdl_window != NULL)
        SDL_DestroyWindow(sdl_window);

    return 0;
}
