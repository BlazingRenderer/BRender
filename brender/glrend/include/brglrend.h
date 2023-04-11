#ifndef _BRGLREND_H_
#define _BRGLREND_H_

#ifndef _BRENDER_H_
#error Please include brender.h first
#endif

typedef void *BR_CALLBACK br_device_pixelmap_gl_createcontext_cbfn(br_pixelmap *pm, void *user);

typedef void BR_CALLBACK br_device_pixelmap_gl_deletecontext_cbfn(br_pixelmap *pm, void *ctx, void *user);

typedef br_error BR_CALLBACK br_device_pixelmap_gl_makecurrent_cbfn(br_pixelmap *pm, void *ctx, void *user);

typedef void BR_CALLBACK br_device_pixelmap_gl_swapbuffers_cbfn(br_pixelmap *pm, void *user);

typedef void *BR_CALLBACK br_device_pixelmap_gl_getprocaddress_cbfn(const char *name);

typedef void BR_CALLBACK br_device_pixelmap_gl_preswap_hook_cbfn(br_pixelmap *pm, unsigned int fbo, void *user);

typedef struct br_device_gl_procs {

    br_device_pixelmap_gl_createcontext_cbfn  *create_context;
    br_device_pixelmap_gl_deletecontext_cbfn  *delete_context;
    br_device_pixelmap_gl_makecurrent_cbfn    *make_current;
    br_device_pixelmap_gl_getprocaddress_cbfn *get_proc_address;
    br_device_pixelmap_gl_swapbuffers_cbfn    *swap_buffers;
    br_device_pixelmap_gl_preswap_hook_cbfn   *preswap_hook;
    void                                      *user;

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
