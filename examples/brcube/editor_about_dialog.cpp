#include <brddi.h>
#include "editor_about_dialog.hpp"

Editor::AboutDialog::AboutDialog(const char *id)
    : m_screen(nullptr), m_visible(false), m_gl_buffer(), m_gl_extensions(nullptr), m_gl_num_extensions(0), m_gl_current_extension(0)
{
    this->m_id.reset(BrResSprintf(nullptr, "About BReditor##%s", id));
}

void Editor::AboutDialog::Refresh(br_pixelmap *screen, br_registry *reg)
{
    br_token       BRT_OPENGL_EXTENSIONS_PL = BrTokenFind("OPENGL_EXTENSIONS_PL");
    br_size_t      size                     = 0;
    br_int_32      count                    = 0;
    br_token_value tv[]                     = {
        {.t = BRT_OPENGL_VERSION_CSTR, .v = {}},
        {.t = BRT_OPENGL_RENDERER_CSTR, .v = {}},
        {.t = BRT_OPENGL_VENDOR_CSTR, .v = {}},
        {.t = BRT_OPENGL_EXTENSIONS_PL, .v = {}},
        {.t = BR_NULL_TOKEN},
    };
    ObjectQueryManySize(screen, &size, tv);

    this->m_gl_buffer.reset(reinterpret_cast<const char **>(BrResAllocate(nullptr, size, BR_MEMORY_APPLICATION)));

    ObjectQueryMany(screen, tv, this->m_gl_buffer.get(), size, &count);

    this->m_gl_version        = tv[0].v.cstr;
    this->m_gl_renderer       = tv[1].v.cstr;
    this->m_gl_vendor         = tv[2].v.cstr;
    this->m_gl_extensions     = const_cast<const char **>(reinterpret_cast<char **>(tv[3].v.pl));
    this->m_gl_num_extensions = 0;

    for(const char **pp = this->m_gl_extensions; *pp != nullptr; ++pp)
        ++this->m_gl_num_extensions;

    m_pm_logo = static_cast<br_pixelmap *>(BrRegistryFind(reg, "breditor_logo"));
}

bool Editor::AboutDialog::IsVisible() const noexcept
{
    return this->m_visible;
}

bool Editor::AboutDialog::SetVisible(bool visible) noexcept
{
    bool b          = this->m_visible;
    this->m_visible = visible;
    return b;
}

void Editor::AboutDialog::DrawHeader()
{
    GLuint logo_tex       = GetPixelmapTexture(this->m_pm_logo);
    ImVec2 content_region = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("Header", ImVec2(content_region.x, this->m_pm_logo->height), ImGuiChildFlags_None);
    {
        /* Draw the logo. */
        ImGui::BeginChild("Logo", ImVec2(this->m_pm_logo->width, this->m_pm_logo->height), ImGuiChildFlags_None);
        if(logo_tex != 0) {
            ImGui::Image(static_cast<ImTextureID>(logo_tex), ImVec2(this->m_pm_logo->width, this->m_pm_logo->height), ImVec2(0, 0), ImVec2(1, 1));
        }
        ImGui::EndChild();

        ImGui::SameLine();

        /* Draw the app info. */
        ImGui::BeginChild("Info", ImVec2(), ImGuiChildFlags_None);
        {
            ImGui::Text("BReditor");
            ImGui::Text("using BRender %d.%d.%d", (__BRENDER__ & 0xff0000) >> 16, (__BRENDER__ & 0x00ff00) >> 8, (__BRENDER__ & 0x0000ff) >> 0);
            ImGui::Text("\n");
            ImGui::TextWrapped("https://github.com/BlazingRenderer/BRender");
            ImGui::Separator();

            if(ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
                this->m_visible = false;
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

void Editor::AboutDialog::Draw()
{
    if(this->m_visible)
        ImGui::OpenPopup(this->m_id.get());

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(838, 0));

    if(ImGui::BeginPopupModal(this->m_id.get(), nullptr, ImGuiWindowFlags_NoResize)) {
        this->DrawHeader();

        ImGui::Separator();

        if(ImGui::BeginTabBar("XXXXX", ImGuiTabBarFlags_None)) {

            if(ImGui::BeginTabItem("OpenGL")) {

                ImGui::InputText("Version", &this->m_gl_version, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("Renderer", &this->m_gl_renderer, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputText("Vendor", &this->m_gl_vendor, ImGuiInputTextFlags_ReadOnly);

                ImGui::Text("Available Extensions");
                ImGui::PushItemWidth(-1);
                if(ImGui::BeginListBox("##Extensions", ImVec2(-FLT_MIN, 0))) {
                    for(size_t i = 0; i < this->m_gl_num_extensions; ++i) {
                        const bool selected = this->m_gl_current_extension == i;
                        if(ImGui::Selectable(m_gl_extensions[i], selected))
                            this->m_gl_current_extension = i;
                    }
                    ImGui::EndListBox();
                }
                ImGui::PopItemWidth();

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::EndPopup();
    }
}
