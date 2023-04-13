#include "drv.h"

br_device *BR_EXPORT BrDrv1GLBegin(const char *arguments)
{
    br_device *dev = DeviceGLAllocate("opengl", arguments);
    if(!dev)
        return NULL;

    return dev;
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1GLBegin(arguments);
}
#endif
