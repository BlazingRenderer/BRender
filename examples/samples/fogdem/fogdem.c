/* fogdem.c
 *
 * A demo to show BRender's support for fogging
 */

#include <brdemo.h>

#define SQUARE                   30
#define FADETIME                 128
#define ROLLING_BALL_SENSITIVITY 500
#define MSCALE                   BR_SCALAR(0.006)

static void Matrix33Mul(br_matrix34 *, br_matrix34 *, br_matrix34 *);

typedef struct br_demo_fog {
    int textcol;
    int textshad;

    br_pixelmap *fog;
    br_pixelmap *shade;

    br_actor *world;
    br_actor *light;
    br_actor *mainobj;
    br_actor *ground;
    br_actor *spotlight;

    br_angle  a;
    br_scalar t, x, y, ox, oy, oz;
    int       sr, er, sg, eg, sb, eb;

    /*
     * Feature toggling functions
     */
    br_boolean helpon;
    br_boolean fogon;
    br_boolean mainon;
    br_boolean spoton;
    br_boolean groundon;
    br_boolean fadeon;
} br_demo_fog;

/*
 * Generate ground mesh
 */
void Initground(br_model *model, const int square, br_material *material)
{
    br_vertex *vertex;
    br_face   *face;
    int        i, j;

    vertex = model->vertices;
    face   = model->faces;

    for(i = 0; i <= square; i++)
        for(j = 0; j <= square; j++, vertex++) {
            BrVector3SetFloat(&vertex->p, ((float)i - ((float)square) / 2.0) / 4.0, 0, ((float)j - ((float)square) / 2.0) / 4.0);
            BrVector2SetFloat(&vertex->map, ((float)i - ((float)square) / 2.0) / 16.0,
                              ((float)j - ((float)square) / 2.0) / 16.0);
        }

    for(i = 0; i < square; i++)
        for(j = 0; j < square; j++) {
            face->material    = material;
            face->vertices[0] = i + j * (square + 1);
            face->vertices[1] = i + j * (square + 1) + 1;
            face->vertices[2] = i + (j + 1) * (square + 1);
            face->smoothing   = 1;
            face++;
            face->material    = material;
            face->vertices[0] = i + (j + 1) * (square + 1);
            face->vertices[1] = i + j * (square + 1) + 1;
            face->vertices[2] = i + (j + 1) * (square + 1) + 1;
            face->smoothing   = 1;
            face++;
        }
}

/*
 * Build a 16x16 blended palette, for optimum "fogability"
 * Takes rgb ranges
 */
void makepalette(br_pixelmap *palette, int startr, int endr, int startg, int endg, int startb, int endb)
{
    br_colour *palette_entry;
    int        i, j;

    palette_entry = ((br_colour *)palette->pixels) + 16;

    for(i = 0; i < 14; i++)
        for(j = 0; j < 16; j++)
            *(palette_entry++) = BR_COLOUR_RGB((((startr + j * (endr - startr) / 15) - 128) * (13 - i) / 13) + 128,
                                               (((startg + j * (endg - startg) / 15) - 128) * (13 - i) / 13) + 128,
                                               (((startb + j * (endb - startb) / 15) - 128) * (13 - i) / 13) + 128);
}

/*
 * Blend rgb ranges over a period of time, to create a surreal palette
 * fading effect
 */
void updatecolours(int *sr, int *sg, int *sb, int *er, int *eg, int *eb)
{
    static int time  = 0;
    static int range = 0;
    static int csr, cer, csg, ceg, csb, ceb;
    static int ranges[8][6] = {
        {10, 190, 10, 170, 0,  255},
        {10, 190, 10, 170, 0,  128},
        {40, 200, 10, 170, 0,  255},
        {0,  128, 40, 128, 40, 192},
        {40, 140, 40, 140, 0,  140},
        {20, 100, 50, 150, 60, 255},
        {0,  200, 0,  150, 0,  255},
        {80, 140, 80, 140, 80, 140},
    };

    if(time > 0) {
        time--;
        *sr = (time * csr + (FADETIME - time) * ranges[range][0]) / FADETIME;
        *er = (time * cer + (FADETIME - time) * ranges[range][1]) / FADETIME;
        *sg = (time * csg + (FADETIME - time) * ranges[range][2]) / FADETIME;
        *eg = (time * ceg + (FADETIME - time) * ranges[range][3]) / FADETIME;
        *sb = (time * csb + (FADETIME - time) * ranges[range][4]) / FADETIME;
        *eb = (time * ceb + (FADETIME - time) * ranges[range][5]) / FADETIME;
    } else {
        if(range >= 7)
            range = 0;
        else
            range++;
        time = FADETIME;
        csr  = *sr;
        cer  = *er;
        csg  = *sg;
        ceg  = *eg;
        csb  = *sb;
        ceb  = *eb;
    }
}

