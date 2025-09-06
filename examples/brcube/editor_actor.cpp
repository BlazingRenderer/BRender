#include "editor_actor.hpp"

Editor::ActorState::ActorState() noexcept : mat(), forward{}, up{}, right{}
{
    BrMatrix34Identity(&this->mat);
    BrVector3SetFloat(&this->forward, 0, 0, -1);
    BrVector3SetFloat(&this->up, 0, 1, 0);
    BrVector3SetFloat(&this->right, 1, 0, 0);
}

Editor::ActorUser::ActorUser() noexcept : slots()
{
}

Editor::ActorUser *Editor::ActorUser::Get(br_actor *actor) noexcept
{
    return reinterpret_cast<Editor::ActorUser *>(actor->user);
}
