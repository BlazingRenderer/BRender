/*
 * Private renderer structure
 */
#ifndef _RENDERER_H_
#define _RENDERER_H_

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 4)
typedef struct br_immvert_gl {
    br_vector3 position;
    br_vector3 normal;
    br_vector2 map;
    br_colour  colour;
} br_immvert_gl;
#pragma pack(pop)

typedef struct br_sampler_info_gl {
    GLint filter_min;
    GLint filter_mag;
    GLint wrap_s;
    GLint wrap_t;
} br_sampler_info_gl;

typedef struct br_state_info_gl {
    br_sampler_info_gl sampler;

    GLuint colour_map;
    GLuint colour_palette;

    struct {
        br_uint_8 is_blended : 1;
        br_uint_8 is_filtered : 1;
        br_uint_8 is_indexed : 1;
        br_uint_8 disable_colour_key : 1;
        br_uint_8 write_colour : 1;
    };
} br_primitive_state_info_gl;

typedef struct br_sampler_gl {
    br_sampler_info_gl key;
    GLuint             sampler;
} br_sampler_gl;

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

    struct {
        br_immvert_gl pool[1024 * 3];
        br_size_t     next;
        GLuint        vao;
        GLuint        vbo;
    } trans;

    state_all state;

    br_sampler_gl sampler_pool[BR_GLREND_MAX_SAMPLERS];
    br_size_t     sampler_pool_size;
    br_size_t     sampler_count;

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
