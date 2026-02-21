/*
 * Stored input buffers.
 *
 * The one buffer shape the renderer needs is a colour map: a host pixelmap uploaded once
 * into a device image that scene draws sample through a push descriptor. The upload runs at
 * update() time, on the device queue with its own one-shot command buffer, and leaves the
 * image in SHADER_READ_ONLY_OPTIMAL, where every draw expects to find it.
 *
 * An INDEX_8 map is two images: the index plane, which the format table gives the
 * unsigned-integer format R8_UINT, and the palette from the pixelmap's map, uploaded raw in
 * its own (non-indexed) format. The indexed scene shaders fetch the index through a
 * usampler2D and look the palette entry up by row.
 *
 * Only host pixelmaps are uploaded. A device pixelmap is sampled where its image lies
 * instead, through a transient view of it: the render-to-texture case, where one scene
 * writes an offscreen and another reads it as a colour map. Its image is bound in the
 * layout its pixelmap records, GENERAL - the driver's resting layout after every write
 * path. The SHADER_READ_ONLY transition the rect draw performs cannot be recorded here,
 * because the scene draw records inside a dynamic rendering scope, where
 * vkCmdPipelineBarrier is forbidden (VUID-vkCmdPipelineBarrier-None-09553). See
 * BufferStoredVkGetViewLayout().
 */
#include "drv.h"
#include "brassert.h"

/*
 * Default dispatch table for buffer stored (defined at end of file)
 */
static const struct br_buffer_stored_dispatch bufferStoredDispatch;

/*
 * Buffer stored info. template
 */
#define F(f) offsetof(struct br_buffer_stored, f)

static br_tv_template_entry bufferStoredTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR), F(identifier), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
};
#undef F

/*
 * Hand an uploaded plane's image to the front screen's current frame to be destroyed once
 * that frame's fence has signalled, or destroy it in place when there is no front screen.
 *
 * The image may still be referenced by the front screen's current command buffer, recorded
 * but not submitted, through the sampler descriptor gv1buckt.c pushes; retire it against the
 * frame that would read it rather than destroying it in place. With no front screen
 * (teardown, after the queue has been idled) direct destruction is safe.
 */
static void BufferStoredVkReleasePlane(br_device *dev, VkImage *image, VkImageView *view, VmaAllocation *allocation)
{
    const GladVulkanContext *vk     = &dev->context;
    br_device_pixelmap      *screen = dev->screen;

    if(screen != NULL) {
        if(*image != VK_NULL_HANDLE)
            DevicePixelmapVkRetireImage(DevicePixelmapVkGetCurrentFrame(screen), *image, *view, *allocation);
        else if(*view != VK_NULL_HANDLE)
            DevicePixelmapVkRetireImage(DevicePixelmapVkGetCurrentFrame(screen), VK_NULL_HANDLE, *view, NULL);
    } else {
        if(*view != VK_NULL_HANDLE)
            vk->DestroyImageView(dev->vk_device, *view, NULL);
        if(*image != VK_NULL_HANDLE)
            vmaDestroyImage(dev->vma_allocator, *image, *allocation);
    }

    *image      = VK_NULL_HANDLE;
    *view       = VK_NULL_HANDLE;
    *allocation = NULL;
}

static void BufferStoredVkDestroyImage(br_buffer_stored *self)
{
    BufferStoredVkReleasePlane(self->device, &self->image, &self->view, &self->allocation);
    BufferStoredVkReleasePlane(self->device, &self->clut_image, &self->clut_view, &self->clut_allocation);

    self->width       = 0;
    self->height      = 0;
    self->clut_width  = 0;
    self->clut_height = 0;

    self->indexed       = BR_FALSE;
    self->device_source = BR_FALSE;
    self->device_image  = VK_NULL_HANDLE;
    self->clut_fmt      = NULL;
}

/*
 * Record one image layout transition for an upload's one-shot command buffer. Every barrier
 * an upload records moves a contiguous range of mip levels between the transfer layouts or
 * into SHADER_READ_ONLY_OPTIMAL, on the single colour aspect and array layer.
 */
static void BufferStoredVkRecordTransition(const GladVulkanContext *vk, VkCommandBuffer cmd, VkImage image, VkImageLayout old_layout,
                                           VkImageLayout new_layout, VkAccessFlags src_access, VkAccessFlags dst_access,
                                           VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage, br_uint_32 base_level,
                                           br_uint_32 level_count)
{
    const VkImageMemoryBarrier barrier = {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask       = src_access,
        .dstAccessMask       = dst_access,
        .oldLayout           = old_layout,
        .newLayout           = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = {VK_IMAGE_ASPECT_COLOR_BIT, base_level, level_count, 0, 1},
    };

    vk->CmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
}

