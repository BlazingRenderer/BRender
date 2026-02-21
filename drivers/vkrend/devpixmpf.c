#include <inttypes.h>
#include <string.h>
#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for device (defined at end of file)
 */
static const struct br_device_pixelmap_dispatch devicePixelmapFrontDispatch;

/*
 * Device pixelmap info. template
 */
#define F(f) offsetof(br_device_pixelmap, f)
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

    if((res = vk->AllocateCommandBuffers(dev->vk_device, &command_buffer_allocate_info, &frame->command_buffer))) {
        BR_FATAL1("vkAllocateCommandBuffers() returned %" PRIu32, res);
    }
}

br_device_pixelmap_vk_frame *DevicePixelmapVkGetCurrentFrame(br_device_pixelmap *self)
{
    return self->screen->as_front.frames + self->screen->as_front.frame_index;
}

/*
 * Submit everything recorded into the front screen's current frame slot so far, wait for
 * it, then begin a fresh command buffer for subsequent work.
 *
 * Device work is normally submitted only by DevicePixelmapVkEndFrame(), from
 * doubleBuffer(), so a readback issued before then would race the recording. This flushes
 * the recorded work so its results are readable, without presenting: no image is acquired
 * outside doubleBuffer(), so there is no image_acquired wait and no render_finished signal.
 * It must not be called from between AcquireNextImageKHR() and EndFrame() in doubleBuffer(),
 * where the frame's command buffer also holds the swapchain blit - there it would submit
 * that blit without the acquire wait. That window is entirely inside doubleBuffer(), and
 * the application cannot re-enter it.
 */
void DevicePixelmapVkFlushFrame(br_device_pixelmap *screen)
{
    const GladVulkanContext     *vk = &screen->device->context;
    br_device_pixelmap_vk_frame *frame;
    VkResult                     res;

    UASSERT(screen->use_type == BRT_NONE);

    frame = DevicePixelmapVkGetCurrentFrame(screen);

    if((res = vk->EndCommandBuffer(frame->command_buffer)) != VK_SUCCESS) {
        BR_FATAL1("vkEndCommandBuffer() returned %" PRIu32, res);
    }

    if((res = vk->ResetFences(screen->device->vk_device, 1, &frame->fence)) != VK_SUCCESS) {
        BR_FATAL1("vkResetFences() returned %" PRIu32, res);
    }

    const VkSubmitInfo submit_info = {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &frame->command_buffer,
    };

    if((res = vk->QueueSubmit(screen->device->vk_queue, 1, &submit_info, frame->fence)) != VK_SUCCESS) {
        BR_FATAL1("vkQueueSubmit() returned %" PRIu32, res);
    }

    if((res = vk->WaitForFences(screen->device->vk_device, 1, &frame->fence, VK_TRUE, BR_VKREND_DEFAULT_WAIT)) != VK_SUCCESS) {
        BR_FATAL1("vkWaitForFences() returned %" PRIu32, res);
    }

    /*
     * The fence we just waited covers the submission above, so every resource retired
     * against this slot - which is what a never-presenting render-and-readback loop
     * accumulates (the per-scene block is retired on every sceneBegin) - is now safe to
     * destroy. BeginFrame() normally drains the slot, but a flush re-begins without going
     * through it, so without this the list would grow without bound.
     */
    DevicePixelmapVkRetireDrain(frame);

    const VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if((res = vk->BeginCommandBuffer(frame->command_buffer, &command_buffer_begin_info)) != VK_SUCCESS) {
        BR_FATAL1("vkBeginCommandBuffer() returned %" PRIu32, res);
    }
}

/*
 * Reserve the next entry on a frame slot's retire list, growing it if it is full.
 *
 * The list is a BRender resource child of the front pixelmap (frame->owner), so it is
 * released with the pixelmap even if teardown never drains it; growing allocates a
 * fresh block from the same parent and frees the old one.
 */
