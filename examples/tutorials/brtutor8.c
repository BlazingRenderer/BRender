/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Texture Mapped Duck (15-bit)
 */
#include <brdemo.h>

typedef struct br_demo_tut8 {
    br_actor *duck;
} br_demo_tut8;

static br_error Tutorial8LoadSWRes(br_demo *demo)
{
    br_pixelmap *std_pal;
    br_pixelmap *shade_tab;
    br_pixelmap *gold_pm;

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
     * Load and Register `gold' Texture.
     *
     * NB: The source texture is 320x200. In order for the 15/16bpp software renderer to use it (8bpp is fine), it
     * needs to be POT.
     *  convert examples/tutorials/dat/gold.gif  -resize 256x256\! gold256.png
     *  texconv gold256.png -n gold -Q examples/tutorials/dat/std.pal -O image -o examples/tutorials/dat/gold8.pix
     */
    if((gold_pm = BrPixelmapLoad("gold8.pix")) == NULL) {
        BrLogError("DEMO", "Unable to load gold8.pix");
        return BRE_FAIL;
    }

    /*
     * Per-pixelmap palette needed for non-indexed renderers.
     */
    gold_pm->map = std_pal;
    BrMapAdd(gold_pm);

    /*
     * Indexed targets need a palette.
     */
    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        BrPixelmapPaletteSet(demo->colour_buffer, std_pal);
    }

    return BRE_OK;
}

static br_error Tutorial8LoadHWRes(br_demo *demo)
{
    br_pixelmap *gold_pm;

    /*
     * Load and Register `gold' Texture
     */
    if((gold_pm = BrPixelmapLoad("gold15.pix")) == NULL) {
        BrLogError("DEMO", "Unable to load gold15.pix");
        return BRE_FAIL;
    }

    BrMapAdd(gold_pm);
    return BRE_OK;
}

static br_error Tutorial8Init(br_demo *demo)
{
    br_demo_tut8 *tut;
    br_actor     *light, *observer;
    br_model     *duck_model;
    br_material  *gold_mat;
    br_camera    *camera_data;
    br_error      err;

    if((err = demo->hw_accel ? Tutorial8LoadHWRes(demo) : Tutorial8LoadSWRes(demo)) != BRE_OK)
        return err;

    if((gold_mat = BrFmtScriptMaterialLoad("gold.mat")) == NULL) {
        BrLogError("DEMO", "Unable to load gold.mat");
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
    tut->duck->material = BrMaterialFind("gold");
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
