/*
 * Pixelmap -> device text draw on the GPU.
 *
 * One character is one instance of a glyph quad: the per-instance vertex data carries the
 * glyph's rectangle in destination pixels and the glyph index with its width within the
 * atlas layer, and the shaders sample that layer of a sampler2DArray. A per-font glyph
 * atlas is a 2D array image with one layer per character code, built from the font's
 * bitmaps through BrPixelmapText() into a host pixelmap and uploaded once. The glyph's
 * per-character width therefore reaches the shader as per-instance data: the CPU knows
 * the font's width table, so it normalises each glyph's width by the atlas width and
 * bakes it into the instance, and no per-glyph width table needs a buffer of its own.
 *
 * Everything coloured is a push constant - the text colour and the destination size -
 * and the glyphs are looked up through a push descriptor, so as with the rect draw the
 * machinery needs dynamic rendering and push descriptors. The pipeline is specialised by
 * destination format and sample count for the same reason the rect and line pipelines
 * are: with dynamic rendering there is no VkRenderPass, so
 * VkPipelineRenderingCreateInfo::pColorAttachmentFormats names the attachment. The
 * pipeline layout, descriptor set layout, sampler, shader modules and all font atlases
 * are device-scoped and built on first use; the atlases are VMA images, so DeviceVkTextFini()
 * must run before the VMA allocator is destroyed - see the device's free().
 *
 * text() falls back to BRE_UNSUPPORTED whenever the draw cannot be served (a non-offscreen
 * destination, an indexed destination, a destination format the device cannot render to,
 * or no dynamic rendering/push descriptors), and it is decided before anything is recorded.
 * A font that is not one of the three the device keeps an atlas for returns BRE_FAIL
 * instead - there is no CPU fallback.
 */
#include "drv.h"
#include "brassert.h"

#include <stddef.h>
#include <string.h>

#include "text.vert.spv.h"
#include "text.frag.spv.h"

/*
 * \brief Number of glyphs, i.e. atlas layers.
 */
#define BR_VKREND_TEXT_GLYPH_COUNT 256

/*
 * \brief The push constant payload for one text draw.
 *
 * The fields must match the TextData block in text.vert.glsl and text.frag.glsl. In the
 * shaders' std430 layout the two used members end at 24 bytes, but the block is rounded
 * up to the 16-byte alignment of its vec4 first member; the trailing pair makes the C
 * struct that same 32 bytes so the declared push constant range covers the block exactly.
 */
typedef struct br_vk_text_data {
    float colour[4]; /* r, g, b, a in [0,1]. */
    float size[2];   /* destination width, height in pixels. */
    float padding[2];
} br_vk_text_data;

/*
 * \brief The per-instance vertex data for one glyph.
 *
 * rect is the glyph rectangle in destination pixels and glyph.x is the glyph's width over
 * the atlas layer's width, i.e. the u extent the quad samples; glyph.y is the character
 * code, which selects the atlas array layer.
 */
typedef struct br_vk_text_instance {
    float rect[4];  /* x, y, w, h of the glyph rectangle, in destination pixels. */
    float glyph[2]; /* x = u extent (width / atlas layer width), y = glyph index. */
} br_vk_text_instance;

/**
 * \brief Does the device support the given optimal-tiling features for a format?
 */
static br_boolean DeviceVkTextFormatSupported(br_device *dev, VkFormat format, VkFormatFeatureFlags required)
{
    VkFormatProperties props;

    dev->context.GetPhysicalDeviceFormatProperties(dev->physical_device->dev, format, &props);
    return (props.optimalTilingFeatures & required) == required;
}

/**
 * \brief Release the text machinery's device-scoped resources and mark it unavailable.
 */
