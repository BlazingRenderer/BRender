/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Revolving Texture-Mapped Fork (15-bit mode)
 */

#include <stddef.h>
#include "brender.h"
#include "dosio.h"

int main()
{
	br_pixelmap *screen_buffer, *back_buffer, *depth_buffer;
	br_actor *world, *observer, *fork;
	br_pixelmap *tile_pm;
	int i;
	br_material *mats[10];

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
	 * Load and Register TILE0011 Texture
	 */
	tile_pm = BrPixelmapLoad("refmap.pix");
	if (tile_pm == NULL)
	{
		BR_ERROR("Couldn't load refmap.pix");
	}
	BrMapAdd(tile_pm);

	/*
	 * Load and Apply fork Material
	 */

	BrMaterialAdd(BrFmtScriptMaterialLoad("fork.mat"));
	/*
	 * Load and Position fork Actor
	 */

	fork = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	fork->model = BrModelLoad("fork.dat");
	fork->model->flags |= BR_MODF_UPDATEABLE;
	BrModelAdd(fork->model);
	BrModelApplyMap(fork->model, BR_APPLYMAP_PLANE, NULL);
	fork->t.type = BR_TRANSFORM_MATRIX34;
	/*
	 * Assign fork material
	 */
	fork->material = BrMaterialFind("CHROME GIFMAP");

	/********************** Animation Loop **********************************/

	float dt;
	while (UpdateSample(observer, &dt))
	{
		BrPixelmapFill(back_buffer, 0);
		BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
		BrZbSceneRender(world, observer, back_buffer, depth_buffer);
		BrPixelmapDoubleBuffer(screen_buffer, back_buffer);
		BrMatrix34PostRotateY(&fork->t.t.mat, BR_ANGLE_DEG(2.0));
	}
	/* Close down */

	BrZbEnd();
	DOSGfxEnd();
	BrEnd();
	return 0;
}
