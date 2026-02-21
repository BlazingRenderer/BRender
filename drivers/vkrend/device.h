#ifndef DEVICE_H_
#define DEVICE_H_

typedef struct br_device_vk_phy {
    VkPhysicalDevice             dev;
    uint32_t                     index;
    VkPhysicalDeviceIDProperties id_props;
    VkPhysicalDeviceProperties2  props2;

    uint32_t                 qf_count;
    VkQueueFamilyProperties *qf;
    uint32_t                 qf_gfx;

    int rank;
} br_device_vk_phy;

typedef struct br_device_vk_select {
    /* User-provided device ID. */
    br_device_vk_uuid device_uuid;
} br_device_vk_select;

#ifdef BR_DEVICE_PRIVATE

/*
 * Private state of device
 */
typedef struct br_device {
    /*
     * Dispatch table
     */
    const struct br_device_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * List of objects associated with this device
     */
    void *object_list;

    /*
     * Anchor for all device's resources
     */
    void *res;

    /*
     * Driver-wide template store
     */
    struct device_templates templates;

    VkInstance                vulkan;
    GladVulkanContext         context;
    PFN_vkGetInstanceProcAddr getInstanceProcAddr;
    br_device_vk_init         init_info;
    br_device_vk_phy         *physical_device;
    VkDevice                  vk_device;
    VmaAllocator              vma_allocator;
    VkQueue                   vk_queue;
    VkCommandPool             vk_command_pool;
} br_device;

#endif /* BR_DEVICE_PRIVATE */
#endif /* DEVICE_H_ */
