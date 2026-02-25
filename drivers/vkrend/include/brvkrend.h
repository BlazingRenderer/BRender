#ifndef BRVKREND_H_
#define BRVKREND_H_

#ifndef _BRENDER_H_
#error Please include brender.h first
#endif

enum {
    BR_DEVICE_VK_INIT_VERSION_1 = 1,
    BR_DEVICE_VK_INIT_VERSION   = BR_DEVICE_VK_INIT_VERSION_1,
};

typedef br_error BR_CALLBACK br_device_pixelmap_vk_create_surface_cbfn(void *instance, const void *allocator, void *surface, void *user);
typedef void BR_CALLBACK     br_device_pixelmap_vk_destroy_surface_cbfn(void *instance, const void *allocator, void *surface, void *user);
typedef br_error BR_CALLBACK br_device_pixelmap_vk_resize_cbfn(br_pixelmap *pm, br_int_32 w, br_int_32 h, void *user);
typedef void BR_CALLBACK     br_device_pixelmap_vk_preswap_cbfn(br_pixelmap *pm, void *user);
typedef void BR_CALLBACK     br_device_pixelmap_vk_free_cbfn(br_pixelmap *pm, void *user);
typedef br_error BR_CALLBACK br_device_pixelmap_vk_handle_window_event_cbfn(br_pixelmap *pm, void *arg, void *user);

typedef void(br_device_vk_void_function)(void);

typedef void(br_device_vk_cbfn)(void);

typedef struct br_device_vk_init {
    uint32_t           version;
    br_device_vk_cbfn *get_instance_proc_address;
    uint32_t           required_layer_count;
    const char *const *required_layer_names;
    uint32_t           required_extension_count;
    const char *const *required_extension_names;

    br_device_pixelmap_vk_create_surface_cbfn      *create_surface;
    br_device_pixelmap_vk_destroy_surface_cbfn     *destroy_surface;
    br_device_pixelmap_vk_resize_cbfn              *resize;
    br_device_pixelmap_vk_preswap_cbfn             *preswap_hook;
    br_device_pixelmap_vk_free_cbfn                *free;
    br_device_pixelmap_vk_handle_window_event_cbfn *handle_window_event;
} br_device_vk_init;

/*
 * Function prototypes
 */
#ifndef _NO_PROTOTYPES

#ifndef _BRVKREND_P_H
#include "brvkrend_p.h"
#endif

#endif /* _NO_PROTOTYPES */

#endif /* BRVKREND_H_ */
