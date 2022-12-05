/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Texture Mapped Duck (15-bit)
 */

#include <stddef.h>
#include "brender.h"
#include "dosio.h"

int main()
{
	br_pixelmap *screen_buffer, *back_buffer, *depth_buffer;
	br_actor *world, *observer, *duck;
	br_pixelmap *gold_pm;

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
	 * Load and Register `gold' Texture
	 */
	gold_pm = BrPixelmapLoad("gold15.pix");
	if (gold_pm == NULL)
	{
		BR_ERROR("Couldn't load gold15.pix");
	}
	BrMapAdd(gold_pm);

	/*
	 * Load and Apply 'gold' Material
	 */
	BrMaterialAdd(BrFmtScriptMaterialLoad("gold15.mat"));

	/*
	 * Load and Position Duck Actor
	 */

	duck = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	duck->model = BrModelLoad("duck.dat");
	duck->model->flags |= BR_MODF_UPDATEABLE;
	BrModelAdd(duck->model);
	BrModelApplyMap(duck->model, BR_APPLYMAP_PLANE, NULL);
	duck->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34RotateX(&duck->t.t.mat, BR_ANGLE_DEG(30));
	duck->material = BrMaterialFind("gold15");

	/********************** Animation Loop **********************************/

	float dt;
	while (UpdateSample(observer, &dt))
	{
		BrPixelmapFill(back_buffer, 0);
		BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
		BrZbSceneRender(world, observer, back_buffer, depth_buffer);
		BrPixelmapDoubleBuffer(screen_buffer, back_buffer);
		BrMatrix34PostRotateY(&duck->t.t.mat, BR_ANGLE_DEG(2.0));
	}
	/* Close down */

	BrPixelmapFree(depth_buffer);
	BrPixelmapFree(back_buffer);
	BrZbEnd();
	DOSGfxEnd();
	BrEnd();
	return 0;
}