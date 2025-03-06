/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to display a Planet, Moon, Satellite animation
 */

#include <stddef.h>
#include <brdemo.h>

typedef struct br_demo_tut3 {
    br_actor *planet;
    br_actor *satellite;
    br_actor *moon;
} br_demo_tut3;

static br_error Tutorial3Init(br_demo *demo)
{
    br_demo_tut3 *tut;
    br_actor     *light, *observer;
    br_camera    *camera_data;

    tut = BrResAllocate(demo, sizeof(br_demo_tut3), BR_MEMORY_APPLICATION);

    /*
     * Load and Enable Default Light Source
     */
    BrLightEnable(BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL)));

    /*
     * Load and Position Camera
     */
    observer         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    observer->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(6.0));
    demo->camera = observer;

    camera_data              = observer->type_data;
    camera_data->yon_z       = BR_SCALAR(50);
    demo->order_table->min_z = camera_data->hither_z;
    demo->order_table->max_z = camera_data->yon_z;

    /*
     * Load Planet Model
     */
    tut->planet        = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->planet->model = BrModelLoad("sph16.dat");
    BrModelAdd(tut->planet->model);

    /*
     * Load and Position Moon Model
     */
    tut->moon        = BrActorAdd(tut->planet, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->moon->model = BrModelLoad("sph8.dat");
    BrModelAdd(tut->moon->model);
    tut->moon->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Scale(&tut->moon->t.t.mat, BR_SCALAR(0.5), BR_SCALAR(0.5), BR_SCALAR(0.5));
    BrMatrix34PostTranslate(&tut->moon->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(2.0));

    /*
     * Load and Position Satellite Model
     */
    tut->satellite        = BrActorAdd(tut->moon, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->satellite->model = BrModelLoad("sph8.dat");
    BrModelAdd(tut->satellite->model);
    tut->satellite->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Scale(&tut->satellite->t.t.mat, BR_SCALAR(0.25), BR_SCALAR(0.25), BR_SCALAR(0.25));
    BrMatrix34PostTranslate(&tut->satellite->t.t.mat, BR_SCALAR(1.5), BR_SCALAR(0.0), BR_SCALAR(0.0));

    demo->user = tut;
    return BRE_OK;
}

static void Tutorial3Update(br_demo *demo, br_scalar dt)
{
    br_demo_tut3 *tut   = demo->user;
    float         speed = dt * 25;

    BrMatrix34PreRotateY(&tut->planet->t.t.mat, BR_ANGLE_DEG(1.0f * speed));
    BrMatrix34PreRotateY(&tut->satellite->t.t.mat, BR_ANGLE_DEG(4.0f * speed));
    BrMatrix34PreRotateZ(&tut->moon->t.t.mat, BR_ANGLE_DEG(1.5f * speed));
    BrMatrix34PostRotateZ(&tut->satellite->t.t.mat, BR_ANGLE_DEG(-2.5f * speed));
    BrMatrix34PostRotateY(&tut->moon->t.t.mat, BR_ANGLE_DEG(-2.0f * speed));
}

static br_demo_dispatch dispatch = {
    .init          = Tutorial3Init,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = Tutorial3Update,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRun("BRender Tutorial 3", 1280, 720, &dispatch);
}
