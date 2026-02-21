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
    VkSurfaceFormatKHR  chosen;
    br_boolean          matched = BR_FALSE;
    VkResult            res;

    if((res = vk->GetPhysicalDeviceSurfaceFormatsKHR(phy, surf, &format_count, NULL)) != VK_SUCCESS) {
        return res;
    }

    if(format_count == 0) {
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    if((formats = BrScratchAllocate(sizeof(VkSurfaceFormatKHR) * format_count)) == NULL) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    if((res = vk->GetPhysicalDeviceSurfaceFormatsKHR(phy, surf, &format_count, formats)) != VK_SUCCESS) {
        BrScratchFree(formats);
        return res;
    }

    chosen = formats[0];

    /*
     * If a format's been explicitly requested, match it exactly. Where the
     * surface offers the same format under more than one colour space, prefer
     * an sRGB one.
     */
    if(in_fmt != VK_FORMAT_UNDEFINED) {
        for(uint32_t i = 0; i < format_count; ++i) {
            if(formats[i].format == in_fmt) {
                chosen  = formats[i];
                matched = BR_TRUE;
                break;
            }
        }

        if(matched) {
            for(uint32_t i = 0; i < format_count; ++i) {
                if(formats[i].format == in_fmt && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    chosen = formats[i];
                    break;
                }
            }
        }
    }

    /*
     * Failing an exact match, prefer a common 8-bit format in order of
     * desirability, again preferring an sRGB colour space. A lone
     * VK_FORMAT_UNDEFINED entry means the surface has no preference and
     * leaves the format up to us, so start from formats[0] in that case.
     */
    if(!matched && !(format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED)) {
        static const VkFormat preferred_formats[] = {
            VK_FORMAT_B8G8R8A8_SRGB,
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_FORMAT_R8G8B8A8_UNORM,
        };

        for(uint32_t i = 0; i < BR_ASIZE(preferred_formats); ++i) {
            VkSurfaceFormatKHR match;
            br_boolean         found = BR_FALSE;

            for(uint32_t j = 0; j < format_count; ++j) {
                if(formats[j].format != preferred_formats[i])
                    continue;

                if(!found) {
                    match = formats[j];
                    found = BR_TRUE;
                }

                if(formats[j].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    match = formats[j];
                    break;
                }
            }

            if(found) {
                chosen = match;
                break;
            }
        }
    }

    *out = chosen;
    BrLogTrace("VKREND", "Selected swapchain format %d, colour space %d", chosen.format, chosen.colorSpace);

    BrScratchFree(formats);
    return VK_SUCCESS;
}

/*
 * FIFO is the only present mode the spec guarantees, and the one we default to;
 * only look for another one if the surface somehow doesn't offer it.
 */
static VkPresentModeKHR DeviceVkGetPresentMode(const GladVulkanContext *vk, VkPhysicalDevice phy, VkSurfaceKHR surf)
{
    uint32_t          mode_count = 0;
    VkPresentModeKHR *modes;
    VkPresentModeKHR  mode = VK_PRESENT_MODE_FIFO_KHR; /* TODO: This may be a good config option. */

    if(vk->GetPhysicalDeviceSurfacePresentModesKHR(phy, surf, &mode_count, NULL) != VK_SUCCESS || mode_count == 0) {
        return mode;
    }

    if((modes = BrScratchAllocate(sizeof(VkPresentModeKHR) * mode_count)) == NULL) {
        return mode;
    }

    if(vk->GetPhysicalDeviceSurfacePresentModesKHR(phy, surf, &mode_count, modes) == VK_SUCCESS) {
        for(uint32_t i = 0; i < mode_count; ++i) {
            if(modes[i] == VK_PRESENT_MODE_FIFO_KHR) {
                BrScratchFree(modes);
                return mode;
            }
        }

        mode = modes[0];
        BrLogWarn("VKREND", "Surface does not support the FIFO present mode, using %d instead", mode);
    }

    BrScratchFree(modes);
    return mode;
}

