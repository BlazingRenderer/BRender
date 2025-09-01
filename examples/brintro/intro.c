/**
 * \brief Implementation of the BRender intro sequence.
 */
#include "intro.h"

#define INTRO_NUM_MODELS 2              /** \brief brender, ball. */
#define INTRO_NUM_PIX    4              /** \brief 2 textures, palette, shade table */
#define INTRO_ANIM_SIZE  98             /** \brief Number of frames of animation in anim file. */
#define INTRO_DATFILE    "intro.dat"    /** \brief Name of the model file. */
#define INTRO_PIXFILE    "intro.pix"    /** \brief Name of the pixelmap file. */
#define INTRO_ANIMFILE   "intro.anm"    /** \brief Name of the animation file. */
#define INTRO_ANIM_SPF   (1.0f / 25.0f) /** \brief 1/(animation fps). */
#define INTRO_WAIT_TIME  1.0f           /** \brief Time to wait after the animation, in seconds. */
#define INTRO_FADE_TIME  0.5f           /** \brief Fade-out speed, in seconds. */

enum {
    INTRO_STATE_ANIM,
    INTRO_STATE_WAIT,
    INTRO_STATE_FADE,
    INTRO_STATE_DONE,
};

struct br_intro {
    br_actor *world;
    br_actor *camera;

    br_actor       *light;
    br_actor       *word;
    br_actor       *sphere;
    br_order_table *order_table;
    br_matrix34    *animation_sequence;

    br_actor *fade;

    float     accum;
    br_size_t frame_index;
    float     fade_opacity;
    int       state;
};

static int load_pixelmaps(void)
{
    br_pixelmap *pixtable[INTRO_NUM_PIX];
    br_pixelmap *palette;
    br_uint_16   npix;

    /*
     * Load the pixelmaps, doing the song and dance with the palettes.
     */
    npix = BrPixelmapLoadMany(INTRO_PIXFILE, pixtable, INTRO_NUM_PIX);
    BrMapAddMany(pixtable, npix);

    if(npix != INTRO_NUM_PIX)
        return -1;

    if((palette = BrMapFind("intro_palette")) == NULL)
        return -1;

    for(size_t i = 0; i < INTRO_NUM_PIX; ++i) {
        if(pixtable[i] == palette)
            continue;

        if(pixtable[i]->type == BR_PMT_INDEX_8 && pixtable[i]->map == NULL) {
            pixtable[i]->map = palette;
            BrMapUpdate(pixtable[i], BR_MAPU_ALL);
        }
    }

    return 0;
}

static br_material *create_word_material(void)
{
    br_material *mat = BrMaterialAllocate("intro_word");
    // mat->colour      = BR_COLOUR_RGB(0, 0, 0);
    // mat->opacity     = 255;
    mat->ka    = BR_UFRACTION(0.20);
    mat->kd    = BR_UFRACTION(0.60);
    mat->ks    = BR_UFRACTION(0.0);
    mat->power = BR_SCALAR(0.0);
    mat->flags |= BR_MATF_LIGHT;
    mat->mode          = BR_MATM_DEPTH_TEST_LT;
    mat->map_transform = (br_matrix23){
        {BR_VECTOR2(1, 0), BR_VECTOR2(0, 1), BR_VECTOR2(0, 0)}
    };
    mat->index_base  = 0;
    mat->index_range = 63;
    return mat;
}

static br_material *create_xplos_mat(void)
{
    br_material *mat = BrMaterialAllocate("intro_xplos");
    // mat->colour      = BR_COLOUR_RGB(0, 0, 0);
    // mat->opacity     = 255;
    mat->ka    = BR_UFRACTION(0.20);
    mat->kd    = BR_UFRACTION(0.65);
    mat->ks    = BR_UFRACTION(0.30);
    mat->power = BR_SCALAR(20);
    mat->flags |= BR_MATF_LIGHT | BR_MATF_SMOOTH;
    mat->map_transform = (br_matrix23){
        {BR_VECTOR2(1, 0), BR_VECTOR2(0, 1), BR_VECTOR2(0, 0)}
    };
    mat->index_base  = 0;
    mat->index_range = 63;
    return mat;
}

