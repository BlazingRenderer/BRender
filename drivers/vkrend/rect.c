/*
 * Pixelmap -> device rect draw on the GPU.
 *
 * The source is either host memory, uploaded raw in its own format, or another offscreen's
 * device image, sampled where it lies through a transient image view. A sub-pixelmap source
 * resolves to the offscreen that owns it (a sub aliases its owner through as_sub.parent
 * over as_back, so its own as_back must never be read), and its rectangle is offset by the
 * sub's base inside that owner. Either way it is drawn as a textured quad into the
 * destination image. Nothing here is specialised by the
 * source format: the sampler normalises whatever channel layout the source uses (its
 * VkComponentMapping comes from the format table) and the destination's colour-attachment
 * write converts the floats back. An indexed source is the one source shape that takes a
 * second texture, and so a second pipeline: the index plane is fetched through a
 * usampler2D and looked up in the palette, which is uploaded raw in its own format
 * exactly as a host source is. The palette is the source's own map when it has one,
 * otherwise the source offscreen's device CLUT, otherwise the destination offscreen's
 * - glrend's source-then-destination fallback, with the source's own map taking precedence
 * over both. The destination format is the one thing that must be specialised, because
 * VK_KHR_dynamic_rendering has no VkRenderPass to derive the attachment formats from, so
 * VkPipelineRenderingCreateInfo::pColorAttachmentFormats names it and the pipeline only
 * matches that format, sample count and (plain or indexed) source shape.
 *
 * All of it is device-scoped and built on first use. A VkPipeline is a VkDevice object
 * that outlives any surface, window or frame, so none of it uses the per-frame retire
 * machinery - only the per-draw resources (each uploaded image and its view, the staging
 * buffer that carried it, and a device source's transient view) are handed to it.
 *
 * A device source's image is transitioned into SHADER_READ_ONLY_OPTIMAL for the draw and
 * back into the layout it was in, so its as_back.layout stays accurate. rectangleCopyTo()
 * falls back to the CPU staging path whenever this returns BRE_UNSUPPORTED, which is
 * decided before anything is recorded. The known cases are a device source that is not an
 * offscreen image, an indexed source whose palette - its own map, then the two populated
 * device CLUTs - is not host memory or is in a format the device cannot sample, the index
 * plane not being the unsigned-integer format the index shader declares, an indexed
 * destination (the draw writes a colour, not an index), and source, palette or destination
 * formats the device cannot sample or render to. An indexed source with no palette at all
 * is drawn with a white one, as glrend does.
 */
#include "drv.h"
#include "brassert.h"

#include <string.h>

#include "rect.vert.spv.h"
#include "rect.frag.spv.h"
#include "rect.indexed.frag.spv.h"

/*
 * The palette the indexed draw uses when the source resolves to none: glrend samples a 1x1
 * white texture in that case, so every index comes out white. A device CLUT is allocated
 * for every offscreen whether or not the application ever sets an entry, so it is only a
 * palette once populated (see br_device_clut::populated); the fallback keeps such a CLUT's
 * zero entries from rendering black. The single entry is the format a device CLUT uses, and
 * the index shader clamps every index to its one row.
 */
static const br_uint_8   vk_rect_white_pixels[4] = {0xFF, 0xFF, 0xFF, 0xFF};
static const br_pixelmap vk_rect_white_palette   = {
      .pixels    = (void *)vk_rect_white_pixels,
      .row_bytes = 4,
      .type      = BR_PMT_RGBX_888,
      .width     = 1,
      .height    = 1,
};

/**
 * \brief Does the device support the given optimal-tiling features for a format?
 */
static br_boolean DeviceVkRectFormatSupported(br_device *dev, VkFormat format, VkFormatFeatureFlags required)
{
    VkFormatProperties props;

    dev->context.GetPhysicalDeviceFormatProperties(dev->physical_device->dev, format, &props);
    return (props.optimalTilingFeatures & required) == required;
}

/**
 * \brief Release the rect machinery's device-scoped resources and mark it unavailable.
 */
