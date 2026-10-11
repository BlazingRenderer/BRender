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
 * Actor state that a glTF node has no way to carry.
 *
 * A node is a transform plus optional references to a mesh, a camera or a
 * light; a br_actor additionally carries br_actor::render_style, which selects
 * the topology the engine draws - points, edges or faces - and applies to the
 * whole subtree below the actor that sets it. glTF's primitive
 * `mode` is not it: mode says what the indices in the file describe, while
 * render_style says what BRender draws, and BRender draws an edge or point
 * topology over a triangle list. Without it every actor loaded from a .gltf is
 * BR_RSTYLE_DEFAULT and the point and line rasterisers have no witness at all.
 *
 * This is actor state rather than mesh state, so it belongs on the node: one
 * mesh may be referenced by two nodes and drawn as faces by one and edges by
 * the other. Model-level state (br_model::pivot, ::flags, ::crease_angle) is
 * not here - it belongs on the mesh, because it is shared with it. Neither
 * br_actor::type nor the representation of br_actor::t is carried here: a
 * round trip loses both, and both were audited and deliberately left out.
 */
typedef struct cgltf_brender_actor {
    /* A BR_RSTYLE_* value; absent means BR_RSTYLE_DEFAULT. */
    cgltf_int render_style;
} cgltf_brender_actor;

/*
 * Data URI prefixes for an image whose PNG channels are the raw samples of a
 * lookup table, rather than colour.
 *
 * A material's lookup table (index_shade, index_blend, index_fog or screendoor)
 * is a pixelmap with no palette that the rasterisers index directly. For the
 * indexed output it is BR_PMT_INDEX_8 and the samples are indices; but a shade
 * table's type must equal the output format, so an RGB output needs an RGB-typed
 * table whose samples are the output words. Either way there is no palette to
 * expand through and any conversion to colour would lose the only thing that
 * matters: the sample bytes. The marker names the type so the loader rebuilds the
 * exact pixelmap the renderer needs - a three-channel PNG is not enough, because
 * BR_PMT_RGB_555 and BR_PMT_RGB_565 are both two bytes per sample and sharing one
 * marker would collapse them onto whichever the loader guessed.
 */
#define CGLTF_BR_INDEX_8_PNG_URI "data:image/png;brender=index8;base64,"
#define CGLTF_BR_RGB_555_PNG_URI "data:image/png;brender=rgb555;base64,"
#define CGLTF_BR_RGB_565_PNG_URI "data:image/png;brender=rgb565;base64,"
#define CGLTF_BR_RGB_888_PNG_URI "data:image/png;brender=rgb888;base64,"

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

typedef struct cgltf_brender_light_region {
    cgltf_size   planes_count;
    cgltf_float (*planes)[4];
} cgltf_brender_light_region;

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

    /*
     * Cutoff volumes (br_light_volume). Each region is a convex intersection of
     * half-spaces; a light reaches a vertex inside any region, and fades
     * linearly to nothing over `falloff_distance` beyond a boundary. The planes
     * are in the light actor's local space.
     */
    cgltf_float                 falloff_distance;
    cgltf_brender_light_region *regions;
    cgltf_size                  regions_count;
} cgltf_brender_light;

#endif /* CGLTF_BRENDER_H_ */
