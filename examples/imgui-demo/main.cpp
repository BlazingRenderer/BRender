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
    SDL_Window  *window;
    br_pixelmap *palette;
    br_scalar    dt;
    br_boolean   sdl3_ready;
    br_boolean   brender_ready;
} br_imgui_demo;

/*
 * The palette an INDEX_8 destination draws through. An indexed batch has no colour
 * to interpolate - it writes index values - so the colours have to be chosen ahead
 * of time: index 0 is the transparent key the indexed kernels mask on, a grey ramp
 * carries the glyph antialiasing the font atlas supplies as alpha, and the rest is
 * ImGui's own style colours plus a coarse cube for anything else it asks for.
 */
#define UI_GREYS 32

static br_pixelmap *build_ui_palette()
{
    br_pixelmap      *palette;
    br_colour        *entry;
    const ImGuiStyle& style = ImGui::GetStyle();
    int               n     = 1;

    if((palette = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, nullptr, BR_PMAF_NORMAL)) == nullptr)
        return nullptr;

    entry = static_cast<br_colour *>(palette->pixels);

    /* Wipe first: only some of the 256 entries end up filled. */
    BrMemSet(entry, 0, 256 * sizeof(*entry));

    for(int i = 0; i < UI_GREYS && n < 256; ++i, ++n) {
        const br_uint_8 grey = static_cast<br_uint_8>((i * 255 + (UI_GREYS - 1) / 2) / (UI_GREYS - 1));

        entry[n] = BR_COLOUR_RGB(grey, grey, grey);
    }

    for(int i = 0; i < ImGuiCol_COUNT && n < 256; ++i, ++n) {
        const ImVec4& c = style.Colors[i];

        entry[n] = BR_COLOUR_RGB(static_cast<br_uint_8>(c.x * 255.0f), static_cast<br_uint_8>(c.y * 255.0f), static_cast<br_uint_8>(c.z * 255.0f));
    }

    /* The rest of the palette is a uniform cube, for whatever colour ImGui mixes. */
    for(int b = 0; b < 6 && n < 256; ++b) {
        for(int g = 0; g < 4 && n < 256; ++g) {
            for(int r = 0; r < 7 && n < 256; ++r, ++n) {
                const br_uint_8 red   = static_cast<br_uint_8>(r * 255 / 6);
                const br_uint_8 green = static_cast<br_uint_8>(g * 255 / 3);
                const br_uint_8 blue  = static_cast<br_uint_8>(b * 255 / 5);

                entry[n] = BR_COLOUR_RGB(red, green, blue);
            }
        }
    }

    return palette;
}

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

    /*
     * The palette is built from ImGui's style, so it has to come after the context
     * exists; an INDEX_8 destination draws through it, and so does the front screen.
     */
    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        if((state->palette = build_ui_palette()) == nullptr) {
            BrLogError("IMGUI", "Could not build the UI palette.");
            return BRE_FAIL;
        }

        /* The front screen resolves its palette from the buffer it presents. */
        BrPixelmapPaletteSet(demo->colour_buffer, state->palette);
        demo->clear_colour = 0;
    }

    /* An example should not write imgui.ini (or a log) into the working directory. */
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    /* BRender has no cursor of its own; let the UI draw the mouse. */
    io.MouseDrawCursor = true;

    if(!(state->sdl3_ready = ImGui_ImplSDL3_InitForOther(state->window)))
        return BRE_FAIL;

    if(!(state->brender_ready = ImGui_ImplBrender_Init(state->palette)))
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

    BrPixelmapFree(state->palette);
    state->palette = nullptr;
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
