#ifndef _VIDEO_H
#define _VIDEO_H

#include <stdint.h>
#include "gl_core_3_2.h"

#define VIDEO_API

#ifdef __cplusplus
extern "C" {
#endif

	struct _VIDEO;
	typedef struct _VIDEO VIDEO, *HVIDEO;

	struct br_pixelmap;
	typedef struct br_pixelmap br_pixelmap;

	VIDEO_API HVIDEO VIDEO_Open(HVIDEO hVideo);
	VIDEO_API void VIDEO_Close(HVIDEO hVideo);

	VIDEO_API GLuint VIDEO_BrPixelmapToGLTexture(br_pixelmap *pm);
#ifdef __cplusplus
}
#endif

#endif /* _VIDEO_H */