/* fogdem.c
 *
 * A demo to show BRender's support for fogging
 */

#include "..\misc\br_demo.h"
#include <stdio.h>
#include <dos.h>

#include "brender.h"
#include "dosio.h"

#define SQUARE 30
#define FADETIME 128
#define ROLLING_BALL_SENSITIVITY 500
#define MSCALE BR_SCALAR(0.006)

br_pixelmap *screen, *zbuffer, *backscreen, *palette, *fog, *shade;
br_actor *world, *camera, *light, *mainobj, *ground, *spotlight;
br_material *mats[20], *mat;
br_matrix34 roll, temp;
int n;
long int mouse_x, mouse_y;
unsigned long mouse_buttons;
br_scalar t = 0, x = 0, y = 0, ox = 0, oy = 0, oz = 0;
br_angle a = 0;
int sr = 10, er = 190, sg = 10, eg = 170, sb = 0, eb = 255;
int textcol, textshad;
int i;
void Matrix33Mul(br_matrix34 *, br_matrix34 *, br_matrix34 *);
void ControlObject(br_actor *target);
int Control(br_actor *world, br_actor *observer, br_actor *target);

br_error LoadShadeTable( int type )
{
	br_pixelmap *pShade ;

	// Load a shade table 

	pShade = NULL ;

        pShade = BrPixelmapLoad("winsh8.tab");

        if ( !pShade )
        {
		BR_ERROR0("Could not load shade table");
		return BRE_FAIL ;
	}

        BrTableAdd(pShade);

        return BRE_OK ;
}


/*
 * Generate ground mesh
 */
void Initground(br_model *model, const int square, br_material *material)
{
	br_vertex *vertex;
	br_face *face;
        int i, j;

	vertex = model->vertices;
	face = model->faces;

        for(i = 0; i <= square; i++)
        for(j = 0; j <= square; j++, vertex++){
            BrVector3SetFloat(&vertex->p,
                ((float)i-((float)square)/2.0)/4.0, 0,
                ((float)j-((float)square)/2.0)/4.0);
            BrVector2SetFloat(&vertex->map,
                ((float)i-((float)square)/2.0)/16.0,
                ((float)j-((float)square)/2.0)/16.0);
        }

    for(i = 0; i < square; i++)
        for(j = 0; j < square; j++){
            face->material = material;
            face->vertices[0] = i + j*(square+1);
            face->vertices[1] = i + j*(square+1) + 1;
            face->vertices[2] = i + (j+1)*(square+1);
            face->smoothing = 1;
            face++;
            face->material = material;
            face->vertices[0] = i + (j+1)*(square+1);
            face->vertices[1] = i + j*(square+1) + 1;
            face->vertices[2] = i + (j+1)*(square+1) + 1;
            face->smoothing = 1;
            face++;
        }
}


/*
 * Build a 16x16 blended palette, for optimum "fogability"
 * Takes rgb ranges
 */
void makepalette(br_pixelmap *palette, int startr, int endr, int startg,
                 int endg, int startb, int endb)
{
    br_colour *palette_entry;
    int i, j;

    palette_entry = ((br_colour *)palette->pixels)+16;

    for(i = 0; i<14; i++)
        for(j = 0; j<16; j++)
            *(palette_entry++) = BR_COLOUR_RGB(
                (((startr+j*(endr-startr)/15)-128)*(13-i)/13)+128,
                (((startg+j*(endg-startg)/15)-128)*(13-i)/13)+128,
                (((startb+j*(endb-startb)/15)-128)*(13-i)/13)+128);
}

/*
 * Blend rgb ranges over a period of time, to create a surreal palette
 * fading effect
 */
void updatecolours(int *sr, int *sg, int *sb, int *er, int *eg, int *eb)
{
    static int time = 0;
    static int range = 0;
    static int csr, cer, csg, ceg, csb, ceb;
    static int ranges[8][6] = {
        {10, 190, 10, 170, 0, 255},
        {10, 190, 10, 170, 0, 128},
        {40, 200, 10, 170, 0, 255},
        {0, 128, 40, 128, 40, 192},
        {40, 140, 40, 140, 0, 140},
        {20, 100, 50, 150, 60, 255},
        {0, 200, 0, 150, 0, 255},
        {80, 140, 80, 140, 80, 140},
    };

    if(time > 0){
        time--;
        *sr = (time*csr+(FADETIME-time)*ranges[range][0])/FADETIME;
        *er = (time*cer+(FADETIME-time)*ranges[range][1])/FADETIME;
        *sg = (time*csg+(FADETIME-time)*ranges[range][2])/FADETIME;
        *eg = (time*ceg+(FADETIME-time)*ranges[range][3])/FADETIME;
        *sb = (time*csb+(FADETIME-time)*ranges[range][4])/FADETIME;
        *eb = (time*ceb+(FADETIME-time)*ranges[range][5])/FADETIME;
    } else {
        if(range >= 7)
            range = 0;
        else
            range++;
        time = FADETIME;
        csr = *sr;
        cer = *er;
        csg = *sg;
        ceg = *eg;
        csb = *sb;
        ceb = *eb;
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
        NULL
    };
    char **text = help;
    int i;

    for(i = y; *text; i += 7)
        BrPixelmapTextF(buffer, x - buffer->origin_x,
            i - buffer->origin_y, colour, BrFontProp4x6,
            *text++);
}

