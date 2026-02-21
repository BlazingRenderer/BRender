/*
 * Geometry format for a version 1 model's stored GPU buffers
 */
#ifndef VKREND_GSTORED_H_
#define VKREND_GSTORED_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Vertex layout of the per-model VBO. Matches the attribute descriptions in the scene
 * pipeline (gv1buckt.c) and the locations declared by render.vert.glsl.
 */
typedef struct br_geometry_stored_vertex {
    br_vector3_f position;
    br_vector3_f normal;
    br_vector2_f map;
    br_vector4_f colour;
} br_geometry_stored_vertex;

typedef struct br_geometry_stored_group {
    /*
     * Element count for vkCmdDrawIndexed(): 3 * nfaces.
     */
    br_uint_32 count;

    /*
     * Byte offset of this group's first index in the IBO.
     */
    br_uint_32 offset;
} br_geometry_stored_group;

#ifdef BR_GEOMETRY_STORED_PRIVATE

typedef struct br_geometry_stored {
    /*
     * Dispatch table
     */
    const struct br_geometry_stored_dispatch *dispatch;

    const char *identifier;

    struct br_device *device;

    struct br_geometry_v1_model *gv1model;

    br_boolean       shared;
    struct v11model *model;

    /*
     * Device-scoped vertex and index buffers, built once per stored model. Freed with
     * this object.
     */
    VkBuffer      vertex_buffer;
    VmaAllocation vertex_allocation;
    VkBuffer      index_buffer;
    VmaAllocation index_allocation;

    br_geometry_stored_group *groups;
} br_geometry_stored;

#endif

#ifdef __cplusplus
};
#endif
#endif /* VKREND_GSTORED_H_ */
