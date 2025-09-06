#include "editor_log_pane.hpp"

Editor::LogPane::LogPane(const char *id) : m_source(nullptr)
{
    this->m_id.reset(BrResSprintf(nullptr, "Log Messages##%s", id));
}

void Editor::LogPane::SetSource(const std::vector<LogEntry> *source)
{
    this->m_source = source;
}

static const char *LevelToString(br_uint_8 level)
{
    switch(level) {
        case BR_LOG_ERROR:
            return "Error";
        case BR_LOG_WARN:
            return "Warning";
        case BR_LOG_INFO:
            return "Info";
        case BR_LOG_DEBUG:
            return "Debug";
        case BR_LOG_TRACE:
            return "Trace";
        default:
            return "";
    }
}

void Editor::LogPane::Draw()
{
    ImGui::Begin(this->m_id.get());

    ImGui::PushItemWidth(-1);

    ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
                            ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
    if(ImGui::BeginTable("##Messages", 4, flags)) {
        size_t n_entries = this->m_source != nullptr ? this->m_source->size() : 0;

        ImGui::TableSetupColumn("Timestamp");
        ImGui::TableSetupColumn("Level");
        ImGui::TableSetupColumn("Component");
        ImGui::TableSetupColumn("Message");
        ImGui::TableHeadersRow();

        for(size_t i = 0; i < n_entries; ++i) {
            const LogEntry& e = this->m_source->at(i);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            /* FIXME: Get proper iso8601 output. */
            auto xx = std::chrono::duration_cast<std::chrono::microseconds>(e.timestamp.time_since_epoch()).count();
            ImGui::Text("%lld", xx);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s\n", LevelToString(e.level));
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s\n", e.component.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s\n", e.message.c_str());
        }

        ImGui::EndTable();
    }
    ImGui::PopItemWidth();

    ImGui::End();
}