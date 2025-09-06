#ifndef EDITOR_CAMERA_PANE_HPP_
#define EDITOR_CAMERA_PANE_HPP_

#include <string_view>
#include "editor_common.hpp"
#include "editorcam.h"

namespace Editor
{

class CameraPane
{
public:
    explicit CameraPane(const char *id, br_pixelmap *screen);
    ~CameraPane();

    [[nodiscard]] br_editor_camera *GetCamera() const noexcept;

    [[nodiscard]] bool IsFocused() const noexcept;

    void Update(float dt) noexcept;
    void Draw(br_actor *world);

private:
    void DrawInternal(br_actor *world);
    void MaybeResize(float width, float height) noexcept;

    static void CheckInput(br_editor_camera *cam) noexcept;

    br_res_ptr<char>  m_id;
    br_pixelmap      *m_screen;
    br_editor_camera *m_camera;
    bool              m_is_focused;

    br_pixelmap_ptr m_depth_buffer;
    br_pixelmap_ptr m_colour_buffer;
};

} // namespace Editor

#endif /* EDITOR_CAMERA_PANE_HPP_ */