static int setup_materials(void)
{
    br_material *material;

    material = create_word_material();
    material->flags &= ~BR_MATF_SMOOTH;

    if((material->colour_map = BrMapFind("intro_word")) == NULL)
        return -1;

    if((material->index_shade = BrMapFind("intro_shade_table")) == NULL)
        return -1;

    BrMaterialAdd(material);

    material = create_xplos_mat();
    if((material->colour_map = BrMapFind("intro_xplos")) == NULL)
        return -1;

    material->index_shade = BrMapFind("intro_shade_table");
    BrMaterialAdd(material);

    material          = BrMaterialAllocate("intro_fade");
    material->colour  = BR_COLOUR_RGB(0, 0, 0);
    material->opacity = 0;
    material->flags |= BR_MATF_DISABLE_COLOUR_KEY;
    material->flags &= ~BR_MATF_LIGHT;
    BrMaterialAdd(material);
    return 0;
}

static int load_model(void)
{
    br_uint_32 nmodel;
    br_model  *mtable[INTRO_NUM_MODELS];

    nmodel = BrModelLoadMany(INTRO_DATFILE, mtable, INTRO_NUM_MODELS);
    BrModelAddMany(mtable, nmodel);

    if(nmodel != INTRO_NUM_MODELS)
        return -1;

    return 0;
}

static br_matrix34 *load_anim(void *res)
{
    br_size_t    anim_size;
    br_matrix34 *animation_sequence;

    anim_size = 0;
    if((animation_sequence = BrFileLoad(res, INTRO_ANIMFILE, &anim_size)) == NULL)
        return NULL;

    if(anim_size != INTRO_ANIM_SIZE * 2 * sizeof(br_matrix34)) {
        BrResFree(animation_sequence);
        return NULL;
    }

    for(size_t i = 0; i < INTRO_ANIM_SIZE * 2; ++i) {
        BrMatrix34FixedToFloat((br_matrix34_f *)(animation_sequence + i), (const br_matrix34_x *)(animation_sequence + i));
    }

    return animation_sequence;
}

static br_uint_32 BR_CALLBACK ModelEnum(br_model *item, void *arg)
{
    BrModelFree(BrModelRemove(item));

    return 0;
}

static br_uint_32 BR_CALLBACK MaterialEnum(br_material *item, void *arg)
{
    BrMaterialFree(BrMaterialRemove(item));

    return 0;
}

static br_uint_32 BR_CALLBACK MapEnum(br_pixelmap *item, void *arg)
{
    BrPixelmapFree(BrMapRemove(item));

    return 0;
}

void BrIntroDestroy(br_intro *state)
{
    BrModelEnum("intro_*", ModelEnum, NULL);
    BrMaterialEnum("intro_*", MaterialEnum, NULL);
    BrMapEnum("intro_*", MapEnum, NULL);

    if(state == NULL)
        return;

    if(state->world != NULL) {
        if(state->world->prev != NULL)
            BrActorRemove(state->world);

        BrActorFree(state->world);
    }

    BrResFree(state);
}

void BrIntroReset(br_intro *state)
{
    state->accum                   = INTRO_ANIM_SPF;
    state->frame_index             = 0;
    state->fade_opacity            = 0.0f;
    state->fade->material->opacity = 0;
    BrMaterialUpdate(state->fade->material, BR_MATU_ALL);
    state->state = INTRO_STATE_ANIM;
}