void DeviceVkRectFini(br_device *dev)
{
    const GladVulkanContext *vk = &dev->context;

    if(dev->vk_device != NULL) {
        for(br_uint_32 i = 0; i < dev->rect_pipeline_count; ++i) {
            if(dev->rect_pipelines[i].pipeline != VK_NULL_HANDLE)
                vk->DestroyPipeline(dev->vk_device, dev->rect_pipelines[i].pipeline, NULL);
            dev->rect_pipelines[i].pipeline = VK_NULL_HANDLE;
        }

        if(dev->rect_sampler != VK_NULL_HANDLE)
            vk->DestroySampler(dev->vk_device, dev->rect_sampler, NULL);
        if(dev->rect_pipeline_layout != VK_NULL_HANDLE)
            vk->DestroyPipelineLayout(dev->vk_device, dev->rect_pipeline_layout, NULL);
        if(dev->rect_set_layout != VK_NULL_HANDLE)
            vk->DestroyDescriptorSetLayout(dev->vk_device, dev->rect_set_layout, NULL);
        if(dev->rect_vert != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->rect_vert, NULL);
        if(dev->rect_frag != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->rect_frag, NULL);
        if(dev->rect_indexed_frag != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->rect_indexed_frag, NULL);
    }

    dev->rect_vert            = VK_NULL_HANDLE;
    dev->rect_frag            = VK_NULL_HANDLE;
    dev->rect_indexed_frag    = VK_NULL_HANDLE;
    dev->rect_set_layout      = VK_NULL_HANDLE;
    dev->rect_pipeline_layout = VK_NULL_HANDLE;
    dev->rect_sampler         = VK_NULL_HANDLE;
    dev->rect_pipeline_count  = 0;
    dev->rect_state           = BR_VK_RECT_UNAVAILABLE;
}

/**
 * \brief Build the rect machinery's device-scoped resources, once.
 *
 * Returns BR_VK_RECT_READY if the renderer can draw, BR_VK_RECT_UNAVAILABLE otherwise
 * (the result is remembered, so a device that cannot is only tried once).
 */