/*
 * Show which keys do what
 */
void displayhelp(br_pixelmap *buffer, int x, int y, int colour)
{
    static char *help[] = {
        "",
        "FOG DEMO - (c) Argonaut 1996",
        "",
        "h - help text on/off",
        "f - fog on/off",
        "m - main light on/off",
        "s - spot light on/off",
        "g - ground on/off",
        "p - fade palette on/off",
        "mouse - move main object",
        NULL,
    };
    char **text = help;
    int    i;

    for(i = y; *text; i += 7)
        BrPixelmapTextF(buffer, x - buffer->origin_x, i - buffer->origin_y, colour, BrFontProp7x9, *text++);
}

void helptoggle(br_demo *demo)
{
    (void)demo;
}

void fogtoggle(br_demo *demo)
{
    br_demo_fog *state = demo->user;

    if(state->fogon) {
        state->mainobj->material->index_fog  = state->fog;
        state->mainobj->material->fog_min    = 2;
        state->mainobj->material->fog_max    = 6;
        state->mainobj->material->fog_colour = BR_COLOUR_RGB(128, 128, 128);
        state->mainobj->material->flags |= BR_MATF_FOG_LOCAL;
        BrMaterialUpdate(state->mainobj->material, BR_MATU_ALL);
        state->ground->material->index_fog  = state->fog;
        state->ground->material->fog_min    = 2;
        state->ground->material->fog_max    = 6;
        state->ground->material->fog_colour = BR_COLOUR_RGB(128, 128, 128);
        state->ground->material->flags |= BR_MATF_FOG_LOCAL;
        BrMaterialUpdate(state->ground->material, BR_MATU_ALL);
    } else {
        state->mainobj->material->index_fog = NULL;
        state->mainobj->material->flags &= ~BR_MATF_FOG_LOCAL;
        BrMaterialUpdate(state->mainobj->material, BR_MATU_ALL);
        state->ground->material->index_fog = NULL;
        state->ground->material->flags &= ~BR_MATF_FOG_LOCAL;
        BrMaterialUpdate(state->ground->material, BR_MATU_ALL);
    }
}

void maintoggle(br_demo *demo)
{
    br_demo_fog *state = demo->user;

    if(state->mainon)
        BrLightEnable(state->light);
    else
        BrLightDisable(state->light);
}

void spottoggle(br_demo *demo)
{
    br_demo_fog *state = demo->user;

    if(state->spoton)
        BrLightEnable(state->spotlight);
    else
        BrLightDisable(state->spotlight);
}

void groundtoggle(br_demo *demo)
{
    br_demo_fog *state = demo->user;

    if(state->groundon)
        state->ground->render_style = BR_RSTYLE_DEFAULT;
    else
        state->ground->render_style = BR_RSTYLE_NONE;
}

void fadetoggle(br_demo *demo)
{
    (void)demo;
}

