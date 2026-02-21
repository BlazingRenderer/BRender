/*
 * Geometry format for the order table buckets produced by a prior hidden-surface render,
 * plus the device-scoped scene-draw pipeline the buckets are drawn with.
 *
 * render() walks the buckets from last to first, restores each deferred primitive's cached
 * state into the renderer and draws it. A deferred primitive is either a whole
 * BRT_GEOMETRY_STORED group or one face as a BRT_TRIANGLE (the Z-sort/transparent split,
 * see gstored.c); both reach the rasteriser through the same pipeline and buffer bind.
 *
 * The pipeline is specialised, like the rect and line pipelines, because dynamic rendering
 * has no VkRenderPass to derive the attachment formats from, so
 * VkPipelineRenderingCreateInfo names them. The key is the eight splits device.h lists for
 * br_device_vk_render_pipeline - lighting, texture and index shape, cull, blend, depth,
 * shading mode and front-face winding - on top of the destination format and sample count.
 * It is device-scoped and built on first use.
 *
 * The push constant block carries the per-draw model-view, surface colour and lighting
 * factors. The projection and the frame's packed lights come from a scene block bound as a
 * push descriptor, which the vertex stage also needs for the projection.
 */
#include "drv.h"
#include "brassert.h"

#include "render.vert.spv.h"
#include "render.frag.spv.h"
#include "render.lit.frag.spv.h"
#include "render.textured.frag.spv.h"
#include "render.textured.indexed.frag.spv.h"
#include "render.lit.textured.frag.spv.h"
#include "render.lit.textured.indexed.frag.spv.h"

/*
 * Geometry format info. template
 */
#define F(f) offsetof(struct br_geometry_v1_buckets, f)

static br_tv_template_entry geometryV1BucketsTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),     F(identifier),        BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(RENDERER_FACILITY_O), F(renderer_facility), BRTV_QUERY | BRTV_ALL, BRTV_CONV_COPY},
    {BRT(FACILITY_O),          F(renderer_facility), BRTV_QUERY,            BRTV_CONV_COPY},
};
#undef F

/*
 * The push constant payload for one group draw. The layout must match the RenderData block
 * in render.common.glsl. A br_matrix4's floats are copied in row-major order; a GLSL mat4
 * built from them is the transpose of the BRender matrix, so `model_view * vec4(pos, 1)`
 * computes the BRender row-vector `pos * model_view`. The projection is not here: it comes
 * from the scene block, because the lighting model needs the unprojected model-view for the
 * normal and view position.
 */
typedef struct br_vk_render_data {
    float    model_view[16];   /* model_to_view, copied row-major; a GLSL mat4 reads it transposed. */
    float    colour[4];        /* (R, G, B, opacity) of the surface.                                   */
    float    factors[4];       /* ka, kd, ks, power.                                                   */
    float    map_linear[4];    /* m00, m01, m10, m11 - the map transform's linear part.                */
    float    map_translate[2]; /* m20, m21 - the map transform's translation.                          */
    uint32_t flags[2];         /* [0] bit 0 flat, bit 1 colour key, bits 2..31 cull 0..29; [1] cull 30..47, bit 18
                                  colour source, bit 19 prelighting, bit 20 GOURAUD, bits 21..22 mapping source. */
} br_vk_render_data;

/**
 * \brief Release the scene-draw machinery's device-scoped resources and mark it unavailable.
 */
void DeviceVkRenderFini(br_device *dev)
{
    const GladVulkanContext *vk = &dev->context;

    if(dev->vk_device != NULL) {
        for(br_uint_32 i = 0; i < dev->render_pipeline_count; ++i) {
            if(dev->render_pipelines[i].pipeline != VK_NULL_HANDLE)
                vk->DestroyPipeline(dev->vk_device, dev->render_pipelines[i].pipeline, NULL);
            dev->render_pipelines[i].pipeline = VK_NULL_HANDLE;
        }

        for(br_uint_32 i = 0; i < dev->render_sampler_count; ++i) {
            if(dev->render_samplers[i].sampler != VK_NULL_HANDLE)
                vk->DestroySampler(dev->vk_device, dev->render_samplers[i].sampler, NULL);
            dev->render_samplers[i].sampler = VK_NULL_HANDLE;
        }

        if(dev->render_pipeline_layout != VK_NULL_HANDLE)
            vk->DestroyPipelineLayout(dev->vk_device, dev->render_pipeline_layout, NULL);
        if(dev->render_set_layout != VK_NULL_HANDLE)
            vk->DestroyDescriptorSetLayout(dev->vk_device, dev->render_set_layout, NULL);
        if(dev->render_vert != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->render_vert, NULL);
        if(dev->render_frag != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->render_frag, NULL);
        if(dev->render_lit != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->render_lit, NULL);
        if(dev->render_textured != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->render_textured, NULL);
        if(dev->render_textured_indexed != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->render_textured_indexed, NULL);
        if(dev->render_lit_textured != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->render_lit_textured, NULL);
        if(dev->render_lit_textured_indexed != VK_NULL_HANDLE)
            vk->DestroyShaderModule(dev->vk_device, dev->render_lit_textured_indexed, NULL);
    }

    dev->render_vert                 = VK_NULL_HANDLE;
    dev->render_frag                 = VK_NULL_HANDLE;
    dev->render_lit                  = VK_NULL_HANDLE;
    dev->render_textured             = VK_NULL_HANDLE;
    dev->render_textured_indexed     = VK_NULL_HANDLE;
    dev->render_lit_textured         = VK_NULL_HANDLE;
    dev->render_lit_textured_indexed = VK_NULL_HANDLE;
    dev->render_set_layout           = VK_NULL_HANDLE;
    dev->render_pipeline_layout      = VK_NULL_HANDLE;
    dev->render_pipeline_count       = 0;
    dev->render_sampler_count        = 0;
    dev->render_state                = BR_VK_RENDER_UNAVAILABLE;
}

