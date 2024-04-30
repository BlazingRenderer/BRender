/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Texture Mapped Sphere (15-bit colour)
 */
#include <brdemo.h>

typedef struct br_demo_tut6 {
    br_actor *planet;
} br_demo_tut6;

br_error Tutorial6Init(br_demo *demo)
{
    br_demo_tut6 *tut;
    br_actor     *light, *observer;
    br_camera    *camera_data;
    br_material  *planet_material;
    br_model     *planet_model;
    br_pixelmap  *earth_pm;

    if((planet_model = BrModelLoad("sph32.dat")) == NULL) {
        BrLogError("DEMO", "Unable to load sph32.dat");
        return BRE_FAIL;
    }
    BrModelAdd(planet_model);

    /*
     * Load and Register `earth' Texture
     */
    if((earth_pm = BrPixelmapLoad("earth15.pix")) == NULL) {
        BrLogError("DEMO", "Unable to load earth15.pix");
        return BRE_FAIL;
    }
    BrMapAdd(earth_pm);

    /*
     * Load and Apply `earth' Material
     */
    if((planet_material = BrFmtScriptMaterialLoad("earth.mat")) == NULL) {
        BrLogError("DEMO", "Unable to load earth.mat");
        return BRE_FAIL;
    }
    BrMaterialAdd(planet_material);

    tut = BrResAllocate(demo, sizeof(br_demo_tut6), BR_MEMORY_APPLICATION);

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
     * Load and Position Planet Actor
     */
    tut->planet           = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->planet->t.type   = BR_TRANSFORM_MATRIX34;
    tut->planet->model    = planet_model;
    tut->planet->material = BrMaterialFind("earth_map");

    BrMatrix34RotateX(&tut->planet->t.t.mat, BR_ANGLE_DEG(90));

    demo->user = tut;
    return BRE_OK;
}

void Tutorial6Update(br_demo *demo, br_scalar dt)
{
    br_demo_tut6 *tut = demo->user;

    BrMatrix34PostRotateY(&tut->planet->t.t.mat, BR_ANGLE_DEG(2.0 * dt * 25));
}

static br_demo_dispatch dispatch = {
    .init          = Tutorial6Init,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = Tutorial6Update,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRun("BRender Tutorial 6", 1280, 720, &dispatch);
}
