/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to display a Planet-Satellite animation
 */
#include <brdemo.h>

typedef struct br_demo_tut4 {
    br_actor *planet;
    br_actor *sat;
} br_demo_tut4;

static br_error Tutorial4Init(br_demo *demo)
{
    br_demo_tut4 *tut;
    br_actor     *light, *observer, *wings1, *wings2;
    br_camera    *camera_data;

    tut = BrResAllocate(demo, sizeof(br_demo_tut4), BR_MEMORY_APPLICATION);

    /*
     * If indexed, load and set the palette.
     */
    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        br_pixelmap *std_pal;

        if((std_pal = BrPixelmapLoad("std.pal")) == NULL) {
            BrLogError("DEMO", "Unable to load std.pal");
            return BRE_FAIL;
        }

        BrPixelmapPaletteSet(demo->colour_buffer, std_pal);
    }

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
    BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(8.0));
    demo->camera = observer;

    camera_data              = observer->type_data;
    camera_data->yon_z       = BR_SCALAR(350);
    camera_data->hither_z    = BR_SCALAR(0.5);
    demo->order_table->min_z = camera_data->hither_z;
    demo->order_table->max_z = camera_data->yon_z;

    /*
     * Load and Position Planet Actor
     */
    tut->planet        = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->planet->model = BrModelLoad("sph32.dat");
    BrModelAdd(tut->planet->model);
    tut->planet->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&tut->planet->t.t.mat, BR_SCALAR(14.0), BR_SCALAR(14.0), BR_SCALAR(-40.0));

    /*
     * Load and Position Satellite
     */
    tut->sat        = BrActorAdd(tut->planet, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->sat->model = BrModelLoad("sph16.dat");
    BrModelAdd(tut->sat->model);
    tut->sat->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Scale(&tut->sat->t.t.mat, BR_SCALAR(0.5), BR_SCALAR(0.5), BR_SCALAR(0.5));
    BrMatrix34PostTranslate(&tut->sat->t.t.mat, BR_SCALAR(2.0), BR_SCALAR(0.0), BR_SCALAR(0.0));

    /*
     * Add `wings' to Satellite
     */
    wings1        = BrActorAdd(tut->sat, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    wings1->model = BrModelLoad("cylinder.dat");
    BrModelAdd(wings1->model);
    wings1->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Scale(&wings1->t.t.mat, BR_SCALAR(0.25), BR_SCALAR(0.25), BR_SCALAR(2.0));

    /*
     * Add more `wings' to Satellite
     */
    wings2        = BrActorAdd(tut->sat, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    wings2->model = BrModelLoad("cylinder.dat");
    BrModelAdd(wings2->model);
    wings2->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Scale(&wings2->t.t.mat, BR_SCALAR(0.25), BR_SCALAR(0.25), BR_SCALAR(2.0));
    BrMatrix34PostRotateY(&wings2->t.t.mat, BR_ANGLE_DEG(90.0));

    demo->user = tut;
    return BRE_OK;
}

static void Tutorial4Update(br_demo *demo, br_scalar dt)
{
    br_demo_tut4 *tut   = demo->user;
    float         speed = dt * 25;

    BrMatrix34PostTranslate(&tut->planet->t.t.mat, BR_SCALAR(-0.033 * speed), BR_SCALAR(-0.032 * speed),
                            BR_SCALAR(0.1 * speed));
    BrMatrix34PreRotateY(&tut->planet->t.t.mat, BR_ANGLE_DEG(1.0f * speed));
    BrMatrix34PreRotateX(&tut->sat->t.t.mat, BR_ANGLE_DEG(15.0f * speed));
    BrMatrix34PreRotateY(&tut->sat->t.t.mat, BR_ANGLE_DEG(10.0f * speed));
    BrMatrix34PostRotateZ(&tut->sat->t.t.mat, BR_ANGLE_DEG(1.0f * speed));
    BrMatrix34PostRotateY(&tut->sat->t.t.mat, BR_ANGLE_DEG(3.0f * speed));
}

static br_demo_dispatch dispatch = {
    .init          = Tutorial4Init,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = Tutorial4Update,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("BRender Tutorial 4", &dispatch, argc, argv);
}
