/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brintro.c 1.1 1995/02/22 21:36:54 joey Exp $
 * $Locker: $
 *
 * BRender intro sequence
 *
 *	22/2/95	SE	Runs under BRender v 1.1, 8 bit indexed
 *	22/5/95 SE	Added video card timing
 *	28/5/95 SE	Fix for last frame sphere/word pixel bug thing...
 *	08/9/95	SE	Animation changed to matrix stream
 *			Fixed for all res 8 bit indexed
 *	13/9/95 SE	Error codes, clean exit
 *	14/9/95	SE	Fixed for v1.1.2
 *
 * NB. DOS 8 bit indexed only!  Artwork needs to be reworked for true colour.
 *
 * pass pointer to screen device pixelmap (th'one returned by DOSGfxBegin)
 *
 * return	
 *   		0	no error
 *		1	failed to load pixelmaps
 *		2	failed to find materials
 *		3	failed to find shade table
 *		4	failed to load models
 *		5	failed to open animation file
 *		6	failed to load animation
 *
 *		hardware palette contains all (rgb = 0,0,0)
 *		screen all index 0
 *
 * needs BrBegin and DOSGfxBegin to have already been performed
 * doesn't need (and like) BrZbBegin to have been performed yet!
 *
 * Handy hint!	Call this before you set up colour and z buffers as this
 *		will use sooo much memory because local c & z buffers
 *		are used here.
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "brender.h"
#include "dosio.h"
#include "syshost.h"

#define intro_num_models	2 /* brender, ball */
#define intro_num_pix		4 /* 2 textures, palette, shade table */
#define background_colour	0

#define ANIM_SIZE	98	/* number of frames of animation in anim file */
#define magic_dust	18

/*
 * error codes
 */
enum {
    	EXIT_NORMAL	= 0x0000,	/* no error */
	EXIT_PIXELMAPS,			/* failed to load pixelmaps */
	EXIT_MATERIALS,			/* failed to find materials */
	EXIT_SHADE_TABLE,		/* failed to find shade table */
	EXIT_MODELS,			/* failed to load models */
	EXIT_OPEN_ANIM,			/* failed to open animation file */
	EXIT_LOAD_ANIM,			/* failed to load animation */
};

static char *intro_datfile =	{"intro.dat"};
static char *intro_pixfile =	{"intro.pix"};
static char *intro_animfile =	{"intro.anm"};

/*
 * support function prototypes
 */
static br_uint_32 BR_CALLBACK ModelEnum(br_model *item, void *arg);
static br_uint_32 BR_CALLBACK MapEnum(br_pixelmap *item, void *arg);
static br_uint_32 BR_CALLBACK MaterialEnum(br_material *item, void *arg);

static void BR_CALLBACK RenderBoundsCallback(br_actor *actor,
		br_model *model,
		br_material *material,
		void *render_data,
		br_uint_8 style,
		br_matrix4 *model_to_screen,
		br_int_32 bounds[4]);
static void (BR_CALLBACK *oldrendercallback)(br_actor *actor,
		br_model *model,
		br_material *material,
		void *render_data,
		br_uint_8 style,
		br_matrix4 *model_to_screen,
		br_int_32 bounds[4]);
static void WaitVbl(void);
static int ShutDown(int exitval, br_pixelmap *screen);

/*
 * global variables
 */
static int render_bounds[intro_num_models][4];	/* dirty rectangle array */
static int model_number;

static br_material word_mat = {
	"intro_word",
	BR_COLOUR_RGB(0,0,0),
	0,
	BR_UFRACTION(0.20),
	BR_UFRACTION(0.60),
	BR_UFRACTION(0.0),
	BR_SCALAR(0.0),
	BR_MATF_LIGHT,
	{{BR_VECTOR2(1,0),BR_VECTOR2(0,1),BR_VECTOR2(0,0)}},
	0,63,
};

static br_material xplos_mat = {
	"intro_xplos",
	BR_COLOUR_RGB(0,0,0),
	0,
	BR_UFRACTION(0.20),
	BR_UFRACTION(0.65),
	BR_UFRACTION(0.30),
	BR_SCALAR(20),
	BR_MATF_LIGHT | BR_MATF_SMOOTH,
	{{BR_VECTOR2(1,0),BR_VECTOR2(0,1),BR_VECTOR2(0,0)}},
	0,63,
};

static br_pixelmap *colour_buffer, *z_buffer, *palette, **pixtable;
static br_actor *world, *camera, *light, *sphere, *word;
static br_model **mtable;
static br_matrix34 *animation_sequence;
static void *fh;

int BrenderIntro(br_pixelmap *screen)
{
	br_material *material;
	
	unsigned int frame,time1,time2, total_time;
	float num_frames;
	int nmodels,npix;
	int i,anim_loop,anim_step,anim_start;
	int r,g,b;
	int finished;

	int open_mode = BR_FS_MODE_BINARY;
	
	/*
	 * start zb, set up display
	 */
	colour_buffer = BrPixelmapMatch(screen,BR_PMMATCH_OFFSCREEN);
	z_buffer = BrPixelmapMatch(colour_buffer,BR_PMMATCH_DEPTH_16);

	oldrendercallback = NULL;

	BrZbBegin(colour_buffer->type,z_buffer->type);

	/*
	 * load palette
	 */
	pixtable = BrMemCalloc(sizeof(*pixtable),intro_num_pix,BR_MEMORY_APPLICATION);
	if((npix = BrPixelmapLoadMany(intro_pixfile,pixtable,intro_num_pix)) != intro_num_pix)
		return ShutDown(EXIT_PIXELMAPS, screen);

	BrMapAddMany(pixtable,intro_num_pix);

	if((palette = BrMapFind("intro_palette")) == NULL)
		return ShutDown(EXIT_PIXELMAPS, screen);

	DOSGfxPaletteSet(palette);

	/*
	 * fix materials
	 */
	material = BrMaterialAllocate(NULL);
	*material = word_mat;
	if((material->colour_map = BrMapFind("intro_word")) == NULL)
		return ShutDown(EXIT_MATERIALS, screen);

	if((material->index_shade = BrMapFind("intro_shade_table")) == NULL)
		return ShutDown(EXIT_SHADE_TABLE, screen);
	BrMaterialAdd(material);

	material = BrMaterialAllocate(NULL);
	*material = xplos_mat;
	if((material->colour_map = BrMapFind("intro_xplos")) == NULL)
		return ShutDown(EXIT_MATERIALS, screen);
		
	material->index_shade = BrMapFind("intro_shade_table");
	BrMaterialAdd(material);
	
	/*
	 * load intro models (2)
	 */
	mtable = BrMemCalloc(sizeof(*mtable),intro_num_models,BR_MEMORY_APPLICATION);
	if((nmodels = BrModelLoadMany(intro_datfile,mtable,intro_num_models)) != intro_num_models)
		return ShutDown(EXIT_MODELS, screen);

	BrModelAddMany(mtable,intro_num_models);

	/*
	 * load intro animation (stream of matricies, 2 matricies per frame, ANIM_SIZE frames)
	 */
	animation_sequence = BrMemCalloc(sizeof(*animation_sequence), ANIM_SIZE*2, BR_MEMORY_APPLICATION);
	 
	if((fh = BrFileOpenRead(intro_animfile, 0, NULL, &open_mode)) == NULL)
		return ShutDown(EXIT_OPEN_ANIM, screen);

	if(BrFileRead(animation_sequence, sizeof(br_matrix34), ANIM_SIZE*2, fh) != ANIM_SIZE*2)
		return ShutDown(EXIT_LOAD_ANIM, screen);

	BrFileClose(fh);
	fh = NULL;

	/*
	 * setup world
	 */
	world = BrActorAllocate(BR_ACTOR_NONE,NULL);
	camera = BrActorAdd(world,BrActorAllocate(BR_ACTOR_CAMERA,NULL));
	light = BrActorAdd(world,BrActorAllocate(BR_ACTOR_LIGHT,NULL));

	sphere = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL,NULL));
	word = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL,NULL));

	word->model = BrModelFind("intro_word");
	sphere->model = BrModelFind("intro_sphere");

	/*
	 * setup camera, orientation of light (start, over left shoulder)
	 */
	((br_camera *)(camera->type_data))->aspect = BR_SCALAR(1.46);
	((br_camera *)(camera->type_data))->hither_z = BR_SCALAR(0.01);

	BrMatrix34Translate(&camera->t.t.mat,BR_SCALAR(0),BR_SCALAR(0),BR_SCALAR(2.0));
	BrMatrix34RotateY(&light->t.t.mat,BR_ANGLE_DEG(-45));
	BrMatrix34PostRotateZ(&light->t.t.mat,BR_ANGLE_DEG(45));
	BrLightEnable(light);

	/*
	 * setup dirty rectangles
	 */
	oldrendercallback = BrZbSetRenderBoundsCallback(RenderBoundsCallback);

	for(i=0; i<intro_num_models; i++) {

	    	render_bounds[i][0] = 0;
		render_bounds[i][1] = 0;
		render_bounds[i][2] = screen->width;
		render_bounds[i][3] = screen->height;
	}
	/*
	 * how long does it take to render scene?
	 * avg over 10 frames
	 */
	DOSClockBegin();
	time1 = DOSClockRead();

	for(anim_loop = 0; anim_loop < 10; anim_loop++) {
		BrZbSceneRender(world,camera,colour_buffer,z_buffer);	/* test render */

		/*
		 * make sure nothing appears on screen
		 */
		for(i=0, model_number = 0; i<intro_num_models; i++) {
			BrPixelmapRectangleFill(colour_buffer,render_bounds[i][0]-1,render_bounds[i][1]-1,
					render_bounds[i][2]-render_bounds[i][0]+2,
					render_bounds[i][3]-render_bounds[i][1]+2,
					background_colour);
				
			BrPixelmapRectangleFill(z_buffer,render_bounds[i][0]-1,render_bounds[i][1]-1,
					render_bounds[i][2]-render_bounds[i][0]+2,
					render_bounds[i][3]-render_bounds[i][1]+2,
					0xffffffff);
			}
			
		WaitVbl();
			
		colour_buffer = BrPixelmapDoubleBuffer(screen,colour_buffer); /* test video card */
	};

	time2 = DOSClockRead();
	total_time = time2-time1;
	num_frames = magic_dust - ((float)BR_DOS_CLOCK_RATE/((float)total_time/(float)anim_loop));
	anim_step = (int)num_frames;

	if(anim_step <= 0) anim_step = 1;
	if(anim_step > 3) anim_step = 3;

	anim_start = (ANIM_SIZE - 1) - (((ANIM_SIZE - 1)/anim_step)*anim_step);

	/*
	 * reset dirty rectangles
	 */
	for(i=0; i<intro_num_models; i++) {

	    	render_bounds[i][0] = 0;
		render_bounds[i][1] = 0;
		render_bounds[i][2] = screen->width;
		render_bounds[i][3] = screen->height;
	}

	for(anim_loop = anim_start, finished = 0; (anim_loop<ANIM_SIZE) && (!finished); anim_loop += anim_step) {

	    		/*
			 * clear dirty rectangles
			 */
			for(i=0, model_number = 0; i<intro_num_models; i++) {
				BrPixelmapRectangleFill(colour_buffer,render_bounds[i][0]-1,render_bounds[i][1]-1,
						render_bounds[i][2]-render_bounds[i][0]+2,
						render_bounds[i][3]-render_bounds[i][1]+2,
						background_colour);
					
				BrPixelmapRectangleFill(z_buffer,render_bounds[i][0]-1,render_bounds[i][1]-1,
						render_bounds[i][2]-render_bounds[i][0]+2,
						render_bounds[i][3]-render_bounds[i][1]+2,
						0xffffffff);
			}

			/*
			 * animate
			 */
			BrMatrix34Copy(&word->t.t.mat, animation_sequence + (2*anim_loop));
			BrMatrix34Copy(&sphere->t.t.mat, animation_sequence + ((2*anim_loop) + 1));

			/*
			 * render
			 */
			BrZbSceneRenderBegin(world, camera, colour_buffer, z_buffer);

			BrZbSceneRenderAdd(word);
			BrZbSceneRenderAdd(sphere);

			BrZbSceneRenderEnd();

			WaitVbl();

			colour_buffer = BrPixelmapDoubleBuffer(screen,colour_buffer);
		
			if(KBHIT()) finished = 1;
	}

	/*
	 * wait for a bit
	 */
	if(!finished) {
		time1 = DOSClockRead();
		while(DOSClockRead() < time1+BR_DOS_CLOCK_RATE);
	}

	return ShutDown(EXIT_NORMAL, screen);
}

