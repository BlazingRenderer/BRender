#include "drv.h"

#define BRPM_TO_VK(_pm_type, _format, _blended, _indexed, _swr, _swg, _swb, _swa) \
    [_pm_type] = {                                                                \
        .pm_type   = _pm_type,                                                    \
        .format    = VK_FORMAT_##_format,                                         \
        .blended   = _blended,                                                    \
        .indexed   = _indexed,                                                    \
        .swizzle_r = VK_COMPONENT_SWIZZLE_##_swr,                                 \
        .swizzle_g = VK_COMPONENT_SWIZZLE_##_swg,                                 \
        .swizzle_b = VK_COMPONENT_SWIZZLE_##_swb,                                 \
        .swizzle_a = VK_COMPONENT_SWIZZLE_##_swa,                                 \
    }

// clang-format off
const static br_pixelmap_vk_fmt br2vk[BR_PMT_MAX] = {
    BRPM_TO_VK(BR_PMT_INDEX_8,       R8_UINT,               BR_FALSE, BR_TRUE,  R, ZERO, ZERO, ONE),
    BRPM_TO_VK(BR_PMT_RGB_332,       UNDEFINED,             BR_FALSE, BR_FALSE, ZERO, ZERO, ZERO, ZERO), /* unsupported */
    BRPM_TO_VK(BR_PMT_RGB_555,       A1R5G5B5_UNORM_PACK16, BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_BGR_555,       A1R5G5B5_UNORM_PACK16, BR_FALSE, BR_FALSE, B, G, R, ONE),
    BRPM_TO_VK(BR_PMT_RGB_565,       R5G6B5_UNORM_PACK16,   BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_BGR_565,       B5G6R5_UNORM_PACK16,   BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_RGBA_4444,     R4G4B4A4_UNORM_PACK16, BR_TRUE,  BR_FALSE, R, G, B, A),
    BRPM_TO_VK(BR_PMT_ARGB_4444,     R4G4B4A4_UNORM_PACK16, BR_TRUE,  BR_FALSE, G, B, A, R),
    BRPM_TO_VK(BR_PMT_RGB_888,       B8G8R8_UNORM,          BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_RGBX_888,      B8G8R8A8_UNORM,        BR_FALSE, BR_FALSE, R, G, B, ONE),
    BRPM_TO_VK(BR_PMT_RGBA_8888,     B8G8R8A8_UNORM,        BR_TRUE,  BR_FALSE, R, G, B, A),
    BRPM_TO_VK(BR_PMT_RGBA_8888_ARR, R8G8B8A8_UNORM,        BR_TRUE,  BR_FALSE, R, G, B, A),
    BRPM_TO_VK(BR_PMT_R8G8B8A8,      R8G8B8A8_UNORM,        BR_TRUE,  BR_FALSE, R, G, B, A),
    BRPM_TO_VK(BR_PMT_DEPTH_16,      D16_UNORM,             BR_FALSE, BR_FALSE, R, G, B, A),
    BRPM_TO_VK(BR_PMT_DEPTH_24,      X8_D24_UNORM_PACK32,   BR_FALSE, BR_FALSE, R, G, B, A),
    BRPM_TO_VK(BR_PMT_DEPTH_32,      UNDEFINED,             BR_FALSE, BR_FALSE, ZERO, ZERO, ZERO, ZERO), /* unsupported */
    BRPM_TO_VK(BR_PMT_DEPTH_FP32,    D32_SFLOAT,            BR_FALSE, BR_FALSE, R, G, B, A),
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
