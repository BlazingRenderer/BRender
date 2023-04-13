/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Revolving Yellow Duck (8-bit)
 */

#include <stddef.h>
#include "brender.h"
#include "dosio.h"

int main()
{
	br_pixelmap *screen_buffer, *back_buffer, *depth_buffer, *palette;
	br_actor *world, *observer, *duck;
	br_material *mats[10]; /*for storing pointers to material descriptions*/
	int i;

	/************* Initialise BRender and Graphics Hardware *********************/
	InitializeSampleZBuffer(&screen_buffer, &back_buffer, &depth_buffer);

	/*************** Build the World Database **********************************/

	world = BrActorAllocate(BR_ACTOR_NONE, NULL);
	BrLightEnable(BrActorAdd(world, BrActorAllocate(BR_ACTOR_LIGHT, NULL)));
	observer = CreateSampleCamera(world);
	observer->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0),
	                    BR_SCALAR(5.0));
	/*
	 * Load and Apply Duck Materials
	 */

	i = BrFmtScriptMaterialLoadMany("duck8.mat", mats, BR_ASIZE(mats));
	BrMaterialAddMany(mats, i);

	/*
	 * Load and Position Duck Actor
	 */

	duck = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	duck->model = BrModelLoad("duck.dat");
	BrModelAdd(duck->model);
	duck->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34RotateY(&duck->t.t.mat, BR_ANGLE_DEG(30));

	/********************** Animation Loop **********************************/

	float dt;
	while (UpdateSample(observer, &dt))
	{
		BrPixelmapFill(back_buffer, 0);
		BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
		BrZbSceneRender(world, observer, back_buffer, depth_buffer);
		BrPixelmapDoubleBuffer(screen_buffer, back_buffer);

		BrMatrix34PostRotateX(&duck->t.t.mat, BR_ANGLE_DEG(2.0));
	}
	/* Close down */

	BrZbEnd();
	DOSGfxEnd();
	BrEnd();
	return 0;
}
