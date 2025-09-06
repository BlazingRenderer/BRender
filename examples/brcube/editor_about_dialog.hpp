#ifndef EDITOR_ABOUT_DIALOG_HPP_
#define EDITOR_ABOUT_DIALOG_HPP_
#include <vector>
#include <string>
#include "editor_common.hpp"

namespace Editor
{

class AboutDialog
{
public:
    explicit AboutDialog(const char *id);

    [[nodiscard]] bool IsVisible() const noexcept;
    bool               SetVisible(bool visible) noexcept;

    void Refresh(br_pixelmap *screen, br_registry *reg);
    void Draw();

private:
    void DrawHeader();

    br_res_ptr<char> m_id;
    br_pixelmap     *m_screen;
    bool             m_visible;

    br_pixelmap *m_pm_logo;

    br_res_ptr<void> m_gl_buffer;
    std::string      m_gl_version;
    std::string      m_gl_renderer;
    std::string      m_gl_vendor;
    const char     **m_gl_extensions;
    int              m_gl_num_extensions;
    size_t           m_gl_current_extension;

};

} // namespace Editor

#endif /* EDITOR_ABOUT_DIALOG_HPP_ */
