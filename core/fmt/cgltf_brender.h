#ifndef CGLTF_BRENDER_H_
#define CGLTF_BRENDER_H_

#if !defined(CGLTF_H_INCLUDED__)
#error Please include cgltf.h first
#endif

typedef enum cgltf_brender_light_type {
    cgltf_brender_light_type_invalid,
    cgltf_brender_light_type_point,
    cgltf_brender_light_type_direct,
    cgltf_brender_light_type_spot,
    cgltf_brender_light_type_ambient,
    cgltf_brender_light_type_max,
} cgltf_brender_light_type;

typedef struct cgltf_image cgltf_image;

/*
 * Data URI prefix for an image whose single PNG channel is the raw indices of a
 * BR_PMT_INDEX_8 pixelmap, rather than colour.
 *
 * A material's lookup tables (index_shade, index_blend, index_fog and screendoor)
 * are INDEX_8 pixelmaps with no palette - that is the engine's own definition of a
 * lookup table, and the rasterisers index them directly. There is therefore no
 * palette to expand them through, and any conversion to colour would lose the only
 * thing that matters: the index bytes. Marking the payload lets the loader read the
 * indices back verbatim instead of treating the image as colour.
 */
#define CGLTF_BR_INDEX_8_PNG_URI "data:image/png;brender=index8;base64,"

typedef struct cgltf_brender_material {
    char        *identifier;
    cgltf_float  colour[3];
    cgltf_float  opacity;
    cgltf_float  ka;
    cgltf_float  kd;
    cgltf_float  ks;
    cgltf_float  power;
    cgltf_int    flags;
    cgltf_float  map_transform[6];
    cgltf_int    mode;
    cgltf_int    index_base;
    cgltf_int    index_range;
    cgltf_image *colour_map;
    cgltf_image *screendoor;
    cgltf_image *index_shade;
    cgltf_image *index_blend;
    cgltf_image *index_fog;
    // TODO: extra_surf
    // TODO: extra_prim
    cgltf_float fog_min;
    cgltf_float fog_max;
    cgltf_float fog_colour[3];
    cgltf_int   subdivide_tolerance;
    cgltf_float depth_bias;
} cgltf_brender_material;

typedef struct cgltf_brender_light {
    char                    *identifier;
    cgltf_brender_light_type type;
    cgltf_bool               view_space;
    cgltf_bool               linear_falloff;
    cgltf_float              colour[3];
    cgltf_float              attenuation_c;
    cgltf_float              attenuation_l;
    cgltf_float              attenuation_q;
    cgltf_float              cone_outer;
    cgltf_float              cone_inner;
    cgltf_float              radius_outer;
    cgltf_float              radius_inner;

    /* TODO: volumes, if I ever see any. */
} cgltf_brender_light;

#endif /* CGLTF_BRENDER_H_ */
