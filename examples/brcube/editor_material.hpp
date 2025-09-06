#ifndef EDITOR_MATERIAL_HPP_
#define EDITOR_MATERIAL_HPP_

#include "editor_common.hpp"

namespace Editor
{

struct MaterialUser {
    MaterialUser() noexcept;

    static MaterialUser *Get(br_material *mat) noexcept;
    static MaterialUser *Ensure(br_material *mat) noexcept;

    br_pixelmap *preview;
};

} // namespace Editor

#endif /* EDITOR_MATERIAL_HPP_ */
