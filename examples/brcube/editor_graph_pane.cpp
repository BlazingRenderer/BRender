#include "editor_graph_pane.hpp"

Editor::GraphPane::GraphPane(size_t state_slot)
    : m_state_slot(state_slot), m_selected(nullptr), m_selection_valid(false)
{
}

void Editor::GraphPane::Draw(br_actor *root)
{
    this->DrawGraph(root);
}

br_actor *Editor::GraphPane::GetSelection() const noexcept
{
    if(!this->m_selection_valid)
        return nullptr;

    return this->m_selected;
}

Editor::ActorState *Editor::GraphPane::ActorState(br_actor *a) const noexcept
{
    Editor::ActorUser *user = Editor::ActorUser::Get(a);
    if(user == nullptr)
        return nullptr;

    return user->slots + this->m_state_slot;
}

void Editor::GraphPane::DrawGraph(br_actor *root)
{
    /*
     * Our selection may have been deleted since last draw.
     */
    this->m_selection_valid = false;

    if(ImGui::Begin("Scene Graph##BREditor")) {
        if(root == nullptr) {
            ImGui::Text("World is NULL");
        } else {
            this->DrawActor(root);
        }
    }

    ImGui::End();

    /*
     * Our currently selected node was not seen during traversal.
     * It's probably been deleted, so unselect it.
     */
    if(!this->m_selection_valid) {
        this->m_selected        = nullptr;
        this->m_selection_valid = true;
    }
}

void Editor::GraphPane::DrawActor(br_actor *root)
{
    const char         *identifier = "";
    bool                node_open  = false;
    Editor::ActorState *state      = ActorState(root);

    if(state == nullptr) {
        root->user = BrResAllocate(root, sizeof(Editor::ActorUser), BR_MEMORY_APPLICATION);
        new (root->user) Editor::ActorUser();
        state = ActorState(root);
        BrTransformToMatrix34(&state->mat, &root->t);
    }

    if(root->parent != nullptr) {
        br_matrix34         mt;
        Editor::ActorState *parent_state = this->ActorState(root->parent);

        BrTransformToMatrix34(&mt, &root->t);
        BrMatrix34Mul(&state->mat, &mt, &parent_state->mat);

        br_vector3 world_right   = BR_VECTOR3(1, 0, 0);
        br_vector3 world_up      = BR_VECTOR3(0, 1, 0);
        br_vector3 world_forward = BR_VECTOR3(0, 0, -1);

        BrMatrix34ApplyV(&state->right, &world_right, &state->mat);
        BrVector3Normalise(&state->right, &state->right);

        BrMatrix34ApplyV(&state->up, &world_up, &state->mat);
        BrVector3Normalise(&state->up, &state->up);

        BrMatrix34ApplyV(&state->forward, &world_forward, &state->mat);
        BrVector3Normalise(&state->forward, &state->forward);
    }

    if(root->identifier != nullptr)
        identifier = root->identifier;

    ImGui::PushID(root);

    node_open = ImGui::TreeNodeEx(root, ImGuiTreeNodeFlags_OpenOnArrow, "%s (%p)", identifier, root);

    if(ImGui::IsItemClicked(0)) {
        this->m_selected        = root;
        this->m_selection_valid = true;
    } else if(root == this->m_selected) {
        this->m_selection_valid = true;
    }

    if(node_open) {
        void *_a;

        BR_FOR_SIMPLELIST(&root->children, _a) {
            br_actor *a = reinterpret_cast<br_actor *>(_a);

            this->DrawActor(a);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}
