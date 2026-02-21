/*
 * Pixelmap -> device line and point draw on the GPU.
 *
 * A line or point is clipped to the destination and then drawn as a short, vertex-
 * buffer-less draw: gl_VertexIndex selects the start or end push constant for a line,
 * and a point is a one-vertex point primitive. The destination size is pushed too, so
 * the vertex shader places each endpoint at the pixel centre in NDC, and the fragment
 * shader outputs the pushed colour. There is no texture and no per-draw resource, so
 * unlike the rect draw there is nothing to upload and nothing to retire.
 *
 * The pipeline is specialised by the destination format and sample count for the same
 * reason the rect pipeline is - VK_KHR_dynamic_rendering has no VkRenderPass to derive
 * the attachment formats from, so VkPipelineRenderingCreateInfo::pColorAttachmentFormats
 * names it - and by topology, because a line list and a point list are different
 * VkPipelineInputAssemblyStateCreateInfo, and by plain/indexed source shape, because an
 * indexed destination takes the integer-output fragment program. It is device-scoped and
 * built on first use; a VkPipeline is a VkDevice object that outlives any surface, window
 * or frame, so none of it uses the per-frame retire machinery.
 *
 * line() and pixelSet() fall back to BRE_UNSUPPORTED whenever the draw cannot be served
 * (a non-offscreen destination, a destination format the device cannot render to, or no
 * dynamic rendering), and it is decided before anything is recorded. An indexed
 * destination is drawn with the integer-output fragment program and the index carried in
 * the colour, as the indexed fill does.
 */
#include "drv.h"
#include "brassert.h"

#include <string.h>

#include "line.vert.spv.h"
#include "line.frag.spv.h"
#include "line_int.frag.spv.h"

/**
 * \brief The push constant payload for one line or point draw.
 *
 * The fields must match the LineData block in line.vert.glsl and line.frag.glsl. In the
 * shaders' std430 layout the four used members end at 40 bytes, but the block is rounded
 * up to the 16-byte alignment of its vec4 first member; the trailing pair makes the C
 * struct that same 48 bytes so the declared push constant range covers the block exactly.
 */
typedef struct br_vk_line_data {
    float colour[4]; /* r, g, b, a in [0,1]. */
    float size[2];   /* destination width, height in pixels. */
    float start[2];  /* start endpoint, in destination pixels. */
    float end[2];    /* end endpoint, in destination pixels. */
    float padding[2];
} br_vk_line_data;

/**
 * \brief Does the device support the given optimal-tiling features for a format?
 */
static br_boolean DeviceVkLineFormatSupported(br_device *dev, VkFormat format, VkFormatFeatureFlags required)
{
    VkFormatProperties props;

    dev->context.GetPhysicalDeviceFormatProperties(dev->physical_device->dev, format, &props);
    return (props.optimalTilingFeatures & required) == required;
}

/**
 * \brief Release the line machinery's device-scoped resources and mark it unavailable.
 */
void DeviceVkLineFini(br_device *dev)
{
    const GladVulkanContext *vk = &dev->context;

    if(dev->vk_device != NULL) {
        for(br_uint_32 i = 0; i < dev->line_pipeline_count; ++i) {
            if(dev->line_pipelines[i].pipeline != VK_NULL_HANDLE)
                vk->DestroyPipeline(dev->vk_device, dev->line_pipelines[i].pipeline, NULL);
            dev->line_pipelines[i].pipeline = VK_NULL_HANDLE;
        }

        if(dev->line_pipeline_layout != VK_NULL_HANDLE)
            vk->DestroyPipelineLayout(dev->vk_device, dev->line_pipeline_layout, NULL);
        if(dev->line_vert != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->line_vert, NULL);
        if(dev->line_frag != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->line_frag, NULL);
        if(dev->line_frag_int != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->line_frag_int, NULL);
    }

    dev->line_vert            = VK_NULL_HANDLE;
    dev->line_frag            = VK_NULL_HANDLE;
    dev->line_frag_int        = VK_NULL_HANDLE;
    dev->line_pipeline_layout = VK_NULL_HANDLE;
    dev->line_pipeline_count  = 0;
    dev->line_state           = BR_VK_LINE_UNAVAILABLE;
}

