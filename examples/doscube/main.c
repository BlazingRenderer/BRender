#include <inttypes.h>
#include <brender.h>
#include <brddi.h>
#include <priminfo.h>
#include <stdio.h>
#include <time.h>
#include <pc.h>

void BR_CALLBACK _BrBeginHook(void)
{
    struct br_device *BR_EXPORT BrDrv1SoftPrimBegin(const char *arguments);
    struct br_device *BR_EXPORT BrDrv1SoftRendBegin(const char *arguments);
    struct br_device *BR_EXPORT BrDrv1MCGABegin(const char *arguments);

    BrDevAddStatic(NULL, BrDrv1SoftPrimBegin, NULL);
    BrDevAddStatic(NULL, BrDrv1SoftRendBegin, NULL);
    BrDevAddStatic(NULL, BrDrv1MCGABegin, NULL);
}

void BR_CALLBACK _BrEndHook(void)
{
}

static char primitive_heap[1500 * 1024];

static void draw_info(br_pixelmap *screen, br_material *mat, br_colour text_colour)
{
    br_token   pstate_token = BR_NULL_TOKEN;
    br_object *primstate    = NULL;
    br_uint_16 font_height  = 0;
    brp_block *block;

    if((pstate_token = BrTokenFind("PRIMITIVE_STATE_O")) == BR_NULL_TOKEN)
        return;

    // ObjectQuery(BrV1dbRendererQuery(), &primstate, pstate_token);
    ObjectQuery(mat->stored, &primstate, pstate_token);
    if(primstate == NULL)
        return;

    ObjectQuery(primstate, &block, BRT_PRIMITIVE_BLOCK_P);
    if(block == NULL)
        return;

    font_height = BrPixelmapTextHeight(screen, BrFontProp7x9);

    BrPixelmapTextF(screen, -(screen->width / 2) + font_height, -(screen->height / 2) + font_height, text_colour, BrFontProp7x9,
                    "Rasteriser: %s", block->identifier);
}

#define PALETTE_NAME "std.pal"
#define CLEAR_COLOUR 16
#define TEXT_COLOUR  63

// #define PALETTE_NAME "vga.pal" // For easier debugging
// #define CLEAR_COLOUR 20
// #define TEXT_COLOUR  15

int main(int argc, char **argv)
{
    br_pixelmap *screen = NULL, *colour_buffer = NULL, *depth_buffer = NULL, *pal = NULL, *shade_table = NULL;
    br_actor    *world, *camera, *cube, *light;
    int          ret = 1;
    uclock_t     ticks_last, ticks_now;
    br_colour    clear_colour = CLEAR_COLOUR, text_colour = TEXT_COLOUR;
    br_error     err;

    BrBegin();

    BrLogSetLevel(BR_LOG_DEBUG);

    if((pal = BrPixelmapLoad(PALETTE_NAME)) == NULL) {
        BrLogError("APP", "loading " PALETTE_NAME " failed");
        BrEnd();
        return 1;
    }

    if((shade_table = BrPixelmapLoad("shade.tab")) == NULL) {
        BrLogError("DEMO", "Unable to load shade.tab");
        BrEnd();
        return BRE_FAIL;
    }
    BrTableAdd(shade_table);

    // clang-format off
    err = BrDevBeginVar(&screen, "MCGA",
        BRT_WIDTH_I32, 320,
        BRT_HEIGHT_I32, 200,
        BR_NULL_TOKEN
    );
    // clang-format on

    if(err != BRE_OK) {
        BrLogError("APP", "BrDevBeginVar() failed");
        goto create_fail;
    }

    BrPixelmapPaletteSet(screen, pal);

    BrLogInfo("APP", "Running at 8-bpp");

    if((colour_buffer = BrPixelmapMatch(screen, BR_PMMATCH_OFFSCREEN)) == NULL) {
        BrLogError("APP", "BrPixelmapAllocate() failed");
        goto create_fail;
    }

    if((depth_buffer = BrPixelmapMatch(colour_buffer, BR_PMMATCH_DEPTH_16)) == NULL) {
        BrLogError("APP", "BrPixelmapMatch() failed");
        goto create_fail;
    }

    colour_buffer->origin_x = depth_buffer->origin_x = colour_buffer->width >> 1;
    colour_buffer->origin_y = depth_buffer->origin_y = colour_buffer->height >> 1;

    BrRendererBegin(colour_buffer, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    world = BrActorAllocate(BR_ACTOR_NONE, NULL);

    {
        br_camera *camera_data;

        camera         = BrActorAdd(world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
        camera->t.type = BR_TRANSFORM_MATRIX34;
        BrMatrix34Translate(&camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(5.0));

        camera_data         = (br_camera *)camera->type_data;
        camera_data->aspect = BR_DIV(BR_SCALAR(colour_buffer->width), BR_SCALAR(colour_buffer->height));
    }

    BrModelFindHook(BrModelFindFailedLoad);
    BrMapFindHook(BrMapFindFailedLoad);
    BrMaterialFindHook(BrMaterialFindFailedLoad);

    cube         = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    cube->t.type = BR_TRANSFORM_MATRIX34;
    cube->model  = BrModelFind("cube.dat");

    cube->material = BrMaterialLoad("checkerboard8.mat");

    cube->material->flags |= BR_MATF_DITHER;      // Dithering.
    cube->material->flags |= BR_MATF_SMOOTH;      // Makes lighting look _much_ better.
    cube->material->opacity = 255; // < 255 selects screendoor renderer

    cube->material->index_shade = BrTableFind("shade_table");
    cube->material->index_base  = 0;
    cube->material->index_range = 63;

    BrMapUpdate(cube->material->colour_map, BR_MAPU_ALL);
    BrMaterialUpdate(cube->material, BR_MATU_ALL);

    BrMatrix34RotateY(&cube->t.t.mat, BR_ANGLE_DEG(30));

    light = BrActorAdd(world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    BrLightEnable(light);

    ticks_last = uclock();

    for(;;) {
        float dt;

        ticks_now  = uclock();
        dt         = (float)(ticks_now - ticks_last) / (float)UCLOCKS_PER_SEC;
        ticks_last = ticks_now;

        if(kbhit()) {
            goto done;
        }

        BrMatrix34PostRotateY(&cube->t.t.mat, BR_ANGLE_DEG(BR_SCALAR(50) * BR_SCALAR(dt)));

        BrRendererFrameBegin();
        BrPixelmapFill(colour_buffer, clear_colour);
        BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
        BrZbSceneRender(world, camera, colour_buffer, depth_buffer);
        BrRendererFrameEnd();

        draw_info(colour_buffer, cube->material, text_colour);
        BrPixelmapDoubleBuffer(screen, colour_buffer);
    }

done:
    ret = 0;

    BrRendererEnd();

create_fail:

    if(depth_buffer != NULL)
        BrPixelmapFree(depth_buffer);

    if(colour_buffer != NULL)
        BrPixelmapFree(colour_buffer);

    BrEnd();

    return ret;
}
