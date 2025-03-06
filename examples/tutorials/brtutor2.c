/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to display a scene containing a Box, a Sphere and a Torus
 */
#include <brdemo.h>

typedef struct br_demo_tut2 {
    br_actor *box;
    br_actor *torus;
    br_actor *sphere;
} br_demo_tut2;

static br_error Tutorial2Init(br_demo *demo)
{
    br_demo_tut2 *tut;
    br_actor     *light, *observer;
    br_camera    *camera_data;

    tut = BrResAllocate(demo, sizeof(br_demo_tut2), BR_MEMORY_APPLICATION);

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
    BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(10.0));
    demo->camera = observer;

    camera_data              = observer->type_data;
    camera_data->yon_z       = BR_SCALAR(50);
    demo->order_table->min_z = camera_data->hither_z;
    demo->order_table->max_z = camera_data->yon_z;

    /*
     * Load and position Box model
     */
    tut->box         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->box->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34RotateY(&tut->box->t.t.mat, BR_ANGLE_DEG(30));
    BrMatrix34PostTranslate(&tut->box->t.t.mat, BR_SCALAR(-2.5), BR_SCALAR(0.0), BR_SCALAR(0.0));
    BrMatrix34PreScale(&tut->box->t.t.mat, BR_SCALAR(2.0), BR_SCALAR(1.0), BR_SCALAR(1.0));

    /*
     * Load and Position Sphere Model
     */
    tut->sphere        = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->sphere->model = BrModelLoad("sph32.dat");
    BrModelAdd(tut->sphere->model);
    tut->sphere->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&tut->sphere->t.t.mat, BR_SCALAR(2.0), BR_SCALAR(0.0), BR_SCALAR(0.0));

    /*
     * Load and Position Torus Model
     */
    tut->torus        = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->torus->model = BrModelLoad("torus.dat");
    BrModelAdd(tut->torus->model);
    tut->torus->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&tut->torus->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(3.0));

    demo->user = tut;
    return BRE_OK;
}

static void Tutorial2Update(br_demo *demo, br_scalar dt)
{
    br_demo_tut2 *tut   = demo->user;
    float         speed = dt * 25;

    BrMatrix34PostRotateX(&tut->box->t.t.mat, BR_ANGLE_DEG(2.0f * speed));
    BrMatrix34PreRotateZ(&tut->torus->t.t.mat, BR_ANGLE_DEG(4.0f * speed));
    BrMatrix34PreRotateY(&tut->torus->t.t.mat, BR_ANGLE_DEG(-6.0f * speed));
    BrMatrix34PreRotateX(&tut->torus->t.t.mat, BR_ANGLE_DEG(2.0f * speed));
    BrMatrix34PostRotateX(&tut->torus->t.t.mat, BR_ANGLE_DEG(1.0f * speed));
    BrMatrix34PostRotateY(&tut->sphere->t.t.mat, BR_ANGLE_DEG(0.8f * speed));
}

static br_demo_dispatch dispatch = {
    .init          = Tutorial2Init,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = Tutorial2Update,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRun("BRender Tutorial 2", 1280, 720, &dispatch);
}