void DeviceVkTextFini(br_device *dev)
{
    const GladVulkanContext *vk = &dev->context;

    if(dev->vk_device != NULL) {
        for(br_uint_32 i = 0; i < dev->text_pipeline_count; ++i) {
            if(dev->text_pipelines[i].pipeline != VK_NULL_HANDLE)
                vk->DestroyPipeline(dev->vk_device, dev->text_pipelines[i].pipeline, NULL);
            dev->text_pipelines[i].pipeline = VK_NULL_HANDLE;
        }

        if(dev->text_sampler != VK_NULL_HANDLE)
            vk->DestroySampler(dev->vk_device, dev->text_sampler, NULL);
        if(dev->text_pipeline_layout != VK_NULL_HANDLE)
            vk->DestroyPipelineLayout(dev->vk_device, dev->text_pipeline_layout, NULL);
        if(dev->text_set_layout != VK_NULL_HANDLE)
            vk->DestroyDescriptorSetLayout(dev->vk_device, dev->text_set_layout, NULL);
        if(dev->text_vert != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->text_vert, NULL);
        if(dev->text_frag != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->text_frag, NULL);
    }

    /*
     * The glyph atlases are VMA images, so this must run before the allocator is gone.
     */
    for(br_uint_32 i = 0; i < BR_VKREND_TEXT_FONT_COUNT; ++i) {
        br_device_vk_text_font *font = &dev->text_fonts[i];

        if(font->view != VK_NULL_HANDLE)
            vk->DestroyImageView(dev->vk_device, font->view, NULL);
        if(font->image != VK_NULL_HANDLE)
            vmaDestroyImage(dev->vma_allocator, font->image, font->allocation);

        font->image      = VK_NULL_HANDLE;
        font->view       = VK_NULL_HANDLE;
        font->allocation = NULL;
        font->max_width  = 0;
    }

    dev->text_vert            = VK_NULL_HANDLE;
    dev->text_frag            = VK_NULL_HANDLE;
    dev->text_set_layout      = VK_NULL_HANDLE;
    dev->text_pipeline_layout = VK_NULL_HANDLE;
    dev->text_sampler         = VK_NULL_HANDLE;
    dev->text_pipeline_count  = 0;
    dev->text_state           = BR_VK_TEXT_UNAVAILABLE;
}

/**
 * \brief Build the text machinery's device-scoped resources, once.
 *
 * Returns BR_VK_TEXT_READY if the renderer can draw, BR_VK_TEXT_UNAVAILABLE otherwise
 * (the result is remembered, so a device that cannot is only tried once).
 */
