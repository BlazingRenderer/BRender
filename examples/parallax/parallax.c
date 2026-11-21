/*
 * Parallax mapping, self-contained: the brick wall and its normal map are generated here,
 * so the example needs nothing on disk and shows the whole of the material's side of the
 * feature.
 *
 * What it demonstrates is the two fields and the sign convention between them:
 *
 *   bump_map    the tangent-space normal map. Its RGB perturbs the shading normal; its
 *               alpha is a *height*: 1 is the outermost point of the surface, 0 the
 *               innermost.
 *   bump_scale  how far the map displaces the normal. 0 leaves it a plain texture.
 *   bump_depth  how far the height displaces the sampled co-ordinate, in the bump map's
 *               own UV units. 0 leaves the texture flat. A *negative* depth describes a
 *               map whose alpha is a depth (white is far), which is how several tutorials
 *               ship theirs; the same arithmetic then mirrors the offset.
 *
 * Keys, while the window has focus:
 *
 *   B  bind and unbind the bump map           (depth 0 and no map draw the same frame)
 *   N  bump_scale between 1 and 0             (the shading, not the offset)
 *   M  bump_depth through 0, +0.02 and -0.02  (the offset, and the sign that mirrors it)
 */
#include <math.h>
#include <stddef.h>
#include <brdemo.h>

#include "editorcam.h"

#define MAP_SIZE 256
#define COURSES  6
#define DEPTH    0.02f

typedef struct br_parallax_state {
    br_editor_camera *editorcam;

    br_pixelmap *albedo;
    br_pixelmap *bump;
    br_material *mat;
    br_actor    *actor;
    br_model    *model;

    int depth_step;
} br_parallax_state;

typedef struct KeyMap {
    SDL_Keycode key;
    br_uint_32  flag;
} KeyMap;

const static KeyMap keymap[] = {
    {.key = SDLK_W,      .flag = BR_ECAMF_MOVE_FORWARD},
    {.key = SDLK_S,      .flag = BR_ECAMF_MOVE_BACK   },
    {.key = SDLK_A,      .flag = BR_ECAMF_MOVE_LEFT   },
    {.key = SDLK_D,      .flag = BR_ECAMF_MOVE_RIGHT  },
    {.key = SDLK_Q,      .flag = BR_ECAMF_MOVE_DOWN   },
    {.key = SDLK_E,      .flag = BR_ECAMF_MOVE_UP     },
    {.key = SDLK_LSHIFT, .flag = BR_ECAMF_FASTER      },
};

typedef struct ButtonMap {
    Uint8      button;
    br_uint_32 flag;
} ButtonMap;

const static ButtonMap buttonmap[] = {
    {.button = SDL_BUTTON_MIDDLE, .flag = BR_ECAMF_MODE_PAN },
    {.button = SDL_BUTTON_RIGHT,  .flag = BR_ECAMF_MODE_LOOK},
};

/*
 * One brick field drives all three channels, so the mortar that is drawn and the mortar
 * that is carved cannot disagree. Returns the height in [0,1] and the colour, both from
 * the same distance into the brick.
 */
static void brick_sample(double u, double v, double *h, double *r, double *g, double *b)
{
    const double course   = 1.0 / COURSES;
    const double length   = 2.0 / COURSES;
    const double mortar_u = 0.04, mortar_v = 0.10;
    int          row, col;
    double       uu, ul, vl, tint;

    row = (int)(v / course);
    vl  = v / course - row;

    uu  = u / length + ((row & 1) ? 0.5 : 0.0);
    col = (int)uu;
    ul  = uu - col;

    if(ul < mortar_u || ul > 1.0 - mortar_u || vl < mortar_v || vl > 1.0 - mortar_v) {
        *h = 0.0;
        *r = 0.62;
        *g = 0.59;
        *b = 0.52;
        return;
    }

    /* A tint per brick, so the courses read as bricks and not as a pattern. */
    tint = 0.72 + 0.28 * fmod((double)((row * 7 + col * 13) % 11) / 11.0, 1.0);

    *h = 0.85;
    *r = 0.80 * tint;
    *g = 0.30 * tint;
    *b = 0.22 * tint;
}

/* The albedo: the field's colour, opaque. */
static br_pixelmap *make_albedo(void)
{
    br_pixelmap *pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, MAP_SIZE, MAP_SIZE, NULL, BR_PMAF_NORMAL);

    if(pm == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, "parallax-albedo");

    for(int y = 0; y < MAP_SIZE; ++y) {
        br_uint_8 *px = (br_uint_8 *)pm->pixels + (br_size_t)y * pm->row_bytes;

        for(int x = 0; x < MAP_SIZE; ++x) {
            double h, r, g, b;

            /* A host 888 map is byte-ordered B,G,R in memory. */
            brick_sample((double)x / MAP_SIZE, (double)y / MAP_SIZE, &h, &r, &g, &b);
            px[x * 4 + 0] = (br_uint_8)(b * 255.0 + 0.5);
            px[x * 4 + 1] = (br_uint_8)(g * 255.0 + 0.5);
            px[x * 4 + 2] = (br_uint_8)(r * 255.0 + 0.5);
            px[x * 4 + 3] = 255;
        }
    }

    BrMapAdd(pm);
    return pm;
}

