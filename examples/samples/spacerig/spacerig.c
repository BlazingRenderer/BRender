//////////////////////////////////////////////////////////////////////
//
// Project  : BRender SpaceRig Demo
//            Copyright (c) 1996 Argonaut Technologies Ltd.
//            All rights reserved.
//
// Author        : Tony Roberts
//
// Modifications : Owain Green, Steve Williams
//                  - Modified to work with new demo framework.
//
// File     : SPACERIG.C
//
// Requires : BRender V1.3
//            Microsoft DirectX 3.0, Win95/NT or DOS
//            
// Comments : SpaceRig specific source.
//
//////////////////////////////////////////////////////////////////////

#include <brdemo.h>

#include "stars.h"
#include "spacerig.h"
#include "math.h"

#define DEPTH_LOW_VALUE 40000
#define DEPTH_HIGH_VALUE 54000
// Global variables 

// Hierarchy structure

br_actor *world; // World of actor type none
br_actor *hull, *hull_up, *airlock_01, *airlock_02;
br_actor *my_view;
br_actor *man, *light1, *camera_rear, *camera;

br_actor *lift_area;  // Elevator part of model- actor_bounds_correct 
br_actor *liftflor, *door_1_lft, *door_1_rgt, *b_door_lft, *b_door_rgt;
br_actor *elev_front;
br_actor *upper_area; // Upper area of model - actor_bounds_correct
br_actor *level_1bot;
br_actor *argo_null; // Argonaut logo models -actor_bounds_correct 
br_actor *argo_blu, *argo_grn, *argo_yel, *argo_red;  
br_actor *box, *table, *tv;
br_actor *probe;
br_actor *camera_probe;

br_actor *lower_area; /* lower_room of model -actor bounds correct  */
br_actor *basefloor, *power_core;

br_actor *maze_area; /* corridor section of model -actor_bounds_correcr */
br_actor *corridor_1;

br_pixelmap *depth_buffer, *palette;
br_pixelmap *colour_buffer, *screen, *backdrop_pm;
br_pixelmap *rear_pm, *rear_depth, *feedback_pm, *feedback_depth;
br_pixelmap *star_pm;
br_material *mats[150]; 
br_material *pulse_wave, *flick, *dark;
br_material *change, *swap, *animate_material, *oldMaterial, *newMaterial;
br_model    *mods[20], *mod1s[20], *mod2s[5]; 

br_matrix34 rotation;
long mouse_x, mouse_y;
unsigned long mouse_buttons;

int count = 0;
int Rotate = 0;
int elev_flag = 0;
int light_tag = 0;
int lamp_tag = 0;
int stardraw = 0;
int names = 0; 
int airlock_doors = 0;
int present_level = 0;
int angle = 0;
int x,i,n,m,m1,m2,m3,elevator; 
int camera_probe_flag = 0;
int info = 0;
int menu ;
br_boolean bShowHelp = BR_TRUE ;
br_boolean bCollisionDetect = 1;
                         
float move = 0.0F;
float shift = 0.0F;
float mouse_move = 0.0F;

br_uint_16 depth_value1, depth_value2, depth_value3, depth_value4, depth_value;
br_uint_16 window_dx, window_dy, curr_width, curr_height;
br_uint_16 Collision_Detect_X[3], Collision_Detect_Y[3] ;
br_uint_16 screen_scale=0;

br_scalar atten = BR_SCALAR(0.8);     
br_scalar t = BR_SCALAR(0.0);

br_matrix4 probe_path = {{BR_VECTOR4(25.07,4.2, -6.4,1.0),
                          BR_VECTOR4( 8.81,4.5,-12.38,1.0),
                          BR_VECTOR4( 8.83,4.7, -9.33,1.0),
                          BR_VECTOR4(18.50,5.1, -1.44,1.0)
                        }};

br_vector4 probe_centre = BR_VECTOR4(25.07,4.2,-6.4,1.0);

br_vector4 v_t = BR_VECTOR4(0.0,0.0,0.0,0.0);
char *default_name = "default";
br_pixelmap *pShade ;


br_uint_16 SysCols8[7] = { 41, 64, 96, 132, 158, 191, 226 };
br_uint_16 SysCols15[7] = { 0x7fff, 0xd000, 0xd000, 0xd000, 0xd000, 0xd000, 0xd000 };
br_uint_16 SysCols16[7] = { 0xffff, 0xd000, 0xd000, 0xd000, 0xd000, 0xd000, 0xd000 };




void Init_Actor( br_actor *parent, 
                 br_actor **new_actor, 
                 char *modelname,
                 char *material, 
                 br_boolean load_flag,
                 br_boolean material_flag,
                 float X_axis_rotation,
                 float X_translation,
                 float Y_translation,
                 float Z_translation,
                 br_boolean keep_flag)

{

     br_actor *pActor;

     pActor = BrActorAdd(parent,BrActorAllocate(BR_ACTOR_MODEL,NULL));

     if (load_flag)
        pActor->model = BrModelLoad(modelname);
      else
        pActor->model = BrModelFind(modelname);

     if (pActor->model==NULL)
     {
          BR_ERROR1("Cannot open %s\n", modelname);
     }

     if (keep_flag)
        pActor->model->flags |= BR_MODF_KEEP_ORIGINAL;

     if (load_flag)
        BrModelAdd(pActor->model);

     if (material_flag)
        pActor->material = BrMaterialFind(material);

        pActor->t.type = BR_TRANSFORM_MATRIX34 ;
        BrMatrix34PreRotateX(&pActor->t.t.mat,BR_ANGLE_DEG(X_axis_rotation));
        BrMatrix34PostTranslate(&pActor->t.t.mat,
        BR_SCALAR(X_translation),BR_SCALAR(Y_translation),BR_SCALAR(Z_translation));
        *new_actor = pActor ;
}

