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
