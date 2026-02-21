/*
 * Device methods
 */
#include <inttypes.h>
#include "drv.h"

#define DEVICE_TITLE   "Vulkan"
#define DEVICE_VERSION BR_VERSION(1, 0, 0)
#define DEVICE_CREATOR "Zane van Iperen"
#define DEVICE_PRODUCT "Vulkan"

/*
 * Device info. template
 */
static const char deviceTitle[] = DEVICE_TITLE;

static const char deviceCreator[] = DEVICE_CREATOR;

static const char deviceProduct[] = DEVICE_PRODUCT;

#define F(f) offsetof(br_device, f)
#define A(a) ((br_uintptr_t)(a))

static br_tv_template_entry deviceTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),       F(identifier),      BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0              },
    {BRT(VERSION_U32),           0,                  BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, DEVICE_VERSION },
    {BRT(BRENDER_VERSION_U32),   0,                  BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, __BRENDER__    },
    {BRT(DDI_VERSION_U32),       0,                  BRTV_QUERY | BRTV_ALL,            BRTV_CONV_DIRECT, __BRENDER_DDI__},
    {BRT(CREATOR_CSTR),          A(deviceCreator),   BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
    {BRT(TITLE_CSTR),            A(deviceTitle),     BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
    {BRT(PRODUCT_CSTR),          A(deviceProduct),   BRTV_QUERY | BRTV_ALL | BRTV_ABS, BRTV_CONV_COPY,   0              },
    {DEV(VULKAN_INSTANCE_H),     F(vulkan),          BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0              },
    {DEV(VULKAN_QUEUE_H),        F(vk_queue),        BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0              },
    {DEV(VULKAN_COMMAND_POOL_H), F(vk_command_pool), BRTV_QUERY | BRTV_ALL,            BRTV_CONV_COPY,   0              },
};
#undef F
#undef A

static void BR_CMETHOD_DECL(br_device_vk, free)(br_object *_self)
{
    br_device *self = (br_device *)_self;

    /*
     * Remove attached objects
     */
    BrObjectContainerFree((br_object_container *)self, BR_NULL_TOKEN, NULL, NULL);

    if(self->vk_queue != NULL) {
        VkResult r = self->context.QueueWaitIdle(self->vk_queue);
        int      x = 0;
    }

    if(self->vk_device != NULL) {
        VkResult r = self->context.DeviceWaitIdle(self->vk_device);
        int      x = 0;
    }

    if(self->vma_allocator != NULL) {
        vmaDestroyAllocator(self->vma_allocator);
        self->vma_allocator = NULL;
    }

    if(self->vk_command_pool != NULL) {
        self->context.DestroyCommandPool(self->vk_device, self->vk_command_pool, NULL);
        self->vk_command_pool = NULL;
    }

    if(self->vk_device != NULL) {
        self->context.DestroyDevice(self->vk_device, NULL);
        self->vk_device = NULL;
    }

    if(self->vulkan != NULL) {
        self->context.DestroyInstance(self->vulkan, NULL);
        self->vulkan = NULL;
    }

    /*
     * Remove resources
     */
    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_device_vk, identifier)(br_object *self)
{
    return ((br_device *)self)->identifier;
}

static br_token BR_CMETHOD_DECL(br_device_vk, type)(br_object *self)
{
    (void)self;
    return BRT_DEVICE;
}

static br_boolean BR_CMETHOD_DECL(br_device_vk, isType)(br_object *self, br_token t)
{
    (void)self;
    return (t == BRT_DEVICE) || (t == BRT_OBJECT_CONTAINER) || (t == BRT_OBJECT);
}

static br_device *BR_CMETHOD_DECL(br_device_vk, device)(br_object *self)
{
    return (br_device *)self;
}

static br_size_t BR_CMETHOD_DECL(br_device_vk, space)(br_object *self)
{
    (void)self;
    return sizeof(br_device);
}

static struct br_tv_template *BR_CMETHOD_DECL(br_device_vk, templateQuery)(br_object *self)
{
    return ((br_device *)self)->templates.deviceTemplate;
}

static void *BR_CMETHOD_DECL(br_device_vk, listQuery)(br_object_container *self)
{
    return ((br_device *)self)->object_list;
}

/*
 * List of tokens which are not significant in matching (for output facilities)
 */
// clang-format off
static const br_token insignificantMatchTokens[] = {
    BRT_WIDTH_I32,
    BRT_HEIGHT_I32,
    BRT_PIXEL_BITS_I32,
    BRT_PIXEL_TYPE_U8,
    BRT_WINDOW_MONITOR_I32,
    BRT_MSAA_SAMPLES_I32,
    BRT_WINDOW_HANDLE_H,
    BR_NULL_TOKEN,
};
// clang-format on

static const br_tv_match_info *BR_CMETHOD_DECL(br_device_vk, tokensMatchInfoQuery)(br_object_container *self)
{
    static const br_tv_match_info matchinfo[] = {
        {.type = BRT_OUTPUT_FACILITY, .insignificant = insignificantMatchTokens},
        {.type = BR_NULL_TOKEN,       .insignificant = NULL                    },
    };

    (void)self;

    return matchinfo;
}

/*
 * Default dispatch table for device
 */
static const struct br_device_dispatch deviceDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_device_vk, free),
    ._identifier = BR_CMETHOD_REF(br_device_vk, identifier),
    ._type       = BR_CMETHOD_REF(br_device_vk, type),
    ._isType     = BR_CMETHOD_REF(br_device_vk, isType),
    ._device     = BR_CMETHOD_REF(br_device_vk, device),
    ._space      = BR_CMETHOD_REF(br_device_vk, space),

    ._templateQuery = BR_CMETHOD_REF(br_device_vk, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._listQuery            = BR_CMETHOD_REF(br_device_vk, listQuery),
    ._tokensMatchBegin     = BR_CMETHOD_REF(br_object_container, tokensMatchBegin),
    ._tokensMatch          = BR_CMETHOD_REF(br_object_container, tokensMatch),
    ._tokensMatchEnd       = BR_CMETHOD_REF(br_object_container, tokensMatchEnd),
    ._tokensMatchInfoQuery = BR_CMETHOD_REF(br_device_vk, tokensMatchInfoQuery),
    ._addFront             = BR_CMETHOD_REF(br_object_container, addFront),
    ._removeFront          = BR_CMETHOD_REF(br_object_container, removeFront),
    ._remove               = BR_CMETHOD_REF(br_object_container, remove),
    ._find                 = BR_CMETHOD_REF(br_object_container, find),
    ._findMany             = BR_CMETHOD_REF(br_object_container, findMany),
    ._count                = BR_CMETHOD_REF(br_object_container, count),
};

