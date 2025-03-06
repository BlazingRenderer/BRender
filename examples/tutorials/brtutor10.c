/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Revolving Texture-Mapped Fork (15-bit mode)
 */
#include <brdemo.h>

typedef struct br_demo_tut10 {
    br_actor *fork;
} br_demo_tut10;

static br_error Tutorial10LoadSWRes(br_demo *demo)
{
    br_pixelmap *std_pal;
    br_pixelmap *shade_tab;
    br_pixelmap *chrome_pm;

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
     * Load and Register `chrome' Texture.
     *
     *  convert examples/tutorials/dat/tile0011.tga  -resize 128x128\! tile128.png
     *  texconv tile128.png -n REFMAP -Q examples/tutorials/dat/std.pal -O image -o examples/tutorials/dat/tile8.pix
     */
    if((chrome_pm = BrPixelmapLoad("tile8.pix")) == NULL) {
        BrLogError("DEMO", "Unable to load tile8.pix");
        return BRE_FAIL;
    }

    /*
     * Per-pixelmap palette needed for non-indexed renderers.
     */
    chrome_pm->map = std_pal;
    BrMapAdd(chrome_pm);

    /*
     * Indexed targets need a palette.
     */
    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        BrPixelmapPaletteSet(demo->colour_buffer, std_pal);
    }

    return BRE_OK;
}

static br_error Tutorial10LoadHWRes(br_demo *demo)
{
    br_pixelmap *chrome_pm;

    /*
     * Load and Register TILE0011 Texture
     */
    if((chrome_pm = BrPixelmapLoad("tile.pix")) == NULL) {
        BrLogError("DEMO", "Unable to load tile.pix");
        return BRE_FAIL;
    }

    BrMapAdd(chrome_pm);
    return BRE_OK;
}

static br_error Tutorial10Init(br_demo *demo)
{
    br_demo_tut10 *tut;
    br_actor      *light, *observer;
    br_model      *fork_model;
    br_material   *fork_mat;
    br_camera     *camera_data;
    br_error       err;

    if((err = demo->hw_accel ? Tutorial10LoadHWRes(demo) : Tutorial10LoadSWRes(demo)) != BRE_OK)
        return err;

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

static void Tutorial10Update(br_demo *demo, br_scalar dt)
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
