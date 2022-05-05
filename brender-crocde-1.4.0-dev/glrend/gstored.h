/*
 * Geometry format
 */
#ifndef _GSTORED_H_
#define _GSTORED_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gl_vertex_f {
    /*
     * Point in model space
     */
    br_vector3_f p;

    /*
     * Mapping coordinates
     */
    br_vector2_f map;

    /*
     * Surface normal at vertex
     */
    br_vector3_f n;

    /*
     * Colour
     */
    br_colour c;
} gl_vertex_f;

typedef struct gl_groupinfo {
    /*
     * Count for glDrawElements(). Use GL_TRIANGLES.
     */
    GLsizei count;

    /*
     * Offset for glDrawElements()
     */
    void *offset;

#if DEBUG
    /*
     * The group itself
     */
    struct v11group_f *group;
#endif

    /*
     * Stored renderer state for the group
     */
    br_renderer_state_stored *stored;
} gl_groupinfo;

#ifdef BR_GEOMETRY_STORED_PRIVATE

/*
 * Private state of geometry format
 */
typedef struct br_geometry_stored {
    /*
     * Dispatch table
     */
    const struct br_geometry_stored_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    struct br_geometry_v1_model *gv1model;

    br_boolean        shared;
    struct v11model_f *model;

    GLuint gl_vao;
    GLuint gl_vbo;
    GLuint gl_ibo;

    gl_groupinfo *groups;
} br_geometry_stored;

#endif

#ifdef __cplusplus
};
#endif
#endif /* _GSTORED_H_ */