/*
 * Feature toggling functions
 */
br_boolean helpon = BR_TRUE;
br_boolean fogon = BR_TRUE;
br_boolean mainon = BR_TRUE;
br_boolean spoton = BR_TRUE;
br_boolean groundon = BR_TRUE;
br_boolean fadeon = BR_TRUE;

void helptoggle(void)
{
}

void fogtoggle(void)
{
    if((fogon)){
        mainobj->material->index_fog = fog;
        mainobj->material->fog_min = 2;
        mainobj->material->fog_max = 6;
        mainobj->material->fog_colour = BR_COLOUR_RGB(128,128,128);
        mainobj->material->flags |= BR_MATF_FOG_LOCAL;
        BrMaterialUpdate(mainobj->material, BR_MATU_ALL);
        ground->material->index_fog = fog;
        ground->material->fog_min = 2;
        ground->material->fog_max = 6;
        ground->material->fog_colour = BR_COLOUR_RGB(128,128,128);
        ground->material->flags |= BR_MATF_FOG_LOCAL;
        BrMaterialUpdate(ground->material, BR_MATU_ALL);
    } else {
        mainobj->material->index_fog = NULL;
        mainobj->material->flags &= ~BR_MATF_FOG_LOCAL;
        BrMaterialUpdate(mainobj->material, BR_MATU_ALL);
        ground->material->index_fog = NULL;
        ground->material->flags &= ~BR_MATF_FOG_LOCAL;
        BrMaterialUpdate(ground->material, BR_MATU_ALL);
    }
}

void maintoggle(void)
{
    if((mainon))
        BrLightEnable(light);
    else
        BrLightDisable(light);
}

void spottoggle(void)
{
    if((spoton))
        BrLightEnable(spotlight);
    else
        BrLightDisable(spotlight);
}

void groundtoggle(void)
{
    if((groundon))
        ground->render_style = BR_RSTYLE_DEFAULT;
    else
        ground->render_style = BR_RSTYLE_NONE;
}

void fadetoggle(void)
{
}

void checkkeys(void)
{
    struct{
        int keytest;
        br_boolean *flag;
        void (*togglefn)(void);
    } keytable[] = {
        {SC_H, &helpon, helptoggle},
        {SC_F, &fogon, fogtoggle},
        {SC_M, &mainon, maintoggle},
        {SC_S, &spoton, spottoggle},
        {SC_G, &groundon, groundtoggle},
        {SC_P, &fadeon, fadetoggle},
        {0, NULL}
    };

    int i = 0;

    while(keytable[i].flag){
        if((DOSKeyTest(keytable[i].keytest,0,REPT_FIRST_DOWN))){
            *keytable[i].flag = !*keytable[i].flag;
            keytable[i].togglefn();
        }
        i++;
    }
}

