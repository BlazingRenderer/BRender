//////////////////////////////////////////////////////////////////////
//
// Project  : BRender Forest Demo
//            Copyright (c) 1996 Argonaut Technologies Ltd.
//            All rights reserved.
//          : Copyright(c) 2025 Zane van Iperen
//
// Author         : Simon Everett
// Demo Framework : Steve Williams
// Ported by      : Zane van Iperen
//
// File     : forest.c
//
// Requires : BRender V1.4
//
// Comments : Forest z-sort demo
//
// SPDX-License-Identifier: Apache-2.0
//
//////////////////////////////////////////////////////////////////////

#include <brdemo.h>
#include <string.h>

#define SPF (1.0f / 60.0f)

typedef struct br_demo_forest_textures {
    br_pixelmap *spcship;
    br_pixelmap *floor;
    br_pixelmap *plant;
    br_pixelmap *column;
    br_pixelmap *cloud;
} br_demo_forest_textures;

typedef struct br_demo_forest_texdef {
    const char *filename;
    br_size_t   offset;
} br_demo_forest_texdef;

static const br_demo_forest_texdef Small_Bitmaps[] = {
    {.filename = "spcship.pix", .offset = offsetof(br_demo_forest_textures, spcship)},
    {.filename = "sfloor.pix", .offset = offsetof(br_demo_forest_textures, floor)},
    {.filename = "splant.pix", .offset = offsetof(br_demo_forest_textures, plant)},
    {.filename = "scolumn.pix", .offset = offsetof(br_demo_forest_textures, column)},
    {.filename = "scloud.pix", .offset = offsetof(br_demo_forest_textures, cloud)},
    {.filename = "", .offset = 0},
};

static const br_demo_forest_texdef Big_Bitmaps[] = {
    {.filename = "spcship.pix", .offset = offsetof(br_demo_forest_textures, spcship)},
    {.filename = "floor.pix", .offset = offsetof(br_demo_forest_textures, floor)},
    {.filename = "plant.pix", .offset = offsetof(br_demo_forest_textures, plant)},
    {.filename = "column.pix", .offset = offsetof(br_demo_forest_textures, column)},
    {.filename = "cloud.pix", .offset = offsetof(br_demo_forest_textures, cloud)},
    {.filename = "", .offset = 0},
};

typedef struct br_demo_forest_materials {
    br_material *spcship;
    br_material *floor;
    br_material *plant;
    br_material *column;
    br_material *cloud;
} br_demo_forest_materials;

typedef struct br_demo_forest {
    br_actor *camera;
    br_actor *pivot0;
    br_actor *pivot1;
    br_actor *object;

    br_demo_forest_textures  tex_big;
    br_demo_forest_textures  tex_sml;
    br_demo_forest_materials mats;

    br_matrix34 *loop;

    int        counter;
    int        animate;
    int        use_small;
    int        counter1;
    int        counter2;
    int        counter3;
    int        auto_change;
    int        start;
    br_uint_32 frames;

    br_uint_32 camera_look_at;
    br_scalar  xpos, ypos, zpos;

    br_uint_32 current_camera_mode;

    float accum;
} br_demo_forest;

// Prototypes for CameraUpdate#, CameraReset#
static void CameraUpdate0(br_demo_forest *state);
static void CameraReset0(br_demo_forest *state);
static void CameraUpdate1(br_demo_forest *state);
static void CameraReset1(br_demo_forest *state);
static void CameraUpdate2(br_demo_forest *state);
static void CameraReset2(br_demo_forest *state);
static void CameraUpdate3(br_demo_forest *state);
static void CameraReset3(br_demo_forest *state);

typedef void camera_fn(br_demo_forest *state);

const static struct {
    camera_fn *update;
    camera_fn *reset;
    br_uint_32 frames;
} CameraModes[] = {
    {CameraUpdate0, CameraReset0, 1800},
    {CameraUpdate1, CameraReset1, 1800},
    {CameraUpdate2, CameraReset2, 900 },
    {CameraUpdate3, CameraReset3, 1800},
};

br_error Add_Textures(const br_demo_forest_texdef Bitmaps[], br_demo_forest_textures *textures)
{
    for(int i = 0;; ++i) {
        br_pixelmap *pm;

        if(Bitmaps[i].filename[0] == '\0')
            break;

        if((pm = BrPixelmapLoad(Bitmaps[i].filename)) == NULL) {
            BrLogError("DEMO", "Error loading %s.", Bitmaps[i].filename);
            return BRE_FAIL;
        }

        if(pm)
            BrMapAdd(pm);

        *(br_pixelmap **)((br_uintptr_t)textures + Bitmaps[i].offset) = pm;
    }

    return BRE_OK;
}

