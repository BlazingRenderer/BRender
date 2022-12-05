/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to Display a Revolving Illuminated Cube.
 */

#include <stddef.h>
#include "brender.h"
#include "dosio.h"

int main()
{
	/*
	 * Need screen and back buffers for double-buffering, a Z-Buffer to store
	 * depth information, and a storage buffer for the currently loaded palette
	 */
	br_pixelmap *screen_buffer, *back_buffer, *depth_buffer, *palette;
	/*
	 * The actors in the world: Need a root actor, a camera actor, a light actor,
	 * and a model actor
	 */
	br_actor *world, *observer, *light, *cube;
	int i;                        /*counter*/

	/************* Initialise BRender and Graphics Hardware *********************/
	InitializeSampleZBuffer(&screen_buffer, &back_buffer, &depth_buffer);

	/*************** Build the World Database **********************************/
	/*
	 * Start with None actor at root of actor tree and call it 'world'
	 */
	world = BrActorAllocate(BR_ACTOR_NONE, NULL);
	/*
	 * Add a camera actor as a child of None actor 'world'
	 */
	observer = CreateSampleCamera(world);
	/*
	 * Add and enable the default light source
	 */
	light = BrActorAdd(world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
	BrLightEnable(light);
	/*
	 * Move camera 5 units along +z axis so model becomes visible
	 */
	observer->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0),
	                    BR_SCALAR(5.0));
	/*
	 * Add a model actor, the default cube
	 */
	cube = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	/*
	 * Rotate cube to enhance visibility
	 */
	cube->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34RotateY(&cube->t.t.mat, BR_ANGLE_DEG(30));

	/********************** Animation Loop **********************************/
	/*
	 * Rotate cube around x-axis
	 */
	float dt;
	while (UpdateSample(observer, &dt))
	{
		/*
		 * Initialise depth buffer and set background colour to black
		 */
		BrPixelmapFill(back_buffer, 0);
		BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
		/*
		 * Render scene
		 */
		BrZbSceneRender(world, observer, back_buffer, depth_buffer);
		BrPixelmapDoubleBuffer(screen_buffer, back_buffer);
		/*
		 * Rotate cube
		 */
		BrMatrix34PostRotateX(&cube->t.t.mat, BR_ANGLE_DEG(2.0));
	}
	/* Close down */

	BrPixelmapFree(depth_buffer);
	BrPixelmapFree(back_buffer);
	BrZbEnd();
	DOSGfxEnd();
	BrEnd();
	return 0;
}