static br_error DeviceVKStringListToArray(void *vparent, const br_simple_list *list, const char *const **out, unsigned int *count)
{
    const char  *s;
    const char **arr;
    unsigned int n;

    n = 0;
    BR_FOR_SIMPLELIST(list, s) {
        ++n;
    }

    if(((arr = BrResAllocate(vparent, sizeof(const char *) * n, BR_MEMORY_DRIVER))) == NULL)
        return BRE_NO_MEMORY;

    n = 0;
    BR_FOR_SIMPLELIST(list, s) {
        if((arr[n++] = BrResStrDup(arr, s)) == NULL)
            return BRE_NO_MEMORY;
    }

    *out   = arr;
    *count = n;
    return BRE_OK;
}

typedef VkResult BR_CALLBACK asdfasdfasdfasdfasd(br_simple_list *layers, br_simple_list *extensions);

static VkResult qwerqwe(br_device *self, br_simple_list *layers, br_simple_list *extensions, VkInstance *vk)
{
    unsigned int num_layers = 0, num_extension = 0;

    static const VkApplicationInfo appinfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = NULL,
        .pApplicationName   = "BRender",
        .applicationVersion = __BRENDER__,
        .pEngineName        = "BRender",
        .engineVersion      = __BRENDER__,
        .apiVersion         = VK_API_VERSION_1_3,
    };

    VkInstanceCreateInfo createinfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = NULL,
        .flags                   = 0,
        .pApplicationInfo        = &appinfo,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = NULL,
        .enabledExtensionCount   = 0,
        .ppEnabledExtensionNames = NULL,
    };

    if(DeviceVKStringListToArray(self, layers, &createinfo.ppEnabledLayerNames, &createinfo.enabledLayerCount) != BRE_OK)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    if(DeviceVKStringListToArray(self, extensions, &createinfo.ppEnabledExtensionNames, &createinfo.enabledExtensionCount) != BRE_OK)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    return self->context.CreateInstance(&createinfo, NULL, vk);
}

