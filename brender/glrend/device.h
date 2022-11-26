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
     * Device-wide renderer facility.
     */
    struct br_renderer_facility *renderer_facility;

    /*
     * Device-wide VIDEO instance.
     */
    VIDEO video;

    /*
     * System-specific OpenGL function pointers.
     */
    br_device_gl_ext_procs ext_procs;

    /*
     * OpenGL context
     */
    void *gl_context;

    /*
     * Vertex shader source
     */
    char *vertex_shader;

    /*
     * Fragment shader source
     */
    char *fragment_shader;

    const char *gl_version;
    const char *gl_vendor;
    const char *gl_renderer;

    br_font_gl font_fixed3x5;
    br_font_gl font_prop4x6;
    br_font_gl font_prop7x9;
} br_device;

#endif /* BR_DEVICE_PRIVATE */
#endif /* _DEVICE_H_ */
