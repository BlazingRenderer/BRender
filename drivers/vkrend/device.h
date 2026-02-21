#ifndef DEVICE_H_
#define DEVICE_H_

typedef struct br_device_vk_phy {
    VkPhysicalDevice             dev;
    uint32_t                     index;
    VkPhysicalDeviceIDProperties id_props;
    VkPhysicalDeviceProperties2  props2;
    VkPhysicalDeviceFeatures2    features2;

    uint32_t                 qf_count;
    VkQueueFamilyProperties *qf;
    uint32_t                 qf_gfx;

    int rank;
} br_device_vk_phy;

typedef struct br_device_vk_select {
    /* User-provided device ID. */
    br_device_vk_uuid device_uuid;
} br_device_vk_select;

/**
 * \brief A rect-draw pipeline, specialised by the destination format and the source shape.
 *
 * A pipeline created against VkPipelineRenderingCreateInfo names its colour attachment
 * formats, so with dynamic rendering it only matches destination images of the same
 * format and sample count. An indexed source needs a second texture - its palette - and so
 * a second fragment shader, which is the other half of the key. One is built lazily per
 * destination format/variant and destroyed with the device - a VkPipeline is a VkDevice
 * object, independent of any surface or frame.
 */
typedef struct br_device_vk_rect_pipeline {
    VkFormat              format;
    VkSampleCountFlagBits samples;
    br_boolean            indexed;
    VkPipeline            pipeline;
} br_device_vk_rect_pipeline;

/**
 * \brief Largest number of rect pipelines (destination format x sample count x
 * plain/indexed variant) a device can hold.
 */
#define BR_VKREND_MAX_RECT_PIPELINES BR_PMT_MAX

/**
 * \brief A line/point-draw pipeline, specialised by the destination format, sample count
 * and primitive topology.
 *
 * Like the rect pipeline, VkPipelineRenderingCreateInfo names the colour attachment
 * format, so a pipeline only matches destination images of the same format and sample
 * count. Lines and points differ only in topology and vertex count, which is a pipeline
 * state, so topology is the other half of the key. A draw into an indexed (R8_UINT)
 * destination needs the integer-output fragment program and no blending, so `indexed`
 * keys that variant in. One is built lazily per destination format/variant and destroyed
 * with the device.
 */
typedef struct br_device_vk_line_pipeline {
    VkFormat              format;
    VkSampleCountFlagBits samples;
    VkPrimitiveTopology   topology;
    br_boolean            indexed;
    VkPipeline            pipeline;
} br_device_vk_line_pipeline;

/**
 * \brief Largest number of line pipelines (destination format x sample count x topology
 * x plain/indexed variant) a device can hold.
 */
#define BR_VKREND_MAX_LINE_PIPELINES BR_PMT_MAX

/**
 * \brief Whether the device's pixelmap -> device rect draw machinery has been brought up.
 */
typedef enum br_vk_rect_state {
    BR_VK_RECT_UNINITIALISED,
    BR_VK_RECT_READY,
    BR_VK_RECT_UNAVAILABLE,
} br_vk_rect_state;

/**
 * \brief Whether the device's pixelmap -> device line/point draw machinery has been brought up.
 */
typedef enum br_vk_line_state {
    BR_VK_LINE_UNINITIALISED,
    BR_VK_LINE_READY,
    BR_VK_LINE_UNAVAILABLE,
} br_vk_line_state;

/**
 * \brief A text-draw pipeline, specialised by the destination format and sample count.
 *
 * Text is an instanced textured quad draw, but it still names its colour attachment
 * format through VkPipelineRenderingCreateInfo, so like the rect and line pipelines a
 * pipeline only matches destination images of the same format and sample count. One is
 * built lazily per destination format and destroyed with the device.
 */
typedef struct br_device_vk_text_pipeline {
    VkFormat              format;
    VkSampleCountFlagBits samples;
    VkPipeline            pipeline;
} br_device_vk_text_pipeline;

/**
 * \brief Largest number of destination formats a device can have text pipelines for.
 */
#define BR_VKREND_MAX_TEXT_PIPELINES BR_PMT_MAX

/**
 * \brief Number of predefined fonts a device keeps a glyph atlas for.
 */
#define BR_VKREND_TEXT_FONT_COUNT 3

/**
 * \brief A predefined font's glyph atlas.
 *
 * A 2D array image with BR_VKREND_TEXT_GLYPH_COUNT layers, one per character code, each
 * layer the font's widest glyph by its height. Built lazily on the first text draw that
 * uses the font and destroyed with the device. max_width is the atlas layer's width in
 * pixels, which normalises a glyph's width to the shader's u extent.
 */
typedef struct br_device_vk_text_font {
    VkImage       image;
    VkImageView   view;
    VmaAllocation allocation;
    br_uint_32    max_width;
} br_device_vk_text_font;

/**
 * \brief Whether the device's pixelmap -> device text draw machinery has been brought up.
 */
typedef enum br_vk_text_state {
    BR_VK_TEXT_UNINITIALISED,
    BR_VK_TEXT_READY,
    BR_VK_TEXT_UNAVAILABLE,
} br_vk_text_state;

