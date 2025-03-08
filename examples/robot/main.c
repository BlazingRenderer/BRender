/*
 * BRender Robot Example
 *
 * A rewrite of the classic robot example.
 */
#include <brender.h>
#include <brdemo.h>

#define MAXFRAMES     100
#define MAXTRANSFORMS 2000

const float ANIM_FPS   = 1.0f / 30.0f;
const float CAM_XSPEED = 3.0f;
const float CAM_YSPEED = 3.0f;

typedef struct br_demo_robot {
    br_actor *actor;
    br_vue   *vue;

    br_actor  *camera_pivot;
    br_actor  *camera;
    br_vector2 cam_robot;

    br_scalar accumulator;
} br_demo_robot;

static br_error RobotInit(br_demo *demo)
{
    br_demo_robot *robot;
    br_actor      *light;
    br_light      *light_data;
    br_material   *materials[20];
    br_uint_32     nmaterials;
    br_model      *models[50];
    br_uint_32     nmodels;
    br_camera     *camera_data;

    /*
     * If indexed, load and set the palette.
     */
    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        br_pixelmap *std_pal, *shade_table;

        if((std_pal = BrPixelmapLoad("os2pal.pal")) == NULL) {
            BrLogError("DEMO", "Unable to load std.pal");
            return BRE_FAIL;
        }

        BrPixelmapPaletteSet(demo->colour_buffer, std_pal);

        if((shade_table = BrPixelmapLoad("os2shade.tab")) == NULL) {
            BrLogError("DEMO", "Unable to load os2shade.tab");
            return BRE_FAIL;
        }
        BrTableAdd(shade_table);
    }

    if((nmaterials = BrFmtScriptMaterialLoadMany("robo.msc", materials, BR_ASIZE(materials))) == 0) {
        BrLogError("DEMO", "Unable to load robo.msc");
        return BRE_FAIL;
    }
    BrMaterialAddMany(materials, nmaterials);
    BrLogInfo("DEMO", "Loaded %u materials from robo.msc", nmaterials);

    if((nmodels = BrModelLoadMany("robo.dat", models, BR_ASIZE(models))) == 0) {
        BrLogError("DEMO", "Unable to load robo.dat");
        return BRE_FAIL;
    }
    BrModelAddMany(models, nmodels);
    BrLogInfo("DEMO", "Loaded %u models from robo.dat", nmodels);

    /*
     * HACKFIX: Set this so BrModelUpdate() doesn't choke.
     * Figure out why.
     */
    for(int i = 0; i < nmodels; ++i)
        models[i]->flags |= BR_MODF_UPDATEABLE;

    robot      = BrResAllocate(demo, sizeof(br_demo_robot), BR_MEMORY_APPLICATION);
    demo->user = robot;

    robot->accumulator = 0.0f;

    if((robot->actor = BrActorLoad("robo.act")) == NULL) {
        BrLogError("DEMO", "Unable to load robo.act");
        return BRE_FAIL;
    }
    BrLogInfo("DEMO", "Loaded robo.act");

    BrActorAdd(demo->world, robot->actor);

    robot->vue = BrVueAllocate(MAXFRAMES, MAXTRANSFORMS);
    BrResAdd(robot, robot->vue);

    BrLoadVUE("robo.vue", robot->actor, robot->vue);

    light = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));

    light_data                = light->type_data;
    light_data->type          = BR_LIGHT_DIRECT;
    light_data->attenuation_c = BR_SCALAR(1.0);

    BrMatrix34RotateY(&light->t.t.mat, BR_ANGLE_DEG(45.0));
    BrMatrix34PostRotateZ(&light->t.t.mat, BR_ANGLE_DEG(45.0));
    BrLightEnable(light);

    /*
     * Create the camera
     */
    robot->camera_pivot = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_NONE, NULL));
    robot->camera       = BrActorAdd(robot->camera_pivot, BrActorAllocate(BR_ACTOR_CAMERA, NULL));

    camera_data                = robot->camera->type_data;
    camera_data->type          = BR_CAMERA_PERSPECTIVE_OLD;
    camera_data->field_of_view = BR_ANGLE_DEG(45.0);
    camera_data->hither_z      = BR_SCALAR(50.0);
    camera_data->yon_z         = BR_SCALAR(3000.0);
    camera_data->aspect        = BR_SCALAR(1.8);
    BrMatrix34Translate(&robot->camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(1700));

    demo->camera             = robot->camera;
    demo->order_table->min_z = camera_data->hither_z;
    demo->order_table->max_z = camera_data->yon_z;

    demo->clear_colour = BR_COLOUR_RGB(66, 66, 66);

    /*
     * Robotate the model to face the camera.
     */
    BrMatrix34RotateX(&robot->actor->t.t.mat, BR_ANGLE_DEG(-90.0));

    BrVector2Set(&robot->cam_robot, BR_SCALAR(0), BR_SCALAR(0));
    return BRE_OK;
}

static void RobotUpdate(br_demo *demo, br_scalar dt)
{
    br_demo_robot *robot = demo->user;

    robot->accumulator += dt;

    /*
     * Apply the animation once we've hit our accumulator.
     */
    while(robot->accumulator >= ANIM_FPS) {
        BrApplyVue(robot->vue, robot->actor);
        robot->accumulator -= ANIM_FPS;
    }
}

static void RobotProcessEvent(br_demo *demo, const SDL_Event *evt)
{
    br_demo_robot *robot = demo->user;

    switch(evt->type) {
        case SDL_KEYDOWN: {
            switch(evt->key.keysym.sym) {
                case SDLK_SPACE:
                    BrMatrix34Identity(&robot->camera_pivot->t.t.mat);
                    break;
                case 'q':
                    SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT});
                    break;
            }
            break;
        }

        case SDL_MOUSEMOTION: {
            if(evt->motion.state & SDL_BUTTON_LMASK) {
                br_angle ax, ay;
                /* NB: dt is already accounted for in xrel/yrel. */
                robot->cam_robot.v[0] -= ((float)evt->motion.xrel * CAM_XSPEED) / (float)demo->colour_buffer->width;
                robot->cam_robot.v[0] = fmodf(robot->cam_robot.v[0], 1.0f);

                robot->cam_robot.v[1] -= ((float)evt->motion.yrel * CAM_YSPEED) / (float)demo->colour_buffer->height;
                robot->cam_robot.v[1] = fmodf(robot->cam_robot.v[1], 1.0f);

                ax = BrScalarToAngle(BrFloatToScalar(robot->cam_robot.v[0]));
                ay = BrScalarToAngle(BrFloatToScalar(robot->cam_robot.v[1]));

                BrMatrix34RotateY(&robot->camera_pivot->t.t.mat, ax);
                BrMatrix34PreRotateX(&robot->camera_pivot->t.t.mat, ay);
            }
            break;
        }
    }
}

const static br_demo_dispatch dispatch = {
    .init          = RobotInit,
    .process_event = RobotProcessEvent,
    .update        = RobotUpdate,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRun("BRender Robot Demo", 1280, 720, &dispatch);
}
