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
 * A whole BRender pixelmap, as the engine's own .pix serialiser writes it. glTF
 * has no notion of a pixel type, so this is how one survives a save and a load:
 * the type, the samples, the identifier and the palette, with nothing here to
 * keep in step when a pixel type is added. Only BRender can read it - a debug
 * interchange, not a viewable image.
 */
#define CGLTF_BR_PIX_URI "data:application/vnd.brender.pix;base64,"

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
