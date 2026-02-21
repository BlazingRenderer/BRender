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
#define BR_DEVICE_CLUT_PRIVATE
#define BR_RENDERER_FACILITY_PRIVATE
#define BR_RENDERER_PRIVATE
#define BR_BUFFER_STORED_PRIVATE
#define BR_GEOMETRY_V1_MODEL_PRIVATE
#define BR_GEOMETRY_V1_BUCKETS_PRIVATE
#define BR_GEOMETRY_STORED_PRIVATE
#define BR_RENDERER_STATE_STORED_PRIVATE

/**
 * \brief Maximum amount of swapchain images we can handle.
 */
#define BR_VKREND_MAX_SWAPCHAIN_IMAGES 4

/**
 * \brief Maximum number of possible entries on a CLUT.
 *
 * BRender supports no more than 256 (for BR_PMT_INDEX_8).
 */
#define BR_VKREND_MAX_CLUT_ENTRIES 256

/**
 * \brief Maximum of time to wait for a GPU fence before failing. 60s.
 */
#define BR_VKREND_DEFAULT_WAIT 60000000000L

#include "brddi.h"
#include "brvkrend.h"
#include "formats.h"
#include "pm.h"

typedef struct br_pixelmap_vk_fmt {
    br_uint_8  pm_type;
    VkFormat   format;
    br_uint_8  bytes_per_pixel;
    br_boolean indexed;

    /*
     * Whether the pixelmap is a depth buffer. A depth image is created and viewed with the
     * DEPTH aspect and the depth-stencil-attachment usage, not the colour ones.
     */
    br_boolean         depth;
    VkComponentSwizzle swizzle_r;
    VkComponentSwizzle swizzle_g;
    VkComponentSwizzle swizzle_b;
    VkComponentSwizzle swizzle_a;
} br_pixelmap_vk_fmt;

/**
 * \brief Initial capacity of a frame slot's retire list.
 */
#define BR_VKREND_RETIRE_INITIAL_CAPACITY 4

struct br_device_pixelmap;

/**
 * \brief Kind of GPU resource held by a frame slot's retire list.
 */
typedef enum br_vk_retire_kind {
    BR_VK_RETIRE_IMAGE,
    BR_VK_RETIRE_BUFFER,
} br_vk_retire_kind;

/**
 * \brief A GPU resource retired from the frame slot that last referenced it.
 *
 * The kind selects which handles are valid: an IMAGE entry owns the view, image and
 * allocation, a BUFFER entry owns the buffer and allocation.
 */
typedef struct br_vk_retire_entry {
    br_vk_retire_kind kind;
    VkImage           image;
    VkImageView       view;
    VmaAllocation     allocation;
    VkBuffer          buffer;
} br_vk_retire_entry;

typedef struct br_device_pixelmap_vk_frame {
    br_size_t       index;
    VkFence         fence;
    VkSemaphore     image_acquired;
    VkCommandBuffer command_buffer;

    /**
     * \brief The front pixelmap this slot belongs to, and the resource parent of retire_entries.
     */
    struct br_device_pixelmap *owner;

    /**
     * \brief Resources that may still be referenced by work in flight, destroyed once fence has
     * signalled. Retired from the render thread only; the driver has no locking.
     */
    br_vk_retire_entry *retire_entries;
    br_uint_32          retire_count;
    br_uint_32          retire_capacity;
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
    VkExtent2D               extent;
    VkSwapchainKHR           swapchain;
    uint32_t                 image_count;
    VkImage                  images[BR_VKREND_MAX_SWAPCHAIN_IMAGES];
} DeviceVkSwapchain;

#include "uuid.h"
#include "template.h"
#include "device.h"
#include "devclut.h"
#include "outfcty.h"
#include "devpixmp.h"
#include "rendfcty.h"
#include "state.h"
#include "renderer.h"
#include "sbuffer.h"
#include "gstored.h"
#include "gv1model.h"
#include "gv1buckt.h"

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
