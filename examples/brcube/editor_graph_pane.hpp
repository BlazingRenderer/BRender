#ifndef EDITOR_GRAPH_HPP_
#define EDITOR_GRAPH_HPP_

#include "editor_common.hpp"
#include "editor_actor.hpp"

namespace Editor
{

class GraphPane
{
public:
    explicit GraphPane(size_t state_slot);

    void Draw(br_actor *root);

    [[nodiscard]] br_actor *GetSelection() const noexcept;

    Editor::ActorState *ActorState(br_actor *a) const noexcept;

private:
    void DrawGraph(br_actor *root);
    void DrawActor(br_actor *root);

    size_t    m_state_slot;
    br_actor *m_selected;
    bool      m_selection_valid;
};

} // namespace Editor

#endif /* EDITOR_GRAPH_HPP_ */
