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
     * Pool of state_stack, used for snapshotting state
     * when deferring.
     */
    br_pool *state_pool;

    state_all state;

    int has_begun;

    struct {
        br_uint_32 face_group_count;
        br_uint_32 triangles_drawn_count;
        br_uint_32 triangles_rendered_count;
        br_uint_32 vertices_rendered_count;
    } stats;
} br_renderer;

#endif

#ifdef __cplusplus
};
#endif
#endif