static void ForestSwitchTextures(br_demo *demo, int small)
{
    br_demo_forest                *state = demo->user;
    const br_demo_forest_textures *tex   = small ? &state->tex_sml : &state->tex_big;

    state->mats.spcship->colour_map = tex->spcship;
    BrMaterialUpdate(state->mats.spcship, BR_MATU_COLOURMAP);

    state->mats.floor->colour_map = tex->floor;
    BrMaterialUpdate(state->mats.floor, BR_MATU_COLOURMAP);

    state->mats.plant->colour_map = tex->plant;
    BrMaterialUpdate(state->mats.plant, BR_MATU_COLOURMAP);

    state->mats.column->colour_map = tex->column;
    BrMaterialUpdate(state->mats.column, BR_MATU_COLOURMAP);

    state->mats.cloud->colour_map = tex->cloud;
    BrMaterialUpdate(state->mats.cloud, BR_MATU_COLOURMAP);
}

static br_error ForestInit(br_demo *demo)
{
    br_demo_forest *state;
    br_camera      *camera_data;
    br_model       *model_floor, *model_column, *model_ship;
    br_pixelmap    *shade_table;
    br_material    *pMaterial[32];
    br_uint_16      Num_Materials;

    state      = BrResAllocate(demo, sizeof(br_demo_forest), BR_MEMORY_APPLICATION);
    demo->user = state;

    *state = (br_demo_forest){
        .animate = 1,
        .start   = 0,
        .accum   = SPF,
    };

    if((demo->palette = BrPixelmapLoad("forest.pal")) == NULL) {
        BrLogError("DEMO", "Error loading forest.pal.");
        return BRE_FAIL;
    }
    BrTableAdd(demo->palette);
    BrPixelmapPaletteSet(demo->colour_buffer, demo->palette);

    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        demo->text_colour = 118;
    }

    Add_Textures(Small_Bitmaps, &state->tex_sml);
    Add_Textures(Big_Bitmaps, &state->tex_big);

    if((Num_Materials = BrFmtScriptMaterialLoadMany("forest.mat", pMaterial, 32)) == 0) {
        BrLogError("DEMO", "Error loading forest.mat.");
        return BRE_FAIL;
    }
    BrMaterialAddMany(pMaterial, Num_Materials);

    state->mats.spcship = BrMaterialFind("SPACE_SHIP");
    state->mats.floor   = BrMaterialFind("floor_0");
    state->mats.plant   = BrMaterialFind("plant");
    state->mats.column  = BrMaterialFind("column");
    state->mats.cloud   = BrMaterialFind("cloud");

    ForestSwitchTextures(demo, state->use_small);

    if((model_floor = BrModelLoad("floor.dat")) == NULL) {
        BrLogError("DEMO", "Error loading floor.dat.");
        return BRE_FAIL;
    }
    BrModelAdd(model_floor);

    if((model_column = BrModelLoad("column.dat")) == NULL) {
        BrLogError("DEMO", "Error loading column.dat.");
        return BRE_FAIL;
    }
    BrModelAdd(model_column);

    if((model_ship = BrModelLoad("ship.dat")) == NULL) {
        BrLogError("DEMO", "Error loading ship.dat.");
        return BRE_FAIL;
    }
    BrModelAdd(model_ship);

    // Setup generic data for samples
    demo->world->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Identity(&demo->world->t.t.mat);

    demo->camera = state->camera = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    state->camera->identifier    = BrResStrDup(state->camera, "camera");
    state->camera->t.type        = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&state->camera->t.t.mat, 0, 0, BR_SCALAR(1));

    camera_data                = state->camera->type_data;
    camera_data->type          = BR_CAMERA_PERSPECTIVE;
    camera_data->aspect        = BR_SCALAR(1.21);
    camera_data->hither_z      = BR_SCALAR(0.05);
    camera_data->yon_z         = BR_SCALAR(1000);
    camera_data->field_of_view = BR_ANGLE_DEG(45.0);

    demo->order_table->min_z = camera_data->hither_z;
    demo->order_table->max_z = camera_data->yon_z;

    br_actor *light                        = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    light->identifier                      = BrResStrDup(light, "main_light");
    light->t.type                          = BR_TRANSFORM_MATRIX34;
    ((br_light *)light->type_data)->colour = BR_COLOUR_RGB(255, 255, 255);
    BrMatrix34Identity(&light->t.t.mat);

    BrLightEnable(light);

    // setup order table at root (generate new bounds,
    // use average of 3 vertices for sorting buckets)

    // BrZsActorOrderTableSet(root, BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_NEW_BOUNDS | BR_ORDER_TABLE_SORT_NEAR, BR_SORT_MAX));

    switch(demo->colour_buffer->type) {
        case BR_PMT_INDEX_8:
            if((shade_table = BrPixelmapLoad("forest8.tab")) == NULL) {
                BrLogError("DEMO", "Error loading forest8.tab.");
                return BRE_FAIL;
            }
            BrTableAdd(shade_table);
            break;

        case BR_PMT_RGB_555:
            if((shade_table = BrPixelmapLoad("forest15.tab")) == NULL) {
                BrLogError("DEMO", "Error loading forest16.tab.");
                return BRE_FAIL;
            }
            BrTableAdd(shade_table);
            break;

        case BR_PMT_RGB_565:
            if((shade_table = BrPixelmapLoad("forest16.tab")) == NULL) {
                BrLogError("DEMO", "Error loading forest16.tab.");
                return BRE_FAIL;
            }
            BrTableAdd(shade_table);
            break;

        default:
            break;
    }

    state->current_camera_mode = 0;
    state->frames              = CameraModes[state->current_camera_mode].frames;
    state->auto_change         = 1;

    state->pivot0         = BrActorAllocate(BR_ACTOR_NONE, NULL);
    state->pivot0->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&state->pivot0->t.t.mat, state->xpos, state->ypos, state->zpos);

    state->pivot1         = BrActorAdd(state->pivot0, BrActorAllocate(BR_ACTOR_NONE, NULL));
    state->pivot1->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Identity(&state->pivot1->t.t.mat);

    CameraModes[state->current_camera_mode].reset(state);

    br_actor *floor_actor = BrActorAdd(state->pivot1, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    floor_actor->model    = model_floor;
    floor_actor->t.type   = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&floor_actor->t.t.mat, 0, BR_SCALAR(-5), 0);

    for(int i = 0; i < 4; i++) {
        br_actor *column = BrActorAdd(state->pivot1, BrActorAllocate(BR_ACTOR_MODEL, NULL));
        column->model    = model_column;
        column->t.type   = BR_TRANSFORM_MATRIX34;

        BrMatrix34Translate(&column->t.t.mat, BR_SCALAR(18), 0, 0);
        BrMatrix34PostRotateY(&column->t.t.mat, BR_ANGLE_DEG(i * (360 / 4) + (360 / 4) / 2));
    }

    state->object        = BrActorAdd(state->pivot1, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    state->object->model = model_ship;

    state->object->t.type = BR_TRANSFORM_MATRIX34;

    BrActorAdd(demo->world, state->pivot0);

    state->loop = BrResAllocate(state, sizeof(br_matrix34) * 360, BR_MEMORY_APPLICATION);

    state->counter = 0, state->counter1 = 0, state->counter2 = 0, state->counter3 = 0;
    for(int i = 0; i <= 180; i++) {

        state->counter += 2;
        if(state->counter >= 360)
            state->counter -= 360;
        state->counter1 += 4;
        if(state->counter1 >= 360)
            state->counter1 -= 360;

        BrMatrix34Translate(&state->object->t.t.mat, BR_ADD(BR_MUL(BR_SIN(BR_ANGLE_DEG(state->counter1)), BR_SCALAR(8)), BR_SCALAR(18)),
                            BR_SIN(BR_ANGLE_DEG(state->counter1 * 2)), 0);
        BrMatrix34PostRotateY(&state->object->t.t.mat, BR_ANGLE_DEG(state->counter));
        BrMatrix34PreRotateZ(&state->object->t.t.mat, BR_CONST_DIV(BR_SIN(BR_ANGLE_DEG(state->counter1)), 10));

        BrMatrix34Copy(state->loop + i, &state->object->t.t.mat);
    }

    state->counter = 0, state->counter1 = 0, state->counter2 = 0, state->counter3 = 0;

    return BRE_OK;
}