static br_error SpaceRigInit(br_demo *demo)
{   
    // Initialise BRender 
    
    BrMapFindHook(LoadMapFFHook);    

    if((demo->palette = BrPixelmapLoad("winpal.pal")) == NULL) {
        BrLogError("DEMO", "Error loading winpal.pal");
        return BRE_FAIL;
    }
    BrMapAdd(demo->palette);

	// HACK
	screen = demo->_screen;
	colour_buffer = demo->colour_buffer;
	depth_buffer = demo->depth_buffer;
	palette = demo->palette;

    // Variables set to current screen height/width used for screen reduction 
    
    curr_width  = colour_buffer->width;
    curr_height = colour_buffer->height;
    window_dx = (br_uint_16) (colour_buffer->width * 0.2);
    window_dy = (br_uint_16) (colour_buffer->height* 0.2);

    // Determine z-buffering sampling positions of window 

    Collision_Detect_X[0] = (br_uint_16) ( ((screen_scale*window_dx)/2)+(curr_width*0.05) );
    Collision_Detect_Y[0] = (br_uint_16) ( (colour_buffer->height)*0.5 );
    Collision_Detect_X[1] = (br_uint_16) ( (colour_buffer->width)*0.5 );
    Collision_Detect_Y[1] = (br_uint_16) ( (colour_buffer->height)*0.5 );
    Collision_Detect_X[2] = (br_uint_16) ( ((screen_scale*window_dx)/2)+(curr_width*0.95) );
    Collision_Detect_Y[2] = (br_uint_16) ( (colour_buffer->height)*0.5 );

    // Set pixelmaps for camera_rear used in collision detection

    rear_pm = BrPixelmapMatchSized( colour_buffer, BR_PMMATCH_HIDDEN_BUFFER,4,4 );
    
	if ( rear_pm )
	   rear_depth = BrPixelmapMatch( rear_pm, BR_PMMATCH_DEPTH_16 );   
	else
	   rear_depth = NULL ;

    // BrZbBegin( screen->type, BR_PMT_DEPTH_16 );

    pShade = NULL ;

    switch ( screen->type )
    {
       case BR_PMT_INDEX_8 : pShade = BrPixelmapLoad("winsh8.tab");
                             break ;

       case BR_PMT_RGB_555 : pShade = BrPixelmapLoad("winsh15.tab");
                             break ;

       case BR_PMT_RGB_565 : pShade = BrPixelmapLoad("winsh16.tab");
                             break ;
   }

   if ( !pShade )
   {
       BR_ERROR0("Couldn't load shade table");
       return BRE_FAIL ;
   }

   BrTableAdd(pShade);

   star_pm = BrPixelmapLoad(STARPIX);
    
   if ( star_pm == NULL ) 
       BR_ERROR0("Couldn't load star(w).pix");
    
   BrMapAdd(star_pm);

   TextureMapLoad();
    
   // Load materials

   n = BrFmtScriptMaterialLoadMany( "spacerig.mat", mats, BR_ASIZE(mats) );
    
   if ( n == 0 )
   {
		BR_ERROR0("Could not load spacerig materials");
		return BRE_FAIL ;
   }
	
   BrMaterialAddMany(mats,n);

   // Load models (multiple model files)
       
   m = BrModelLoadMany("room.dat",mods,BR_ASIZE(mods));
   BrModelAddMany(mods,m);
    
   m1=BrModelLoadMany("argohead.dat",mod1s,BR_ASIZE(mod1s));
   BrModelAddMany(mod1s,m1);
    
   m2=BrModelLoadMany("spotlite.dat",mod2s,BR_ASIZE(mod2s));
   BrModelAddMany(mod2s,m2);

   // HACK
   world=demo->world;

   Init_World_Bounds( world, &lift_area );
   Init_World_Bounds( world, &maze_area );
   Init_World_Bounds( world, &upper_area ); 
   Init_World_Bounds( upper_area, &argo_null );
   Init_World_Bounds( world, &lower_area );

   // Build rest of world 

   Init_Actor(lift_area, &elev_front, "elev_front",
                      default_name, BR_FALSE, BR_FALSE, -180.0F, 0.0F, 0.0F, 0.0F, BR_FALSE);
   
   Init_Actor(lift_area, &door_1_lft, "door_1_lft",
                      default_name, BR_FALSE, BR_FALSE, -180.0F, 0.1F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor(lift_area, &door_1_rgt, "door_1_rgt",
                      default_name, BR_FALSE, BR_FALSE, -180.0F, 0.1F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor(lift_area, &b_door_lft, "b_door_lft",
                      default_name, BR_FALSE, BR_FALSE, -180.0F, -0.1F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor(lift_area, &b_door_rgt, "b_door_rgt",
                      default_name, BR_FALSE, BR_FALSE, -180.0F, -0.1F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor(lift_area,&liftflor,"liftflor.dat",
                      "red plstc", BR_TRUE, BR_TRUE, -180.0F, 0.0F, 0.2F, 0.0F, BR_FALSE);

   Init_Actor(maze_area,&corridor_1, "corridor_1",
                      default_name, BR_FALSE, BR_FALSE, -180.0F, 0.0F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor(upper_area,&level_1bot, "level_1bot",
                      default_name,BR_FALSE, BR_FALSE, -180.0F, 0.0F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor(world,&hull_up,"hull_up.dat",
                      default_name,BR_TRUE, BR_FALSE, -180.0F, 0.0F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor(upper_area,&box, "cube.dat",
                      "MATERIAL 23", BR_TRUE, BR_TRUE, -180.0F, 10.0F, 4.6F, -13.5F, BR_FALSE);

   // Load logo model items 
    
   Init_Actor(argo_null,&argo_blu, "argo_blu",
                      "blue_flat", BR_FALSE, BR_TRUE, -90.0F, 17.95F, 4.16F, -14.5F, BR_FALSE);

   Init_Actor(argo_null,&argo_grn, "argo_grn",
                    "green_flat", BR_FALSE, BR_TRUE, -90.0F, 17.95F, 4.15F, -14.5F, BR_FALSE);

   Init_Actor(argo_null,&argo_yel, "argo_yel",
                    "yellow_flat", BR_FALSE, BR_TRUE, -90.0F, 17.95F, 4.13F, -14.5F, BR_FALSE);

   Init_Actor(argo_null,&argo_red, "argo_red",
                  "red_flat", BR_FALSE, BR_TRUE, -90.0F, 18.0F, 4.11F, -14.5F, BR_FALSE);

   Init_Actor(upper_area,&probe, "probe.dat",
                      default_name, BR_TRUE, BR_FALSE, 0.0F, 0.0F, 0.0F, 0.0F, BR_FALSE);
   probe->t.type=BR_TRANSFORM_LOOK_UP;
   
   BrVector3Set(&probe->t.t.look_up.up,
       BR_SCALAR(0.0),BR_SCALAR(1.0),BR_SCALAR(0.0));

   Init_Camera(probe,&camera_probe);
   
   BrMatrix34PostTranslate(&camera_probe->t.t.mat,
         BR_SCALAR(0.0),BR_SCALAR(0.0),BR_SCALAR(-1.0));    

   Init_Actor(upper_area,&table, "table.dat",
                  "MATERIAL 24", BR_TRUE, BR_TRUE, 0.0F, 18.0F, 3.2F, -6.5F, BR_TRUE );
    
   BrMaterialUpdate(table->material,BR_MATU_ALL);
    
   oldMaterial = BrMaterialFind("GREY_FLAT");
   newMaterial = BrMaterialFind("MATERIAL 24");
       
   for( x = 0 ; x < table->model->nfaces ; x++ )
   {
      if ( table->model->faces[x].material == oldMaterial )
           table->model->faces[x].material=newMaterial;
   }

   BrModelUpdate(table->model,BR_MODU_ALL);

   Init_Actor( upper_area,&tv, "tv.dat",
                  "red_flat", BR_TRUE, BR_TRUE, 0.0F, 17.58F, 4.7F, -5.38F, BR_FALSE);

   Init_Actor(lower_area,&basefloor, "basefloor",
                  default_name, BR_FALSE, BR_FALSE, -180.0F, 0.0F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor(world,&hull, "hull.dat",
                  default_name, BR_TRUE, BR_FALSE, -180.0F, 0.0F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor( lower_area, &power_core, "powerrod.dat",
                default_name, BR_TRUE, BR_FALSE, -180.0F, 0.0F, 0.0F, 0.0F, BR_FALSE );

   Init_Actor(world,&airlock_02, "airlock_02",
                  default_name, BR_FALSE, BR_FALSE, -180.0F, 0.0F, 0.0F, 0.0F, BR_FALSE);

   Init_Actor(world,&airlock_01, "airlock_01",
                  default_name, BR_FALSE, BR_FALSE, -180.0F, 0.0F, 0.0F, 0.0F, BR_FALSE);

   // Establish Actor Bounds 

   BrActorToBounds(((br_bounds*)lift_area->type_data),lift_area);

   BrActorToBounds(((br_bounds*)maze_area->type_data),maze_area);

   BrActorToBounds(((br_bounds*)upper_area->type_data),upper_area);

   BrActorToBounds(((br_bounds*)lower_area->type_data),lower_area);

   my_view = BrActorAdd(world,BrActorAllocate(BR_ACTOR_NONE,NULL));
    
   Init_Camera(my_view,&camera);

   Init_Camera(my_view,&camera_rear);
         BrMatrix34PreRotateY(&camera_rear->t.t.mat,BR_ANGLE_DEG(180.0));  
    
   Init_Light(camera,&light1,atten );

   // Translate my_view 

   my_view->t.type=BR_TRANSFORM_MATRIX34;
   
   BrMatrix34PostTranslate(&my_view->t.t.mat,
        BR_SCALAR(-7.0),BR_SCALAR(-1.5),BR_SCALAR(5.5));
    
   BrActorRemove(upper_area);
   BrActorRemove(door_1_rgt);
   BrActorRemove(door_1_lft);
   BrActorRemove(liftflor);
   BrActorRemove(elev_front);
   BrActorRemove(hull);
   BrActorRemove(hull_up);

   maze_area->render_style = BR_RSTYLE_NONE;

   // Set up of pointers for animated and flipping textures

   pulse_wave=BrMaterialFind("MATERIAL 21");  
   change=BrMaterialFind("MONITOR"); 
   flick=BrMaterialFind("silver");  
   swap=BrMaterialFind("grey plstc");

   change->flags |= BR_MATF_PERSPECTIVE;

   // Now setup the starfield 

   SetupStarfield();   

   return BRE_OK;

} // End BrDemo_InitApp()



void Open_Airlock_Doors()
{
   shift = shift + 0.15F;
      
   RiseDoors(airlock_01,airlock_02);
      
   if (shift>2.00)
   { 
      airlock_doors=2;  /*  airlock doors now open   */
      shift = 0.0F;
   }
}


void Upper_Level_Animation()
{
   Animate_box(box, argo_null, argo_blu, argo_grn, argo_yel, argo_red);
   
   angle = Animate_wall_texture("MATERIAL 03", angle);
   angle = Animate_wall_texture("MATERIAL 04", angle);

   // Position probe in room accoriding to bezier curve 
    
   t = BR_ADD(t,BR_SCALAR(0.01));
       
   if (t>BR_SCALAR(1.00))
      t=BR_SCALAR(0.0);

   CalcBezierBasis(t,&probe_path,&probe_centre,&v_t);

   BrVector3Set(&probe->t.t.look_up.look,v_t.v[X],v_t.v[Y],v_t.v[Z]);
   BrVector3Set(&probe->t.t.look_up.t,
   probe_centre.v[X],probe_centre.v[Y], probe_centre.v[Z]);

   // This code flicks between pages on monitor screen
    
   if (camera_probe_flag==0)   
     count = ChangeTVimage(count, change);
}


void Do_Elevator_Stuff1()
{
   // Close basement doors then elevate to next level
       
   if (elev_flag==2 && move<2.00)
   { 
      move = move + 0.15F;
      MoveDoors(b_door_lft,b_door_rgt);
   }
   else
   { 
       my_view->t.t.mat.m[3][Y]=(my_view->t.t.mat.m[3][Y])
                                       +BR_SCALAR(0.2);
       liftflor->t.t.mat.m[3][Y]=(liftflor->t.t.mat.m[3][Y])
                                     +BR_SCALAR(0.2);
       if ((BrScalarToFloat(my_view->t.t.mat.m[3][Y]))>4.5)
       { 
           elevator=0;
           elev_flag=3;  
           present_level=1;        /* flag to open upper level doors*/
           BrActorAdd(world,upper_area); /*  Add upper room actors */

           // Remove lower room actors 

           BrActorRemove(maze_area);  
           BrActorRemove(lower_area);
           airlock_01->render_style=BR_RSTYLE_NONE;
           airlock_02->render_style=BR_RSTYLE_NONE;
            
           // elevator doors on basement closed and now on 1st floor
      
           move = 0.0F;
       }
   }
}



void Do_Elevator_Stuff2()
{
    // If upper level doors are closed then descend
       
    if (elev_flag==6) 
    { 
         my_view->t.t.mat.m[3][Y]=(my_view->t.t.mat.m[3][Y])
                                    +BR_SCALAR(-0.2);
         liftflor->t.t.mat.m[3][Y]=(liftflor->t.t.mat.m[3][Y])
                                    +BR_SCALAR(-0.2);
         if ((BrScalarToFloat(my_view->t.t.mat.m[3][Y]))<-1.5)
         { 
            elevator=0;
            elev_flag=1;             
            present_level=0;   //  flag to open lower elevator doors 
        
            // Set liftfloor so visible just above ground floor 

            BrMatrix34Identity(&liftflor->t.t.mat);  
            BrMatrix34PostTranslate(&liftflor->t.t.mat,
                                BR_SCALAR(0.0),BR_SCALAR(0.2),BR_SCALAR(0.0));
            BrMatrix34PreRotateX(&liftflor->t.t.mat,BR_ANGLE_DEG(-180.0));
               
            BrActorAdd(world,maze_area);
            BrActorAdd(world,lower_area);
            airlock_01->render_style=BR_RSTYLE_DEFAULT;
            airlock_02->render_style=BR_RSTYLE_DEFAULT;
            
            // Ensure normal view on screen - not monitor or probe view
        
            camera_probe_flag=0;
            BrActorRemove(upper_area);

            // elevator doors on upper level are closed and now on basement floor 
        
            // Ensure monitor rotation & torch are disabled now that on basement floor
        
            if (light_tag==1)
            {
               BrLightDisable(light1);
               light_tag=0;
            }
      
			Rotate=0;
            move = 0.0F;
        }
     }
     else 
       elev_flag=5;
}


void Do_Camera_Probe_Stuff()
{
    // Display on the monitor the probe camera image 

    BrPixelmapFill( feedback_depth, 0xffffffff );
    BrPixelmapFill( feedback_pm, 0x0 );
    BrZbSceneRender( world, camera_probe, feedback_pm, feedback_depth );

    // change is a pointer to the material of the monitor screen which       
    // is assigned a new colour map.
 
    change->colour_map=feedback_pm;
    BrMaterialUpdate(change, BR_MATU_ALL);
    BrZbSceneRender( world, camera, colour_buffer, depth_buffer );
}


void Check_Collision()
{
   depth_value1 = BrPixelmapPixelGet( depth_buffer, Collision_Detect_X[0], Collision_Detect_Y[0] );
   depth_value2 = BrPixelmapPixelGet( depth_buffer, Collision_Detect_X[1], Collision_Detect_Y[1] );
   depth_value3 = BrPixelmapPixelGet( depth_buffer, Collision_Detect_X[2], Collision_Detect_Y[2] );
   depth_value  = MIN(depth_value1,MIN(depth_value2,depth_value3));
}


void Close_Doors_Upper()
{
    move = move + 0.15F;
  
    MoveDoors(door_1_rgt,door_1_lft);
       
	if (move>2.00)
    {
        elev_flag=6;

/*  upper level doors are closed 
 *  Active geometry culling
 *  remove actors outside of upper level when upper level doors are closed                                   
 *  but only if not descending in elevator
 */

	    if (elevator!=-1)
        {
           BrActorRemove(b_door_rgt);
           BrActorRemove(b_door_lft);
           BrActorRemove(elev_front);
           BrActorRemove(liftflor);
           BrActorRemove(hull);
        }
       
		move = 0.0F;
    }
}


void Do_Elevator_Doors_Stuff()
{
   // Open elevator door on upper level 
    
   if (elev_flag==3 && move<2.00)
   {
       move = move + 0.15F;
       MoveDoors(door_1_lft,door_1_rgt);
       if (move>2.00)
        {
          elev_flag=4; // Upper level doors are open 
          move = 0.0F;
        }
   }

   // Close elevator doors on upper level
    
   if ( elev_flag==5 && move<2.00 )
       Close_Doors_Upper();
}


void Close_Airlock_Doors()
{ 
    shift = shift + 0.15F;

    RiseDoors(airlock_02,airlock_01);
	
	if (shift>2.00)
	{
        airlock_doors=0; // Airlock doors now closed
          
		if ((BrScalarToFloat(my_view->t.t.mat.m[3][Z])<7.0))  
             maze_area->render_style=BR_RSTYLE_NONE;
        else
        {
			// Set render style of models notwithin corridor to be zero
            // Actors inherit rendering styles thus 
            
            lower_area->render_style=BR_RSTYLE_NONE;
            upper_area->render_style=BR_RSTYLE_NONE;
            lift_area->render_style=BR_RSTYLE_NONE;

            hull->render_style=BR_RSTYLE_NONE;                  
            hull_up->render_style=BR_RSTYLE_NONE;
        }
      
		shift = 0.0F;
    }
}


void Door_Stuff()
{
    //  CODE FOR THE OPENING OF DOORS 

    // Default value of elev_flag is zero, set to 1 on pressing space bar 

    if (elev_flag==1)
    {
        move= move + 0.15F;
        MoveDoors(b_door_rgt,b_door_lft);
       
	    if ( move > 2.00 )
        {
           elev_flag = 2;  // Lower elevator doors are open
           move = 0.0F;
        }
    }
    
    // Ascent
	
	if (elevator==1)
       	Do_Elevator_Stuff1();

	// Descent
	
    if (elevator==-1)
    	Do_Elevator_Stuff2();

	Do_Elevator_Doors_Stuff();
	
	// Open airlock doors
      
    if (airlock_doors==1 && shift<2.00)
		Open_Airlock_Doors();

    // Close airlock doors 

    if (airlock_doors==2)
    	Close_Airlock_Doors();
}

  
void SpaceRigUpdate(br_demo *demo, br_scalar dt)
{    
    if (!colour_buffer) 
      return;

    if (info || menu)
       BrPixelmapFill(colour_buffer,0x0);   
    else
       BrPixelmapRectangleFill(colour_buffer,(screen_scale*window_dx)/2,
                    (screen_scale*window_dy)/2,
               (colour_buffer->width),(colour_buffer->height),0x0);
    
    BrPixelmapFill( depth_buffer, 0xffffffff );
    
    // Upper level animation in scene
    
    if (present_level)
      Upper_Level_Animation();  

    if ( present_level == 0 )
       count = ChangeMaps(count, flick, swap, pulse_wave);
     
    // Draw the starfield in the current viewing window

    if (stardraw)
       StarfieldDraw( colour_buffer, world, camera, pShade, star_pm, names);

    if ( camera_probe_flag == 0 )
       BrZbSceneRender( world, camera, colour_buffer, depth_buffer );

    if (camera_probe_flag==1)    
   		Do_Camera_Probe_Stuff();

    // This code displays the camera view 

    if ( camera_probe_flag == 2 )
       BrZbSceneRender( world, camera_probe, colour_buffer, depth_buffer );
   
    if ( (camera_probe_flag == 0) && (bCollisionDetect == 1) )
 		Check_Collision();
		
	Door_Stuff();

/********************USER INTERACTION MOUSE & KEYBOARD*********************/                  

   // Routine for airlock switching 

    if ( (DOSKeyTest(SC_SPACE,0,0)) && 
         ((BrScalarToFloat(my_view->t.t.mat.m[3][Z])>3.00) &&
         (BrScalarToFloat(my_view->t.t.mat.m[3][Z])<11.2) ) )  
     { 
		if (airlock_doors==0)
        { 
			airlock_doors=1;
            maze_area->render_style = BR_RSTYLE_DEFAULT;

/* Set render style of models outside corridor to be default 
 * when airlock doors are open must be able to see both corridor
 * and lower room interior
 */                          
            
          lower_area->render_style=BR_RSTYLE_DEFAULT;
          upper_area->render_style=BR_RSTYLE_DEFAULT;
          lift_area->render_style=BR_RSTYLE_DEFAULT;
          hull->render_style=BR_RSTYLE_DEFAULT;                  
          hull_up->render_style=BR_RSTYLE_DEFAULT;

          shift = 0.0F;
        }
     }
 
/*  Routine for elevator switching */

    if (( DOSKeyTest(SC_SPACE,0,0)) && 
   (
    (BrScalarToFloat(my_view->t.t.mat.m[3][X])<5.80) && 
    (BrScalarToFloat(my_view->t.t.mat.m[3][X])>3.26)
   )
       )  
     { if (present_level==0)
          elevator=1;
       if (present_level==1)
          elevator=-1;
     }

    // Allow user to move the system around

    mouse_x = mouse_y = 0;

    DOSMouseRead( &mouse_x, &mouse_y, &mouse_buttons );
         
    // Allow swivel control of monitor

    if  ( (Rotate==1) && (mouse_buttons & BR_MSM_BUTTONL) )
    {
       BrMatrix34RollingBall(&rotation,mouse_x,-mouse_y,800);
       BrMatrix34Pre(&tv->t.t.mat,&rotation);
    }

    // Allow mouse movement (Rotation) control
    
    if ( (Rotate!=1) && (mouse_buttons & BR_MSM_BUTTONL) )
    {
       BrMatrix34RollingBall(&rotation,-mouse_x,0,800);
       BrMatrix34Pre(&my_view->t.t.mat,&rotation);
    }
 
    // Routine for fist person movement with/without collison detection 

    if (bCollisionDetect)
    { 

       if ((mouse_buttons & BR_MSM_BUTTONR) && depth_value>DEPTH_LOW_VALUE)
       {
          if (mouse_y>0)
             mouse_move=BR_SCALAR(-0.3);
    
		  BrMatrix34PreTranslate(&my_view->t.t.mat,0,0,mouse_move);
       }

       if ((mouse_buttons & BR_MSM_BUTTONR) && depth_value>DEPTH_HIGH_VALUE)
       { 
		   if (mouse_y>0)
              mouse_move=BR_SCALAR(-0.2);
           
		   BrMatrix34PreTranslate(&my_view->t.t.mat,0,0,mouse_move);
       }
      
       if ( ( DOSKeyTest(SC_C_UP+SC_EXTENDED,0,0)) && (depth_value>DEPTH_LOW_VALUE) )   
           BrMatrix34PreTranslate(&my_view->t.t.mat,0,0,BR_SCALAR(-0.2)); 
        
       if ( DOSKeyTest(SC_C_DOWN+SC_EXTENDED,0,0) )
       {
          if ( rear_pm )
		  {
			  BrPixelmapFill( rear_depth, 0xffffffff );
              BrPixelmapFill( rear_pm, 0x0 );
              BrZbSceneRender( world,camera_rear,rear_pm,rear_depth);   
              depth_value4= BrPixelmapPixelGet(rear_depth,2,2);    

/*  if moving backwards do an additional rendering and sample z-buffer          
 *  for distance if greater than 40000 (chosen by experimentation) then 
 *  allow movement 
 */
              if (depth_value4>DEPTH_LOW_VALUE)
                  BrMatrix34PreTranslate(&my_view->t.t.mat,0,0,BR_SCALAR(0.2));
		  }
       }
    }  

/*  End of bCollisionDetect==1 */


    if ( bCollisionDetect == 0 )
    {
       if ( mouse_buttons & BR_MSM_BUTTONR )
          BrMatrix34PreTranslate(&my_view->t.t.mat,0,0,
                                  BR_MUL(BR_SCALAR(mouse_y),BR_SCALAR(0.01)));

       if ( DOSKeyTest(SC_C_UP+SC_EXTENDED,0,0) ) 
          BrMatrix34PreTranslate(&my_view->t.t.mat,0,0,BR_SCALAR(-0.2)); 
          
       if ( ( DOSKeyTest(SC_C_UP+SC_EXTENDED,0,0)) && ((DOSKeyTest(SC_LSHIFT,0,0)) || (DOSKeyTest(SC_RSHIFT,0,0))) )
          BrMatrix34PreTranslate(&my_view->t.t.mat,0,0,BR_SCALAR(-0.6));
          
       if ( DOSKeyTest(SC_C_DOWN+SC_EXTENDED,0,0) )
          BrMatrix34PreTranslate(&my_view->t.t.mat,0,0,BR_SCALAR(0.2)); 
    }          
       
    // End of collsion==0 

    if ( DOSKeyTest(SC_C_RIGHT+SC_EXTENDED,0,0))
       BrMatrix34PreRotateY(&my_view->t.t.mat,BR_ANGLE_DEG(-5.0));

    if (DOSKeyTest(SC_C_LEFT+SC_EXTENDED,0,0))
       BrMatrix34PreRotateY(&my_view->t.t.mat,BR_ANGLE_DEG(+5.0));


    // SpaceBar Opening of Doors 

    if ( DOSKeyTest(SC_SPACE,0,0) )
    { 
		if ( (BrScalarToFloat(my_view->t.t.mat.m[3][Z])<-1.00) && (elev_flag==0))
        {  

          // Add elev_shaft and upper level doors actors when door first opens 
          
          BrActorAdd(lift_area,door_1_rgt);
          BrActorAdd(lift_area,door_1_lft);
          BrActorAdd(lift_area,elev_front);
          BrActorAdd(lift_area,liftflor);
          BrActorAdd(world,hull);
          BrActorAdd(world,hull_up);
          elev_flag=1;   
// close doors flag 
        }
      
       if (elev_flag==4)  
// doors are open flag 
          elev_flag=5;   
// flag to close doors

       if (elev_flag==6)
        { 

// Add models outside of upper level when upper level doors are re-open

          BrActorAdd(lift_area,b_door_rgt);
          BrActorAdd(lift_area,b_door_lft);
          BrActorAdd(lift_area,elev_front);
          BrActorAdd(lift_area,liftflor);
          BrActorAdd(world,hull);

// flag to open doors
          elev_flag=3;
        }
      
      }
       
// End of SpaceBar Opening of Doors


// Monitor Display 

    if ( DOSKeyTest(SC_PG_UP,0,REPT_FIRST_DOWN))
    { 
		if (camera_probe_flag==0 && present_level==1)
        {
            camera_probe_flag=1;
            feedback_pm=BrPixelmapAllocate(BR_PMT_INDEX_8,256,256,NULL,0);
            BrMapAdd(feedback_pm);
            feedback_depth=BrPixelmapMatch(feedback_pm,BR_PMMATCH_DEPTH_16);   

            // Close elevator doors when viewing on monitor to prevent transparency 
          
		    if (elev_flag!=6)
               elev_flag=5;
        }
        else if (camera_probe_flag==1)
        {
               camera_probe_flag=0;
               BrPixelmapFree(feedback_pm);
               BrPixelmapFree(feedback_depth);
               change->colour_map=BrMapFind("page_0");
               BrMaterialUpdate(change, BR_MATU_ALL);
               count=0;
             }
     }
   
/*  End of monitor display */

   
    if (DOSKeyTest(SC_PG_DOWN,0,REPT_FIRST_DOWN)) 
    { 
		if (camera_probe_flag==0 && present_level==1)
        {  
			camera_probe_flag=2;

/*  An actor of a man is linked to my_view, when using the camera probe         
 *  switch rendering style on so a man can be displayed in position of
 *  my_view
 *
 *         man->render_style=BR_RSTYLE_DEFAULT;   
 */
        }   
        else
          { camera_probe_flag=0;
/* similarly when returning to first person view switch off rendering style         
 *
 *         man->render_style=BR_RSTYLE_NONE; 
 */
          }
     }     
      
       
    if (DOSKeyTest(SC_INSERT,0,REPT_FIRST_DOWN))
       if (Rotate==0 && present_level==1)
           Rotate ^= 1;
        else
           Rotate=0;

    if (DOSKeyTest(SC_TAB,0,REPT_FIRST_DOWN))
        bCollisionDetect ^= 1 ;
   
    if (DOSKeyTest(SC_I,0,REPT_FIRST_DOWN))
       info ^= 1;

    if (DOSKeyTest(SC_HOME,0,0)) 
       BrMatrix34Identity(&camera->t.t.mat);     
        

    if (DOSKeyTest(SC_R,0,0))
     {
       BrMatrix34Identity(&tv->t.t.mat);
       BrMatrix34PostTranslate(&tv->t.t.mat,
       BR_SCALAR(17.58),BR_SCALAR(4.7),BR_SCALAR(-5.38));
     }
    
    if (DOSKeyTest(SC_A,0,0))
       BrMatrix34PreRotateX(&camera->t.t.mat,BR_ANGLE_DEG(5.0));  

    if (DOSKeyTest(SC_Z,0,0))
       BrMatrix34PreRotateX(&camera->t.t.mat,BR_ANGLE_DEG(-5.0)); 
      
    if (DOSKeyTest(SC_X,0,0))
     { (box->material->map_transform.m[0][0])=
              (box->material->map_transform.m[0][0])*(2.0);
       (box->material->map_transform.m[1][1])=
              (box->material->map_transform.m[1][1])*(2.0); 
     }

    if (DOSKeyTest(SC_C,0,0))
     { (box->material->map_transform.m[0][0])=
              (box->material->map_transform.m[0][0])/(2.0);
       (box->material->map_transform.m[1][1])=
              (box->material->map_transform.m[1][1])/(2.0); 
     }

    if (DOSKeyTest(SC_P,0,0))
    {
        BrPixelmapSave("scrn_dmp.pix",colour_buffer);
        
		BrPixelmapTextF(colour_buffer,(screen->width)*0.5,(screen->height)*0.5
            ,RED,BrFontFixed3x5,"Screen Dumped");   
    }

      
    if (DOSKeyTest(SC_L,0,REPT_FIRST_DOWN))
       light_tag = Light_on_off(light1, light_tag, present_level);

    if (DOSKeyTest(SC_COMMA,0,REPT_FIRST_DOWN))
       atten = BrightenLight(colour_buffer, light1, atten);

    if (DOSKeyTest(SC_DOT,0,REPT_FIRST_DOWN))
       atten = DimLight(colour_buffer, light1, atten);

    if (DOSKeyTest(SC_8,0,REPT_FIRST_DOWN))
       stardraw ^= 1 ;
       
    if (DOSKeyTest(SC_N,0,REPT_FIRST_DOWN))
       names ^= 1 ;
             
    if (DOSKeyTest(SC_Q,0,REPT_FIRST_DOWN) || DOSKeyTest(SC_ESC,0,REPT_FIRST_DOWN))
       return;
       
    if ( DOSKeyTest( SC_F1,0, REPT_FIRST_DOWN ) )
		bShowHelp ^= 1;
	
	
	if ( info )
    {
       Add_Screen_Info( colour_buffer, screen,
                        Collision_Detect_X, Collision_Detect_Y,
                        depth_value1, depth_value2, depth_value3,
                        depth_value, my_view, bCollisionDetect,
                        stardraw, light_tag );
    }
   
    Add_Camera_Probe_Info( colour_buffer, camera_probe_flag );

    if (bShowHelp)
    {
       Show_Help( colour_buffer );
    }

    BrPixelmapDoubleBuffer(screen,colour_buffer);

    return;
}  // End Render_Loop()




void Add_Camera_Probe_Info( br_pixelmap *colour_buffer, int flag )
{
    br_uint_16 *SysCol ;

    switch ( colour_buffer->type )
    {
       case BR_PMT_INDEX_8 : SysCol = SysCols8 ;
                             break ;

       case BR_PMT_RGB_555 : SysCol = SysCols15 ;
                             break ;

       case BR_PMT_RGB_565 : SysCol = SysCols16 ;
                             break ;
    }


   if ( flag == 1 )
      BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.35,(colour_buffer->height)*0.5,
                SysCol[RED], BrFontFixed3x5, "DOUBLE RENDERING MONITOR ON!!"); 

    
    else if ( flag == 2 )
      BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.40,(colour_buffer->height)*0.5,
                SysCol[MAGENTA], BrFontFixed3x5, "PROBE CAMERA VIEW" ); 
}

void Add_Screen_Info( br_pixelmap *colour_buffer, br_pixelmap *screen,
                      br_uint_16 *Collision_Detect_X,
                      br_uint_16 *Collision_Detect_Y, 
                      br_uint_16 depth_value1, br_uint_16 depth_value2, br_uint_16 depth_value3,
                      br_uint_16 depth_value, br_actor *my_view, br_boolean bCollisionDetect,
                      int stardraw, int light_tag )
{
    br_uint_16 *SysCols ;

    switch ( colour_buffer->type )
    {
       case BR_PMT_INDEX_8 : SysCols = SysCols8 ;
                             break ;

       case BR_PMT_RGB_555 : SysCols = SysCols15 ;
                             break ;

       case BR_PMT_RGB_565 : SysCols = SysCols16 ;
                             break ;
    }

    BrPixelmapTextF(colour_buffer, Collision_Detect_X[0], Collision_Detect_Y[0], SysCols[YELLOW], BrFontFixed3x5,"+"); 
    BrPixelmapTextF(colour_buffer, Collision_Detect_X[1], Collision_Detect_Y[1], SysCols[YELLOW], BrFontFixed3x5,"+");  
    BrPixelmapTextF(colour_buffer, Collision_Detect_X[2], Collision_Detect_Y[2], SysCols[YELLOW], BrFontFixed3x5,"+");  
    
    BrPixelmapTextF(colour_buffer,
                   (colour_buffer->width)*0.5,
                   (colour_buffer->height)*0.9,
                   SysCols[YELLOW], BrFontFixed3x5,                                 
   "z-buffer vals. %d %d %d %d", depth_value1, depth_value2, depth_value3, depth_value);     
    
     
   // CODE TO READ POSITION OF MY_VIEW FROM 3D WORLD SPACE

   BrPixelmapTextF( colour_buffer,(screen->width)*0.05,(screen->height)*0.9,
          SysCols[GREEN], BrFontFixed3x5,                                         
         "Position: x:=%5.2f y:%5.2f z:=%5.2f",
          BrScalarToFloat(my_view->t.t.mat.m[3][X]), 
          BrScalarToFloat(my_view->t.t.mat.m[3][Y]),
          BrScalarToFloat(my_view->t.t.mat.m[3][Z])
        );     
    
     
   if (bCollisionDetect)
       BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,(colour_buffer->height)*0.96,
                SysCols[GREEN],BrFontFixed3x5,"COLLISION ON"); 
    else
       BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,(colour_buffer->height)*0.96,
                SysCols[RED],BrFontFixed3x5,"COLLISION OFF"); 

   if (stardraw)   
       BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.45,(colour_buffer->height)*0.96,
                SysCols[GREEN],BrFontFixed3x5,"STARFIELD ON"); 
    else  
       BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.45,(colour_buffer->height)*0.96,
                SysCols[RED], BrFontFixed3x5,"STARFIELD OFF"); 
    
   if (light_tag)                                
      BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.8,(colour_buffer->height)*0.96,
                SysCols[BLUE], BrFontFixed3x5,"TORCH ON"); 
    else                        
      BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.8,(colour_buffer->height)*0.96,
                SysCols[CYAN],BrFontFixed3x5,"TORCH OFF"); 
}


void Show_Help( br_pixelmap *colour_buffer )
{
    br_uint_16 *SysCols ;

    switch ( colour_buffer->type )
    {
       case BR_PMT_INDEX_8 : SysCols = SysCols8 ;
                             break ;

       case BR_PMT_RGB_555 : SysCols = SysCols15 ;
                             break ;

       case BR_PMT_RGB_565 : SysCols = SysCols16 ;
                             break ;
    }


    //  HELP screen using BrPixelmapTextF 

    BrPixelmapTextF( colour_buffer, (colour_buffer->width) * 0.30,
               (colour_buffer->height)*0.05,
               SysCols[MAGENTA], BrFontProp7x9,"      CONTROLS"); 
     
    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.15,
               SysCols[WHITE],BrFontProp4x6,
        "CURSOR KEYS (UP/DOWN)SHIFT");     

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.15,
               SysCols[WHITE],BrFontProp4x6,
        "Walk forward/backward (RUN)"); 

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.20,
               SysCols[WHITE],BrFontProp4x6,
        "CURSOR KEYS (LEFT/RIGHT)");     

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.20,
               SysCols[WHITE],BrFontProp4x6,
        "Rotate Left/Right"); 
     

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.25,
               SysCols[WHITE],BrFontProp4x6,
        "A/Z");     

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.25,
               SysCols[WHITE],BrFontProp4x6,
        "Look Up/Down"); 

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.30,
               SysCols[WHITE],BrFontProp4x6,
        "SPACE BAR");      

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.30,
               SysCols[WHITE],BrFontProp4x6,
        "Open Door/Activate lift"); 

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.35,
               SysCols[WHITE],BrFontProp4x6,
        "8/N");      

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (long)(colour_buffer->height)*0.35,
               SysCols[WHITE],BrFontProp4x6,
        "Toggle on/off Starfield/Show Names"); 


    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.40,
               SysCols[WHITE],BrFontProp4x6,
        "TAB");      

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.40,
               SysCols[WHITE],BrFontProp4x6,
        "Toggle on/off Collision Detection"); 

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.45,
               SysCols[WHITE],BrFontProp4x6,
        "PAGE UP/PAGE DOWN");      
        
    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.45,
               SysCols[WHITE],BrFontProp4x6,
        "CAMERA PROBE VIEW MONITOR/SCREEN"); 
        
    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.50,
               SysCols[WHITE],BrFontProp4x6,
        "INSERT");      

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
              (colour_buffer->height)*0.50,
               SysCols[WHITE],BrFontProp4x6,
        "Toggle on/off mouse control of TV"); 


    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.55,
               SysCols[WHITE],BrFontProp4x6,
        "L");      

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.55,
               SysCols[WHITE],BrFontProp4x6,
        "Toggle on/off TORCH (Upper Floor Only)"); 

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.59,
               SysCols[WHITE], BrFontProp4x6,
        "</>");      
     
    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.59,
               SysCols[WHITE], BrFontProp4x6,
        "Decrease/Increase Intensity"); 

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.75,
               SysCols[GREEN], BrFontProp4x6,
        "P");     

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.75,
               SysCols[GREEN], BrFontProp4x6,
        "DUMP SCREEN"); 
        
    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.80,
              SysCols[YELLOW], BrFontProp4x6,
         "I");     

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.80,
              SysCols[YELLOW], BrFontProp4x6,
        "SCENE INFO"); 
        
    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.85,
              SysCols[YELLOW], BrFontProp4x6,
        "F1");     

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.85,
              SysCols[YELLOW], BrFontProp4x6,
        "HELP/MENU"); 

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.1,
               (colour_buffer->height)*0.93,
               SysCols[RED], BrFontProp4x6,
        "ESC/Q");     

    BrPixelmapTextF(colour_buffer,(colour_buffer->width)*0.5,
               (colour_buffer->height)*0.93,
               SysCols[RED], BrFontProp4x6,
        "QUIT"); 
}



