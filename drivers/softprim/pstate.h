/*
 * softprim private primitive state structure
 */
#ifndef _SOFTPRIM_PSTATE_H_
#define _SOFTPRIM_PSTATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Mask bits for state
 */
enum {
    MASK_STATE_OUTPUT    = BR_STATE_OUTPUT,
    MASK_STATE_PRIMITIVE = BR_STATE_PRIMITIVE,
    MASK_STATE_CACHE     = BR_STATE_CACHE
};

/*
 * state.prim.flags
 *
 * The public flags are the same bits softrend uses. The internal ones are the
 * matcher flags pentprim derives from the bound buffers in renderBegin, and
 * they are numbered as pentprim numbers them so the generated blocks' flag
 * predicates (drivers/softprim/infogen.pl, match_flags_set/clear) mean the
 * same thing here as there.
 */
enum {
    PRIMF_FORCE_FRONT    = (1 << 0),
    PRIMF_SMOOTH         = (1 << 1),
    PRIMF_DECAL          = (1 << 2),
    PRIMF_DITHER_COLOUR  = (1 << 3),
    PRIMF_DITHER_MAP     = (1 << 4),
    PRIMF_DEPTH_WRITE    = (1 << 5),
    PRIMF_INDEXED_COLOUR = (1 << 6),
    PRIMF_BLEND          = (1 << 7),
    PRIMF_MODULATE       = (1 << 8),
    PRIMF_FOG            = (1 << 9),

    /* Internal flags used for matching */
    PRIMF_OPAQUE_MAP      = (1 << 10),
    PRIMF_NO_SKIP         = (1 << 11),
    PRIMF_PERSPECTIVE     = (1 << 12),
    PRIMF_POWER2          = (1 << 13),
    PRIMF_STRIDE_POSITIVE = (1 << 14),
    PRIMF_PALETTE         = (1 << 15),
    PRIMF_RANGE_ZERO      = (1 << 16),
};

struct input_buffer {
    /*
     * Object on map
     */
    struct br_buffer_stored *buffer;

    /*
     * Cached info about current map
     */
    br_uint_32 width;
    br_uint_32 height;
    br_uint_32 stride;
    br_uint_32 type;
};

struct output_buffer {
    /*
     * Object on pixelmap
     */
    struct br_device_pixelmap *pixelmap;

    /*
     * Cached info about current buffer
     */
    br_uint_32 width;
    br_uint_32 height;
    br_uint_32 stride;
    br_uint_32 type;

    br_boolean viewport_changed;
};

typedef struct br_primitive_state {
    /*
     * Dispatch table
     */
    const struct br_primitive_state_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * Library that this state is attached to
     */
    struct br_primitive_library *plib;

    /*
     * PRIMITIVE part of state
     */
    struct {
        br_uint_32 timestamp;
        br_uint_32 timestamp_major;
        br_uint_32 flags;

        br_int_32 index_base;
        br_int_32 index_range;
        br_token  colour_type;
        br_token  perspective_type;
        br_int_32 subdivide_tolerance;

        struct input_buffer colour_map;
        struct input_buffer index_shade;
        struct input_buffer index_blend;
        struct input_buffer index_fog;
        struct input_buffer screendoor;
        struct input_buffer lighting;
        struct input_buffer bump;

        br_token fog_type;

        struct brp_block *custom_block;

    } prim;

    /*
     * OUTPUT part of state
     */
    struct {
        br_uint_32 timestamp;
        br_uint_32 timestamp_major;

        struct output_buffer colour;
        struct output_buffer depth;

    } out;

    /*
     * Cached info derived from the rest of the state
     */
    struct {
        struct brp_block *last_block;
        br_token          last_type;

        br_scalar comp_offsets[NUM_COMPONENTS];
        br_scalar comp_scales[NUM_COMPONENTS];

        br_uint_32 timestamp_prim;
        br_uint_32 timestamp_out;
    } cache;

} br_primitive_state;

#ifdef __cplusplus
};
#endif
#endif
