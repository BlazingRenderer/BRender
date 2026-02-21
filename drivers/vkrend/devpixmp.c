/*
 * Device pixelmap methods
 */

#include <inttypes.h>
#include <string.h>
#include "drv.h"
#include "brassert.h"

static const struct br_device_pixelmap_dispatch devicePixelmapDispatch;

static br_error custom_query(br_value *pvalue, void **extra, br_size_t *pextra_size, void *block, const struct br_tv_template_entry *tep,
                             void *context)
{
    const br_device_pixelmap *self = block;

    if(tep->token == BRT_CLUT_O) {
        if(self->use_type == BRT_OFFSCREEN)
            pvalue->o = (br_object *)self->as_back.clut;
        else
            pvalue->o = NULL;

        return BRE_OK;
    }

    return BRE_UNKNOWN;
}

static const br_tv_custom custom = {
    .query      = custom_query,
    .set        = NULL,
    .extra_size = NULL,
};

#define F(f) offsetof(struct br_device_pixelmap, f)
static br_tv_template_entry devicePixelmapTemplateEntries[] = {
    {BRT(WIDTH_I32),         F(pm_width),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(HEIGHT_I32),        F(pm_height),       BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U16, 0                    },
    {BRT(PIXEL_TYPE_U8),     F(pm_type),         BRTV_QUERY | BRTV_ALL, BRTV_CONV_I32_U8,  0                    },
    {BRT(OUTPUT_FACILITY_O), F(output_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
    {BRT(FACILITY_O),        F(output_facility), BRTV_QUERY,            BRTV_CONV_COPY,    0                    },
    {BRT(CLUT_O),            0,                  BRTV_QUERY | BRTV_ALL, BRTV_CONV_CUSTOM,  (br_uintptr_t)&custom},
    {BRT(IDENTIFIER_CSTR),   F(pm_identifier),   BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY,    0                    },
};
#undef F

/*
 * The offscreen that owns a pixelmap's image, or NULL if the pixelmap is neither a plain
 * vkrend offscreen nor a sub of one.
 *
 * A sub-pixelmap inherits its parent's use_type and copies its fields, but its as_sub
 * member aliases as_back - as_back.image would hold the owner's br_device_pixelmap *, not a
 * VkImage. So a sub's own as_back must never be read: every site that needs the image
 * resolves the owner here and uses the owner's as_back (image/view/layout).
 *
 * devicePixelmapSubDispatch is file-local to devpmsub.c, so DevicePixelmapVkIsSub() is the
 * discriminator rather than a duplicated dispatch comparison. A renderer created directly on
 * a sub is accepted too (RendererVkAllocate()); it is over this owner that the renderer
 * opens its scope.
 */
br_device_pixelmap *DevicePixelmapVkResolve(const br_device_pixelmap *pm)
{
    if(pm->dispatch == &devicePixelmapDispatch)
        return (br_device_pixelmap *)pm;

    if(DevicePixelmapVkIsSub(pm))
        return pm->as_sub.parent;

    return NULL;
}

/*
 * Whether a pixelmap may be the destination of a vkrend renderer: whether it resolves to a
 * plain vkrend offscreen. A sub-pixelmap qualifies because it resolves to its owner, whose
 * image is rendered into with the viewport and scissor set to the sub's rect.
 */
br_boolean DevicePixelmapVkIsOffscreen(const br_device_pixelmap *pm)
{
    br_device_pixelmap *owner = DevicePixelmapVkResolve(pm);

    return owner != NULL && owner->use_type == BRT_OFFSCREEN;
}

/*
 * Whether a pixelmap is a vkrend depth buffer. The renderer attaches one only through its
 * DEPTH aspect, so a sub or a foreign pixelmap carrying the token is rejected by the
 * dispatch check the same way a sub is rejected as a colour destination.
 */
br_boolean DevicePixelmapVkIsDepth(const br_device_pixelmap *pm)
{
    return pm->use_type == BRT_DEPTH && pm->dispatch == &devicePixelmapDispatch;
}

static void DevicePixelmapVkDestroyImage(br_device_pixelmap *pm);

static br_error DevicePixelmapVkCreateImage(br_device *dev, br_device_pixelmap *pm, br_uint_8 pm_type, br_int_32 width, br_int_32 height)
{
    const GladVulkanContext  *vk = &dev->context;
    const br_pixelmap_vk_fmt *fmt;
    VkResult                  res;

    fmt = DeviceVkGetFormatDetails(pm_type);
    if(fmt == NULL || fmt->format == VK_FORMAT_UNDEFINED)
        return BRE_FAIL;

    /*
     * An indexed pixelmap is a single-channel R8_UINT image holding CLUT indices; the
     * palette lives in the pixelmap's map, not in the image, so nothing format-specific
     * happens here. Only the plain colour formats can be rendered into (see the rect
     * draw's indexed-destination check), so an indexed image is filled and copied through
     * the transfer engine. A depth pixelmap is a distinct kind again: it is a depth image,
     * viewed and barriered with the DEPTH aspect and attached as a depth-stencil target.
     */
    VkFormatProperties format_props;
    dev->context.GetPhysicalDeviceFormatProperties(dev->physical_device->dev, fmt->format, &format_props);

    const VkImageAspectFlags aspect = fmt->depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    /*
     * The usage has to come from the format's capabilities, not be assumed. B8G8R8_UNORM
     * (BR_PMT_RGB_888) in particular supports neither transfer nor storage with optimal
     * tiling on RADV, and vkCreateImage() will hand back an image that is then invalid to
     * copy into - so take only the bits the device reports.
     *
     * For a colour image TRANSFER_DST is non-negotiable: without it the image cannot be
     * filled or copied into at all. TRANSFER_SRC is what presents an offscreen or blits it,
     * COLOR_ATTACHMENT is the rect draw's render target, and SAMPLED is what lets another
     * offscreen's rect draw sample this one; each is taken when offered, and its absence
     * just routes that work elsewhere. A depth image's one non-negotiable capability is
     * being a depth-stencil attachment; the transfer and sampled bits are taken when the
     * device offers them and are not otherwise required.
     */
    VkImageUsageFlags usage = 0;

    if(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT)
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if(fmt->depth) {
        if(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        if(!(usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            BrLogError("VKREND", "Pixelmap format %d cannot be a depth attachment with optimal tiling on this device", pm_type);
            return BRE_FAIL;
        }
    } else {
        if(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if(!(usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
            BrLogError("VKREND", "Pixelmap format %d cannot be a transfer destination with optimal tiling on this device", pm_type);
            return BRE_FAIL;
        }
    }

    if(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo image_info = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .format        = fmt->format,
        .extent        = {(uint32_t)width, (uint32_t)height, 1},
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = usage,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo alloc_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    if((res = vmaCreateImage(dev->vma_allocator, &image_info, &alloc_info, &pm->as_back.image, &pm->as_back.allocation, NULL)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create image: VkResult = %d", res);
        return BRE_FAIL;
    }

    VkImageViewCreateInfo view_info = {
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image    = pm->as_back.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format   = fmt->format,
        .subresourceRange =
            {
                               .aspectMask     = aspect,
                               .baseMipLevel   = 0,
                               .levelCount     = 1,
                               .baseArrayLayer = 0,
                               .layerCount     = 1,
                               },
    };

    if((res = vk->CreateImageView(dev->vk_device, &view_info, NULL, &pm->as_back.view)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create image view: VkResult = %d", res);
        vmaDestroyImage(dev->vma_allocator, pm->as_back.image, pm->as_back.allocation);
        pm->as_back.image      = VK_NULL_HANDLE;
        pm->as_back.allocation = NULL;
        return BRE_FAIL;
    }

    pm->as_back.layout = VK_IMAGE_LAYOUT_UNDEFINED;

    {
        VkCommandBuffer                   cmd;
        const VkCommandBufferAllocateInfo alloc_info = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = dev->vk_command_pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        if((res = vk->AllocateCommandBuffers(dev->vk_device, &alloc_info, &cmd)) != VK_SUCCESS) {
            DevicePixelmapVkDestroyImage(pm);
            BrResFreeNoCallback(pm);
            return BRE_FAIL;
        }

        const VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        vk->BeginCommandBuffer(cmd, &begin_info);

        VkImageSubresourceRange range = {
            .aspectMask = aspect,
            .levelCount = 1,
            .layerCount = 1,
        };
        /*
         * The resting layout is GENERAL for both kinds. A depth image is barriered into
         * DEPTH_STENCIL_ATTACHMENT_OPTIMAL for the scene and back to GENERAL afterwards,
         * the same way the colour image is.
         */
        VkAccessFlags initial_access = fmt->depth ? (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
                                                  : (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
        VkImageMemoryBarrier barrier = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = 0,
            .dstAccessMask    = initial_access,
            .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
            .image            = pm->as_back.image,
            .subresourceRange = range,
        };
        vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        vk->EndCommandBuffer(cmd);

        const VkSubmitInfo submit_info = {
            .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers    = &cmd,
        };
        vk->QueueSubmit(dev->vk_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk->QueueWaitIdle(dev->vk_queue);

        vk->FreeCommandBuffers(dev->vk_device, dev->vk_command_pool, 1, &cmd);
    }

    pm->as_back.layout = VK_IMAGE_LAYOUT_GENERAL;
    return BRE_OK;
}

/*
 * Retire an offscreen pixelmap's image onto the front screen's current frame slot, to be
 * destroyed once that slot's fence has signalled.
 *
 * The image may still be referenced by the command buffer being recorded (and not yet
 * submitted), so it cannot be freed here - the old vkQueueWaitIdle() could not cover that
 * case and global-stalled the queue besides. DevicePixelmapVkRetireDrain() destroys it at
 * the start of the next use of the slot. The handles are cleared so nothing reuses them.
 */
static void DevicePixelmapVkDestroyImage(br_device_pixelmap *pm)
{
    if(pm->as_back.image == VK_NULL_HANDLE && pm->as_back.view == VK_NULL_HANDLE)
        return;

    DevicePixelmapVkRetireImage(DevicePixelmapVkGetCurrentFrame(pm->screen), pm->as_back.image, pm->as_back.view, pm->as_back.allocation);

    pm->as_back.image      = VK_NULL_HANDLE;
    pm->as_back.view       = VK_NULL_HANDLE;
    pm->as_back.allocation = NULL;
}

void DevicePixelmapVkIncRef(br_device_pixelmap *self)
{
    UASSERT(self->num_refs >= 0);
    ++self->num_refs;
}

void DevicePixelmapVkDecRef(br_device_pixelmap *self)
{
    UASSERT(self->num_refs > 0);
    --self->num_refs;
}

static void BR_CMETHOD_DECL(br_device_pixelmap_vk, free)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    UASSERT(self->num_refs == 0);

    if(self->use_type == BRT_OFFSCREEN || self->use_type == BRT_DEPTH) {
        DevicePixelmapVkDecRef(self->screen);
        DevicePixelmapVkDestroyImage(self);
    }

    ObjectContainerRemove(self->output_facility, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_pixelmap_vk, identifier)(br_object *self)
{
    return ((br_device_pixelmap *)self)->pm_identifier;
}

static br_token BR_CMETHOD_DECL(br_device_pixelmap_vk, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE_PIXELMAP;
}

static br_boolean BR_CMETHOD_DECL(br_device_pixelmap_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE_PIXELMAP) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_pixelmap_vk, device)(br_object *self)
{
    (void)self;
    return ((br_device_pixelmap *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_device_pixelmap_vk, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device_pixelmap);
}

static br_tv_template *BR_CMETHOD_DECL(br_device_pixelmap_vk, templateQuery)(br_object *_self)
{
    br_device_pixelmap *self = (br_device_pixelmap *)_self;

    if(self->device->templates.devicePixelmapTemplate == NULL) {
        self->device->templates.devicePixelmapTemplate = BrTVTemplateAllocate(self->device, devicePixelmapTemplateEntries,
                                                                              BR_ASIZE(devicePixelmapTemplateEntries));
    }

    return self->device->templates.devicePixelmapTemplate;
}

/*
 * Resize an offscreen or depth pixelmap in place.
 *
 * Both kinds are a single VkImage viewed and barriered with their own aspect, so the same
 * destroy-and-recreate serves them: DevicePixelmapVkCreateImage() keeps the pm_type's
 * aspect and usage and leaves as_back.layout at GENERAL, which is the resting layout every
 * other path assumes. This is what lets BrPixelmapResizeBuffersTV() take its in-place
 * branch; when a depth pixelmap's resize is rejected the fallback frees the colour as well
 * as the depth, presents the half-built buffer and recreates both - and the renderer and
 * its application outlive that teardown holding the freed colour.
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, resize)(br_device_pixelmap *self, br_int_32 width, br_int_32 height)
{
    if(self->use_type != BRT_OFFSCREEN && self->use_type != BRT_DEPTH)
        return BRE_UNSUPPORTED;

    DevicePixelmapVkDestroyImage(self);

    if(DevicePixelmapVkCreateImage(self->device, self, self->pm_type, width, height) != BRE_OK)
        return BRE_FAIL;

    self->pm_width  = (br_uint_16)width;
    self->pm_height = (br_uint_16)height;
    return BRE_OK;
}

struct pixelmapMatchTokens {
    br_int_32 width;
    br_int_32 height;
    br_int_32 pixel_bits;
    br_uint_8 type;
    br_token  use_type;
};

#define F(f) offsetof(struct pixelmapMatchTokens, f)
static br_tv_template_entry pixelmapMatchTemplateEntries[] = {
    {BRT_WIDTH_I32,      NULL, F(width),      BRTV_SET, BRTV_CONV_COPY},
    {BRT_HEIGHT_I32,     NULL, F(height),     BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_BITS_I32, NULL, F(pixel_bits), BRTV_SET, BRTV_CONV_COPY},
    {BRT_PIXEL_TYPE_U8,  NULL, F(type),       BRTV_SET, BRTV_CONV_COPY},
    {BRT_USE_T,          NULL, F(use_type),   BRTV_SET, BRTV_CONV_COPY},
};
#undef F

br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv)
{
    br_int_32                  count;
    br_error                   err;
    br_device_pixelmap        *pm;
    struct pixelmapMatchTokens mt = {
        .width      = self->pm_width,
        .height     = self->pm_height,
        .pixel_bits = -1,
        .type       = BR_PMT_MAX,
        .use_type   = BRT_NONE,
    };

    if(self->device->templates.pixelmapMatchTemplate == NULL) {
        self->device->templates.pixelmapMatchTemplate = BrTVTemplateAllocate(self->device, pixelmapMatchTemplateEntries,
                                                                             BR_ASIZE(pixelmapMatchTemplateEntries));
    }

    err = BrTokenValueSetMany(&mt, &count, NULL, tv, self->device->templates.pixelmapMatchTemplate, NULL);
    if(err != BRE_OK)
        return err;

    if(mt.use_type == BRT_NO_RENDER)
        mt.use_type = BRT_OFFSCREEN;

    /*
     * A depth pixelmap is matched from the offscreen it belongs to, never from the front
     * screen, and its type comes from the requested bits rather than the source format -
     * the front screen's format is a colour format, not a depth one. glrend makes the same
     * match: 16 bits is BR_PMT_DEPTH_16, the default (and 32) is BR_PMT_DEPTH_FP32, and 24
     * is the packed BR_PMT_DEPTH_24.
     */
    if(mt.use_type == BRT_DEPTH) {
        if(self->use_type != BRT_OFFSCREEN)
            return BRE_UNSUPPORTED;

        switch(mt.pixel_bits) {
            case 16:
                mt.type = BR_PMT_DEPTH_16;
                break;

            case 24:
                mt.type = BR_PMT_DEPTH_24;
                break;

            case -1:
            case 32:
                mt.type = BR_PMT_DEPTH_FP32;
                break;

            default:
                return BRE_UNSUPPORTED;
        }
    } else if(mt.use_type != BRT_OFFSCREEN) {
        return BRE_UNSUPPORTED;
    }

    if(self->use_type == BRT_NONE && mt.use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if(mt.type == BR_PMT_MAX)
        mt.type = self->pm_type;

    if(DeviceVkGetFormatDetails(mt.type) == NULL)
        return BRE_FAIL;

    pm                  = BrResAllocate(self->device, sizeof(br_device_pixelmap), BR_MEMORY_OBJECT);
    pm->dispatch        = &devicePixelmapDispatch;
    pm->device          = self->device;
    pm->output_facility = self->output_facility;
    pm->use_type        = mt.use_type;
    pm->screen          = self->screen;
    DevicePixelmapVkIncRef(self->screen);

    pm->pm_type     = mt.type;
    pm->pm_width    = mt.width;
    pm->pm_height   = mt.height;
    pm->pm_flags    = BR_PMF_NO_ACCESS;
    pm->pm_origin_x = 0;
    pm->pm_origin_y = 0;
    pm->pm_base_x   = 0;
    pm->pm_base_y   = 0;

    pm->as_back.image      = VK_NULL_HANDLE;
    pm->as_back.view       = VK_NULL_HANDLE;
    pm->as_back.allocation = NULL;
    pm->as_back.layout     = VK_IMAGE_LAYOUT_UNDEFINED;
    pm->as_back.clut       = NULL;
    pm->num_refs           = 0;

    pm->pm_origin_x = self->pm_origin_x;
    pm->pm_origin_y = self->pm_origin_y;

    /*
     * An offscreen owns a palette even when its own type is not indexed: an indexed
     * source copied into it resolves through the destination's CLUT, which is what
     * lets a colour buffer hold the palette for a source that has none. It is a child
     * of pm, so free() needs no explicit release, and DevicePixelmapVkCreateImage()
     * below neither touches nor replaces it, so a resize keeps it. A depth pixelmap
     * never carries one.
     */
    if(mt.use_type == BRT_OFFSCREEN)
        pm->as_back.clut = DeviceClutVkAllocate(pm);

    if(DevicePixelmapVkCreateImage(self->device, pm, mt.type, mt.width, mt.height) != BRE_OK) {
        DevicePixelmapVkDecRef(self->screen);
        BrResFreeNoCallback(pm);
        return BRE_FAIL;
    }

    *newpm = pm;
    ObjectContainerAddFront(self->output_facility, (br_object *)pm);
    return BRE_OK;
}

/*
 * Can vkCmdBlitImage serve a device-to-device copy between these two pixelmap types?
 *
 * A blit only represents the copy when both sides are the same image format and the
 * device reports the blit features it needs. Everything else - an INDEX_8 source into a
 * colour destination, whose indices have to be resolved through the palette, or any other
 * mismatched pair - is the rect draw's work.
 */
static br_boolean DevicePixelmapVkBlitSupported(br_device *dev, br_uint_8 dst_type, br_uint_8 src_type)
{
    const br_pixelmap_vk_fmt *src_fmt = DeviceVkGetFormatDetails(src_type);
    const br_pixelmap_vk_fmt *dst_fmt = DeviceVkGetFormatDetails(dst_type);
    VkFormatProperties        src_props, dst_props;

    if(src_fmt == NULL || dst_fmt == NULL)
        return BR_FALSE;

    if(src_fmt->format != dst_fmt->format)
        return BR_FALSE;

    dev->context.GetPhysicalDeviceFormatProperties(dev->physical_device->dev, src_fmt->format, &src_props);
    dev->context.GetPhysicalDeviceFormatProperties(dev->physical_device->dev, dst_fmt->format, &dst_props);

    if(!(src_props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
        return BR_FALSE;

    if(!(dst_props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
        return BR_FALSE;

    return BR_TRUE;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleStretchCopy)(br_device_pixelmap *self, br_rectangle *d,
                                                                             br_device_pixelmap *src, br_rectangle *s)
{
    const GladVulkanContext     *vk = &self->device->context;
    br_device_pixelmap_vk_frame *frame;
    br_rectangle                 srect, drect;
    br_device_pixelmap          *src_owner;
    br_int_32                    src_base_x, src_base_y;

    if(self->use_type != BRT_OFFSCREEN || src->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if(self == src)
        return BRE_OK;

    /*
     * Both rectangles are clipped to their pixelmaps first, as glrend's rectangleStretchCopy
     * and rectangleStretchCopyTo() below do. An offscreen inherits its parent's origin, so an
     * unclipped blit would carry that origin into the image offsets, which the validation
     * layer rejects.
     */
    if(PixelmapRectangleClip(&srect, s, (br_pixelmap *)src) == BR_CLIP_REJECT)
        return BRE_OK;

    if(PixelmapRectangleClip(&drect, d, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    /*
     * vkCmdBlitImage serves a same-format pair the device can blit; everything else falls
     * to the rect draw, which samples the source (host memory or a device image) and
     * writes the destination's format. That is the only path for a mismatched pair or an
     * INDEX_8 source, whose palette a raw blit cannot resolve.
     */
    if(!DevicePixelmapVkBlitSupported(self->device, self->pm_type, src->pm_type))
        return DeviceVkRectDraw(self, &drect, src, &srect);

    /*
     * A sub-pixelmap's as_sub.parent aliases its as_back, so resolve the owner before
     * reading the image, and offset the clipped source rect by the sub's base inside it.
     * DeviceVkRectDraw() above resolves the owner itself and is the path for a source that
     * does not resolve.
     */
    if((src_owner = DevicePixelmapVkResolve(src)) == NULL || src_owner->use_type != BRT_OFFSCREEN || src_owner->as_back.image == VK_NULL_HANDLE)
        return BRE_UNSUPPORTED;

    src_base_x = src->pm_base_x - src_owner->pm_base_x;
    src_base_y = src->pm_base_y - src_owner->pm_base_y;

    frame = DevicePixelmapVkGetCurrentFrame(self->screen);

    /*
     * A blit is a transfer command, so nothing about it is ordered against what was
     * recorded before it in this command buffer: the source may be an offscreen that an
     * earlier copy or draw in the same frame wrote - the application's "previous frame"
     * scratch buffer, copied in at the end of one frame and read back at the start of the
     * next - and the destination may have been read earlier. Without a barrier the blit
     * reads that source's old contents and the result is silently wrong.
     *
     * Both images are moved through the transfer layouts and back, as the staging copy in
     * rectangleCopyTo() does: the transition in acquires whatever was written before, and
     * the transition out releases the blit's own write for the paths that read the image
     * next. GENERAL is a legal blit layout for both sides, but on RADV a barrier that
     * leaves the layouts alone, however broad, does not make the source's earlier write
     * visible to this blit's transfer, while the layout transition does. Both pixelmaps are
     * left in the layout they were in - GENERAL - which is what every other path assumes.
     */
    {
        const VkImageSubresourceRange range = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        };
        const VkImageMemoryBarrier source_to_transfer = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_TRANSFER_READ_BIT,
            .oldLayout        = src_owner->as_back.layout,
            .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .image            = src_owner->as_back.image,
            .subresourceRange = range,
        };
        const VkImageMemoryBarrier dest_to_transfer = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout        = self->as_back.layout,
            .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image            = self->as_back.image,
            .subresourceRange = range,
        };
        const VkImageMemoryBarrier to_transfer[2] = {source_to_transfer, dest_to_transfer};

        vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
                               NULL, BR_ASIZE(to_transfer), to_transfer);
    }

    VkImageBlit blit = {
        .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .srcOffsets = {{srect.x + src_base_x, srect.y + src_base_y, 0}, {srect.x + src_base_x + srect.w, srect.y + src_base_y + srect.h, 1}},
        .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .dstOffsets     = {{drect.x, drect.y, 0}, {drect.x + drect.w, drect.y + drect.h, 1}},
    };

    vk->CmdBlitImage(frame->command_buffer, src_owner->as_back.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, self->as_back.image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);

    {
        const VkImageSubresourceRange range = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        };
        const VkImageMemoryBarrier source_to_resting = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_TRANSFER_READ_BIT,
            .dstAccessMask    = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .newLayout        = src_owner->as_back.layout,
            .image            = src_owner->as_back.image,
            .subresourceRange = range,
        };
        const VkImageMemoryBarrier dest_to_resting = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout        = self->as_back.layout,
            .image            = self->as_back.image,
            .subresourceRange = range,
        };
        const VkImageMemoryBarrier to_resting[2] = {source_to_resting, dest_to_resting};

        vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
                               NULL, BR_ASIZE(to_resting), to_resting);
    }

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleCopy)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                      br_rectangle *sr)
{
    br_rectangle d = {
        .x = p->x,
        .y = p->y,
        .w = src->pm_width,
        .h = src->pm_height,
    };

    return BR_CMETHOD(br_device_pixelmap_vk, rectangleStretchCopy)(self, &d, src, sr);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    const GladVulkanContext     *vk = &self->device->context;
    br_device_pixelmap_vk_frame *frame;
    const br_pixelmap_vk_fmt    *fmt;
    br_rectangle                 clip;
    VkCommandBuffer              cmd;

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if((fmt = DeviceVkGetFormatDetails(self->pm_type)) == NULL)
        return BRE_FAIL;

    /* An empty or fully clipped rectangle is nothing to do. */
    if(PixelmapRectangleClip(&clip, rect, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;
    frame = DevicePixelmapVkGetCurrentFrame(self->screen);
    cmd   = frame->command_buffer;

    const VkImageSubresourceRange range = {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    };

    /*
     * An indexed pixel's value is the CLUT index, and BrColourUnpack() refuses the indexed
     * types, so for INDEX_8 the colour is the index itself. R8_UINT is an integer format and
     * clears through the uint32 member; the UNORM formats clear through the float one.
     */
    VkClearColorValue clear_value;

    if(fmt->indexed) {
        clear_value = (VkClearColorValue){
            .uint32 = {colour & 0xffu, 0, 0, 0}
        };
    } else {
        br_uint_8 r, g, b, a;

        BrColourUnpack(colour, self->pm_type, &r, &g, &b, &a);
        clear_value = (VkClearColorValue){
            .float32 = {(float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f}
        };
    }

    /*
     * A clip that covers the whole image is a clear of the image. This is also the only
     * path for the indexed (R8_UINT) format, which is not a colour-attachment format and so
     * cannot be cleared through a rendering scope.
     */
    if(clip.x == 0 && clip.y == 0 && clip.w == self->pm_width && clip.h == self->pm_height) {
        if(self->as_back.layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && self->as_back.layout != VK_IMAGE_LAYOUT_GENERAL) {
            VkImageMemoryBarrier barrier = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = 0,
                .dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
                .oldLayout        = self->as_back.layout,
                .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .image            = self->as_back.image,
                .subresourceRange = range,
            };
            vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
            self->as_back.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }

        vk->CmdClearColorImage(cmd, self->as_back.image, self->as_back.layout, &clear_value, 1, &range);

        return BRE_OK;
    }

    /*
     * An indexed image is not a colour attachment either, so its sub-rect is written with a
     * staging buffer: the index byte repeated across the clipped rectangle, copied into the
     * image through the transfer engine. The buffer is not necessarily read by a submitted
     * command buffer, so it is retired against this frame slot rather than freed (as
     * rectangleCopyTo()'s staging path does).
     */
    if(fmt->indexed) {
        const br_uint_32 row_bytes   = (br_uint_32)clip.w * fmt->bytes_per_pixel;
        const br_uint_32 buffer_size = row_bytes * (br_uint_32)clip.h;
        VkBuffer         staging_buffer;
        VmaAllocation    staging_allocation;
        VkResult         res;

        const VkBufferCreateInfo buffer_info = {
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = buffer_size,
            .usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        const VmaAllocationCreateInfo alloc_info = {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        if((res = vmaCreateBuffer(self->device->vma_allocator, &buffer_info, &alloc_info, &staging_buffer, &staging_allocation, NULL)) != VK_SUCCESS) {
            BrLogError("VKREND", "Failed to create the fill staging buffer: VkResult = %d", res);
            return BRE_FAIL;
        }

        VmaAllocationInfo alloc_info_out;
        vmaGetAllocationInfo(self->device->vma_allocator, staging_allocation, &alloc_info_out);
        memset(alloc_info_out.pMappedData, (int)(colour & 0xffu), buffer_size);
        vmaFlushAllocation(self->device->vma_allocator, staging_allocation, 0, buffer_size);

        {
            const VkImageMemoryBarrier to_transfer_dst = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = 0,
                .dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
                .oldLayout        = self->as_back.layout,
                .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .image            = self->as_back.image,
                .subresourceRange = range,
            };
            vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &to_transfer_dst);
            self->as_back.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }

        {
            const VkBufferImageCopy copy_region = {
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                .imageOffset       = {clip.x, clip.y, 0},
                .imageExtent       = {(uint32_t)clip.w, (uint32_t)clip.h, 1},
            };
            vk->CmdCopyBufferToImage(cmd, staging_buffer, self->as_back.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
        }

        {
            const VkImageMemoryBarrier to_general = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
                .image            = self->as_back.image,
                .subresourceRange = range,
            };
            vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &to_general);
            self->as_back.layout = VK_IMAGE_LAYOUT_GENERAL;
        }

        DevicePixelmapVkRetireBuffer(frame, staging_buffer, staging_allocation);

        return BRE_OK;
    }

    /*
     * Any other format is a colour attachment, so a sub-rect clear goes through a dynamic
     * rendering scope with loadOp LOAD, which keeps the pixels the rectangle does not
     * cover. The barrier shapes mirror the rect draw's in rect.c, and the image is left in
     * GENERAL - the resting layout every other path assumes.
     */
    {
        VkFormatProperties format_props;

        vk->GetPhysicalDeviceFormatProperties(self->device->physical_device->dev, fmt->format, &format_props);
        if(!(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
            BrLogError("VKREND", "Pixelmap format %d cannot be a colour attachment for a sub-rect fill", self->pm_type);
            return BRE_UNSUPPORTED;
        }
    }

    {
        const VkImageMemoryBarrier to_colour_attachment = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout        = self->as_back.layout,
            .newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image            = self->as_back.image,
            .subresourceRange = range,
        };
        vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL,
                               1, &to_colour_attachment);
        self->as_back.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        const VkRect2D render_area = {
            .offset = {0,                        0                        },
            .extent = {(uint32_t)self->pm_width, (uint32_t)self->pm_height},
        };
        const VkRenderingAttachmentInfo colour_attachment = {
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = self->as_back.view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        };
        const VkRenderingInfo rendering = {
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea           = render_area,
            .layerCount           = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments    = &colour_attachment,
        };
        vk->CmdBeginRendering(cmd, &rendering);

        const VkClearAttachment attachment = {
            .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
            .colorAttachment = 0,
            .clearValue      = {.color = clear_value},
        };
        const VkClearRect clear_rect = {
            .rect =
                {
                       .offset = {clip.x, clip.y},
                       .extent = {(uint32_t)clip.w, (uint32_t)clip.h},
                       },
            .baseArrayLayer = 0,
            .layerCount     = 1,
        };
        vk->CmdClearAttachments(cmd, 1, &attachment, 1, &clear_rect);

        vk->CmdEndRendering(cmd);

        const VkImageMemoryBarrier to_general = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
            .image            = self->as_back.image,
            .subresourceRange = range,
        };
        vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL,
                               1, &to_general);
        self->as_back.layout = VK_IMAGE_LAYOUT_GENERAL;
    }

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleCopyTo)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                        br_rectangle *sr);

static br_error BR_CMETHOD(br_device_pixelmap_vk, rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *d,
                                                                          br_device_pixelmap *_src, br_rectangle *s)
{
    br_rectangle srect, drect;

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    /*
     * The source and destination rectangles are clipped independently, as glrend does,
     * so a partially clipped scaled blit is approximate: the drawn region maps the whole
     * visible source over the whole visible destination rather than the sub-rectangle a
     * proportional clip would. Callers that need the exact sub-rectangle stay on the CPU
     * path.
     */
    if(PixelmapRectangleClip(&srect, s, (br_pixelmap *)_src) == BR_CLIP_REJECT)
        return BRE_OK;

    if(PixelmapRectangleClip(&drect, d, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    br_error err = DeviceVkRectDraw(self, &drect, _src, &srect);

    if(err == BRE_OK)
        return BRE_OK;
    if(err != BRE_UNSUPPORTED)
        return err;

    /*
     * The rect draw is the only path here that scales. rectangleCopyTo()'s staging copy
     * does not, so fall back to it only for a copy that is unscaled both as asked and
     * after clipping; a scaled blit the draw cannot serve must fail rather than silently
     * land at the source's size.
     */
    if(d->w == s->w && d->h == s->h && drect.w == srect.w && drect.h == srect.h) {
        br_point p = {.x = d->x, .y = d->y};

        return BR_CMETHOD(br_device_pixelmap_vk, rectangleCopyTo)(self, &p, _src, s);
    }

    return BRE_UNSUPPORTED;
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleCopyTo)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *src,
                                                                        br_rectangle *sr)
{
    const GladVulkanContext     *vk = &self->device->context;
    br_device_pixelmap_vk_frame *frame;
    const br_pixelmap_vk_fmt    *dst_fmt;
    VkResult                     res;
    br_uint_32                   dst_row_bytes;
    VkBuffer                     staging_buffer;
    VmaAllocation                staging_allocation;

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if(src->pm_pixels == NULL)
        return BRE_FAIL;

    dst_fmt = DeviceVkGetFormatDetails(self->pm_type);
    if(dst_fmt == NULL)
        return BRE_FAIL;

    frame = DevicePixelmapVkGetCurrentFrame(self->screen);

    br_rectangle src_rect;
    br_point     dst_point;

    if(PixelmapRectangleClipTwo(&src_rect, &dst_point, sr, p, (br_pixelmap *)self, (br_pixelmap *)src) == BR_CLIP_REJECT)
        return BRE_OK;

    if(src_rect.w <= 0 || src_rect.h <= 0)
        return BRE_OK;

    /*
     * Prefer the GPU path: upload the source raw, in its own format, and draw it into the
     * destination. The source's channel order comes from the sampler's view swizzle and
     * the colour-attachment write converts to the destination format, so no CPU
     * conversion or per-pair upload is involved. An indexed source is drawn the same way
     * with its palette uploaded alongside it as a second texture, white when the source
     * resolves to none. Anything the pipeline cannot serve (an indexed source with a
     * palette or format the device cannot sample, a source format the device cannot
     * sample, an indexed destination, a destination format it cannot render to) is
     * decided before any state is touched, so the CPU staging copy below stands in for
     * all of it.
     */
    /*
     * The destination rectangle is the clipped destination point plus the clipped source
     * size; rectangleCopyTo() is a same-size copy, so the rect draw gets equal-sized
     * rectangles. rectangleStretchCopyTo() builds rectangles that may differ in size and
     * so scales.
     */
    br_rectangle drect = {
        .x = dst_point.x,
        .y = dst_point.y,
        .w = src_rect.w,
        .h = src_rect.h,
    };

    if(DeviceVkRectDraw(self, &drect, src, &src_rect) == BRE_OK)
        return BRE_OK;

    /*
     * The staging copy is always made in the destination's own format. When the source is a
     * different format, convert it through the engine's converter rather than hand-rolling
     * one per pair; the converted pixelmap is host memory, so it can be released as soon as
     * its rows have been copied out. A GL-based driver gets this conversion from GL on the
     * GPU side; doing it there here would mean the driver's first pipeline, so it is out of
     * scope and the CPU converter stands in.
     */
    br_pixelmap *converted = NULL;

    if(src->pm_type != self->pm_type) {
        if((converted = BrPixelmapConvert((br_pixelmap *)src, self->pm_type, NULL)) == NULL) {
            BrLogError("VKREND", "Unable to convert source pixelmap for rectangleCopyTo.");
            return BRE_FAIL;
        }
    }

    const br_pixelmap *copy_src = (converted != NULL) ? converted : (const br_pixelmap *)src;

    dst_row_bytes          = src_rect.w * dst_fmt->bytes_per_pixel;
    br_uint_32 buffer_size = dst_row_bytes * src_rect.h;

    VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = buffer_size,
        .usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo alloc_info = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    };

    if((res = vmaCreateBuffer(self->device->vma_allocator, &buffer_info, &alloc_info, &staging_buffer, &staging_allocation, NULL)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create staging buffer: VkResult = %d", res);
        if(converted != NULL)
            BrPixelmapFree(converted);
        return BRE_FAIL;
    }

    VmaAllocationInfo alloc_info_out;
    vmaGetAllocationInfo(self->device->vma_allocator, staging_allocation, &alloc_info_out);

    br_uint_8 *dst_pixels = (br_uint_8 *)alloc_info_out.pMappedData;
    br_uint_8 *src_row    = ((br_uint_8 *)copy_src->pixels) + src_rect.y * copy_src->row_bytes + src_rect.x * dst_fmt->bytes_per_pixel;

    for(br_int_32 y = 0; y < src_rect.h; ++y) {
        memcpy(dst_pixels + y * dst_row_bytes, src_row + y * copy_src->row_bytes, dst_row_bytes);
    }

    if(converted != NULL)
        BrPixelmapFree(converted);

    vmaFlushAllocation(self->device->vma_allocator, staging_allocation, 0, buffer_size);

    {
        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        };

        VkImageMemoryBarrier barrier = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = 0,
            .dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout        = self->as_back.layout,
            .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image            = self->as_back.image,
            .subresourceRange = range,
        };
        vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
                               NULL, 1, &barrier);
        self->as_back.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }

    {
        VkBufferImageCopy copy_region = {
            .bufferOffset      = 0,
            .bufferRowLength   = 0,
            .bufferImageHeight = 0,
            .imageSubresource  = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .imageOffset       = {dst_point.x, dst_point.y, 0},
            .imageExtent       = {(uint32_t)src_rect.w, (uint32_t)src_rect.h, 1},
        };

        vk->CmdCopyBufferToImage(frame->command_buffer, staging_buffer, self->as_back.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    }

    {
        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        };

        VkImageMemoryBarrier barrier = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
            .image            = self->as_back.image,
            .subresourceRange = range,
        };
        vk->CmdPipelineBarrier(frame->command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
                               NULL, 1, &barrier);
        self->as_back.layout = VK_IMAGE_LAYOUT_GENERAL;
    }

    /*
     * The staging buffer is read by this frame's command buffer, which is not necessarily
     * submitted yet, so retire it for destruction once the slot's fence has signalled
     * rather than leaking it (or freeing it against work still in flight).
     */
    DevicePixelmapVkRetireBuffer(frame, staging_buffer, staging_allocation);

    return BRE_OK;
}

/*
 * Device -> host copy. The source image is copied into a host-visible staging buffer with
 * a one-shot command buffer, then those rows are written into the destination - the
 * reverse of rectangleCopyTo()'s CPU staging path. Vulkan rows are top-down on both
 * sides, so there is no y-flip here; glrend's flip is a GL-origin artefact.
 *
 * The source may still be referenced by the front screen's current command buffer, which
 * has not necessarily been submitted, so the frame is flushed first (and a fresh command
 * buffer begun) or this reads stale pixels.
 */
static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p, br_device_pixelmap *dest,
                                                                          br_rectangle *r)
{
    const GladVulkanContext  *vk = &self->device->context;
    const br_pixelmap_vk_fmt *src_fmt;
    br_rectangle              srect;
    br_point                  dpoint;
    br_uint_32                src_row_bytes;
    br_uint_32                dst_row_bytes;
    br_uint_32                dst_pixel_bytes;
    br_uint_32                buffer_size;
    VkBuffer                  staging_buffer;
    VmaAllocation             staging_allocation;
    VkCommandBuffer           cmd;
    VkFence                   fence;
    VkResult                  res;

    if(self->use_type != BRT_OFFSCREEN || self->as_back.image == VK_NULL_HANDLE)
        return BRE_UNSUPPORTED;

    if((src_fmt = DeviceVkGetFormatDetails(self->pm_type)) == NULL)
        return BRE_FAIL;

    /* Need contiguous pixels and whole rows on the host side, as glrend does. */
    if(!(dest->pm_flags & (BR_PMF_LINEAR | BR_PMF_ROW_WHOLEPIXELS)))
        return BRE_FAIL;

    /*
     * The image was created with TRANSFER_SRC only when the format supports it, so refuse
     * a format whose image cannot be a copy source rather than record an invalid copy.
     */
    {
        VkFormatProperties format_props;

        vk->GetPhysicalDeviceFormatProperties(self->device->physical_device->dev, src_fmt->format, &format_props);
        if(!(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT))
            return BRE_UNSUPPORTED;
    }

    if(PixelmapRectangleClipTwo(&srect, &dpoint, r, p, (br_pixelmap *)dest, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    DevicePixelmapVkFlushFrame(self->screen);

    src_row_bytes = (br_uint_32)srect.w * src_fmt->bytes_per_pixel;
    buffer_size   = src_row_bytes * (br_uint_32)srect.h;

    const VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = buffer_size,
        .usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo alloc_info = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_GPU_TO_CPU,
    };

    if((res = vmaCreateBuffer(self->device->vma_allocator, &buffer_info, &alloc_info, &staging_buffer, &staging_allocation, NULL)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create readback staging buffer: VkResult = %d", res);
        return BRE_FAIL;
    }

    const VkCommandBufferAllocateInfo cmd_alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = self->device->vk_command_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    if((res = vk->AllocateCommandBuffers(self->device->vk_device, &cmd_alloc_info, &cmd)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to allocate readback command buffer: VkResult = %d", res);
        vmaDestroyBuffer(self->device->vma_allocator, staging_buffer, staging_allocation);
        return BRE_FAIL;
    }

    const VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    if((res = vk->CreateFence(self->device->vk_device, &fence_info, NULL, &fence)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create readback fence: VkResult = %d", res);
        vk->FreeCommandBuffers(self->device->vk_device, self->device->vk_command_pool, 1, &cmd);
        vmaDestroyBuffer(self->device->vma_allocator, staging_buffer, staging_allocation);
        return BRE_FAIL;
    }

    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vk->BeginCommandBuffer(cmd, &begin_info);

    const VkImageSubresourceRange range = {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    };

    const VkImageMemoryBarrier to_transfer_src = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask    = VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask    = VK_ACCESS_TRANSFER_READ_BIT,
        .oldLayout        = self->as_back.layout,
        .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .image            = self->as_back.image,
        .subresourceRange = range,
    };
    vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &to_transfer_src);

    const VkBufferImageCopy copy_region = {
        .bufferOffset      = 0,
        .bufferRowLength   = 0,
        .bufferImageHeight = 0,
        .imageSubresource  = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .imageOffset       = {srect.x, srect.y, 0},
        .imageExtent       = {(uint32_t)srect.w, (uint32_t)srect.h, 1},
    };
    vk->CmdCopyImageToBuffer(cmd, self->as_back.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging_buffer, 1, &copy_region);

    const VkImageMemoryBarrier to_general = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask    = VK_ACCESS_TRANSFER_READ_BIT,
        .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
        .image            = self->as_back.image,
        .subresourceRange = range,
    };
    vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &to_general);
    self->as_back.layout = VK_IMAGE_LAYOUT_GENERAL;

    vk->EndCommandBuffer(cmd);

    const VkSubmitInfo submit_info = {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &cmd,
    };
    if((res = vk->QueueSubmit(self->device->vk_queue, 1, &submit_info, fence)) != VK_SUCCESS)
        BR_FATAL1("vkQueueSubmit() returned %" PRIu32, res);

    if((res = vk->WaitForFences(self->device->vk_device, 1, &fence, VK_TRUE, BR_VKREND_DEFAULT_WAIT)) != VK_SUCCESS)
        BR_FATAL1("vkWaitForFences() returned %" PRIu32, res);

    vk->DestroyFence(self->device->vk_device, fence, NULL);
    vk->FreeCommandBuffers(self->device->vk_device, self->device->vk_command_pool, 1, &cmd);

    VmaAllocationInfo staging_info;
    vmaGetAllocationInfo(self->device->vma_allocator, staging_allocation, &staging_info);
    vmaInvalidateAllocation(self->device->vma_allocator, staging_allocation, 0, buffer_size);

    const br_uint_8 *src_pixels = (const br_uint_8 *)staging_info.pMappedData;
    br_uint_32       src_stride = src_row_bytes;
    br_pixelmap     *converted  = NULL;

    /*
     * The staged data is in the source's format. A destination of another type is served
     * by converting the staged region through the engine's converter - the reverse of
     * rectangleCopyTo()'s mismatch path - rather than hand-rolling a conversion per pair.
     */
    if(dest->pm_type != self->pm_type) {
        br_pixelmap *staged_pm = BrPixelmapAllocate(self->pm_type, srect.w, srect.h, NULL, BR_PMAF_NORMAL);

        if(staged_pm == NULL) {
            BrLogError("VKREND", "Unable to allocate a staging pixelmap for the readback conversion.");
            vmaDestroyBuffer(self->device->vma_allocator, staging_buffer, staging_allocation);
            return BRE_FAIL;
        }

        for(br_int_32 y = 0; y < srect.h; ++y)
            memcpy((br_uint_8 *)staged_pm->pixels + (br_uint_32)y * staged_pm->row_bytes, src_pixels + (br_uint_32)y * src_row_bytes, src_row_bytes);

        converted = BrPixelmapConvert(staged_pm, dest->pm_type, NULL);
        BrPixelmapFree(staged_pm);

        if(converted == NULL) {
            BrLogError("VKREND", "Unable to convert the readback pixels to the destination type.");
            vmaDestroyBuffer(self->device->vma_allocator, staging_buffer, staging_allocation);
            return BRE_FAIL;
        }

        src_pixels = (const br_uint_8 *)converted->pixels;
        src_stride = converted->row_bytes;
    }

    dst_pixel_bytes = BrPixelmapPixelSize((br_pixelmap *)dest) >> 3;
    dst_row_bytes   = (br_uint_32)srect.w * dst_pixel_bytes;

    br_uint_8 *dst_pixels = (br_uint_8 *)dest->pm_pixels + (dest->pm_base_y + dpoint.y) * (br_int_32)dest->pm_row_bytes +
                            (dest->pm_base_x + dpoint.x) * (br_int_32)dst_pixel_bytes;

    for(br_int_32 y = 0; y < srect.h; ++y)
        memcpy(dst_pixels + y * dest->pm_row_bytes, src_pixels + (br_uint_32)y * src_stride, dst_row_bytes);

    if(converted != NULL)
        BrPixelmapFree(converted);

    vmaDestroyBuffer(self->device->vma_allocator, staging_buffer, staging_allocation);

    return BRE_OK;
}

static br_error BR_CMETHOD(br_device_pixelmap_vk, text)(br_device_pixelmap *self, br_point *point, br_font *font, const char *text, br_uint_32 colour)
{
    br_point                  start;
    const br_pixelmap_vk_fmt *fmt;
    br_uint_8                 r = 0, g = 0, b = 0, a = 255;
    float                     rgba[4];

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    /*
     * An indexed destination holds CLUT indices and the draw writes a colour, so it stays
     * unsupported; an INDEX_8 image is R8_UINT and not a colour attachment. An indexed
     * type would make BrColourUnpack() refuse the colour anyway.
     */
    if((fmt = DeviceVkGetFormatDetails(self->pm_type)) == NULL || fmt->indexed)
        return BRE_UNSUPPORTED;

    /* A string whose starting point is offscreen draws nothing at all. */
    if(PixelmapPointClip(&start, point, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    BrColourUnpack(colour, self->pm_type, &r, &g, &b, &a);
    rgba[0] = (float)r / 255.0f;
    rgba[1] = (float)g / 255.0f;
    rgba[2] = (float)b / 255.0f;
    rgba[3] = (float)a / 255.0f;

    return DeviceVkTextDraw(self, &start, font, text, rgba);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, line)(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
    br_point                  start, end;
    const br_pixelmap_vk_fmt *fmt;
    br_uint_8                 r = 0, g = 0, b = 0, a = 255;
    float                     rgba[4];
    br_boolean                indexed;

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if((fmt = DeviceVkGetFormatDetails(self->pm_type)) == NULL)
        return BRE_UNSUPPORTED;

    if(PixelmapLineClip(&start, &end, s, e, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    /*
     * An indexed destination holds CLUT indices and the draw writes a colour, so
     * BrColourUnpack() refuses the type and the colour is the index itself - the same
     * convention the indexed fill uses. The draw then selects the integer-output pipeline.
     */
    if((indexed = fmt->indexed)) {
        rgba[0] = (float)(colour & 0xffu);
        rgba[1] = rgba[2] = rgba[3] = 0.0f;
    } else {
        BrColourUnpack(colour, self->pm_type, &r, &g, &b, &a);
        rgba[0] = (float)r / 255.0f;
        rgba[1] = (float)g / 255.0f;
        rgba[2] = (float)b / 255.0f;
        rgba[3] = (float)a / 255.0f;
    }

    return DeviceVkLineDraw(self, &start, &end, rgba, BR_FALSE, indexed);
}

static br_error BR_CMETHOD_DECL(br_device_pixelmap_vk, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    br_point                  point;
    const br_pixelmap_vk_fmt *fmt;
    br_uint_8                 r = 0, g = 0, b = 0, a = 255;
    float                     rgba[4];
    br_boolean                indexed;

    if(self->use_type != BRT_OFFSCREEN)
        return BRE_UNSUPPORTED;

    if((fmt = DeviceVkGetFormatDetails(self->pm_type)) == NULL)
        return BRE_UNSUPPORTED;

    if(PixelmapPointClip(&point, p, (br_pixelmap *)self) == BR_CLIP_REJECT)
        return BRE_OK;

    /* As for line(): an indexed destination stores the index, not a colour. */
    if((indexed = fmt->indexed)) {
        rgba[0] = (float)(colour & 0xffu);
        rgba[1] = rgba[2] = rgba[3] = 0.0f;
    } else {
        BrColourUnpack(colour, self->pm_type, &r, &g, &b, &a);
        rgba[0] = (float)r / 255.0f;
        rgba[1] = (float)g / 255.0f;
        rgba[2] = (float)b / 255.0f;
        rgba[3] = (float)a / 255.0f;
    }

    /* A point is a one-vertex point primitive; the pipeline places it at its pixel centre. */
    return DeviceVkLineDraw(self, &point, &point, rgba, BR_TRUE, indexed);
}

static const struct br_device_pixelmap_dispatch devicePixelmapDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_pixelmap_vk, free),
    ._identifier = BR_CMETHOD_REF(br_device_pixelmap_vk, identifier),
    ._type       = BR_CMETHOD_REF(br_device_pixelmap_vk, type),
    ._isType     = BR_CMETHOD_REF(br_device_pixelmap_vk, isType),
    ._device     = BR_CMETHOD_REF(br_device_pixelmap_vk, device),
    ._space      = BR_CMETHOD_REF(br_device_pixelmap_vk, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_pixelmap_vk, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._validSource = BR_CMETHOD_REF(br_device_pixelmap_null, validSource),
    ._resize      = BR_CMETHOD_REF(br_device_pixelmap_vk, resize),
    ._match       = BR_CMETHOD_REF(br_device_pixelmap_vk, match),
    ._allocateSub = BR_CMETHOD_REF(br_device_pixelmap_vk, allocateSub),

    ._copy         = BR_CMETHOD_REF(br_device_pixelmap_gen, copy),
    ._copyTo       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyTo),
    ._copyFrom     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFrom),
    ._fill         = BR_CMETHOD_REF(br_device_pixelmap_gen, fill),
    ._doubleBuffer = BR_CMETHOD_REF(br_device_pixelmap_fail, doubleBuffer),

    ._copyDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, copyDirty),
    ._copyToDirty       = BR_CMETHOD_REF(br_device_pixelmap_gen, copyToDirty),
    ._copyFromDirty     = BR_CMETHOD_REF(br_device_pixelmap_gen, copyFromDirty),
    ._fillDirty         = BR_CMETHOD_REF(br_device_pixelmap_gen, fillDirty),
    ._doubleBufferDirty = BR_CMETHOD_REF(br_device_pixelmap_gen, doubleBufferDirty),

    ._rectangle                = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle),
    ._rectangle2               = BR_CMETHOD_REF(br_device_pixelmap_gen, rectangle2),
    ._rectangleCopy            = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleCopy),
    ._rectangleCopyTo          = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleCopyTo),
    ._rectangleCopyFrom        = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleCopyFrom),
    ._rectangleStretchCopy     = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleStretchCopy),
    ._rectangleStretchCopyTo   = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleStretchCopyTo),
    ._rectangleStretchCopyFrom = BR_CMETHOD_REF(br_device_pixelmap_fail, rectangleStretchCopyFrom),
    ._rectangleFill            = BR_CMETHOD_REF(br_device_pixelmap_vk, rectangleFill),
    ._pixelSet                 = BR_CMETHOD_REF(br_device_pixelmap_vk, pixelSet),
    ._line                     = BR_CMETHOD_REF(br_device_pixelmap_vk, line),
    ._copyBits                 = BR_CMETHOD_REF(br_device_pixelmap_fail, copyBits),

    ._text       = BR_CMETHOD_REF(br_device_pixelmap_vk, text),
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

    ._handleWindowEvent = BR_CMETHOD_REF(br_device_pixelmap_gen, handleWindowEvent),
};