void Init_World_Bounds( br_actor *parent, br_actor **new_actor )
{    
     br_actor *pActor;

     pActor=BrActorAdd(parent,BrActorAllocate(BR_ACTOR_BOUNDS_CORRECT,NULL));

     *new_actor=pActor;
}
  

void TextureMapLoad()
{
     br_pixelmap *txt[MAX_TEXTURES];
     br_pixelmap *txt1[MAX_TEXTURES];
     br_pixelmap *txt2[MAX_TEXTURES];
     br_pixelmap *txt3[MAX_TEXTURES];
     int o,p,q,r;
   
/* These pixelmap files contain multiple pixelmaps
 * texconv picture1.pix picture2.pix -o picture.pix
 * concatenates 
 */

    o=BrPixelmapLoadMany(ENERGYPIX,txt,MAX_TEXTURES);
    BrMapAddMany(txt,o);

    p=BrPixelmapLoadMany(PAGEPIX,txt1,MAX_TEXTURES);
    BrMapAddMany(txt1,p);

    q=BrPixelmapLoadMany(BASE_BGPIX,txt2,MAX_TEXTURES);
    BrMapAddMany(txt2,q);

    r=BrPixelmapLoadMany(ALCWALLPIX,txt3,MAX_TEXTURES);
    BrMapAddMany(txt3,r);
}
 

