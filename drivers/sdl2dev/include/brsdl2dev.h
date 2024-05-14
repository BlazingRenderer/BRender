#ifndef _BRSDL2DEV_H_
#define _BRSDL2DEV_H_

#ifndef _BRENDER_H_
#error Please include brender.h first
#endif

/*
 * Defined to 1 if built as a shared library.
 */
#if !defined(BR_SDL2_BUILD_SHARED)
#define BR_SDL2_BUILD_SHARED 0
#endif

/*
 * Defined to 1 if built as a static library.
 */
#if !defined(BR_SDL2_BUILD_STATIC)
#define BR_SDL2_BUILD_STATIC 0
#endif


#if(!BR_SDL2_BUILD_SHARED && !BR_SDL2_BUILD_STATIC) || (BR_SDL2_BUILD_SHARED && BR_SDL2_BUILD_STATIC)
#error Invalid linkage configuration
#endif

#if !defined(BR_SDL2_EXPORTS)
#define BR_SDL2_EXPORTS 0
#endif

#if BR_SDL2_BUILD_SHARED
#define BR_SDL2_API_EXPORT BR_DLL_EXPORT
#define BR_SDL2_API_IMPORT BR_DLL_IMPORT
#else
#define BR_SDL2_API_EXPORT
#define BR_SDL2_API_IMPORT
#endif

#if BR_SDL2_EXPORTS
#define BR_SDL2_API BR_SDL2_API_EXPORT
#else
#define BR_SDL2_API BR_SDL2_API_IMPORT
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

#ifndef _BRSDL2DEV_P_H
#include "brsdl2dev_p.h"
#endif

#endif /* _NO_PROTOTYPES */

#endif /* _BRSDL2DEV_H_ */
