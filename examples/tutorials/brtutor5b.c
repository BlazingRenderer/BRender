/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Revolving Illuminated Blue Cube (8-bit mode)
 */

#include <stddef.h>
#include "brender.h"
#include "dosio.h"

int main()
{
	br_pixelmap *screen_buffer, *back_buffer, *depth_buffer, *palette;
	br_actor *world, *observer, *cube;
	br_material *cube_material;
	int i;

	/************* Initialise BRender and Graphics Hardware *********************/
	InitializeSampleZBuffer(&screen_buffer, &back_buffer, &depth_buffer);

	/*************** Build the World Database **********************************/
	/* 
	 * Load Root Actor. Load and Enable Deault Light Source
	 */
	world = BrActorAllocate(BR_ACTOR_NONE, NULL);
	BrLightEnable(BrActorAdd(world, BrActorAllocate(BR_ACTOR_LIGHT, NULL)));
	/* 
	 *Load and Position Camera
	 */
	observer = CreateSampleCamera(world);
	observer->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0),
	                    BR_SCALAR(5.0));
	/*
	 * Load and Position Cube Model
	 */
	cube = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	cube->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34RotateY(&cube->t.t.mat, BR_ANGLE_DEG(30));
	/*
	 * Load and Apply Cube's Material
	 */
	cube_material = BrFmtScriptMaterialLoad("cube8.mat");
	BrMaterialAdd(cube_material);
	cube->material = BrMaterialFind("BLUE MATERIAL");

	/********************** Animation Loop **********************************/
	float dt;
	while (UpdateSample(observer, &dt))
	{
		BrPixelmapFill(back_buffer, 0);
		BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
		BrZbSceneRender(world, observer, back_buffer, depth_buffer);
		BrPixelmapDoubleBuffer(screen_buffer, back_buffer);
		BrMatrix34PostRotateX(&cube->t.t.mat, BR_ANGLE_DEG(2.0));
	}
	/* Close down */

	BrZbEnd();
	DOSGfxEnd();
	BrEnd();
	return 0;
}