void CalcBezierBasis(br_scalar t,
           br_matrix4 *probe_path, 
           br_vector4 *probe_centre, 
           br_vector4 *v_t) 
{


     br_scalar s=BR_ADD(t,BR_SCALAR(0.01));
     br_vector4 basis;
     br_vector4 delta_basis;
     br_vector4 delta_probe_centre;
     br_scalar t_term=BR_SUB(BR_SCALAR(1.0),t);
     br_scalar s_term=BR_SUB(BR_SCALAR(1.0),s);

/* calculation  of 4 basis functions of 3rd order curve  */

     basis.v[0]=BR_POW(t_term,BR_SCALAR(3.0));
     basis.v[1]=BR_MUL(BR_MUL(BR_SCALAR(3.0),t),
                BR_POW(t_term,BR_SCALAR(2.0)));
     basis.v[2]=BR_MUL(BR_MUL(BR_SCALAR(3.0),t),BR_MUL(t,t_term));
     basis.v[3]=BR_POW(t,BR_SCALAR(3.0));

/* matrix multiplication of basis values with four control points
 * to give present position on curve for a given value of the t parameter
 */

     BrMatrix4Apply(probe_centre,&basis,probe_path);

/* calculate a position on the path slightly ahead of that already computed 
 * t + delta t i.e s=t+0.01
 */

     delta_basis.v[0]=BR_POW(s_term,BR_SCALAR(3.0));
     delta_basis.v[1]=BR_MUL(BR_MUL(BR_SCALAR(3.0),s),
                      BR_POW(s_term,BR_SCALAR(2.0)));
     delta_basis.v[2]=BR_MUL(BR_MUL(BR_SCALAR(3.0),s),BR_MUL(s,s_term));
     delta_basis.v[3]=BR_POW(s,BR_SCALAR(3.0));

     BrMatrix4Apply(&delta_probe_centre,&delta_basis,probe_path);    

/* calculate vector between two positions calculated on curve */

     v_t->v[0]=BR_SUB(delta_probe_centre.v[0],probe_centre->v[0]);
     v_t->v[1]=BR_SUB(delta_probe_centre.v[1],probe_centre->v[1]);  
     v_t->v[2]=BR_SUB(delta_probe_centre.v[2],probe_centre->v[2]);  
}