static void ForestRender(br_demo *demo)
{
    br_demo_forest *state = demo->user;
#if 0 /* Enable to use Z-buffering (much faster) instead of Z-sorting. */
    BrDemoDefaultRender(demo);
#else

    BrPixelmapFill(demo->colour_buffer, 0);

    BrRendererFrameBegin();
    BrZsSceneRenderBegin(demo->world, demo->camera, demo->colour_buffer);
    BrZsSceneRenderAdd(state->pivot0);
    BrZsSceneRenderEnd();
    BrRendererFrameEnd();
#endif

    {
        br_int_32 base_x      = -demo->colour_buffer->origin_x + 5;
        br_int_32 base_y      = -demo->colour_buffer->origin_y + 5;
        br_colour col         = demo->text_colour;
        br_int_32 text_height = BrPixelmapTextHeight(demo->colour_buffer, BrFontProp7x9);

        static const char *usage_lines[] = {
            "Usage:",
            "  'A':            Freezes a frame of the animation",
            "  SPACEBAR:   Switches between camera modes",
            "  'Q':            Quit Demo",
            "  'C':            Toggle auto camera mode switching",
            "  'T':            Toggle big/small textures",
            "  '1'/'0':        Move forwards/backwards",
        };

        base_y += text_height * 2;

        for(size_t i = 0; i < BR_ASIZE(usage_lines); ++i) {
            base_y += text_height * 2;
            BrPixelmapText(demo->colour_buffer, base_x, base_y, col, BrFontProp7x9, usage_lines[i]);
        }
    }
}

