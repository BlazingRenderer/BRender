#ifndef BRENDERSAMPLE_DOSIO_H
#define BRENDERSAMPLE_DOSIO_H

#include "brender.h"
#include <stdbool.h>

struct SDL_KeyboardEvent;

br_pixelmap *BR_PUBLIC_ENTRY DOSGfxBegin(const char *setup_string);

void BR_PUBLIC_ENTRY DOSGfxEnd(void);

void InitializeSampleZBuffer(br_pixelmap **screen_buffer, br_pixelmap **back_buffer, br_pixelmap **depth_buffer);

/**
 * Big difference with InitializeSampleZBuffer, `BrBegin()` is NOT called on this function, because `BrMemAllocate`
 * won't work, and you'll need that to allocate `primitive`
 */
void InitializeSampleZSort(br_pixelmap **screen_buffer, br_pixelmap **back_buffer, void *primitive, br_uint_32 primitiveCount);

bool UpdateSample(br_actor *camera, float *dt);

bool UpdateSampleWithKeyHandler(br_actor *camera, float *dt, void (*keyCallback)(const struct SDL_KeyboardEvent *evt));

br_actor *CreateSampleCamera(br_actor *parent);

#endif // BRENDERSAMPLE_DOSIO_H