void Init_Camera(br_actor *parent, br_actor **new_observer)
{

     br_actor *pCam;

     pCam = BrActorAdd(parent,BrActorAllocate(BR_ACTOR_CAMERA,NULL));
     ((br_camera*)pCam->type_data)->type=BR_CAMERA_PERSPECTIVE_OLD;
     ((br_camera*)pCam->type_data)->field_of_view=BR_ANGLE_DEG(45.0);  
     ((br_camera*)pCam->type_data)->hither_z=BR_SCALAR(0.3); 
     ((br_camera*)pCam->type_data)->yon_z=BR_SCALAR(25.0);  
     ((br_camera*)pCam->type_data)->aspect=BR_SCALAR(1.46); 
     pCam->t.type=BR_TRANSFORM_MATRIX34; 

     *new_observer = pCam;
}


void Init_Light(br_actor *parent, br_actor **light, br_scalar atten)
{
     br_actor *pLight;

     pLight=BrActorAdd(parent,BrActorAllocate(BR_ACTOR_LIGHT,NULL));
     ((br_light*)pLight->type_data)->type=BR_LIGHT_SPOT;
     ((br_light*)pLight->type_data)->attenuation_c=atten;
     pLight->t.type=BR_TRANSFORM_MATRIX34;

     *light = pLight;
}