static br_vk_retire_entry *DevicePixelmapVkRetireReserve(br_device_pixelmap_vk_frame *frame)
{
    if(frame->retire_count == frame->retire_capacity) {
        br_uint_32          capacity = frame->retire_capacity ? frame->retire_capacity * 2 : BR_VKREND_RETIRE_INITIAL_CAPACITY;
        br_vk_retire_entry *entries  = BrResAllocate(frame->owner, capacity * sizeof(*entries), BR_MEMORY_OBJECT_DATA);

        if(entries == NULL) {
            BR_FATAL0("Unable to grow frame retire list");
        }

        if(frame->retire_entries != NULL) {
            memcpy(entries, frame->retire_entries, frame->retire_count * sizeof(*entries));
            BrResFree(frame->retire_entries);
        }

        frame->retire_entries  = entries;
        frame->retire_capacity = capacity;
    }

    return frame->retire_entries + frame->retire_count++;
}

void DevicePixelmapVkRetireImage(br_device_pixelmap_vk_frame *frame, VkImage image, VkImageView view, VmaAllocation allocation)
{
    br_vk_retire_entry *entry = DevicePixelmapVkRetireReserve(frame);

    entry->kind       = BR_VK_RETIRE_IMAGE;
    entry->image      = image;
    entry->view       = view;
    entry->allocation = allocation;
    entry->buffer     = VK_NULL_HANDLE;
}

void DevicePixelmapVkRetireBuffer(br_device_pixelmap_vk_frame *frame, VkBuffer buffer, VmaAllocation allocation)
{
    br_vk_retire_entry *entry = DevicePixelmapVkRetireReserve(frame);

    entry->kind       = BR_VK_RETIRE_BUFFER;
    entry->image      = VK_NULL_HANDLE;
    entry->view       = VK_NULL_HANDLE;
    entry->allocation = allocation;
    entry->buffer     = buffer;
}

/*
 * Destroy everything retired against a frame slot. The caller must have established that
 * the slot's fence has signalled, i.e. that whatever recorded those resources has finished.
 */
