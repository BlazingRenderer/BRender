/*
 * Geometry format
 */
#ifndef _GLIGHTING_H_
#define _GLIGHTING_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_GEOMETRY_LIGHTING_PRIVATE

/*
 * Private state of geometry format
 */
typedef struct br_geometry_lighting {
    /*
     * Dispatch table
     */
    const struct br_geometry_lighting_dispatch *dispatch;

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
} br_geometry_lighting;

#endif

#ifdef __cplusplus
};
#endif
#endif