br_error BrDemo_InitApp(int argc, char **argv)
{
   br_error r;

   r = BrDemo_Allocate_Pixelmaps("Fog Demo", &screen, &backscreen, 
				 &zbuffer, &palette, "fogdem.pal");

   if ( r != BRE_OK )
	   return r ;

   switch(screen->type){
        case BR_PMT_RGB_555:
        case BR_PMT_RGB_565:
        case BR_PMT_RGB_888:
            textcol = 0xffffff;
            textshad = 0x000000;
            break;
        case BR_PMT_INDEX_8:
        default :
            textcol = 31;
            textshad = 16;
            break;
    }

    backscreen->origin_x = backscreen->width/2;
    backscreen->origin_y = backscreen->height/2;

    fog = BrTableAdd(BrPixelmapLoad("fogtable.tab"));
    shade = BrTableAdd(BrPixelmapLoad("fogshade.tab"));

    BrZbBegin(screen->type, BR_PMT_DEPTH_16);

    BrTableFindHook(BrTableFindFailedLoad);
    BrMapFindHook(BrMapFindFailedLoad);

    n = BrFmtScriptMaterialLoadMany("fogdem.mat", mats, BR_ASIZE(mats));
    BrMaterialAddMany(mats, n);

    world = BrActorAllocate(BR_ACTOR_NONE, NULL);
    BrDemo_SetRootActor(world);
    camera = BrActorAllocate(BR_ACTOR_CAMERA, NULL);
    light = BrActorAllocate(BR_ACTOR_LIGHT, NULL);
    mainobj = BrActorAllocate(BR_ACTOR_MODEL, NULL);
    ground = BrActorAllocate(BR_ACTOR_MODEL, NULL);
    spotlight = BrActorAllocate(BR_ACTOR_LIGHT, NULL);

    ((br_camera *) camera->type_data)->type = BR_CAMERA_PERSPECTIVE;
	((br_camera *) camera->type_data)->field_of_view = BR_ANGLE_DEG(45.0);
	((br_camera *) camera->type_data)->hither_z = BR_SCALAR(0.1);
    ((br_camera *) camera->type_data)->yon_z = BR_SCALAR(20.0);
    ((br_camera *) camera->type_data)->aspect = BR_SCALAR(320.0/240.0);
	camera->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&camera->t.t.mat, 0, BR_SCALAR(0.2), BR_SCALAR(5));
    BrMatrix34PreRotateX(&camera->t.t.mat, BR_ANGLE_DEG(-12));

    ((br_light *) light->type_data)->type = BR_LIGHT_DIRECT;
    light->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34RotateX(&light->t.t.mat, BR_ANGLE_DEG(-30));
    BrLightEnable(light);

    ((br_light *) spotlight->type_data)->type = BR_LIGHT_SPOT;
    ((br_light *) spotlight->type_data)->cone_inner = BR_ANGLE_DEG(30);
    ((br_light *) spotlight->type_data)->cone_outer = BR_ANGLE_DEG(70);
    spotlight->t.type = BR_TRANSFORM_MATRIX34;
    BrLightEnable(spotlight);

    mainobj->model = BrModelLoad("skull.dat");
    mainobj->material = BrMaterialFind("environment");
    BrMaterialUpdate(mainobj->material, BR_MATU_ALL);
    BrModelAdd(mainobj->model);

    ground->model = BrModelAllocate("ground", (SQUARE+1)*(SQUARE+1), SQUARE*SQUARE*2);
    ground->material = BrMaterialFind("earth");
    Initground(ground->model, SQUARE, ground->material);
    BrMaterialUpdate(ground->material, BR_MATU_ALL);
    ground->model->flags = BR_MODF_UPDATEABLE;
    BrModelAdd(ground->model);
    BrModelUpdate(ground->model,BR_MODU_ALL);

    BrActorAdd(world, camera);
    BrActorAdd(world, light);
    BrActorAdd(world, mainobj);
    BrActorAdd(world, ground);
    BrActorAdd(mainobj, spotlight);

    BrMatrix34RotateY(&spotlight->t.t.mat, BR_ANGLE_DEG(-0));

    return(BRE_OK);


}



br_error BrDemo_Render_Loop()
{
	if ( !backscreen )
       return BRE_OK ;

	BrMatrix34Identity(&roll);

    checkkeys();

    if((fadeon)){
      updatecolours(&sr, &sg, &sb, &er, &eg, &eb);
      makepalette(palette, sr, er, sg, eg, sb, eb);
      DOSGfxPaletteSet(palette);
      BrMapUpdate(ground->material->colour_map, BR_MAPU_ALL);
    }

    a += BR_ANGLE_DEG(0.25);
    
    x = BR_MUL(BR_SIN(a), BR_SCALAR(0.013));
    y = BR_MUL(BR_SIN(a+BR_ANGLE_DEG(65)), BR_SCALAR(0.007));

    switch(backscreen->type){
        case BR_PMT_INDEX_8:
	    BrPixelmapFill(backscreen, 239);
	    break;
        case BR_PMT_RGB_555:
        case BR_PMT_RGB_565:
        case BR_PMT_RGB_888:
	    BrPixelmapFill(backscreen, 0x808080);
	    break;
        default:
	    BrPixelmapFill(backscreen, 0);
	    break;
        }

    BrPixelmapFill(zbuffer, 0xffffffff);

    BrZbSceneRender(world, camera, backscreen, zbuffer);
    if(helpon){
      displayhelp(backscreen, 12, 12, textshad);
      displayhelp(backscreen, 11, 11, textcol);
    }
    BrPixelmapDoubleBuffer(screen, backscreen);
    
    BrMatrix23PostTranslate(&ground->material->map_transform, x, y);
    BrMatrix23PreRotate(&ground->material->map_transform,
			BR_ANGLE_DEG(BrScalarToFloat(BR_MUL(BR_SIN(a), 
							    BR_SCALAR(1)))));
    BrMaterialUpdate(ground->material, BR_MATU_MAP_TRANSFORM);
    
    BrMatrix34Translate(&ground->t.t.mat, 0, BR_SCALAR(-0.75), 0);
    BrMatrix34PostRotateZ(&ground->t.t.mat,
	 BR_ANGLE_DEG(BrScalarToFloat(BR_MUL(BR_SCALAR(5),BR_COS(a)))));

    if (Control(world, camera, mainobj))
        return BRE_FAIL;
    else
        return BRE_OK;

}