br_intro *BrIntroCreate(void)
{
    br_intro  *state;
    br_camera *camdata;

    if(load_pixelmaps() < 0) {
        BrIntroDestroy(NULL);
        return NULL;
    }

    if(setup_materials() < 0) {
        BrIntroDestroy(NULL);
        return NULL;
    }

    if(load_model() < 0) {
        BrIntroDestroy(NULL);
        return NULL;
    }

    state  = BrResAllocate(NULL, sizeof(br_intro), BR_MEMORY_APPLICATION);
    *state = (br_intro){
        .accum       = INTRO_ANIM_SPF, /* Set this initially to trigger the update immediately. */
        .frame_index = 0,
        .state       = INTRO_STATE_ANIM,
    };

    if((state->animation_sequence = load_anim(state)) == NULL) {
        BrIntroDestroy(state);
        return NULL;
    }

    state->world  = BrActorAllocate(BR_ACTOR_NONE, NULL);
    state->camera = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    state->light  = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));

    state->sphere = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    state->word   = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    state->fade   = BrActorAdd(state->world, BrActorAllocate(BR_ACTOR_MODEL, NULL));

    state->order_table = BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_LEAVE_BOUNDS | BR_ORDER_TABLE_BUCKET_SORT, BR_SORT_AVERAGE);
    BrZsActorOrderTableSet(state->world, state->order_table);

    state->word->model    = BrModelFind("intro_word");
    state->sphere->model  = BrModelFind("intro_sphere");
    state->fade->material = BrMaterialFind("intro_fade");

    /*
     * setup camera, orientation of light (start, over left shoulder)
     */
    camdata           = state->camera->type_data;
    camdata->hither_z = BR_SCALAR(0.01);

    state->order_table->min_z = camdata->hither_z;
    state->order_table->max_z = camdata->yon_z;

    BrMatrix34Translate(&state->camera->t.t.mat, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(2));
    BrMatrix34RotateY(&state->light->t.t.mat, BR_ANGLE_DEG(-45));
    BrMatrix34PostRotateZ(&state->light->t.t.mat, BR_ANGLE_DEG(45));
    BrLightEnable(state->light);

    BrIntroReset(state);
    return state;
}

void BrIntroSetAspect(br_intro *state, br_scalar aspect)
{
    br_camera *camdata = (br_camera *)state->camera->type_data;
    camdata->aspect    = aspect;
}

void BrIntroUpdate(br_intro *state, float dt)
{
    switch(state->state) {
        case INTRO_STATE_ANIM:
            /*
             * Apply the animation once we've hit our accumulator.
             */
            state->accum += dt;
            while(state->accum >= INTRO_ANIM_SPF) {
                state->accum -= INTRO_ANIM_SPF;
                BrMatrix34Copy(&state->word->t.t.mat, state->animation_sequence + (2 * state->frame_index));
                BrMatrix34Copy(&state->sphere->t.t.mat, state->animation_sequence + ((2 * state->frame_index) + 1));

                state->frame_index++;
                if(state->frame_index == INTRO_ANIM_SIZE) {
                    state->accum = 0.0f;
                    state->state = INTRO_STATE_WAIT;
                }
                break;
            }
            break;

        case INTRO_STATE_WAIT:
            state->accum += dt;
            if(state->accum >= INTRO_WAIT_TIME) {
                state->accum = 0.0f;
                state->state = INTRO_STATE_FADE;
            }
            break;

        case INTRO_STATE_FADE:
            state->fade_opacity += dt / INTRO_FADE_TIME;

            if(state->fade_opacity >= 1.0f) {
                state->fade->material->opacity = 255;
                state->state                   = INTRO_STATE_DONE;
            } else {
                state->fade->material->opacity = (br_uint_8)(state->fade_opacity * 255);
            }

            BrMaterialUpdate(state->fade->material, BR_MATU_ALL);
            break;

        case INTRO_STATE_DONE:
        default:
            // BrIntroReset(state);
            break;
    }
}

br_actor *BrIntroGetWorld(br_intro *state)
{
    return state->world;
}

br_actor *BrIntroGetCamera(br_intro *state)
{
    return state->camera;
}

void BrIntroForceFade(br_intro *state)
{
    if(state->state == INTRO_STATE_ANIM)
        state->state = INTRO_STATE_FADE;
}

br_boolean BrIntroIsDone(br_intro *state)
{
    return state->state == INTRO_STATE_DONE;
}