static void FogProcessEvent(br_demo *demo, const SDL_Event *evt)
{
    br_demo_fog *state = demo->user;

    struct {
        SDL_KeyCode keytest;
        br_boolean *flag;
        void (*togglefn)(br_demo *demo);
    } keytable[] = {
        {SDLK_h, &state->helpon, helptoggle},
        {SDLK_f, &state->fogon, fogtoggle},
        {SDLK_m, &state->mainon, maintoggle},
        {SDLK_s, &state->spoton, spottoggle},
        {SDLK_g, &state->groundon, groundtoggle},
        {SDLK_p, &state->fadeon, fadetoggle},
        {0, NULL}
    };

    switch(evt->type) {
        case SDL_KEYDOWN: {
            if(evt->key.keysym.sym == 'q') {
                SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});
                break;
            }

            for(int i = 0; i < BR_ASIZE(keytable); ++i) {
                if(keytable[i].keytest == evt->key.keysym.sym) {
                    *keytable[i].flag = !*keytable[i].flag;
                    keytable[i].togglefn(demo);
                }
            }
            break;
        }

        case SDL_MOUSEMOTION: {
            br_scalar   tx, ty, tz;
            br_actor   *target = state->mainobj;
            br_matrix34 mat_target, mat_roll;

            Sint32 mouse_x = evt->motion.xrel;
            Sint32 mouse_y = evt->motion.yrel;

            if((evt->motion.state & SDL_BUTTON_LMASK) && (evt->motion.state & SDL_BUTTON_RMASK)) {
                // Drag object around in x/y plane

                tx = BR_MUL(BR_SCALAR(mouse_x), MSCALE);
                tz = BR_MUL(BR_SCALAR(mouse_y), MSCALE);

                BrMatrix34PostTranslate(&target->t.t.mat, tx, BR_SCALAR(0.0), tz);

            } else if(evt->motion.state & SDL_BUTTON_LMASK) {
                // Move object along z axis

                tx = BR_MUL(BR_SCALAR(mouse_x), MSCALE);
                ty = -BR_MUL(BR_SCALAR(mouse_y), MSCALE);

                BrMatrix34PostTranslate(&target->t.t.mat, tx, ty, BR_SCALAR(0.0));
            } else if(evt->motion.state & SDL_BUTTON_RMASK) {
                // Rotate object via rolling ball interface

                BrMatrix34RollingBall(&mat_roll, mouse_x, -mouse_y, ROLLING_BALL_SENSITIVITY);

                // Only modify the top 3x3

                BrMatrix34Copy(&mat_target, &target->t.t.mat);
                Matrix33Mul(&target->t.t.mat, &mat_target, &mat_roll);
            }
            break;
        }
        default:
            break;
    }
}

