#include "drv.h"
#include <SDL3/SDL_vulkan.h>
#include <brvkrend.h>
#include <brassert.h>

#define VULKAN_DEVICE_NAME "vkrend"

typedef struct sdl_vk_state {
    SDL_Window *window;
    int         in_resize_callback;

    br_device_sdl_ext_procs ext_procs;
} sdl_vk_state;

static br_error sdl_vk_resize(br_pixelmap *pm, br_int_32 w, br_int_32 h, void *user)
{
    sdl_vk_state *state = user;

    (void)pm;

    /*
     * Prevent _issues_.
     */
    if(state->in_resize_callback)
        return BRE_OK;

    SDL_SetWindowSize(state->window, w, h);
    return BRE_OK;
}

static void sdl_vk_free(br_pixelmap *pm, void *user)
{
    sdl_vk_state *state = user;

    (void)pm;

    UASSERT(state->in_resize_callback == 0);

    SDL_DestroyWindow(state->window);
}

static br_error sdl_vk_create_surface(void *_instance, const void *_allocator, void *_surface, void *user)
{
    VkInstance                          instance  = _instance;
    const struct VkAllocationCallbacks *allocator = _allocator;
    VkSurfaceKHR                       *surface   = _surface;
    sdl_vk_state                       *state     = user;

    if(!SDL_Vulkan_CreateSurface(state->window, instance, allocator, surface))
        return BRE_FAIL;

    return BRE_OK;
}

static void sdl_vk_destroy_surface(void *_instance, const void *_allocator, void *_surface, void *user)
{
    VkInstance                          instance  = _instance;
    const struct VkAllocationCallbacks *allocator = _allocator;
    VkSurfaceKHR                        surface   = _surface;
    sdl_vk_state                       *state     = user;

    (void)state;

    SDL_Vulkan_DestroySurface(instance, surface, allocator);
}

