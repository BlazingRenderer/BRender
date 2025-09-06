
#include "editor.hpp"
#include "ImGuizmo/ImGuizmo.h"

bool ImGui::InputUInt16(const char *label, uint16_t *v, int16_t step, int16_t step_fast, ImGuiInputTextFlags extra_flags)
{
    int iv = *v;

    bool b = ImGui::InputInt(label, &iv, step, step_fast, extra_flags);
    if(b) {
        if(iv > UINT16_MAX)
            iv = UINT16_MAX;

        if(iv < 0)
            iv = 0;

        *v = static_cast<uint16_t>(iv);
    }

    return b;
}

Editor::Editor::Editor()
    : m_screen(nullptr), m_log_handler(), m_log_pane("LogPane"), m_about_dialog("AboutDialog"), m_next_camera_pane_no(0), m_world(nullptr),
      m_graph_pane(0)
{
    this->m_context.reset(ImGui::CreateContext());
    ImGui::SetCurrentContext(this->m_context.get());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    this->m_log_handler = (br_loghandler){
        .identifier = "editor log handler",
        .handler    = Editor::LogHandlerStub,
        .user       = this,
    };

    BrRegistryNew(&this->m_reg);
    this->m_reg.find_failed_hook = nullptr;
}

Editor::Editor::~Editor()
{
    // TODO: clear registry
}

Editor::CameraPane *Editor::Editor::AddCameraPane()
{
    char id[32];
    BrSprintf(id, "Cam%02zu", m_next_camera_pane_no++);
    return this->m_camera_panes.emplace_back(std::make_unique<CameraPane>(id, this->m_screen)).get();
}

void Editor::Editor::LogMessageV(br_uint_8 level, const char *component, const char *fmt, va_list ap)
{
    br_res_ptr<char> message(BrResVSprintf(nullptr, fmt, ap));

    this->m_logs.emplace_back(LogEntry{
        .timestamp = std::chrono::system_clock::now(),
        .sdl_ticks = SDL_GetTicksNS(),
        .level     = level,
        .component = component,
        .message   = message.get(),
    });
}

void Editor::Editor::Init(SDL_Window *window, SDL_GLContext ctx, br_pixelmap *screen)
{
    this->m_config.Read("editor.ini");

    ImGui::SetCurrentContext(this->m_context.get());
    ImGui_ImplSDL3_InitForOpenGL(window, ctx);
    ImGui_ImplOpenGL3_Init("#version 150");
    this->m_screen = screen;

    this->LoadResources();

    this->m_log_pane.SetSource(&this->m_logs);
    this->m_about_dialog.Refresh(screen, &this->m_reg);

    for(size_t i = 0; i < this->m_config.num_camera_panes; ++i) {
        this->AddCameraPane();
    }

    m_resource_panes.emplace_back(std::make_unique<ResourcesPane>("TestResourcesPane", screen, &this->m_reg));
}

void Editor::Editor::LoadResources()
{
    br_res_ptr<char> path;
    br_pixelmap     *pm;
    br_model        *mdl;

    path.reset(BrResSprintf(nullptr, "editor/breditor_logo.png"));
    if((pm = BrFmtPNGLoad(path.get(), 0)) != nullptr) {
        if(pm->identifier)
            BrResFree(pm->identifier);

        pm->identifier = BrResStrDup(pm, "breditor_logo");

        BrMapUpdate(pm, BR_MATU_ALL);
        BrRegistryAdd(&this->m_reg, pm);
    }

    path.reset(BrResSprintf(nullptr, "editor/breditor_sph32.dat"));
    if((mdl = BrModelLoad(path.get())) != nullptr) {
        if(mdl->identifier)
            BrResFree(mdl->identifier);

        mdl->identifier = BrResStrDup(mdl, "breditor_sph32");

        BrModelUpdate(mdl, BR_MODU_ALL);
        BrRegistryAdd(&this->m_reg, mdl);
    }
}

void Editor::Editor::SetWorld(br_actor *world)
{
    this->m_world = world;
}

void Editor::Editor::Shutdown()
{
    ImGui::SetCurrentContext(this->m_context.get());

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    this->m_config.Write("editor.ini");
}

bool Editor::Editor::ProcessEvent(const SDL_Event *event)
{
    ImGui::SetCurrentContext(this->m_context.get());
    ImGuiIO& io = ImGui::GetIO();

    return ImGui_ImplSDL3_ProcessEvent(event);
    //
    // switch(event->type) {
    //     case SDL_EVENT_MOUSE_MOTION:
    //     case SDL_EVENT_MOUSE_BUTTON_DOWN:
    //     case SDL_EVENT_MOUSE_BUTTON_UP:
    //     case SDL_EVENT_MOUSE_WHEEL:
    //         if(io.WantCaptureMouse)
    //             return true;
    //
    //     case SDL_EVENT_KEY_DOWN:
    //     case SDL_EVENT_KEY_UP:
    //     case SDL_EVENT_TEXT_EDITING:
    //     case SDL_EVENT_TEXT_INPUT:
    //     case SDL_EVENT_KEYMAP_CHANGED:
    //         if(io.WantCaptureKeyboard)
    //             return true;
    // }
    //
    // return false;
}

void Editor::Editor::Update(float dt)
{
    for(auto& cam : this->m_camera_panes)
        cam->Update(dt);
}

void Editor::Editor::DrawMainMenuBar()
{
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Exit")) {
                SDL_Event evt{
                    .quit = (SDL_QuitEvent){.type = SDL_EVENT_QUIT, .timestamp = SDL_GetTicksNS()},
                };

                SDL_PushEvent(&evt);
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("View")) {

            ImGui::SeparatorText("Camera");

            if(ImGui::MenuItem("New Viewport")) {
                this->AddCameraPane();
            }

            //            if(ImGui::MenuItem("Reset##Camera")) {
            //                for(auto& cam : this->m_camera_panes) {
            //                    if(cam->IsFocused()) {
            //                        BrEditorCamReset(cam->GetCamera());
            //                        break;
            //                    }
            //                }
            //            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Help")) {
            if(ImGui::MenuItem("About")) {
                this->m_about_dialog.SetVisible(true);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Editor::Editor::Render()
{
    ImGui::SetCurrentContext(this->m_context.get());

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    //    ImGuizmo::SetOrthographic(true);
    //    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    //    ImGuizmo::BeginFrame();

    // ImGui::GetMainViewport()
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    this->DrawMainMenuBar();

#if 0
    ImGui::Begin("XXXX");
    {
        if(ImGui::Button("XXX")) {
            ++m_new_camera_panes;
        }
    }
    ImGui::End();
#endif

    bool show_demo_window = true;
    ImGui::ShowDemoWindow(&show_demo_window);

    m_log_pane.Draw();
    m_graph_pane.Draw(this->m_world);
    m_inspector.Draw(m_graph_pane.GetSelection(), 0);

    for(auto& cam : this->m_camera_panes) {
        cam->Draw(this->m_world);
    }

    for(auto& p : this->m_resource_panes) {
        p->Draw();
    }

    this->m_about_dialog.Draw();

    ImGui::Render();
}

void Editor::Editor::PreSwapHook(br_pixelmap *pm, void *user)
{
    reinterpret_cast<Editor *>(user)->preswap(pm);
}

void Editor::Editor::preswap(br_pixelmap *pm)
{
    ImGui::SetCurrentContext(this->m_context.get());
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::Editor::LogHandlerStub(void *user, br_uint_8 level, const char *component, const char *fmt, va_list ap)
{
    return reinterpret_cast<Editor *>(user)->LogMessageV(level, component, fmt, ap);
}