/**
 * \brief Build the scene-draw machinery's device-scoped resources, once.
 *
 * Returns BR_VK_RENDER_READY if a draw can be recorded, BR_VK_RENDER_UNAVAILABLE otherwise
 * (remembered, so a device that cannot is only tried once).
 */
br_vk_render_state DeviceVkRenderEnsureInit(br_device *dev)
{
    const GladVulkanContext *vk = &dev->context;
    VkResult                 res;

    if(dev->render_state != BR_VK_RENDER_UNINITIALISED)
        return dev->render_state;

    dev->render_state = BR_VK_RENDER_UNAVAILABLE;

    if(!dev->dynamic_rendering || !dev->push_descriptor || dev->context.CmdBeginRendering == NULL || dev->context.CmdPushDescriptorSet == NULL) {
        BrLogInfo("VKREND", "Scene draw disabled: dynamic rendering or push descriptors are unavailable.");
        return dev->render_state;
    }

    /*
     * The vertex shader statically writes gl_ClipDistance (render.vert.glsl), which needs
     * VkPhysicalDeviceFeatures::shaderClipDistance; with the feature off the shader module's
     * ClipDistance built-in is invalid, so there is no clip-free variant to fall back to and
     * the scene draw stays unavailable rather than being built invalid.
     */
    if(!dev->shader_clip_distance) {
        BrLogError("VKREND", "Scene draw disabled: the device does not support shader clip distances (shaderClipDistance).");
        return dev->render_state;
    }

    const VkShaderModuleCreateInfo vert_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_render_vert_spv),
        .pCode    = (const uint32_t *)vkrend_render_vert_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &vert_info, NULL, &dev->render_vert)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the scene vertex shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo frag_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_render_frag_spv),
        .pCode    = (const uint32_t *)vkrend_render_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &frag_info, NULL, &dev->render_frag)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the scene fragment shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo lit_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_render_lit_frag_spv),
        .pCode    = (const uint32_t *)vkrend_render_lit_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &lit_info, NULL, &dev->render_lit)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the lit scene fragment shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo textured_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_render_textured_frag_spv),
        .pCode    = (const uint32_t *)vkrend_render_textured_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &textured_info, NULL, &dev->render_textured)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the textured scene fragment shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo lit_textured_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_render_lit_textured_frag_spv),
        .pCode    = (const uint32_t *)vkrend_render_lit_textured_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &lit_textured_info, NULL, &dev->render_lit_textured)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the lit textured scene fragment shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo textured_indexed_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_render_textured_indexed_frag_spv),
        .pCode    = (const uint32_t *)vkrend_render_textured_indexed_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &textured_indexed_info, NULL, &dev->render_textured_indexed)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the indexed textured scene fragment shader module: VkResult = %d", res);
        goto fail;
    }

    const VkShaderModuleCreateInfo lit_textured_indexed_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vkrend_render_lit_textured_indexed_frag_spv),
        .pCode    = (const uint32_t *)vkrend_render_lit_textured_indexed_frag_spv,
    };
    if((res = vk->CreateShaderModule(dev->vk_device, &lit_textured_indexed_info, NULL, &dev->render_lit_textured_indexed)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the indexed lit textured scene fragment shader module: VkResult = %d", res);
        goto fail;
    }

    /*
     * Binding 0 is the scene block, which the vertex stage reads the projection from and the
     * lit fragment stage reads the packed lights from, so both stages name it. Binding 1 is
     * the colour map, which only the textured fragment shaders statically use, and binding 2
     * is its palette, which only the indexed textured shaders use. Binding 3 is the
     * per-draw material block (fog for now), which every fragment shader reads for its fog
     * state, so only the fragment stage names it. A push descriptor that is not used can be
     * left undefined, so an untextured draw pushes bindings 0 and 3 and a plain textured
     * draw pushes 1 as well.
     */
    const VkDescriptorSetLayoutBinding scene_bindings[4] = {
        {
         .binding         = 0,
         .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount = 1,
         .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
         },
        {
         .binding         = 1,
         .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
        {
         .binding         = 2,
         .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
        {
         .binding         = 3,
         .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount = 1,
         .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
    };
    const VkDescriptorSetLayoutCreateInfo set_info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
        .bindingCount = BR_ASIZE(scene_bindings),
        .pBindings    = scene_bindings,
    };
    if((res = vk->CreateDescriptorSetLayout(dev->vk_device, &set_info, NULL, &dev->render_set_layout)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the scene descriptor set layout: VkResult = %d", res);
        goto fail;
    }

    const VkPushConstantRange push_range = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset     = 0,
        .size       = sizeof(br_vk_render_data),
    };
    const VkPipelineLayoutCreateInfo layout_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &dev->render_set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &push_range,
    };
    if((res = vk->CreatePipelineLayout(dev->vk_device, &layout_info, NULL, &dev->render_pipeline_layout)) != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the scene pipeline layout: VkResult = %d", res);
        goto fail;
    }

    dev->render_state = BR_VK_RENDER_READY;
    return dev->render_state;

fail:
    DeviceVkRenderFini(dev);
    return dev->render_state;
}

/*
 * The depth compare op a primitive state's depth_test selects, matching glrend's
 * DeviceGLExtractPrimitiveState() (drivers/glrend/v1model.c) and its default of GL_LESS.
 */
static VkCompareOp depth_compare_for(br_token depth_test)
{
    switch(depth_test) {
        case BRT_GREATER:
            return VK_COMPARE_OP_GREATER;

        case BRT_LESS_OR_EQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;

        case BRT_GREATER_OR_EQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;

        case BRT_EQUAL:
            return VK_COMPARE_OP_EQUAL;

        case BRT_NOT_EQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;

        case BRT_NEVER:
            return VK_COMPARE_OP_NEVER;

        case BRT_ALWAYS:
            return VK_COMPARE_OP_ALWAYS;

        case BRT_LESS:
        default:
            return VK_COMPARE_OP_LESS;
    }
}

/*
 * The fixed-function blend factors a primitive state's blend_mode selects, matching
 * glrend's apply_blend_mode() (drivers/glrend/v1model.c): standard and dimmed are
 * (srcAlpha, 1-srcAlpha), summed is (srcAlpha, one) and premultiplied is (one,
 * 1-srcAlpha). glrend's BlendFuncSeparate() calls blend the alpha channel (one, zero).
 */
static void blend_factors_for(br_token blend_mode, VkBlendFactor *src, VkBlendFactor *dst)
{
    switch(blend_mode) {
        case BRT_BLEND_SUMMED:
            *src = VK_BLEND_FACTOR_SRC_ALPHA;
            *dst = VK_BLEND_FACTOR_ONE;
            break;

        case BRT_BLEND_PREMULTIPLIED:
            *src = VK_BLEND_FACTOR_ONE;
            *dst = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;

        case BRT_BLEND_STANDARD:
        case BRT_BLEND_DIMMED:
        default:
            *src = VK_BLEND_FACTOR_SRC_ALPHA;
            *dst = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;
    }
}

/**
 * \brief Get (building on first use) the scene pipeline for a destination format, sample
 * count, lighting state, colour-map state, cull state, front-face winding and blend state.
 *
 * \return the pipeline, or VK_NULL_HANDLE if one could not be built.
 */
VkPipeline DeviceVkRenderGetPipeline(br_device *dev, VkFormat format, VkSampleCountFlagBits samples, br_boolean lit, br_boolean textured,
                                     br_boolean indexed, br_token shading_mode, VkCullModeFlags cull, VkFrontFace front_face,
                                     br_boolean blended, br_token blend_mode, br_boolean depth_test, br_boolean depth_write,
                                     VkCompareOp depth_compare, VkFormat depth_format)
{
    const GladVulkanContext *vk = &dev->context;
    VkShaderModule           frag;

    for(br_uint_32 i = 0; i < dev->render_pipeline_count; ++i) {
        if(dev->render_pipelines[i].format == format && dev->render_pipelines[i].samples == samples &&
           dev->render_pipelines[i].lit == lit && dev->render_pipelines[i].textured == textured &&
           dev->render_pipelines[i].indexed == indexed && dev->render_pipelines[i].shading_mode == shading_mode &&
           dev->render_pipelines[i].cull == cull && dev->render_pipelines[i].front_face == front_face &&
           dev->render_pipelines[i].blended == blended && dev->render_pipelines[i].blend_mode == blend_mode &&
           dev->render_pipelines[i].depth_format == depth_format && dev->render_pipelines[i].depth_test == depth_test &&
           dev->render_pipelines[i].depth_write == depth_write && dev->render_pipelines[i].depth_compare == depth_compare)
            return dev->render_pipelines[i].pipeline;
    }

    if(dev->render_pipeline_count >= BR_VKREND_MAX_RENDER_PIPELINES) {
        BrLogError("VKREND", "Too many scene pipelines (%u) for destination format %d.", dev->render_pipeline_count, (int)format);
        return VK_NULL_HANDLE;
    }

    if(lit)
        frag = textured ? (indexed ? dev->render_lit_textured_indexed : dev->render_lit_textured) : dev->render_lit;
    else
        frag = textured ? (indexed ? dev->render_textured_indexed : dev->render_textured) : dev->render_frag;

    const VkPipelineShaderStageCreateInfo stages[2] = {
        {
         .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage  = VK_SHADER_STAGE_VERTEX_BIT,
         .module = dev->render_vert,
         .pName  = "main",
         },
        {
         .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = frag,
         .pName  = "main",
         },
    };

    const VkVertexInputBindingDescription binding = {
        .binding   = 0,
        .stride    = sizeof(br_geometry_stored_vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    const VkVertexInputAttributeDescription attributes[4] = {
        {
         .location = 0,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32_SFLOAT,
         .offset   = offsetof(br_geometry_stored_vertex,        position),
         },
        {
         .location = 1,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32_SFLOAT,
         .offset   = offsetof(br_geometry_stored_vertex,                                         normal),
         },
        {
         .location = 2,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32_SFLOAT,
         .offset   = offsetof(br_geometry_stored_vertex,                                                               map),
         },
        {
         .location = 3,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
         .offset   = offsetof(br_geometry_stored_vertex,colour),
         },
    };
    const VkPipelineVertexInputStateCreateInfo input = {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &binding,
        .vertexAttributeDescriptionCount = BR_ASIZE(attributes),
        .pVertexAttributeDescriptions    = attributes,
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
        .cullMode    = cull,
        .frontFace   = front_face,
        .lineWidth   = 1.0f,
    };

    const VkPipelineMultisampleStateCreateInfo multisample = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = samples,
    };

    /*
     * Depth-stencil state. For a Zs scene (depth_format == VK_FORMAT_UNDEFINED) both
     * enables are off; for a Zb scene the test/write/compare come from the material state
     * the caller derived from glrend's apply_stored_properties(). When the test is off the
     * write must be off too, or the pipeline is invalid.
     */
    const VkPipelineDepthStencilStateCreateInfo depth_stencil = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable       = depth_test,
        .depthWriteEnable      = depth_write && depth_test,
        .depthCompareOp        = depth_compare,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
    };

    VkBlendFactor blend_src = VK_BLEND_FACTOR_ONE;
    VkBlendFactor blend_dst = VK_BLEND_FACTOR_ZERO;

    if(blended)
        blend_factors_for(blend_mode, &blend_src, &blend_dst);

    const VkPipelineColorBlendAttachmentState blend_attachment = {
        .blendEnable         = blended,
        .srcColorBlendFactor = blend_src,
        .dstColorBlendFactor = blend_dst,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
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

    const VkPipelineRenderingCreateInfo rendering = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &format,
        .depthAttachmentFormat   = depth_format,
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
        .pDepthStencilState  = &depth_stencil,
        .pColorBlendState    = &blend,
        .pDynamicState       = &dynamic,
        .layout              = dev->render_pipeline_layout,
        .renderPass          = VK_NULL_HANDLE,
        .subpass             = 0,
    };

    VkPipeline pipeline;
    VkResult   res = vk->CreateGraphicsPipelines(dev->vk_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline);
    if(res != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create the scene pipeline for destination format %d: VkResult = %d", (int)format, res);
        return VK_NULL_HANDLE;
    }

    br_device_vk_render_pipeline *entry = &dev->render_pipelines[dev->render_pipeline_count++];
    entry->format                       = format;
    entry->samples                      = samples;
    entry->lit                          = lit;
    entry->textured                     = textured;
    entry->indexed                      = indexed;
    entry->shading_mode                 = shading_mode;
    entry->cull                         = cull;
    entry->front_face                   = front_face;
    entry->blended                      = blended;
    entry->blend_mode                   = blend_mode;
    entry->depth_format                 = depth_format;
    entry->depth_test                   = depth_test;
    entry->depth_write                  = depth_write;
    entry->depth_compare                = depth_compare;
    entry->pipeline                     = pipeline;

    return pipeline;
}

/**
 * \brief Get (building on first use) the sampler for a primitive state's filter, mip filter
 * and wrap modes.
 *
 * The primitive defaults to nearest filtering with wrapping and no mip chain, so the common
 * case builds one sampler. A sampler is a VkDevice object independent of any draw, so the
 * cache lives with the device and is destroyed with it.
 *
 * `filter` (BRT_MAP_INTERPOLATION) chooses min/mag linear-vs-nearest. `mip_filter`
 * (BRT_MIP_INTERPOLATION) chooses whether a mip chain participates at all: BRT_LINEAR sets
 * VK_SAMPLER_MIPMAP_MODE_LINEAR and lets the LOD reach every level (glrend's
 * GL_LINEAR_MIPMAP_LINEAR), BRT_NONE sets nearest mipmap mode and pins maxLod to 0 - the
 * VkSamplerCreateInfo default - so only level 0 is sampled (glrend's GL_LINEAR /
 * GL_NEAREST with no mipmap). This mirrors glrend's table (drivers/glrend/v1model.c). An
 * unexpected token takes the nearest/none behaviour rather than asserting, so the draw is
 * not lost.
 *
 * \return the sampler, or VK_NULL_HANDLE if the cache is full.
 */
VkSampler DeviceVkRenderGetSampler(br_device *dev, br_token filter, br_token mip_filter, br_token wrap_s, br_token wrap_t)
{
    const GladVulkanContext *vk = &dev->context;

    for(br_uint_32 i = 0; i < dev->render_sampler_count; ++i) {
        if(dev->render_samplers[i].filter == filter && dev->render_samplers[i].mip_filter == mip_filter &&
           dev->render_samplers[i].wrap_s == wrap_s && dev->render_samplers[i].wrap_t == wrap_t)
            return dev->render_samplers[i].sampler;
    }

    if(dev->render_sampler_count >= BR_VKREND_MAX_RENDER_SAMPLERS) {
        BrLogError("VKREND", "Too many scene samplers (%u).", dev->render_sampler_count);
        return VK_NULL_HANDLE;
    }

    const VkFilter filter_mode = (filter == BRT_LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

    /*
     * BRT_LINEAR participates in the mip chain and lets the minification blend between
     * levels; BRT_NONE selects one level with nearest mipmap mode and clamps the LOD to 0 so
     * the chain is unreachable. Any other token is treated as BRT_NONE.
     */
    const br_boolean          mipmaps     = (mip_filter == BRT_LINEAR);
    const VkSamplerMipmapMode mipmap_mode = mipmaps ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;

    const VkSamplerAddressMode wrap_modes[3] = {VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};

    br_uint_32 addr_u = 0;
    br_uint_32 addr_v = 0;

    switch(wrap_s) {
        case BRT_CLAMP:
            addr_u = 1;
            break;
        case BRT_MIRROR:
            addr_u = 2;
            break;
        default:
            break;
    }

    switch(wrap_t) {
        case BRT_CLAMP:
            addr_v = 1;
            break;
        case BRT_MIRROR:
            addr_v = 2;
            break;
        default:
            break;
    }

    const VkSamplerCreateInfo sampler_info = {
        .sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter    = filter_mode,
        .minFilter    = filter_mode,
        .mipmapMode   = mipmap_mode,
        .addressModeU = wrap_modes[addr_u],
        .addressModeV = wrap_modes[addr_v],
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .maxLod       = mipmaps ? VK_LOD_CLAMP_NONE : 0.0f,
    };

    VkSampler sampler;
    VkResult  res = vk->CreateSampler(dev->vk_device, &sampler_info, NULL, &sampler);
    if(res != VK_SUCCESS) {
        BrLogError("VKREND", "Failed to create a scene sampler: VkResult = %d", res);
        return VK_NULL_HANDLE;
    }

    br_device_vk_render_sampler *entry = &dev->render_samplers[dev->render_sampler_count++];
    entry->filter                      = filter;
    entry->mip_filter                  = mip_filter;
    entry->wrap_s                      = wrap_s;
    entry->wrap_t                      = wrap_t;
    entry->sampler                     = sampler;

    return sampler;
}

/*
 * The core publishes a material's BRT_CULL on every draw as BRT_ONE_SIDED (the default),
 * BRT_TWO_SIDED or BRT_NONE; glrend enables GL_CULL_FACE + glCullFace(GL_BACK) for
 * BRT_ONE_SIDED and disables culling otherwise (drivers/glrend/v1model.c). Culling must
 * agree with the winding the rasteriser sees, which the scene vertex shader fixes by
 * emitting Vulkan clip space (render.vert.glsl negates y): a BRender front face - the
 * side the model normal points to - has a positive framebuffer area, the winding
 * VK_FRONT_FACE_COUNTER_CLOCKWISE calls front. That was measured, not assumed: building
 * the pipeline with the opposite front face culls the front-facing triangle of the
 * devpmtest witness. With culling off frontFace is inert. That is the non-mirrored case;
 * the front face itself is chosen per draw by front_face_for() below.
 */
static VkCullModeFlags cull_mode_for(const state_stack *state)
{
    if(!(state->valid & MASK_STATE_CULL))
        return VK_CULL_MODE_NONE;

    switch(state->cull.type) {
        case BRT_ONE_SIDED:
            return VK_CULL_MODE_BACK_BIT;

        case BRT_TWO_SIDED:
        case BRT_NONE:
        default:
            return VK_CULL_MODE_NONE;
    }
}

/*
 * The front face a draw's model-view selects, matching glrend's apply_state()
 * (drivers/glrend/v1model.c). When the upper 3x3 determinant of model_to_view is below
 * 1e-6 - which glrend reads as a reflection, and so includes a degenerate model-view - the
 * transformation changes handedness, the BRender front face is projected with the opposite
 * screen winding, and the rasteriser must be told the other winding is front. glrend flips
 * to GL_CW for that and stays GL_CCW otherwise; vkrend does the same with the Vulkan
 * enumerants, so a model-view that mirrors draws the same side glrend draws.
 *
 * The determinant is of the BRender row-vector model_to_view. The vertex shader builds a
 * GLSL mat4 from its row-major copy, which is the transpose; a transpose has the same
 * determinant, so the sign is exactly what the rasteriser sees.
 */
static VkFrontFace front_face_for(const br_matrix34 *model_to_view)
{
    return (BrScalarToFloat(BrMatrix34Determinant3(model_to_view)) < 1e-6f) ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

/*
 * Copy the current scene and per-draw state into the push constants and bind the geometry
 * buffers. The rendering scope, viewport and scene light block were set by sceneBegin().
 *
 * A colour map, when the primitive state carries one, is bound as a push descriptor and
 * selects the textured pipeline variant: the texel modulates the surface colour. The map's
 * transform was folded into the UV by the vertex shader.
 *
 * One index range is drawn: the whole group at its IBO offset, or one face's three
 * vertices at its own. Returns whether anything was drawn, so the caller can count the
 * triangle only when it reaches the rasteriser.
 */
static br_boolean VkV1ModelRenderIndexed(br_geometry_stored *geom, br_renderer *renderer, br_uint_32 count, br_uint_32 first_index)
{
    const state_stack        *state;
    br_device_pixelmap       *dst;
    br_device                *dev;
    const br_pixelmap_vk_fmt *dst_fmt;
    VkPipeline                pipeline;
    VkCullModeFlags           cull;
    VkFrontFace               front_face;
    br_boolean                blended;
    br_token                  blend_mode;
    VkFormat                  depth_format  = VK_FORMAT_UNDEFINED;
    br_boolean                depth_test    = BR_FALSE;
    br_boolean                depth_write   = BR_FALSE;
    VkCompareOp               depth_compare = VK_COMPARE_OP_LESS;
    VkCommandBuffer           cmd;
    br_boolean                lit;
    br_boolean                textured;
    br_boolean                indexed;
    br_token                  shading_mode;
    br_buffer_stored         *colour_map;
    br_vk_render_data         data = {0};
    br_vk_material_state     *material;
    VkDeviceSize              material_offset;

    if(!renderer->has_begun)
        return BR_FALSE;

    if((dst = renderer->scene_dst) == NULL)
        return BR_FALSE;

    if(renderer->scene_buffer == VK_NULL_HANDLE)
        return BR_FALSE;

    if(geom->index_buffer == VK_NULL_HANDLE || geom->vertex_buffer == VK_NULL_HANDLE || count == 0)
        return BR_FALSE;

    dev = dst->device;

    if((dst_fmt = DeviceVkGetFormatDetails(dst->pm_type)) == NULL)
        return BR_FALSE;

    const GladVulkanContext *vk = &dev->context;

    state = renderer->state.current;
    lit   = (state->valid & MASK_STATE_SURFACE) && state->surface.lighting;

    /*
     * The primitive's shading mode names where the lighting is evaluated. The core leaves
     * it at BRT_FLAT until a material sets BRT_SHADING_MODE_T, and a draw with no primitive
     * state has none, so both fall back to BRT_FLAT. BRT_FLAT is out of scope here and draws
     * as it always has (the flat-qualified normal, shaded per fragment); BRT_GOURAUD and
     * BRT_PHONG are the two paths this key separates.
     */
    shading_mode = (state->valid & MASK_STATE_PRIMITIVE) ? state->prim.shading_mode : BRT_FLAT;

    colour_map = ((state->valid & MASK_STATE_PRIMITIVE) && BufferStoredVkGetView(state->prim.colour_map) != VK_NULL_HANDLE) ? state->prim.colour_map
                                                                                                                            : NULL;
    textured = colour_map != NULL;

    /*
     * An INDEX_8 colour map draws through the indexed fragment shaders, which bind the
     * palette alongside the index plane. Its index texture is an integer format the sampler
     * cannot filter, so an indexed draw takes the sampler's filter as NEAREST regardless of
     * the primitive's.
     */
    indexed = textured && BufferStoredVkIsIndexed(colour_map);

    cull = cull_mode_for(state);

    /*
     * The front face is chosen from this draw's model-view, so a mirrored actor selects
     * the other winding's pipeline (the two are separate entries in the pipeline cache).
     */
    front_face = front_face_for(&state->matrix.model_to_view);

    /*
     * The core buckets a PRIMF_BLEND, blend-table or opacity < 1 primitive as transparent
     * (gstored.c) and glrend blends it with blend_mode's factors; without a blend the
     * bucket returns opaque. A non-blended primitive is given BRT_NONE so the pipeline key
     * does not multiply over an irrelevant blend_mode.
     */
    blended    = (state->valid & MASK_STATE_PRIMITIVE) && (state->prim.flags & PRIMF_BLEND);
    blend_mode = blended ? state->prim.blend_mode : BRT_NONE;

    /*
     * Depth state, derived with glrend's apply_stored_properties() (drivers/glrend/v1model.c):
     * a Zs scene has no depth attachment at all; a Zb scene tests depth unless the surface
     * forces front or back, writes depth when PRIMF_DEPTH_WRITE is set and the primitive is
     * not blended, and compares with the op the primitive's depth_test selects. The depth
     * format is part of the pipeline key because VkPipelineRenderingCreateInfo names it, and
     * it must match the attachment sceneBegin() opened.
     */
    if(renderer->scene_depth != NULL) {
        const br_pixelmap_vk_fmt *depth_fmt = DeviceVkGetFormatDetails(renderer->scene_depth->pm_type);

        if(depth_fmt == NULL)
            return BR_FALSE;

        depth_format = depth_fmt->format;

        if(state->valid & MASK_STATE_SURFACE)
            depth_test = !state->surface.force_front && !state->surface.force_back;

        if(state->valid & MASK_STATE_PRIMITIVE) {
            depth_write   = (state->prim.flags & PRIMF_DEPTH_WRITE) != 0;
            depth_compare = depth_compare_for(state->prim.depth_test);
        } else {
            depth_write = BR_TRUE;
        }

        if(blended)
            depth_write = BR_FALSE;
    }

    if((pipeline = DeviceVkRenderGetPipeline(dev, dst_fmt->format, VK_SAMPLE_COUNT_1_BIT, lit, textured, indexed, shading_mode, cull, front_face,
                                             blended, blend_mode, depth_test, depth_write, depth_compare, depth_format)) == VK_NULL_HANDLE)
        return BR_FALSE;

    for(int r = 0; r < 4; ++r) {
        for(int c = 0; c < 3; ++c)
            data.model_view[r * 4 + c] = (float)state->matrix.model_to_view.m[r][c];

        /*
         * A br_matrix34 is four rows of three columns, so as a homogeneous 4x4 its last
         * column is (0, 0, 0, 1). Copying a fourth column out of the struct would read the
         * first element of the next row - and for the last row, the first float of the next
         * member of the state block - corrupting the w the shader's `model_view * vec4(pos,
         * 1)` returns and so the whole clip position.
         */
        data.model_view[r * 4 + 3] = (r == 3) ? 1.0f : 0.0f;
    }

    if(state->valid & MASK_STATE_SURFACE) {
        data.colour[0] = BR_RED(state->surface.colour) / 255.0f;
        data.colour[1] = BR_GRN(state->surface.colour) / 255.0f;
        data.colour[2] = BR_BLU(state->surface.colour) / 255.0f;
        data.colour[3] = (float)state->surface.opacity;

        data.factors[0] = (float)state->surface.ka;
        data.factors[1] = (float)state->surface.kd;
        data.factors[2] = (float)state->surface.ks;
        data.factors[3] = (float)state->surface.power;
    } else {
        data.colour[0] = data.colour[1] = data.colour[2] = data.colour[3] = 1.0f;
    }

    /*
     * The map transform is a br_matrix23 in the BRender row-vector convention: `du = m00*u +
     * m10*v + m20`, `dv = m01*u + m11*v + m21`. map_linear holds the four linear terms and
     * map_translate the two constants; the vertex shader applies them.
     */
    data.map_linear[0]    = (float)state->surface.map_transform.m[0][0];
    data.map_linear[1]    = (float)state->surface.map_transform.m[0][1];
    data.map_linear[2]    = (float)state->surface.map_transform.m[1][0];
    data.map_linear[3]    = (float)state->surface.map_transform.m[1][1];
    data.map_translate[0] = (float)state->surface.map_transform.m[2][0];
    data.map_translate[1] = (float)state->surface.map_transform.m[2][1];

    /*
     * flags packs flat shading (bit 0), the colour key (bit 1), the per-model culled-light
     * mask in the bits above and the surface's colour source, pre-lighting state and the
     * GOURAUD shading bit in the high bits of flags[1], matching the decode in
     * render.common.glsl. The cull mask's first light sits at bit 2, so lights 0..29 fill
     * flags[0], 30..47 fill flags[1] bits 0..17, and bits 18, 19 and 20 carry the two
     * surface booleans and the shading bit.
     */
    data.flags[0] = (shading_mode == BRT_FLAT) ? 1u : 0u;
    data.flags[1] = 0;

    /*
     * A GOURAUD surface accumulates the lights in the vertex stage and shades with the
     * interpolated result; PHONG and the out-of-scope FLAT shade per fragment. The bit
     * rides with the push constants and only selects between the two shader paths, while
     * the shading mode in the pipeline key keeps their pipelines distinct.
     */
    if(shading_mode == BRT_GOURAUD)
        data.flags[1] |= (1u << 20);

    if((state->valid & MASK_STATE_PRIMITIVE) && (state->prim.flags & PRIMF_COLOUR_KEY))
        data.flags[0] |= 2u;

    /*
     * colour_source names where the surface colour comes from: the core sets it to
     * BRT_GEOMETRY (or BRT_VERTEX) for a material carrying per-vertex colours and to
     * BRT_SURFACE otherwise, the split glrend's apply_stored_properties() keys on. The
     * vertex colours are in the VBO, uploaded from the prepared model's vertex_colours.
     */
    if((state->valid & MASK_STATE_SURFACE) && state->surface.colour_source != BRT_SURFACE)
        data.flags[1] |= (1u << 18);

    /*
     * Prelighting is the BR_MATF_PRELIT | BR_MATF_LIGHT combination under which the core
     * writes a lighting result into the model's vertex_colours (core/v1db/prelight.c); the
     * fragment stage adds it to the shaded colour.
     */
    if((state->valid & MASK_STATE_SURFACE) && state->surface.prelighting)
        data.flags[1] |= (1u << 19);

    /*
     * The mapping source rides in the two flag bits above the shading bit (21..22): the core
     * maps BR_MATF_ENVIRONMENT_L and BR_MATF_ENVIRONMENT_I to BRT_ENVIRONMENT_LOCAL and
     * BRT_ENVIRONMENT_INFINITE (core/v1db/prepmatl.c), and the fragment stage computes the
     * sphere-map UV for those two and leaves the plain map-transform UV for every other
     * value (see mappingSource() in render.common.glsl).
     */
    if(state->valid & MASK_STATE_SURFACE) {
        if(state->surface.mapping_source == BRT_ENVIRONMENT_LOCAL)
            data.flags[1] |= (1u << 21);
        else if(state->surface.mapping_source == BRT_ENVIRONMENT_INFINITE)
            data.flags[1] |= (2u << 21);
    }

    for(br_uint_32 i = 0; i < renderer->scene_light_count; ++i) {
        br_uint_32 part = renderer->scene_light_part[i];
        br_uint_32 bit  = i + 2;

        if(!state->light[part].culled)
            continue;

        if(bit < 32)
            data.flags[0] |= (1u << bit);
        else
            data.flags[1] |= (1u << (bit - 32));
    }

    /*
     * The per-draw material block, from glrend's extract (drivers/glrend/v1model.c): fog is
     * on when the primitive's fog type is not BRT_NONE, its colour is the fog colour's RGB
     * over 255 with alpha 1, and fog_range is fog_min/fog_max. The primitive's fog_scale is
     * carried in state but glrend never applies it, so it is not packed. The block is
     * reserved before anything is recorded for the draw, so a failed reserve declines the
     * draw cleanly rather than drawing it unfogged.
     */
    material = RendererVkMaterialReserve(renderer, &material_offset);
    if(material == NULL)
        return BR_FALSE;

    material->fog_colour[0] = 1.0f;
    material->fog_colour[1] = 1.0f;
    material->fog_colour[2] = 1.0f;
    material->fog_colour[3] = 1.0f;
    material->fog_range[0]  = 0.0f;
    material->fog_range[1]  = 0.0f;
    material->fog_enabled   = 0;
    material->reserved      = 0;

    if((state->valid & MASK_STATE_PRIMITIVE) && state->prim.fog_type != BRT_NONE) {
        material->fog_colour[0] = BR_RED(state->prim.fog_colour) / 255.0f;
        material->fog_colour[1] = BR_GRN(state->prim.fog_colour) / 255.0f;
        material->fog_colour[2] = BR_BLU(state->prim.fog_colour) / 255.0f;
        material->fog_colour[3] = 1.0f;
        material->fog_range[0]  = (float)state->prim.fog_min;
        material->fog_range[1]  = (float)state->prim.fog_max;
        material->fog_enabled   = 1;
    }

    RendererVkMaterialFlush(renderer, material_offset);

    cmd = DevicePixelmapVkGetCurrentFrame(dst->screen)->command_buffer;

    vk->CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    const VkDeviceSize offset = 0;
    vk->CmdBindVertexBuffers(cmd, 0, 1, &geom->vertex_buffer, &offset);
    vk->CmdBindIndexBuffer(cmd, geom->index_buffer, 0, VK_INDEX_TYPE_UINT16);

    const VkDescriptorBufferInfo scene_info = {
        .buffer = renderer->scene_buffer,
        .offset = 0,
        .range  = sizeof(br_vk_scene_state),
    };
    const VkWriteDescriptorSet scene_write = {
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding      = 0,
        .descriptorCount = 1,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo     = &scene_info,
    };
    vk->CmdPushDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dev->render_pipeline_layout, 0, 1, &scene_write);

    /*
     * The material block every fragment shader reads for its fog state. Its descriptor is
     * the block at this draw's offset in the scene's material buffer, so two draws with
     * different fog state can share the buffer without overwriting each other.
     */
    const VkDescriptorBufferInfo material_info = {
        .buffer = renderer->material_buffer,
        .offset = material_offset,
        .range  = sizeof(br_vk_material_state),
    };
    const VkWriteDescriptorSet material_write = {
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding      = 3,
        .descriptorCount = 1,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo     = &material_info,
    };
    vk->CmdPushDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dev->render_pipeline_layout, 0, 1, &material_write);

    if(textured) {
        /*
         * A device colour map is sampled where its image lies, in the layout the source
         * pixelmap records; the scene draw cannot transition it, because it records inside a
         * dynamic rendering scope (see BufferStoredVkGetViewLayout()). A source in no layout
         * this draw can sample is declined, rather than bound with a layout it is not in.
         */
        VkImageLayout colour_map_layout = BufferStoredVkGetViewLayout(colour_map);

        if(colour_map_layout == VK_IMAGE_LAYOUT_UNDEFINED)
            return BR_FALSE;

        /*
         * The index plane is an integer texture, so the sampler cannot filter it and the
         * index map has no mip chain: an indexed draw takes NEAREST/NEAREST with level 0
         * fixed, whatever the primitive asked for, while a plain colour map keeps the
         * primitive's filter and mip filter. glrend overrides an indexed colour map the
         * same way (drivers/glrend/v1model.c).
         */
        VkSampler sampler = DeviceVkRenderGetSampler(dev, indexed ? BRT_NEAREST : state->prim.filter, indexed ? BRT_NONE : state->prim.mip_filter,
                                                     state->prim.map_width_limit, state->prim.map_height_limit);

        if(sampler == VK_NULL_HANDLE)
            return BR_FALSE;

        const VkDescriptorImageInfo texture_info = {
            .sampler     = sampler,
            .imageView   = BufferStoredVkGetView(colour_map),
            .imageLayout = colour_map_layout,
        };
        const VkWriteDescriptorSet texture_write = {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding      = 1,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = &texture_info,
        };
        vk->CmdPushDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dev->render_pipeline_layout, 0, 1, &texture_write);

        /*
         * An INDEX_8 map binds its palette as binding 2. Its sampler is the primitive's:
         * the palette is a colour format the sampler can filter, and a palette of fewer
         * than 256 rows is clamped by the shader's texelFetch, not the sampler's wrap mode.
         */
        if(indexed) {
            const VkDescriptorImageInfo clut_info = {
                .sampler     = sampler,
                .imageView   = BufferStoredVkGetClutView(colour_map),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
            const VkWriteDescriptorSet clut_write = {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstBinding      = 2,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo      = &clut_info,
            };
            vk->CmdPushDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dev->render_pipeline_layout, 0, 1, &clut_write);
        }
    }

    vk->CmdPushConstants(cmd, dev->render_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(data), &data);

    vk->CmdDrawIndexed(cmd, count, 1, first_index, 0, 0);

    return BR_TRUE;
}

