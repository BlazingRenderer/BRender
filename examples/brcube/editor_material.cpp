#include "editor_material.hpp"

Editor::MaterialUser::MaterialUser() noexcept : preview(nullptr)
{
}

Editor::MaterialUser *Editor::MaterialUser::Get(br_material *mat) noexcept
{
    return reinterpret_cast<MaterialUser *>(mat->user);
}

Editor::MaterialUser *Editor::MaterialUser::Ensure(br_material *mat) noexcept
{
    if(mat->user == nullptr)
        mat->user = BrResAllocate(mat, sizeof(MaterialUser), BR_MEMORY_APPLICATION);

    return reinterpret_cast<MaterialUser *>(mat->user);
}
