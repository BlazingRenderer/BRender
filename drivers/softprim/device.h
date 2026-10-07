/*
 * softprim private device structure
 */
#ifndef _SOFTPRIM_DEVICE_H_
#define _SOFTPRIM_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

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

} br_device;

#define DeviceSoftPrimResource(d) (((br_device *)d)->res)

#ifdef __cplusplus
};
#endif
#endif
