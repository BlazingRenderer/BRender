/****************************************************************************
 *                                                                          *
 * SOURCE FILE NAME: Robot.c                                                *
 *                                                                          *
 * DESCRIPTIVE NAME: Manage a 3D world and the views into it                *
 *                                                                          *
 * COMPONENT: Robot                                                         *
 *                                                                          *
 * COPYRIGHT: (C) Copyright Argonaut Software. 1994.  All rights reserved.  *
 *            (C) Copyright IBM, 1995.  All rights reserved.                *
 *                                                                          *
 * FUNCTIONS:                                                               *
 *    LoadVUE                                                               *
 *    ApplyVUE                                                              *
 *    WorldAllocate                                                         *
 *    WorldFree                                                             *
 *    WorldUpdate                                                           *
 *    ViewBufferSet                                                         *
 *    ViewRender                                                            *
 *    ViewScreenUpdate                                                      *
 *                                                                          *
 * BRENDER FUNCTIONS                                                        *
 *    BrFloatToScalar                                                       *
 *    BrIntToScalar                                                         *
 *    BrScalarToFixed                                                       *
 *    BrActorAdd                                                            *
 *    BrActorAllocate                                                       *
 *    BrActorLoad                                                           *
 *    BrActorSearch                                                         *
 *    BrFmtScriptMaterialLoadMany                                           *
 *    BrLightEnable                                                         *
 *    BrMapFindHook                                                         *
 *    BrMaterialAddMany                                                     *
 *    BrMatrix34RotateX                                                     *
 *    BrMatrix34Translate                                                   *
 *    BrMatrix34RotateY                                                     *
 *    BrMatrix34PostRotateZ                                                 *
 *    BrMemAllocate                                                         *
 *    BrModelLoadMany                                                       *
 *    BrModelAddMany                                                        *
 *    BrPixelmapFree                                                        *
 *    BrPixelmapAllocate                                                    *
 *    BrPixelmapMatch                                                       *
 *    BrPixelmapFill                                                        *
 *    BrPixelmapRectangleFill                                               *
 *    BrTableFindHook                                                       *
 *    BrZbSetRenderBoundsCallback                                           *
 *    BrZbSceneRender                                                       *
 *                                                                          *
 ****************************************************************************/

#include <stdio.h>                /* Standard I/O                           */
#include <string.h>               /* String information                     */

#include <brender.h>              /* Master include file for BRender        */
#include <brassert.h>             /* Assertion macros                       */
#include <brsdl.h>

#include "world.h"                /* 3D world and views into it             */
#include "app.h"                  /* Application prototypes and defines     */

/*
 * Forward declarations
 */
static void SetupTexture(char *material,
                         char *pix_file,
                         br_pixelmap *shade_table);

static void BR_CALLBACK ViewBoundsCallback(br_actor *actor,
                                           br_model *model,
                                           br_material *material,
                                           br_uint_8 style,
                                           br_matrix4 *model_to_screen,
                                           br_int_32 *bounds);

/*
 * Dark green background
 */
#define BACK_COLOUR 80

#define MAXFRAMES 100
#define MAXTRANSFORMS 2000

static br_pixelmap *BR_CALLBACK mapFindFailedLoad(const char *name)
{
    br_pixelmap *pm, *pm2;

    if((pm = BrPixelmapLoad(name)) != NULL) {
        if((pm2 = BrPixelmapDeCLUT(pm)) != NULL) {
            BrResFree(pm->map);
            BrResFree(pm);
            pm = pm2;
        }

        if(pm->identifier != NULL)
            BrResFree(pm->identifier);

        pm->identifier = BrResStrDup(pm, name);
        BrMapAdd(pm);
    }

    return pm;
}


/*
 * FUNCTION NAME:  WorldAllocate
 *
 * DESCRIPTION:    Create a new 3D world
 */
