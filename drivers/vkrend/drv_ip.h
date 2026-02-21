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
 * renderer.c
 */
br_renderer          *RendererVkAllocate(br_device *device, br_renderer_facility *facility, br_device_pixelmap *dest);
state_stack          *RendererVkAllocState(br_renderer *self, const state_stack *tpl, br_uint_32 refs);
void                  RendererVkUnrefState(br_renderer *self, state_stack *state);
br_vk_material_state *RendererVkMaterialReserve(br_renderer *self, VkDeviceSize *out_offset);
void                  RendererVkMaterialFlush(br_renderer *self, VkDeviceSize offset);

/*
 * state.c
 */
void                      StateVkInit(state_all *state, void *res);
struct br_tv_template    *StateVkGetStateTemplate(state_all *state, br_token part, br_int_32 index);
void                      StateVkTemplateActions(state_all *state, uint32_t mask);
br_boolean                StateVkPush(state_all *state, uint32_t mask);
br_boolean                StateVkPop(state_all *state, uint32_t mask);
void                      StateVkDefault(state_all *state, uint32_t mask);
void                      StateVkCopy(state_stack *dst, const state_stack *src, uint32_t mask);
br_renderer_state_stored *RendererStateStoredVkAllocate(br_renderer *renderer, state_stack *base_state, br_uint_32 m, br_token_value *tv);

/*
 * gstored.c
 */
br_geometry_stored *GeometryStoredVkAllocate(br_geometry_v1_model *gv1model, const char *id, br_renderer *r, struct v11model *model);

/*
 * sbuffer.c
 */
struct br_buffer_stored *BufferStoredVkAllocate(br_renderer *renderer, br_token use, struct br_device_pixelmap *pm, br_token_value *tv);
VkImageView              BufferStoredVkGetView(const struct br_buffer_stored *self);
VkImageView              BufferStoredVkGetClutView(const struct br_buffer_stored *self);
VkImageLayout            BufferStoredVkGetViewLayout(const struct br_buffer_stored *self);
br_boolean               BufferStoredVkIsIndexed(const struct br_buffer_stored *self);

/*
 * gv1model.c
 */
br_geometry_v1_model *GeometryV1ModelVkAllocate(br_renderer_facility *type, const char *id);

/*
 * gv1buckt.c
 */
br_geometry_v1_buckets *GeometryV1BucketsVkAllocate(br_renderer_facility *type, const char *id);
br_vk_render_state      DeviceVkRenderEnsureInit(br_device *dev);
void                    DeviceVkRenderFini(br_device *dev);
VkPipeline DeviceVkRenderGetPipeline(br_device *dev, VkFormat format, VkSampleCountFlagBits samples, br_boolean lit, br_boolean textured,
                                     br_boolean indexed, br_token shading_mode, VkCullModeFlags cull, VkFrontFace front_face,
                                     br_boolean blended, br_token blend_mode, br_boolean depth_test, br_boolean depth_write,
                                     VkCompareOp depth_compare, VkFormat depth_format);
VkSampler  DeviceVkRenderGetSampler(br_device *dev, br_token filter, br_token mip_filter, br_token wrap_s, br_token wrap_t);
void       VkV1ModelRenderGroup(br_geometry_stored *geom, br_renderer *renderer, const br_geometry_stored_group *group);
void       VkV1ModelRenderFace(br_geometry_stored *geom, br_renderer *renderer, br_uint_32 first_index);

/*
 * Null geometry lighting, shared with nulldev.
 */
br_geometry_lighting *GeometryLightingNullAllocate(br_renderer_facility *type, const char *id);

/*
 * outfcty.c
 */
br_output_facility *OutputFacilityVkAllocate(br_device *dev, br_renderer_facility *rendfcty);

/*
 * devclut.c
 */
br_device_clut *DeviceClutVkAllocate(br_device_pixelmap *pm);

/*
 * devpixmpf.c
 */
br_device_pixelmap          *DevicePixelmapVkAllocateFront(br_device *dev, br_output_facility *outfcty, br_token_value *tv);
br_device_pixelmap_vk_frame *DevicePixelmapVkGetCurrentFrame(br_device_pixelmap *self);
void                         DevicePixelmapVkFlushFrame(br_device_pixelmap *screen);
void DevicePixelmapVkRetireImage(br_device_pixelmap_vk_frame *frame, VkImage image, VkImageView view, VmaAllocation allocation);
void DevicePixelmapVkRetireBuffer(br_device_pixelmap_vk_frame *frame, VkBuffer buffer, VmaAllocation allocation);
void DevicePixelmapVkRetireDrain(br_device_pixelmap_vk_frame *frame);

/*
 * devpixmp.c
 */
br_error            BR_CMETHOD_DECL(br_device_pixelmap_vk, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv);
void                DevicePixelmapVkIncRef(br_device_pixelmap *self);
void                DevicePixelmapVkDecRef(br_device_pixelmap *self);
br_boolean          DevicePixelmapVkIsOffscreen(const br_device_pixelmap *pm);
br_boolean          DevicePixelmapVkIsSub(const br_device_pixelmap *pm);
br_device_pixelmap *DevicePixelmapVkResolve(const br_device_pixelmap *pm);
br_boolean          DevicePixelmapVkIsDepth(const br_device_pixelmap *pm);

/*
 * devpmsub.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_rectangle *rect);

/*
 * formats.c
 */
const br_pixelmap_vk_fmt *DeviceVkGetFormatDetails(br_uint_8 type);

/*
 * rect.c
 */
br_vk_rect_state DeviceVkRectEnsureInit(br_device *dev);
void             DeviceVkRectFini(br_device *dev);
br_error         DeviceVkRectDraw(br_device_pixelmap *dst, br_rectangle *dst_rect, br_device_pixelmap *src, br_rectangle *src_rect);

/*
 * line.c
 */
br_vk_line_state DeviceVkLineEnsureInit(br_device *dev);
void             DeviceVkLineFini(br_device *dev);
br_error DeviceVkLineDraw(br_device_pixelmap *dst, br_point *start, br_point *end, const float colour[4], br_boolean point, br_boolean indexed);

/*
 * text.c
 */
br_vk_text_state DeviceVkTextEnsureInit(br_device *dev);
void             DeviceVkTextFini(br_device *dev);
br_error         DeviceVkTextDraw(br_device_pixelmap *dst, br_point *point, br_font *font, const char *text, const float colour[4]);

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
void     DevicePixelmapVkExtPreSwap(br_device_pixelmap *self);
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
