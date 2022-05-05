#ifndef _RENDFCTY_HPP_
#define _RENDFCTY_HPP_

#include "drv.hpp"


struct br_output_facility_gl;
typedef struct br_renderer_facility_gl
{
	/* Dispatch table */
	struct br_renderer_facility_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Pointer to owning device */
	br_output_facility_gl *output_facility;

	/* List of objects associated with this device */
	void *object_list;

} br_renderer_facility_gl;


#endif /* _RENDFCTY_HPP_ */