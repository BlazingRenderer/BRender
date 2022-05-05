#ifndef _RENDFCTY_HPP_
#define _RENDFCTY_HPP_

#include "drv.hpp"

typedef struct br_renderer_facility_gl
{
	/* Dispatch table */
	const struct br_renderer_facility_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Pointer to owning device */
	struct br_device_gl *device;

	/* List of objects associated with this device */
	void *object_list;

	struct br_output_facility_gl *output_facility;

} br_renderer_facility_gl;


#endif /* _RENDFCTY_HPP_ */