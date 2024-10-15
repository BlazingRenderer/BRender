/*
 * Editor/Unity-like Camera for BRender
 * https://github.com/BlazingRenderer/BRender
 *
 * SPDX-License-Identifier: MIT
 * Copyright(c) 2025 Zane van Iperen
 */
#ifndef EDITORCAM_H_
#define EDITORCAM_H_

#include <brender.h>

enum {
    BR_ECAMF_NONE         = 0 << 0,
    BR_ECAMF_MOVE_FORWARD = 1 << 0,
    BR_ECAMF_MOVE_BACK    = 1 << 1,
    BR_ECAMF_MOVE_LEFT    = 1 << 2,
    BR_ECAMF_MOVE_RIGHT   = 1 << 3,
    BR_ECAMF_MOVE_UP      = 1 << 4,
    BR_ECAMF_MOVE_DOWN    = 1 << 5,
    BR_ECAMF_FASTER       = 1 << 6,

    BR_ECAMF_MODE_LOOK = 1 << 7,
    BR_ECAMF_MODE_PAN  = 1 << 8,
};

typedef struct br_editor_camera {
    /*
     * The camera's top-level actor.
     */
    br_actor *actor;

    /*
     * The actor holding the actual camera.
     */
    br_actor *camera;

    /*
     * The camera data.
     */
    br_camera camera_data;

    br_uint_32 flags;

    br_scalar current_speed;
    br_scalar standard_speed;
    br_scalar speed_multiplier;

    br_uint_16 viewport_width;
    br_uint_16 viewport_height;

    br_scalar mx_rel;
    br_scalar my_rel;
    br_scalar mwheel_rel;

} br_editor_camera;

#include "editorcam_p.h"

#endif /* EDITORCAM_H_ */
