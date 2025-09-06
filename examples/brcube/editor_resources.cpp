#include <brddi.h>
#include <brglrend.h>
#include "editor_common.hpp"

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