/**
 * \brief A scene-draw sampler, keyed by the primitive state's filter and wrap modes.
 *
 * A sampler is a VkDevice object, independent of any draw, so one is built lazily per
 * (filter, wrap) combination the primitive state asks for and destroyed with the device.
 * The colour map's descriptor names it, so a filtered or clamped draw does not need its own
 * pipeline.
 */
typedef struct br_device_vk_render_sampler {
    br_token  filter;
    br_token  mip_filter;
    br_token  wrap_s;
    br_token  wrap_t;
    VkSampler sampler;
} br_device_vk_render_sampler;

/**
 * \brief Largest number of (filter, mip filter, wrap) combinations a device can hold
 * samplers for.
 *
 * Two filters by two mip filters by three wrap modes each way is 36; the handful of
 * combinations the primitive state defaults to fit many times over.
 */
#define BR_VKREND_MAX_RENDER_SAMPLERS 36

/**
 * \brief A scene-draw pipeline, specialised by the destination format, sample count, whether
 * the surface is lit, and whether it has a colour map.
 *
 * The br_renderer draws a model's triangles into the destination named by its OUTPUT
 * state. Like the rect, line and text pipelines the colour attachment format is named
 * through VkPipelineRenderingCreateInfo, because dynamic rendering has no VkRenderPass to
 * derive it from, so a pipeline only matches a destination image of the same format and
 * sample count. The lighting split is the other half of the key: a BRT_LIGHTING_B surface
 * shades through render.lit.frag.glsl, an unlit one through render.frag.glsl, and neither
 * should pay for the other's fragment work. The textured split keeps the untextured
 * shaders free of the sampler: a colour map draws through render.textured.frag.glsl or
 * render.lit.textured.frag.glsl, which sample and modulate. An INDEX_8 colour map takes a
 * second texture - its palette - and so the indexed variant of each textured shader, which
 * is the indexed half of the key. The cull split is the fourth:
 * a one-sided material culls back faces, a two-sided one culls nothing. The blend split is
 * the fifth: a PRIMF_BLEND primitive's blend_mode selects the fixed-function factors, a
 * non-blended one draws opaque. The depth split is the sixth: a Zb scene attaches a depth
 * buffer and a material's depth test/write state and compare op change the fixed-function
 * state, while a Zs scene has no depth attachment at all. A pipeline names its depth
 * attachment format through VkPipelineRenderingCreateInfo alongside the colour format, so
 * the depth format is part of the key too. The shading split is the seventh: a GOURAUD
 * surface accumulates the lights in the vertex stage and its fragment stage takes the
 * interpolated result, while a PHONG surface accumulates per fragment. The mode reaches
 * the shaders as a push-constant bit (the flags construction in gv1buckt.c,
 * gouraudShading() in render.common.glsl) and both stages share one module across FLAT,
 * GOURAUD and PHONG, so the shading mode in the key serves to keep their pipelines
 * distinct. The winding split is the eighth: a model-view whose upper 3x3 determinant is
 * negative reflects, reversing the screen winding of a BRender front face, so the front
 * face the pipeline names flips with it (front_face_for() in gv1buckt.c) and the two
 * windings get their own pipelines. One is built lazily per destination format and
 * destroyed with the device.
 */
typedef struct br_device_vk_render_pipeline {
    VkFormat              format;
    VkSampleCountFlagBits samples;
    br_boolean            lit;
    br_boolean            textured;
    br_boolean            indexed;
    br_token              shading_mode;
    VkCullModeFlags       cull;
    VkFrontFace           front_face;
    br_boolean            blended;
    br_token              blend_mode;
    VkFormat              depth_format;
    br_boolean            depth_test;
    br_boolean            depth_write;
    VkCompareOp           depth_compare;
    VkPipeline            pipeline;
} br_device_vk_render_pipeline;

/**
 * \brief Largest number of destination formats a device can have scene pipelines for.
 *
 * Each format can hold lit/unlit, untextured/textured/indexed-textured,
 * culled/unculled and front/back-winding variants, times the three shading modes
 * (BRT_FLAT, BRT_GOURAUD and BRT_PHONG), times the five blend states (no blending, and the
 * four blend_mode tokens of which BRT_NONE and the default share one), times the depth
 * states a scene uses.
 */
#define BR_VKREND_MAX_RENDER_PIPELINES (BR_PMT_MAX * 720)

/**
 * \brief Whether the device's br_renderer scene draw machinery has been brought up.
 */
typedef enum br_vk_render_state {
    BR_VK_RENDER_UNINITIALISED,
    BR_VK_RENDER_READY,
    BR_VK_RENDER_UNAVAILABLE,
} br_vk_render_state;

#ifdef BR_DEVICE_PRIVATE

/*
 * Private state of device
 */