void DevicePixelmapVkRetireDrain(br_device_pixelmap_vk_frame *frame)
{
    br_device_pixelmap      *owner = frame->owner;
    const GladVulkanContext *vk    = &owner->device->context;

    for(br_uint_32 i = 0; i < frame->retire_count; ++i) {
        br_vk_retire_entry *entry = frame->retire_entries + i;

        switch(entry->kind) {
            case BR_VK_RETIRE_IMAGE:
                if(entry->view != VK_NULL_HANDLE)
                    vk->DestroyImageView(owner->device->vk_device, entry->view, NULL);
                if(entry->image != VK_NULL_HANDLE)
                    vmaDestroyImage(owner->device->vma_allocator, entry->image, entry->allocation);
                break;

            case BR_VK_RETIRE_BUFFER:
                if(entry->buffer != VK_NULL_HANDLE)
                    vmaDestroyBuffer(owner->device->vma_allocator, entry->buffer, entry->allocation);
                break;
        }
    }

    frame->retire_count = 0;
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

    /*
     * Everything retired against a slot is safe to destroy once that slot's fence has
     * signalled, which the wait above guarantees. The recreate path waited all fences, so
     * every slot's list can be drained; otherwise only the current one's.
     */
    if(self->as_front.need_recreate) {
        for(br_uint_32 i = 0; i < BR_VKREND_MAX_SWAPCHAIN_IMAGES; ++i)
            DevicePixelmapVkRetireDrain(self->as_front.frames + i);
    } else {
        DevicePixelmapVkRetireDrain(frame);
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

        /*
         * DeviceVkCreateSwapchain() idles the queue immediately before creating the
         * swapchain, and that idle is load-bearing beyond the swapchain: a present's wait on
         * render_finished[i] is one of its queue operations, so an idle queue proves every
         * earlier present has consumed its semaphore. That is what keeps re-signalling
         * render_finished[i] for this new swapchain's image i legal. Replace the idle with
         * per-generation present fences (VK_EXT_swapchain_maintenance1) before removing it,
         * or the re-signal can land while a present still waits on the semaphore
         * (VUID-vkQueueSubmit-pSignalSemaphores-00067).
         */
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
    VkSemaphore                  render_finished;
    VkResult                     res;

    UASSERT(self->use_type == BRT_NONE);

    frame = DevicePixelmapVkGetCurrentFrame(self);

    if((res = vk->EndCommandBuffer(frame->command_buffer)) != VK_SUCCESS) {
        BR_FATAL1("vkEndCommandBuffer() returned %" PRIu32, res);
    }

    const br_boolean skip_present = (image_index == UINT32_MAX);

    /*
     * The render-finished semaphore belongs to the acquired image, not to this frame
     * slot: the present that waits on it may outlive the submission's fence, so the
     * same semaphore cannot be signalled again from another slot until that image is
     * re-acquired.
     */
    render_finished = skip_present ? VK_NULL_HANDLE : self->as_front.render_finished[image_index];

    const VkPipelineStageFlags wm = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    /*
     * A vkAcquireNextImageKHR() that failed (VK_ERROR_OUT_OF_DATE_KHR) signals nothing, so
     * this frame is submitted without a wait: waiting on a binary semaphore that has no
     * pending signal is invalid (VUID-vkQueueSubmit-pWaitSemaphores-03238).
     */
    const VkSubmitInfo submit_info = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = NULL,
        .waitSemaphoreCount   = skip_present ? 0 : 1,
        .pWaitSemaphores      = &frame->image_acquired,
        .pWaitDstStageMask    = &wm,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &frame->command_buffer,
        .signalSemaphoreCount = skip_present ? 0 : 1,
        .pSignalSemaphores    = &render_finished,
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

    if(!skip_present) {
        const VkPresentInfoKHR present_info = {
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext              = NULL,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &render_finished,
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
    VkResult                  res;
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

    BrTokenValueSetMany(&pt, &count, NULL, tv, dev->templates.pixelmapNewTemplate, NULL);

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
    self->num_refs        = 0;

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
        br_device_pixelmap_vk_frame *frame = self->as_front.frames + i;

        DeviceVkInitFrame(frame, dev, i);

        frame->owner           = self;
        frame->retire_count    = 0;
        frame->retire_capacity = BR_VKREND_RETIRE_INITIAL_CAPACITY;
        frame->retire_entries = BrResAllocate(self, BR_VKREND_RETIRE_INITIAL_CAPACITY * sizeof(*frame->retire_entries), BR_MEMORY_OBJECT_DATA);
        if(frame->retire_entries == NULL)
            frame->retire_capacity = 0;

        self->as_front.frame_fences[i] = frame->fence;
    }

    /*
     * One render-finished semaphore per swapchain image, so its reuse is gated on that
     * image being re-acquired (see the field's comment in devpixmp.h). Four are always
     * created; a swapchain may use fewer.
     */
    {
        const VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        for(uint32_t i = 0; i < BR_VKREND_MAX_SWAPCHAIN_IMAGES; ++i) {
            self->as_front.render_finished[i] = VK_NULL_HANDLE;

            if((res = vk->CreateSemaphore(dev->vk_device, &semaphore_create_info, NULL, &self->as_front.render_finished[i]))) {
                BR_FATAL1("vkCreateSemaphore() returned %" PRIu32, res);
            }
        }
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
    dev->screen                  = self;
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

    UASSERT(self->num_refs == 0);

    /*
     * Detach from the device first: with no front screen, an object destroyed later in
     * teardown cannot retire against one and destroys directly instead, which is safe
     * because the queue is idled below. See br_device::screen.
     */
    if(self->device->screen == self)
        self->device->screen = NULL;

    if((res = vk->EndCommandBuffer(current_frame->command_buffer)) != VK_SUCCESS) {
        BR_FATAL1("vkEndCommandBuffer() returned %" PRIu32, res);
    }

    if((res = vk->QueueWaitIdle(vk_queue)) != VK_SUCCESS) {
        BR_FATAL1("vkQueueWaitIdle() returned %" PRIu32, res);
    }

    /*
     * With the queue idle every frame fence has signalled, so force-drain every slot. This
     * catches the last frame's retirements, which no following BeginFrame drains. The retire
     * arrays themselves are BRender children of this pixelmap and are released by the final
     * BrResFreeNoCallback() below.
     */
    for(uint32_t i = 0; i < BR_VKREND_MAX_SWAPCHAIN_IMAGES; ++i) {
        DevicePixelmapVkRetireDrain(self->as_front.frames + i);
    }

    /* NB: no need to wait for fences because we've just waited for the queue to finish. */

    DeviceVkDestroySwapchain(vk, vk_device, &self->as_front.swapchain);

    for(uint32_t i = 0; i < BR_VKREND_MAX_SWAPCHAIN_IMAGES; ++i) {
        if(self->as_front.render_finished[i] != VK_NULL_HANDLE) {
            vk->DestroySemaphore(vk_device, self->as_front.render_finished[i], NULL);
            self->as_front.render_finished[i] = VK_NULL_HANDLE;
        }
    }

    for(uint32_t i = 0; i < BR_VKREND_MAX_SWAPCHAIN_IMAGES; ++i) {
        br_device_pixelmap_vk_frame *frame = self->as_front.frames + i;

        vk->FreeCommandBuffers(vk_device, self->device->vk_command_pool, 1, &frame->command_buffer);
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
     * Ask for the swapchain to be rebuilt on the next frame. Waiting for
     * vkAcquireNextImageKHR() to report VK_SUBOPTIMAL_KHR is not enough: the spec does
     * not require a surface to report it, and the headless surface never does, so a
     * resize would leave the swapchain at its old size indefinitely.
     */
    self->pm_width               = width;
    self->pm_height              = height;
    self->as_front.need_recreate = BR_TRUE;
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_vk_front, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    const GladVulkanContext     *vk = &self->device->context;
    br_device_pixelmap_vk_frame *frame;
    VkResult                     res;
    uint32_t                     image_index;
    br_boolean                   skip_present = BR_FALSE;

    if(self == src)
        return BRE_OK;

    if(ObjectDevice(src) != self->device)
        return BRE_UNSUPPORTED;

    if(self->use_type != BRT_NONE || src->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    frame = DevicePixelmapVkGetCurrentFrame(self);

    res = vk->AcquireNextImageKHR(self->device->vk_device, self->as_front.swapchain.swapchain, UINT64_MAX, frame->image_acquired,
                                  VK_NULL_HANDLE, &image_index);
    switch(res) {
        case VK_SUCCESS:
            break;

        case VK_SUBOPTIMAL_KHR:
            self->as_front.need_recreate = BR_TRUE;
            break;

        case VK_ERROR_OUT_OF_DATE_KHR:
            self->as_front.need_recreate = BR_TRUE;
            skip_present                 = BR_TRUE;
            break;

        default:
            BR_FATAL1("AcquireNextImageKHR() returned %" PRIu32, res);
    }

    if(!skip_present) {
        UASSERT(image_index < self->as_front.swapchain.image_count);

        const VkImageSubresourceRange image_subresource_range = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        };

        {
            const VkImageMemoryBarrier src_barrier = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = 0,
                .dstAccessMask    = VK_ACCESS_TRANSFER_READ_BIT,
                .oldLayout        = src->as_back.layout,
                .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .image            = src->as_back.image,
                .subresourceRange = image_subresource_range,
            };
            vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
                                   NULL, 1, &src_barrier);
        }

        {
            const VkImageMemoryBarrier dst_barrier = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = 0,
                .dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
                .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .image            = self->as_front.swapchain.images[image_index],
                .subresourceRange = image_subresource_range,
            };
            vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
                                   NULL, 1, &dst_barrier);
        }

        {
            int32_t src_w = src->pm_width;
            int32_t src_h = src->pm_height;
            int32_t dst_w = (int32_t)self->as_front.swapchain.extent.width;
            int32_t dst_h = (int32_t)self->as_front.swapchain.extent.height;

            if(dst_w > 0 && dst_h > 0) {
                VkImageBlit blit = {
                    .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                    .srcOffsets     = {{0, 0, 0}, {src_w, src_h, 1}},
                    .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                    .dstOffsets     = {{0, 0, 0}, {dst_w, dst_h, 1}},
                };

                vk->CmdBlitImage(frame->command_buffer, src->as_back.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 self->as_front.swapchain.images[image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);
            }
        }

        src->as_back.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        {
            const VkImageMemoryBarrier src_restore_barrier = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = VK_ACCESS_TRANSFER_READ_BIT,
                .dstAccessMask    = 0,
                .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
                .image            = src->as_back.image,
                .subresourceRange = image_subresource_range,
            };
            vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
                                   NULL, 1, &src_restore_barrier);
        }

        src->as_back.layout = VK_IMAGE_LAYOUT_GENERAL;

        {
            const VkImageMemoryBarrier present_barrier = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask    = 0,
                .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .image            = self->as_front.swapchain.images[image_index],
                .subresourceRange = image_subresource_range,
            };
            vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL,
                                   0, NULL, 1, &present_barrier);
        }

        /*
         * Give the embedder its chance to do any last-minute work before the
         * frame is submitted and presented.
         */
        DevicePixelmapVkExtPreSwap(self);
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
