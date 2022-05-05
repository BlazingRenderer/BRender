#ifndef _GPRIM_HPP_
#define _GPRIM_HPP_

#include "drv.hpp"

/*
* Private state of geometry format
*/
typedef struct br_geometry_primitives_gl
{
	/*
	* Dispatch table
	*/
	struct br_geometry_primitives_dispatch *dispatch;

	/*
	* Standard object identifier
	*/
	const char *identifier;

	/*
	* Pointer to owning device
	*/
	struct br_device_gl *device;

	/*
	* Renderer type this format is associated with
	*/
	struct br_renderer_facility_gl *renderer_facility;

} br_geometry_primitives_gl;

#endif /* _GPRIM_HPP_ */



