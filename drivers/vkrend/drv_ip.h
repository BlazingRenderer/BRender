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
 * formats.c
 */
const br_pixelmap_vk_fmt *DeviceVkGetFormatDetails(br_uint_8 type);


/*
 * util.c
 */
br_uint_8 DeviceVkTypeOrBits(br_uint_8 pixel_type, br_int_32 pixel_bits);
VkResult DeviceVkGetSwapchainFormat(const GladVulkanContext *vk, VkPhysicalDevice phy, VkSurfaceKHR surf, VkFormat in_fmt, VkSurfaceFormatKHR *out);

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
