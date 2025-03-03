//////////////////////////////////////////////////////////////////////
//
// Project  : BRender Mirror Sample
//            Copyright (c) 1996 Argonaut Technologies Ltd.
//            All rights reserved.
//
// Author         : Sam Littlewood
// Demo Framework : Steve Williams 
//
// File     : MIRROR.C
//
// Requires : BRender V1.3
//            Microsoft DirectX 3.0, Win95/NT or DOS
//
// Comments : Mirror demo
//
//////////////////////////////////////////////////////////////////////


#include <string.h>
#include <math.h>
#include <limits.h>
#define ROLLING_BALL_SENSITIVITY 500
#define TESTMAT "test.mat"
#define ZB 1
#define ZS 0

#define PRIMSIZE 1000000

#include "..\misc\br_demo.h"

void dprintf(int x, int y, char *fmt,...);
void PrintMatrix34(int x,int y,br_matrix34 *t);

void BuildTestWorld(void);

void BlendPixelmaps(br_pixelmap *,br_pixelmap *,br_pixelmap *,br_pixelmap *);

static void BR_CALLBACK ViewBoundsCallback(
	br_actor *actor,
	br_model *model,
	br_material *material,
	void *render_data,
	br_uint_8       style,
	br_matrix4 *model_to_screen,
	br_int_32 *bounds);


void ControlObject(br_actor *camera_actor, br_actor *translate_actor, br_actor *rotate_actor,
	br_int_32 mouse_x, br_int_32 mouse_y);

void UpdateMirror(br_actor *dest, br_actor *src, br_actor *world);

br_pixelmap *screen, *render_pm, *depth, *palette, *offscreen, *reflected_pm;
br_pixelmap *blend_table;

static br_int_32 mouse_x,mouse_y;
br_uint_32 mouse_buttons;

br_model *model,*md, *mirror_model;
br_actor *world, *observer, *root, *floor_actor, *clip_plane;
br_actor *model_t_actor, *model_actor;
br_actor *a, *observer_r_actor;
br_actor *light_t_actor, *light_actor;
br_actor *mirror_t_actor, *mirror_actor;

br_camera *camera;
br_light *light;

br_error r;
int m = 0, nmats,i;
br_material *mats[128], *mat1, *mat2;
char *model_name,*device_string = NULL;
void *primitives;
br_pixelmap *pm, *st;

br_error BrDemo_InitApp(int argc, char **argv)
{
        br_pixelmap *pShade;

	r = BrDemo_Allocate_Pixelmaps( "Mirror Demo",
				    &screen,&offscreen,&depth,
                                                        &palette,"winstd.pal");
       
        if ( r != BRE_OK )
		return r ;

        pShade = BrPixelmapLoad("winsh8.tab");
        
        if ( !pShade ) {
           BR_ERROR0("Could not load shade table");
           return BRE_FAIL ;
        }

        blend_table = BrPixelmapLoad("blend.tab");

        if (!blend_table) {
           BR_ERROR0("Could not load blend.tab"); 
           return BRE_FAIL ;
        }

        BrTableAdd(pShade);

        render_pm = offscreen;
        render_pm->origin_x = render_pm->width/2;
	render_pm->origin_y = render_pm->height/2;
									  
	reflected_pm = BrPixelmapMatch(screen, BR_PMMATCH_OFFSCREEN);
	
	if ( !reflected_pm )
	{
	    BR_ERROR0("Could not match reflected pixelmap");
		return BRE_FAIL ;
	}

	reflected_pm->origin_x = reflected_pm->width/2;
	reflected_pm->origin_y = reflected_pm->height/2;

	/*
	 * Start up renderer
	 */

#if ZB
	BrZbBegin(render_pm->type, depth->type);
#endif

#if ZS
	primitives = BrMemAllocate(PRIMSIZE,BR_MEMORY_APPLICATION);
	BrZsBegin(render_pm->type, primitives,PRIMSIZE);
#endif

	/*
	 * Load some materials
	 */

	BrMapFindHook(BrMapFindFailedLoad);

	nmats = BrFmtScriptMaterialLoadMany(TESTMAT,mats,BR_ASIZE(mats));
	BrMaterialAddMany(mats,nmats);

	/*
	 * Load and add a model
	 */
	model = BrModelLoad("cube.dat");

	if(model == NULL)
		BR_ERROR1("could not load '%s'",model_name);

	if(model)
		BrModelAdd(model);
	else
		BR_ERROR1("Could not load model '%s'",model_name);

	mirror_model = BrModelLoad("floor.dat");
	
	if(mirror_model)
		BrModelAdd(mirror_model);

	/*
	 * Build world
	 */
	BuildTestWorld();
    
    return BRE_OK ;
} // End Init_App()


