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
