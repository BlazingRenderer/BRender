/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Texture Mapped Sphere (8-bit colour)
 */

#include <stddef.h>
#include "brender.h"
#include "dosio.h"

int main()
{
	br_pixelmap *screen_buffer, *back_buffer, *depth_buffer, *palette, *shade;
	br_actor *world, *observer, *planet;
	br_material *planet_material;
	int i;

	/************* Initialise BRender and Graphics Hardware *********************/
	InitializeSampleZBuffer(&screen_buffer, &back_buffer, &depth_buffer);

	/*
	 * Load Shade Table
	 */

	shade = BrPixelmapLoad("shade.tab");
	if (shade == NULL)
	{
		BR_ERROR("Couldn't load shade.tab");
	}
	BrTableAdd(shade);

/*************** Build the World Database **********************************/

	world = BrActorAllocate(BR_ACTOR_NONE, NULL);
	BrLightEnable(BrActorAdd(world, BrActorAllocate(BR_ACTOR_LIGHT, NULL)));
	observer = CreateSampleCamera(world);
	observer->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0),
	                    BR_SCALAR(5.0));
	/*
	 * Load and Position Planet Actor
	 */

	planet = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	planet->model = BrModelLoad("sph32.dat");
	BrModelAdd(planet->model);
	planet->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34RotateX(&planet->t.t.mat, BR_ANGLE_DEG(90));

	/*
	 * Load and Register `earth' Texture
	 */

	BrMapAdd(BrPixelmapLoad("earth8.pix"));

	/*
	 * Load and Apply 'earth' Material
	 */

	planet_material = BrFmtScriptMaterialLoad("earth8.mat");
	BrMaterialAdd(planet_material);
	planet->material = BrMaterialFind("earth_.map");

	/********************** Animation Loop **********************************/

	float dt;
	while (UpdateSample(observer, &dt))
	{
		BrPixelmapFill(back_buffer, 0);
		BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
		BrZbSceneRender(world, observer, back_buffer, depth_buffer);
		BrPixelmapDoubleBuffer(screen_buffer, back_buffer);
		BrMatrix34PostRotateY(&planet->t.t.mat, BR_ANGLE_DEG(2.0));
	}
	/* Close down */

	BrZbEnd();
	DOSGfxEnd();
	BrEnd();
	return 0;
}