/*
 * Tumble the actor around
 */

br_error BrDemo_Render_Loop() 
{
	if (!offscreen ) 
		return BRE_OK ;

        /*
	 * Clear the buffers
	 */

	BrPixelmapFill(render_pm, 0);
	BrPixelmapFill(reflected_pm, 0);

        if (offscreen->type != BR_PMT_INDEX_8) {
           BrPixelmapTextF(offscreen,-(BrPixelmapTextWidth(offscreen,BrFontProp7x9,
                           "ONLY 8 BIT SCREEN MODES ARE SUPPORTED")/2),
                           (BrFontProp7x9->glyph_y)/2,BR_COLOUR_RGB(200,200,200),
                           BrFontProp7x9,
                           "ONLY 8 BIT SCREEN MODES ARE SUPPORTED");
           BrPixelmapDoubleBuffer(screen,offscreen);
           if(DOSKeyTest(SC_Q,0,REPT_FIRST_DOWN))
               return BRE_FAIL;
           else return BRE_OK;
        }
        

#if ZB
	BrClipPlaneEnable(mirror_actor);

	BrPixelmapFill(depth, 0xFFFFFFFF);

	floor_actor->render_style = BR_RSTYLE_NONE;
	world->render_style = BR_RSTYLE_DEFAULT;

	/*
	 * Render the scene
	 */
	BrMatrix34Identity(&world->t.t.mat);
	BrZbSceneRender(root, observer, render_pm, depth);
	
	/*
	 * Render the reflection
	 */
	UpdateMirror(world, mirror_actor, world);
	BrZbSceneRender(root, observer, reflected_pm, depth);

	BrClipPlaneDisable(mirror_actor);

	/*
	 * Render the reflecting floor
	 */
	floor_actor->render_style = BR_RSTYLE_DEFAULT;
	world->render_style = BR_RSTYLE_NONE;
	BrActorToActorMatrix34(&floor_actor->t.t.mat, mirror_actor, world);
	BrZbSceneRender(root, observer, render_pm, depth);

	BlendPixelmaps(render_pm, render_pm, reflected_pm, blend_table);
#endif

#if ZS
	BrZsSceneRender(world, observer, render_pm);
#endif

#if 0
	PrintMatrix34(0,0,&model_actor->t.t.mat);
#endif

	BrPixelmapDoubleBuffer(screen, offscreen);

	/*
	 * Controls
	 */
	DOSMouseRead(&mouse_x,&mouse_y,&mouse_buttons);
		
	if(mouse_x || mouse_y) 
	{
		if(DOSKeyTest(SC_L,0,0))
			ControlObject(observer, light_t_actor, light_actor, mouse_x, mouse_y);
		else if(DOSKeyTest(SC_M,0,0))
			ControlObject(observer, mirror_t_actor, mirror_actor, mouse_x, mouse_y);
		else if(DOSKeyTest(SC_O,0,0))
			ControlObject(observer, observer, observer_r_actor, -mouse_x, -mouse_y);
		else
			ControlObject(observer, model_t_actor, model_actor, mouse_x, mouse_y);

		mouse_x = 0;
		mouse_y = 0;
	}

	if(DOSKeyTest(SC_Q,0,REPT_FIRST_DOWN))
	 return BRE_FAIL;


	if(DOSKeyTest(SC_F1,QUAL_NONE,0)) 
	{
		light->type = (light->type & BR_LIGHT_VIEW) | BR_LIGHT_DIRECT;
	}

	if(DOSKeyTest(SC_F2,QUAL_NONE,0)) 
	{
		light->type = (light->type & BR_LIGHT_VIEW) | BR_LIGHT_POINT;
	}

	if(DOSKeyTest(SC_F3,QUAL_NONE,0)) 
	{
		light->type = (light->type & BR_LIGHT_VIEW) | BR_LIGHT_SPOT;
	}

	if(DOSKeyTest(SC_F4,0,REPT_FIRST_DOWN)) 
	{
		light->type ^= BR_LIGHT_VIEW;
	}

		if(DOSKeyTest(SC_F5,QUAL_NONE,0)) {
			light->cone_inner += BR_ANGLE_DEG(1.0);
			light->cone_outer = light->cone_inner + BR_ANGLE_DEG(5.0);
		}

		if(DOSKeyTest(SC_F6,QUAL_NONE,0)) {
			light->cone_inner -= BR_ANGLE_DEG(1.0);
			light->cone_outer = light->cone_inner + BR_ANGLE_DEG(5.0);
		}

		if(DOSKeyTest(SC_D,0,REPT_FIRST_DOWN)) {
			
			a = BrActorAdd(world,BrActorAllocate(BR_ACTOR_MODEL,NULL));
			a->model = model_actor->model;
			a->material = model_actor->material;
			a->t.type = BR_TRANSFORM_MATRIX34_LP;
			BrActorToActorMatrix34(&a->t.t.mat, model_actor, world);
		}
	return BRE_OK ;
}

