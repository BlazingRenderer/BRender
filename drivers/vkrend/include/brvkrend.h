#ifndef BRVKREND_H_
#define BRVKREND_H_

#ifndef _BRENDER_H_
#error Please include brender.h first
#endif

enum {
    BR_DEVICE_VK_INIT_VERSION_1 = 1,
    BR_DEVICE_VK_INIT_VERSION   = BR_DEVICE_VK_INIT_VERSION_1,
};

typedef void(br_device_vk_void_function)(void);

typedef void(br_device_vk_cbfn)(void);

typedef struct br_device_vk_init {
    uint32_t           version;
    br_device_vk_cbfn *get_instance_proc_address;
    uint32_t           required_layer_count;
    const char *const *required_layer_names;
    uint32_t           required_extension_count;
    const char *const *required_extension_names;
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