br_pixelmap *BR_CALLBACK LoadMapFFHook(const char *name)
{
     br_pixelmap *pm;

     if ((pm = BrPixelmapLoad(name)) != NULL)
      {
        pm->identifier = BrMemStrDup(name);
        BrMapAdd(pm);
      }

     return pm;
}


br_scalar DimLight(br_pixelmap *colour_buffer, br_actor *light1, br_scalar atten)
{
     atten = BR_SUB(atten,BR_SCALAR(0.05));

     if (atten<BR_SCALAR(0.60))
        atten=BR_SCALAR(0.60);

     ((br_light*)light1->type_data)->attenuation_c=atten;
     BrPixelmapTextF(colour_buffer,100,30,BLUE,BrFontFixed3x5,                                         
              "lighting attenuation: %4.2f",BrScalarToFloat(atten));     

     return atten;
}

br_scalar BrightenLight(br_pixelmap *colour_buffer, br_actor *light1, br_scalar atten)
{
     atten = BR_ADD(atten,BR_SCALAR(0.05));
   
     if (atten>BR_SCALAR(1.0))
        atten=BR_SCALAR(1.0);

     ((br_light*)light1->type_data)->attenuation_c=atten;
     BrPixelmapTextF(colour_buffer,100,30,BLUE,BrFontFixed3x5,                                         
            "lighting attenuation: %4.2f",BrScalarToFloat(atten));     

     return atten;

}

