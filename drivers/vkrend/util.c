#include <inttypes.h>
#include "drv.h"

br_uint_8 DeviceVkTypeOrBits(br_uint_8 pixel_type, br_int_32 pixel_bits)
{
    if(pixel_type != BR_PMT_MAX)
        return pixel_type;

    switch(pixel_bits) {
        case 16:
            return BR_PMT_RGB_565;
        case 24:
            return BR_PMT_RGB_888;
        case 32:
            return BR_PMT_RGBX_888;
        default:
            break;
    }

    return BR_PMT_MAX;
}

VkResult DeviceVkGetSwapchainFormat(const GladVulkanContext *vk, VkPhysicalDevice phy, VkSurfaceKHR surf, VkFormat in_fmt, VkSurfaceFormatKHR *out)
{
    uint32_t            format_count = 0;
    VkSurfaceFormatKHR *formats      = NULL;
    VkResult            res;

    if((res = vk->GetPhysicalDeviceSurfaceFormatsKHR(phy, surf, &format_count, NULL)) != VK_SUCCESS) {
        return res;
    }

    if((formats = BrScratchAllocate(sizeof(VkSurfaceFormatKHR) * format_count)) == NULL) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    if((res = vk->GetPhysicalDeviceSurfaceFormatsKHR(phy, surf, &format_count, formats)) != VK_SUCCESS) {
        BrScratchFree(formats);
        return res;
    }

    /*
     * If a format's been explicitly requested, match it exactly.
     */
    if(in_fmt != VK_FORMAT_UNDEFINED) {
        for(uint32_t i = 0; i < format_count; ++i) {
            if(formats[i].format == in_fmt) {
                *out = formats[i];
                BrScratchFree(formats);
                return VK_SUCCESS;
            }
        }
    }

    BrLogWarn("VKREND", "auto-format selection not implemented yet");
    BrScratchFree(formats);
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

VkResult DeviceVkCreateSwapchain(const GladVulkanContext *vk, VkPhysicalDevice phy, VkDevice device, VkQueue queue,
                                 const DeviceVkCreateSwapchainInfo *info, DeviceVkSwapchain *out)
{
    VkSurfaceFormatKHR       format = {.format = VK_FORMAT_UNDEFINED, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    VkSurfaceCapabilitiesKHR caps;
    VkSwapchainKHR           swap_chain;
    uint32_t                 image_count;
    VkImage                 *images;
    VkResult                 res;

    BrLogTrace("VKREND", "(Re)creating swapchain(w=%d,h=%d)", info->width, info->height);

    if((res = DeviceVkGetSwapchainFormat(vk, phy, info->surface, info->wanted_format, &format)) != VK_SUCCESS) {
        BrLogError("VKREND", "Unable to retrieve swapchain format: VkResult = %d", res);
        return res;
    }

    if((res = vk->GetPhysicalDeviceSurfaceCapabilitiesKHR(phy, info->surface, &caps)) != VK_SUCCESS) {
        BrLogError("VKREND", "Unable to query physical device surface capabilities: VkResult = %d", res);
        return res;
    }

    /*
     * Wait for the GPU to finish whatever it's doing.
     */
    if((res = vk->QueueWaitIdle(queue)) != VK_SUCCESS) {
        BR_FATAL1("vkQueueWaitIdle() returned %" PRIu32, res);
    }

    const VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface               = info->surface,
        .minImageCount         = caps.minImageCount, /* Always request the min. */
        .imageFormat           = format.format,
        .imageColorSpace       = format.colorSpace,
        .imageExtent           = {.width = info->width, .height = info->height},
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = NULL,
        .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode           = VK_PRESENT_MODE_FIFO_KHR, /* TODO: This may be a good config option. */
        .clipped               = VK_TRUE,
        .oldSwapchain          = info->old_swapchain,
    };
    if((res = vk->CreateSwapchainKHR(device, &swapchain_create_info, NULL, &swap_chain)) != VK_SUCCESS) {
        BrLogError("VKREND", "Unable to create swapchain: VkResult = %d", res);
        return res;
    }

    if((res = vk->GetSwapchainImagesKHR(device, swap_chain, &image_count, NULL)) != VK_SUCCESS) {
        BR_FATAL1("vkGetSwapchainImagesKHR() returned %" PRIu32, res);
    }

    images = BrScratchAllocate(sizeof(VkImage) * image_count);

    res = vk->GetSwapchainImagesKHR(device, swap_chain, &image_count, images);
    if(res != VK_SUCCESS && res != VK_INCOMPLETE) {
        BR_FATAL1("vkGetSwapchainImagesKHR() returned %" PRIu32, res);
    }

    if(image_count > BR_VKREND_MAX_SWAPCHAIN_IMAGES) {
        image_count = BR_VKREND_MAX_SWAPCHAIN_IMAGES;
    }

    *out = (DeviceVkSwapchain){
        .surface_format       = format,
        .surface_capabilities = caps,
        .swapchain            = swap_chain,
        .image_count          = image_count,
    };

    for(uint32_t i = 0; i < image_count; ++i) {
        out->images[i] = images[i];
    }

    BrScratchFree(images);
    return VK_SUCCESS;
}

void DeviceVkDestroySwapchain(const GladVulkanContext *vk, VkDevice device, DeviceVkSwapchain *swapchain)
{
    for(br_uint_32 i = 0; i < BR_ASIZE(swapchain->images); ++i) {
        swapchain->images[i] = VK_NULL_HANDLE;
    }

    if(swapchain->swapchain != VK_NULL_HANDLE) {
        vk->DestroySwapchainKHR(device, swapchain->swapchain, NULL);
        swapchain->swapchain = VK_NULL_HANDLE;
    }
}