static void ForestProcessEvent(br_demo *demo, const SDL_Event *evt)
{
    br_demo_forest *state = demo->user;

    switch(evt->type) {
        case SDL_EVENT_KEY_DOWN: {
            switch(evt->key.key) {
                case SDLK_Q:
                    SDL_PushEvent(&(SDL_Event){.type = SDL_EVENT_QUIT});
                    break;

                case SDLK_A:
                    state->animate ^= 1;
                    break;

                case SDLK_C:
                    state->auto_change ^= 1;
                    break;

                case SDLK_T:
                    state->use_small ^= 1;
                    ForestSwitchTextures(demo, state->use_small);
                    break;

                case SDLK_SPACE:
                    state->start  = 1;
                    state->frames = 0;
                    break;

                case SDLK_UP:
                    state->ypos = BR_ADD(state->ypos, BR_SCALAR(0.5));
                    break;

                case SDLK_DOWN:
                    state->ypos = BR_SUB(state->ypos, BR_SCALAR(0.5));
                    break;

                case SDLK_LEFT:
                    state->xpos = BR_SUB(state->xpos, BR_SCALAR(0.5));
                    break;

                case SDLK_RIGHT:
                    state->xpos = BR_ADD(state->xpos, BR_SCALAR(0.5));
                    break;

                case SDLK_1:
                    state->zpos = BR_ADD(state->zpos, BR_SCALAR(0.5));
                    break;

                case SDLK_0:
                    state->zpos = BR_SUB(state->zpos, BR_SCALAR(0.5));
                    break;

                default:
                    break;
            }
            break;
        }

        case SDL_EVENT_MOUSE_MOTION:
            if(evt->motion.state & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) {
                state->xpos = BR_ADD(state->xpos, BR_DIV(evt->motion.xrel, BR_SCALAR(16)));

                if(evt->motion.state & SDL_BUTTON_LMASK)
                    state->ypos = BR_ADD(state->ypos, BR_DIV(evt->motion.yrel, BR_SCALAR(16)));

                if(evt->motion.state & SDL_BUTTON_RMASK)
                    state->zpos = BR_ADD(state->zpos, BR_DIV(evt->motion.yrel, BR_SCALAR(16)));
            }
            break;

        default:
            break;
    }

    if(state->ypos > BR_SCALAR(3))
        state->ypos = BR_SCALAR(3);
    if(state->ypos < BR_SCALAR(-60))
        state->ypos = BR_SCALAR(-60);

    if(state->zpos > BR_SCALAR(80))
        state->zpos = BR_SCALAR(80);
    if(state->zpos < BR_SCALAR(-80))
        state->zpos = BR_SCALAR(-80);

    if(state->xpos > BR_SCALAR(80))
        state->xpos = BR_SCALAR(80);
    if(state->xpos < BR_SCALAR(-80))
        state->xpos = BR_SCALAR(-80);
}

