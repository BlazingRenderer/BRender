//////////////////////////////////////////////////////////////////////
//
// Project  : BRender Forest Demo
//            Copyright (c) 1996 Argonaut Technologies Ltd.
//            All rights reserved.
//
// Author         : Simon Everett
// Demo Framework : Steve Williams 
//
// File     : Forest.c
//
// Requires : BRender V1.3
//            Microsoft DirectX 3.0, Win95/NT or DOS
//
// Comments : Forest z-sort demo
//
//////////////////////////////////////////////////////////////////////

#include "..\misc\br_demo.h"
#include "forest.h"
#include <string.h>

#define PRIMITIVE_HEAP  1024000 /* BRender Scratch Workspace */
#define TILE_SIZE   256         /* Tile size for textures    */

// Uses small 32x32 textures if 0, 
// if 0, selects 256x256 versions.  

static br_boolean bUseSmall = BR_FALSE ;	


br_pixelmap *screen_buffer, *back_buffer, *palette;
br_actor *root, *camera, *light;

int counter = 0, animate = 1, counter1, counter2, counter3, auto_change, frames;
br_scalar xpos, ypos, zpos;
extern br_uint_8 debug;

br_uint_32 time = 0, max_time = 0;
int count;

br_uint_16 Num_Materials = 0 ;
br_material *pMaterial[32];

br_uint_32 NumCameraModes, CurrentCameraMode;


typedef struct {
   char *filename;
   br_uint_16 clut_x;
   br_uint_16 clut_y;
   br_uint_8 palette_trans;
   br_uint_16 tpage_x;
   br_uint_16 tpage_y;
       
} BR_BITMAPS ;


void Add_Textures( BR_BITMAPS *Bitmaps );


BR_BITMAPS Small_Bitmaps[] =
{
   {"spcship.pix", 512, 256+241, 1, 448, 0},
   {"sfloor.pix", 512, 256+240, 1, 576, 0},
   {"splant.pix", 512, 256+244, 0, 640, 0},
   {"scolumn.pix", 512, 256+246, 1, 512, 256},
   {"scloud.pix", 512, 256+245, 0, 448, 256},
   {"",0,0,0,0,0},
};
   
BR_BITMAPS Big_Bitmaps[] =
{   
   {"spcship.pix", 512, 256+241, 1, 448, 0},
   {"floor.pix", 512, 256+240, 1, 576, 0},
   {"plant.pix", 512, 256+244, 0, 640, 0},
   {"column.pix", 512, 256+246, 1, 512, 256},
   {"cloud.pix", 512, 256+245, 0, 448, 256},
   {"",0,0,0,0,0},
};


// Prototypes for CameraUpdate#, CameraReset#
void CameraUpdate0(); void CameraReset0();
void CameraUpdate1(); void CameraReset1();
void CameraUpdate2(); void CameraReset2();
void CameraUpdate3(); void CameraReset3();

typedef void camera_fn(void);

struct {
   camera_fn *update;
   camera_fn *reset;
   br_uint_32 frames;

} CameraModes[] = {

   {   CameraUpdate0, CameraReset0, 1800,   },
   {   CameraUpdate1, CameraReset1, 1800,   },
   {   CameraUpdate2, CameraReset2, 900,   },
   {   CameraUpdate3, CameraReset3, 1800,   },
};


br_actor *pivot0, *floor_actor, *object, *column[6], *pivot1;
br_order_table *primary_order_table;
br_matrix34 *loop;

br_uint_32 *ot_start[2];
br_uint_32 CameraLookAt;

br_uint_8 debug = 1;
br_uint_32 *bucket[2];

int demoend;


void Zap( char *name )
{
    br_pixelmap *pm ;

	pm = BrMapFind( name );
	
	if ( pm )
	{
		BrMapRemove(pm);
		BrPixelmapFree(pm);
	}
}


br_error ToggleTextureCallback(void)
{
    bUseSmall ^= 1 ;

	BrDemo_Restart();

    return BRE_OK ;
}