/**
 * \brief Build the line machinery's device-scoped resources, once.
 *
 * Returns BR_VK_LINE_READY if the renderer can draw, BR_VK_LINE_UNAVAILABLE otherwise
 * (the result is remembered, so a device that cannot is only tried once).
 */
br_vk_line_state DeviceVkLineEnsureInit(br_device *dev)
{
    const GladVulkanContext *vk = &dev->context;
    VkResult                 res;

    if(dev->line_state != BR_VK_LINE_UNINITIALISED)
        return dev->line_state;

    dev->line_state = BR_VK_LINE_UNAVAILABLE;

    /*
     * Dynamic rendering supplies the rendering scope. Unlike the rect draw there is no
     * source binding, so push descriptors are not needed.
     */
    if(!dev->dynamic_rendering || dev->context.CmdBeginRendering == NULL) {
        BrLogInfo("VKREND", "Line draw disabled: dynamic rendering is unavailable.");
        return dev->line_state;
    }

    const VkShaderModuleCreateInfo vert_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_line_vert_spv),
        .pCode    = (const uint32_t *)vkrend_line_vert_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &vert_info, NULL, &dev->line_vert)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the line vertex shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo frag_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_line_frag_spv),
        .pCode    = (const uint32_t *)vkrend_line_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &frag_info, NULL, &dev->line_frag)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the line fragment shader module: VkResult = %d", res);
        goto fail;
    }

    /*
     * The same vertex shader feeds the integer-output program used to draw on an indexed
     * (R8_UINT) destination; only the fragment stage differs.
     */
    const VkShaderModuleCreateInfo frag_int_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_line_int_frag_spv),
        .pCode    = (const uint32_t *)vkrend_line_int_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &frag_int_info, NULL, &dev->line_frag_int)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the line indexed fragment shader module: VkResult = %d", res);
        goto fail;
    }

    /*
     * One push constant block carries the whole draw: the colour, the destination size
     * and the endpoints. There is no descriptor set at all, so the layout has none.
     */
    const VkPushConstantRange push_range = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset     = 0,
        .size       = sizeof(br_vk_line_data),
    };
    const VkPipelineLayoutCreateInfo layout_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 0,
        .pSetLayouts            = NULL,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &push_range,
    };
    if((res = vk->CreatePipelineLayout(dev->vk_device, &layout_info, NULL, &dev->line_pipeline_layout)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the line pipeline layout: VkResult = %d", res);
        goto fail;
    }

    dev->line_state = BR_VK_LINE_READY;
    return dev->line_state;

fail:
    DeviceVkLineFini(dev);
    return dev->line_state;
}

/**
 * \brief Get (building on first use) the line pipeline for a destination format, sample
 * count and topology.
 *
 * \return the pipeline, or VK_NULL_HANDLE if one could not be built.
 */
