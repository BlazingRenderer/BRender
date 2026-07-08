/*
 * Private device CLUT state
 */
#ifndef _DEVCLUT_H_
#define _DEVCLUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Private state of device CLUT
 */
typedef struct br_device_clut {
    /*
     * Dispatch table
     */
    const struct br_device_clut_dispatch *dispatch;

    /*
     * Standard handle identifier
     */
    const char *identifier;

    /*
     * Device pointer
     */
    br_device *device;

    /*
     * Internal texture, used to make our lives easier.
     */
    br_pixelmap *pm;

    /*
     * Storage.
     */
    br_colour entries[BR_GLREND_MAX_CLUT_ENTRIES];
} br_device_clut;

#ifdef __cplusplus
};
#endif
#endif
