#include "drv.h"

br_device *BR_EXPORT BrDrv1GL1xBegin(const char *arguments)
{
    return DeviceGL1xAllocate("glrend1x", arguments);
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1GL1xBegin(arguments);
}
#endif
