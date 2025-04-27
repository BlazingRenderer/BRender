#include <brddi.h>

#include "editor_config.hpp"

br_token Editor::BRT_EDITOR_NUM_CAMERA_PANES_U32 = BR_NULL_TOKEN;

void Editor::RegisterTokens()
{
    BRT_EDITOR_NUM_CAMERA_PANES_U32 = BrTokenCreate("EDITOR_NUM_CAMERA_PANES_U32", BR_NULL_TOKEN);
}

Editor::Config::Config() noexcept : num_camera_panes(1)
{
    RegisterTokens();

    m_config.reset(BrAssociativeArrayAllocate());
}

br_error Editor::Config::Read(const char *filename) noexcept
{
    br_error err;

    if((err = BrReadIniSection(this->m_config.get(), filename, "Editor")) != BRE_OK)
        return err;

    this->ReadValue(BRT_EDITOR_NUM_CAMERA_PANES_U32, [this](const br_value *tv) { this->num_camera_panes = tv->u32; });

    return BRE_OK;
}

br_error Editor::Config::WriteValue(br_token t, br_value v) noexcept
{
    return BrAssociativeArraySet(this->m_config.get(), t, v);
}

br_error Editor::Config::WriteValue(br_token t, br_uint_32 v) noexcept
{
    br_value vv = {.u32 = v};
    return this->WriteValue(t, vv);
}

br_error Editor::Config::Write(const char *filename) noexcept
{
    br_error err;

    if((err = this->WriteValue(BRT_EDITOR_NUM_CAMERA_PANES_U32, this->num_camera_panes)) != BRE_OK)
        return err;

    return BrWriteIniSection(this->m_config.get(), filename, "Editor");
}
