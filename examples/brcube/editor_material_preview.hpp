#ifndef EDITOR_MATERIAL_PREVIEW_HPP_
#define EDITOR_MATERIAL_PREVIEW_HPP_

#include "editor_common.hpp"

namespace Editor
{
class MaterialPreview
{
public:
    explicit MaterialPreview(br_pixelmap *screen, br_registry *reg) noexcept;

    void Draw(br_pixelmap *rt, br_material *mat) noexcept;

private:
    br_actor_ptr    m_root;
    br_actor       *m_sphere;
    br_actor       *m_light;
    br_pixelmap_ptr m_colour_buffer;
    br_pixelmap_ptr m_depth_buffer;
};

} // namespace Editor

#endif /* EDITOR_MATERIAL_PREVIEW_HPP_ */
