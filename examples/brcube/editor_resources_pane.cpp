#include <brddi.h>
#include "editor_material.hpp"
#include "editor_resources_pane.hpp"

Editor::ResourcesPane::ResourcesPane(const char *id, br_pixelmap *screen, br_registry *reg)
    : m_type(TYPE_PIXELMAP), m_searchbuf{0}, m_mat_preview(screen, reg), m_screen(screen)
{
    this->m_id.reset(BrResSprintf(nullptr, "Resources##%s", id));
}

Editor::ResourcesPane::~ResourcesPane()
{
}

void Editor::ResourcesPane::DrawObject(size_t id, br_pixelmap *pm, ImVec2 region)
{
    (void)id;

    GLuint tex;
    if((tex = Editor::GetPixelmapTexture(pm)) != 0) {
        ImGui::Image(static_cast<ImTextureID>(tex), region, ImVec2(0, 0), ImVec2(1, 1));
    }
}

void Editor::ResourcesPane::DrawObject(size_t id, br_material *mat, ImVec2 region)
{
    MaterialUser *user = MaterialUser::Ensure(mat);

    if(user->preview == nullptr) {
        user->preview = BrPixelmapMatchTypedSized(this->m_screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBA_8888, 128, 128);
    }

    this->m_mat_preview.Draw(user->preview, mat);

    this->DrawObject(id, user->preview, region);
}

void Editor::ResourcesPane::DrawObject(size_t id, br_model *mod, ImVec2 region)
{
}

static const char *typestrings[] = {
    "Pixelmaps", "Tables", "Materials", "Models", "Actors",
};

void Editor::ResourcesPane::Draw()
{
    ImGui::Begin(this->m_id.get());

    ImGui::Columns(2);
    if(ImGui::BeginCombo("Type", typestrings[this->m_type], 0)) {
        for(int i = 0; i < TYPE_MAX; ++i) {
            const bool is_selected = this->m_type == i;

            if(ImGui::Selectable(typestrings[i], is_selected))
                this->m_type = static_cast<Type>(i);

            if(is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ImGui::NextColumn();
    ImGui::InputText("Search", m_searchbuf, sizeof(m_searchbuf));
    ImGui::Columns(1);

    auto xx = ImGui::GetContentRegionAvail();

    size_t num_x = static_cast<size_t>(xx.x / 128.0f);
    if(num_x == 0)
        num_x = 1;

    switch(this->m_type) {
        case TYPE_PIXELMAP:
            // clang-format off
            BrMapEnum(nullptr, [](br_pixelmap *pm, void *arg) -> br_uint_32 {
                ResourcesPane *self = reinterpret_cast<ResourcesPane *>(arg);
                self->m_pixelmaps.push_back(pm);
                return 0;
            }, this);
            // clang-format on

            for(size_t i = 0; i < this->m_pixelmaps.size(); ++i) {
                this->DrawItem(i, this->m_pixelmaps[i]);
                if((i + 1) % num_x)
                    ImGui::SameLine();
            }
            break;

        case TYPE_MATERIAL:
            BrMaterialEnum(
                nullptr,
                [](br_material *mat, void *arg) -> br_uint_32 {
                    ResourcesPane *self = reinterpret_cast<ResourcesPane *>(arg);
                    self->m_materials.push_back(mat);
                    return 0;
                },
                this);

            for(size_t i = 0; i < this->m_materials.size(); ++i) {
                DrawItem(i, this->m_materials[i]);
                if((i + 1) % num_x)
                    ImGui::SameLine();
            }

            break;

        case TYPE_MODEL:
            BrModelEnum(
                nullptr,
                [](br_model *mod, void *arg) -> br_uint_32 {
                    ResourcesPane *self = reinterpret_cast<ResourcesPane *>(arg);
                    self->m_models.push_back(mod);
                    return 0;
                },
                this);
    }

    this->m_pixelmaps.clear();
    this->m_materials.clear();
    this->m_models.clear();
    ImGui::End();
}
