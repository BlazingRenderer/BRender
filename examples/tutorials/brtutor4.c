/*
 * Copyright (c) 1996 Argonaut Technologies Limited. All rights reserved.
 * Program to display a Planet-Satellite animation
 */

#include <stddef.h>
#include "brender.h"
#include "dosio.h"

int main()
{
	br_pixelmap *screen_buffer, *back_buffer, *depth_buffer, *palette;
	br_actor *world, *observer, *cube, *planet, *sat, *wings1, *wings2;
	int i;
	br_camera *camera_data;

	/************* Initialise BRender and Graphics Hardware *********************/
	InitializeSampleZBuffer(&screen_buffer, &back_buffer, &depth_buffer);

	/*************** Build the World Database **********************************/
	world = BrActorAllocate(BR_ACTOR_NONE, NULL);
	BrLightEnable(BrActorAdd(world, BrActorAllocate(BR_ACTOR_LIGHT, NULL)));
	/*
	 * Load and Position Camera
	 */
	observer = CreateSampleCamera(world);
	observer->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34Translate(&observer->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0),
	                    BR_SCALAR(8.0));
	camera_data = (br_camera *) observer->type_data;
	camera_data->yon_z = BR_SCALAR(350);
	camera_data->hither_z = BR_SCALAR(0.5);
	/*
	 * Load and Position Planet Actor
	 */
	planet = BrActorAdd(world, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	planet->model = BrModelLoad("sph32.dat");
	BrModelAdd(planet->model);
	planet->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34Translate(&planet->t.t.mat, BR_SCALAR(14.0), BR_SCALAR(14.0),
	                    BR_SCALAR(-40.0));
	/*
	 * Load and Position Satellite
	 */
	sat = BrActorAdd(planet, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	sat->model = BrModelLoad("sph16.dat");
	BrModelAdd(sat->model);
	sat->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34Scale(&sat->t.t.mat, BR_SCALAR(0.5), BR_SCALAR(0.5),
	                BR_SCALAR(0.5));
	BrMatrix34PostTranslate(&sat->t.t.mat, BR_SCALAR(2.0), BR_SCALAR(0.0),
	                        BR_SCALAR(0.0));
	/* Add `wings' to Satellite
	 */
	wings1 = BrActorAdd(sat, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	wings1->model = BrModelLoad("cylinder.dat");
	BrModelAdd(wings1->model);
	wings1->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34Scale(&wings1->t.t.mat, BR_SCALAR(0.25), BR_SCALAR(0.25),
	                BR_SCALAR(2.0));
	/* Add more `wings' to Satellite
	 */
	wings2 = BrActorAdd(sat, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	wings2->model = BrModelLoad("cylinder.dat");
	BrModelAdd(wings2->model);
	wings2->t.type = BR_TRANSFORM_MATRIX34;
	BrMatrix34Scale(&wings2->t.t.mat, BR_SCALAR(0.25), BR_SCALAR(0.25),
	                BR_SCALAR(2.0));
	BrMatrix34PostRotateY(&wings2->t.t.mat, BR_ANGLE_DEG(90.0));

	/********************** Animation Loop **********************************/
	float dt;
	while (UpdateSample(observer, &dt))
	{
		BrPixelmapFill(back_buffer, 0);
		BrPixelmapFill(depth_buffer, 0xFFFFFFFF);
		BrZbSceneRender(world, observer, back_buffer, depth_buffer);
		BrPixelmapDoubleBuffer(screen_buffer, back_buffer);
		BrMatrix34PostTranslate(&planet->t.t.mat, BR_SCALAR(-0.033),
		                        BR_SCALAR(-0.032), BR_SCALAR(0.1));
		BrMatrix34PreRotateY(&planet->t.t.mat, BR_ANGLE_DEG(1.0));
		BrMatrix34PreRotateX(&sat->t.t.mat, BR_ANGLE_DEG(15.0));
		BrMatrix34PreRotateY(&sat->t.t.mat, BR_ANGLE_DEG(10.0));
		BrMatrix34PostRotateZ(&sat->t.t.mat, BR_ANGLE_DEG(1.0));
		BrMatrix34PostRotateY(&sat->t.t.mat, BR_ANGLE_DEG(3.0));
	}
	/* Close down */
	BrPixelmapFree(depth_buffer);
	BrPixelmapFree(back_buffer);
	BrZbEnd();
	DOSGfxEnd();
	BrEnd();
	return 0;
}