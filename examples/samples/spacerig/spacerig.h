//////////////////////////////////////////////////////////////////////
//
// Project  : BRender SpaceRig Demo
//            Copyright (c) 1996 Argonaut Technologies Ltd.
//            All rights reserved.
//
// Author        : Tony Roberts
// Modifications : Owain Green - Modified to work with new demo
//                 framework.
//
// File     : SPACERIG.H
//
// Requires : BRender V1.3
//            Microsoft DirectX 3.0, Win95/NT or DOS
//			   
// Comments : SpaceRig specific source.
//
//////////////////////////////////////////////////////////////////////


#define WHITE   0
#define BLUE    1
#define GREEN   2 
#define CYAN    3
#define RED     4 
#define MAGENTA 5
#define YELLOW  6

#define STARPIX "starw.pix"
#define ENERGYPIX "energyw.pix"
#define PAGEPIX "pagew.pix"
#define BASE_BGPIX "base_bgw.pix"
#define ALCWALLPIX "alcwallw.pix"
#define TITLE_LOPIX "title_lw.pix"
#define TITLE_HIPIX "title_hw.pix"


#define MAX_TEXTURES 10
#define X 0
#define Y 1
#define Z 2
#define GET_PIXEL_16(pm,x,y) (*((br_uint_16*)((char *)pm->pixels + pm->row_bytes * (y)) +(x)))
#define MIN(a,b) ((a)<(b)?(a):(b))

void Show_Help( br_pixelmap *colour_buffer );

void Add_Camera_Probe_Info( br_pixelmap *colour_buffer, int flag );

void Add_Screen_Info( br_pixelmap *colour_buffer, br_pixelmap *screen,
                      br_uint_16 *Collision_Detect_X,
                      br_uint_16 *Collision_Detect_Y,
                      br_uint_16 depth_value1, br_uint_16 depth_value2, br_uint_16 depth_value3,
                      br_uint_16 depth_value, br_actor *my_view, br_boolean collision,
                      int stardraw, int light_tag );

void Init_World_Bounds(br_actor *parent,
                       br_actor **new_actor);

void TextureMapLoad();

void CalcBezierBasis(br_scalar t,
                     br_matrix4 *probe_path, 
                     br_vector4 *probe_centre, 
                     br_vector4 *v_t); 

br_pixelmap *Load_Intro_Screen(br_pixelmap *screen);
    
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
                 br_boolean keep_flag);

void Init_Camera(br_actor *parent, br_actor **new_observer);

void Init_Light(br_actor *parent, br_actor **light, br_scalar atten);

void Quit();

br_pixelmap *BR_CALLBACK LoadMapFFHook(char *name); 

br_scalar DimLight(br_pixelmap *colour_buffer, br_actor *light1, br_scalar atten);

br_scalar BrightenLight(br_pixelmap *colour_buffer, br_actor *light1, br_scalar atten);

int Light_on_off(br_actor *light1, int light_tag, int present_level);

br_uint_16 EnlargeScreen( br_pixelmap *colour_buffer,
                          br_pixelmap *window,
                          br_uint_16 screen_scale,
                          br_uint_16 curr_width,
                          br_uint_16 curr_height,
                          br_uint_16 window_dx,
                          br_uint_16 window_dy,
                          br_uint_16 *Collision_Detect_X,
                          br_uint_16 *Collision_Detect_Y
                        );

void Animate_box(br_actor *box,
                 br_actor *argo_null,
                 br_actor *argo_blu,
                 br_actor *argo_grn,
                 br_actor *argo_yel,
                 br_actor *argo_red);

int Animate_wall_texture(char *material_name,
                          int angle);

int ChangeTVimage(int count,
                  br_material *change);

int ChangeMaps(int count,
               br_material *flick,       /* flick textures on video walls  */
               br_material *swap,        /* swap textures on airlock walls */
               br_material *pulse_wave); /* pulse textures on column       */

void MoveDoors(br_actor *first_door,
               br_actor *second_door);

void RiseDoors(br_actor *first_door,
               br_actor *second_door);
