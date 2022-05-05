#ifndef _DEVPIXMP_HPP_
#define _DEVPIXMP_HPP_

#include "gl_core_3_2.h"
#include "pixelmap.h"

typedef struct
{
	float x, y, z;
	float r, g, b;
	float u, v;
} br_device_pixelmap_gl_tri;

/*
* Private state of device pixelmap
*/
typedef struct br_device_pixelmap_gl
{
	/* Dispatch table */
	const struct br_device_pixelmap_dispatch *dispatch;

	/* Standard handle identifier */
	const char *pm_identifier;

	/** Standard pixelmap members (not including identifier) **/

	BR_PIXELMAP_MEMBERS;

	/** End of br_pixelmap fields **/

	struct br_device_gl *device;
	struct br_output_facility_gl *output_facility;

	br_token use_type;

	br_int_32 msaa_samples;

	/*
	* Pointer to renderer currently opened on this pixelmap (N.B. This is only set on the screen
	* pixelmap)
	*/
	struct br_renderer *renderer;

	/*
	* Renderer is currently drawing to on this pixelmap
	*/
	br_boolean renderer_active;


	/* OpenGL crap */

	GLenum glInternalFormat;
	GLenum glFormat;
	GLenum glType;
	GLsizeiptr glElemBytes;

	union
	{
		struct
		{
			struct br_device_pixelmap_gl *depthbuffer;
			GLuint glFbo;
			GLuint glTex;

			struct
			{
				br_device_pixelmap_gl_tri tris[4];
				GLuint defaultVao;
				GLuint textVao;
				GLuint buffers[2];
			} quad;
		} asBack;
		struct
		{
			struct br_device_pixelmap_gl *backbuffer;
			GLuint glDepth;
		} asDepth;
	};

} br_device_pixelmap_gl;

#endif /* _DEVPIXMP_HPP_ */