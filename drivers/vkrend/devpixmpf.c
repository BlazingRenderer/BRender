#include <inttypes.h>
#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_pixelmap_dispatch devicePixelmapFrontDispatch;

/*
 * Device pixelmap info. template
 */
#define F(f)  offsetof(br_device_pixelmap, f)
#define FF(f) offsetof(br_device_pixelmap, asFront.context_state.f)
static br_tv_template_entry devicePixelmapFrontTemplateEntries[] = {
    {BRT(WIDTH_I32),          F(pm_width),                     BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT(HEIGHT_I32),         F(pm_height),                    BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16},
    {BRT(PIXEL_TYPE_U8),      F(pm_type),                      BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8 },
    {BRT(OUTPUT_FACILITY_O),  F(output_facility),              BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {BRT(FACILITY_O),         F(output_facility),              BRTV_QUERY,            BRTV_CONV_COPY   },
    {BRT(IDENTIFIER_CSTR),    F(pm_identifier),                BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {DEV(VULKAN_SURFACE_H),   F(as_front.surface),             BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
    {DEV(VULKAN_SWAPCHAIN_H), F(as_front.swapchain.swapchain), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY   },
};
#undef FF
#undef F

struct pixelmapNewTokens {
    br_int_32 width;
    br_int_32 height;
    br_int_32 pixel_bits;
    br_uint_8 pixel_type;
    void     *window_handle;
};

#define F(f) offsetof(struct pixelmapNewTokens, f)
static br_tv_template_entry pixelmapNewTemplateEntries[] = {
    {BRT(WIDTH_I32),       F(width),         BRTV_SET, BRTV_CONV_COPY},
    {BRT(HEIGHT_I32),      F(height),        BRTV_SET, BRTV_CONV_COPY},
    {BRT(PIXEL_BITS_I32),  F(pixel_bits),    BRTV_SET, BRTV_CONV_COPY},
    {BRT(PIXEL_TYPE_U8),   F(pixel_type),    BRTV_SET, BRTV_CONV_COPY},
    {BRT(WINDOW_HANDLE_H), F(window_handle), BRTV_SET, BRTV_CONV_COPY},
};
#undef F

static void DeviceVkInitFrame(br_device_pixelmap_vk_frame *frame, br_device *dev, uint32_t index)
{
    const GladVulkanContext *vk = &dev->context;

    const VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    const VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = dev->vk_command_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkResult res;

    frame->index = index;

    if((res = vk->CreateFence(dev->vk_device, &fence_create_info, NULL, &frame->fence))) {
        BR_FATAL1("vkCreateFence() returned %" PRIu32, res);
    }

    if((res = vk->CreateSemaphore(dev->vk_device, &semaphore_create_info, NULL, &frame->image_acquired))) {
        BR_FATAL1("vkCreateSemaphore() returned %" PRIu32, res);
    }

    if((res = vk->CreateSemaphore(dev->vk_device, &semaphore_create_info, NULL, &frame->render_finished))) {
        BR_FATAL1("vkCreateSemaphore() returned %" PRIu32, res);
    }

    if((res = vk->AllocateCommandBuffers(dev->vk_device, &command_buffer_allocate_info, &frame->command_buffer))) {
        BR_FATAL1("vkAllocateCommandBuffers() returned %" PRIu32, res);
    }
}

br_device_pixelmap_vk_frame *DevicePixelmapVkGetCurrentFrame(br_device_pixelmap *self)
{
    return self->screen->as_front.frames + self->screen->as_front.frame_index;
}

static void DevicePixelmapVkBeginFrame(br_device_pixelmap *self)
{
    const GladVulkanContext     *vk     = &self->device->context;
    VkDevice                     device = self->device->vk_device;
    br_device_pixelmap_vk_frame *frame  = DevicePixelmapVkGetCurrentFrame(self);

    VkResult res;

    /*
     * Always do this first, as we need the command buffer to be in a "begin" state if swapchain creation fails.
     * XXOPTIM: coalesce the waits if we need to recreate the swapchain.
     */
    if(self->as_front.need_recreate) {
        res = vk->WaitForFences(device, BR_ASIZE(self->as_front.frame_fences), self->as_front.frame_fences, VK_TRUE, BR_VKREND_DEFAULT_WAIT);
    } else {
        res = vk->WaitForFences(device, 1, &frame->fence, VK_TRUE, BR_VKREND_DEFAULT_WAIT);
    }

    switch(res) {
        case VK_SUCCESS:
            break;

        case VK_TIMEOUT:
            BR_FATAL0("Waiting for frame timed out, potential GPU hang");

        default:
            BR_FATAL1("vkWaitForFences() returned %" PRIu32, res);
    }

    const VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if((res = vk->BeginCommandBuffer(frame->command_buffer, &command_buffer_begin_info)) != VK_SUCCESS) {
        BR_FATAL1("vkBeginCommandBuffer() returned %" PRIu32, res);
    }

    if(self->as_front.need_recreate) {
        VkPhysicalDevice  physical_device = self->device->physical_device->dev;
        VkDevice          device          = self->device->vk_device;
        VkQueue           queue           = self->device->vk_queue;
        DeviceVkSwapchain swapchain;

        const DeviceVkCreateSwapchainInfo create_swapchain_info = {
            .surface       = self->as_front.surface,
            .width         = self->pm_width,
            .height        = self->pm_height,
            .wanted_format = self->as_front.swapchain.surface_format.format,
            .old_swapchain = self->as_front.swapchain.swapchain,
        };

        if((res = DeviceVkCreateSwapchain(vk, physical_device, device, queue, &create_swapchain_info, &swapchain)) != VK_SUCCESS) {
            /*
             * We're literally not designed to deal with the "front screen" disappearing during render. Just die.
             */
            BR_FATAL1("unable to recreate swapchain: VkResult = %d", res);
        }

        DeviceVkDestroySwapchain(vk, device, &self->as_front.swapchain);
        self->as_front.swapchain     = swapchain;
        self->as_front.need_recreate = BR_FALSE;
    }
}

static void DevicePixelmapVkEndFrame(br_device_pixelmap *self, uint32_t image_index)
{
    const GladVulkanContext     *vk = &self->device->context;
    br_device_pixelmap_vk_frame *frame;
    VkResult                     res;

    UASSERT(self->use_type == BRT_NONE);

    frame = DevicePixelmapVkGetCurrentFrame(self);

    if((res = vk->EndCommandBuffer(frame->command_buffer)) != VK_SUCCESS) {
        BR_FATAL1("vkEndCommandBuffer() returned %" PRIu32, res);
    }

    const VkPipelineStageFlags wm = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubmitInfo submit_info = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = NULL,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &frame->image_acquired,
        .pWaitDstStageMask    = &wm,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &frame->command_buffer,
        .signalSemaphoreCount = (image_index == UINT32_MAX) ? 0 : 1,
        .pSignalSemaphores    = &frame->render_finished,
    };

    if((res = vk->ResetFences(self->device->vk_device, 1, &frame->fence)) != VK_SUCCESS) {
        BR_FATAL1("vkResetFences() returned %" PRIu32, res);
    }

    /*
     * NB: We still submit the queue even if skip presenting because we need to signal the frame's fence.
     */

    if((res = vk->QueueSubmit(self->device->vk_queue, 1, &submit_info, frame->fence)) != VK_SUCCESS) {
        BR_FATAL1("vkQueueSubmit() returned %" PRIu32, res);
    }

    if(image_index != UINT32_MAX) {
        const VkPresentInfoKHR present_info = {
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext              = NULL,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &frame->render_finished,
            .swapchainCount     = 1,
            .pSwapchains        = &self->as_front.swapchain.swapchain,
            .pImageIndices      = &image_index,
        };
        res = vk->QueuePresentKHR(self->device->vk_queue, &present_info);
        switch(res) {
            case VK_SUCCESS:
                break;

            case VK_SUBOPTIMAL_KHR:
            case VK_ERROR_OUT_OF_DATE_KHR:
                /*
                 * These can be handled the same, there's nothing else to do.
                 */
                self->as_front.need_recreate = BR_TRUE;
                break;

            default:
                BR_FATAL1("vkQueuePresentKHR() returned %" PRIu32, res);
        }
    }

    self->as_front.frame_index = (self->as_front.frame_index + 1) % self->as_front.swapchain.image_count;
}

br_device_pixelmap *DevicePixelmapVkAllocateFront(br_device *dev, br_output_facility *outfcty, br_token_value *tv)
{
    const GladVulkanContext  *vk              = &dev->context;
    VkPhysicalDevice          physical_device = dev->physical_device->dev;
    VkDevice                  device          = dev->vk_device;
    VkQueue                   queue           = dev->vk_queue;
    br_device_pixelmap       *self;
    br_int_32                 count;
    const br_pixelmap_vk_fmt *fmt;
    struct pixelmapNewTokens  pt = {
         .width         = -1,
         .height        = -1,
         .pixel_bits    = -1,
         .pixel_type    = BR_PMT_MAX,
         .window_handle = NULL,
    };

    if(dev->templates.pixelmapNewTemplate == NULL) {
        dev->templates.pixelmapNewTemplate = BrTVTemplateAllocate(dev, pixelmapNewTemplateEntries, BR_ASIZE(pixelmapNewTemplateEntries));
    }

    BrTokenValueSetMany(&pt, &count, NULL, tv, dev->templates.pixelmapNewTemplate);

    if(pt.window_handle == NULL || pt.width <= 0 || pt.height <= 0)
        return NULL;

    if((pt.pixel_type = DeviceVkTypeOrBits(pt.pixel_type, pt.pixel_bits)) == BR_PMT_MAX)
        return NULL;

    if((fmt = DeviceVkGetFormatDetails(pt.pixel_type)) == NULL)
        return NULL;

    if(fmt->indexed)
        return NULL;

    self                  = BrResAllocate(dev->res, sizeof(br_device_pixelmap), BR_MEMORY_OBJECT);
    self->pm_identifier   = BrResSprintf(self, "Vulkan:Screen:%dx%d", pt.width, pt.height);
    self->dispatch        = &devicePixelmapFrontDispatch;
    self->device          = dev;
    self->output_facility = outfcty;
    self->use_type        = BRT_NONE;
    self->screen          = self;

    self->pm_type   = pt.pixel_type;
    self->pm_width  = pt.width;
    self->pm_height = pt.height;
    self->pm_flags |= BR_PMF_NO_ACCESS;

    self->as_front.window_handle = pt.window_handle;

    if(DeviceVkExtCreateSurface(dev, pt.window_handle, &self->as_front.surface) != BRE_OK) {
        BrLogError("VKREND", "Unable to create surface.");
        BrResFreeNoCallback(self);
        return NULL;
    }

    for(uint32_t i = 0; i < BR_VKREND_MAX_SWAPCHAIN_IMAGES; ++i) {
        DeviceVkInitFrame(self->as_front.frames + i, dev, i);
        self->as_front.frame_fences[i] = self->as_front.frames[i].fence;
    }

    const DeviceVkCreateSwapchainInfo create_swapchain_info = {
        .surface       = self->as_front.surface,
        .width         = pt.width,
        .height        = pt.height,
        .wanted_format = fmt->format,
        .old_swapchain = VK_NULL_HANDLE,
    };
    if(DeviceVkCreateSwapchain(vk, physical_device, device, queue, &create_swapchain_info, &self->as_front.swapchain) != VK_SUCCESS) {
        BrResFree(self);
        return NULL;
    }

    ObjectContainerAddFront(self->output_facility, (br_object *)self);

    self->as_front.frame_index   = 0;
    self->as_front.need_recreate = BR_FALSE;
    DevicePixelmapVkBeginFrame(self);
    return self;
}

static void BR_CMETHOD_DECL(br_device_pixelmap_vk_front, free)(br_object *_self)
{
    br_device_pixelmap          *self          = (br_device_pixelmap *)_self;
    const GladVulkanContext     *vk            = &self->device->context;
    VkQueue                      vk_queue      = self->device->vk_queue;
    VkDevice                     vk_device     = self->device->vk_device;
    br_device_pixelmap_vk_frame *current_frame = DevicePixelmapVkGetCurrentFrame(self);
    VkResult                     res;

    BrLogTrace("VKREND", "Freeing %s", self->pm_identifier);

    if((res = vk->EndCommandBuffer(current_frame->command_buffer)) != VK_SUCCESS) {
        BR_FATAL1("vkEndCommandBuffer() returned %" PRIu32, res);
    }

    if((res = vk->QueueWaitIdle(vk_queue)) != VK_SUCCESS) {
        BR_FATAL1("vkQueueWaitIdle() returned %" PRIu32, res);
    }

    /* NB: no need to wait for fences because we've just waited for the queue to finish. */

    DeviceVkDestroySwapchain(vk, vk_device, &self->as_front.swapchain);

    for(uint32_t i = 0; i < BR_VKREND_MAX_SWAPCHAIN_IMAGES; ++i) {
        br_device_pixelmap_vk_frame *frame = self->as_front.frames + i;

        vk->FreeCommandBuffers(vk_device, self->device->vk_command_pool, 1, &frame->command_buffer);
        vk->DestroySemaphore(vk_device, frame->render_finished, NULL);
        vk->DestroySemaphore(vk_device, frame->image_acquired, NULL);
        vk->DestroyFence(vk_device, frame->fence, NULL);

        self->as_front.frame_fences[i] = VK_NULL_HANDLE;
    }

    ObjectContainerRemove(self->output_facility, (br_object *)self);

    DeviceVkExtDestroySurface(self->device, self->as_front.window_handle, self->as_front.surface);
    DevicePixelmapVkExtFree(self);

    BrResFreeNoCallback(self);
}

const char *BR_CMETHOD_DECL(br_device_pixelmap_vk_front, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

br_token BR_CMETHOD_DECL(br_device_pixelmap_vk_front, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE_PIXELMAP;
}

br_boolean BR_CMETHOD_DECL(br_device_pixelmap_vk_front, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

br_device *BR_CMETHOD_DECL(br_device_pixelmap_vk_front, device)(br_object *self)
{
    (void)self;
    return ((br_device_pixelmap *)self)->device;
}

br_size_t BR_CMETHOD_DECL(br_device_pixelmap_vk_front, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device_pixelmap);
}

br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_vk_front, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapFrontTemplate == NULL) {
        self->device->templates.devicePixelmapFrontTemplate = BrTVTemplateAllocate(self->device, devicePixelmapFrontTemplateEntries,
                                                                                   BR_ASIZE(devicePixelmapFrontTemplateEntries));
    }

    return self->device->templates.devicePixelmapFrontTemplate;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_vk_front, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    br_error err;

    if(self->pm_width == width && self->pm_height == height)
        return BRE_OK;

    if((err = DevicePixelmapVkExtResize(self, width, height)) != BRE_OK)
        return err;

    /*
     * NB: No need to recreate the swapchain, it'll be picked up a VK_SUBOPTIMAL_KHR.
     */
    self->pm_width  = width;
    self->pm_height = height;
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_vk_front, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    const GladVulkanContext     *vk = &self->device->context;
    br_device_pixelmap_vk_frame *frame;
    VkResult                     res;
    uint32_t                     image_index;
    br_boolean                   skip_present = BR_FALSE;

    /*
     * Ignore self-blit.
     */
    if(self == src)
        return BRE_OK;

    if(ObjectDevice(src) != self->device)
        return BRE_UNSUPPORTED;

    if(self->use_type != BRT_NONE || src->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    frame = DevicePixelmapVkGetCurrentFrame(self);

    res = vk->AcquireNextImageKHR(self->device->vk_device, self->as_front.swapchain.swapchain, UINT64_MAX, frame->image_acquired, NULL, &image_index);
#if 0
    static int xxx = 0;
    ++xxx;
    if(xxx == 60) {
        xxx = 0;
        res = VK_ERROR_OUT_OF_DATE_KHR;
    }
#endif
    switch(res) {
        case VK_SUCCESS:
            break;

        case VK_SUBOPTIMAL_KHR: {
            /*
             * Finish the frame, recreate next.
             */
            self->as_front.need_recreate = BR_TRUE;
            break;
        }

        case VK_ERROR_OUT_OF_DATE_KHR: {
            /*
             * Drop the frame, recreate next.
             */
            self->as_front.need_recreate = BR_TRUE;
            skip_present                 = BR_TRUE;
            break;
        }
        default:
            BR_FATAL1("AcquireNextImageKHR() returned %" PRIu32, res);
    }

    if(!skip_present) {
        const VkImageSubresourceRange image_subresource_range = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        };

        {
            const VkImageMemoryBarrier image_memory_barrier = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
                .image            = self->as_front.swapchain.images[image_index],
                .subresourceRange = image_subresource_range,
            };
            vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                                   NULL, 0, NULL, 1, &image_memory_barrier);
        }
#if 1
        /* Test fill with red */
        {
            const VkClearColorValue xx = {
                .float32 = {1, 0, 0, 1},
            };
            vk->CmdClearColorImage(frame->command_buffer, self->as_front.swapchain.images[image_index], VK_IMAGE_LAYOUT_GENERAL, &xx, 1,
                                   &image_subresource_range);
        }
#endif
        const VkImageMemoryBarrier image_memory_barrier = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout        = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image            = self->as_front.swapchain.images[image_index],
            .subresourceRange = image_subresource_range,
        };
        vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL,
                               0, NULL, 1, &image_memory_barrier);
    }

    DevicePixelmapVkEndFrame(self, skip_present ? UINT32_MAX : image_index);
    DevicePixelmapVkBeginFrame(self);
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_vk_front, handleWindowEvent)(br_device_pixelmap *self, void *arg)
{
    return DevicePixelmapVkExtHandleWindowEvent(self, arg);
}

