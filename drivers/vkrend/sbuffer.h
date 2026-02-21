/*
 * A stored input buffer: a colour map uploaded into a device image.
 */
#ifndef VKREND_SBUFFER_H_
#define VKREND_SBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_BUFFER_STORED_PRIVATE

/*
 * Block structure.
 *
 * The map's pixels are uploaded once, at update(), into a device image the renderer
 * samples through a push descriptor. An INDEX_8 map is two images: the index plane
 * (R8_UINT, sampled through a usampler2D) and the palette from the pixelmap's map,
 * sampled through a sampler2D. A host pixelmap is uploaded; a same-device offscreen is
 * instead sampled where its image lies, through a transient view of it (the
 * render-to-texture case) - see BufferStoredVkUpdateDevice().
 */
typedef struct br_buffer_stored {
    /*
     * Dispatch table
     */
    const struct br_buffer_stored_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    br_device   *device;
    br_renderer *renderer;

    /*
     * Source pixelmap
     */
    br_pixelmap *source;

    /*
     * Copy of source flags
     */
    br_uint_16 source_flags;

    const br_pixelmap_vk_fmt *fmt;

    /*
     * Whether the source is INDEX_8, i.e. the image holds indices and clut_image holds
     * the palette. Both planes rest in VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL and are
     * freed with this object.
     */
    br_boolean indexed;

    /*
     * Whether the source is a device pixelmap sampled where its image lies rather than a
     * host upload. `view` is then a transient view of that image carrying the format
     * table's swizzle, and device_image records the image it was built for so a resized
     * source can be declined.
     */
    br_boolean device_source;
    VkImage    device_image;

    VkImage       image;
    VkImageView   view;
    VmaAllocation allocation;
    br_uint_32    width;
    br_uint_32    height;

    /*
     * The palette for an indexed source, or zero when the source is a colour format.
     */
    const br_pixelmap_vk_fmt *clut_fmt;
    VkImage                   clut_image;
    VkImageView               clut_view;
    VmaAllocation             clut_allocation;
    br_uint_32                clut_width;
    br_uint_32                clut_height;

    struct br_tv_template *templates;
} br_buffer_stored;

#endif

#ifdef __cplusplus
};
#endif
#endif /* VKREND_SBUFFER_H_ */