int Light_on_off(br_actor *light1, int light_tag, int present_level)
{
     if (light_tag)
      {
        BrLightDisable(light1);
        light_tag=0;
      }
      else 
        {
          if (present_level==1)
           {
             BrLightEnable(light1);
             light_tag=1;
           }
        }

      return light_tag;

}


void Animate_box(br_actor *box,
                 br_actor *argo_null,
                 br_actor *argo_blu,
                 br_actor *argo_grn,
                 br_actor *argo_yel,
                 br_actor *argo_red)

{
      BrMatrix34PreRotateY(&box->t.t.mat,BR_ANGLE_DEG(6.0));
      BrMatrix34PreRotateX(&box->t.t.mat,BR_ANGLE_DEG(6.0));

      BrMatrix34PreRotateZ(&argo_blu->t.t.mat,BR_ANGLE_DEG(-6.0));
      BrMatrix34PreRotateZ(&argo_grn->t.t.mat,BR_ANGLE_DEG(3.0));
      BrMatrix34PreRotateZ(&argo_yel->t.t.mat,BR_ANGLE_DEG(-6.0));
      BrMatrix34PreRotateZ(&argo_red->t.t.mat,BR_ANGLE_DEG(3.0));
      BrActorToBounds(((br_bounds*)argo_null->type_data),argo_null);
}

