#ifndef _GV1BUCKT_H_
#define _GV1BUCKT_H_

#include "drv.hpp"


/*
* Private state of geometry format
*/
typedef struct br_geometry_v1_buckets_gl
{
	/*
	* Dispatch table
	*/
	struct br_geometry_v1_buckets_dispatch *dispatch;

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

} br_geometry_v1_buckets_gl;


#endif /* _GV1BUCKT_H_ */