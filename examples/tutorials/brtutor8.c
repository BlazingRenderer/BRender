/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Texture Mapped Duck (15-bit)
 */
#include <brdemo.h>

typedef struct br_demo_tut8 {
    br_actor *duck;
} br_demo_tut8;

static br_error Tutorial8Init(br_demo *demo)
{
    br_demo_tut8 *tut;
    br_actor     *light, *observer;
    br_model     *duck_model;
    br_material  *gold_mat;
    br_camera    *camera_data;
    br_pixelmap  *gold_pm;

    if((gold_pm = BrPixelmapLoad("gold15.pix")) == NULL) {
        BrLogError("DEMO", "Unable to load gold15.pix");
        return BRE_FAIL;
    }
    BrMapAdd(gold_pm);

    if((gold_mat = BrFmtScriptMaterialLoad("gold15.mat")) == NULL) {
        BrLogError("DEMO", "Unable to load gold15.mat");
        return BRE_FAIL;
    }
    BrMaterialAdd(gold_mat);

    if((duck_model = BrModelLoad("duck.dat")) == NULL) {
        BrLogError("DEMO", "Error loading duck.dat.");
        return BRE_FAIL;
    }
    BrModelApplyMap(duck_model, BR_APPLYMAP_PLANE, NULL);
    BrModelAdd(duck_model);

    tut = BrResAllocate(demo, sizeof(br_demo_tut8), BR_MEMORY_APPLICATION);

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
     * Load and Position Duck Actor
     */
    tut->duck           = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->duck->model    = duck_model;
    tut->duck->material = BrMaterialFind("gold15");
    tut->duck->t.type   = BR_TRANSFORM_MATRIX34;
    BrMatrix34RotateX(&tut->duck->t.t.mat, BR_ANGLE_DEG(30));

    demo->user = tut;
    return BRE_OK;
}

static void Tutorial8Update(br_demo *demo, br_scalar dt)
{
    br_demo_tut8 *tut = demo->user;

    BrMatrix34PostRotateX(&tut->duck->t.t.mat, BR_ANGLE_DEG(2.0 * dt * 25));
}

static br_demo_dispatch dispatch = {
    .init          = Tutorial8Init,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = Tutorial8Update,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRun("BRender Tutorial 8", 1280, 720, &dispatch);
}
