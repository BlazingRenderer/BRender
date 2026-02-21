#include "drv.h"

br_device *BR_EXPORT BrDrv1VkBegin(const char *arguments)
{
    br_device            *device;
    br_renderer_facility *rendfcty;

    if((device = DeviceVkAllocate("vkrend", arguments)) == NULL)
        return NULL;

    if((rendfcty = RendererFacilityVkAllocate(device)) == NULL) {
        BrResFree(device);
        return NULL;
    }

    if(OutputFacilityVkAllocate(device, rendfcty) == NULL) {
        BrResFree(device);
        return NULL;
    }

    return device;
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1VkBegin(arguments);
}
#endif
