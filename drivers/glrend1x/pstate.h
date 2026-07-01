/*
 * Private primitive state structure
 */
#ifndef _PSTATE_H_
#define _PSTATE_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MASK_STATE_OUTPUT    = BR_STATE_OUTPUT,
    MASK_STATE_PRIMITIVE = BR_STATE_PRIMITIVE,
    MASK_STATE_CACHE     = BR_STATE_CACHE
};

/* Public primitive flags — bit positions */
enum {
    PRIMF_FORCE_FRONT_BIT,
    PRIMF_DITHER_COLOUR_BIT,
    PRIMF_DITHER_MAP_BIT,
    PRIMF_DEPTH_WRITE_BIT,
    PRIMF_COLOUR_KEY_BIT,
    PRIMF_INDEXED_COLOUR_BIT,
    PRIMF_BLEND_BIT,
    PRIMF_MODULATE_BIT,
    PRIMF_FOG_BIT,
};

/* Public primitive flags — bit values */
enum {
    PRIMF_FORCE_FRONT    = (1 << PRIMF_FORCE_FRONT_BIT),
    PRIMF_DITHER_COLOUR  = (1 << PRIMF_DITHER_COLOUR_BIT),
    PRIMF_DITHER_MAP     = (1 << PRIMF_DITHER_MAP_BIT),
    PRIMF_DEPTH_WRITE    = (1 << PRIMF_DEPTH_WRITE_BIT),
    PRIMF_COLOUR_KEY     = (1 << PRIMF_COLOUR_KEY_BIT),
    PRIMF_INDEXED_COLOUR = (1 << PRIMF_INDEXED_COLOUR_BIT),
    PRIMF_BLEND          = (1 << PRIMF_BLEND_BIT),
    PRIMF_MODULATE       = (1 << PRIMF_MODULATE_BIT),
    PRIMF_FOG            = (1 << PRIMF_FOG_BIT),
};

/* Extra internal flag: whether a texture is bound */
#define PRIMF_TEXTURE_BUFFER (1 << 24)

typedef struct state_primitive {
    br_uint_32               flags;
    br_token                 colour_type;
    br_token                 depth_test;
    br_token                 blend_mode;
    br_token                 shading_mode;
    br_token                 perspective_type;
    struct br_buffer_stored *colour_map;
    br_token                 map_width_limit;
    br_token                 map_height_limit;
    br_token                 filter;
    br_token                 fog_type;
    br_scalar                fog_min;
    br_scalar                fog_max;
    br_colour                fog_colour;
} state_primitive;

typedef struct state_output {
    br_device_pixelmap *colour;
    br_device_pixelmap *depth;
} state_output;

typedef struct br_primitive_state {
    const struct br_primitive_state_dispatch *dispatch;
    const char                               *identifier;
    struct br_device                         *device;
    struct br_primitive_library              *plib;

    state_primitive prim;
    state_output    out;

    struct {
        /* Last GL state pushed — cached flags + texture handle */
        br_uint_32 last_flags;
        GLuint     last_tex;
        br_token   last_type;
    } cache;

} br_primitive_state;

#ifdef __cplusplus
};
#endif
#endif
