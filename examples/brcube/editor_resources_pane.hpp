#ifndef EDITOR_RESOURCES_PANE_HPP_
#define EDITOR_RESOURCES_PANE_HPP_

#include <brender.h>
#include <vector>
#include "editor_common.hpp"
#include "editor_material_preview.hpp"

namespace Editor
{

class ResourcesPane
{
public:
    constexpr const static ImVec2 ITEM_SIZE = ImVec2(128.0f, 128.0f);

    enum Type {
        TYPE_PIXELMAP,
        TYPE_TABLE,
        TYPE_MATERIAL,
        TYPE_MODEL,
        TYPE_ACTOR,
        TYPE_MAX,
    };

    explicit ResourcesPane(const char *id, br_pixelmap *screen, br_registry *reg);
    ~ResourcesPane();

    void Draw();

private:
    void DrawObject(size_t id, br_pixelmap *pm, ImVec2 region);
    void DrawObject(size_t id, br_material *mat, ImVec2 region);
    void DrawObject(size_t id, br_model *mod, ImVec2 region);

    template <typename T>
    void DrawItem(size_t id, T *item)
    {
        const char *identifier;
        ImVec2      top_left;

        ImGui::PushID(static_cast<int>(id));
        ImGui::BeginChild("", ITEM_SIZE, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

        /*
         * Save the top-left position.
         */
        top_left = ImGui::GetCursorScreenPos();

        this->DrawObject(id, item, ImGui::GetContentRegionAvail());

        /*
         * Draw the name.
         */
        if((identifier = item->identifier) == NULL)
            identifier = "(unnamed)";

        ImVec2 label_size = ImGui::CalcTextSize(identifier);
        ImVec2 label_loc  = ImGui::GetCursorScreenPos();
        label_loc.y -= label_size.y;
        ImGui::SetCursorScreenPos(label_loc);
        ImGui::Text(identifier);

        /*
         * Draw the [index] marker.
         */
        ImGui::SetCursorScreenPos(top_left);
        ImGui::Text("[%zu]", id);

        ImGui::EndChild();
        ImGui::PopID();
    }

    br_res_ptr<char> m_id;

    std::vector<br_pixelmap *> m_pixelmaps;
    std::vector<br_material *> m_materials;
    std::vector<br_model *>    m_models;

    Type m_type;
    char m_searchbuf[256];

    MaterialPreview m_mat_preview;
    br_pixelmap    *m_screen;
};

} // namespace Editor

#endif /* EDITOR_RESOURCES_PANE_HPP_ */
