#ifndef DEVPIXMP_H_
#define DEVPIXMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_DEVICE_PIXELMAP_PRIVATE

typedef struct br_device_pixelmap {
    /*
     * Dispatch table
     */
    const struct br_device_pixelmap_dispatch *dispatch;

    /*
     * Standard handle identifier
     */
    const char *pm_identifier;

    /** Standard pixelmap members (not including identifier) **/

    BR_PIXELMAP_MEMBERS

    /** End of br_pixelmap fields **/

    br_device          *device;
    br_output_facility *output_facility;

    /*
     * Type of buffer (when matched)
     */
    br_token use_type;

    /*
     * Current screen pixelmap. Valid on ALL types. The screen points to itself.
     */
    struct br_device_pixelmap *screen;

    union {
        struct {
            void             *window_handle;
            VkSurfaceKHR      surface;
            DeviceVkSwapchain swapchain;

            /**
             * \brief The current frame index.
             */
            br_uint_32                  frame_index;
            VkFence                     frame_fences[BR_VKREND_MAX_SWAPCHAIN_IMAGES]; /* non-owning, for convenience. */
            br_device_pixelmap_vk_frame frames[BR_VKREND_MAX_SWAPCHAIN_IMAGES];

            /**
             * \brief Does the swapchain need to be recreated next frame?
             */
            br_boolean                  need_recreate;
        } as_front;
    };
} br_device_pixelmap;

#endif

#ifdef __cplusplus
};
#endif

#endif /* DEVPIXMP_H_ */
