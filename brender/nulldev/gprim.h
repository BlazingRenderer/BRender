/*
 * Geometry format
 */
#ifndef _GPRIM_H_
#define _GPRIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_GEOMETRY_PRIMITIVES_PRIVATE

/*
 * Private state of geometry format
 */
typedef struct br_geometry_primitives {
    /*
     * Dispatch table
     */
    const struct br_geometry_primitives_dispatch *dispatch;

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

    /*
     * Object query templates
     */
    struct br_tv_template *templates;
} br_geometry_primitives;

#endif

#ifdef __cplusplus
};
#endif
#endif
