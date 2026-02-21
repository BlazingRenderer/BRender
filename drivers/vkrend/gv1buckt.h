/*
 * Geometry format for the order table buckets spat out by a prior hidden-surface render
 */
#ifndef VKREND_GV1BUCKT_H_
#define VKREND_GV1BUCKT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_GEOMETRY_V1_BUCKETS_PRIVATE

typedef struct br_geometry_v1_buckets {
    /*
     * Dispatch table
     */
    const struct br_geometry_v1_buckets_dispatch *dispatch;

    const char *identifier;

    struct br_device *device;

    /*
     * Renderer type this format is associated with
     */
    struct br_renderer_facility *renderer_facility;

} br_geometry_v1_buckets;

#endif

#ifdef __cplusplus
};
#endif
#endif /* VKREND_GV1BUCKT_H_ */
