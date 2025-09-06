#ifndef EDITOR_INSPECTOR_HPP_
#define EDITOR_INSPECTOR_HPP_

#include "editor_common.hpp"

namespace Editor
{

class InspectorPane
{
public:
    void Draw(br_actor *root, size_t slot);
};

} // namespace Editor

#endif /* EDITOR_INSPECTOR_HPP_ */