int Animate_wall_texture(char *material_name,
                                  int angle)
{
      br_material *pMaterial;

      pMaterial = BrMaterialFind(material_name);

      angle += BR_ANGLE_DEG(3);
      pMaterial->map_transform.m[0][0] = 10*BR_ADD(BR_CONST_DIV(BR_SIN(angle),4),BR_SCALAR(0.75));
      pMaterial->map_transform.m[1][1] = 5*BR_ADD(BR_CONST_DIV(BR_COS(angle),4),BR_SCALAR(0.75));

      BrMaterialUpdate(pMaterial, BR_MATU_ALL);

      return angle;
}

int ChangeTVimage(int count,
                      br_material *change)
{
      if (count%50==0)
         change->colour_map=BrMapFind("page_0");
    
      if (count%50==10)
         change->colour_map=BrMapFind("page_1");
    
      if (count%50==20)
         change->colour_map=BrMapFind("page_2");
    
      if (count%50==30)
         change->colour_map=BrMapFind("page_3");
     
      if (count%50==40)
         change->colour_map=BrMapFind("page_4");

      BrMaterialUpdate(change, BR_MATU_ALL);

      count++;
      return count;
}

int ChangeMaps(int count,
               br_material *flick,       /* flick textures on video walls  */
               br_material *swap,        /* swap textures on airlock walls */
               br_material *pulse_wave)  /* pulse textures on column       */
                   
{
	if (count%8==0)                                                       
    {
        flick->colour_map=BrMapFind("base_pg1");
        swap->colour_map=BrMapFind("alcwall1");
        pulse_wave->colour_map=BrMapFind("energyd1");
    }

    if (count%8==1)
    {
        flick->colour_map=BrMapFind("base_pg2");
        swap->colour_map=BrMapFind("alcwall2");
        pulse_wave->colour_map=BrMapFind("energyd2");
    }

    if (count%8==2)
    {
        flick->colour_map=BrMapFind("base_pg3");
        swap->colour_map=BrMapFind("alcwall3");
        pulse_wave->colour_map=BrMapFind("energyd3");
    }

    if (count%8==3)
    {
        flick->colour_map=BrMapFind("base_pg4");
        swap->colour_map=BrMapFind("alcwall4");
        pulse_wave->colour_map=BrMapFind("energyd4");
    }     
    
     if (count%8==4)
     {
        swap->colour_map=BrMapFind("alcwall5");
        pulse_wave->colour_map=BrMapFind("energyd5");
     }

     if (count%8==5)
     {
        swap->colour_map=BrMapFind("alcwall4");
        pulse_wave->colour_map=BrMapFind("energyd6");
     } 
    
     if (count%8==6)
     {
         swap->colour_map=BrMapFind("alcwall3");
         pulse_wave->colour_map=BrMapFind("energyd7");
     }

     if (count%8==7)
     {
         swap->colour_map=BrMapFind("alcwall2");
         pulse_wave->colour_map=BrMapFind("energyd8");
     }

     if (count%8==8)
         swap->colour_map=BrMapFind("alcwall1");

     BrMaterialUpdate(flick, BR_MATU_ALL);
     BrMaterialUpdate(swap, BR_MATU_ALL);
     BrMaterialUpdate(pulse_wave, BR_MATU_ALL);

     count++;
     return count;
}

void MoveDoors(br_actor *first_door, br_actor *second_door)
{
    BrMatrix34PostTranslate(&first_door->t.t.mat,
                            BR_SCALAR(0.0),BR_SCALAR(0.0),BR_SCALAR(+0.1));
    BrMatrix34PostTranslate(&second_door->t.t.mat,
                            BR_SCALAR(0.0),BR_SCALAR(0.0),BR_SCALAR(-0.1));
}

void RiseDoors(br_actor *first_door, br_actor *second_door)
{
    BrMatrix34PostTranslate(&first_door->t.t.mat,
                            BR_SCALAR(0.0),BR_SCALAR(+0.1),BR_SCALAR(0.0));
    
	BrMatrix34PostTranslate(&second_door->t.t.mat,
                            BR_SCALAR(0.0),BR_SCALAR(-0.1),BR_SCALAR(0.0));
}


void SpaceRigOnResize(br_demo *demo, br_uint_16 width, br_uint_16 height)
{
	BrDemoDefaultOnResize(demo, width, height);

	// HACK
	screen = demo->_screen;
	colour_buffer = demo->colour_buffer;
	depth_buffer = demo->depth_buffer;
	palette = demo->palette;

	screen->origin_x=0;//screen->width/2;
    screen->origin_y=0;//screen->height/2;
    colour_buffer->origin_x = screen->origin_x;
    colour_buffer->origin_y = screen->origin_y;
	depth_buffer->origin_x=depth_buffer->origin_y=0;
   
    curr_width=colour_buffer->width;
    curr_height=colour_buffer->height;
    window_dx=(colour_buffer->width * 0.2);
    window_dy=(colour_buffer->height* 0.2);

    Collision_Detect_X[0] = ((screen_scale*window_dx)/2)+(curr_width*0.05);
    Collision_Detect_Y[0] = (colour_buffer->height)*0.5;
    Collision_Detect_X[1] = (colour_buffer->width)*0.5;
    Collision_Detect_Y[1] = (colour_buffer->height)*0.5;
    Collision_Detect_X[2] = ((screen_scale*window_dx)/2)+(curr_width*0.95);
    Collision_Detect_Y[2] = (colour_buffer->height)*0.5;
}

const static br_demo_dispatch dispatch = {
	.init          = SpaceRigInit,
	.process_event = BrDemoDefaultProcessEvent,
	.update        = SpaceRigUpdate,
	.render        = BrDemoDefaultRender,
	.on_resize     = SpaceRigOnResize,
	.destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
	return BrDemoRunArgv("BRender Space Rig Demo", &dispatch, argc, argv);
}