typedef struct br_device {
    /*
     * Dispatch table
     */
    const struct br_device_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * List of objects associated with this device
     */
    void *object_list;

    /*
     * Anchor for all device's resources
     */
    void *res;

    /*
     * Driver-wide template store
     */
    struct device_templates templates;

    VkInstance                vulkan;
    GladVulkanContext         context;
    PFN_vkGetInstanceProcAddr getInstanceProcAddr;
    br_device_vk_init         init_info;
    br_device_vk_phy         *physical_device;
    VkDevice                  vk_device;
    VmaAllocator              vma_allocator;
    VkQueue                   vk_queue;
    VkCommandPool             vk_command_pool;

    /**
     * \brief The front screen attached to this device, or NULL.
     *
     * A stored geometry or buffer can be destroyed between frames, while the front
     * screen's current command buffer - recorded but not submitted - still references its
     * VkBuffer or VkImage. Instead of destroying those in place, they are retired against
     * the front screen's current frame (DevicePixelmapVkRetireBuffer/RetireImage); this is
     * how an object with no pixelmap of its own reaches that frame. Set by
     * DevicePixelmapVkAllocateFront() and cleared by the front screen's free, which idles
     * the queue, so a NULL value means no in-flight work can reference anything and direct
     * destruction is safe.
     */
    struct br_device_pixelmap *screen;

    /**
     * \brief Whether VkPhysicalDeviceVulkan13Features::dynamicRendering was enabled on the
     * logical device. The rect, line, text and scene draws need it and fall back to the CPU
     * path without it.
     */
    br_boolean dynamic_rendering;

    /**
     * \brief Whether VkPhysicalDeviceVulkan14Features::pushDescriptor was enabled on the
     * logical device. The rect, text and scene draws bind their resources with push
     * descriptors, so they need this alongside dynamic rendering; the line draw binds
     * nothing and needs only dynamic rendering.
     */
    br_boolean push_descriptor;

    /**
     * \brief Whether VkPhysicalDeviceFeatures::shaderClipDistance was enabled on the logical
     * device. The scene vertex shader statically writes gl_ClipDistance, so without this the
     * scene draw cannot be built (see DeviceVkRenderEnsureInit()).
     */
    br_boolean shader_clip_distance;

    /*
     * Pixelmap -> device rect draw. Device-scoped and brought up on first use; a pipeline
     * needs no surface and rectangleCopyTo() runs with no renderer.
     */
    br_vk_rect_state           rect_state;
    VkShaderModule             rect_vert;
    VkShaderModule             rect_frag;
    VkShaderModule             rect_indexed_frag;
    VkDescriptorSetLayout      rect_set_layout;
    VkPipelineLayout           rect_pipeline_layout;
    VkSampler                  rect_sampler;
    br_device_vk_rect_pipeline rect_pipelines[BR_VKREND_MAX_RECT_PIPELINES];
    br_uint_32                 rect_pipeline_count;

    /*
     * Pixelmap -> device line/point draw. Device-scoped and brought up on first use, like
     * the rect draw. It uses push constants alone, so it has no descriptor set layout.
     */
    br_vk_line_state           line_state;
    VkShaderModule             line_vert;
    VkShaderModule             line_frag;
    VkShaderModule             line_frag_int;
    VkPipelineLayout           line_pipeline_layout;
    br_device_vk_line_pipeline line_pipelines[BR_VKREND_MAX_LINE_PIPELINES];
    br_uint_32                 line_pipeline_count;

    /*
     * Pixelmap -> device text draw. Device-scoped and brought up on first use, like the
     * rect draw. It samples a per-font glyph atlas through a push descriptor, so it needs
     * push descriptors alongside dynamic rendering.
     */
    br_vk_text_state           text_state;
    VkShaderModule             text_vert;
    VkShaderModule             text_frag;
    VkDescriptorSetLayout      text_set_layout;
    VkPipelineLayout           text_pipeline_layout;
    VkSampler                  text_sampler;
    br_device_vk_text_pipeline text_pipelines[BR_VKREND_MAX_TEXT_PIPELINES];
    br_uint_32                 text_pipeline_count;
    br_device_vk_text_font     text_fonts[BR_VKREND_TEXT_FONT_COUNT];

    /*
     * br_renderer scene draw. Device-scoped and brought up on first use, like the rect
     * draw. It draws from a geometry_stored vertex/index buffer pair into the renderer's
     * current destination, so its pipeline is specialised by destination format, sample
     * count and whether the surface is lit. Its push constant block carries the per-draw
     * model-view, colour and lighting factors, and its push descriptor binds the per-scene
     * block of packed lights.
     */
    br_vk_render_state           render_state;
    VkShaderModule               render_vert;
    VkShaderModule               render_frag;
    VkShaderModule               render_lit;
    VkShaderModule               render_textured;
    VkShaderModule               render_textured_indexed;
    VkShaderModule               render_lit_textured;
    VkShaderModule               render_lit_textured_indexed;
    VkDescriptorSetLayout        render_set_layout;
    VkPipelineLayout             render_pipeline_layout;
    br_device_vk_render_pipeline render_pipelines[BR_VKREND_MAX_RENDER_PIPELINES];
    br_uint_32                   render_pipeline_count;
    br_device_vk_render_sampler  render_samplers[BR_VKREND_MAX_RENDER_SAMPLERS];
    br_uint_32                   render_sampler_count;
} br_device;

#endif /* BR_DEVICE_PRIVATE */
#endif /* DEVICE_H_ */
