#ifndef _DEVICE_HPP_
#define _DEVICE_HPP_

#include "gl_core_3_2.h"
#include "brlists.h"
#include "host.h"
#include "template.hpp"
#include "rendfcty.hpp"

#include "video_ip.hpp"

struct br_object_gl;
struct br_device_gl;
struct br_output_facility_gl;
struct br_renderer_facility_gl;

typedef struct br_object_gl
{
	/* Dispatch table */
	struct br_object_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Device pointer */
	struct br_device_gl *device;

} br_object_gl;

/* Private state of device */
typedef struct br_device_gl
{
	/* Dispatch table */
	struct br_device_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Device pointer */
	struct br_device_gl *device;

	/* List of objects associated with this device */
	void *object_list;

	/* Number of output facilities available. */
	br_uint_32 output_facility_count;

	/* Driver-wide template store */
	struct device_templates_gl templates;

	VIDEO video;
	GLFWwindow *glfwWindow;
	char *vertShader;
	char *fragShader;
} br_device_gl;

typedef struct br_output_facility_gl
{
	const struct br_output_facility_dispatch *dispatch;

	const char *identifier;

	struct br_device_gl *device;

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

	struct br_renderer_facility_gl *renderer_facility;

} br_output_facility_gl;

#endif /* _DEVICE_HPP_ */
