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

#include <SDL3/SDL.h>

/*
 * The demo's original DOS scancodes, mapped onto SDL3 scancodes. SC_EXTENDED
 * is kept at 0 so the original SC_C_* + SC_EXTENDED expressions still name
 * the cursor keys.
 */
#define SC_EXTENDED            0
#define SC_SPACE               SDL_SCANCODE_SPACE
#define SC_C_UP                SDL_SCANCODE_UP
#define SC_C_DOWN              SDL_SCANCODE_DOWN
#define SC_C_LEFT              SDL_SCANCODE_LEFT
#define SC_C_RIGHT             SDL_SCANCODE_RIGHT
#define SC_COMMA               SDL_SCANCODE_COMMA
#define SC_DOT                 SDL_SCANCODE_PERIOD
#define SC_8                   SDL_SCANCODE_8
#define SC_F1                  SDL_SCANCODE_F1
#define SC_ESC                 SDL_SCANCODE_ESCAPE
#define SC_PG_UP               SDL_SCANCODE_PAGEUP
#define SC_PG_DOWN             SDL_SCANCODE_PAGEDOWN
#define SC_INSERT              SDL_SCANCODE_INSERT
#define SC_TAB                 SDL_SCANCODE_TAB
#define SC_HOME                SDL_SCANCODE_HOME
#define SC_LSHIFT              SDL_SCANCODE_LSHIFT
#define SC_RSHIFT              SDL_SCANCODE_RSHIFT
#define SC_A                   SDL_SCANCODE_A
#define SC_C                   SDL_SCANCODE_C
#define SC_I                   SDL_SCANCODE_I
#define SC_L                   SDL_SCANCODE_L
#define SC_N                   SDL_SCANCODE_N
#define SC_P                   SDL_SCANCODE_P
#define SC_Q                   SDL_SCANCODE_Q
#define SC_R                   SDL_SCANCODE_R
#define SC_X                   SDL_SCANCODE_X
#define SC_Z                   SDL_SCANCODE_Z

#define REPT_FIRST_DOWN        1

#define WHITE                  0
#define BLUE                   1
#define GREEN                  2
#define CYAN                   3
#define RED                    4
#define MAGENTA                5
#define YELLOW                 6

#define STARPIX                "starw.pix"
#define ENERGYPIX              "energyw.pix"
#define PAGEPIX                "pagew.pix"
#define BASE_BGPIX             "base_bgw.pix"
#define ALCWALLPIX             "alcwallw.pix"
#define TITLE_LOPIX            "title_lw.pix"
#define TITLE_HIPIX            "title_hw.pix"

#define MAX_TEXTURES           10
#define X                      0
#define Y                      1
#define Z                      2
#define GET_PIXEL_16(pm, x, y) (*((br_uint_16 *)((char *)pm->pixels + pm->row_bytes * (y)) + (x)))
#define MIN(a, b)              ((a) < (b) ? (a) : (b))

void Show_Help(br_pixelmap *colour_buffer);

void Add_Camera_Probe_Info(br_pixelmap *colour_buffer, int flag);

void Add_Screen_Info(br_pixelmap *colour_buffer, br_pixelmap *screen, br_uint_16 *Collision_Detect_X, br_uint_16 *Collision_Detect_Y,
                     br_scalar collision_distance1, br_scalar collision_distance2, br_scalar collision_distance3,
                     br_scalar collision_distance, br_actor *my_view, br_boolean collision, int stardraw, int light_tag);

void Init_World_Bounds(br_actor *parent, br_actor **new_actor);

void TextureMapLoad();

void CalcBezierBasis(br_scalar t, br_matrix4 *probe_path, br_vector4 *probe_centre, br_vector4 *v_t);

void Init_Actor(br_actor *parent, br_actor **new_actor, char *modelname, char *material, br_boolean load_flag, br_boolean material_flag,
                float X_axis_rotation, float X_translation, float Y_translation, float Z_translation);

void Init_Camera(br_actor *parent, br_actor **new_observer);

void Init_Light(br_actor *parent, br_actor **light, br_scalar atten);

br_pixelmap *BR_CALLBACK LoadMapFFHook(const char *name);

br_scalar DimLight(br_pixelmap *colour_buffer, br_actor *light1, br_scalar atten);

br_scalar BrightenLight(br_pixelmap *colour_buffer, br_actor *light1, br_scalar atten);

int Light_on_off(br_actor *light1, int light_tag, int present_level);

void Animate_box(br_actor *box, br_actor *argo_null, br_actor *argo_blu, br_actor *argo_grn, br_actor *argo_yel, br_actor *argo_red);

br_angle Animate_wall_texture(char *material_name, br_angle angle);

int ChangeTVimage(int count, br_material *change);

int ChangeMaps(int count, br_material *flick, /* flick textures on video walls  */
               br_material *swap,             /* swap textures on airlock walls */
               br_material *pulse_wave);      /* pulse textures on column       */

void MoveDoors(br_actor *first_door, br_actor *second_door);

void RiseDoors(br_actor *first_door, br_actor *second_door);