br_vk_text_state DeviceVkTextEnsureInit(br_device *dev)
{
    const GladVulkanContext *vk = &dev->context;
    VkResult                 res;

    if(dev->text_state != BR_VK_TEXT_UNINITIALISED)
        return dev->text_state;

    dev->text_state = BR_VK_TEXT_UNAVAILABLE;

    /*
     * Dynamic rendering supplies the rendering scope and push descriptors bind the glyph
     * atlas without a descriptor pool a later frame would still be reading.
     */
    if(!dev->dynamic_rendering || !dev->push_descriptor || dev->context.CmdBeginRendering == NULL || dev->context.CmdPushDescriptorSet == NULL) {
        BrLogInfo("VKREND", "Text draw disabled: dynamic rendering and/or push descriptors are unavailable.");
        return dev->text_state;
    }

    const VkShaderModuleCreateInfo vert_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_text_vert_spv),
        .pCode    = (const uint32_t *)vkrend_text_vert_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &vert_info, NULL, &dev->text_vert)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the text vertex shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo frag_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_text_frag_spv),
        .pCode    = (const uint32_t *)vkrend_text_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &frag_info, NULL, &dev->text_frag)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the text fragment shader module: VkResult = %d", res);
        goto fail;
    }

    /*
     * Binding 0 is the glyph atlas, statically used by the fragment shader alone.
     */
    const VkDescriptorSetLayoutBinding binding = {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    const VkDescriptorSetLayoutCreateInfo set_info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
        .bindingCount = 1,
        .pBindings    = &binding,
    };
    if((res = vk->CreateDescriptorSetLayout(dev->vk_device, &set_info, NULL, &dev->text_set_layout)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the text descriptor set layout: VkResult = %d", res);
        goto fail;
    }

    const VkPushConstantRange push_range = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset     = 0,
        .size       = sizeof(br_vk_text_data),
    };
    const VkPipelineLayoutCreateInfo layout_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &dev->text_set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &push_range,
    };
    if((res = vk->CreatePipelineLayout(dev->vk_device, &layout_info, NULL, &dev->text_pipeline_layout)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the text pipeline layout: VkResult = %d", res);
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
    if((res = vk->CreateSampler(dev->vk_device, &sampler_info, NULL, &dev->text_sampler)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the text sampler: VkResult = %d", res);
        goto fail;
    }

    dev->text_state = BR_VK_TEXT_READY;
    return dev->text_state;

fail:
    DeviceVkTextFini(dev);
    return dev->text_state;
}

/**
 * \brief Get (building on first use) the text pipeline for a destination format and sample
 * count.
 *
 * \return the pipeline, or VK_NULL_HANDLE if one could not be built.
 */
static VkPipeline DeviceVkTextGetPipeline(br_device *dev, VkFormat format, VkSampleCountFlagBits samples)
{
    const GladVulkanContext *vk = &dev->context;

    for(br_uint_32 i = 0; i < dev->text_pipeline_count; ++i) {
        if(dev->text_pipelines[i].format == format && dev->text_pipelines[i].samples == samples)
            return dev->text_pipelines[i].pipeline;
    }

    if(dev->text_pipeline_count >= BR_VKREND_MAX_TEXT_PIPELINES) {
        BrLogError("VKREND", "Too many text pipelines (%u) for destination format %d.", dev->text_pipeline_count, (int)format);
        return VK_NULL_HANDLE;
    }

    const VkPipelineShaderStageCreateInfo stages[2] = {
        {
         .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage  = VK_SHADER_STAGE_VERTEX_BIT,
         .module = dev->text_vert,
         .pName  = "main",
         },
        {
         .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = dev->text_frag,
         .pName  = "main",
         },
    };

    const VkVertexInputBindingDescription vertex_binding = {
        .binding   = 0,
        .stride    = sizeof(br_vk_text_instance),
        .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
    };
    const VkVertexInputAttributeDescription vertex_attributes[2] = {
        {
         .location = 0,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
         .offset   = offsetof(br_vk_text_instance, rect),
         },
        {
         .location = 1,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32_SFLOAT,
         .offset   = offsetof(br_vk_text_instance,                            glyph),
         },
    };
    const VkPipelineVertexInputStateCreateInfo input = {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &vertex_binding,
        .vertexAttributeDescriptionCount = BR_ASIZE(vertex_attributes),
        .pVertexAttributeDescriptions    = vertex_attributes,
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

    /*
     * GL's text draw blends with GL_SRC_ALPHA / GL_ONE_MINUS_SRC_ALPHA, and its fragment
     * shader outputs the tinted colour with the glyph's coverage as alpha. The same
     * factors for both colour and alpha reproduce that equation exactly, so nothing is
     * premultiplied here.
     */
    const VkPipelineColorBlendAttachmentState blend_attachment = {
        .blendEnable         = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
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
        .layout              = dev->text_pipeline_layout,
        .renderPass          = VK_NULL_HANDLE,
        .subpass             = 0,
    };

    VkPipeline pipeline;
    VkResult   res = vk->CreateGraphicsPipelines(dev->vk_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline);
    if(res != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the text pipeline for destination format %d: VkResult = %d", (int)format, res);
        return VK_NULL_HANDLE;
    }

    br_device_vk_text_pipeline *entry = &dev->text_pipelines[dev->text_pipeline_count++];
    entry->format                     = format;
    entry->samples                    = samples;
    entry->pipeline                   = pipeline;

    return pipeline;
}

/**
 * \brief Map a predefined font to its device-scoped atlas slot.
 *
 * \return the slot index, or -1 if the font is not one of the predefined fonts.
 */
static int DeviceVkTextFontIndex(br_font *font)
{
    if(font == BrFontFixed3x5)
        return 0;
    if(font == BrFontProp4x6)
        return 1;
    if(font == BrFontProp7x9)
        return 2;

    return -1;
}

/**
 * \brief Build one predefined font's glyph atlas, once.
 *
 * The atlas is a 2D array image with one layer per character code, each layer the font's
 * widest glyph by its height. Every layer is rendered on the CPU into a host pixelmap by
 * the generic text path and uploaded whole. The host pixelmap's type is BR_PMT_RGBA_8888,
 * whose image format B8G8R8A8_UNORM places the glyph's white plus its coverage in alpha,
 * which is what the fragment shader expects.
 */
static br_error DeviceVkTextBuildFont(br_device *dev, br_font *font, br_device_vk_text_font *atlas)
{
    const GladVulkanContext  *vk = &dev->context;
    const br_pixelmap_vk_fmt *fmt;
    VkImage                   image              = VK_NULL_HANDLE;
    VmaAllocation             allocation         = NULL;
    VkImageView               view               = VK_NULL_HANDLE;
    VkBuffer                  staging            = VK_NULL_HANDLE;
    VmaAllocation             staging_allocation = NULL;
    VkCommandBuffer           cmd                = VK_NULL_HANDLE;
    br_pixelmap              *glyph_pm           = NULL;
    br_uint_32                max_width;
    br_uint_32                height;
    VkResult                  res;

    if((fmt = DeviceVkGetFormatDetails(BR_PMT_RGBA_8888)) == NULL || fmt->format == VK_FORMAT_UNDEFINED)
        return BRE_FAIL;

    /*
     * The atlas layer must be as wide as the widest glyph, so a proportional font's widest
     * character fits; a fixed font's glyph_x is the width of all of them.
     */
    max_width = font->glyph_x;
    height    = font->glyph_y;
    for(br_uint_32 i = 0; i < BR_VKREND_TEXT_GLYPH_COUNT; ++i) {
        br_int_32 width = (font->flags & BR_FONTF_PROPORTIONAL) ? font->width[i] : font->glyph_x;

        if(width > (br_int_32)max_width)
            max_width = (br_uint_32)width;
    }

    if(max_width == 0 || height == 0)
        return BRE_FAIL;

    if(!DeviceVkTextFormatSupported(dev, fmt->format, VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
        return BRE_FAIL;

    const VkImageCreateInfo image_info = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .format        = fmt->format,
        .extent        = {max_width, height, 1},
        .mipLevels     = 1,
        .arrayLayers   = BR_VKREND_TEXT_GLYPH_COUNT,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo image_alloc_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    if((res = vmaCreateImage(dev->vma_allocator, &image_info, &image_alloc_info, &image, &allocation, NULL)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create a text glyph atlas image: VkResult = %d", res);
        goto fail;
    }

    const VkImageViewCreateInfo view_info = {
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image    = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        .format   = fmt->format,
        .subresourceRange =
            {
                               .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                               .baseMipLevel   = 0,
                               .levelCount     = 1,
                               .baseArrayLayer = 0,
                               .layerCount     = BR_VKREND_TEXT_GLYPH_COUNT,
                               },
    };

    if((res = vk->CreateImageView(dev->vk_device, &view_info, NULL, &view)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create a text glyph atlas image view: VkResult = %d", res);
        goto fail;
    }

    const br_uint_32 layer_bytes = max_width * height * fmt->bytes_per_pixel;
    const br_uint_32 total_bytes = layer_bytes * BR_VKREND_TEXT_GLYPH_COUNT;

    const VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = total_bytes,
        .usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo buffer_alloc_info = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    };

    if((res = vmaCreateBuffer(dev->vma_allocator, &buffer_info, &buffer_alloc_info, &staging, &staging_allocation, NULL)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create a text glyph atlas staging buffer: VkResult = %d", res);
        goto fail;
    }

    if((glyph_pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, max_width, height, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("VKREND", "Failed to allocate a pixelmap for the text glyph atlas.");
        goto fail;
    }

    {
        VmaAllocationInfo alloc_info;
        vmaGetAllocationInfo(dev->vma_allocator, staging_allocation, &alloc_info);

        br_uint_8 *mapped = (br_uint_8 *)alloc_info.pMappedData;

        for(br_uint_32 i = 0; i < BR_VKREND_TEXT_GLYPH_COUNT; ++i) {
            char c[2] = {(char)i, '\0'};

            BrPixelmapFill(glyph_pm, BR_COLOUR_RGBA(0, 0, 0, 0));
            BrPixelmapText(glyph_pm, -glyph_pm->origin_x, -glyph_pm->origin_y, BR_COLOUR_RGBA(255, 255, 255, 255), font, c);

            for(br_uint_32 row = 0; row < height; ++row) {
                memcpy(mapped + i * layer_bytes + row * max_width * fmt->bytes_per_pixel,
                       (const br_uint_8 *)glyph_pm->pixels + row * glyph_pm->row_bytes, max_width * fmt->bytes_per_pixel);
            }
        }

        vmaFlushAllocation(dev->vma_allocator, staging_allocation, 0, total_bytes);
    }

    {
        const VkCommandBufferAllocateInfo cmd_alloc_info = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = dev->vk_command_pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        if((res = vk->AllocateCommandBuffers(dev->vk_device, &cmd_alloc_info, &cmd)) != VK_SUCCESS) {
            BrLogError("VKREND", "Failed to allocate a text glyph atlas command buffer: VkResult = %d", res);
            goto fail;
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
            .layerCount     = BR_VKREND_TEXT_GLYPH_COUNT,
        };

        const VkImageMemoryBarrier to_transfer_dst = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = 0,
            .dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image            = image,
            .subresourceRange = range,
        };
        vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &to_transfer_dst);

        /*
         * One region covers every layer: imageExtent.depth is the per-layer depth (1 for a
         * 2D image) and layerCount selects all BR_VKREND_TEXT_GLYPH_COUNT layers, whose
         * tightly packed rows follow one another in the staging buffer.
         */
        const VkBufferImageCopy copy_region = {
            .bufferOffset      = 0,
            .bufferRowLength   = 0,
            .bufferImageHeight = 0,
            .imageSubresource  = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, BR_VKREND_TEXT_GLYPH_COUNT},
            .imageOffset       = {0, 0, 0},
            .imageExtent       = {max_width, height, 1},
        };
        vk->CmdCopyBufferToImage(cmd, staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

        const VkImageMemoryBarrier to_sample = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .image            = image,
            .subresourceRange = range,
        };
        vk->CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &to_sample);

        vk->EndCommandBuffer(cmd);

        const VkSubmitInfo submit_info = {
            .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers    = &cmd,
        };
        if((res = vk->QueueSubmit(dev->vk_queue, 1, &submit_info, VK_NULL_HANDLE)) != VK_SUCCESS) {
            BrLogError("VKREND", "Failed to submit a text glyph atlas upload: VkResult = %d", res);
            goto fail;
        }
        vk->QueueWaitIdle(dev->vk_queue);

        vk->FreeCommandBuffers(dev->vk_device, dev->vk_command_pool, 1, &cmd);
        cmd = VK_NULL_HANDLE;
    }

    BrPixelmapFree(glyph_pm);
    vmaDestroyBuffer(dev->vma_allocator, staging, staging_allocation);

    atlas->image      = image;
    atlas->view       = view;
    atlas->allocation = allocation;
    atlas->max_width  = max_width;

    return BRE_OK;

fail:
    if(cmd != VK_NULL_HANDLE)
        vk->FreeCommandBuffers(dev->vk_device, dev->vk_command_pool, 1, &cmd);
    if(glyph_pm != NULL)
        BrPixelmapFree(glyph_pm);
    if(staging != VK_NULL_HANDLE)
        vmaDestroyBuffer(dev->vma_allocator, staging, staging_allocation);
    if(view != VK_NULL_HANDLE)
        vk->DestroyImageView(dev->vk_device, view, NULL);
    if(image != VK_NULL_HANDLE)
        vmaDestroyImage(dev->vma_allocator, image, allocation);

    return BRE_FAIL;
}

/**
 * \brief Draw a string into an offscreen pixelmap.
 *
 * point is the starting point in destination base coordinates (that is, with the
 * pixelmap's origin already applied), as PixelmapPointClip() returns it, and colour is
 * r, g, b, a in [0,1]. Each character advances by its width plus the font's implicit
 * one-pixel gap; a character whose glyph rectangle is entirely clipped ends the string,
 * as the rest of a left-to-right run past the edge is offscreen too. Everything that can
 * make the GPU path impossible is decided before anything is recorded, so BRE_UNSUPPORTED
 * means the caller keeps the CPU path.
 */
br_error DeviceVkTextDraw(br_device_pixelmap *dst, br_point *point, br_font *font, const char *text, const float colour[4])
{
    br_device                   *dev = dst->device;
    const GladVulkanContext     *vk  = &dev->context;
    const br_pixelmap_vk_fmt    *dst_fmt;
    br_device_pixelmap_vk_frame *frame;
    br_device_vk_text_font      *atlas;
    br_vk_text_instance         *instances;
    VkBuffer                     buffer     = VK_NULL_HANDLE;
    VmaAllocation                allocation = NULL;
    VkPipeline                   pipeline;
    br_uint_32                   capacity;
    br_uint_32                   count = 0;
    br_int_32                    x     = point->x;
    int                          font_index;
    VkResult                     res;

    if((dst_fmt = DeviceVkGetFormatDetails(dst->pm_type)) == NULL)
        return BRE_UNSUPPORTED;

    if(dst_fmt->format == VK_FORMAT_UNDEFINED)
        return BRE_UNSUPPORTED;

    if(DeviceVkTextEnsureInit(dev) != BR_VK_TEXT_READY)
        return BRE_UNSUPPORTED;

    /*
     * The same query DevicePixelmapVkCreateImage() used to decide the image's colour-
     * attachment usage, so an image that could not be rendered to cannot reach here.
     */
    if(!DeviceVkTextFormatSupported(dev, dst_fmt->format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
        return BRE_UNSUPPORTED;

    /* vkrend creates every image single-sampled, so that is the only sample count here. */
    if((pipeline = DeviceVkTextGetPipeline(dev, dst_fmt->format, VK_SAMPLE_COUNT_1_BIT)) == VK_NULL_HANDLE)
        return BRE_FAIL;

    if((font_index = DeviceVkTextFontIndex(font)) < 0)
        return BRE_FAIL;

    atlas = &dev->text_fonts[font_index];
    if(atlas->image == VK_NULL_HANDLE) {
        if(DeviceVkTextBuildFont(dev, font, atlas) != BRE_OK)
            return BRE_FAIL;
    }

    if((capacity = (br_uint_32)strlen(text)) == 0)
        return BRE_OK;

    {
        const br_uint_32 size = sizeof(*instances) * capacity;

        const VkBufferCreateInfo buffer_info = {
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = size,
            .usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        const VmaAllocationCreateInfo alloc_info = {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        if((res = vmaCreateBuffer(dev->vma_allocator, &buffer_info, &alloc_info, &buffer, &allocation, NULL)) != VK_SUCCESS) {
            BrLogError("VKREND", "Failed to create the text instance buffer: VkResult = %d", res);
            return BRE_FAIL;
        }

        VmaAllocationInfo mapped_info;
        vmaGetAllocationInfo(dev->vma_allocator, allocation, &mapped_info);
        instances = (br_vk_text_instance *)mapped_info.pMappedData;
    }

    /*
     * Build one instance per character, clipping each glyph rectangle to the destination.
     * A rectangle that clips away ends the string.
     */
    for(br_uint_32 i = 0; i < capacity; ++i) {
        const br_uint_8 ch = (br_uint_8)text[i];
        br_int_32       width;
        br_rectangle    rect;
        br_rectangle    clip;

        width = (font->flags & BR_FONTF_PROPORTIONAL) ? font->width[ch] : font->glyph_x;
        if(width < 0)
            width = 0;

        rect.x = x;
        rect.y = point->y;
        rect.w = width;
        rect.h = font->glyph_y;

        clip.x = rect.x < 0 ? 0 : rect.x;
        clip.y = rect.y < 0 ? 0 : rect.y;
        clip.w = (rect.x + rect.w) > dst->pm_width ? dst->pm_width - clip.x : rect.x + rect.w - clip.x;
        clip.h = (rect.y + rect.h) > dst->pm_height ? dst->pm_height - clip.y : rect.y + rect.h - clip.y;

        if(clip.w <= 0 || clip.h <= 0)
            break;

        instances[count].rect[0]  = (float)clip.x;
        instances[count].rect[1]  = (float)clip.y;
        instances[count].rect[2]  = (float)clip.w;
        instances[count].rect[3]  = (float)clip.h;
        instances[count].glyph[0] = (float)width / (float)atlas->max_width;
        instances[count].glyph[1] = (float)ch;
        ++count;

        x += width + 1;
    }

    if(count == 0) {
        vmaDestroyBuffer(dev->vma_allocator, buffer, allocation);
        return BRE_OK;
    }

    vmaFlushAllocation(dev->vma_allocator, allocation, 0, sizeof(*instances) * count);

    frame = DevicePixelmapVkGetCurrentFrame(dst->screen);

    {
        const VkCommandBuffer         cmd   = frame->command_buffer;
        const VkImageSubresourceRange range = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        };

        /*
         * Destination: colour attachment for the render scope, then back to GENERAL.
         * GENERAL is the driver's resting layout - the staging copy and the blit both
         * assume it - and loadOp LOAD keeps everything the glyphs do not cover.
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

        /*
         * The viewport is the whole destination image and the shader places each glyph in
         * it, so the scissor is the whole image too.
         */
        const VkViewport viewport = {
            .x        = 0.0f,
            .y        = 0.0f,
            .width    = (float)dst->pm_width,
            .height   = (float)dst->pm_height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        const VkRect2D area = {
            .offset = {0,             0             },
            .extent = {dst->pm_width, dst->pm_height},
        };
        vk->CmdSetViewport(cmd, 0, 1, &viewport);
        vk->CmdSetScissor(cmd, 0, 1, &area);

        const VkRenderingAttachmentInfo colour_attachment = {
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
            .pColorAttachments    = &colour_attachment,
        };
        vk->CmdBeginRendering(cmd, &rendering);

        vk->CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        const VkDeviceSize offset = 0;
        vk->CmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);

        const VkDescriptorImageInfo image_desc = {
            .sampler     = dev->text_sampler,
            .imageView   = atlas->view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        const VkWriteDescriptorSet descriptor_write = {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding      = 0,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = &image_desc,
        };
        vk->CmdPushDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dev->text_pipeline_layout, 0, 1, &descriptor_write);

        const br_vk_text_data data = {
            .colour = {colour[0], colour[1], colour[2], colour[3]},
            .size   = {(float)dst->pm_width, (float)dst->pm_height},
        };
        vk->CmdPushConstants(cmd, dev->text_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(data), &data);

        /* Six vertices per glyph, one instance per character. */
        vk->CmdDraw(cmd, 6, count, 0, 0);

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
    }

    /*
     * The instance buffer may still be referenced by the recorded command buffer, so it
     * is retired against this slot's fence rather than freed here.
     */
    DevicePixelmapVkRetireBuffer(frame, buffer, allocation);

    return BRE_OK;
}
