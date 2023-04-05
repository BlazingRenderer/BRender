#include <SDL.h>
#include <brender.h>
#include <brsdl.h>

int main(int argc, char **argv)
{
    SDL_Window  *sdl_window;
    br_pixelmap *screen, *colour_buffer, *depth_buffer;
    int          width, height, ret = -1;
    br_error     r;
    br_uint_64   ticks_last, ticks_now;
    br_pixelmap *checkerboard, *last_frame;

    /*
     * Init SDL
     */
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        BrLogError("SDL", "Initialisation error: %s", SDL_GetError());
        goto sdl_init_failed;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    if((sdl_window = SDL_CreateWindow("BRender Sample Application", 0, 0, 1280, 720,
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI)) == NULL) {
        BrLogError("SDL", "Window creation error: %s", SDL_GetError());
        goto sdl_createwindow_failed;
    }

    BrBegin();

    BrSDLDevAddStaticGL(sdl_window);

    SDL_GL_GetDrawableSize(sdl_window, &width, &height);

    r = BrDevBeginVar(&screen, "opengl", BRT_WIDTH_I32, (br_int_32)width, BRT_HEIGHT_I32, (br_int_32)height,
                      BRT_PIXEL_BITS_I32, 24, BR_NULL_TOKEN);

    if(r != BRE_OK) {
        BrLogError("APP", "BrDevBeginVar() failed.");
        goto screen_creation_failed;
    }

    if((colour_buffer = BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGB_888, width, height)) == NULL) {
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

    last_frame = BrPixelmapAllocate(BR_PMT_RGBA_8888, colour_buffer->width, colour_buffer->height, NULL, BR_PMAF_NORMAL);

    for(SDL_Event evt;;) {
        float dt;

        ticks_now  = SDL_GetTicks64();
        dt         = (float)(ticks_now - ticks_last) / 1000.0f;
        ticks_last = ticks_now;

        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_QUIT:
                    goto done;
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

        /* Put a checkerboard in the center */
        BrPixelmapRectangleStretchCopy(colour_buffer, -128, -128, 256, 256, checkerboard, 0, 0, checkerboard->width,
                                       checkerboard->height);

        /* Draw the previous frame (black initially) in the top-right corner. */
        BrPixelmapRectangleStretchCopy(colour_buffer, 160, -320, 1280 / 3, 720 / 3, last_frame, 0, 0, last_frame->width,
                                       last_frame->height);

        BrPixelmapDoubleBuffer(screen, colour_buffer);

        /* Capture the last frame. */
        BrPixelmapCopy(last_frame, colour_buffer);
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