br_vk_rect_state DeviceVkRectEnsureInit(br_device *dev)
{
    const GladVulkanContext *vk = &dev->context;
    VkResult                 res;

    if(dev->rect_state != BR_VK_RECT_UNINITIALISED)
        return dev->rect_state;

    dev->rect_state = BR_VK_RECT_UNAVAILABLE;

    /*
     * Dynamic rendering (core 1.3, enabled as a feature on the logical device) supplies
     * the rendering scope, and push descriptors supply the source binding without a
     * descriptor pool that some later frame would still be reading.
     */
    if(!dev->dynamic_rendering || !dev->push_descriptor || dev->context.CmdBeginRendering == NULL || dev->context.CmdPushDescriptorSet == NULL) {
        BrLogInfo("VKREND", "Rect draw disabled: dynamic rendering and/or push descriptors are unavailable.");
        return dev->rect_state;
    }

    const VkShaderModuleCreateInfo vert_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_rect_vert_spv),
        .pCode    = (const uint32_t *)vkrend_rect_vert_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &vert_info, NULL, &dev->rect_vert)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect vertex shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo frag_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_rect_frag_spv),
        .pCode    = (const uint32_t *)vkrend_rect_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &frag_info, NULL, &dev->rect_frag)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect fragment shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo indexed_frag_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_rect_indexed_frag_spv),
        .pCode    = (const uint32_t *)vkrend_rect_indexed_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &indexed_frag_info, NULL, &dev->rect_indexed_frag)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect indexed fragment shader module: VkResult = %d", res);
        goto fail;
    }

    /*
     * One set layout for both variants: binding 0 is the source and binding 1 the palette,
     * which only the indexed fragment shader statically uses. A push descriptor that is not
     * statically used can be left undefined, so a plain draw pushes binding 0 alone.
     */
    const VkDescriptorSetLayoutBinding bindings[2] = {
        {
         .binding         = 0,
         .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
        {
         .binding         = 1,
         .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
    };
    const VkDescriptorSetLayoutCreateInfo set_info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
        .bindingCount = BR_ASIZE(bindings),
        .pBindings    = bindings,
    };
    if((res = vk->CreateDescriptorSetLayout(dev->vk_device, &set_info, NULL, &dev->rect_set_layout)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect descriptor set layout: VkResult = %d", res);
        goto fail;
    }

    const VkPushConstantRange push_range = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset     = 0,
        .size       = sizeof(float) * 4,
    };
    const VkPipelineLayoutCreateInfo layout_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &dev->rect_set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &push_range,
    };
    if((res = vk->CreatePipelineLayout(dev->vk_device, &layout_info, NULL, &dev->rect_pipeline_layout)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect pipeline layout: VkResult = %d", res);
        goto fail;
    }

    const VkSamplerCreateInfo sampler_info = {
        .sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter    = VK_FILTER_NEAREST,
        .minFilter    = VK_FILTER_NEAREST,
        .mipmapMode   = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .maxLod       = 0.0f,
    };
    if((res = vk->CreateSampler(dev->vk_device, &sampler_info, NULL, &dev->rect_sampler)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect sampler: VkResult = %d", res);
        goto fail;
    }

    dev->rect_state = BR_VK_RECT_READY;
    return dev->rect_state;

fail:
    DeviceVkRectFini(dev);
    return dev->rect_state;
}

/**
 * \brief Get (building on first use) the rect pipeline for a destination format, sample
 * count and source shape.
 *
 * \return the pipeline, or VK_NULL_HANDLE if one could not be built.
 */
static VkPipeline DeviceVkRectGetPipeline(br_device *dev, VkFormat format, VkSampleCountFlagBits samples, br_boolean indexed)
{
    const GladVulkanContext *vk = &dev->context;

    for(br_uint_32 i = 0; i < dev->rect_pipeline_count; ++i) {
        if(dev->rect_pipelines[i].format == format && dev->rect_pipelines[i].samples == samples && dev->rect_pipelines[i].indexed == indexed)
            return dev->rect_pipelines[i].pipeline;
    }

    if(dev->rect_pipeline_count >= BR_VKREND_MAX_RECT_PIPELINES) {
        BrLogError("VKREND", "Too many rect pipelines (%u) for destination format %d.", dev->rect_pipeline_count, (int)format);
        return VK_NULL_HANDLE;
    }

    const VkPipelineShaderStageCreateInfo stages[2] = {
        {
         .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage  = VK_SHADER_STAGE_VERTEX_BIT,
         .module = dev->rect_vert,
         .pName  = "main",
         },
        {
         .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = indexed ? dev->rect_indexed_frag : dev->rect_frag,
         .pName  = "main",
         },
    };

    const VkPipelineVertexInputStateCreateInfo input = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    const VkPipelineInputAssemblyStateCreateInfo assembly = {
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    const VkPipelineViewportStateCreateInfo viewport = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1,
    };

    const VkPipelineRasterizationStateCreateInfo raster = {
        .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode    = VK_CULL_MODE_NONE,
        .frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth   = 1.0f,
    };

    const VkPipelineMultisampleStateCreateInfo multisample = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = samples,
    };

    const VkPipelineColorBlendAttachmentState blend_attachment = {
        .blendEnable    = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    const VkPipelineColorBlendStateCreateInfo blend = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &blend_attachment,
    };

    const VkDynamicState dynamic_states[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    const VkPipelineDynamicStateCreateInfo dynamic = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = BR_ASIZE(dynamic_states),
        .pDynamicStates    = dynamic_states,
    };

    /*
     * Dynamic rendering has no VkRenderPass, so the destination format is named here.
     * This is what makes the pipeline destination-specific.
     */
    const VkPipelineRenderingCreateInfo rendering = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &format,
    };

    const VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &rendering,
        .stageCount          = BR_ASIZE(stages),
        .pStages             = stages,
        .pVertexInputState   = &input,
        .pInputAssemblyState = &assembly,
        .pViewportState      = &viewport,
        .pRasterizationState = &raster,
        .pMultisampleState   = &multisample,
        .pColorBlendState    = &blend,
        .pDynamicState       = &dynamic,
        .layout              = dev->rect_pipeline_layout,
        .renderPass          = VK_NULL_HANDLE,
        .subpass             = 0,
    };

    VkPipeline pipeline;
    VkResult   res = vk->CreateGraphicsPipelines(dev->vk_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline);
    if(res != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect pipeline for destination format %d: VkResult = %d", (int)format, res);
        return VK_NULL_HANDLE;
    }

    br_device_vk_rect_pipeline *entry = &dev->rect_pipelines[dev->rect_pipeline_count++];
    entry->format                     = format;
    entry->samples                    = samples;
    entry->indexed                    = indexed;
    entry->pipeline                   = pipeline;

    return pipeline;
}

