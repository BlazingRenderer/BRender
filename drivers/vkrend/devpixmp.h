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

    /*
     * No. dependent pixelmaps (offscreens created through match()). For screens, this
     * is everything.
     *
     * This must live outside the union below. Inside as_back it aliased whichever
     * as_front member sat at its offset - the swapchain's surface_capabilities.maxImageCount -
     * so a swapchain recreation reset the count while dependent offscreens were live,
     * and the matching DecRef then underflowed.
     */
    br_int_32 num_refs;

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
             * \brief One render-finished semaphore per swapchain image, indexed by the image
             * the present acquires.
             *
             * Present waits on render_finished[i] for image i. A binary semaphore must not be
             * signalled again until the present waiting on it has completed, and the
             * presentation engine only releases image i once its present is done - so
             * re-acquiring i proves render_finished[i] is free. Nothing fences present
             * completion (the frame fences cover submission, not presentation), so a semaphore
             * shared between frame slots can be re-signalled while a present still waits on
             * it. Across a swapchain recreation the re-acquire argument does not hold (image
             * indices restart), but DeviceVkCreateSwapchain() idles the queue before creating
             * the new swapchain, and that idle drains every earlier present's wait on these
             * semaphores - see DevicePixelmapVkBeginFrame's recreation branch.
             */
            VkSemaphore render_finished[BR_VKREND_MAX_SWAPCHAIN_IMAGES];

            /**
             * \brief Does the swapchain need to be recreated next frame?
             */
            br_boolean need_recreate;
        } as_front;

        struct {
            VkImage       image;
            VkImageView   view;
            VmaAllocation allocation;
            VkImageLayout layout;

            /*
             * The offscreen's palette, a child of this pixelmap. It sits after layout
             * so each field before it keeps its offset: as_sub.parent aliases the union
             * base, so a sub's own as_back must never be read (DevicePixelmapVkResolve
             * is the guard).
             */
            br_device_clut *clut;
        } as_back;

        struct {
            struct br_device_pixelmap *parent;
        } as_sub;
    };
} br_device_pixelmap;

#endif

#ifdef __cplusplus
};
#endif

#endif /* DEVPIXMP_H_ */
