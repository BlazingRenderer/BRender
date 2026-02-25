/*
 * Prototypes for functions internal to driver
 */
#ifndef DRV_IP_H_
#define DRV_IP_H_

#ifndef NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

/*
 * device.c
 */
br_device *DeviceVkAllocate(const char *identifier, const char *arguments);

/*
 * select.c
 */
br_device_vk_phy *DeviceVkSelect(void *res, const GladVulkanContext *vk, VkInstance instance, const br_device_vk_select *select);

/*
 * rendfcty.c
 */
br_renderer_facility *RendererFacilityVkAllocate(br_device *dev);

/*
 * outfcty.c
 */
br_output_facility *OutputFacilityVkAllocate(br_device *dev, br_renderer_facility *rendfcty);

/*
 * devpixmpf.c
 */
br_device_pixelmap          *DevicePixelmapVkAllocateFront(br_device *dev, br_output_facility *outfcty, br_token_value *tv);
br_device_pixelmap_vk_frame *DevicePixelmapVkGetCurrentFrame(br_device_pixelmap *self);

/*
 * devpixmp.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv);

/*
 * formats.c
 */
const br_pixelmap_vk_fmt *DeviceVkGetFormatDetails(br_uint_8 type);

/*
 * util.c
 */
br_uint_8 DeviceVkTypeOrBits(br_uint_8 pixel_type, br_int_32 pixel_bits);
VkResult DeviceVkGetSwapchainFormat(const GladVulkanContext *vk, VkPhysicalDevice phy, VkSurfaceKHR surf, VkFormat in_fmt, VkSurfaceFormatKHR *out);
VkResult DeviceVkCreateSwapchain(const GladVulkanContext *vk, VkPhysicalDevice phy, VkDevice device, VkQueue queue,
                                 const DeviceVkCreateSwapchainInfo *info, DeviceVkSwapchain *out);
void     DeviceVkDestroySwapchain(const GladVulkanContext *vk, VkDevice device, DeviceVkSwapchain *swapchain);

/*
 * ext_procs.c
 */
br_error DeviceVkExtCreateSurface(br_device *self, void *window_handle, VkSurfaceKHR *surface);
void     DeviceVkExtDestroySurface(br_device *self, void *window_handle, VkSurfaceKHR surface);
br_error DevicePixelmapVkExtResize(br_device_pixelmap *self, br_int_32 w, br_int_32 h);
br_error DevicePixelmapVkExtHandleWindowEvent(br_device_pixelmap *self, void *arg);
void     DevicePixelmapVkExtFree(br_device_pixelmap *self);

/*
 * uuid.c
 */
void       DeviceVkUUIDNil(br_device_vk_uuid *uuid);
void       DeviceVkUUIDMax(br_device_vk_uuid *uuid);
char      *DeviceVkRawUUIDToString(char buf[DEVICE_VK_UUID_STRING_SIZE], const br_uint_8 uuid[VK_UUID_SIZE]);
char      *DeviceVkUUIDToString(char buf[DEVICE_VK_UUID_STRING_SIZE], const br_device_vk_uuid *uuid);
char      *DeviceVkResUUIDToString(void *vparent, const br_device_vk_uuid *uuid);
br_error   DeviceVkUUIDParse(br_device_vk_uuid *uuid, const char *s);
int        DeviceVkRawUUIDCompare(const br_uint_8 a[VK_UUID_SIZE], const br_uint_8 b[VK_UUID_SIZE]);
int        DeviceVkUUIDCompare(const br_device_vk_uuid *a, const br_device_vk_uuid *b);
br_boolean DeviceVkUUIDIsNil(const br_device_vk_uuid *uuid);
br_boolean DeviceVkUUIDIsMax(const br_device_vk_uuid *uuid);
void       DeviceVkUUIDCopy(br_device_vk_uuid *dst, const br_device_vk_uuid *src);

#ifdef __cplusplus
};
#endif

#endif
#endif /* DRV_IP_H_ */