static int ShutDown(int exitval, br_pixelmap *screen)
{
	br_colour *palette_entry;
	int i, anim_loop, r, g, b;

    	if(palette) {
		/*
		 * fade to black
		 */
		for(palette_entry = (br_colour *)palette->pixels, anim_loop=64; anim_loop > 0; anim_loop--) {
			for(i=0; i<256; i++) {
				r = BR_RED(palette_entry[i]) >> 2;
				g = BR_GRN(palette_entry[i]) >> 2;
				b = BR_BLU(palette_entry[i]) >> 2;
	
				if(r > 0)
					--r;
				if(g > 0)
					--g;
				if(b > 0)
					--b;
	
				palette_entry[i] = BR_COLOUR_RGB(r << 2,g << 2,b << 2);
			}
 		   	WaitVbl();
			DOSGfxPaletteSet(palette);
		}
	}

	BrPixelmapFill(colour_buffer,0);
	colour_buffer = BrPixelmapDoubleBuffer(screen,colour_buffer);

   	/*
	 * close any open files
	 */
	if(fh)
		BrFileClose(fh);
	
	/*
	 * kill all, return screen
	 */
	DOSClockEnd();
	BrZbEnd();

	/*
	 * clear my models, materials, pixelmaps
	 */
	if(light)	 
		BrLightDisable(light);

	if(oldrendercallback) 
		BrZbSetRenderBoundsCallback(oldrendercallback);

	/*
	 * clear new entries from registries
	 */
	BrModelEnum("intro_*", ModelEnum, NULL);
	BrMaterialEnum("intro_*", MaterialEnum, NULL);
	BrMapEnum("intro_*", MapEnum, NULL);

	/*
	 * release animation sequence
	 */
	if(animation_sequence)
		BrMemFree(animation_sequence);

	/*
	 * release hierarchy
	 */
	if(world)
		BrActorFree(world);

	/*
	 * clear tables
	 */
	if(mtable)		
		BrMemFree(mtable);

	if(pixtable)		
		BrMemFree(pixtable);

	/*
	 * free rendering areas
	 */
	if(colour_buffer)
		BrPixelmapFree(colour_buffer);

	if(z_buffer)
		BrPixelmapFree(z_buffer);

	return exitval;
}

static void WaitVbl(void)
{
    	unsigned int i;

	/*
	 * already in vbl?
	 */
	while(inp(0x3da) & 0x8);

	/*
	 * wait for next vbl
	 */
	while(!(inp(0x3da) & 0x8));
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

static void BR_CALLBACK RenderBoundsCallback(br_actor *actor,
						br_model *model,
						br_material *material,
						void *render_data,
						br_uint_8 style,
						br_matrix4 *model_to_screen,
						br_int_32 bounds[4])
{
	render_bounds[model_number][0] = bounds[BR_BOUNDS_MIN_X]-1;
	render_bounds[model_number][1] = bounds[BR_BOUNDS_MIN_Y]-1;
	render_bounds[model_number][2] = bounds[BR_BOUNDS_MAX_X]+1;
	render_bounds[model_number][3] = bounds[BR_BOUNDS_MAX_Y]+1;
	
	model_number++;
}

