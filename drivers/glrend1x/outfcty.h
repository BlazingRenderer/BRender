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
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * List of instances associated with type
     */
    void *object_list;
} br_output_facility;

#ifdef __cplusplus
};
#endif
#endif
