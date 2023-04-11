#ifndef _DEVICE_H_
#define _DEVICE_H_

#ifdef BR_DEVICE_PRIVATE

/*
 * Private state of device
 */
typedef struct br_device {
    /*
     * Dispatch table
     */
    const struct br_device_dispatch *dispatch;

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

    /*
     * Anchor for all device's resources
     */
    void *res;

    /*
     * Driver-wide template store
     */
    struct device_templates templates;

    /*
     * Device-wide output facility.
     */
    struct br_output_facility *output_facility;

    /*
     * Device-wide renderer facility.
     */
    struct br_renderer_facility *renderer_facility;
} br_device;

#endif /* BR_DEVICE_PRIVATE */
#endif /* _DEVICE_H_ */
