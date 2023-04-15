/*
 * Geometry format
 */
#ifndef _GV1MODEL_H_
#define _GV1MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_GEOMETRY_V1_MODEL_PRIVATE

/*
 * Private state of geometry format
 */
typedef struct br_geometry_v1_model {
    /*
     * Dispatch table
     */
    const struct br_geometry_v1_model_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * Renderer type this format is associated with
     */
    struct br_renderer_facility *renderer_facility;

} br_geometry_v1_model;

#endif

#ifdef __cplusplus
};
#endif
#endif