/*
 * The bump map: the same field differentiated for the normal, and the field itself in
 * alpha, which is where the parallax reads its height. RGBA and not RGBX, because a
 * renderer swizzles an RGBX map's alpha to one and the height never arrives.
 */
static br_pixelmap *make_bump(void)
{
    br_pixelmap *pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, MAP_SIZE, MAP_SIZE, NULL, BR_PMAF_NORMAL);

    if(pm == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, "parallax-bump");

    for(int y = 0; y < MAP_SIZE; ++y) {
        br_uint_8 *px = (br_uint_8 *)pm->pixels + (br_size_t)y * pm->row_bytes;

        for(int x = 0; x < MAP_SIZE; ++x) {
            const double u = (double)x / MAP_SIZE, v = (double)y / MAP_SIZE, d = 1.0 / MAP_SIZE;
            double       h, hl, hr, hd, hu, r, g, b, nx, ny, nz, len;

            brick_sample(u, v, &h, &r, &g, &b);
            brick_sample(u - d, v, &hl, &r, &g, &b);
            brick_sample(u + d, v, &hr, &r, &g, &b);
            brick_sample(u, v - d, &hd, &r, &g, &b);
            brick_sample(u, v + d, &hu, &r, &g, &b);

            nx = -(hr - hl) / (2.0 * d) * 0.004;
            ny = -(hu - hd) / (2.0 * d) * 0.004;
            nz = 1.0;

            len = sqrt(nx * nx + ny * ny + nz * nz);
            nx /= len;
            ny /= len;
            nz /= len;

            /* A host 888 map is byte-ordered B,G,R, so x goes in the third byte. */
            px[x * 4 + 0] = (br_uint_8)((nz * 0.5 + 0.5) * 255.0 + 0.5);
            px[x * 4 + 1] = (br_uint_8)((ny * 0.5 + 0.5) * 255.0 + 0.5);
            px[x * 4 + 2] = (br_uint_8)((nx * 0.5 + 0.5) * 255.0 + 0.5);
            px[x * 4 + 3] = (br_uint_8)(h * 255.0 + 0.5);
        }
    }

    BrMapAdd(pm);
    return pm;
}

/* A quad in the z=0 plane, its map coordinate running with x and y, so u = (x+1)/2. */
static br_model *make_quad(void)
{
    br_model *model = BrModelAllocate("parallax-quad", 4, 2);

    if(model == NULL)
        return NULL;

    for(int i = 0; i < 4; ++i) {
        br_vertex *v = &model->vertices[i];

        BrVector3Set(&v->p, BR_SCALAR(i == 0 || i == 3 ? -1.0 : 1.0), BR_SCALAR(i < 2 ? -1.0 : 1.0), BR_SCALAR(0.0));
        v->map.v[0] = i == 0 || i == 3 ? BR_SCALAR(0.0) : BR_SCALAR(1.0);
        v->map.v[1] = i < 2 ? BR_SCALAR(0.0) : BR_SCALAR(1.0);
        v->n.v[0]   = BrScalarToFraction(BR_SCALAR(0.0));
        v->n.v[1]   = BrScalarToFraction(BR_SCALAR(0.0));
        v->n.v[2]   = BrScalarToFraction(BR_SCALAR(1.0));
    }

    model->flags |= BR_MODF_CUSTOM_NORMALS;

    model->faces[0].vertices[0] = 0;
    model->faces[0].vertices[1] = 1;
    model->faces[0].vertices[2] = 2;

    model->faces[1].vertices[0] = 0;
    model->faces[1].vertices[1] = 2;
    model->faces[1].vertices[2] = 3;

    BrModelUpdate(model, BR_MODU_ALL);
    return model;
}

static br_error ParallaxInit(br_demo *demo)
{
    br_parallax_state *state;
    br_material       *mat;

    state = BrResAllocate(demo, sizeof(*state), BR_MEMORY_APPLICATION);

    state->albedo = make_albedo();
    state->bump   = make_bump();
    state->model  = make_quad();

    if(state->albedo == NULL || state->bump == NULL || state->model == NULL)
        return BRE_FAIL;

    mat = BrMaterialAllocate("parallax");
    BrMaterialAdd(mat);

    mat->colour     = BR_COLOUR_RGB(255, 255, 255);
    mat->opacity    = 255;
    mat->ka         = BR_SCALAR(0.25);
    mat->kd         = BR_SCALAR(0.85);
    mat->ks         = BR_SCALAR(0.0);
    mat->flags      = BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE;
    mat->colour_map = state->albedo;
    mat->bump_map   = state->bump;
    mat->bump_scale = BR_SCALAR(1.0);
    mat->bump_depth = BR_FRACTION(DEPTH);
    mat->mode       = (mat->mode & ~BR_MATM_SHADING_MODE_MASK) | BR_MATM_SHADING_MODE_PHONG;

    BrMatrix23Identity(&mat->map_transform);
    BrMatrix23Identity(&mat->bump_map_transform);
    BrMaterialUpdate(mat, BR_MATU_ALL);

    BrLightEnable(BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL)));

    state->actor           = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    state->actor->model    = state->model;
    state->actor->material = mat;

    /*
     * Put the quad in front of the camera.
     */
    BrMatrix34Translate(&state->actor->t.t.mat, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(-3));

    state->mat = mat;

    state->editorcam                 = BrEditorCamAllocate(state);
    state->editorcam->standard_speed = 2.0f;
    demo->camera                     = state->editorcam->camera;
    BrActorAdd(demo->world, state->editorcam->actor);

    demo->order_table->min_z = state->editorcam->camera_data.hither_z;
    demo->order_table->max_z = state->editorcam->camera_data.yon_z;

    demo->user = state;
    return BRE_OK;
}