static void ForestUpdate(br_demo *demo, br_scalar dt)
{
    br_demo_forest *state = demo->user;

    if(state->auto_change == 1) {
        if(state->frames == 0)
            state->start = 1;
        else
            state->frames--;
    }

    if(state->start) {
        state->current_camera_mode = (state->current_camera_mode + 1) % BR_ASIZE(CameraModes);
        CameraModes[state->current_camera_mode].reset(state);

        state->frames = CameraModes[state->current_camera_mode].frames;

        state->start = 0;
    }

    br_transform xform;

    state->accum += dt;
    while(state->accum >= SPF) {
        state->counter++;
        if(state->counter >= 180)
            state->counter -= 180;

        state->counter1 += 2;
        if(state->counter1 >= 360)
            state->counter1 -= 360;

        state->counter2++;
        if(state->counter2 >= 60)
            state->counter2 -= 60;

        if((state->counter2 % 2) == 0)
            state->counter3 += 1;
        if(state->counter3 == 64)
            state->counter3 = 0;

        state->accum -= SPF;
    }

    if(state->animate == 1) {
        BrMatrix34Copy(&state->object->t.t.mat, state->loop + state->counter);
    }

    if(state->camera_look_at == 1) {
        BrVector3SetInt(&state->camera->t.t.look_up.up, 0, 1, 0);
        BrVector3Set(&state->camera->t.t.look_up.t, 0, 0, 0);
    }

    BrMatrix34Translate(&state->pivot0->t.t.mat, state->xpos, state->ypos, state->zpos);

    CameraModes[state->current_camera_mode].update(state);

    if(state->camera_look_at == 1) {
        BrActorToActorMatrix34(&xform.t.mat, state->object, state->camera->parent);
        BrVector3Sub(&state->camera->t.t.look_up.look, &xform.t.translate.t, &state->camera->t.t.look_up.t);
    }
}

static void CameraReset0(br_demo_forest *state)
{
    BrMatrix34Identity(&state->pivot1->t.t.mat);
    state->camera->t.type = BR_TRANSFORM_LOOK_UP;
    state->xpos           = 0;
    state->ypos           = BR_SCALAR(-6);
    state->zpos           = BR_SCALAR(-30);
    state->camera_look_at = 1;
}

static void CameraUpdate0(br_demo_forest *state)
{
    (void)state;
}

static void CameraReset1(br_demo_forest *state)
{
    state->camera->t.type = BR_TRANSFORM_LOOK_UP;
    state->xpos           = 0;
    state->ypos           = BR_SCALAR(-7);
    state->zpos           = BR_SCALAR(8);
    state->camera_look_at = 1;
}

static void CameraUpdate1(br_demo_forest *state)
{
}

static void CameraReset2(br_demo_forest *state)
{
    state->camera->t.type = BR_TRANSFORM_MATRIX34;
    state->camera_look_at = 0;
}

static void CameraUpdate2(br_demo_forest *state)
{
    BrMatrix34Copy(&state->camera->t.t.mat, state->loop + ((state->counter + 170) % 180));
    BrMatrix34PreTranslate(&state->camera->t.t.mat, state->xpos, state->ypos, state->zpos);
}

static void CameraReset3(br_demo_forest *state)
{
    BrMatrix34Identity(&state->pivot1->t.t.mat);
    state->camera->t.type = BR_TRANSFORM_LOOK_UP;
    state->xpos           = 0;
    state->ypos           = BR_SCALAR(-20);
    state->zpos           = BR_SCALAR(-50);
    state->camera_look_at = 1;
}

static void CameraUpdate3(br_demo_forest *state)
{
    BrMatrix34RotateY(&state->pivot1->t.t.mat, BR_ANGLE_DEG(state->counter1));
}

void ForestOnResize(br_demo *demo, br_uint_16 width, br_uint_16 height)
{
    BrDemoDefaultOnResize(demo, width, height);

    /*
     * Restore the palette.
     */
    BrPixelmapPaletteSet(demo->colour_buffer, demo->palette);
}

const static br_demo_dispatch dispatch = {
    .init          = ForestInit,
    .process_event = ForestProcessEvent,
    .update        = ForestUpdate,
    .render        = ForestRender,
    .on_resize     = ForestOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("Forest Demo", &dispatch, argc, argv);
}
