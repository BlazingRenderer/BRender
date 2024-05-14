#ifndef _BRGLREND_H_
#define _BRGLREND_H_

#ifndef _BRENDER_H_
#error Please include brender.h first
#endif



/*
 * Defined to 1 if built as a shared library.
 */
#if !defined(BR_GLREND_BUILD_SHARED)
#define BR_GLREND_BUILD_SHARED 0
#endif

/*
 * Defined to 1 if built as a static library.
 */
#if !defined(BR_GLREND_BUILD_STATIC)
#define BR_GLREND_BUILD_STATIC 0
#endif


#if(!BR_GLREND_BUILD_SHARED && !BR_GLREND_BUILD_STATIC) || (BR_GLREND_BUILD_SHARED && BR_GLREND_BUILD_STATIC)
#error Invalid linkage configuration
#endif

#if !defined(BR_GLREND_EXPORTS)
#define BR_GLREND_EXPORTS 0
#endif

#if BR_GLREND_BUILD_SHARED
#define BR_GLREND_API_EXPORT BR_DLL_EXPORT
#define BR_GLREND_API_IMPORT BR_DLL_IMPORT
#else
#define BR_GLREND_API_EXPORT
#define BR_GLREND_API_IMPORT
#endif

#if BR_GLREND_EXPORTS
#define BR_GLREND_API BR_GLREND_API_EXPORT
#else
#define BR_GLREND_API BR_GLREND_API_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern BR_GLREND_API br_token BRT_OPENGL_TEXTURE_U32;

#ifdef __cplusplus
}
#endif

typedef void *BR_CALLBACK br_device_pixelmap_gl_createcontext_cbfn(br_pixelmap *pm, void *user);

typedef void BR_CALLBACK br_device_pixelmap_gl_deletecontext_cbfn(br_pixelmap *pm, void *ctx, void *user);

typedef br_error BR_CALLBACK br_device_pixelmap_gl_makecurrent_cbfn(br_pixelmap *pm, void *ctx, void *user);

typedef void BR_CALLBACK br_device_pixelmap_gl_swapbuffers_cbfn(br_pixelmap *pm, void *user);

typedef br_error BR_CALLBACK br_device_pixelmap_gl_resize_cbfn(br_pixelmap *pm, br_int_32 w, br_int_32 h, void *user);

typedef void *BR_CALLBACK br_device_pixelmap_gl_getprocaddress_cbfn(const char *name);

typedef void BR_CALLBACK br_device_pixelmap_gl_preswap_hook_cbfn(br_pixelmap *pm, unsigned int fbo, void *user);

typedef void BR_CALLBACK br_device_pixelmap_gl_free_cbfn(br_pixelmap *pm, void *user);

typedef br_error BR_CALLBACK br_device_pixelmap_gl_handle_window_event_cbfn(br_pixelmap *pm, void *arg, void *user);

typedef struct br_device_gl_procs {

    br_device_pixelmap_gl_createcontext_cbfn       *create_context;
    br_device_pixelmap_gl_deletecontext_cbfn       *delete_context;
    br_device_pixelmap_gl_makecurrent_cbfn         *make_current;
    br_device_pixelmap_gl_getprocaddress_cbfn      *get_proc_address;
    br_device_pixelmap_gl_resize_cbfn              *resize;
    br_device_pixelmap_gl_swapbuffers_cbfn         *swap_buffers;
    br_device_pixelmap_gl_preswap_hook_cbfn        *preswap_hook;
    br_device_pixelmap_gl_free_cbfn                *free;
    br_device_pixelmap_gl_handle_window_event_cbfn *handle_window_event;
    void                                           *user;

} br_device_gl_ext_procs;

/*
 * Function prototypes
 */
#ifndef _NO_PROTOTYPES

#ifndef _BRGLREND_P_H
#include "brglrend_p.h"
#endif

#endif /* _NO_PROTOTYPES */

#endif /* _BRGLREND_H_ */
