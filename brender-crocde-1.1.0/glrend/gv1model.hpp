#ifndef _GV1MODEL_H_
#define _GV1MODEL_H_

#include "drv.hpp"
#include "sstate.hpp"

typedef struct br_geometry_v1_model_gl
{
	/* Dispatch table */
	const struct br_geometry_v1_model_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Pointer to owning device */
	struct br_device_gl *device;

	/* Renderer type this format is associated with */
	struct br_renderer_facility_gl *renderer_facility;

} br_geometry_v1_model_gl;

template<>
inline constexpr bool is_object_v<br_geometry_v1_model_gl> = true;

typedef struct br_geometry_stored_gl
{
	/* Dispatch table */
	const struct br_geometry_stored_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Pointer to owning device */
	br_device_gl *device;

	void *object_list;

	br_geometry_v1_model_gl *gv1model;

	br_boolean shared;
	struct v11model_f *model;

	GLuint glVao;
	GLuint glVbo;
	GLuint glIbo;
} br_geometry_stored_gl;

template<>
inline constexpr bool is_object_v<br_geometry_stored_gl> = true;

#endif