VkResult DeviceVkCreateSwapchain(const GladVulkanContext *vk, VkPhysicalDevice phy, VkDevice device, VkQueue queue,
                                 const DeviceVkCreateSwapchainInfo *info, DeviceVkSwapchain *out)
{
    VkSurfaceFormatKHR            format = {.format = VK_FORMAT_UNDEFINED, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    VkSurfaceCapabilitiesKHR      caps;
    VkSwapchainKHR                swap_chain;
    VkExtent2D                    extent;
    VkSurfaceTransformFlagBitsKHR pre_transform;
    VkCompositeAlphaFlagBitsKHR   composite_alpha;
    VkPresentModeKHR              present_mode;
    uint32_t                      image_count;
    VkImage                      *images;
    VkResult                      res;

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
     * If the surface reports a concrete extent it has already sized itself to
     * the window; otherwise clamp the requested size into the supported range.
     */
    if(caps.currentExtent.width != UINT32_MAX) {
        extent = caps.currentExtent;
    } else {
        uint32_t w = (uint32_t)info->width;
        uint32_t h = (uint32_t)info->height;

        if(w < caps.minImageExtent.width)
            w = caps.minImageExtent.width;
        else if(w > caps.maxImageExtent.width)
            w = caps.maxImageExtent.width;

        if(h < caps.minImageExtent.height)
            h = caps.minImageExtent.height;
        else if(h > caps.maxImageExtent.height)
            h = caps.maxImageExtent.height;

        extent = (VkExtent2D){.width = w, .height = h};
    }

    /*
     * currentTransform must name a single transform; if the surface reports
     * something else, prefer IDENTITY when it's supported and the lowest
     * supported bit otherwise.
     */
    pre_transform = caps.currentTransform;
    if((pre_transform & (pre_transform - 1)) != 0) {
        if(caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        } else {
            pre_transform = (VkSurfaceTransformFlagBitsKHR)(caps.supportedTransforms & (~caps.supportedTransforms + 1u));
        }
    }

    /*
     * compositeAlpha must be a single supported bit; take the first of the
     * four in preference order.
     */
    {
        static const VkCompositeAlphaFlagBitsKHR supported_alpha[] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };

        composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        for(uint32_t i = 0; i < BR_ASIZE(supported_alpha); ++i) {
            if(caps.supportedCompositeAlpha & supported_alpha[i]) {
                composite_alpha = supported_alpha[i];
                break;
            }
        }
    }

    /*
     * We only ever write to the swapchain images with the transfer engine, so
     * a surface that can't do that is unusable to us.
     */
    if(!(caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        BrLogError("VKREND", "Surface does not support VK_IMAGE_USAGE_TRANSFER_DST_BIT (supportedUsageFlags = 0x%" PRIx32 ")",
                   caps.supportedUsageFlags);
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    /*
     * The frame ring is fixed at BR_VKREND_MAX_SWAPCHAIN_IMAGES entries, so a
     * surface that needs a deeper swapchain cannot be driven by us. Fail up
     * front rather than let vkAcquireNextImageKHR() hand back an index we
     * cannot index.
     */
    if(caps.minImageCount > BR_VKREND_MAX_SWAPCHAIN_IMAGES) {
        BrLogError("VKREND", "Surface requires at least %" PRIu32 " swapchain images, but only %d are supported", caps.minImageCount,
                   BR_VKREND_MAX_SWAPCHAIN_IMAGES);
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    present_mode = DeviceVkGetPresentMode(vk, phy, info->surface);

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
        .imageExtent           = extent,
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = NULL,
        .preTransform          = pre_transform,
        .compositeAlpha        = composite_alpha,
        .presentMode           = present_mode,
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
        BrLogError("VKREND", "Swapchain reported %" PRIu32 " images, but only %d are supported", image_count, BR_VKREND_MAX_SWAPCHAIN_IMAGES);

        vk->DestroySwapchainKHR(device, swap_chain, NULL);
        BrScratchFree(images);
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    *out = (DeviceVkSwapchain){
        .surface_format       = format,
        .surface_capabilities = caps,
        .extent               = swapchain_create_info.imageExtent,
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
