/*
 * Geometry format for version 1 models
 */
#ifndef VKREND_GV1MODEL_H_
#define VKREND_GV1MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_GEOMETRY_V1_MODEL_PRIVATE

typedef struct br_geometry_v1_model {
    /*
     * Dispatch table
     */
    const struct br_geometry_v1_model_dispatch *dispatch;

    const char *identifier;

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
#endif /* VKREND_GV1MODEL_H_ */