/**
 * \brief A host pixelmap region uploaded into a new sampled image, with the staging buffer
 * that carried it.
 */
typedef struct br_vk_rect_upload {
    VkImage       image;
    VkImageView   view;
    VmaAllocation allocation;
    VkBuffer      staging;
    VmaAllocation staging_allocation;

    /* The extent of the image, i.e. the size of the region that was copied into it. */
    br_uint_32 width;
    br_uint_32 height;
} br_vk_rect_upload;

/**
 * \brief Release an upload that never reached the frame slot's retire list.
 *
 * Safe to call twice, so a failure part-way through an upload and the caller's cleanup
 * share it.
 */
static void DeviceVkRectUploadDestroy(br_device *dev, br_vk_rect_upload *upload)
{
    const GladVulkanContext *vk = &dev->context;

    if(upload->view != VK_NULL_HANDLE)
        vk->DestroyImageView(dev->vk_device, upload->view, NULL);
    if(upload->image != VK_NULL_HANDLE)
        vmaDestroyImage(dev->vma_allocator, upload->image, upload->allocation);
    if(upload->staging != VK_NULL_HANDLE)
        vmaDestroyBuffer(dev->vma_allocator, upload->staging, upload->staging_allocation);

    *upload = (br_vk_rect_upload){0};
}

/**
 * \brief Upload a region of a host pixelmap's pixels into a new sampled image.
 *
 * The pixels are copied raw, at the source's own stride and in the source's own format;
 * the view carries the format table's swizzle, so the sampler presents the shader with the
 * channel order it expects and the copy needs no per-format shader. The staging buffer is
 * not destroyed here: it is retired against the frame slot with the image once the draw has
 * been recorded, and only a failure before that frees it here.
 */
