#include <SDL.h>
#include <brender.h>
#include <brsdl.h>

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
    SDL_Window  *sdl_window;
    br_pixelmap *screen = NULL, *colour_buffer = NULL, *depth_buffer = NULL;
    int          width, height;
    int          ret = -1;
    br_error     r;
    br_uint_64   ticks_last, ticks_now;
    br_pixelmap *checkerboard, *last_frame_memory, *last_frame_device, *rainbow_rect;
    br_pixelmap *last_frame_hxw;
    float        h, s, l;
    int          want_screenshot;

    /*
     * Init SDL
     */
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        BrLogError("SDL", "Initialisation error: %s", SDL_GetError());
        goto sdl_init_failed;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    if((sdl_window = SDL_CreateWindow("BRender Sample Application", 0, 0, 1280, 720,
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI)) == NULL) {
        BrLogError("SDL", "Window creation error: %s", SDL_GetError());
        goto sdl_createwindow_failed;
    }

    BrBegin();

    BrLogSetLevel(BR_LOG_TRACE);

    BrSDLDevAddStaticGL(sdl_window);

    SDL_GL_GetDrawableSize(sdl_window, &width, &height);

    r = BrDevBeginVar(&screen, "opengl", BRT_WIDTH_I32, (br_int_32)width, BRT_HEIGHT_I32, (br_int_32)height,
                      BRT_PIXEL_BITS_I32, 24, BR_NULL_TOKEN);

    if(r != BRE_OK) {
        BrLogError("APP", "BrDevBeginVar() failed.");
        goto screen_creation_failed;
    }

    if((colour_buffer = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBX_888, width, height)) == NULL) {
        BrLogError("APP", "BrPixelmapMatchTypedSized() failed.");
        goto screen_creation_failed;
    }

    if((depth_buffer = BrPixelmapMatch(colour_buffer, BR_PMMATCH_DEPTH_16)) == NULL) {
        BrLogError("APP", "BrPixelmapMatch() failed.");
        goto screen_creation_failed;
    }

    screen->origin_x = colour_buffer->origin_x = depth_buffer->origin_x = (br_int_16)(width / 2);
    screen->origin_y = colour_buffer->origin_y = depth_buffer->origin_y = (br_int_16)(height / 2);

    BrMapFindHook(BrMapFindFailedLoad);
    checkerboard = BrMapFind("checkerboard.pix");

    ticks_last = SDL_GetTicks64();

    last_frame_memory = BrPixelmapAllocate(colour_buffer->type, colour_buffer->width, colour_buffer->height, NULL,
                                           BR_PMAF_NORMAL);
    last_frame_device = BrPixelmapMatch(colour_buffer, BR_PMMATCH_OFFSCREEN);

    rainbow_rect = BrPixelmapAllocate(BR_PMT_RGBX_888, 128, 128, NULL, BR_PMAF_NORMAL);
    BrPixelmapFill(rainbow_rect, BR_COLOUR_RGB(0, 0, 0));

    last_frame_hxw = BrPixelmapAllocate(BR_PMT_RGBX_888, colour_buffer->height, colour_buffer->width, NULL, BR_PMAF_NORMAL);

    /* Create a rotating rainbow gradient for effect. */
    h = 0.0f;
    s = 1.0f;
    l = 0.5f;

    for(SDL_Event evt;;) {
        br_colour col;
        float     dt;

        ticks_now  = SDL_GetTicks64();
        dt         = (float)(ticks_now - ticks_last) / 1000.0f;
        ticks_last = ticks_now;

        want_screenshot = 0;
        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_QUIT:
                    goto done;
                case SDL_KEYDOWN: {
                    if(evt.key.keysym.sym == SDLK_F5)
                        want_screenshot = 1;
                }
            }
        }

        /* Clear the screen black. */
        BrPixelmapFill(colour_buffer, 0x00000000);

        /* Add a red inner box. Simulates a black outline. */
        BrPixelmapRectangleFill(colour_buffer, -colour_buffer->origin_x + 10, -colour_buffer->origin_y + 10,
                                colour_buffer->width - 20, colour_buffer->height - 20, 0x00FF0000);

        /* Add in some text. */
        {
            br_int_16 xoffset = -colour_buffer->origin_x + 20;
            br_int_16 yoffset = -colour_buffer->origin_y + 20;

            BrPixelmapTextF(colour_buffer, xoffset, yoffset, 0xFFFFFFFF, BrFontProp7x9,
                            "Hello! I am an example of BRender's pixelmap text rendering.");
            yoffset += BrPixelmapTextHeight(colour_buffer, BrFontProp7x9) + 5;

            BrPixelmapTextF(colour_buffer, xoffset, yoffset, 0xFF00FF00, BrFontProp4x6,
                            "I am text in the proportional 4x6 font. In Green!");
            yoffset += BrPixelmapTextHeight(colour_buffer, BrFontProp4x6) + 5;

            BrPixelmapTextF(colour_buffer, xoffset, yoffset, 0xFF0000FF, BrFontProp7x9,
                            "I am text in the proportional 7x9 font. In Blue!");
            yoffset += BrPixelmapTextHeight(colour_buffer, BrFontProp7x9) + 5;

            BrPixelmapTextF(colour_buffer, xoffset, yoffset, 0xFF000000, BrFontFixed3x5,
                            "I am text in the fixed 3x5 font. In Black!");
            yoffset += BrPixelmapTextHeight(colour_buffer, BrFontFixed3x5) + 5;

            BrPixelmapTextF(colour_buffer, xoffset, yoffset, 0xFF0000FF, BrFontProp7x9, "last frame delta (msec): %f",
                            dt * 1000);
        }

        /*
         * Put a checkerboard in the center.
         * Tests memory->device non-stretch copy.
         */
        BrPixelmapRectangleStretchCopy(colour_buffer, -128, -128, 256, 256, checkerboard, 0, 0, checkerboard->width,
                                       checkerboard->height);

        /*
         * Draw the previous frame (black initially) in the top-right corner.
         * Tests memory->device stretch copy.
         */
        BrPixelmapRectangleStretchCopy(colour_buffer, 160, -320, 1280 / 3, 720 / 3, last_frame_memory, 0, 0,
                                       last_frame_memory->width, last_frame_memory->height);
        BrPixelmapText(colour_buffer, 160, -340, 0xFF0000FF, BrFontProp7x9, "Memory->device stretch copy");

        /*
         * Draw the previous frame (black initially) underneath it.
         * Tests device->device stretch copy.
         */
        BrPixelmapRectangleStretchCopy(colour_buffer, 160, 0, 1280 / 3, 720 / 3, last_frame_device, -last_frame_device->origin_x,
                                       -last_frame_device->origin_y, last_frame_device->width, last_frame_device->height);
        BrPixelmapText(colour_buffer, 160, -20, 0xFF0000FF, BrFontProp7x9, "Device->device stretch copy");

        /*
         * Tests a device->memory non-stretch copy, with device memory non-addressable.
         * Hack to invoke rectangleStretchCopyFrom().
         */
        {
            last_frame_device->flags |= BR_PMF_NO_ACCESS;
            BrPixelmapRectangleStretchCopy(last_frame_hxw, -last_frame_hxw->origin_x, -last_frame_hxw->origin_y,
                                           last_frame_hxw->width, last_frame_hxw->height, last_frame_device,
                                           -last_frame_device->origin_x, -last_frame_device->origin_y,
                                           last_frame_device->width, last_frame_device->height);
            last_frame_device->flags &= ~BR_PMF_NO_ACCESS;

            BrPixelmapRectangleStretchCopy(colour_buffer, -colour_buffer->origin_x + 20, -colour_buffer->origin_y + 100,
                                           last_frame_hxw->width / 6, last_frame_hxw->height / 6, last_frame_hxw,
                                           -last_frame_hxw->origin_x, -last_frame_hxw->origin_y, last_frame_hxw->width,
                                           last_frame_hxw->height);
        }

        /*
         * Put our rainbow rect somewhere.
         * Tests:
         *  - Whole fill
         *  - Memory->device non-stretch copy
         *  - Line drawing
         */
        {
            const br_int_32 base_x = -rainbow_rect->width / 2;
            const br_int_32 base_y = -rainbow_rect->height / 2;

            h   = fmodf(h + dt * 0.1f, 1.0f);
            col = hsl2rgb(h, s, l);
            BrPixelmapFill(rainbow_rect, col);
            BrPixelmapRectangleCopy(colour_buffer, base_x, base_y, rainbow_rect, -rainbow_rect->origin_x,
                                    -rainbow_rect->origin_y, rainbow_rect->width, rainbow_rect->height);

            /*
             * Draw a grid over the rainbow rect using the opposite hue.
             */
            col = hsl2rgb(1.0f - h, s, l);

            for(int i = 0; i < 17; ++i) {
                int x1 = base_x;
                int x2 = x1 + 128;
                int y1 = base_y + (i * 8);
                int y2 = base_y + (i * 8);
                BrPixelmapLine(colour_buffer, x1, y1, x2, y2, col);
            }

            for(int i = 0; i < 17; ++i) {
                int x1 = base_x + (i * 8);
                int x2 = base_x + (i * 8);
                int y1 = base_y;
                int y2 = y1 + 128;
                BrPixelmapLine(colour_buffer, x1, y1, x2, y2, col);
            }
        }

        /*
         * Draw an outline of dots around the screen.
         */
        {
            const int base_x = -colour_buffer->origin_x;
            const int base_y = -colour_buffer->origin_y;

            for(int i = 0; i < colour_buffer->width / 10; ++i) {
                BrPixelmapPixelSet(colour_buffer, base_x + (i * 10), base_y, BR_COLOUR_RGB(255, 255, 255));
                BrPixelmapPixelSet(colour_buffer, base_x + (i * 10), -base_y - 1, BR_COLOUR_RGB(255, 255, 255));
            }

            for(int i = 0; i < colour_buffer->height / 10; ++i) {
                BrPixelmapPixelSet(colour_buffer, base_x, base_y + (i * 10), BR_COLOUR_RGB(255, 255, 255));
                BrPixelmapPixelSet(colour_buffer, -base_x - 1, base_y + (i * 10), BR_COLOUR_RGB(255, 255, 255));
            }
        }

        BrPixelmapDoubleBuffer(screen, colour_buffer);

        /* Capture the last frame, using a device->memory non-stretch copy. */
        BrPixelmapCopy(last_frame_memory, colour_buffer);

        /* Capture the last frame, using a device->device non-stretch copy. */
        BrPixelmapCopy(last_frame_device, colour_buffer);

        if(want_screenshot) {
            BrLogInfo("APP", "Saving screenshot to devpmtest.pix");
            BrPixelmapSave("devpmtest.pix", last_frame_memory);
        }
    }

done:
    ret = 0;

screen_creation_failed:
    if(depth_buffer != NULL)
        BrPixelmapFree(depth_buffer);

    if(colour_buffer != NULL)
        BrPixelmapFree(colour_buffer);

    if(screen != NULL)
        BrPixelmapFree(screen);

    BrEnd();

    SDL_DestroyWindow(sdl_window);

sdl_createwindow_failed:
    /*
     * Close down SDL
     */
    SDL_Quit();

sdl_init_failed:
    return ret;
}