/*
 * Upload a host pixelmap's pixels raw into a new sampled image and leave it, all mip levels
 * included, in SHADER_READ_ONLY_OPTIMAL.
 *
 * The pixels are copied at the source's own stride and in the source's own format; the view
 * carries the format table's swizzle, so the sampler presents the shader with the channel
 * order it expects and no per-format shader is needed. The copy runs on a one-shot command
 * buffer submitted and waited for on the device queue, so the image is ready before any
 * draw that references it and needs no frame retire.
 *
 * When `mipmap` is set and the format can blit both ways with linear filtering, the image is
 * created with a full chain, floor(log2(max(w, h))) + 1 levels, and each lower level is the
 * VK_FILTER_LINEAR blit of the one above it. This is not glGenerateMipmap's box filter: it
 * is the closest the transfer engine offers and bit-identity is not attempted, only the
 * level count and reachability glrend has. A format that cannot blit keeps one level, and
 * the sampler's maxLod clamp then makes that behave exactly as mip_filter == BRT_NONE.
 */
static br_error BufferStoredVkUploadPlane(br_device *dev, const br_pixelmap_vk_fmt *fmt, const br_uint_8 *pixels, br_int_32 row_bytes,
                                          br_uint_32 width, br_uint_32 height, br_boolean mipmap, VkImage *out_image, VkImageView *out_view,
                                          VmaAllocation *out_allocation)
{
    const GladVulkanContext *vk = &dev->context;
    VkFormatProperties       props;
    VkBuffer                 staging;
    VmaAllocation            staging_allocation;
    VkCommandBuffer          cmd;
    VkResult                 res;

    *out_image      = VK_NULL_HANDLE;
    *out_view       = VK_NULL_HANDLE;
    *out_allocation = NULL;

    vk->GetPhysicalDeviceFormatProperties(dev->physical_device->dev, fmt->format, &props);
    if((props.optimalTilingFeatures & (VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) !=
       (VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
        return BRE_FAIL;

    /*
     * Building the chain needs the format to be a blit source and destination and, for the
     * linear blit, to support linear filtering. A format that offers none of that keeps its
     * one level rather than losing the upload; the 1-texel dimensions have nothing to build.
     */
    const VkFormatFeatureFlags mip_features = VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                                              VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    const br_boolean mipmaps = mipmap && width > 1 && height > 1 && (props.optimalTilingFeatures & mip_features) == mip_features;

    br_uint_32 levels = 1;
    for(br_uint_32 s = (width > height) ? width : height; mipmaps && s > 1; s >>= 1)
        ++levels;

    const VkImageUsageFlags image_usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                          ((levels > 1) ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0);

    const VkImageCreateInfo image_info = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .format        = fmt->format,
        .extent        = {width, height, 1},
        .mipLevels     = levels,
        .arrayLayers   = 1,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = image_usage,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    const VmaAllocationCreateInfo image_alloc_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    if((res = vmaCreateImage(dev->vma_allocator, &image_info, &image_alloc_info, out_image, out_allocation, NULL)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create a colour map image: VkResult = %d", res);
        goto fail;
    }

    const VkImageViewCreateInfo view_info = {
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image    = *out_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format   = fmt->format,
        .components =
            {
                         .r = fmt->swizzle_r,
                         .g = fmt->swizzle_g,
                         .b = fmt->swizzle_b,
                         .a = fmt->swizzle_a,
                         },
        .subresourceRange =
            {
                         .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                         .baseMipLevel   = 0,
                         .levelCount     = levels,
                         .baseArrayLayer = 0,
                         .layerCount     = 1,
                         },
    };

    if((res = vk->CreateImageView(dev->vk_device, &view_info, NULL, out_view)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create a colour map image view: VkResult = %d", res);
        goto fail;
    }

    const br_uint_32 packed_row_bytes = width * fmt->bytes_per_pixel;
    const br_uint_32 buffer_size      = packed_row_bytes * height;
    {
        const VkBufferCreateInfo buffer_info = {
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = buffer_size,
            .usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        const VmaAllocationCreateInfo buffer_alloc_info = {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        if((res = vmaCreateBuffer(dev->vma_allocator, &buffer_info, &buffer_alloc_info, &staging, &staging_allocation, NULL)) != VK_SUCCESS) {
            BrLogError("VKREND", "Failed to create a colour map staging buffer: VkResult = %d", res);
            goto fail;
        }

        VmaAllocationInfo alloc_info;
        vmaGetAllocationInfo(dev->vma_allocator, staging_allocation, &alloc_info);

        br_uint_8 *mapped = (br_uint_8 *)alloc_info.pMappedData;

        for(br_uint_32 y = 0; y < height; ++y)
            BrMemCpy(mapped + y * packed_row_bytes, pixels + y * row_bytes, packed_row_bytes);

        vmaFlushAllocation(dev->vma_allocator, staging_allocation, 0, buffer_size);
    }

    {
        const VkCommandBufferAllocateInfo alloc_info = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = dev->vk_command_pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        if((res = vk->AllocateCommandBuffers(dev->vk_device, &alloc_info, &cmd)) != VK_SUCCESS) {
            BrLogError("VKREND", "Failed to allocate the colour map upload command buffer: VkResult = %d", res);
            vmaDestroyBuffer(dev->vma_allocator, staging, staging_allocation);
            goto fail;
        }

        const VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        vk->BeginCommandBuffer(cmd, &begin_info);

        /*
         * Every level starts in TRANSFER_DST_OPTIMAL, so a blit's destination is already in
         * the layout it needs. Level 0 is copied from the staging buffer; each lower level
         * is the linear blit of the one above it. A source level moves to
         * TRANSFER_SRC_OPTIMAL for its blit and never moves back, so after the loop levels
         * 0..levels-2 are transfer sources and the last level is still a transfer
         * destination; each is then moved to SHADER_READ_ONLY_OPTIMAL for the draws.
         */
        BufferStoredVkRecordTransition(vk, cmd, *out_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0,
                                       VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, levels);

        const VkBufferImageCopy copy_region = {
            .bufferOffset      = 0,
            .bufferRowLength   = 0,
            .bufferImageHeight = 0,
            .imageSubresource  = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .imageOffset       = {0, 0, 0},
            .imageExtent       = {width, height, 1},
        };
        vk->CmdCopyBufferToImage(cmd, staging, *out_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

        for(br_uint_32 level = 1; level < levels; ++level) {
            const br_uint_32 src_w = (width >> (level - 1)) ? (width >> (level - 1)) : 1;
            const br_uint_32 src_h = (height >> (level - 1)) ? (height >> (level - 1)) : 1;
            const br_uint_32 dst_w = (width >> level) ? (width >> level) : 1;
            const br_uint_32 dst_h = (height >> level) ? (height >> level) : 1;

            BufferStoredVkRecordTransition(vk, cmd, *out_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT, level - 1, 1);

            const VkImageBlit blit = {
                .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 0, 1},
                .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1},
                .srcOffsets     = {{0, 0, 0}, {(br_int_32)src_w, (br_int_32)src_h, 1}},
                .dstOffsets     = {{0, 0, 0}, {(br_int_32)dst_w, (br_int_32)dst_h, 1}},
            };
            vk->CmdBlitImage(cmd, *out_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *out_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &blit, VK_FILTER_LINEAR);
        }

        if(levels > 1) {
            BufferStoredVkRecordTransition(vk, cmd, *out_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, levels - 1);
            BufferStoredVkRecordTransition(vk, cmd, *out_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, levels - 1, 1);
        } else {
            BufferStoredVkRecordTransition(vk, cmd, *out_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1);
        }

        vk->EndCommandBuffer(cmd);

        const VkSubmitInfo submit_info = {
            .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers    = &cmd,
        };
        vk->QueueSubmit(dev->vk_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk->QueueWaitIdle(dev->vk_queue);

        vk->FreeCommandBuffers(dev->vk_device, dev->vk_command_pool, 1, &cmd);
        vmaDestroyBuffer(dev->vma_allocator, staging, staging_allocation);
    }

    return BRE_OK;

fail:
    BufferStoredVkReleasePlane(dev, out_image, out_view, out_allocation);
    return BRE_FAIL;
}

/*
 * Upload a host pixelmap into the buffer's images.
 *
 * A colour map is one image. An INDEX_8 map is two: the index plane in the unsigned-integer
 * format the format table gives INDEX_8, and the palette in the pixelmap's own non-indexed
 * format, uploaded raw with its own swizzle exactly as a plain source image is. The palette
 * is addressed the way BrPixelmapPixelGet(map, 0, index) addresses it - column 0, row
 * `index` - which the indexed shaders' texelFetch(clut, ivec2(0, index), 0) matches.
 */
static br_error BufferStoredVkUpdateImage(br_buffer_stored *self, br_pixelmap *pm)
{
    const br_pixelmap *clut;

    if((pm->flags & BR_PMF_NO_ACCESS) || pm->pixels == NULL)
        return BRE_FAIL;

    if((self->fmt = DeviceVkGetFormatDetails(pm->type)) == NULL || self->fmt->format == VK_FORMAT_UNDEFINED)
        return BRE_FAIL;

    if(pm->width <= 0 || pm->height <= 0)
        return BRE_FAIL;

    self->indexed = self->fmt->indexed;
    clut          = NULL;

    if(self->indexed) {
        if(pm->map == NULL || pm->map->pixels == NULL || (pm->map->flags & BR_PMF_NO_ACCESS))
            return BRE_FAIL;

        if(pm->map->width <= 0 || pm->map->height <= 0)
            return BRE_FAIL;

        if((self->clut_fmt = DeviceVkGetFormatDetails(pm->map->type)) == NULL || self->clut_fmt->indexed || self->clut_fmt->format == VK_FORMAT_UNDEFINED)
            return BRE_FAIL;

        clut = pm->map;
    }

    /*
     * A non-indexed colour map gets a mip chain, as glGenerateMipmap gives it in glrend; the
     * index plane and the palette do not - an INDEX_8 map is a lookup whose index plane the
     * sample clamps to level 0, exactly the mip_filter == BRT_NONE behaviour.
     */
    if(BufferStoredVkUploadPlane(self->device, self->fmt, (const br_uint_8 *)pm->pixels, pm->row_bytes, (br_uint_32)pm->width,
                                 (br_uint_32)pm->height, !self->indexed, &self->image, &self->view, &self->allocation) != BRE_OK) {
        BufferStoredVkDestroyImage(self);
        return BRE_FAIL;
    }

    if(clut != NULL) {
        if(BufferStoredVkUploadPlane(self->device, self->clut_fmt, (const br_uint_8 *)clut->pixels, clut->row_bytes, (br_uint_32)clut->width,
                                     (br_uint_32)clut->height, BR_FALSE, &self->clut_image, &self->clut_view, &self->clut_allocation) != BRE_OK) {
            BufferStoredVkDestroyImage(self);
            return BRE_FAIL;
        }

        self->clut_width  = (br_uint_32)clut->width;
        self->clut_height = (br_uint_32)clut->height;
    }

    self->source       = pm;
    self->source_flags = pm->flags;
    self->width        = (br_uint_32)pm->width;
    self->height       = (br_uint_32)pm->height;
    return BRE_OK;
}

/*
 * Accept a device pixelmap as the colour map and build a transient sampled view of its
 * image carrying the format table's swizzle.
 *
 * The image is sampled where it lies rather than uploaded: its resting layout is GENERAL,
 * which a shader may sample from, so no layout transition is recorded here. The image must
 * be an offscreen image in a non-indexed colour format the device can sample; the pixelmap
 * create path gives the image VK_IMAGE_USAGE_SAMPLED_BIT only when the format reports the
 * sampled-image feature, so that feature is the gate. The view is owned by this stored
 * buffer and retired with it.
 *
 * The source may be a sub-pixelmap. A sub's as_sub member aliases as_back, so the image
 * sampled is its owner's, the whole image the sub windows into; the owner is resolved the
 * same way the renderer resolves its destination.
 */
static br_error BufferStoredVkUpdateDevice(br_buffer_stored *self, br_device_pixelmap *pm)
{
    br_device                *dev = self->device;
    const GladVulkanContext  *vk  = &dev->context;
    const br_pixelmap_vk_fmt *fmt;
    br_device_pixelmap       *owner;
    VkFormatProperties        props;
    VkResult                  res;

    if((fmt = DeviceVkGetFormatDetails(pm->pm_type)) == NULL || fmt->format == VK_FORMAT_UNDEFINED || fmt->indexed || fmt->depth)
        return BRE_FAIL;

    if(!DevicePixelmapVkIsOffscreen(pm) || (owner = DevicePixelmapVkResolve(pm)) == NULL || owner->as_back.image == VK_NULL_HANDLE)
        return BRE_FAIL;

    vk->GetPhysicalDeviceFormatProperties(dev->physical_device->dev, fmt->format, &props);
    if(!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
        return BRE_FAIL;

    const VkImageViewCreateInfo view_info = {
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image    = owner->as_back.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format   = fmt->format,
        .components =
            {
                         .r = fmt->swizzle_r,
                         .g = fmt->swizzle_g,
                         .b = fmt->swizzle_b,
                         .a = fmt->swizzle_a,
                         },
        .subresourceRange =
            {
                         .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                         .baseMipLevel   = 0,
                         .levelCount     = 1,
                         .baseArrayLayer = 0,
                         .layerCount     = 1,
                         },
    };

    if((res = vk->CreateImageView(dev->vk_device, &view_info, NULL, &self->view)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the device colour map view: VkResult = %d", res);
        return BRE_FAIL;
    }

    self->fmt           = fmt;
    self->device_source = BR_TRUE;
    self->device_image  = owner->as_back.image;
    self->source        = (br_pixelmap *)pm;
    self->source_flags  = pm->pm_flags;
    self->width         = (br_uint_32)owner->pm_width;
    self->height        = (br_uint_32)owner->pm_height;
    return BRE_OK;
}

/*
 * Replace the buffer's images from `pm`: a host pixelmap is uploaded, a same-device device
 * pixelmap is sampled where it lies. The old images are retired, not destroyed - a map
 * updated mid-frame could still be referenced by the front screen's unsubmitted command
 * buffer, so they are handed to the frame that would read them (see
 * BufferStoredVkReleasePlane). A pixelmap from another device cannot be used here.
 */
static br_error BufferStoredVkUpdate(br_buffer_stored *self, br_device_pixelmap *pm)
{
    BufferStoredVkDestroyImage(self);

    if(ObjectDevice(pm) != NULL) {
        if(ObjectDevice(pm) != self->device)
            return BRE_FAIL;

        return BufferStoredVkUpdateDevice(self, pm);
    }

    return BufferStoredVkUpdateImage(self, (br_pixelmap *)pm);
}

static br_error BR_CMETHOD_DECL(br_buffer_stored_vk, update)(struct br_buffer_stored *self, struct br_device_pixelmap *pm, br_token_value *tv)
{
    (void)tv;

    return BufferStoredVkUpdate(self, pm);
}

/*
 * Set up a stored buffer. The use token picks the buffer's role; only a colour map is
 * implemented, so every other role fails without an object.
 */
struct br_buffer_stored *BufferStoredVkAllocate(br_renderer *renderer, br_token use, struct br_device_pixelmap *pm, br_token_value *tv)
{
    struct br_buffer_stored *self;
    const char              *ident;

    switch(use) {
        case BRT_TEXTURE_O:
        case BRT_COLOUR_MAP_O:
            ident = "colour_map";
            break;

        default:
            return NULL;
    }

    self                  = BrResAllocate(renderer, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch        = &bufferStoredDispatch;
    self->identifier      = BrResSprintf(self, "%s:%s", ident, pm->pm_identifier ? pm->pm_identifier : "(unnamed)");
    self->device          = ObjectDevice(renderer);
    self->renderer        = renderer;
    self->source          = NULL;
    self->fmt             = NULL;
    self->indexed         = BR_FALSE;
    self->image           = VK_NULL_HANDLE;
    self->view            = VK_NULL_HANDLE;
    self->allocation      = NULL;
    self->width           = 0;
    self->height          = 0;
    self->clut_fmt        = NULL;
    self->clut_image      = VK_NULL_HANDLE;
    self->clut_view       = VK_NULL_HANDLE;
    self->clut_allocation = NULL;
    self->clut_width      = 0;
    self->clut_height     = 0;
    self->device_source   = BR_FALSE;
    self->device_image    = VK_NULL_HANDLE;
    self->templates = BrTVTemplateAllocate(self, (br_tv_template_entry *)bufferStoredTemplateEntries, BR_ASIZE(bufferStoredTemplateEntries));

    if(BufferStoredVkUpdate(self, pm) != BRE_OK) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    ObjectContainerAddFront(renderer, (br_object *)self);
    return self;
}

/*
 * The image view a scene draw binds as the colour map, or VK_NULL_HANDLE if this buffer
 * holds no upload. For an INDEX_8 source this is the index plane (the palette is
 * BufferStoredVkGetClutView()); for a device source it is a transient view of the source
 * image carrying the format table's swizzle.
 */
VkImageView BufferStoredVkGetView(const struct br_buffer_stored *self)
{
    if(self == NULL)
        return VK_NULL_HANDLE;

    return self->view;
}

/*
 * The palette view an indexed scene draw binds alongside the index plane, or
 * VK_NULL_HANDLE when the source is not indexed.
 */
VkImageView BufferStoredVkGetClutView(const struct br_buffer_stored *self)
{
    if(self == NULL)
        return VK_NULL_HANDLE;

    return self->clut_view;
}

/*
 * The layout the colour map's image is in when a draw binds it.
 *
 * A host upload rests in SHADER_READ_ONLY_OPTIMAL. A device source is sampled where it
 * lies, in the layout its pixelmap records: GENERAL, the driver's resting layout after
 * every write path (the rect draw and sceneEnd restore it, fills and copies leave it), and
 * a shader may sample from it. The scene draw records inside a dynamic rendering scope,
 * where the SHADER_READ_ONLY transition the rect draw performs is forbidden
 * (VUID-vkCmdPipelineBarrier-None-09553), so there is nowhere to record one. Any other
 * layout is not one this draw can sample, so VK_IMAGE_LAYOUT_UNDEFINED is returned and the
 * caller declines the draw rather than naming a wrong layout. A device image that has been
 * replaced (an in-place resize) is likewise declined until the buffer is updated.
 */
VkImageLayout BufferStoredVkGetViewLayout(const struct br_buffer_stored *self)
{
    const br_device_pixelmap *owner;

    if(self == NULL)
        return VK_IMAGE_LAYOUT_UNDEFINED;

    if(!self->device_source)
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    /*
     * The source may be a sub-pixelmap, whose as_back aliases as_sub; the image and layout
     * are the owner's, and device_image was recorded from the owner too.
     */
    if(self->source == NULL)
        return VK_IMAGE_LAYOUT_UNDEFINED;

    owner = DevicePixelmapVkResolve((const br_device_pixelmap *)self->source);

    if(owner == NULL || owner->as_back.image != self->device_image)
        return VK_IMAGE_LAYOUT_UNDEFINED;

    if(owner->as_back.layout == VK_IMAGE_LAYOUT_GENERAL || owner->as_back.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        return owner->as_back.layout;

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

/*
 * Whether the colour map's source was INDEX_8, i.e. whether the draw must bind a palette
 * and use the indexed shader.
 */
br_boolean BufferStoredVkIsIndexed(const struct br_buffer_stored *self)
{
    return self != NULL && self->indexed;
}

static void BR_CMETHOD_DECL(br_buffer_stored_vk, free)(br_object *_self)
{
    br_buffer_stored *self = (br_buffer_stored *)_self;

    BufferStoredVkDestroyImage(self);

    ObjectContainerRemove(self->renderer, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_buffer_stored_vk, identifier)(br_object *self)
{
    return ((br_buffer_stored *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_buffer_stored_vk, type)(br_object *self)
{
    (void)self;
    return BRT_BUFFER_STORED;
}

static br_boolean BR_CMETHOD_DECL(br_buffer_stored_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_BUFFER_STORED) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_buffer_stored_vk, device)(br_object *self)
{
    return ((br_buffer_stored *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_buffer_stored_vk, space)(br_object *self)
{
    (void)self;
    return BrResSizeTotal(self);
}

static br_tv_template *BR_CMETHOD_DECL(br_buffer_stored_vk, templateQuery)(br_object *_self)
{
    return ((br_buffer_stored *)_self)->templates;
}

/*
 * Default dispatch table for buffer stored
 */
static const struct br_buffer_stored_dispatch bufferStoredDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_buffer_stored_vk, free),
    ._identifier = BR_CMETHOD_REF(br_buffer_stored_vk, identifier),
    ._type       = BR_CMETHOD_REF(br_buffer_stored_vk, type),
    ._isType     = BR_CMETHOD_REF(br_buffer_stored_vk, isType),
    ._device     = BR_CMETHOD_REF(br_buffer_stored_vk, device),
    ._space      = BR_CMETHOD_REF(br_buffer_stored_vk, space),

    ._templateQuery = BR_CMETHOD_REF(br_buffer_stored_vk, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._update = BR_CMETHOD_REF(br_buffer_stored_vk, update),
};
