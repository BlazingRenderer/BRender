/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Revolving Illuminated Blue Cube.
 */
#include <brdemo.h>

typedef struct br_demo_tut5 {
    br_actor *cube;
} br_demo_tut5;

br_error Tutorial5Init(br_demo *demo)
{
    br_demo_tut5 *tut;
    br_actor     *light, *observer, *wings1, *wings2;
    br_camera    *camera_data;
    br_material  *cube_material;

    tut = BrResAllocate(demo, sizeof(br_demo_tut5), BR_MEMORY_APPLICATION);

    /*
     * Add and enable the default light source
     */
    light = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    BrLightEnable(light);

    /*
     * Load and Position Camera
     */
    observer         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    observer->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(5.0));
    demo->camera = observer;

    camera_data              = observer->type_data;
    demo->order_table->min_z = camera_data->hither_z;
    demo->order_table->max_z = camera_data->yon_z;

    /*
     * Load and Position Cube Model
     */
    tut->cube         = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->cube->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34RotateY(&tut->cube->t.t.mat, BR_ANGLE_DEG(30));

    /*
     * Load and Apply Cube's Material
     */
    cube_material = BrFmtScriptMaterialLoad("cube.mat");
    BrMaterialAdd(cube_material);
    tut->cube->material = BrMaterialFind("BLUE MATERIAL");

    demo->user = tut;
    return BRE_OK;
}

void Tutorial5Update(br_demo *demo, br_scalar dt)
{
    br_demo_tut5 *tut = demo->user;

    BrMatrix34PostRotateX(&tut->cube->t.t.mat, BR_ANGLE_DEG(2.0 * dt * 25));
}

static br_demo_dispatch dispatch = {
    .init          = Tutorial5Init,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = Tutorial5Update,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRun("BRender Tutorial 5", 1280, 720, &dispatch);
}
