#ifndef _BRGLREND_H_
#define _BRGLREND_H_

#ifndef _BRENDER_H_
#error Please include brender.h first
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern br_token BRT_OPENGL_TEXTURE_U32;
extern br_token BRT_OPENGL_OPAQUE_DRAW_COUNT_U32;
extern br_token BRT_OPENGL_TRANSPARENT_DRAW_COUNT_U32;
extern br_token BRT_OPENGL_NUM_EXTENSIONS_I32;
extern br_token BRT_OPENGL_EXTENSIONS_PL;

#ifdef __cplusplus
}
#endif

typedef struct br_device_gl_context_info {
    void     *native;
    br_int_32 major;
    br_int_32 minor;
    br_token  profile;
} br_device_gl_context_info;

typedef br_error BR_CALLBACK br_device_pixelmap_gl_createcontext_cbfn(br_pixelmap *pm, br_device_gl_context_info *info, void *user);

typedef void BR_CALLBACK br_device_pixelmap_gl_deletecontext_cbfn(br_pixelmap *pm, void *ctx, void *user);

typedef br_error BR_CALLBACK br_device_pixelmap_gl_makecurrent_cbfn(br_pixelmap *pm, void *ctx, void *user);

typedef void BR_CALLBACK br_device_pixelmap_gl_swapbuffers_cbfn(br_pixelmap *pm, void *user);

typedef br_error BR_CALLBACK br_device_pixelmap_gl_resize_cbfn(br_pixelmap *pm, br_int_32 w, br_int_32 h, void *user);

typedef void (*BR_CALLBACK br_device_pixelmap_gl_getprocaddress_cbfn(const char *name))(void);

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
