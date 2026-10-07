/*
 * softprim private primitive library structure
 */
#ifndef _SOFTPRIM_PLIB_H_
#define _SOFTPRIM_PLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct br_primitive_library {
    /*
     * Dispatch table
     */
    const struct br_primitive_library_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * Colour buffer currently locked for rendering, or NULL
     */
    br_device_pixelmap *colour_buffer;

    /*
     * List of objects associated with this library
     */
    void *object_list;

    /*
     * Reported processor information
     */
    br_token   processor_type;
    br_boolean use_mmx;

} br_primitive_library;

#ifdef __cplusplus
};
#endif
#endif
