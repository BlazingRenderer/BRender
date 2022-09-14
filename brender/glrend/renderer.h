/*
 * Private renderer structure
 */
#ifndef _RENDERER_H_
#define _RENDERER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_RENDERER_PRIVATE

typedef struct br_renderer {
    /*
     * Dispatch table
     */
    const struct br_renderer_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * List of objects associated with this renderer
     */
    void *object_list;


    br_device_pixelmap   *pixelmap;
    br_renderer_facility *renderer_facility;

    /*
     * Pool of GLSTATE_STACK, used for snapshotting state
     * when deferring.
     */
    br_pool *state_pool;

    GLSTATE state;

    int has_begun;
} br_renderer;

#endif

#ifdef __cplusplus
};
#endif
#endif


