#include "drv.h"

// clang-format off
#define BRPM_TO_GL(_pm_type, _internal_format, _format, _type, _bytes, _blended, _indexed) \
    [_pm_type] = {                                                               \
        .pm_type         = _pm_type,                                             \
        .internal_format = _internal_format,                                     \
        .format          = _format,                                              \
        .type            = _type,                                                \
        .bytes           = _bytes,                                               \
        .blended         = _blended,                                             \
        .indexed         = _indexed,                                             \
    }

#if BR_ENDIAN_LITTLE
#   define GL_RGBN GL_BGR
#else
#   define GL_RGBN GL_RGB
#endif

const static br_pixelmap_gl_fmt br2gl[BR_PMT_MAX] = {
    BRPM_TO_GL(BR_PMT_INDEX_8,       GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE,              4, BR_FALSE, BR_TRUE ),
    BRPM_TO_GL(BR_PMT_RGB_332,       GL_RGB,             GL_RGB,             GL_UNSIGNED_BYTE_3_3_2,        1, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGB_555,       GL_RGB,             GL_BGRA,            GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_BGR_555,       GL_RGB,             GL_RGBA,            GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGB_565,       GL_RGB,             GL_RGB,             GL_UNSIGNED_SHORT_5_6_5,       2, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_BGR_565,       GL_RGB,             GL_RGB,             GL_UNSIGNED_SHORT_5_6_5_REV,   2, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGBA_4444,     GL_RGBA,            GL_RGBA,            GL_UNSIGNED_SHORT_4_4_4_4,     2, BR_TRUE,  BR_FALSE),
    BRPM_TO_GL(BR_PMT_ARGB_4444,     GL_RGBA,            GL_BGRA,            GL_UNSIGNED_SHORT_4_4_4_4_REV, 2, BR_TRUE,  BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGB_888,       GL_RGB,             GL_RGBN,            GL_UNSIGNED_BYTE,              3, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGBX_888,      GL_RGB,             GL_BGRA,            GL_UNSIGNED_INT_8_8_8_8_REV,   3, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGBA_8888,     GL_RGBA,            GL_BGRA,            GL_UNSIGNED_INT_8_8_8_8_REV,   4, BR_TRUE,  BR_FALSE),
    BRPM_TO_GL(BR_PMT_RGBA_8888_ARR, GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE,              4, BR_TRUE,  BR_FALSE),
    BRPM_TO_GL(BR_PMT_R8G8B8A8,      GL_RGBA,            GL_RGBA,            GL_UNSIGNED_INT_8_8_8_8,       4, BR_TRUE,  BR_FALSE),
    BRPM_TO_GL(BR_PMT_DEPTH_16,      GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, 0,                             2, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_DEPTH_24,      GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, 0,                             3, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_DEPTH_32,      GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, 0,                             4, BR_FALSE, BR_FALSE),
    BRPM_TO_GL(BR_PMT_DEPTH_FP32,    GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, 0,                             4, BR_FALSE, BR_FALSE),
};
#undef GL_RGBN
#undef BRPM_TO_GL
// clang-format on

const br_pixelmap_gl_fmt *DeviceGL1xGetFormatDetails(br_uint_8 type)
{
    const br_pixelmap_gl_fmt *fmt;
    if(type >= BR_PMT_MAX)
        return NULL;

    fmt = br2gl + type;
    if(fmt->pm_type != type)
        return NULL;

    return fmt;
}