void Add_Textures( BR_BITMAPS *Bitmaps )
{
   br_uint_8 i = 0 ;
   br_pixelmap *pm ;

   for( ;; )
   {
       if ( Bitmaps[i].filename[0] == '\0' )
		   break ;
	   
	   if((pm = BrPixelmapLoad(Bitmaps[i].filename)) == NULL)
          BR_ERROR1("Unable to load texture '%s'", Bitmaps[i].filename );
     
       if (pm)
          BrMapAdd(pm);

	   i++ ;
    }
}



// Generic setup of Gfx, BRender

br_error BrDemo_InitApp(int argc, char **argv)
{
   void *primitive_heap;
   int i;

   br_pixelmap *pm; 
   br_model *model;
   br_error r ;
   

   r = BrDemo_Allocate_Pixelmaps( "Forest Demo",
                               &screen_buffer,&back_buffer,NULL,
                               &palette, "forest.pal");
   if ( r != BRE_OK )
	   return r ;

   // Set offscreen buffer origin

   back_buffer->origin_x = back_buffer->width/2;
   back_buffer->origin_y = back_buffer->height/2;

   if ( screen_buffer->type == BR_PMT_RGB_888 )
	   return BRE_OK ;

   primitive_heap = BrMemAllocate(PRIMITIVE_HEAP, BR_MEMORY_APPLICATION);

   // Start up zsort renderer
    
   BrZsBegin(screen_buffer->type, primitive_heap, PRIMITIVE_HEAP);
        
   // Setup generic data for samples
    
   root = BrActorAllocate(BR_ACTOR_NONE, NULL);
   root->identifier = strdup("main_root");
   root->t.type = BR_TRANSFORM_MATRIX34;
   BrMatrix34Identity(&root->t.t.mat);

   // BrDemo_SetRootActor lets the demo framework know which 
   // actor to enumerate when turning on/off actor based properties.
   
   BrDemo_SetRootActor(root);

   camera = BrActorAdd(root, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
   camera->identifier = "camera";
   ((br_camera *)(camera->type_data))->type = BR_CAMERA_PERSPECTIVE;
   ((br_camera *)(camera->type_data))->aspect = BR_SCALAR(1.21);
   ((br_camera *)(camera->type_data))->hither_z = BR_SCALAR(0.05);
   ((br_camera *)(camera->type_data))->yon_z = BR_SCALAR(100);
    ((br_camera *)(camera->type_data))->field_of_view = BR_ANGLE_DEG(45.0);
   
    camera->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Translate(&camera->t.t.mat, 0, 0, BR_SCALAR(1));

   light = BrActorAdd(root, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
        light->identifier = strdup("main_light");
        light->t.type = BR_TRANSFORM_MATRIX34;
        ((br_light *)light->type_data)->colour = BR_COLOUR_RGB(255, 255, 255);
        BrMatrix34Identity(&light->t.t.mat);

   BrLightEnable(light);
   
         // setup order table at root (generate new bounds,
         // use average of 3 vertices for sorting buckets)
        
   BrZsActorOrderTableSet(root,
             BrZsOrderTableAllocate(1023, BR_ORDER_TABLE_NEW_BOUNDS |
                                    BR_ORDER_TABLE_SORT_NEAR, BR_SORT_MAX));

    pm = NULL ;

    switch ( screen_buffer->type )
    {
       case BR_PMT_INDEX_8 : pm = BrPixelmapLoad("forest8.tab");
                             break ;
                                                       
       case BR_PMT_RGB_555 : pm = BrPixelmapLoad("forest15.tab");
                             break ;

       case BR_PMT_RGB_565 : pm = BrPixelmapLoad("forest16.tab");
                             break ;
   }

   if ( !pm )
   {
       BR_ERROR0("Couldn't load shade table");
       return BRE_FAIL ;
   }

   BrTableAdd(pm);

   NumCameraModes = BR_ASIZE(CameraModes);
   CurrentCameraMode = 0;
   frames = CameraModes[CurrentCameraMode].frames;
   auto_change = 1;
        
   if ( bUseSmall )
       Add_Textures( &Small_Bitmaps );
   else
	   Add_Textures( &Big_Bitmaps );
   
   // load some materials from script

   Num_Materials = BrFmtScriptMaterialLoadMany( "forest.mat", pMaterial, 32);

   BrMaterialAddMany(pMaterial, Num_Materials );

    // load model
   
   pivot0 = BrActorAllocate(BR_ACTOR_NONE, NULL);
   pivot0->t.type = BR_TRANSFORM_MATRIX34;
   BrMatrix34Translate(&pivot0->t.t.mat, xpos, ypos, zpos);

   pivot1 = BrActorAdd(pivot0, BrActorAllocate(BR_ACTOR_NONE, NULL));
   pivot1->t.type = BR_TRANSFORM_MATRIX34;
   BrMatrix34Identity(&pivot1->t.t.mat);

   CameraModes[CurrentCameraMode].reset();

   floor_actor = BrActorAdd(pivot1, BrActorAllocate(BR_ACTOR_MODEL, NULL));
   model = BrModelLoad("floor.dat");
   
   floor_actor->model = BrModelAdd(model);
   floor_actor->t.type = BR_TRANSFORM_MATRIX34;
   BrMatrix34Translate(&floor_actor->t.t.mat, 0, BR_SCALAR(-5), 0);

   model = BrModelLoad("column.dat");
   
   if ( model )
	   BrModelAdd(model);
   
   for(i=0; i<4; i++) 
   {
      column[i] = BrActorAdd(pivot1, BrActorAllocate(BR_ACTOR_MODEL, NULL));
                column[i]->model = model;
                column[i]->t.type = BR_TRANSFORM_MATRIX34;

                BrMatrix34Translate(&column[i]->t.t.mat, BR_SCALAR(18), 0, 0);
                BrMatrix34PostRotateY(&column[i]->t.t.mat, BR_ANGLE_DEG(i*(360/4) + (360/4)/2));
   }

   object = BrActorAdd(pivot1, BrActorAllocate(BR_ACTOR_MODEL, NULL));
        object->model = BrModelLoad("ship.dat");
        object->model = BrModelAdd(object->model);

        object->t.type = BR_TRANSFORM_MATRIX34;

        primary_order_table = BrZsOrderTableAllocate(2048, BR_ORDER_TABLE_LEAVE_BOUNDS, BR_SORT_MIN);
        primary_order_table->min_z = BR_SCALAR(0);
        primary_order_table->max_z = BR_SCALAR(100);

        BrZsActorOrderTableSet(object, BrZsOrderTableAllocate(1024, BR_ORDER_TABLE_NEW_BOUNDS | BR_ORDER_TABLE_SORT_NEAR, BR_SORT_MAX));

   BrZsOrderTablePrimaryEnable(primary_order_table);
   BrZsActorOrderTableSet(root, primary_order_table);

   BrActorAdd(root, pivot0);

   counter = 0, counter1=0, counter2 = 0, counter3 = 0;

        loop = BrMemAllocate(sizeof(br_matrix34) * 360, BR_MEMORY_APPLICATION);

   for(i=0; i<=180; i++) 
   {

      counter += 2;
      if(counter >= 360) counter -= 360;
      counter1 += 4;
      if(counter1 >= 360) counter1 -= 360;
   
      BrMatrix34Translate(&object->t.t.mat, BR_ADD(BR_MUL(BR_SIN(BR_ANGLE_DEG(counter1)), BR_SCALAR(8)), BR_SCALAR(18)), BR_SIN(BR_ANGLE_DEG(counter1*2)), 0);
      BrMatrix34PostRotateY(&object->t.t.mat, BR_ANGLE_DEG(counter));
      BrMatrix34PreRotateZ(&object->t.t.mat, BR_CONST_DIV(BR_SIN(BR_ANGLE_DEG(counter1)), 10));

      BrMatrix34Copy(loop + i, &object->t.t.mat);
   }

   counter=0, counter1=0;

   // Add additional menu item to allow selection of textures

   BrDemo_AddMenuItem( "&Toggle textures", &ToggleTextureCallback );

   return BRE_OK;
}


br_error BrDemo_ShutDown(void)
{
   BrPixelmapFree(back_buffer);
   back_buffer = NULL;

   BrZsEnd(); // terminate Bucket Z Sort Renderer

   return BRE_OK ;
}


br_uint_32 BR_CALLBACK ActorEnumCbfn(br_actor *mat, void *arg)
{
   (*(br_uint_32 *)arg)++;
    
    return BrActorEnum(mat, ActorEnumCbfn, arg);
}

br_error Render_Scene(void)
{
   BrPixelmapFill(back_buffer, 0);

   BrZsSceneRenderBegin(root, camera, back_buffer);
   BrZsSceneRenderAdd(pivot0);
   BrZsSceneRenderEnd();

   return BRE_OK ;
}


void Animate_Scene(void)
{
   br_transform xform;
   
   counter++;
   if(counter >= 180) counter -= 180;

   counter1 += 2;
   if(counter1 >= 360) counter1 -= 360;
   
   counter2++;
   if(counter2 >= 60) counter2 -= 60;

   if((counter2 % 2) == 0)
      counter3 += 1;
   if(counter3 == 64) counter3 = 0;

   if(animate == 1) {
                BrMatrix34Copy(&object->t.t.mat, loop + counter);
   }

   if(CameraLookAt == 1) 
   {
      BrVector3SetInt(&camera->t.t.look_up.up, 0, 1, 0);
      BrVector3Set(&camera->t.t.look_up.t, 0, 0, 0);
   }

        BrMatrix34Translate(&pivot0->t.t.mat, xpos, ypos, zpos);

   CameraModes[CurrentCameraMode].update();

   if ( CameraLookAt == 1 ) 
   {
      BrActorToActorMatrix34(&xform.t.mat, object, camera->parent);
      BrVector3Sub(&camera->t.t.look_up.look, &xform.t.translate.t, &camera->t.t.look_up.t);
   }

}

void Input_Test(void)
{
   static br_int_32 mouse_x, mouse_y, prevmouse_x, prevmouse_y;
   static br_boolean mouse_start = BR_FALSE;
   br_uint_32 mouse_buttons;
   int start = 0;
   br_boolean bButtonDown ;

   if(DOSKeyTest(SC_Q, 0, REPT_FIRST_DOWN))
       demoend = -1;

   if(auto_change == 1) 
   {
      frames--;

      if ( frames == 0 )
         start = 1;
   }

   if ( DOSKeyTest(SC_A, 0, REPT_FIRST_DOWN)) 
   {
       animate ^= 1;
   }

   if(DOSKeyTest(SC_C, 0, REPT_FIRST_DOWN)) 
      auto_change ^= 1;

   if(start || DOSKeyTest(SC_SPACE, 0, REPT_FIRST_DOWN)) 
   {
       CurrentCameraMode = (CurrentCameraMode + 1) % NumCameraModes;
       CameraModes[CurrentCameraMode].reset();

       frames = CameraModes[CurrentCameraMode].frames;
   }

   if(DOSKeyTest(SC_C_UP, 0, REPT_AUTO_REPT)) 
	   ypos = BR_ADD(ypos,BR_SCALAR(0.5));
   
   if(DOSKeyTest(SC_C_DOWN, 0, REPT_AUTO_REPT)) 
	   ypos = BR_SUB(ypos,BR_SCALAR(0.5));

   if(DOSKeyTest(SC_C_LEFT, 0, REPT_AUTO_REPT)) 
	   xpos = BR_SUB(xpos,BR_SCALAR(0.5));
   
   if(DOSKeyTest(SC_C_RIGHT, 0, REPT_AUTO_REPT)) 
	   xpos = BR_ADD(xpos,BR_SCALAR(0.5));

   if(DOSKeyTest(SC_1, 0, REPT_AUTO_REPT)) 
       zpos = BR_ADD(zpos,BR_SCALAR(0.5));
   
   if(DOSKeyTest(SC_0, 0, REPT_AUTO_REPT)) 
       zpos = BR_SUB(zpos,BR_SCALAR(0.5));
   

   DOSMouseRead(&mouse_x, &mouse_y, &mouse_buttons);

   bButtonDown = (mouse_buttons & BR_MSM_BUTTONL) | (mouse_buttons & BR_MSM_BUTTONR);

   if ( bButtonDown && ( ( mouse_x != prevmouse_x ) || (mouse_y != prevmouse_y) ) )
   {             
	    if(mouse_start)
        {
             xpos = BR_ADD(xpos, BR_DIV(BR_SCALAR(mouse_x - prevmouse_x),BR_SCALAR(16)));
 
			 if ( mouse_buttons & BR_MSM_BUTTONL )
                 ypos = BR_ADD(ypos, BR_DIV(BR_SCALAR(mouse_y - prevmouse_y),BR_SCALAR(16)));

			 if ( mouse_buttons & BR_MSM_BUTTONR )
                 zpos = BR_ADD(zpos, BR_DIV(BR_SCALAR(mouse_y - prevmouse_y),BR_SCALAR(16)));
		} 
		else
            mouse_start = BR_TRUE;
   }
	
   prevmouse_x = mouse_x;
   prevmouse_y = mouse_y;

   if(ypos > BR_SCALAR(3)) ypos = BR_SCALAR(3);
   if(ypos < BR_SCALAR(-60)) ypos = BR_SCALAR(-60);

   if(zpos > BR_SCALAR(80)) zpos = BR_SCALAR(80);
   if(zpos < BR_SCALAR(-80)) zpos = BR_SCALAR(-80);

   if(xpos > BR_SCALAR(80)) xpos = BR_SCALAR(80);
   if(xpos < BR_SCALAR(-80)) xpos = BR_SCALAR(-80);

}

br_error BrDemo_Render_Loop(void) 
{

   if (!back_buffer) 
      return BRE_OK;

   if ( back_buffer->type == BR_PMT_RGB_888 )
   {
	   BrPixelmapTextF( back_buffer, 100 - back_buffer->origin_x, 100 - back_buffer->origin_y,
                     0xFFFFFF, BrFontFixed3x5, "This demo does not support 24 bit mode" );
   }
   else
   {
      Input_Test(); // Generic Pad Test
	   
      Animate_Scene(); // Update Data Structures
      Render_Scene();  // Render World

      count = 1; // root

      BrActorEnum(root, ActorEnumCbfn, &count);

      if (demoend == -1) 
         return BRE_FAIL ;
    }		  

    BrPixelmapDoubleBuffer( screen_buffer, back_buffer);

    return BRE_OK ;
}

void CameraReset0(void)
{
   BrMatrix34Identity(&pivot1->t.t.mat);
   camera->t.type = BR_TRANSFORM_LOOK_UP;
   xpos = 0; ypos = BR_SCALAR(-6); zpos = BR_SCALAR(-30);
   CameraLookAt=1;
}

void CameraUpdate0(void)
{
}

void CameraReset1(void)
{
   camera->t.type = BR_TRANSFORM_LOOK_UP;
   xpos = 0; ypos = BR_SCALAR(-7); zpos = BR_SCALAR(8);
   CameraLookAt=1;
}

void CameraUpdate1(void)
{
}

void CameraReset2(void)
{
   camera->t.type = BR_TRANSFORM_MATRIX34;
   CameraLookAt=0;
}

void CameraUpdate2(void)
{
   BrMatrix34Copy(&camera->t.t.mat, loop + ((counter + 170)%180));
   BrMatrix34PreTranslate(&camera->t.t.mat, xpos, ypos, zpos);
}

void CameraReset3(void)
{
   BrMatrix34Identity(&pivot1->t.t.mat);
   camera->t.type = BR_TRANSFORM_LOOK_UP;
   xpos = 0; ypos = BR_SCALAR(-20); zpos = BR_SCALAR(-50);
   CameraLookAt=1;
}

void CameraUpdate3(void)
{
    BrMatrix34RotateY(&pivot1->t.t.mat, BR_ANGLE_DEG(counter1));
}

void VBL_cbfn(br_uint_8 db_index)
{
    BrZsSceneRender(root, camera, back_buffer);
    BrPixelmapDoubleBuffer(screen_buffer, back_buffer);
}


br_error BrDemo_OnResize() 
{
   BrDemo_Refresh_Pointers( &screen_buffer, &back_buffer, NULL );

   // Set offscreen buffer origin

   back_buffer->origin_x = back_buffer->width/2;
   back_buffer->origin_y = back_buffer->height/2;

   return BRE_OK ;
}
