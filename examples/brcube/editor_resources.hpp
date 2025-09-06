#ifndef EDITOR_RESOURCE_HPP_
#define EDITOR_RESOURCE_HPP_

#include "editor_common.hpp"

namespace Editor
{

class Resources
{
public:
    Resources();
    ~Resources();

    void Init();

    [[nodiscard]] br_pixelmap *GetLogo() const noexcept;

private:
    br_pixelmap *m_logo;
};

} // namespace Editor

#endif /* EDITOR_RESOURCE_HPP_ */
