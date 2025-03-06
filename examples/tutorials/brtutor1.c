/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Revolving Illuminated Cube.
 */

#include <brdemo.h>

typedef struct br_demo_tut1 {
    br_actor *cube;
} br_demo_tut1;

static br_error Tutorial1Init(br_demo *demo)
{
    br_demo_tut1 *tut;
    br_actor     *light, *observer;
    br_camera    *camera_data;

    tut = BrResAllocate(demo, sizeof(br_demo_tut1), BR_MEMORY_APPLICATION);

    /*
     * Add and enable the default light source
     */
    light = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    BrLightEnable(light);

    /*
     * Move camera 5 units along +z axis so model becomes visible
     */
    observer         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    observer->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(5.0));
    demo->camera = observer;

    camera_data              = observer->type_data;
    demo->order_table->min_z = camera_data->hither_z;
    demo->order_table->max_z = camera_data->yon_z;

    /*
     * Add a model actor, the default cube, and rotate it to enhance visibility.
     */
    tut->cube         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->cube->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34RotateY(&tut->cube->t.t.mat, BR_ANGLE_DEG(30));

    demo->user = tut;
    return BRE_OK;
}

static void Tutorial1Update(br_demo *demo, br_scalar dt)
{
    br_demo_tut1 *tut = demo->user;

    /*
     * Rotate cube around x-axis
     */
    BrMatrix34PostRotateX(&tut->cube->t.t.mat, BR_ANGLE_DEG(2.0f * dt * 25));
}

static br_demo_dispatch dispatch = {
    .init          = Tutorial1Init,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = Tutorial1Update,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRun("BRender Tutorial 1", 1280, 720, &dispatch);
}
