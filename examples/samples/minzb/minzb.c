#include <stddef.h>
#include <brdemo.h>

static br_error MinZBInit(br_demo *demo)
{
    br_actor  *a;
    br_camera *camera_data;

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

    demo->camera = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));

    camera_data              = demo->camera->type_data;
    demo->order_table->min_z = camera_data->hither_z;
    demo->order_table->max_z = camera_data->yon_z;

    BrLightEnable(BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL)));

    a                    = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    a->t.type            = BR_TRANSFORM_EULER;
    a->t.t.euler.e.order = BR_EULER_ZXY_S;
    BrVector3Set(&a->t.t.euler.t, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(-6.0));

    demo->user = a;
    return BRE_OK;
}

static void MinZBUpdate(br_demo *demo, br_scalar dt)
{
    br_actor *a = demo->user;

    /*
     * Tumble the actor around
     */
    float aa = BrScalarToFloat(BrAngleToScalar(a->t.t.euler.e.a));
    float bb = BrScalarToFloat(BrAngleToScalar(a->t.t.euler.e.a));
    float cc = BrScalarToFloat(BrAngleToScalar(a->t.t.euler.e.a));

    aa = fmodf(aa + (dt * 1) / 25.0f, 1.0f);
    bb = fmodf(bb + (dt * 2) / 25.0f, 1.0f);
    cc = fmodf(cc + (dt * 3) / 25.0f, 1.0f);

    a->t.t.euler.e.a = BrScalarToAngle(aa);
    a->t.t.euler.e.b = BrScalarToAngle(bb);
    a->t.t.euler.e.c = BrScalarToAngle(cc);
}


const static br_demo_dispatch dispatch = {
    .init          = MinZBInit,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = MinZBUpdate,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("BRender Minimal Z-Buffer Demo", &dispatch, argc, argv);
}