br_error BrDemo_ShutDown()
{
   // Close down

   BrPixelmapFree(zbuffer);
   BrPixelmapFree(backscreen);

   zbuffer = NULL ;
   backscreen = NULL ;

   BrZbEnd();

   return(BRE_OK);
		
}



br_error BrDemo_OnResize()
{
	BrDemo_Refresh_Pointers( &screen, &backscreen, &zbuffer );

        backscreen->origin_x = backscreen->width/2;
        backscreen->origin_y = backscreen->height/2;

      	return BRE_OK ;
}



/*
 * Read mouse and keyboard for control info.
 *
 * Return true if user wants to quit
 */

int Control(br_actor *world, br_actor *observer, br_actor *target)
{
   ControlObject(target);
   
   if(DOSKeyTest(SC_Q,0,REPT_FIRST_DOWN))
      return 1;

   return 0;
}



/*
 * Read the mouse to control an actor's translation and rotation
 */
void ControlObject(br_actor *target)
{
   static br_int_32 mouse_x,mouse_y;
   br_uint_32 mouse_buttons;
   br_matrix34 mat_target, mat_roll;
   br_scalar tx,ty,tz;

   DOSMouseRead(&mouse_x,&mouse_y,&mouse_buttons);

   // Mouse controls
   
   if ( ( mouse_buttons & BR_MSM_BUTTONL ) &&
       ( mouse_buttons & BR_MSM_BUTTONR ) )
   {
      // Drag object around in x/y plane

      tx = BR_MUL(BR_SCALAR(mouse_x),MSCALE);
      tz = BR_MUL(BR_SCALAR(mouse_y),MSCALE);

      BrMatrix34PostTranslate(&target->t.t.mat,tx,BR_SCALAR(0.0),tz);
      
   } 
   else if( mouse_buttons & BR_MSM_BUTTONL ) 
   {
      // Move object along z axis

      tx =  BR_MUL(BR_SCALAR(mouse_x),MSCALE);
      ty = -BR_MUL(BR_SCALAR(mouse_y),MSCALE);

      BrMatrix34PostTranslate(&target->t.t.mat,tx,ty,BR_SCALAR(0.0));
   } 
   else if ( mouse_buttons & BR_MSM_BUTTONR ) 
   {
      // Rotate object via rolling ball interface

      BrMatrix34RollingBall(&mat_roll,mouse_x,-mouse_y, ROLLING_BALL_SENSITIVITY );

      // Only modify the top 3x3

      BrMatrix34Copy(&mat_target, &target->t.t.mat);
      Matrix33Mul(&target->t.t.mat,&mat_target, &mat_roll);
   }

   mouse_x = 0;
   mouse_y = 0;
}



// Matrix multiply for top 3x3 only

#define A(x,y) A->m[x][y]
#define B(x,y) B->m[x][y]
#define C(x,y) C->m[x][y]

void Matrix33Mul(br_matrix34 *A, br_matrix34 *B, br_matrix34 *C)
{
   A(0,0) = BR_MAC3(B(0,0),C(0,0), B(0,1),C(1,0), B(0,2),C(2,0));
   A(0,1) = BR_MAC3(B(0,0),C(0,1), B(0,1),C(1,1), B(0,2),C(2,1));
   A(0,2) = BR_MAC3(B(0,0),C(0,2), B(0,1),C(1,2), B(0,2),C(2,2));

   A(1,0) = BR_MAC3(B(1,0),C(0,0), B(1,1),C(1,0), B(1,2),C(2,0));
   A(1,1) = BR_MAC3(B(1,0),C(0,1), B(1,1),C(1,1), B(1,2),C(2,1));
   A(1,2) = BR_MAC3(B(1,0),C(0,2), B(1,1),C(1,2), B(1,2),C(2,2));

   A(2,0) = BR_MAC3(B(2,0),C(0,0), B(2,1),C(1,0), B(2,2),C(2,0));
   A(2,1) = BR_MAC3(B(2,0),C(0,1), B(2,1),C(1,1), B(2,2),C(2,1));
   A(2,2) = BR_MAC3(B(2,0),C(0,2), B(2,1),C(1,2), B(2,2),C(2,2));
}
