/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Texture Mapped Sphere (15-bit colour)
 */
#include <brdemo.h>

typedef struct br_demo_tut6 {
    br_actor *planet;
} br_demo_tut6;

static br_error Tutorial6LoadSWRes(br_demo *demo)
{
    br_pixelmap *std_pal;
    br_pixelmap *shade_tab;
    br_pixelmap *earth_pm;

    if((std_pal = BrPixelmapLoad("std.pal")) == NULL) {
        BrLogError("DEMO", "Unable to load std.pal");
        return BRE_FAIL;
    }

    if((shade_tab = BrPixelmapLoad("shade.tab")) == NULL) {
        BrLogError("DEMO", "Unable to load shade.tab");
        return BRE_FAIL;
    }
    BrTableAdd(shade_tab);

    /*
     * Load and Register `earth' Texture
     */
    if((earth_pm = BrPixelmapLoad("earth8.pix")) == NULL) {
        BrLogError("DEMO", "Unable to load earth8.pix");
        return BRE_FAIL;
    }

    /*
     * Per-pixelmap palette needed for non-indexed renderers.
     */
    earth_pm->map = std_pal;
    BrMapAdd(earth_pm);

    /*
     * Indexed targets need a palette.
     */
    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        BrPixelmapPaletteSet(demo->colour_buffer, std_pal);
    }

    return BRE_OK;
}

static br_error Tutorial6LoadHWRes(br_demo *demo)
{
    br_pixelmap *earth_pm;

    /*
     * Load and Register `earth' Texture
     */
    if((earth_pm = BrPixelmapLoad("earth15.pix")) == NULL) {
        BrLogError("DEMO", "Unable to load earth15.pix");
        return BRE_FAIL;
    }

    BrMapAdd(earth_pm);
    return BRE_OK;
}

static br_error Tutorial6Init(br_demo *demo)
{
    br_demo_tut6 *tut;
    br_actor     *light, *observer;
    br_camera    *camera_data;
    br_material  *planet_material;
    br_model     *planet_model;
    br_error      err;

    if((err = demo->hw_accel ? Tutorial6LoadHWRes(demo) : Tutorial6LoadSWRes(demo)) != BRE_OK)
        return err;

    /*
     * Load and Apply `earth' Material
     */
    if((planet_material = BrFmtScriptMaterialLoad("earth.mat")) == NULL) {
        BrLogError("DEMO", "Unable to load earth.mat");
        return BRE_FAIL;
    }
    BrMaterialAdd(planet_material);

    if((planet_model = BrModelLoad("sph32.dat")) == NULL) {
        BrLogError("DEMO", "Unable to load sph32.dat");
        return BRE_FAIL;
    }
    BrModelAdd(planet_model);

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

static void Tutorial6Update(br_demo *demo, br_scalar dt)
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
