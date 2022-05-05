#ifndef _DEVICE_HPP_
#define _DEVICE_HPP_

#include "gl_core_3_2.h"
#include "brlists.h"
#include "host.h"
#include "template.hpp"
#include "rendfcty.hpp"

#include "video_ip.hpp"

/*
Look in dev_objs.hpp, for the actual C++ version.

I've tried to offset some of the badness by using unions, but it's still pretty bad.
*/

struct br_object_gl;
struct br_device_gl;
struct br_output_facility_gl;
struct br_renderer_facility_gl;

typedef struct br_object_gl
{
	/* Dispatch table */
	const struct br_object_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Device pointer */
	br_device_gl *device;

} br_object_gl;

/* Private state of device */
typedef struct br_device_gl
{
	union
	{
		br_object_gl object;
		struct
		{
			struct br_device_dispatch *dispatch;
			char *identifier;
			struct br_device_gl *device;
		};
	};

	/* List of objects associated with this device */
	void *object_list;

	/* Anchor for all the device's resource */
	void *res;

	/* Number of output facilities available. */
	br_uint_32 output_facility_count;

	/*
	* Driver-wide template store
	*/
	struct device_templates_gl templates;

	VIDEO video;
	GLFWwindow *glfwWindow;
	int msaaSamples;
} br_device_gl;

typedef struct br_output_facility_gl
{
	union
	{
		br_object_gl object;
		struct
		{
			const struct br_output_facility_dispatch *dispatch;
			const char *identifier;
			struct br_device_gl *device;
		};
	};
	

	/* List of instances associated with type */
	void *object_list;

	/* Width */
	br_int_32 width;
	/* Height */
	br_int_32 height;
	/* Colour depth - 8, 16, or 24 */
	br_int_32 colour_bits;
	/* Pixelmap type, this is one of the BR_PMT_* defines */
	br_int_32 colour_type;
	/* Is there a CLUT? */
	br_boolean indexed;
	/* The monitor this mode is on */
	br_int_32 monitor;

	br_renderer_facility_gl *renderer_facility;
#ifdef __cplusplus
	operator struct br_object*(void) { return reinterpret_cast<struct br_object*>(this); }
#endif

} br_output_facility_gl;

#endif /* _DEVICE_HPP_ */