static br_error sdl_vk_handle_window_event(br_pixelmap *pm, void *arg, void *user)
{
    SDL_WindowEvent *evt   = arg;
    sdl_vk_state    *state = user;
    int              wpix, hpix;

    (void)user;

    /*
     * We can only handle window size changes for now.
     */
    if(evt->type != SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
        return BRE_UNSUPPORTED;

    SDL_GetWindowSizeInPixels(state->window, &wpix, &hpix);

    /*
     * vkrend's resize() method is guaranteed to only update the internal state, so this is fine.
     */
    ++state->in_resize_callback;
    BrPixelmapResize(pm, wpix, hpix);
    --state->in_resize_callback;
    return BRE_OK;
}

static void sdl_vk_preswap_hook(br_pixelmap *pm, void *user)
{
    sdl_vk_state *state = user;

    (void)pm;

    if(state->ext_procs.preswap != NULL)
        state->ext_procs.preswap(pm, state->ext_procs.user);
}

static br_error create_vk_pixelmap(SDL_Window *window, br_device *gldev, br_device_sdl_ext_procs *ext_procs, br_device_pixelmap **ppmap)
{
    br_error      err;
    Uint32        format;
    int           width, height;
    br_pixelmap  *pm;
    br_object    *outfcty;
    sdl_vk_state *state;

    br_token_value tv[] = {
        {.t = BRT_WIDTH_I32,       .v = {.p = 0}},
        {.t = BRT_HEIGHT_I32,      .v = {.p = 0}},
        {.t = BRT_PIXEL_TYPE_U8,   .v = {.p = 0}},
        {.t = BRT_WINDOW_HANDLE_H, .v = {.p = 0}},
        {.t = BR_NULL_TOKEN,       .v = {.p = 0}},
    };

    SDL_GetWindowSizeInPixels(window, &width, &height);
    tv[0].v.i32 = (br_int_32)width;
    tv[1].v.i32 = (br_int_32)height;

    format = SDL_GetWindowPixelFormat(window);
    if((err = SDLToBRenderPixelFormat(format, NULL, &tv[2].v.u8)) != BRE_OK)
        return err;

    /*
     * Allocate our state.
     */
    state  = BrResAllocate(NULL, sizeof(sdl_vk_state), BR_MEMORY_APPLICATION);
    *state = (sdl_vk_state){
        .window             = window,
        .in_resize_callback = 0,
        .ext_procs          = (br_device_sdl_ext_procs){.preswap = NULL, .user = NULL},
    };

    if(ext_procs != NULL)
        state->ext_procs = *ext_procs;

    tv[3].v.h = state;

    /*
     * Find an output facility with our given requirements.
     */
    outfcty = NULL;
    if((err = ObjectContainerFind(gldev, &outfcty, BRT_OUTPUT_FACILITY, NULL, tv)) != BRE_OK) {
        BrResFree(state);
        return err;
    }

    /*
     * Create the screen pixelmap.
     */
    if((err = OutputFacilityPixelmapNew(outfcty, (void *)&pm, tv)) != BRE_OK) {
        BrResFree(state);
        return err;
    }

    /*
     * Attach the state to the pixelmap. #notourproblemanymore
     */
    BrResAdd(pm, state);

    *ppmap = (br_device_pixelmap *)pm;
    return BRE_OK;
}

br_error DevicePixelmapSDL3CreateVk(br_device *dev, const pixelmap_new_tokens *pt, br_device_pixelmap **ppmap)
{
    br_device  *vkdev;
    SDL_Window *window;
    br_uint_32  flags;
    char       *args;

    /*
     * Vulkan device pixelmaps are for new windows only.
     */
    if(pt->surface != NULL || pt->window != NULL || pt->use_type != BRT_NONE)
        return BRE_FAIL;

    flags = pt->flags & ~SDL_WINDOW_OPENGL;
    UASSERT(flags & SDL_WINDOW_VULKAN);

    args = BrScratchString();
    BrSprintf(args, "VKREND_INIT_DEVICE=%" PRIuPTR, (br_uintptr_t)dev);

    /*
     * Find or load the device.
     */
    if(BrDevCheckAdd(&vkdev, VULKAN_DEVICE_NAME, args) != BRE_OK || vkdev == NULL) {
        BrLogError("SDL3", "Vulkan window requested, but driver can't be loaded.");
        return BRE_FAIL;
    }

    window = SDL_CreateWindow(pt->title, pt->width, pt->height, pt->flags);
    if(window == NULL) {
        BrLogError("SDL3", "Error creating window: %s", SDL_GetError());
        return BRE_FAIL;
    }

    if(create_vk_pixelmap(window, vkdev, pt->ext_procs, ppmap) != BRE_OK) {
        BrLogError("SDL3", "Error creating Vulkan pixelmap.");
        SDL_DestroyWindow(window);
        return BRE_FAIL;
    }

    return BRE_OK;
}

br_error DeviceSDL3LoadVulkan(br_device *dev)
{
    br_device_vk_init *init;
    Uint32             ext_count = 0;
    const char *const *exts;

    if(dev->vulkan_loaded)
        return BRE_OK;

    BrTokenCreate("VKREND_INIT_P", BRT_POINTER);

    if(!SDL_Vulkan_LoadLibrary(NULL)) {
        BrLogWarn("SDL3", "Unable to load Vulkan library, vkrend interop will be unavailable: %s", SDL_GetError());
        return BRE_FAIL;
    }

    if((exts = SDL_Vulkan_GetInstanceExtensions(&ext_count)) == NULL) {
        BrLogError("SDL3", "Unable to retrieve Vulkan instance extensions, vkrend interop will be unavailable: %s", SDL_GetError());
        return BRE_FAIL;
    }

    init  = BrResAllocate(dev, sizeof(br_device_vk_init), BR_MEMORY_DRIVER);
    *init = (br_device_vk_init){
        .version                   = BR_DEVICE_VK_INIT_VERSION,
        .get_instance_proc_address = SDL_Vulkan_GetVkGetInstanceProcAddr(),
        .required_layer_count      = 0,
        .required_layer_names      = NULL,
        .required_extension_count  = ext_count,
        .required_extension_names  = exts,

        .create_surface      = sdl_vk_create_surface,
        .destroy_surface     = sdl_vk_destroy_surface,
        .resize              = sdl_vk_resize,
        .preswap_hook        = sdl_vk_preswap_hook,
        .free                = sdl_vk_free,
        .handle_window_event = sdl_vk_handle_window_event,
    };

    dev->vkrend_init = init;
    return BRE_OK;
}

void DeviceSDL3UnloadVulkan(br_device *dev)
{
    if(dev->vulkan_loaded) {
        SDL_Vulkan_UnloadLibrary();
    }
}
