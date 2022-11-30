#include <brender.h>
#include <brsdl.h>

void BrSDLUtilCleanupScreen(br_pixelmap **screen, br_pixelmap **colour_buffer,
                            br_pixelmap **depth_buffer)
{
    if(*colour_buffer != NULL)
        BrPixelmapFill(*colour_buffer, 0);

    if(*colour_buffer && *screen)
        BrPixelmapDoubleBuffer(*screen, *colour_buffer);

    BrRendererEnd();

    if(*colour_buffer)
        BrPixelmapFree(*colour_buffer);

    if(*depth_buffer)
        BrPixelmapFree(*depth_buffer);

    if(*screen)
        BrPixelmapFree(*screen);

    *colour_buffer = *depth_buffer = *screen = NULL;
}

br_boolean BrSDLUtilResizeScreen(const char *driver, br_pixelmap **screen,
                                 br_pixelmap **colour_buffer, br_pixelmap **depth_buffer,
                                 void *primitive_heap, br_size_t primitive_heap_size,
                                 br_int_16 width, br_int_16 height, br_int_16 initial_bpp)
{
    br_error    r;
    br_pixelmap *tmp;
    br_int_16   origin_x, origin_y;

    if(*screen != NULL && (*screen)->width == width && (*screen)->height == height)
        return BR_TRUE;

    origin_x = (br_int_16)(width >> 1);
    origin_y = (br_int_16)(height >> 1);

    /*
     * Try to resize the framebuffers directly. Fall back to recreation if we can't.
     */
    if(*screen != NULL && *colour_buffer != NULL && *depth_buffer != NULL) {
        if((tmp = BrPixelmapResize(*screen, width, height)) == NULL)
            goto full_cleanup;

        *screen = tmp;
        tmp->origin_x = origin_x;
        tmp->origin_y = origin_y;

        if((tmp = BrPixelmapResize(*colour_buffer, width, height)) == NULL)
            goto full_cleanup;

        *colour_buffer = tmp;
        tmp->origin_x = origin_x;
        tmp->origin_y = origin_y;

        if((tmp = BrPixelmapResize(*depth_buffer, width, height)) == NULL)
            goto full_cleanup;

        *depth_buffer = tmp;
        tmp->origin_x = origin_x;
        tmp->origin_y = origin_y;

        return BR_TRUE;
    }

full_cleanup:
    BrSDLUtilCleanupScreen(screen, colour_buffer, depth_buffer);

    r = BrDevBeginVar(&tmp, driver,
                      BRT_WIDTH_I32, (br_int_32)width,
                      BRT_HEIGHT_I32, (br_int_32)height,
                      BRT_PIXEL_BITS_I32, (br_int_32)initial_bpp,
                      BR_NULL_TOKEN
    );

    if(r != BRE_OK)
        goto br_dev_begin_failed;

    *screen = tmp;
    tmp->origin_x = origin_x;
    tmp->origin_y = origin_y;

    /* NB: Always request a BR_PMT_RGB_888 buffer, regardless of the screen depth. */
    if((tmp = BrPixelmapMatchTypedSized(*screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGB_888, width, height)) == NULL)
        goto colour_buffer_failed;

    *colour_buffer = tmp;
    tmp->origin_x = origin_x;
    tmp->origin_y = origin_y;

    if((tmp = BrPixelmapMatch(*colour_buffer, BR_PMMATCH_DEPTH_16)) == NULL)
        goto depth_buffer_failed;

    *depth_buffer = tmp;
    tmp->origin_x = origin_x;
    tmp->origin_y = origin_y;

    BrRendererBegin(*colour_buffer, NULL, NULL, primitive_heap, primitive_heap_size);

    return BR_TRUE;

depth_buffer_failed:
    BrPixelmapFree(*colour_buffer);
colour_buffer_failed:
    BrPixelmapFree(*screen);
br_dev_begin_failed:
    *colour_buffer = *depth_buffer = *screen = NULL;
    return BR_FALSE;
}

br_boolean BrSDLUtilSetFullscreen(SDL_Window *sdl_window, br_boolean fullscreen)
{
    if(fullscreen)
        SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    else
        SDL_SetWindowFullscreen(sdl_window, 0);
    return fullscreen;
}

br_boolean BrSDLUtilIsAltEnter(const SDL_KeyboardEvent *evt)
{
    if(evt->repeat)
        return BR_FALSE;

    if(evt->state != SDL_PRESSED)
        return BR_FALSE;

    if(!(evt->keysym.mod & KMOD_ALT))
        return BR_FALSE;

    return evt->keysym.scancode == SDL_SCANCODE_RETURN;
}

br_boolean BrSDLUtilOnResize(SDL_Window *window, const char *driver, br_pixelmap **screen,
                             br_pixelmap **colour_buffer, br_pixelmap **depth_buffer,
                             void *primitive_heap, br_size_t primitive_heap_size,
                             br_camera *camera)
{
    int width, height;

    SDL_GL_GetDrawableSize(window, &width, &height);

    if(width == 0 || height == 0)
        return BR_TRUE;

    if(BrSDLUtilResizeScreen(driver, screen, colour_buffer, depth_buffer, primitive_heap, primitive_heap_size, (br_int_16)width, (br_int_16)height, 24) != BR_TRUE)
        return BR_FALSE;

    if(camera != NULL)
        camera->aspect = BR_DIV(BR_SCALAR(width), BR_SCALAR(height));

    return BR_TRUE;
}