br_error BrDemo_ShutDown() 
{
	BrPixelmapFree(reflected_pm);
	reflected_pm = NULL;

	BrPixelmapFree(offscreen);
	offscreen = NULL ;

#if ZB
	BrZbEnd();
#endif

#if ZS
	BrZsEnd();
#endif

   return BRE_OK ;
}


br_error BrDemo_OnResize()
{
	BrDemo_Refresh_Pointers( &screen, &offscreen, &depth );

	render_pm = offscreen ;
    
        render_pm->origin_x = render_pm->width/2;
	render_pm->origin_y = render_pm->height/2;

        BrPixelmapFree(reflected_pm);

        reflected_pm = BrPixelmapMatch(screen, BR_PMMATCH_OFFSCREEN);
	reflected_pm = BrPixelmapMatch(screen, BR_PMMATCH_OFFSCREEN);
        reflected_pm->origin_x = reflected_pm->width/2;
	reflected_pm->origin_y = reflected_pm->height/2;

    	return BRE_OK ;
}    


#define MSCALE BR_SCALAR(0.006)


void ControlObject(br_actor *camera_actor, br_actor *translate_actor, br_actor *rotate_actor,
	br_int_32 mouse_x, br_int_32 mouse_y)
{
	br_matrix34 mat_roll;
	br_scalar tx,ty,tz;


        if( ( mouse_buttons & BR_MSM_BUTTONL) &&
                ( mouse_buttons & BR_MSM_BUTTONR ) )
        {       
                // move object along the z axis

        	tx =  BR_MUL(BR_SCALAR(mouse_x),MSCALE);
                tz =  BR_MUL(BR_SCALAR(mouse_y),MSCALE);

                BrMatrix34PostTranslate(&translate_actor->t.t.mat,tx,BR_SCALAR(0.0),tz);

	} 
        else if( mouse_buttons & BR_MSM_BUTTONL ) 
	{
                // Move object in the XY plane

		tx = BR_MUL(BR_SCALAR(mouse_x),MSCALE);
                ty = -BR_MUL(BR_SCALAR(mouse_y),MSCALE);

                BrMatrix34PostTranslate(&translate_actor->t.t.mat,tx,ty,BR_SCALAR(0.0));

	} 
        else if ( mouse_buttons & BR_MSM_BUTTONR ) 
	{
                // Rotate object via rolling ball interface
                BrMatrix34RollingBall(&mat_roll,mouse_x,-mouse_y,ROLLING_BALL_SENSITIVITY);
	
		BrMatrix34Post(&rotate_actor->t.t.mat, &mat_roll);
	}
        
        mouse_x = 0;
        mouse_y = 0;
}



void PrintMatrix34(int x,int y,br_matrix34 *t)
{
	int i,j;

	for(i=0; i<3; i++)
		for(j=0; j<4; j++)
			dprintf(x+i*12,y+j*2,"%11.5f",BrScalarToFloat(t->m[j][i]));
}

void dprintf(int x, int y, char *fmt,...)
{
	char temp[256];
	va_list args;
	int o = 0;
	/*
	 * Build output string
	 */
	va_start(args,fmt);
	BrVSprintf(temp,fmt,args);
	va_end(args);
	
	BrPixelmapText(offscreen, x * 4, y * 6, 255, BrFontFixed3x5,  temp);
}

