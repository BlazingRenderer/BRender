/*
 * Geometry format
 */
#ifndef _GV1BUCKT_H_
#define _GV1BUCKT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_GEOMETRY_V1_BUCKETS_PRIVATE

/*
 * Private state of geometry format
 */
typedef struct br_geometry_v1_buckets {
    /*
     * Dispatch table
     */
    const struct br_geometry_v1_buckets_dispatch *dispatch;

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

} br_geometry_v1_buckets;

#endif

#ifdef __cplusplus
};
#endif
#endif