static VkPipeline DeviceVkLineGetPipeline(br_device *dev, VkFormat format, VkSampleCountFlagBits samples, VkPrimitiveTopology topology,
                                          br_boolean indexed)
{
    const GladVulkanContext *vk = &dev->context;

    for(br_uint_32 i = 0; i < dev->line_pipeline_count; ++i) {
        if(dev->line_pipelines[i].format == format && dev->line_pipelines[i].samples == samples &&
           dev->line_pipelines[i].topology == topology && dev->line_pipelines[i].indexed == indexed)
            return dev->line_pipelines[i].pipeline;
    }

    if(dev->line_pipeline_count >= BR_VKREND_MAX_LINE_PIPELINES) {
        BrLogError("VKREND", "Too many line pipelines (%u) for destination format %d.", dev->line_pipeline_count, (int)format);
        return VK_NULL_HANDLE;
    }

    const VkPipelineShaderStageCreateInfo stages[2] = {
        {
         .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage  = VK_SHADER_STAGE_VERTEX_BIT,
         .module = dev->line_vert,
         .pName  = "main",
         },
        {
         .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = indexed ? dev->line_frag_int : dev->line_frag,
         .pName  = "main",
         },
    };

    const VkPipelineVertexInputStateCreateInfo input = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    const VkPipelineInputAssemblyStateCreateInfo assembly = {
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = topology,
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
     * GL's line draw blends with GL_SRC_ALPHA / GL_ONE_MINUS_SRC_ALPHA, so its result is
     * rgb*a + dst*(1-a). The fragment shader premultiplies, making the colour factors
     * ONE / ONE_MINUS_SRC_ALPHA and the same result; the alpha factors stay SRC_ALPHA /
     * ONE_MINUS_SRC_ALPHA, which is what GL used for the alpha channel too.
     *
     * An indexed (R8_UINT) destination is an integer attachment, which Vulkan forbids
     * blending against, and it has only an R component, so blend is off and the write mask
     * covers R alone. The integer fragment program writes the CLUT index there.
     */
    const VkPipelineColorBlendAttachmentState blend_attachment = {
        .blendEnable         = indexed ? VK_FALSE : VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask      = indexed ? VK_COLOR_COMPONENT_R_BIT
                                       : (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT),
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
        .layout              = dev->line_pipeline_layout,
        .renderPass          = VK_NULL_HANDLE,
        .subpass             = 0,
    };

    VkPipeline pipeline;
    VkResult   res = vk->CreateGraphicsPipelines(dev->vk_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline);
    if(res != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the line pipeline for destination format %d: VkResult = %d", (int)format, res);
        return VK_NULL_HANDLE;
    }

    br_device_vk_line_pipeline *entry = &dev->line_pipelines[dev->line_pipeline_count++];
    entry->format                     = format;
    entry->samples                    = samples;
    entry->topology                   = topology;
    entry->indexed                    = indexed;
    entry->pipeline                   = pipeline;

    return pipeline;
}

/**
 * \brief Draw a clipped line or point into an offscreen pixelmap.
 *
 * start and end are destination pixels and colour is r, g, b, a in [0,1]. A point draw
 * passes the same point for both. `indexed` selects the integer-output program and means
 * colour.x is the CLUT index rather than a normalised red, as the caller sets it for an
 * INDEX_8 destination. Everything that can make the GPU path impossible is decided before
 * anything is recorded, so BRE_UNSUPPORTED means the caller can keep the CPU path.
 */
br_error DeviceVkLineDraw(br_device_pixelmap *dst, br_point *start, br_point *end, const float colour[4], br_boolean point, br_boolean indexed)
{
    br_device                   *dev = dst->device;
    const GladVulkanContext     *vk  = &dev->context;
    const br_pixelmap_vk_fmt    *dst_fmt;
    br_device_pixelmap_vk_frame *frame;
    VkPipeline                   pipeline;
    const VkPrimitiveTopology    topology = point ? VK_PRIMITIVE_TOPOLOGY_POINT_LIST : VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    if((dst_fmt = DeviceVkGetFormatDetails(dst->pm_type)) == NULL)
        return BRE_UNSUPPORTED;

    if(dst_fmt->format == VK_FORMAT_UNDEFINED)
        return BRE_UNSUPPORTED;

    if(DeviceVkLineEnsureInit(dev) != BR_VK_LINE_READY)
        return BRE_UNSUPPORTED;

    /*
     * The same query DevicePixelmapVkCreateImage() used to decide the image's colour-
     * attachment usage, so an image that could not be rendered to cannot reach here.
     */
    if(!DeviceVkLineFormatSupported(dev, dst_fmt->format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
        return BRE_UNSUPPORTED;

    /* vkrend creates every image single-sampled, so that is the only sample count here. */
    if((pipeline = DeviceVkLineGetPipeline(dev, dst_fmt->format, VK_SAMPLE_COUNT_1_BIT, topology, indexed)) == VK_NULL_HANDLE)
        return BRE_FAIL;

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
         * assume it - and loadOp LOAD keeps everything the line or point does not cover.
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
         * The viewport is the whole destination image and the shader places each endpoint
         * at its pixel centre, so the scissor is the whole image too.
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

        const br_vk_line_data data = {
            .colour = {colour[0], colour[1], colour[2], colour[3]},
            .size   = {(float)dst->pm_width, (float)dst->pm_height},
            .start  = {(float)start->x, (float)start->y},
            .end    = {(float)end->x, (float)end->y},
        };
        vk->CmdPushConstants(cmd, dev->line_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(data), &data);

        vk->CmdDraw(cmd, point ? 1 : 2, 1, 0, 0);

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

    return BRE_OK;
}
