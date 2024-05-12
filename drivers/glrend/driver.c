#include "drv.h"

br_token BRT_OPENGL_TEXTURE_U32 = BR_NULL_TOKEN;

br_device *BR_EXPORT BrDrv1GLBegin(const char *arguments)
{
    br_device *dev = DeviceGLAllocate("glrend", arguments);
    if(!dev)
        return NULL;

    BRT_OPENGL_TEXTURE_U32 = BrTokenCreate("OPENGL_TEXTURE_U32", BRT_UINT_32);
    return dev;
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1GLBegin(arguments);
}
#endif
