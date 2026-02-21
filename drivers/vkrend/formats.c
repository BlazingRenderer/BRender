#include "drv.h"

/*
 * _depth marks the depth types, which are the only ones created and viewed with the
 * DEPTH aspect and the depth-stencil-attachment usage. It cannot be derived from the
 * VkFormat alone without a second table, so it rides alongside the format here.
 */
#define BRPM_TO_VK(_pm_type, _format, _bpp, _indexed, _depth, _swr, _swg, _swb, _swa) \
    [_pm_type] = {                                                                    \
        .pm_type         = _pm_type,                                                  \
        .format          = VK_FORMAT_##_format,                                       \
        .bytes_per_pixel = _bpp,                                                      \
        .indexed         = _indexed,                                                  \
        .depth           = _depth,                                                    \
        .swizzle_r       = VK_COMPONENT_SWIZZLE_##_swr,                               \
        .swizzle_g       = VK_COMPONENT_SWIZZLE_##_swg,                               \
        .swizzle_b       = VK_COMPONENT_SWIZZLE_##_swb,                               \
        .swizzle_a       = VK_COMPONENT_SWIZZLE_##_swa,                               \
    }

// clang-format off
const static br_pixelmap_vk_fmt br2vk[BR_PMT_MAX] = {
    BRPM_TO_VK(BR_PMT_INDEX_8,       R8_UINT,               1, BR_TRUE,  BR_FALSE, R, ZERO, ZERO, ONE),
    BRPM_TO_VK(BR_PMT_RGB_332,       UNDEFINED,             0, BR_FALSE, BR_FALSE, ZERO, ZERO, ZERO, ZERO), /* unsupported */
    BRPM_TO_VK(BR_PMT_RGB_555,       A1R5G5B5_UNORM_PACK16, 2, BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_BGR_555,       A1R5G5B5_UNORM_PACK16, 2, BR_FALSE, BR_FALSE, B, G, R, ONE),
    BRPM_TO_VK(BR_PMT_RGB_565,       R5G6B5_UNORM_PACK16,   2, BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_BGR_565,       B5G6R5_UNORM_PACK16,   2, BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_RGBA_4444,     R4G4B4A4_UNORM_PACK16, 2, BR_FALSE, BR_FALSE, R, G, B, A),
    BRPM_TO_VK(BR_PMT_ARGB_4444,     R4G4B4A4_UNORM_PACK16, 2, BR_FALSE, BR_FALSE, G, B, A, R),
    BRPM_TO_VK(BR_PMT_RGB_888,       B8G8R8_UNORM,          3, BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_RGBX_888,      B8G8R8A8_UNORM,        4, BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_RGBA_8888,     B8G8R8A8_UNORM,        4, BR_FALSE, BR_FALSE, R, G, B, A),
    BRPM_TO_VK(BR_PMT_RGBA_8888_ARR, R8G8B8A8_UNORM,        4, BR_FALSE, BR_FALSE, R, G, B, A),
    BRPM_TO_VK(BR_PMT_R8G8B8A8,      R8G8B8A8_UNORM,        4, BR_FALSE, BR_FALSE, A, B, G, R),
    BRPM_TO_VK(BR_PMT_DEPTH_16,      D16_UNORM,             2, BR_FALSE, BR_TRUE,  R, G, B, A),
    BRPM_TO_VK(BR_PMT_DEPTH_24,      X8_D24_UNORM_PACK32,   4, BR_FALSE, BR_TRUE,  R, G, B, A),
    BRPM_TO_VK(BR_PMT_DEPTH_32,      UNDEFINED,             0, BR_FALSE, BR_TRUE,  ZERO, ZERO, ZERO, ZERO), /* unsupported */
    BRPM_TO_VK(BR_PMT_DEPTH_FP32,    D32_SFLOAT,            4, BR_FALSE, BR_TRUE,  R, G, B, A),
};
// clang-format on

const br_pixelmap_vk_fmt *DeviceVkGetFormatDetails(br_uint_8 type)
{
    const br_pixelmap_vk_fmt *fmt;
    if(type >= BR_PMT_MAX)
        return NULL;

    fmt = br2vk + type;
    if(fmt->pm_type != type)
        return NULL;

    return fmt;
}
