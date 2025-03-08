#include <stddef.h>
#include <brdemo.h>

static br_error MinZSInit(br_demo *demo)
{
    br_actor  *a;
    br_camera *camera_data;

    /*
     * The demo framework creates a depth buffer, but we don't want it.
     */
    BrPixelmapFree(demo->depth_buffer);
    demo->depth_buffer = NULL;

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

static void MinZSUpdate(br_demo *demo, br_scalar dt)
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

static void MinZSRender(br_demo *demo)
{
    BrPixelmapFill(demo->colour_buffer, 0);

    /*
     * Do a Z-Sort render (no depth buffer).
     */
    BrRendererFrameBegin();
    BrZsSceneRender(demo->world, demo->camera, demo->colour_buffer);
    BrRendererFrameEnd();
}

const static br_demo_dispatch dispatch = {
    .init          = MinZSInit,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = MinZSUpdate,
    .render        = MinZSRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("BRender Minimal Z-Sort Demo", &dispatch, argc, argv);
}
