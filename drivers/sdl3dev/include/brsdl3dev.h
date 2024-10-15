#ifndef _BRSDL3DEV_H_
#define _BRSDL3DEV_H_

#ifndef _BRENDER_H_
#error Please include brender.h first
#endif

struct SDL_KeyboardEvent;
struct SDL_Window;

typedef void BR_CALLBACK br_device_pixelmap_sdl_preswap_hook_cbfn(br_pixelmap *pm, void *user);

typedef struct br_device_sdl_ext_procs {
    br_device_pixelmap_sdl_preswap_hook_cbfn *preswap;
    void                                     *user;
} br_device_sdl_ext_procs;

/*
 * Function prototypes
 */
#ifndef _NO_PROTOTYPES

#ifndef _BRSDL3DEV_P_H
#include "brsdl3dev_p.h"
#endif

#endif /* _NO_PROTOTYPES */

#endif /* _BRSDL3DEV_H_ */
