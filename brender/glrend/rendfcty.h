/*
 * Private renderer facility structure
 */
#ifndef _RENDFCTY_H_
#define _RENDFCTY_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_RENDERER_FACILITY_PRIVATE

/*
 * Private state of renderer facility
 */
typedef struct br_renderer_facility {
    /*
     * Dispatch table
     */
    const struct br_renderer_facility_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * List of objects associated with this device
     */
    void *object_list;

} br_renderer_facility;

#endif

#ifdef __cplusplus
};
#endif
#endif
