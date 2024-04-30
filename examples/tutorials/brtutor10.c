/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Revolving Texture-Mapped Fork (15-bit mode)
 */
#include <brdemo.h>

typedef struct br_demo_tut10 {
    br_actor *fork;
} br_demo_tut10;

br_error Tutorial10Init(br_demo *demo)
{
    br_demo_tut10 *tut;
    br_actor      *light, *observer;
    br_model      *fork_model;
    br_material   *fork_mat;
    br_camera     *camera_data;
    br_pixelmap   *tile_pm;

    /*
     * Load and Register TILE0011 Texture
     */
    if((tile_pm = BrPixelmapLoad("refmap.pix")) == NULL) {
        BrLogError("DEMO", "Unable to load refmap.pix");
        return BRE_FAIL;
    }
    BrMapAdd(tile_pm);

    /*
     * Load and Apply `fork' Material
     */
    if((fork_mat = BrFmtScriptMaterialLoad("fork.mat")) == NULL) {
        BrLogError("DEMO", "Unable to load fork.mat");
        return BRE_FAIL;
    }
    BrMaterialAdd(fork_mat);

    /*
     * Load and Position Fork Actor
     */
    if((fork_model = BrModelLoad("fork.dat")) == NULL) {
        BrLogError("DEMO", "Error loading fork.dat.");
        return BRE_FAIL;
    }
    BrModelApplyMap(fork_model, BR_APPLYMAP_PLANE, NULL);
    BrModelAdd(fork_model);

    tut = BrResAllocate(demo, sizeof(br_demo_tut10), BR_MEMORY_APPLICATION);

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
     * Load and Position Fork Actor
     */
    tut->fork           = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    tut->fork->model    = fork_model;
    tut->fork->material = BrMaterialFind("CHROME GIFMAP");
    tut->fork->t.type   = BR_TRANSFORM_MATRIX34;

    demo->user = tut;
    return BRE_OK;
}

void Tutorial10Update(br_demo *demo, br_scalar dt)
{
    br_demo_tut10 *tut = demo->user;

    BrMatrix34PostRotateY(&tut->fork->t.t.mat, BR_ANGLE_DEG(2.0 * dt * 25));
}

static br_demo_dispatch dispatch = {
    .init          = Tutorial10Init,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = Tutorial10Update,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRun("BRender Tutorial 10", 1280, 720, &dispatch);
}
