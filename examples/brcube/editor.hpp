#ifndef EDITOR_HPP_
#define EDITOR_HPP_

#include <memory>
#include <optional>
#include <vector>
#include <chrono>

#include <brender.h>
#include <brddi.h>
#include <SDL3/SDL.h>
#include "editor_common.hpp"
#include "editor_config.hpp"
#include "editor_resources.hpp"
#include "editor_log_pane.hpp"
#include "editor_actor.hpp"
#include "editor_graph_pane.hpp"
#include "editor_inspector.hpp"
#include "editor_camera_pane.hpp"
#include "editor_resources_pane.hpp"
#include "editor_about_dialog.hpp"

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl3.h"
#include "editorcam.h"

namespace Editor
{

class Editor
{
public:
    Editor();
    ~Editor();

    void Init(SDL_Window *window, SDL_GLContext ctx, br_pixelmap *screen);

    void SetWorld(br_actor *world);

    void Shutdown();
    bool ProcessEvent(const SDL_Event *event);
    void Update(float dt);
    void Render();

    //    void SetCameraTarget(br_pixelmap *pm)
    //    {
    //        m_camera_target = pm;
    //    }

    CameraPane *GetCameraPane() noexcept
    {
        return this->m_camera_panes[0].get();
    }

    br_loghandler *GetLogHandler() noexcept
    {
        return &this->m_log_handler;
    }

    static void PreSwapHook(br_pixelmap *pm, void *user);

    CameraPane *AddCameraPane();

    void LogMessageV(br_uint_8 level, const char *component, const char *fmt, va_list ap);

private:
    using CameraPaneVector    = std::vector<std::unique_ptr<CameraPane>>;
    using ResourcesPaneVector = std::vector<std::unique_ptr<ResourcesPane>>;

    void DrawMainMenuBar();

    void LoadResources();

    void preswap(br_pixelmap *pm);

    static void LogHandlerStub(void *user, br_uint_8 level, const char *component, const char *fmt, va_list ap);

    Config m_config;

    br_pixelmap          *m_screen;
    br_loghandler         m_log_handler;
    std::vector<LogEntry> m_logs;

    Resources        m_resources;
    AboutDialog      m_about_dialog;
    LogPane          m_log_pane;
    GraphPane        m_graph_pane;
    InspectorPane    m_inspector;
    CameraPaneVector m_camera_panes;
    size_t           m_next_camera_pane_no;

    ResourcesPaneVector m_resource_panes;

    br_actor         *m_world;
    imgui_context_ptr m_context;

    br_registry m_reg;
};

} // namespace Editor
#endif /* EDITOR_HPP_ */
