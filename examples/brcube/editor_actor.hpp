#ifndef EDITOR_ACTOR_HPP_
#define EDITOR_ACTOR_HPP_

#include "editor_common.hpp"

namespace Editor
{

struct ActorState {
    ActorState() noexcept;

    br_matrix34 mat;
    br_vector3  forward;
    br_vector3  up;
    br_vector3  right;
};

struct ActorUser {
    const static constexpr size_t NUM_SLOTS = 4;

    ActorUser() noexcept;

    ActorState slots[NUM_SLOTS];

    static ActorUser *Get(br_actor *actor) noexcept;
};

} // namespace Editor

#endif /* EDITOR_ACTOR_HPP_ */
