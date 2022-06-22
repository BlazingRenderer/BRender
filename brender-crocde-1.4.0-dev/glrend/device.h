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
     * SDL window handle
     */
    SDL_Window *sdl_window;

    /*
     * SDL OpenGL context
     */
    SDL_GLContext *sdl_context;

    /*
     * Vertex shader source
     */
    char *vertex_shader;

    /*
     * Fragment shader source
     */
    char *fragment_shader;
} br_device;

#endif /* BR_DEVICE_PRIVATE */
#endif /* _DEVICE_H_ */
