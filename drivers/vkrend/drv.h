/*
 * Private device driver structure.
 * Notes:
 *  - BR_FATAL*() is used liberally where (1) we realistically can't
 *    handle an error, and (2) a failure shouldn't happen under realistic conditions.
 */
#ifndef DRV_H_
#define DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "glad/vulkan.h"
#include "vk_mem_alloc.h"

#define BR_DEVICE_PRIVATE
#define BR_OUTPUT_FACILITY_PRIVATE
#define BR_DEVICE_PIXELMAP_PRIVATE
#define BR_RENDERER_FACILITY_PRIVATE

/**
 * \brief Maximum amount of swapchain images we can handle.
 */
#define BR_VKREND_MAX_SWAPCHAIN_IMAGES 4

/**
 * \brief Maximum of time to wait for a GPU fence before failing. 60s.
 */
#define BR_VKREND_DEFAULT_WAIT 60000000000L

#include "brddi.h"
#include "brvkrend.h"
#include "formats.h"
#include "pm.h"

typedef struct br_pixelmap_vk_fmt {
    br_uint_8          pm_type;
    VkFormat           format;
    br_boolean         blended;
    br_boolean         indexed;
    VkComponentSwizzle swizzle_r;
    VkComponentSwizzle swizzle_g;
    VkComponentSwizzle swizzle_b;
    VkComponentSwizzle swizzle_a;
} br_pixelmap_vk_fmt;

typedef struct br_device_pixelmap_vk_frame {
    br_size_t       index;
    VkFence         fence;
    VkSemaphore     image_acquired;
    VkSemaphore     render_finished;
    VkCommandBuffer command_buffer;
} br_device_pixelmap_vk_frame;

typedef struct BrDeviceVkCreateSwapchainInfo {
    VkSurfaceKHR   surface;
    br_int_32      width;
    br_int_32      height;
    VkFormat       wanted_format;
    VkSwapchainKHR old_swapchain;
} DeviceVkCreateSwapchainInfo;

typedef struct BrDeviceVkSwapchain {
    VkSurfaceFormatKHR       surface_format;
    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkSwapchainKHR           swapchain;
    uint32_t                 image_count;
    VkImage                  images[BR_VKREND_MAX_SWAPCHAIN_IMAGES];
} DeviceVkSwapchain;

#include "uuid.h"
#include "template.h"
#include "device.h"
#include "outfcty.h"
#include "devpixmp.h"
#include "rendfcty.h"

/*
 * Macros that expand to the first two arguments of a template entry
 * Builtin or device token
 */
#define BRT(t) BRT_##t, 0
#define DEV(t) 0, #t

/*
 * Pull in private prototypes
 */
#ifndef _NO_PROTOTYPES

#include "drv_ip.h"

#endif

#ifdef __cplusplus
};
#endif
#endif /* DRV_H_ */
