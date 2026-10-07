/*
 * softprim stored buffer structure
 */
#ifndef _SOFTPRIM_SBUFFER_H_
#define _SOFTPRIM_SBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Flags
 */
enum {
    SBUFF_SHARED = 0x0001, /* Data is shared with user */
};

typedef struct br_buffer_stored {
    /*
     * Dispatch table
     */
    const struct br_buffer_stored_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * Primitive library
     */
    struct br_primitive_library *plib;

    /*
     * Flags
     */
    br_uint_32 flags;

    /*
     * Reduced info about buffer
     */
    softprim_buffer buffer;

    /*
     * Duplicated pixelmap (or NULL if none)
     */
    struct br_device_pixelmap *local_pm;

} br_buffer_stored;

#ifdef __cplusplus
};
#endif
#endif
