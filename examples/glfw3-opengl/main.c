#include <GLFW/glfw3.h>
#include <brender.h>
#include <brglrend.h>

typedef struct glfw_gl_state {
    GLFWwindow *window;
    int         in_resize_callback;

    br_pixelmap *screen;
    br_pixelmap *colour_buffer;
    br_pixelmap *depth_buffer;
} glfw_gl_state;

static void *glfw_gl_create_context(br_pixelmap *pm, void *user)
{
    glfw_gl_state *state = user;

    (void)pm;

    /* GLFW doesn't support manual context creation, just return the window. */
    return state->window;
}

static void glfw_gl_delete_context(br_pixelmap *pm, void *ctx, void *user)
{
    (void)pm;
    (void)ctx;
    (void)user;
    /* nop */
}

static br_error glfw_gl_make_current(br_pixelmap *pm, void *ctx, void *user)
{
    glfw_gl_state *state = user;

    (void)pm;
    (void)ctx;

    // ctx == state->window

    glfwMakeContextCurrent(state->window);

    return BRE_OK;
}

static void (*glfw_gl_get_proc_address(const char *name))(void)
{
    return glfwGetProcAddress(name);
}

static void glfw_gl_swap_buffers(br_pixelmap *pm, void *user)
{
    glfw_gl_state *state = user;

    (void)pm;
    glfwSwapBuffers(state->window);
}

static br_error glfw_gl_resize(br_pixelmap *pm, br_int_32 w, br_int_32 h, void *user)
{
    glfw_gl_state *state = user;
    float          xscale, yscale;

    (void)pm;

    /*
     * If we're being called from a GLFW resize callback, don't call glfwSetWindowSize().
     */
    if(state->in_resize_callback)
        return BRE_OK;

    glfwGetWindowContentScale(state->window, &xscale, &yscale);

    glfwSetWindowSize(state->window, (int)((float)w * xscale), (int)((float)h * yscale));
    return BRE_OK;
}

static br_pixelmap *create_screen(GLFWwindow *window, glfw_gl_state *state)
{
    br_pixelmap *pm = NULL;
    int          width, height;

    /* No need to keep this around, glrend copies it. */
    br_device_gl_ext_procs gl_ext_procs = {
        .create_context      = glfw_gl_create_context,
        .delete_context      = glfw_gl_delete_context,
        .make_current        = glfw_gl_make_current,
        .get_proc_address    = glfw_gl_get_proc_address,
        .resize              = glfw_gl_resize,
        .swap_buffers        = glfw_gl_swap_buffers,
        .preswap_hook        = NULL, /* This is where you'd render things like ImGui. */
        .free                = NULL, /* Not needed for GLFW. */
        .handle_window_event = NULL, /* GLFW doesn't have window events. */
        .user                = state,
    };

    br_token_value tv[] = {
        {.t = BRT_WIDTH_I32,          .v = 0                   },
        {.t = BRT_HEIGHT_I32,         .v = 0                   },
        {.t = BRT_PIXEL_TYPE_U8,      .v = 0                   },
        {.t = BRT_OPENGL_EXT_PROCS_P, .v = {.h = &gl_ext_procs}},
        {.t = BR_NULL_TOKEN,          .v = 0                   },
    };

    glfwGetFramebufferSize(window, &width, &height);
    tv[0].v.i32 = (br_int_32)width;
    tv[1].v.i32 = (br_int_32)height;

    /*
     * GLFW provides no way to get this.
     * It doesn't matter anyway as we can't write directly to the front screen.
     */
    tv[2].v.u8 = BR_PMT_RGB_888;

    if(BrDevBeginTV(&pm, "glrend", tv) != BRE_OK)
        return NULL;

    pm->origin_x = (br_int_16)(pm->width / 2);
    pm->origin_y = (br_int_16)(pm->height / 2);

    return pm;
}

void cb_framebuffer_resize(GLFWwindow *window, int width, int height)
{
    glfw_gl_state *state = glfwGetWindowUserPointer(window);

    if(state == NULL)
        return;

    ++state->in_resize_callback;
    BrPixelmapResize(state->screen, width, height);
    --state->in_resize_callback;

    state->screen->origin_x = (br_int_16)(state->screen->width / 2);
    state->screen->origin_y = (br_int_16)(state->screen->height / 2);

    BrPixelmapResizeBuffers(state->screen, &state->colour_buffer, &state->depth_buffer);
}

void cb_error(int error_code, const char *description)
{
    BrLogError("GLFW", "GLFW Error %d: %s", error_code, description);
}

void _BrBeginHook(void)
{
    BrDevAddStatic(NULL, BrDrv1GLBegin, NULL);
}

void _BrEndHook(void)
{
}

int main(int argc, char **argv)
{
    GLFWwindow *window = NULL;
    int         ret    = 0;

    glfw_gl_state state = {};

    BrBegin();

    BrLogSetLevel(BR_LOG_TRACE);

    glfwSetErrorCallback(cb_error);

    if(!glfwInit()) {
        ret = 1;
        goto glfwinit_failed;
    }

    window = glfwCreateWindow(640, 480, "BRender Sample GLFW Application", NULL, NULL);
    if(window == NULL) {
        ret = 1;
        goto done;
    }

    state.window = window;

    glfwSetWindowUserPointer(window, &state);
    glfwSetFramebufferSizeCallback(state.window, cb_framebuffer_resize);

    if((state.screen = create_screen(state.window, &state)) == NULL) {
        ret = 1;
        goto done;
    }

    if(BrPixelmapResizeBuffers(state.screen, &state.colour_buffer, &state.depth_buffer) != BRE_OK) {
        ret = 1;
        goto done;
    }

    while(!glfwWindowShouldClose(state.window)) {
        const char *text = "Hello, I am a BRender OpenGL application using GLFW.";
        br_uint_16  textwidth;

        BrPixelmapFill(state.colour_buffer, BR_COLOUR_RGB(66, 66, 66));
        BrPixelmapFill(state.depth_buffer, 0xFFFFFFFF);

        textwidth = BrPixelmapTextWidth(state.colour_buffer, BrFontProp7x9, text);
        BrPixelmapTextF(state.colour_buffer, -textwidth / 2, 0, 0xFFFFFFFF, BrFontProp7x9, text);

        BrPixelmapDoubleBuffer(state.screen, state.colour_buffer);
        glfwPollEvents();
    }

done:

    if(state.depth_buffer != NULL)
        BrPixelmapFree(state.depth_buffer);

    if(state.colour_buffer != NULL)
        BrPixelmapFree(state.colour_buffer);

    if(state.screen != NULL)
        BrPixelmapFree(state.screen);

    if(state.window != NULL)
        glfwDestroyWindow(state.window);

    glfwTerminate();
glfwinit_failed:
    BrEnd();
    return ret;
}
