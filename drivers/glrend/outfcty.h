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
     * Renderer facilities to use, and default
     */
    struct br_renderer_facility *renderer_facility;
} br_output_facility;

#endif /* BR_OUTPUT_FACILITY_PRIVATE */

#ifdef __cplusplus
};
#endif
#endif
