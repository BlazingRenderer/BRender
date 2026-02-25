#include "drv.h"
#include "brassert.h"

br_error DeviceVkExtCreateSurface(br_device *self, void *window_handle, VkSurfaceKHR *surface)
{
    return self->init_info.create_surface(self->vulkan, NULL, surface, window_handle);
}

void DeviceVkExtDestroySurface(br_device *self, void *window_handle, VkSurfaceKHR surface)
{
    self->init_info.destroy_surface(self->vulkan, NULL, surface, window_handle);
}

br_error DevicePixelmapVkExtResize(br_device_pixelmap *self, br_int_32 w, br_int_32 h)
{
    UASSERT(self->use_type == BRT_NONE);
    return self->device->init_info.resize((br_pixelmap *)self, w, h, self->as_front.window_handle);
}

void DevicePixelmapVkExtFree(br_device_pixelmap *self)
{
    UASSERT(self->use_type == BRT_NONE);
    self->device->init_info.free((br_pixelmap *)self, self->as_front.window_handle);
}

br_error DevicePixelmapVkExtHandleWindowEvent(br_device_pixelmap *self, void *arg)
{
    UASSERT(self->use_type == BRT_NONE);
    return self->device->init_info.handle_window_event((br_pixelmap *)self, arg, self->as_front.window_handle);
}
