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
        br_uint_8 write_depth : 1;
        br_uint_8 fog : 1;
    };
    GLenum depth_func;

    br_vector4 fog_colour;
    br_scalar  fog_min;
    br_scalar  fog_max;
    br_scalar  fog_scale;
} br_primitive_state_info_gl;

enum {
    BUFFER_RING_GL_FLAG_ORPHAN = (1 << 0), /**< Orphan the buffer each draw instead of appending to. */
    BUFFER_RING_GL_FLAG_MASK   = BUFFER_RING_GL_FLAG_ORPHAN,
};

typedef struct br_buffer_ring_gl {
    const GladGLContext *gl;

    GLuint   buffers[BR_GLREND_MODEL_RB_FRAMES];
    GLsync   fences[BR_GLREND_MODEL_RB_FRAMES];
    uint32_t flags;
    size_t   frame_index;
    GLintptr offset;
    size_t   buffer_size;
    GLintptr aligned_elem_size;
    GLuint   buffer_index;
    GLenum   binding_point;
} br_buffer_ring_gl;

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

    /*
     * GL Dispatch
     */
    const GladGLContext *gl;

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

    br_hashmap *sampler_pool;

    GLint             uniform_buffer_offset_alignment;
    br_buffer_ring_gl model_ring;

    int has_begun;

    struct {
        br_uint_32 face_group_count;
        br_uint_32 triangles_drawn_count;
        br_uint_32 triangles_rendered_count;
        br_uint_32 vertices_rendered_count;
        br_uint_32 opaque_draw_count;
        br_uint_32 transparent_draw_count;
    } stats;
} br_renderer;

#endif

#ifdef __cplusplus
};
#endif
#endif