/*
 * Default dispatch table for device pixelmap
 */
static const struct br_device_pixelmap_dispatch devicePixelmapFrontDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_vk_front, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_vk_front, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_vk_front, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_vk_front, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_vk_front, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_vk_front, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_vk_front, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_null, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_vk_front, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_vk, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_fail, allocateSub),

    ._copy         = BR_CMETHOD_REF(br_device_pixelmap_fail, copy),
    ._copyTo       = BR_CMETHOD_REF(br_device_pixelmap_fail, copyTo),
    ._copyFrom     = BR_CMETHOD_REF(br_device_pixelmap_fail, copyFrom),
    ._fill         = BR_CMETHOD_REF(br_device_pixelmap_fail, fill),
    ._doubleBuffer = BR_CMETHOD_REF(br_device_pixelmap_vk_front, doubleBuffer),

    ._copyDirty         = BR_CMETHOD_REF(br_device_pixelmap_fail, copyDirty),
    ._copyToDirty       = BR_CMETHOD_REF(br_device_pixelmap_fail, copyToDirty),
    ._copyFromDirty     = BR_CMETHOD_REF(br_device_pixelmap_fail, copyFromDirty),
    ._fillDirty         = BR_CMETHOD_REF(br_device_pixelmap_fail, fillDirty),
    ._doubleBufferDirty = BR_CMETHOD_REF(br_device_pixelmap_fail, doubleBufferDirty),

    ._rectangle                = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangle),
    ._rectangle2               = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangle2),
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleCopyTo),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyTo),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_fail, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_fail, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_fail, text),
    ._textBounds = BR_CMETHOD_REF(br_device_pixelmap_gen, textBounds),

    ._rowSize  = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSize),
    ._rowQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, rowQuery),
    ._rowSet   = BR_CMETHOD_REF(br_device_pixelmap_fail, rowSet),

    ._pixelQuery        = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelQuery),
    ._pixelAddressQuery = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressQuery),

    ._pixelAddressSet = BR_CMETHOD_REF(br_device_pixelmap_fail, pixelAddressSet),
    ._originSet       = BR_CMETHOD_REF(br_device_pixelmap_gen, originSet),

    ._flush        = BR_CMETHOD_REF(br_device_pixelmap_fail, flush),
    ._synchronise  = BR_CMETHOD_REF(br_device_pixelmap_fail, synchronise),
    ._directLock   = BR_CMETHOD_REF(br_device_pixelmap_fail, directLock),
    ._directUnlock = BR_CMETHOD_REF(br_device_pixelmap_fail, directUnlock),
    ._getControls  = BR_CMETHOD_REF(br_device_pixelmap_fail, getControls),
    ._setControls  = BR_CMETHOD_REF(br_device_pixelmap_fail, setControls),

    ._handleWindowEvent = BR_CMETHOD_REF(br_device_pixelmap_vk_front, handleWindowEvent),
};
