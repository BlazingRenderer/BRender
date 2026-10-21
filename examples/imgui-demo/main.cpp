/*
 * Dear ImGui, drawn through BRender's 2D primitives.
 *
 * The UI is ImGui's own demo window. Every triangle of it goes through Br2D*()
 * and the engine's immediate geometry path, so the same binary draws it on the
 * software rasteriser, on glrend/glrend1x and on vkrend:
 *
 *   imgui-demo                                glrend
 *   imgui-demo --opengl-device-name=glrend1x  glrend1x
 *   imgui-demo --vulkan                       vkrend
 *   imgui-demo --force-software               softrend + softprim
 *
 * Input is ImGui's SDL3 platform backend; this file only has to hand it the
 * events brdemo would otherwise discard, and to order the 3D frame and the UI
 * within a demo frame.
 */
#include <SDL3/SDL.h>
#include <brender.h>
#include <brdemo.h>
#include <brsdl3dev.h>

#include "imgui.h"
#include "imgui_impl_brender.h"
#include "imgui_impl_sdl3.h"

typedef struct br_imgui_demo {
    SDL_Window *window;
    br_scalar   dt;
    bool        sdl3_ready;
    bool        brender_ready;
} br_imgui_demo;

static br_error demo_init(br_demo *demo)
{
    br_imgui_demo *state;

    state      = static_cast<br_imgui_demo *>(BrResAllocate(demo, sizeof(br_imgui_demo), BR_MEMORY_APPLICATION));
    demo->user = state;

    if((state->window = BrSDL3UtilGetWindow(demo->_screen)) == nullptr) {
        BrLogError("IMGUI", "The demo's window is not an SDL3 one.");
        return BRE_FAIL;
    }

    demo->clear_colour = BR_COLOUR_RGB(28, 32, 38);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    /* An example should not write imgui.ini (or a log) into the working directory. */
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    /* BRender has no cursor of its own; let the UI draw the mouse. */
    io.MouseDrawCursor = true;

    if(!(state->sdl3_ready = ImGui_ImplSDL3_InitForOther(state->window)))
        return BRE_FAIL;

    if(!(state->brender_ready = ImGui_ImplBrender_Init()))
        return BRE_FAIL;

    return BRE_OK;
}

static void demo_process_event(br_demo *demo, const SDL_Event *evt)
{
    (void)demo;
    ImGui_ImplSDL3_ProcessEvent(evt);
}

static void demo_update(br_demo *demo, br_scalar dt)
{
    br_imgui_demo *state = static_cast<br_imgui_demo *>(demo->user);
    state->dt            = dt;
}

static void demo_render(br_demo *demo)
{
    br_imgui_demo *state = static_cast<br_imgui_demo *>(demo->user);

    ImGuiIO& io = ImGui::GetIO();

    /*
     * The SDL3 backend fills in the display size and scale and the input state in
     * NewFrame(); the frame time is the application's to provide.
     */
    io.DeltaTime = state->dt > BR_SCALAR(0) ? state->dt : BR_SCALAR(1.0 / 60.0);

    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();

    /*
     * A batch names its destination and opens the renderer's per-frame state
     * itself, so there is nothing to set up first - this demo has no 3D scene.
     */
    BrPixelmapFill(demo->colour_buffer, demo->clear_colour);
    ImGui_ImplBrender_RenderDrawData(ImGui::GetDrawData(), demo->colour_buffer);
}

static void demo_destroy(br_demo *demo)
{
    br_imgui_demo *state = static_cast<br_imgui_demo *>(demo->user);

    if(state->brender_ready)
        ImGui_ImplBrender_Shutdown();

    if(state->sdl3_ready)
        ImGui_ImplSDL3_Shutdown();

    if(ImGui::GetCurrentContext() != nullptr)
        ImGui::DestroyContext();
}

static constexpr br_demo_dispatch dispatch = {
    /* .init          = */ demo_init,
    /* .process_event = */ demo_process_event,
    /* .update        = */ demo_update,
    /* .render        = */ demo_render,
    /* .on_resize     = */ BrDemoDefaultOnResize,
    /* .destroy       = */ demo_destroy,
};

int main(int argc, char *argv[])
{

    br_demo_run_args args;

    BrDemoDefaultArgs(&args);
    args.title = "Dear ImGui over BRender";

    switch(BrDemoParseArgs(&args, argc, argv, nullptr, 0, nullptr, nullptr, 0)) {
        case BR_DEMO_PARSE_HELP:
            BrDemoUsage(stdout, argv[0], nullptr, &args, nullptr, 0);
            return 0;

        case BR_DEMO_PARSE_ERROR:
            BrDemoUsage(stderr, argv[0], nullptr, &args, nullptr, 0);
            return 1;

        case BR_DEMO_PARSE_OK:
            break;
    }

    /*
     * brdemo's software default is INDEX_8, and the 2D path has no indexed-output
     * blocks: an RGBA map drawn into an 8-bit colour buffer matches no primitive.
     * Default the software renderer to RGB so that --force-software shows a UI
     * rather than an empty frame.
     */
    if(args.force_software && args.pm_type == BR_PMT_MAX)
        args.pm_type = BR_PMT_RGB_888;

    return BrDemoRunArg(&dispatch, &args);
}
