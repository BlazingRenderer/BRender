/*
 * Editor/Unity-like Camera for BRender
 * https://github.com/BlazingRenderer/BRender
 *
 * SPDX-License-Identifier: MIT
 * Copyright(c) 2025 Zane van Iperen
 */
#include "editorcam.h"

// clang-format off
const static br_editor_camera default_camera = {
    .actor             = NULL,
    .camera            = NULL,
    .camera_data       = (br_camera){
        .type          = BR_CAMERA_PERSPECTIVE,
        .field_of_view = BR_ANGLE_DEG(60),
        .hither_z      = BR_SCALAR(0.01f),
        .yon_z         = BR_SCALAR(1000.0f),
        .aspect        = BR_SCALAR(1.0f),
    },
    .flags             = BR_ECAMF_NONE,
    .current_speed     = 0,
    .standard_speed    = BR_SCALAR(50),
    .speed_multiplier  = BR_SCALAR(1.5),

    .viewport_width    = 1000,
    .viewport_height   = 1000,
    .mx_rel            = 0,
    .my_rel            = 0,
    .mwheel_rel        = 0,
};
// clang-format on

br_editor_camera *BrEditorCamAllocate(void *res)
{
    br_editor_camera *cam;

    cam  = BrResAllocate(res, sizeof(br_editor_camera), BR_MEMORY_APPLICATION);
    *cam = default_camera;

    cam->actor             = BrActorAllocate(BR_ACTOR_NONE, NULL);
    cam->actor->identifier = BrResStrDup(cam->actor, "CamTop");
    cam->actor->t.type     = BR_TRANSFORM_MATRIX34;

    cam->camera             = BrActorAdd(cam->actor, BrActorAllocate(BR_ACTOR_CAMERA, &cam->camera_data));
    cam->camera->identifier = BrResStrDup(cam->camera, "CamCam");
    cam->camera->t.type     = BR_TRANSFORM_MATRIX34;
    return cam;
}

void BrEditorCamSetViewportSize(br_editor_camera *cam, br_uint_16 width, br_uint_16 height)
{
    if(width == 0)
        width = 1;

    if(height == 0)
        height = 1;

    cam->viewport_width  = width;
    cam->viewport_height = height;

    cam->camera_data.aspect = BR_DIV(BR_SCALAR(width), BR_SCALAR(height));
}

static br_vector3 accumulate_keys(br_uint_32 flags)
{
    br_vector3 v = BR_VECTOR3(0, 0, 0);

    /* Accumulate our keys. */
    if(flags & BR_ECAMF_MOVE_FORWARD)
        v.v[2] = BR_ADD(v.v[2], BR_SCALAR(-1));

    if(flags & BR_ECAMF_MOVE_BACK)
        v.v[2] = BR_ADD(v.v[2], BR_SCALAR(1));

    if(flags & BR_ECAMF_MOVE_LEFT)
        v.v[0] = BR_ADD(v.v[0], BR_SCALAR(-1));

    if(flags & BR_ECAMF_MOVE_RIGHT)
        v.v[0] = BR_ADD(v.v[0], BR_SCALAR(1));

    if(flags & BR_ECAMF_MOVE_UP)
        v.v[1] = BR_ADD(v.v[1], BR_SCALAR(1));

    if(flags & BR_ECAMF_MOVE_DOWN)
        v.v[1] = BR_ADD(v.v[1], BR_SCALAR(-1));

    return v;
}

void BrEditorCamUpdate(br_editor_camera *cam, float dt)
{
    br_scalar xdiff      = BR_DIV(BR_SCALAR(cam->mx_rel), cam->viewport_width) * dt;
    br_scalar ydiff      = BR_DIV(BR_SCALAR(cam->my_rel), cam->viewport_height) * dt;
    br_scalar wheel_diff = cam->mwheel_rel * dt;

    br_matrix34 camera_to_world;
    // fprintf(stderr, "xrel = %d, yrel = %d, wdiff = %f\n", xrel, yrel, wheel_diff);

    /* Calculate our matrix. */
    BrMatrix34Identity(&camera_to_world);
    BrMatrix34PreTransform(&camera_to_world, &cam->actor->t);
    BrMatrix34PreTransform(&camera_to_world, &cam->camera->t);

    if(cam->flags & BR_ECAMF_MODE_LOOK) {
        br_boolean has_speed = BR_FALSE;
        br_vector3 world_v   = BR_VECTOR3(0, 0, 0);
        br_vector3 v         = accumulate_keys(cam->flags);

        has_speed = BrVector3Normalise0(&v, &v);

        if(has_speed && (cam->flags & BR_ECAMF_FASTER)) {
            cam->current_speed += (cam->speed_multiplier * cam->standard_speed * dt);
        } else {
            cam->current_speed = cam->standard_speed;
        }

        /* Rotate */
        BrMatrix34PostRotateY(&cam->camera->t.t.mat, BR_ANGLE_RAD(-xdiff) * 1000);
        BrMatrix34PreRotateX(&cam->camera->t.t.mat, BR_ANGLE_RAD(-ydiff) * 1000);

        /* Translate */
        BrVector3Scale(&v, &v, cam->current_speed * dt);
        BrMatrix34ApplyV(&world_v, &v, &camera_to_world);
        BrMatrix34PreTranslate(&cam->actor->t.t.mat, world_v.v[0], world_v.v[1], world_v.v[2]);
    } else if(cam->flags & BR_ECAMF_MODE_PAN) {
        /* Pan */
        BrMatrix34PreTranslate(&cam->camera->t.t.mat, xdiff * -1000, ydiff * 1000, 0);
    } else {
        cam->current_speed = BR_SCALAR(0);
    }

    /* Scroll - discrete steps, don't dt multiply. */
    if(BR_ABS(cam->mwheel_rel) > 0.0f) {
        br_vector3 forward = BR_VECTOR3(0, 0, wheel_diff * -500);
        br_vector3 world_forward;
        BrMatrix34ApplyV(&world_forward, &forward, &camera_to_world);
        BrMatrix34PreTranslate(&cam->actor->t.t.mat, world_forward.v[0], world_forward.v[1], world_forward.v[2]);
    }

    cam->mx_rel     = 0;
    cam->my_rel     = 0;
    cam->mwheel_rel = 0;
}

void BrEditorCamReset(br_editor_camera *cam)
{
    BrMatrix34Identity(&cam->actor->t.t.mat);
    BrMatrix34Identity(&cam->camera->t.t.mat);
}

void BrEditorCamFree(br_editor_camera *cam)
{
    if(cam->camera->prev != NULL)
        BrActorRemove(cam->camera);

    BrActorFree(cam->camera);

    if(cam->actor->prev != NULL)
        BrActorRemove(cam->actor);

    BrActorFree(cam->actor);

    BrResFree(cam);
}