static void ParallaxUpdate(br_demo *demo, br_scalar dt)
{
    br_parallax_state *state = demo->user;

    BrEditorCamUpdate(state->editorcam, dt);
}

/* The editor camera's own input, so the example can be orbited while the keys work. */
static void EditorCamProcessEvent(br_demo *demo, br_editor_camera *cam, const SDL_Event *evt)
{
    (void)demo;

    switch(evt->type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            for(size_t i = 0; i < BR_ASIZE(keymap); ++i) {
                if(evt->key.key == keymap[i].key) {
                    if(evt->type == SDL_EVENT_KEY_DOWN)
                        cam->flags |= keymap[i].flag;
                    else
                        cam->flags &= ~keymap[i].flag;
                }
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            for(size_t i = 0; i < BR_ASIZE(buttonmap); ++i) {
                if(evt->button.button == buttonmap[i].button) {
                    if(evt->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                        cam->flags |= buttonmap[i].flag;
                    else
                        cam->flags &= ~buttonmap[i].flag;
                }
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            cam->mx_rel += evt->motion.xrel;
            cam->my_rel += evt->motion.yrel;
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            cam->mwheel_rel += BR_SCALAR(evt->wheel.y / 4.0f);
            break;

        default:
            break;
    }
}

static void ParallaxProcessEvent(br_demo *demo, const SDL_Event *evt)
{
    br_parallax_state *state = demo->user;

    if(evt->type != SDL_EVENT_KEY_DOWN) {
        EditorCamProcessEvent(demo, state->editorcam, evt);
        return;
    }

    switch(evt->key.key) {
        case SDLK_ESCAPE:
            SDL_PushEvent(&(SDL_Event){.type = SDL_EVENT_QUIT});
            break;

        case SDLK_B:
            state->mat->bump_map = state->mat->bump_map != NULL ? NULL : state->bump;
            BrMaterialUpdate(state->mat, BR_MATU_BUMPMAP);
            BrLogInfo("APP", "bump map %s", state->mat->bump_map != NULL ? "bound" : "unbound");
            break;

        case SDLK_N:
            state->mat->bump_scale = state->mat->bump_scale == BR_SCALAR(0.0) ? BR_SCALAR(1.0) : BR_SCALAR(0.0);
            BrMaterialUpdate(state->mat, BR_MATU_BUMPMAP);
            BrLogInfo("APP", "bump_scale %f", (double)BrScalarToFloat(state->mat->bump_scale));
            break;

        case SDLK_M: {
            static const br_fraction depths[] = {BR_FRACTION(0.0), BR_FRACTION(DEPTH), BR_FRACTION(-DEPTH)};

            state->depth_step      = (state->depth_step + 1) % (int)BR_ASIZE(depths);
            state->mat->bump_depth = depths[state->depth_step];
            BrMaterialUpdate(state->mat, BR_MATU_BUMPMAP);
            BrLogInfo("APP", "bump_depth %f", (double)BrScalarToFloat(BrFractionToScalar(state->mat->bump_depth)));
            break;
        }

        default:
            break;
    }

    EditorCamProcessEvent(demo, state->editorcam, evt);
}

static void ParallaxRender(br_demo *demo)
{
    br_parallax_state *state = demo->user;

    BrDemoDefaultRender(demo);

    {
        int line_height = BrPixelmapTextHeight(demo->colour_buffer, BrFontProp7x9);
        line_height += line_height >> 1;

        int x = -demo->colour_buffer->origin_x + 10;
        int y = -demo->colour_buffer->origin_y + line_height * 2;
        BrPixelmapTextF(demo->colour_buffer, x, y, 0xFFFFFFFF, BrFontProp7x9, "[B] Bump Enabled: %d", !!state->mat->bump_map);
        y += line_height;
        BrPixelmapTextF(demo->colour_buffer, x, y, 0xFFFFFFFF, BrFontProp7x9, "[N] Bump Scale:    %f", state->mat->bump_scale);
        y += line_height;
        BrPixelmapTextF(demo->colour_buffer, x, y, 0xFFFFFFFF, BrFontProp7x9, "[M] Bump Depth:   %f", state->mat->bump_depth);
    }
}

const static br_demo_dispatch dispatch = {
    .init          = ParallaxInit,
    .process_event = ParallaxProcessEvent,
    .update        = ParallaxUpdate,
    .render        = ParallaxRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("BRender Parallax Mapping", &dispatch, argc, argv);
}