static br_error DeviceVkRectUpload(br_device *dev, const br_pixelmap_vk_fmt *fmt, const void *pixels, br_uint_32 row_bytes, br_int_32 x,
                                   br_int_32 y, br_uint_32 w, br_uint_32 h, br_vk_rect_upload *upload)
{
    const GladVulkanContext *vk = &dev->context;
    VkResult                 res;

    const VkImageCreateInfo image_info = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .format        = fmt->format,
        .extent        = {w, h, 1},
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo image_alloc_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    if((res = vmaCreateImage(dev->vma_allocator, &image_info, &image_alloc_info, &upload->image, &upload->allocation, NULL)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect upload image: VkResult = %d", res);
        return BRE_FAIL;
    }

    /*
     * The format table's swizzle reconciles the source's channel order with Vulkan's.
     * This is the only place it is needed: the destination is written in the float
     * order the shader produces, exactly as rectangleFill() writes its clear colour.
     */
    const VkImageViewCreateInfo view_info = {
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image    = upload->image,
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

    if((res = vk->CreateImageView(dev->vk_device, &view_info, NULL, &upload->view)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect upload image view: VkResult = %d", res);
        goto fail;
    }

    {
        const br_uint_32 packed_row_bytes = w * fmt->bytes_per_pixel;
        const br_uint_32 buffer_size      = packed_row_bytes * h;

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

        if((res = vmaCreateBuffer(dev->vma_allocator, &buffer_info, &buffer_alloc_info, &upload->staging, &upload->staging_allocation, NULL)) !=
           VK_SUCCESS) {
            BrLogError("VKREND", "Failed to create the rect staging buffer: VkResult = %d", res);
            goto fail;
        }

        VmaAllocationInfo alloc_info;
        vmaGetAllocationInfo(dev->vma_allocator, upload->staging_allocation, &alloc_info);

        br_uint_8       *mapped  = (br_uint_8 *)alloc_info.pMappedData;
        const br_uint_8 *src_row = (const br_uint_8 *)pixels + y * row_bytes + x * fmt->bytes_per_pixel;

        for(br_uint_32 i = 0; i < h; ++i) {
            memcpy(mapped + i * packed_row_bytes, src_row + i * row_bytes, packed_row_bytes);
        }

        vmaFlushAllocation(dev->vma_allocator, upload->staging_allocation, 0, buffer_size);
    }

    upload->width  = w;
    upload->height = h;
    return BRE_OK;

fail:
    DeviceVkRectUploadDestroy(dev, upload);
    return BRE_FAIL;
}

/**
 * \brief Create a sampled view of a device pixelmap's image carrying the format table's
 * swizzle.
 *
 * The offscreen's own as_back.view has no component mapping, so it presents the image's
 * channels in Vulkan's order; the shader expects the format table's order, which the
 * swizzle reconciles. The view is transient - the caller retires it against the frame slot
 * - and the image itself stays the source pixelmap's.
 */
static br_error DeviceVkRectDeviceView(br_device *dev, const br_pixelmap_vk_fmt *fmt, VkImage image, VkImageView *out)
{
    const GladVulkanContext *vk = &dev->context;

    const VkImageViewCreateInfo view_info = {
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image    = image,
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

    VkResult res = vk->CreateImageView(dev->vk_device, &view_info, NULL, out);
    if(res != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the rect draw's device source view: VkResult = %d", res);
        return BRE_FAIL;
    }

    return BRE_OK;
}

/**
 * \brief Upload a host pixelmap's region raw and draw it into a destination rectangle.
 *
 * The source region is the clipped source rect; the destination rectangle is the clipped
 * destination rect. They differ in size for a stretch copy, which the viewport scales.
 *
 * Everything that can make the GPU path impossible is decided before anything is
 * recorded, so BRE_UNSUPPORTED means the caller can fall back to the CPU staging copy
 * without having to undo any state.
 */
br_error DeviceVkRectDraw(br_device_pixelmap *dst, br_rectangle *dst_rect, br_device_pixelmap *src, br_rectangle *src_rect)
{
    br_device                   *dev = dst->device;
    const GladVulkanContext     *vk  = &dev->context;
    const br_pixelmap_vk_fmt    *src_fmt;
    const br_pixelmap_vk_fmt    *dst_fmt;
    const br_pixelmap_vk_fmt    *clut_fmt  = NULL;
    const br_pixelmap           *clut      = NULL;
    br_device_pixelmap          *src_owner = NULL;
    br_device_pixelmap          *dst_owner = NULL;
    br_device_pixelmap_vk_frame *frame;
    br_vk_rect_upload            uploads[2]  = {0};
    VkImageView                  source_view = VK_NULL_HANDLE;
    VkImageLayout                source_layout;
    br_uint_32                   upload_count;
    VkPipeline                   pipeline;
    const br_uint_32             w          = (br_uint_32)src_rect->w;
    const br_uint_32             h          = (br_uint_32)src_rect->h;
    br_int_32                    src_base_x = 0;
    br_int_32                    src_base_y = 0;

    /*
     * The source is either host memory, which is uploaded region by region below, or
     * another offscreen's device image, which is sampled where it lies. A pixelmap with
     * neither (a screen, whose union holds no image here) cannot be drawn from.
     */
    const br_boolean src_is_device = (src->pm_pixels == NULL);

    if(src_rect->w <= 0 || src_rect->h <= 0)
        return BRE_UNSUPPORTED;

    if(src_is_device) {
        /*
         * A sub-pixelmap's as_sub.parent aliases its as_back, so its own as_back must
         * never be read: the image, view and layout are always the resolved owner's, and
         * a source that resolves to nothing or to a non-offscreen (a screen is a plain
         * vkrend pixelmap too, but its union holds no image) cannot be drawn from.
         */
        if((src_owner = DevicePixelmapVkResolve(src)) == NULL || src_owner->use_type != BRT_OFFSCREEN || src_owner->as_back.image == VK_NULL_HANDLE)
            return BRE_UNSUPPORTED;

        /*
         * The source rect is relative to the source's own base; the owner's image holds
         * it at that base (zero for a plain offscreen). Offset the sampled region so it
         * names the sub's area inside the owner's image.
         */
        src_base_x = src->pm_base_x - src_owner->pm_base_x;
        src_base_y = src->pm_base_y - src_owner->pm_base_y;
    }

    if((src_fmt = DeviceVkGetFormatDetails(src->pm_type)) == NULL)
        return BRE_UNSUPPORTED;
    if((dst_fmt = DeviceVkGetFormatDetails(dst->pm_type)) == NULL)
        return BRE_UNSUPPORTED;

    /*
     * An indexed destination would have to be written with an index, and the draw writes a
     * colour, so it stays on the CPU path. An indexed source is drawn from its index plane
     * plus the palette in src->map; the index shader fetches that plane through a
     * usampler2D, so it needs the unsigned-integer format the table gives INDEX_8, and the
     * palette has to be present, be host memory, be addressable and be in a non-indexed
     * format the device can sample.
     */
    if(dst_fmt->indexed)
        return BRE_UNSUPPORTED;

    if(src_fmt->format == VK_FORMAT_UNDEFINED || dst_fmt->format == VK_FORMAT_UNDEFINED)
        return BRE_UNSUPPORTED;

    if(src_fmt->indexed) {
        /*
         * The palette is the source's own map plane when it has one, otherwise the
         * source offscreen's device CLUT, otherwise the destination offscreen's -
         * glrend's source-then-destination fallback, which is what lets a colour buffer
         * hold the palette for an indexed source that has none. A sub-pixelmap resolves
         * to its owner, whose as_back holds the CLUT; a host pixelmap has no device CLUT
         * at all.
         *
         * A device CLUT counts only once an entry has been set: an offscreen always owns
         * one, so its mere existence is not a palette, and glrend likewise ignores a CLUT
         * whose texture was never built. With no palette anywhere, the draw uses a white
         * one rather than the unpopulated CLUT's zeros, exactly as glrend does.
         *
         * Only a host palette is uploaded; a device palette (one whose pixels are not
         * host-addressable) is left unsupported rather than given its own transient
         * view and layout transition, because nothing in the tree builds one and the
         * extra path would be unexercised. A source whose map is such a palette is
         * rejected rather than falling through to a CLUT, which would silently draw with
         * the wrong palette. The device CLUT's entries are host memory by construction,
         * so they take the same upload path.
         */
        dst_owner = DevicePixelmapVkResolve(dst);

        if(src_fmt->format != VK_FORMAT_R8_UINT)
            return BRE_UNSUPPORTED;

        if(src->pm_map != NULL) {
            if(src->pm_map->pixels == NULL)
                return BRE_UNSUPPORTED;
            clut = src->pm_map;
        } else if(src_owner != NULL && src_owner->use_type == BRT_OFFSCREEN && src_owner->as_back.clut != NULL && src_owner->as_back.clut->populated)
            clut = src_owner->as_back.clut->pm;
        else if(dst_owner != NULL && dst_owner->use_type == BRT_OFFSCREEN && dst_owner->as_back.clut != NULL && dst_owner->as_back.clut->populated)
            clut = dst_owner->as_back.clut->pm;

        if(clut == NULL)
            clut = &vk_rect_white_palette;
        if(clut->pixels == NULL)
            return BRE_UNSUPPORTED;
        if(clut->width <= 0 || clut->height <= 0)
            return BRE_UNSUPPORTED;
        if((clut_fmt = DeviceVkGetFormatDetails(clut->type)) == NULL)
            return BRE_UNSUPPORTED;
        if(clut_fmt->indexed || clut_fmt->format == VK_FORMAT_UNDEFINED)
            return BRE_UNSUPPORTED;
    }

    if(DeviceVkRectEnsureInit(dev) != BR_VK_RECT_READY)
        return BRE_UNSUPPORTED;

    /*
     * A host source is uploaded, so it has to be a transfer destination as well as
     * sampleable; a device source is only ever sampled, where it already lies.
     */
    if(!DeviceVkRectFormatSupported(dev, src_fmt->format,
                                    src_is_device ? VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
                                                  : (VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)))
        return BRE_UNSUPPORTED;

    if(clut != NULL && !DeviceVkRectFormatSupported(dev, clut_fmt->format, VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
        return BRE_UNSUPPORTED;

    /*
     * The same query DevicePixelmapVkCreateImage() used to decide the image's colour-
     * attachment usage, so an image that could not be rendered to cannot reach here.
     */
    if(!DeviceVkRectFormatSupported(dev, dst_fmt->format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
        return BRE_UNSUPPORTED;

    /* vkrend creates every image single-sampled, so that is the only sample count here. */
    if((pipeline = DeviceVkRectGetPipeline(dev, dst_fmt->format, VK_SAMPLE_COUNT_1_BIT, src_fmt->indexed)) == VK_NULL_HANDLE)
        return BRE_FAIL;

    frame = DevicePixelmapVkGetCurrentFrame(dst->screen);

    /*
     * A host source is uploaded as the clipped region alone - it is transient and only
     * ever sampled - so the shader's source rect starts at the origin. A device source is
     * bound where it lies through a view carrying the format table's swizzle (the
     * offscreen's own view has none), and the shader's source rect names the region inside
     * it. The palette is uploaded whole, because its rows are the CLUT entries an index
     * names.
     */
    if(src_is_device) {
        if(DeviceVkRectDeviceView(dev, src_fmt, src_owner->as_back.image, &source_view) != BRE_OK)
            goto fail;
    } else {
        if(DeviceVkRectUpload(dev, src_fmt, src->pm_pixels, src->pm_row_bytes, src_rect->x, src_rect->y, w, h, &uploads[0]) != BRE_OK)
            goto fail;
        source_view = uploads[0].view;
    }

    if(clut != NULL && DeviceVkRectUpload(dev, clut_fmt, clut->pixels, clut->row_bytes, 0, 0, (br_uint_32)clut->width,
                                          (br_uint_32)clut->height, &uploads[1]) != BRE_OK)
        goto fail;

    upload_count = (clut != NULL) ? BR_ASIZE(uploads) : 1;

    /*
     * Record. Everything from here on references the resources handed to the retire
     * list at the end, which the current slot's fence covers.
     */
    {
        const VkCommandBuffer         cmd   = frame->command_buffer;
        const VkImageSubresourceRange range = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        };

        /* Each upload: undefined -> transfer destination -> shader read. */
        for(br_uint_32 i = 0; i < upload_count; ++i) {
            if(uploads[i].image == VK_NULL_HANDLE)
                continue; /* a device source has no upload in slot 0 */

            const VkImageMemoryBarrier upload_to_transfer = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = 0,
                .dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
                .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .image            = uploads[i].image,
                .subresourceRange = range,
            };
            vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                                   &upload_to_transfer);

            const VkBufferImageCopy copy_region = {
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                .imageOffset       = {0, 0, 0},
                .imageExtent       = {uploads[i].width, uploads[i].height, 1},
            };
            vk->CmdCopyBufferToImage(cmd, uploads[i].staging, uploads[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

            const VkImageMemoryBarrier upload_to_sample = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT,
                .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .image            = uploads[i].image,
                .subresourceRange = range,
            };
            vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1,
                                   &upload_to_sample);
        }

        /*
         * A device source is sampled in place: bring it into SHADER_READ_ONLY for the
         * draw and put it back where it was afterwards, keeping as_back.layout accurate.
         */
        if(src_is_device) {
            source_layout = src_owner->as_back.layout;

            const VkImageMemoryBarrier source_to_read = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = VK_ACCESS_MEMORY_WRITE_BIT,
                .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT,
                .oldLayout        = source_layout,
                .newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .image            = src_owner->as_back.image,
                .subresourceRange = range,
            };
            vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1,
                                   &source_to_read);
            src_owner->as_back.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        /*
         * Destination: colour attachment for the render scope, then back to GENERAL.
         * GENERAL is the driver's resting layout - the staging copy and the blit both
         * assume it - and loadOp LOAD keeps everything the quad does not cover.
         */
        const VkImageMemoryBarrier dst_to_colour = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout        = dst->as_back.layout,
            .newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image            = dst->as_back.image,
            .subresourceRange = range,
        };
        vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL,
                               1, &dst_to_colour);

        const VkViewport viewport = {
            .x        = (float)dst_rect->x,
            .y        = (float)dst_rect->y,
            .width    = (float)dst_rect->w,
            .height   = (float)dst_rect->h,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        const VkRect2D area = {
            .offset = {dst_rect->x,             dst_rect->y            },
            .extent = {(br_uint_32)dst_rect->w, (br_uint_32)dst_rect->h},
        };
        vk->CmdSetViewport(cmd, 0, 1, &viewport);
        vk->CmdSetScissor(cmd, 0, 1, &area);

        const VkRenderingAttachmentInfo colour = {
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = dst->as_back.view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        };
        const VkRenderingInfo rendering = {
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea           = area,
            .layerCount           = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments    = &colour,
        };
        vk->CmdBeginRendering(cmd, &rendering);

        vk->CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        /*
         * Binding 0 is the source and binding 1 the palette, so one write of two covers
         * the indexed case: a descriptorCount larger than a binding's array continues into
         * the consecutive binding, which the consecutive-binding update rule defines for
         * bindings of identical type and stage flags, as these two are. A plain draw
         * writes binding 0 alone and leaves the palette binding undefined, which its
         * shader does not statically use.
         */
        const VkDescriptorImageInfo image_desc[2] = {
            {
             .sampler     = dev->rect_sampler,
             .imageView   = source_view,
             .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
             },
            {
             .sampler     = dev->rect_sampler,
             .imageView   = uploads[1].view,
             .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
             },
        };
        const VkWriteDescriptorSet descriptor_write = {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding      = 0,
            .descriptorCount = upload_count,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = image_desc,
        };
        vk->CmdPushDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dev->rect_pipeline_layout, 0, 1, &descriptor_write);

        /*
         * A host upload is the region alone, so its shader rect starts at the origin; a
         * device source is the whole image, so the rect starts at the source rect offset
         * into the owner by the source's base.
         */
        const float shader_rect[4] = {src_is_device ? (float)(src_base_x + src_rect->x) : 0.0f,
                                      src_is_device ? (float)(src_base_y + src_rect->y) : 0.0f, (float)w, (float)h};
        vk->CmdPushConstants(cmd, dev->rect_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                             sizeof(shader_rect), shader_rect);

        vk->CmdDraw(cmd, 6, 1, 0, 0);

        vk->CmdEndRendering(cmd);

        const VkImageMemoryBarrier dst_to_general = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout        = VK_IMAGE_LAYOUT_GENERAL,
            .image            = dst->as_back.image,
            .subresourceRange = range,
        };
        vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL,
                               1, &dst_to_general);
        dst->as_back.layout = VK_IMAGE_LAYOUT_GENERAL;

        /*
         * Put the device source back into the layout it was in once the sampling is done,
         * and keep the pixelmap's record of that layout truthful.
         */
        if(src_is_device) {
            const VkImageMemoryBarrier source_restore = {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask    = VK_ACCESS_SHADER_READ_BIT,
                .dstAccessMask    = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
                .oldLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .newLayout        = source_layout,
                .image            = src_owner->as_back.image,
                .subresourceRange = range,
            };
            vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1,
                                   &source_restore);
            src_owner->as_back.layout = source_layout;
        }
    }

    /*
     * Every resource the recorded command buffer references is retired against this slot's
     * fence - it is not necessarily submitted yet - so nothing is destroyed while work may
     * still be reading it, and nothing leaks.
     */
    for(br_uint_32 i = 0; i < upload_count; ++i) {
        DevicePixelmapVkRetireImage(frame, uploads[i].image, uploads[i].view, uploads[i].allocation);
        DevicePixelmapVkRetireBuffer(frame, uploads[i].staging, uploads[i].staging_allocation);
    }

    /* A device source's transient view is ours alone; the image stays the source's. */
    if(source_view != VK_NULL_HANDLE && source_view != uploads[0].view)
        DevicePixelmapVkRetireImage(frame, VK_NULL_HANDLE, source_view, NULL);

    return BRE_OK;

fail:
    DeviceVkRectUploadDestroy(dev, &uploads[0]);
    DeviceVkRectUploadDestroy(dev, &uploads[1]);
    if(source_view != VK_NULL_HANDLE && source_view != uploads[0].view)
        vk->DestroyImageView(dev->vk_device, source_view, NULL);
    return BRE_FAIL;
}
