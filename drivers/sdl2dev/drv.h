/*
 * Private device driver structure
 */
#ifndef _DRV_H_
#define _DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL.h>

// #define BR_OBJECT_PRIVATE
#define BR_DEVICE_PRIVATE
#define BR_OUTPUT_FACILITY_PRIVATE
#define BR_DEVICE_PIXELMAP_PRIVATE
#define BR_DEVICE_CLUT_PRIVATE

/*
 * Define to 1 to disable falling back the br_device_pixelmap_mem methods.
 */
#ifndef NO_MEMORY_FALLBACK
#define NO_MEMORY_FALLBACK 0
#endif

/*
 * Maximum number of possible entries on a CLUT.
 * BRender supports no more than 256 (for BR_PMT_INDEX_8).
 */
#define MAX_CLUT_ENTRIES 256

#include "brddi.h"
#include "brsdl2dev.h"
#include "template.h"
#include "device.h"
#include "outfcty.h"
#include "devpixmp.h"
#include "devclut.h"

/*
 * Macros that expand to the first two arguments of a template entry
 * Builtin or device token
 */
#define BRT(t) BRT_##t, 0
#define DEV(t) 0, #t

typedef int(SDLCALL br_sdl_blit_cbfn)(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);

/*
 * Pull in private prototypes
 */
#ifndef _NO_PROTOTYPES

#include "drv_ip.h"

#endif

#ifdef __cplusplus
};
#endif
#endif /* _DRV_H_ */