void BuildTestWorld(void)
{
	root = BrActorAllocate(BR_ACTOR_NONE,NULL);
	root->t.type = BR_TRANSFORM_IDENTITY;

	// BrDemo_SetRootActor lets the demo framework know which 
	// actor to enumerate when turning on/off actor based properties.
	
	BrDemo_SetRootActor(root);
	
	/*
	 * Build the world
	 */
	world = BrActorAdd(root, BrActorAllocate(BR_ACTOR_NONE,NULL));
	world->t.type = BR_TRANSFORM_MATRIX34_LP;

	observer_r_actor = BrActorAdd(root,BrActorAllocate(BR_ACTOR_NONE,NULL));
	observer_r_actor->t.type = BR_TRANSFORM_MATRIX34_LP;

	observer = BrActorAdd(observer_r_actor,BrActorAllocate(BR_ACTOR_CAMERA,NULL));
	observer->t.type = BR_TRANSFORM_MATRIX34_LP;
	BrVector3Set(&observer->t.t.euler.t,BR_SCALAR(0.0),BR_SCALAR(0.0),BR_SCALAR(5.0));

#if 1
	camera = (br_camera *)observer->type_data;
	camera->type = BR_CAMERA_PERSPECTIVE;
	camera->hither_z = BR_SCALAR(0.1);
	camera->yon_z = BR_SCALAR(10.0);
	camera->aspect = BR_SCALAR(1.46);

//      camera->type = BR_CAMERA_PARALLEL;

	camera->width = BR_SCALAR(2.0);
	camera->height = BR_SCALAR(2.0);
#endif

	model_t_actor = BrActorAdd(world,BrActorAllocate(BR_ACTOR_NONE,NULL));
	model_t_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
	model_actor = BrActorAdd(model_t_actor,BrActorAllocate(BR_ACTOR_MODEL,NULL));
	model_actor->model = model;
	model_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
	model_actor->material = BrMaterialFind("test_mat");
        BrMatrix34PostRotateX(&model_actor->t.t.mat,BR_ANGLE_DEG(45));
        BrMatrix34PostRotateY(&model_actor->t.t.mat,BR_ANGLE_DEG(45));

	light_t_actor = BrActorAdd(world,BrActorAllocate(BR_ACTOR_NONE,NULL));
	light_t_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
	BrVector3Set(&light_t_actor->t.t.translate.t,BR_SCALAR(0.0),BR_SCALAR(0.0),BR_SCALAR(2.0));

	light_actor = BrActorAdd(light_t_actor,BrActorAllocate(BR_ACTOR_LIGHT,NULL));
	light_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
	light = (br_light *)light_actor->type_data;
	light->colour = BR_COLOUR_RGB(255,255,255);
	BrLightEnable(light_actor);

	mirror_t_actor = BrActorAdd(world,BrActorAllocate(BR_ACTOR_NONE,NULL));
	mirror_t_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
	mirror_actor = BrActorAdd(mirror_t_actor,BrActorAllocate(BR_ACTOR_CLIP_PLANE,NULL));
	mirror_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
        BrMatrix34PostRotateY(&mirror_actor->t.t.mat,BR_ANGLE_DEG(45));

	floor_actor = BrActorAdd(root,BrActorAllocate(BR_ACTOR_MODEL,NULL));
	floor_actor->t.type = BR_TRANSFORM_MATRIX34_LP;
	floor_actor->model = mirror_model;
	floor_actor->material = BrMaterialFind("tiles");
}


void UpdateMirror(br_actor *dest, br_actor *src, br_actor *world)
{
	br_matrix34 m,mi,mr,mt;

	BrMatrix34Identity(&mr);
	mr.m[2][2] = BR_SCALAR(-1);

	BrActorToActorMatrix34(&m, src, world);
	BrMatrix34Inverse(&mi,&m);

	BrMatrix34Mul(&mt, &mi, &mr);
	BrMatrix34Mul(&dest->t.t.mat, &mt, &m);
}

void BlendPixelmaps(br_pixelmap *dest,
					br_pixelmap *a,br_pixelmap *b,
					br_pixelmap *op)
{
	unsigned char *dp, *ap, *bp;
        unsigned char (*blend)[256] = (void *)op->pixels;       
	int i, c = dest->width * dest->height;

    if ( !dest->pixels ) {              
		BrPixelmapDirectLock( dest, BR_TRUE );
	}
	dp = dest->pixels;

	if ( !a->pixels ) {
		BrPixelmapDirectLock( a, BR_TRUE );
	}
	ap = a->pixels;
	
	if ( !b->pixels ) {
		BrPixelmapDirectLock( b, BR_TRUE );
	}
	bp = b->pixels;
	
	for(i=0; i < c; i++) {
		*dp = blend[*ap][*bp];

		dp++;
		ap++;
		bp++;
	}

	if ( dest->pixels ) {           
		BrPixelmapDirectUnlock( dest );
		dp = dest->pixels;
	}


	if ( a->pixels ) {
		BrPixelmapDirectUnlock( a );
		ap = a->pixels;
	}
	
	if ( b->pixels ) {
		BrPixelmapDirectUnlock( b );
		bp = b->pixels;
	}
	
}