static br_error FogInit(br_demo *demo)
{
    br_demo_fog *state;
    br_uint_32   n;
    br_material *mats[20];
    br_model    *skull;

    state      = BrResAllocate(demo, sizeof(br_demo_fog), BR_MEMORY_APPLICATION);
    demo->user = state;

    *state = (br_demo_fog){
        .a  = 0,
        .t  = 0,
        .x  = 0,
        .ox = 0,
        .oy = 0,
        .oz = 0,
        .sr = 10,
        .er = 190,
        .sg = 10,
        .eg = 170,
        .sb = 0,
        .eb = 255,

        .helpon   = BR_TRUE,
        .fogon    = BR_TRUE,
        .mainon   = BR_TRUE,
        .spoton   = BR_TRUE,
        .groundon = BR_TRUE,
        .fadeon   = BR_TRUE,
    };

    if((demo->palette = BrPixelmapLoad("fogdem.pal")) == NULL) {
        BrLogError("DEMO", "Error loading fogdem.pal.");
        return BRE_FAIL;
    }

    switch(demo->colour_buffer->type) {
        case BR_PMT_RGB_555:
        case BR_PMT_RGB_565:
        case BR_PMT_RGB_888:
        case BR_PMT_RGBX_888:
            state->textcol  = 0xffffff;
            state->textshad = 0x000000;
            break;
        case BR_PMT_INDEX_8:
        default:
            state->textcol  = 31;
            state->textshad = 16;
            break;
    }

    demo->text_colour = state->textcol;

    if((state->fog = BrPixelmapLoad("fogtable.tab")) == NULL) {
        BrLogError("DEMO", "Error loading fogtable.tab.");
        return BRE_FAIL;
    }
    BrTableAdd(state->fog);

    if((state->shade = BrPixelmapLoad("fogshade.tab")) == NULL) {
        BrLogError("DEMO", "Error loading fogshade.tab.");
        return BRE_FAIL;
    }
    BrTableAdd(state->shade);

    if((skull = BrModelLoad("skull.dat")) == NULL) {
        BrLogError("DEMO", "Error loading skull.dat.");
        return BRE_FAIL;
    }

    BrTableFindHook(BrTableFindFailedLoad);
    BrMapFindHook(BrMapFindFailedLoad);

    n = BrFmtScriptMaterialLoadMany("fogdem.mat", mats, BR_ASIZE(mats));
    BrMaterialAddMany(mats, n);

    br_actor *world = demo->world;

    demo->camera     = BrActorAllocate(BR_ACTOR_CAMERA, NULL);
    state->light     = BrActorAllocate(BR_ACTOR_LIGHT, NULL);
    state->mainobj   = BrActorAllocate(BR_ACTOR_MODEL, NULL);
    state->ground    = BrActorAllocate(BR_ACTOR_MODEL, NULL);
    state->spotlight = BrActorAllocate(BR_ACTOR_LIGHT, NULL);

    ((br_camera *)demo->camera->type_data)->type          = BR_CAMERA_PERSPECTIVE;
    ((br_camera *)demo->camera->type_data)->field_of_view = BR_ANGLE_DEG(45.0);
    ((br_camera *)demo->camera->type_data)->hither_z      = BR_SCALAR(0.1);
    ((br_camera *)demo->camera->type_data)->yon_z         = BR_SCALAR(20.0);
    ((br_camera *)demo->camera->type_data)->aspect        = BR_SCALAR(320.0 / 240.0);
    demo->camera->t.type                                  = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&demo->camera->t.t.mat, 0, BR_SCALAR(0.2), BR_SCALAR(5));
    BrMatrix34PreRotateX(&demo->camera->t.t.mat, BR_ANGLE_DEG(-12));

    demo->order_table->min_z = ((br_camera *)demo->camera->type_data)->hither_z;
    demo->order_table->max_z = ((br_camera *)demo->camera->type_data)->yon_z;

    ((br_light *)state->light->type_data)->type = BR_LIGHT_DIRECT;
    state->light->t.type                        = BR_TRANSFORM_MATRIX34;
    BrMatrix34RotateX(&state->light->t.t.mat, BR_ANGLE_DEG(-30));
    BrLightEnable(state->light);

    ((br_light *)state->spotlight->type_data)->type       = BR_LIGHT_SPOT;
    ((br_light *)state->spotlight->type_data)->cone_inner = BR_ANGLE_DEG(30);
    ((br_light *)state->spotlight->type_data)->cone_outer = BR_ANGLE_DEG(70);
    state->spotlight->t.type                              = BR_TRANSFORM_MATRIX34;
    BrLightEnable(state->spotlight);

    state->mainobj->model    = skull;
    state->mainobj->material = BrMaterialFind("environment");
    BrMaterialUpdate(state->mainobj->material, BR_MATU_ALL);
    BrModelAdd(state->mainobj->model);

    state->ground->model    = BrModelAllocate("ground", (SQUARE + 1) * (SQUARE + 1), SQUARE * SQUARE * 2);
    state->ground->material = BrMaterialFind("earth");
    Initground(state->ground->model, SQUARE, state->ground->material);
    BrMaterialUpdate(state->ground->material, BR_MATU_ALL);
    state->ground->model->flags = BR_MODF_UPDATEABLE;
    BrModelAdd(state->ground->model);
    BrModelUpdate(state->ground->model, BR_MODU_ALL);

    BrActorAdd(world, demo->camera);
    BrActorAdd(world, state->light);
    BrActorAdd(world, state->mainobj);
    BrActorAdd(world, state->ground);
    BrActorAdd(state->mainobj, state->spotlight);

    BrMatrix34RotateY(&state->spotlight->t.t.mat, BR_ANGLE_DEG(-0));

    return BRE_OK;
}

static void FogUpdate(br_demo *demo, br_scalar dt)
{
    br_demo_fog *state = demo->user;
    br_matrix34  roll;

    BrMatrix34Identity(&roll);

    if(state->fadeon) {
        updatecolours(&state->sr, &state->sg, &state->sb, &state->er, &state->eg, &state->eb);
        makepalette(demo->palette, state->sr, state->er, state->sg, state->eg, state->sb, state->eb);
        BrPixelmapPaletteSet(demo->colour_buffer, demo->palette);
        BrMapUpdate(state->ground->material->colour_map, BR_MAPU_ALL);
    }

    state->a += BR_ANGLE_DEG(0.25);

    state->x = BR_MUL(BR_SIN(state->a), BR_SCALAR(0.013));
    state->y = BR_MUL(BR_SIN(state->a + BR_ANGLE_DEG(65)), BR_SCALAR(0.007));

    BrMatrix23PostTranslate(&state->ground->material->map_transform, state->x, state->y);
    BrMatrix23PreRotate(&state->ground->material->map_transform,
                        BR_ANGLE_DEG(BrScalarToFloat(BR_MUL(BR_SIN(state->a), BR_SCALAR(1)))));
    BrMaterialUpdate(state->ground->material, BR_MATU_MAP_TRANSFORM);

    BrMatrix34Translate(&state->ground->t.t.mat, 0, BR_SCALAR(-0.75), 0);
    BrMatrix34PostRotateZ(&state->ground->t.t.mat, BR_ANGLE_DEG(BrScalarToFloat(BR_MUL(BR_SCALAR(5), BR_COS(state->a)))));
}

