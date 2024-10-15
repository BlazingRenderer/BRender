/*
 * Private output facility structure
 */
#ifndef _OUTFCTY_H_
#define _OUTFCTY_H_

#ifdef __cplusplus
extern "C" {
#endif

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
    char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * List of instances associated with type
     */
    void *object_list;

    /* Size of mode in pixels
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
    br_uint_8 colour_type;

    /*
     * Monitor index
     */
    br_int_32 monitor;
} br_output_facility;

#define OutputFacilityVGAType(c) (((br_output_facility *)c)->colour_type)

#ifdef __cplusplus
};
#endif
#endif