typedef struct br_device_args {
    br_device_vk_init *init_info;
    br_device         *init_device;
    br_device_vk_uuid  device_uuid;

    const char *_device_uuid;
} br_device_args;

#define F(f) offsetof(br_device_args, f)
static struct br_tv_template_entry deviceAllocateTemplateEntries[] = {
    {DEV(VKREND_INIT_P),        F(init_info),    BRTV_SET, BRTV_CONV_COPY, 0},
    {DEV(VKREND_INIT_DEVICE_O), F(init_device),  BRTV_SET, BRTV_CONV_COPY, 0},
    {DEV(DEVICE_UUID_CSTR),     F(_device_uuid), BRTV_SET, BRTV_CONV_COPY, 0},
};
#undef F

static br_error unpack_args(br_device *self, br_device_args *args, const char *arguments)
{
    br_token_value args_tv[256];
    br_int_32      count;
    br_error       err;

    *args = (br_device_args){
        .init_info    = NULL,
        .init_device  = NULL,
        ._device_uuid = NULL,
    };

    if((err = BrStringToTokenValue(args_tv, sizeof(args_tv), arguments)) != BRE_OK)
        return err;

    if(self->templates.deviceAllocateTemplate == NULL)
        self->templates.deviceAllocateTemplate = BrTVTemplateAllocate(self, deviceAllocateTemplateEntries, BR_ASIZE(deviceAllocateTemplateEntries));

    err = BrTokenValueSetMany(args, &count, NULL, args_tv, self->templates.deviceAllocateTemplate);
    if(err != BRE_OK)
        return err;

    /*
     * If given an init payload and a device, ignore the device.
     */
    if(args->init_info != NULL) {
        args->init_device = NULL;
    }

    /*
     * If given a device and no init payload, attempt to query it from the device.
     */
    if(args->init_device != NULL) {
        ObjectQuery(args->init_device, &args->init_info, BrTokenFind("VKREND_INIT_P"));
    }

    /*
     * If we're given a device UUID, attempt to parse it. Use the max UUID if invalid.
     */
    if(args->_device_uuid != NULL) {
        if(DeviceVkUUIDParse(&args->device_uuid, args->_device_uuid) != BRE_OK) {
            DeviceVkUUIDMax(&args->device_uuid);
        }

        args->_device_uuid = NULL;
    }

    return BRE_OK;
}

static VkResult create_instance(void *vparent, const GladVulkanContext *ctx, const br_device_vk_init *iinfo, VkInstance *vk, int validation)
{
    uint32_t     num_exts, num_layers;
    const char **exts, **layers;

    static const VkApplicationInfo appinfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = NULL,
        .pApplicationName   = "BRender",
        .applicationVersion = __BRENDER__,
        .pEngineName        = "BRender",
        .engineVersion      = __BRENDER__,
        .apiVersion         = VK_API_VERSION_1_4,
    };

    num_exts = iinfo->required_extension_count;
    exts     = BrResAllocate(vparent, sizeof(const char *) * num_exts, BR_MEMORY_SCRATCH);
    for(uint32_t i = 0; i < iinfo->required_extension_count; ++i) {
        exts[i] = iinfo->required_extension_names[i];
    }

    {
        uint32_t i;
        num_layers = iinfo->required_layer_count + (validation ? 1 : 0);

        layers = BrResAllocate(vparent, sizeof(const char *) * num_layers, BR_MEMORY_SCRATCH);
        for(i = 0; i < iinfo->required_layer_count; ++i) {
            layers[i] = iinfo->required_layer_names[i];
        }

        if(validation) {
            layers[i++] = "VK_LAYER_KHRONOS_validation";
        }
    }

    VkInstanceCreateInfo createinfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = NULL,
        .flags                   = 0,
        .pApplicationInfo        = &appinfo,
        .enabledLayerCount       = num_layers,
        .ppEnabledLayerNames     = layers,
        .enabledExtensionCount   = num_exts,
        .ppEnabledExtensionNames = exts,
    };

    BrLogInfo("VKREND", "Requested layers:");
    for(uint32_t i = 0; i < createinfo.enabledLayerCount; ++i) {
        BrLogInfo("VKREND", "[%" PRIu32 "] %s", i, createinfo.ppEnabledLayerNames[i]);
    }

    BrLogInfo("VKREND", "Requested extensions:");
    for(uint32_t i = 0; i < createinfo.enabledExtensionCount; ++i) {
        BrLogInfo("VKREND", "[%" PRIu32 "] %s", i, createinfo.ppEnabledExtensionNames[i]);
    }

    return ctx->CreateInstance(&createinfo, NULL, vk);
}

