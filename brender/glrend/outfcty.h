/*
 * Private output facility structure
 */
#ifndef _OUTFCTY_H_
#define _OUTFCTY_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_OUTPUT_FACILITY_PRIVATE

/*
 * Private state of output type
 */
typedef struct br_output_facility {
    /*
     * Dispatch table
     */
    const struct br_output_facility_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * List of instances associated with type
     */
    void *object_list;

    /*
     * Size, minimum and maximum size of mode in pixels
     */
    br_int_32 width;
    br_int_32 height;

    /*
     * Bit depth
     */
    br_int_32 colour_bits;

    /*
     * Pixelmap types
     */
    br_int_32 colour_type;

    /*
     * Is there a CLUT?
     */
    br_boolean indexed;

    /*
     * The monitor this mode is on.
     */
    br_int_32 monitor;

    /*
     * This is a "custom" output facility. Represents a
     * "custom" screen resolution (ala resizing the window).
     * It will be destroyed when all dependent pixelmaps are.
     *
     * If not set, this represents a screen mode.
     */
    br_boolean temporary;

    /*
     * Renderer facilities to use, and default
     */
    struct br_renderer_facility *renderer_facility;
} br_output_facility;

#endif /* BR_OUTPUT_FACILITY_PRIVATE */

#ifdef __cplusplus
};
#endif
#endif