brwin_world *WorldAllocate(void)
{
    brwin_world *world;                   /* Ptr to world data structure    */
    br_camera   *camera_data;             /* Ptr to camera data             */
    br_light    *light_data;              /* Ptr to light data              */
    br_actor    *a;                       /* Ptr to robot actor heirarchy   */
    br_material *materials[20];           /* Array of pointers to materials */
    br_model    *models[50];              /* Ptr to array of model data     */
    int         nmodels;                  /* Number of models               */
    int         nmaterials;               /* Number of materials            */

    /*
     * Allocate world structure
     */
    world = BrMemAllocate(sizeof(*world), BR_MEMORY_APPLICATION);

    world->update = WORLD_UPDATE_FOREGROUND;

    world->root = BrActorAllocate(BR_ACTOR_NONE, NULL);

    /*
     * Load materials from material script
     */
    BrTableFindHook(BrTableFindFailedLoad);
    BrMapFindHook(mapFindFailedLoad);

    nmaterials = BrFmtScriptMaterialLoadMany("robo.msc",  /* Materials file */
                                             materials,   /* Ptr to array   */
                                             BR_ASIZE(materials)); /* Max  */

    /*
     * De-index the textures.
     */
    for(int i = 0; i < nmaterials; ++i) {

    }

    BrMaterialAddMany(materials, nmaterials);

    /*
     * Load all the models
     */
    nmodels = BrModelLoadMany("robo.dat", models, BR_ASIZE(models));

    /*
     * HACKFIX: Set this so BrModelUpdate() doesn't choke.
     * Figure out why.
     */
    for(int i = 0; i < nmodels; ++i)
        models[i]->flags |= BR_MODF_UPDATEABLE;

    BrModelAddMany(models, nmodels);

    /*
     * Load all the actors
     */
    world->actor = BrActorAdd(world->root, BrActorAllocate(BR_ACTOR_NONE, NULL));

    a = BrActorLoad("robo.act");
    BrActorAdd(world->actor, a);

    /*
     * Load the animation file
     */
    world->vue = BrVueAllocate(MAXFRAMES, MAXTRANSFORMS);
    BrLoadVUE("robo.vue", a, world->vue);

    /*
     * Turn to face camera
     */
    BrMatrix34RotateX(&world->actor->t.t.mat, BR_ANGLE_DEG(-90.0));

    /*
     * Camera
     */
    world->camera_pivot                    = BrActorAdd(world->root, BrActorAllocate(BR_ACTOR_NONE, NULL));
    world->camera_pivot->t.type            = BR_TRANSFORM_EULER;
    world->camera_pivot->t.t.euler.e.order = BR_EULER_XYZ_S;

    world->camera = BrActorAdd(world->camera_pivot, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
    camera_data = world->camera->type_data;

    camera_data->type          = BR_CAMERA_PERSPECTIVE_OLD;
    camera_data->field_of_view = BR_ANGLE_DEG(45.0);
    camera_data->hither_z      = BR_SCALAR(50.0);
    camera_data->yon_z         = BR_SCALAR(3000.0);
    camera_data->aspect        = BR_SCALAR(1.8);

    BrMatrix34Translate(&world->camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(1700));

    /*
     * Light
     */
    world->light = BrActorAdd(world->root, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    light_data = world->light->type_data;
    light_data->type          = BR_LIGHT_DIRECT;
    light_data->attenuation_c = BR_SCALAR(1.0);

    BrMatrix34RotateY(&world->light->t.t.mat, BR_ANGLE_DEG(45.0));
    BrMatrix34PostRotateZ(&world->light->t.t.mat, BR_ANGLE_DEG(45.0));

    BrLightEnable(world->light);

    return world;

} /* End of WorldAllocate() */


/*
 * FUNCTION NAME:  WorldFree
 *
 * DESCRIPTION:    Release the 3D world
 */
void WorldFree(brwin_world *world)
{
    /* XXX Relies on memory allocator to release everything */

} /* End of WorldFree() */


/*
 * FUNCTION NAME:  WorldUpdate
 *
 * DESCRIPTION:    Time based update of the 3D world
 */
void WorldUpdate(brwin_world *world)
{
    BrApplyVue(world->vue, world->actor);

} /* End of WorldUpdate () */


/*
 * FUNCTION NAME:  ViewBufferSet
 *
 * DESCRIPTION:    Setup the view buffer
 */
void ViewBufferSet(brwin_view *view)
{
    br_error result;

    /*
     * Limit the width and height
     */
    if(view->ulWidth < 2)
        view->ulWidth = 2;

    if(view->ulHeight < 2)
        view->ulHeight = 2;

    /*
     * Release any old maps
     */
    if(view->depth_buffer)
        BrPixelmapFree(view->depth_buffer);

    if(view->colour_buffer)
        BrPixelmapFree(view->colour_buffer);

    if(view->screen)
        BrPixelmapFree(view->screen);

    br_device_gl_ext_procs glprocs = BrSDLMakeGLProcs(view->window);
    result = BrDevBeginVar(&view->screen, "opengl",
                           BRT_WIDTH_I32, view->ulWidth,
                           BRT_HEIGHT_I32, view->ulHeight,
                           BRT_PIXEL_BITS_I32, 24,
                           BRT_OPENGL_EXT_PROCS_P, &glprocs,
                           BR_NULL_TOKEN
    );

    view->colour_buffer = BrPixelmapMatchSized(view->screen, BR_PMMATCH_OFFSCREEN, view->ulWidth, view->ulHeight);
    view->depth_buffer  = BrPixelmapMatch(view->colour_buffer, BR_PMMATCH_DEPTH_16);

    /*
     * Remember camera aspect ratios
     */
    view->aspect = BrScalarToFixed(BR_DIV(BrIntToScalar(view->ulWidth), BrIntToScalar(view->ulHeight)));

    /*
     * Clear buffers
     */
    BrPixelmapFill(view->colour_buffer, BR_COLOUR_RGB(0, 0, 0));
    BrPixelmapFill(view->depth_buffer, 0xffffffff);
} /* End of ViewBufferSet() */


/*
 * FUNCTION NAME: ViewRender
 *
 * DESCRIPTION:   Render an image of the world into test_colour_buffer
 *                and accumulate a list of changes.
 */
void ViewRender(brwin_view *view)
{
    /*
     * Set camera position according to controls
     */
    view->world->camera_pivot->t.t.euler.e.a = TrackingValues[MOUSETRACK_LEFT].y * 130;
    view->world->camera_pivot->t.t.euler.e.b = TrackingValues[MOUSETRACK_LEFT].x * -130;

    view->world->camera->t.t.translate.t.v[2] = BR_SCALAR(1700.0) +
                                                BrIntToScalar(TrackingValues[MOUSETRACK_RIGHT].y);

    view->world->camera->t.t.translate.t.v[0] =
        BrIntToScalar(TrackingValues[MOUSETRACK_LEFT | MOUSETRACK_RIGHT].x * -10);
    view->world->camera->t.t.translate.t.v[1] =
        BrIntToScalar(TrackingValues[MOUSETRACK_LEFT | MOUSETRACK_RIGHT].y * -10);

    /*
     * Hook dirty rectangle callback, and render view
     */
    ((br_camera *)view->world->camera->type_data)->aspect = BrFixedToScalar(view->aspect);

    BrPixelmapFill(view->colour_buffer, 0);
    BrPixelmapFill(view->depth_buffer, 0xffffffff);

    BrRendererFrameBegin();
    BrZbSceneRender(view->world->root, view->world->camera, view->colour_buffer, view->depth_buffer);
    BrRendererFrameEnd();
} /* End of ViewRender() */

/*
 * FUNCTION NAME: ViewScreenUpdate
 *
 * DESCRIPTION:   Update screen from offscreen bitmap - will attempt to
 *                only update the changed area, unless force_all is true
 *
 */
void ViewScreenUpdate(brwin_view *view)
{
    BrPixelmapDoubleBuffer(view->screen, view->colour_buffer);
} /* End of ViewScreenUpdate() */
