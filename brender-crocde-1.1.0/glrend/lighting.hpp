#ifndef _LIGHTING_H_
#define _LIGHTING_H_

#include "drv.hpp"

/* Private state of geometry format */
typedef struct br_geometry_lighting_gl
{
	/* Dispatch table */
	const struct br_geometry_lighting_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Pointer to owning device */
	struct br_device_gl *device;

	/* Renderer type this format is associated with */
	struct br_renderer_facility_gl *renderer_facility;

} br_geometry_lighting_gl;

template<>
inline constexpr bool is_object_v<br_geometry_lighting_gl> = true;

#endif