static void FogRender(br_demo *demo)
{
    br_demo_fog *state = demo->user;

    switch(demo->colour_buffer->type) {
        case BR_PMT_INDEX_8:
            BrPixelmapFill(demo->colour_buffer, 239);
            break;
        case BR_PMT_RGB_555:
            BrPixelmapFill(demo->colour_buffer, BR_COLOUR_RGB_555(0x80, 0x80, 0x80));
            break;
        case BR_PMT_RGB_565:
            BrPixelmapFill(demo->colour_buffer, BR_COLOUR_RGB_565(0x80, 0x80, 0x80));
            break;
        case BR_PMT_RGB_888:
        case BR_PMT_RGBX_888:
        case BR_PMT_RGBA_8888:
            BrPixelmapFill(demo->colour_buffer, BR_COLOUR_RGBA(0x80, 0x80, 0x80, 0xFF));
            break;
        default:
            BrPixelmapFill(demo->colour_buffer, 0);
            break;
    }

    BrPixelmapFill(demo->depth_buffer, 0xFFFFFFFF);

    BrZbSceneRender(demo->world, demo->camera, demo->colour_buffer, demo->depth_buffer);
    if(state->helpon) {
        displayhelp(demo->colour_buffer, 12, 12, state->textshad);
        displayhelp(demo->colour_buffer, 11, 11, state->textcol);
    }
}

static void FogOnResize(br_demo *demo, br_uint_16 width, br_uint_16 height)
{
    BrDemoDefaultOnResize(demo, width, height);

    /*
     * Restore the palette.
     */
    BrPixelmapPaletteSet(demo->colour_buffer, demo->palette);
}

// Matrix multiply for top 3x3 only

#define A(x, y) A->m[x][y]
#define B(x, y) B->m[x][y]
#define C(x, y) C->m[x][y]

static void Matrix33Mul(br_matrix34 *A, br_matrix34 *B, br_matrix34 *C)
{
    A(0, 0) = BR_MAC3(B(0, 0), C(0, 0), B(0, 1), C(1, 0), B(0, 2), C(2, 0));
    A(0, 1) = BR_MAC3(B(0, 0), C(0, 1), B(0, 1), C(1, 1), B(0, 2), C(2, 1));
    A(0, 2) = BR_MAC3(B(0, 0), C(0, 2), B(0, 1), C(1, 2), B(0, 2), C(2, 2));

    A(1, 0) = BR_MAC3(B(1, 0), C(0, 0), B(1, 1), C(1, 0), B(1, 2), C(2, 0));
    A(1, 1) = BR_MAC3(B(1, 0), C(0, 1), B(1, 1), C(1, 1), B(1, 2), C(2, 1));
    A(1, 2) = BR_MAC3(B(1, 0), C(0, 2), B(1, 1), C(1, 2), B(1, 2), C(2, 2));

    A(2, 0) = BR_MAC3(B(2, 0), C(0, 0), B(2, 1), C(1, 0), B(2, 2), C(2, 0));
    A(2, 1) = BR_MAC3(B(2, 0), C(0, 1), B(2, 1), C(1, 1), B(2, 2), C(2, 1));
    A(2, 2) = BR_MAC3(B(2, 0), C(0, 2), B(2, 1), C(1, 2), B(2, 2), C(2, 2));
}

const static br_demo_dispatch dispatch = {
    .init          = FogInit,
    .process_event = FogProcessEvent,
    .update        = FogUpdate,
    .render        = FogRender,
    .on_resize     = FogOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("Fog Demo", &dispatch, argc, argv);
}