static GLADapiproc gladloader(void *userptr, const char *name)
{
    br_device *self = userptr;

    if(self->vk_device) {
        GLADapiproc proc;
        if((proc = self->context.GetDeviceProcAddr(self->vk_device, name)) != NULL)
            return proc;
    }

    /*
     * Work around https://github.com/Dav1dde/glad/issues/451
     * Vulkan 1.3+ can return NULL for instance methods if they're not overridden.
     * GLAD doesn't expect this and clobbers the pointer.
     *
     * Try it with fallback.
     */
    GLADapiproc res = self->getInstanceProcAddr(self->vulkan, name);
    if(res == NULL)
        res = self->getInstanceProcAddr(NULL, name);

    return res;
}

br_device *DeviceVkAllocate(const char *identifier, const char *arguments)
{
    br_device          *self;
    br_device_args      args;
    VkResult            vkres;
    void               *scratch_res;
    br_device_vk_select sel = {
        .device_uuid = {},
    };

    BrTokenCreate("VKREND_INIT_P", BRT_POINTER);
    BrTokenCreate("VKREND_INIT_DEVICE_O", BRT_OBJECT);

    BrTokenCreate("DEVICE_UUID_CSTR", BR_NULL_TOKEN);

    /*
     * Set up device block and resource anchor
     */
    self              = BrResAllocate(NULL, sizeof(br_device), BR_MEMORY_OBJECT);
    self->identifier  = BrResStrDup(self, identifier);
    self->res         = BrResAllocate(self, 0, BR_MEMORY_DRIVER);
    self->dispatch    = &deviceDispatch;
    self->object_list = BrObjectListAllocate(self);
    // self->deferred_free_list = BrObjectListAllocate()

    self->templates.deviceTemplate = BrTVTemplateAllocate(self, deviceTemplateEntries, BR_ASIZE(deviceTemplateEntries));

    if(unpack_args(self, &args, arguments) != BRE_OK) {
        BrResFreeNoCallback(self);
        return NULL;
    }

    if(args.init_info == NULL) {
        BrLogError("VKREND", "No init payload provided.");
        BrResFreeNoCallback(self);
        return NULL;
    }

    if(args.init_info->version != BR_DEVICE_VK_INIT_VERSION_1) {
        BrLogError("VKREND", "Unsupported init version payload %" PRIu32 ".", args.init_info->version);
        BrResFreeNoCallback(self);
        return NULL;
    }

    if(DeviceVkUUIDIsMax(&args.device_uuid)) {
        BrLogError("VKREND", "Invalid device UUID provided.");
        BrResFreeNoCallback(self);
        return NULL;
    }

    DeviceVkUUIDCopy(&sel.device_uuid, &args.device_uuid);

    self->vulkan              = NULL;
    self->getInstanceProcAddr = (PFN_vkGetInstanceProcAddr)args.init_info->get_instance_proc_address;
    self->init_info           = *args.init_info;

    scratch_res = BrResAllocate(self, 0, BR_MEMORY_ANCHOR);

    /*
     * Take 1 - Load the initial Vulkan functions.
     */
    if(gladLoadVulkanContextUserPtr(&self->context, NULL, gladloader, self) == 0) {
        BrLogError("VKREND", "Unable to load initial Vulkan functions.");
        BrResFreeNoCallback(self);
        return NULL;
    }

    /*
     * From here, our free() method should be handle anything partially-constructed.
     */

    /*
     * Create the instance.
     */
    if((vkres = create_instance(scratch_res, &self->context, args.init_info, &self->vulkan, 1)) != VK_SUCCESS) {
        BrLogError("VKREND", "Unable to create Vulkan instance: VkResult = %d", vkres);
        BrResFree(self);
        return NULL;
    }

    /*
     * Take 2 - load the instance functions.
     */
    if(gladLoadVulkanContextUserPtr(&self->context, NULL, gladloader, self) == 0) {
        BrLogError("VKREND", "Unable to load instance Vulkan functions.");
        BrResFree(self);
        return NULL;
    }

    /*
     * Select a physical device as best we can.
     */
    if((self->physical_device = DeviceVkSelect(self, &self->context, self->vulkan, &sel)) == NULL) {
        self->context.DestroyInstance(self->vulkan, NULL);
        BrResFreeNoCallback(self);
        return NULL;
    }

    /*
     * Create the logical device.
     */
    {
        float                         prio = 1.0f;
        const VkDeviceQueueCreateInfo qci  = {
             .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
             .queueFamilyIndex = self->physical_device->qf_gfx,
             .queueCount       = 1,
             .pQueuePriorities = &prio,
        };

        const VkPhysicalDeviceFeatures feat = {};

        static const char *const device_extensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        const VkDeviceCreateInfo dci = {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pQueueCreateInfos       = &qci,
            .queueCreateInfoCount    = 1,
            .pEnabledFeatures        = &feat,
            .enabledExtensionCount   = BR_ASIZE(device_extensions),
            .ppEnabledExtensionNames = device_extensions,
        };

        if((vkres = self->context.CreateDevice(self->physical_device->dev, &dci, NULL, &self->vk_device)) != VK_SUCCESS) {
            BrLogError("VKREND", "Error creating logical device: VkResult = %d", vkres);
            BrResFree(self);
            return NULL;
        }
    }

    /*
     * Take 3 - reload the functions, but prefer device-specific ones as they have no dispatch overhead.
     */
    if(gladLoadVulkanContextUserPtr(&self->context, self->physical_device->dev, gladloader, self) == 0) {
        BrLogError("VKREND", "Unable to load device Vulkan functions.");
        BrResFree(self);
        return NULL;
    }

    self->context.GetDeviceQueue(self->vk_device, self->physical_device->qf_gfx, 0, &self->vk_queue);

    {
        const VkCommandPoolCreateInfo pool_create_info = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = self->physical_device->qf_gfx,
        };
        if((vkres = self->context.CreateCommandPool(self->vk_device, &pool_create_info, NULL, &self->vk_command_pool)) != VK_SUCCESS) {
            BrLogError("VKREND", "Error creating command pool: VkResult = %d", vkres);
            BrResFree(self);
            return NULL;
        }
    }

    {
        const VmaVulkanFunctions vulkan_functions = {
            .vkGetInstanceProcAddr = self->getInstanceProcAddr,
            .vkGetDeviceProcAddr   = self->context.GetDeviceProcAddr,
        };

        VmaAllocatorCreateInfo allocator_create_info = {
            .flags            = 0,
            .vulkanApiVersion = VK_API_VERSION_1_4,
            .instance         = self->vulkan,
            .physicalDevice   = self->physical_device->dev,
            .device           = self->vk_device,
            .pVulkanFunctions = &vulkan_functions,
        };

        if(self->context.KHR_dedicated_allocation)
            allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

        if(self->context.KHR_bind_memory2)
            allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;

        if(self->context.KHR_maintenance4)
            allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;

        if(self->context.KHR_maintenance5)
            allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;

        if(self->context.EXT_memory_budget)
            allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

        if(self->context.KHR_buffer_device_address)
            allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        if(self->context.EXT_memory_priority)
            allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;

        // if(self->context.AMD_device_coherent_memory)
        //     allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;

        if(self->context.KHR_external_memory_win32)
            allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;

        if((vkres = vmaCreateAllocator(&allocator_create_info, &self->vma_allocator)) != VK_SUCCESS) {
            BrLogError("VKREND", "Error creating Vulkan allocator: VkResult = %d", vkres);
            BrResFree(self);
            return NULL;
        }
    }

    BrResFree(scratch_res);
    return self;
}
