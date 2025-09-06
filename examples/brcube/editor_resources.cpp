#include <brddi.h>
#include <brglrend.h>
#include "editor_resources.hpp"

Editor::Resources::Resources() : m_logo(nullptr)
{
}

Editor::Resources::~Resources()
{
}

void Editor::Resources::Init()
{
    this->m_logo = BrPixelmapLoad("/home/zane/Documents/Coding/CrocDE-BRender/examples/brcube/dat/breditor_logo.pix");
    BrMapAdd(this->m_logo);
}

br_pixelmap *Editor::Resources::GetLogo() const noexcept
{
    return this->m_logo;
}

GLuint Editor::GetPixelmapTexture(br_pixelmap *pm)
{
    GLuint tex = 0;

    if(pm == nullptr)
        return 0;

    ObjectQuery(pm, &tex, BRT_OPENGL_TEXTURE_U32);
    if(tex != 0)
        return tex;

    if(pm->stored == nullptr)
        return 0;

    ObjectQuery(pm->stored, &tex, BRT_OPENGL_TEXTURE_U32);
    if(tex != 0)
        return tex;

    return 0;
}