/*
 * Draw one prepared group's whole index range.
 */
void VkV1ModelRenderGroup(br_geometry_stored *geom, br_renderer *renderer, const br_geometry_stored_group *group)
{
    if(!VkV1ModelRenderIndexed(geom, renderer, group->count, group->offset / sizeof(br_uint_16)))
        return;

    renderer->stats.opaque_draw_count++;
    renderer->stats.triangles_drawn_count += group->count / 3;
}

/*
 * Draw one face: three indices at first_index in the shared IBO. This is the draw a
 * deferred BRT_TRIANGLE carries (see gstored.c), the Z-sort split that sorts a group's
 * faces back-to-front where the group-wide draw could not.
 */
void VkV1ModelRenderFace(br_geometry_stored *geom, br_renderer *renderer, br_uint_32 first_index)
{
    if(!VkV1ModelRenderIndexed(geom, renderer, 3, first_index))
        return;

    renderer->stats.face_group_count++;
    renderer->stats.triangles_drawn_count += 1;
    renderer->stats.transparent_draw_count++;
}

static void BR_CMETHOD_DECL(br_geometry_v1_buckets_vk, free)(br_object *_self)
{
    br_geometry_v1_buckets *self = (br_geometry_v1_buckets *)_self;

    ObjectContainerRemove(self->renderer_facility, (br_object *)self);

    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_geometry_v1_buckets_vk, identifier)(br_object *self)
{
    return ((br_geometry_v1_buckets *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_geometry_v1_buckets_vk, type)(br_object *self)
{
    (void)self;
    return BRT_GEOMETRY_V1_BUCKETS;
}

static br_boolean BR_CMETHOD_DECL(br_geometry_v1_buckets_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_GEOMETRY_V1_BUCKETS) || (t == BRT_GEOMETRY) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_geometry_v1_buckets_vk, device)(br_object *self)
{
    return ((br_geometry_v1_buckets *)self)->device;
}

static br_size_t BR_CMETHOD_DECL(br_geometry_v1_buckets_vk, space)(br_object *self)
{
    (void)self;
    return sizeof(br_geometry_v1_buckets);
}

static br_tv_template *BR_CMETHOD_DECL(br_geometry_v1_buckets_vk, templateQuery)(br_object *_self)
{
    br_geometry_v1_buckets *self = (br_geometry_v1_buckets *)_self;

    if(self->device->templates.geometryV1BucketsTemplate == NULL) {
        self->device->templates.geometryV1BucketsTemplate = BrTVTemplateAllocate(
            self->device, (br_tv_template_entry *)geometryV1BucketsTemplateEntries, BR_ASIZE(geometryV1BucketsTemplateEntries));
    }

    return self->device->templates.geometryV1BucketsTemplate;
}

static br_error BR_CMETHOD_DECL(br_geometry_v1_buckets_vk, render)(br_geometry_v1_buckets *self, br_renderer *renderer,
                                                                   br_primitive **buckets, br_int_32 nbuckets)
{
    (void)self;

    if(nbuckets <= 0)
        return BRE_OK;

    /*
     * Render the bucket table from last to first.
     */
    for(buckets += nbuckets - 1; nbuckets--; buckets--) {
        for(br_primitive *p = *buckets; p; p = p->next) {
            br_renderer_state_stored *stored = p->stored;

            ASSERT(stored == NULL || ObjectIsType((br_object *)stored, BRT_RENDERER_STATE_STORED));

            if(p->type == BRT_GEOMETRY_STORED) {
                br_geometry_stored             *geom      = p->v[0];
                state_stack                    *state     = p->v[1];
                const br_geometry_stored_group *groupinfo = p->v[2];

                ASSERT(ObjectIsType((br_object *)geom, BRT_GEOMETRY_STORED));

                /*
                 * Restore the deferred state, then hand the state snapshot back to the
                 * pool. The draw reads the current state synchronously.
                 */
                *renderer->state.current = *state;
                RendererVkUnrefState(renderer, state);

                VkV1ModelRenderGroup(geom, renderer, groupinfo);
            } else if(p->type == BRT_TRIANGLE) {
                br_geometry_stored *geom        = p->v[0];
                state_stack        *state       = p->v[1];
                br_uint_32          first_index = (br_uint_32)(br_uintptr_t)p->v[2];

                ASSERT(ObjectIsType((br_object *)geom, BRT_GEOMETRY_STORED));

                *renderer->state.current = *state;
                RendererVkUnrefState(renderer, state);

                VkV1ModelRenderFace(geom, renderer, first_index);
            } else {
                /*
                 * We can't handle other things yet.
                 */
                continue;
            }
        }
    }

    return BRE_OK;
}

/*
 * Default dispatch table for geometry type
 */
static const struct br_geometry_v1_buckets_dispatch geometryV1BucketsDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_geometry_v1_buckets_vk, free),
    ._identifier = BR_CMETHOD_REF(br_geometry_v1_buckets_vk, identifier),
    ._type       = BR_CMETHOD_REF(br_geometry_v1_buckets_vk, type),
    ._isType     = BR_CMETHOD_REF(br_geometry_v1_buckets_vk, isType),
    ._device     = BR_CMETHOD_REF(br_geometry_v1_buckets_vk, device),
    ._space      = BR_CMETHOD_REF(br_geometry_v1_buckets_vk, space),

    ._templateQuery = BR_CMETHOD_REF(br_geometry_v1_buckets_vk, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._render         = BR_CMETHOD_REF(br_geometry_v1_buckets_vk, render),
    ._renderOnScreen = BR_CMETHOD_REF(br_geometry_v1_buckets_vk, render),
};

br_geometry_v1_buckets *GeometryV1BucketsVkAllocate(br_renderer_facility *type, const char *id)
{
    br_geometry_v1_buckets *self;

    self                    = BrResAllocate(type, sizeof(*self), BR_MEMORY_OBJECT);
    self->dispatch          = &geometryV1BucketsDispatch;
    self->identifier        = id;
    self->device            = type->device;
    self->renderer_facility = type;

    ObjectContainerAddFront(type, (br_object *)self);
    return self;
}
