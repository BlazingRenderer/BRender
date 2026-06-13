#include "drv.h"

br_device *BR_EXPORT BrDrv1GLBegin(const char *arguments)
{
    return DeviceGLAllocate("glrend", arguments);
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1GLBegin(arguments);
}
#endif
