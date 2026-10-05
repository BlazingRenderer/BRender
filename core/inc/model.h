/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: model.h 1.4 1998/07/21 17:33:17 jon Exp $
 * $Locker: $
 *
 * In-memory structures for models, both public and private areas
    Last change:  TN    9 Apr 97    4:40 pm
 */
#ifndef _MODEL_H_
#define _MODEL_H_

/**
 * \brief The vertex data structure, describing a single vertex in a model.
 */
typedef struct br_vertex {
    /**
     * \brief The co-ordinates of a point (in the model's co-ordinate space) representing the vertex of
     *        a group of faces (typically triangular).
     *
     * For example, a cube has eight vertices. If it had unit side and was centred about (0,0,0), it
     * would have vertices (0.5,0.5,0.5), (0.5,0.5,-0.5), etc. Faces are polygons (typically triangles)
     * described in terms of a number of vertex indices.
     */
    br_vector3 p;
    /**
     * \brief The 2D co-ordinates at which this vertex appears in an infinitely* tiled texture map.
     *        (Footnote: * Subject of course to limits of br_scalar representation.)
     */
    br_vector2 map;

    /**
     * \brief Pre-computed lighting index at this vertex.
     *
     * See br_material for how this relates to pre-lit materials.
     */
    br_uint_8 index;
    /**
     * \brief Pre-computed light level at this vertex (in terms of colour intensities†).
     *
     * See br_material for how this relates to pre-lit materials. (Footnote: † Not all platforms support
     * coloured lights.)
     */
    br_uint_8 red;
    /**
     * \brief Pre-computed light level at this vertex (in terms of colour intensities†).
     *
     * See br_material for how this relates to pre-lit materials. (Footnote: † Not all platforms support
     * coloured lights.)
     */
    br_uint_8 grn;
    /**
     * \brief Pre-computed light level at this vertex (in terms of colour intensities†).
     *
     * See br_material for how this relates to pre-lit materials. (Footnote: † Not all platforms support
     * coloured lights.)
     */
    br_uint_8 blu;

    /*
     * Private fields
     */
    br_uint_16  _pad0;
    br_fvector3 n; /* Surface normal at vertex	*/

} br_vertex;

/**
 * \brief The face data structure, describing a single triangular face.
 */
typedef struct br_face {
    /**
     * \brief An array of vertex indices specifying the vertices of this face.
     *
     * This defines a polygon of the model's the surface. The order in which vertices are listed is
     * important. The primary, visible side of a face from the viewpoint has its vertices listed in
     * anticlockwise order. See br_model and br_vertex.
     */
    br_uint_16   vertices[3];
    /**
     * \brief A 16 bit field in which each bit represents a smoothing group.
     *
     * If, when smooth-shading a surface, two adjacent faces share a smoothing group, the edge between
     * them will be smooth.
     */
    br_uint_16   smoothing;
    /**
     * \brief Pointer to the material structure associated with this face.
     *
     * Note that if this is NULL and the face is part of a model actor's model, then the model actor's
     * material (as specified or inherited) will be used.
     */
    br_material *material;

    /*
     * Colour for prelit models
     */
    br_uint_8 index;
    br_uint_8 red;
    br_uint_8 grn;
    br_uint_8 blu;

    /**
     * \brief Face flags, indicating whether the edges of the face abut co-planar faces, and thus do not
     *        need to be drawn in the wire-frame render style BR_RSTYLE_EDGES.
     *
     * The following table describes each flag.
     *
     * \li BR_FACEF_COPLANAR_0 — The face adjoining edge 0 is co-planar with this face.
     * \li BR_FACEF_COPLANAR_1 — The face adjoining edge 1 is co-planar with this face.
     * \li BR_FACEF_COPLANAR_2 — The face adjoining edge 2 is co-planar with this face.
     */
    br_uint_8  flags;
    br_uint_8  _pad0;
    br_uint_32 _pad1;

    br_fvector3 n; /* Plane equation of face				*/
    br_scalar   d;
} br_face;

/*
 * Bits for face flags
 */
enum {
    BR_FACEF_COPLANAR_0 = 0x01, /* The face adjoining edge 0 is coplanar with this face */
    BR_FACEF_COPLANAR_1 = 0x02, /*         ""              1          ""                */
    BR_FACEF_COPLANAR_2 = 0x04, /*         ""              2          ""                */
    BR_FACEF_COPLANAR_3 = 0x08, /*         ""              3          ""                */

    BR_FACEF_QUAD_MASK  = 0x70, /* For quad based texture mapping, which half of a quad this triangle represents */
    BR_FACEF_QUAD_012   = 0x00, /* Vertices 0, 1 and 2 of this face are vertices 0, 1 and 2 of a quad */
    BR_FACEF_QUAD_123   = 0x10, /* Vertices 0, 1 and 2 of this face are vertices 1, 2 and 3 of a quad */
    BR_FACEF_QUAD_230   = 0x20, /* Vertices 0, 1 and 2 of this face are vertices 2, 3 and 0 of a quad */
    BR_FACEF_QUAD_301   = 0x30, /* Vertices 0, 1 and 2 of this face are vertices 3, 0 and 1 of a quad */
    BR_FACEF_QUAD_032   = 0x40, /* Vertices 0, 1 and 2 of this face are vertices 0, 3 and 2 of a quad */
    BR_FACEF_QUAD_103   = 0x50, /* Vertices 0, 1 and 2 of this face are vertices 1, 0 and 3 of a quad */
    BR_FACEF_QUAD_210   = 0x60, /* Vertices 0, 1 and 2 of this face are vertices 2, 1 and 0 of a quad */
    BR_FACEF_QUAD_321   = 0x70, /* Vertices 0, 1 and 2 of this face are vertices 3, 2 and 1 of a quad */
    BR_FACEF_QUAD_SHIFT = 4
};

/*
 * Primitive list added for future expansion
 */

typedef struct br_primitive_list {
    struct br_primitive_list *next;      /* Ptr to next primitive list. */
    br_uint_32                prim_type; /* Primitive type (see below). */
    br_uint_16                nprims;    /* Number of primitives. */
    br_uint_16                nspares;   /* Number of 'spare' structures. */
    void                     *prim;      /* Ptr to array of primitives. */
    void                     *spare;     /* Ptr to array of spares. */
} br_primitive_list;

/*
 * Bits for prim_type field above.
 */

enum {
    BR_PRIM_VERTEX_SGL = 0x01, /* prim points to br_vertex_single structures. */
    BR_PRIM_VERTEX_DBL = 0x02, /* prim points to br_vertex_double structures. */
    BR_PRIM_POINT      = 0x03, /* prim points to br_point_prim structures. */
    BR_PRIM_LINE       = 0x04, /* prim points to br_line structures. */
    BR_PRIM_TRIANGLE   = 0x05, /* prim points to br_triangle structures. */
    BR_PRIM_QUAD       = 0x06, /* prim points to br_quad structures. */
    BR_PRIM_TRI_STRIP  = 0x07, /* prim points to br_tri_strip structures. */
    BR_PRIM_TRI_FAN    = 0x08, /* prim points to br_tri_fan structures. */
    BR_PRIM_QUAD_STRIP = 0x09  /* prim points to br_quad_strip structures. */
};

/*
 * New primitive types
 */

typedef struct br_vertex_single { /* Vertex for single-textured primitives. */
    br_vector3 p;                 /* Point in model space		*/
    br_vector2 map;               /* Mapping coordinates		*/

    /*
     * Colour for prelit models
     */
    br_uint_8 alpha;
    br_uint_8 red;
    br_uint_8 grn;
    br_uint_8 blu;

    /*
     * Private fields
     */
    br_uint_16  _pad0;
    br_fvector3 n; /* Surface normal at vertex	*/
} br_vertex_single;

typedef struct br_vertex_double { /* Vertex for double-textured primitives. */
    br_vector3 p;                 /* Point in model space. */
    br_vector2 map0;              /* Primary U/V */

    br_uint_8 alpha0; /* Primary ARGB */
    br_uint_8 red0;
    br_uint_8 grn0;
    br_uint_8 blu0;

    br_vector2 map1;   /* Secondary U/V */
    br_uint_8  alpha1; /* Secondary ARGB */
    br_uint_8  red1;
    br_uint_8  grn1;
    br_uint_8  blu1;

    /*
     * Private fields
     */
    br_fvector3 n; /* Surface normal at vertex	*/

} br_vertex_double;

typedef struct br_point_prim { /* Point primitive. */
    br_uint_16   vertices[1];  /* Vertex index. */
    br_material *material;     /* Point material (or NULL). */
} br_point_prim;

typedef struct br_line {      /* Line primitive. */
    br_uint_16   vertices[2]; /* Start & end vertices. */
    br_material *material;    /* Line material (or NULL). */

    /*
     * Colours for pre-lit models
     */
    br_uint_8 alpha0;
    br_uint_8 red0;
    br_uint_8 grn0;
    br_uint_8 blu0;

    br_uint_8 alpha1;
    br_uint_8 red1;
    br_uint_8 grn1;
    br_uint_8 blu1;
} br_line;

typedef struct br_triangle {  /* Triangle primitive. */
    br_uint_16   vertices[3]; /* Vertices around triangle	*/
    br_uint_16   smoothing;   /* Controls if shared edges are smooth */
    br_material *material;    /* Triangle material (or NULL) */

    /*
     * Colours for prelit models
     */
    br_uint_8 alpha0;
    br_uint_8 red0;
    br_uint_8 grn0;
    br_uint_8 blu0;

    br_uint_8 alpha1;
    br_uint_8 red1;
    br_uint_8 grn1;
    br_uint_8 blu1;

    br_uint_8 flags; /* Bits 0,1 and 2 denote internal edges	*/

    br_fvector3 n; /* Plane equation of face				*/
    br_scalar   d;
} br_triangle;

typedef struct br_quad {      /* Quadrilateral primitive. */
    br_uint_16   vertices[4]; /* Vertices around quad. */
    br_uint_16   smoothing;   /* Controls if shared edges are smooth. */
    br_material *material;    /* Quad material (or NULL). */

    /*
     * Colour for pre-lit models
     */
    br_uint_8 alpha0;
    br_uint_8 red0;
    br_uint_8 grn0;
    br_uint_8 blu0;

    br_uint_8 alpha1;
    br_uint_8 red1;
    br_uint_8 grn1;
    br_uint_8 blu1;

    br_uint_8 flags; /* Bits 0, 1, 2 and 3 denote internal edges. */

    br_fvector3 n; /* Plane equation of quad. */
    br_scalar   d;
} br_quad;

typedef struct br_strip_face_data {
    br_uint_16 smoothing; /* Smoothing group. */
    /*
     * Colours for pre-lit models
     */
    br_uint_8 alpha0;
    br_uint_8 red0;
    br_uint_8 grn0;
    br_uint_8 blu0;

    br_uint_8 alpha1;
    br_uint_8 red1;
    br_uint_8 grn1;
    br_uint_8 blu1;

    br_uint_8 flags; /* Shared edge flags. */

    br_fvector3 n; /* Plane equation of quad. */
    br_scalar   d;
} br_strip_face_data;

typedef struct br_tri_strip { /* Tri-strip primitive. */
    br_uint_16   nvertices;   /* Number of vertices. */
    br_uint_16  *vertices;    /* Array of indexes to vertices in strip. */
    br_material *material;    /* Strip material (or NULL). */

    /*
     * Array of data for each face in the strip.
     */

    br_strip_face_data *face_data;
} br_tri_strip;

typedef br_tri_strip br_tri_fan; /* Tri-fan primitive. */

typedef br_tri_strip br_quad_strip; /* Quad-strip primitive. */

/*
 * Callback function type for custom models
 */
struct br_actor;
struct br_model;
struct br_material;

/**
 * \brief An application defined call-back function that is called when a model (whose custom member
 *        defined as the address of this function) is about to be processed by the rendering engine.
 *
 * If this function does nothing, the model will not be rendered. The pass through equivalent would
 * be for this function to call Br[Zb|Zs]ModelRender().
 *
 * \param actor       Pointer to model actor referencing the model referring to this call-back.
 * \param model       Pointer to model referring to this call-back.
 * \param material    Pointer to actor's material if defined, or default material otherwise.
 * \param render_data A pointer to the order table the primitives for this model would be inserted
 *                    into, if the Z-Sort renderer is used. The value is NULL if no data is
 *                    appropriate for the renderer, e.g. when using the Z-Buffer renderer.
 * \param style       Actor's rendering style, or default. BRender will not supply BR_RSTYLE_DEFAULT
 *                    or BR_RSTYLE_NONE.
 * \param on_screen   On-screen flag (see BrOnScreenCheck()). The call-back will never be called by
 *                    BRender with the flag value OSC_REJECT.
 *
 * \pre BRender has completed initialisation. Rendering is in progress. The model's bounds intersect
 *      or are within the viewing volume.
 *
 * \post Behaviour is up to the application. Br[Zb|Zs]ModelRender() or any of the operations
 *       described for br_model_custom_cbfn can be used.
 *
 * \remark Any other BRender functions may be called from within this call-back with the following
 *         restrictions:
 * \li Don't call any rendering functions, apart from Br[Zb|Zs]ModelRender().
 * \li Don't modify any light, clip-plane or camera actors.
 * \li Don't access any output buffers until after rendering has completed.
 * \li Don't change the environment actor.
 * \li For best performance, avoid adding, updating or removing registry items – try to do these
 *     things before rendering.
 * \li Do not modify the actor hierarchy
 *
 * \sa br_renderbounds_cbfn, br_primitive_cbfn, br_pick2d_cbfn, br_pick3d_cbfn.
 *
 * \par Example
 * Possible uses include:
 * \li Selecting models with different levels of detail according to viewer distance
 * \li Morphing models (BrModelUpdate() 241 required)
 * \li Collision detection (not necessarily indicating the best method)
 * \li Labelling
 * \li Rendering liquids, gases, particulate, flames, smoke, etc.
 */
typedef void BR_CALLBACK br_model_custom_cbfn(struct br_actor *actor, struct br_model *model, struct br_material *material,
                                              void *render_data, br_uint_8 style, int on_screen);

/*
 * NB: To get at the model & screen matrices during a model
 *     callback, use BrModelToViewQuery() and BrModelToScreenQuery()
 */

/**
 * \brief BRender's model data structure, describing a mesh of triangles.
 */
typedef struct br_model {
    br_uintptr_t _reserved;

    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * Can be used as a handle to retrieve a pointer to the model. Not intended for intensive use.
     * Typically used to collect pointers to models loaded using BrModelLoad() and added to the registry
     * using BrModelAdd(). Also ideal for diagnostic purposes. A non-unique string can be supplied, but
     * which of a set of models having the same string will be matched by search functions (See
     * BrModelFind()), is undefined. Also in consideration of searching, it is not recommended that
     * non-alphabetic characters are used, especially Slash ('/'), Asterisk ('*'), and Query ('?'),
     * which are used for pattern matching. This member can be modified by the programmer at any time.
     * If identifier is set by BrModelLoad() or BrModelLoadMany() it will have been constructed using
     * BrResStrDup().
     */
    char *identifier;

    /**
     * \brief A list of vertex structures describing the model's geometry (also containing texture
     *        co-ordinates and pre-lighting).
     *
     * The vertices can be allocated at the same time as the model, otherwise vertices should point to a
     * list with a sufficient lifetime (and BR_MODF_KEEP_ORIGINAL must be set).
     */
    br_vertex *vertices;
    /**
     * \brief A list of face structures describing the model's surface in terms of its vertices (also
     *        containing smoothing information, edge flags, and materials).
     *
     * The faces can be allocated at the same time as the model, otherwise faces should point to a list
     * with a sufficient lifetime (and BR_MODF_KEEP_ORIGINAL must be set).
     */
    br_face   *faces;

    /**
     * \brief Number of vertices supplied in the list of vertices.
     */
    br_uint_16 nvertices;
    /**
     * \brief Number of faces supplied in the list of faces.
     */
    br_uint_16 nfaces;

    /**
     * \brief Offset from model geometry origin to model origin.
     *
     * Effectively an offset which is subtracted from each model vertex. Alternatively, it may be
     * thought of as a vector in the model's co-ordinate space defining the point at which the model
     * attaches to its parent (assuming an identity transform). This member is provided to facilitate
     * centring geometry (thus not needing to modify vertex data), and thus enables such things as
     * tighter bounding radii. It is not really intended to supplement the model actor transform, i.e.
     * as another way of translating models.
     */
    br_vector3 pivot;

    /**
     * \brief This member determines how the model's geometry is computed.
     *
     * Various flags can be combined using the 'Or' operation. They're described in the following table.
     *
     * \li BR_MODF_KEEP_ORIGINAL — Retain original vertices and faces during model update – otherwise
     *     these are freed and replaced by an optimised and equivalent set (very likely reordered)
     * \li BR_MODF_GENERATE_TAGS — Improve update speed at the expense of face and vertex tag tables.
     *     Only use in conjunction with BR_MODF_KEEP_ORIGINAL
     * \li BR_MODF_QUICK_UPDATE — Improve update speed at the expense of having models that may take
     *     longer to render
     * \li BR_MODF_DONT_WELD — Don't eliminate redundant vertices (having identical co-ordinates)
     * \li BR_MODF_CUSTOM — Invoke a custom call-back for this model
     */
    br_uint_16 flags;

    /**
     * \brief If the BR_MODF_CUSTOM flag is specified, instead of being rendered, the function pointed
     *        to by custom is called.
     *
     * This may of course then call BrZbModelRender(), say. See br_model_custom_cbfn.
     */
    br_model_custom_cbfn *custom;

    /**
     * \brief A member whose usage is entirely application dependent.
     *
     * It can be useful when writing custom model rendering functions (see br_model_custom_cbfn).
     */
    void *user;

    /*
     * Crease angle (used if MODF_CREASE is set)
     */
    br_angle crease_angle;

    /**
     * \brief The maximum vertex length, thus the radius defining the smallest origin centred sphere
     *        enclosing the model.
     *
     * This is computed upon BrModelAdd() and when the BR_MODU_RADIUS flag is specified to
     * BrModelUpdate().
     */
    br_scalar radius;

    /**
     * \brief The minimum and maximum x,y and z ordinates of the vertices, thus the minimal, axis
     *        aligned (orthogonal faced) cuboid enclosing the model.
     *
     * This is computed upon BrModelAdd() and when the BR_MODU_BOUNDING_BOX flag is specified to
     * BrModelUpdate().
     */
    br_bounds bounds;

    /*
     * Private fields
     */
    void *prepared;
    void *stored;

    /*
     * New primitive extensions
     */

    br_uint_16         nprimitive_lists;
    br_primitive_list *primitive_list;

} br_model;

/*
 * Bits for br_model->flags
 */
enum {
    BR_MODF_DONT_WELD     = 0x0001, /* Vertices with same x,y,z cannot be merged	*/
    BR_MODF_KEEP_ORIGINAL = 0x0002, /* Obselete */
    BR_MODF_GENERATE_TAGS = 0x0004, /* Obselete */
    BR_MODF_QUICK_UPDATE  = 0x0010, /* ModelUpdate is fast - but may produce slower models */

    BR_MODF_CUSTOM      = 0x0020, /* Invoke custom callback for this model */
    BR_MODF_PREPREPARED = 0x0040, /* Model structure is pre-prepared - update performs no work */

    BR_MODF_UPDATEABLE = 0x0080, /* ModelUpdate can be used */

    BR_MODF_CREASE         = 0x0100, /* Create creases in smoothing along edges if face<->face angle is g.t model->crease */
    BR_MODF_CUSTOM_NORMALS = 0x0200, /* Uses vertex normals from br_vertex structure */
    BR_MODF_CUSTOM_BOUNDS  = 0x0400, /* Bounding box is already set up				*/
    BR_MODF_FACES_ONLY     = 0x0800, /* Model will only be used to render faces (not edges or points) */

    BR_MODF_USED_PREPARED_USER = 0x1000, /* User fields in prepared data used */

    BR_MODF_CUSTOM_EQUATIONS = 0x2000, /* Uses face equations from br_face structure */

    _BR_MODF_RESERVED = 0x8000
};

/*
 * Flags to BrModelUpdate()
 */
enum {
    BR_MODU_VERTEX_POSITIONS    = 0x0001,
    BR_MODU_VERTEX_COLOURS      = 0x0002,
    BR_MODU_VERTEX_MAPPING      = 0x0004,
    BR_MODU_VERTEX_NORMALS      = 0x0008,
    BR_MODU_PRIMITIVE_MATERIALS = 0x0010,
    BR_MODU_PRIMITIVE_COLOURS   = 0x0020,
    BR_MODU_VERTICES            = 0x0040,
    BR_MODU_FACES               = 0x0080,
    BR_MODU_PIVOT               = 0x0100,
    BR_MODU_PREPARED            = 0x0200,
    BR_MODU_PRIMITIVE_EQUATIONS = 0x0400,
    BR_MODU_ALL                 = 0x7FFF,

    _BR_MODU_RESERVED = 0x8000
};

/*
 * Backwards compatibility
 */

#define BR_MODU_FACE_MATERIALS BR_MODU_PRIMITIVE_MATERIALS
#define BR_MODU_FACE_COLOURS   BR_MODU_PRIMITIVE_COLOURS
#define BR_MODU_FACE_EQUATIONS BR_MODU_PRIMITIVE_EQUATIONS
#define BR_MODU_NORMALS        BR_MODU_VERTEX_POSITIONS
#define BR_MODU_EDGES          BR_MODU_FACES
#define BR_MODU_RADIUS         BR_MODU_VERTEX_POSITIONS
#define BR_MODU_GROUPS         BR_MODU_FACES
#define BR_MODU_BOUNDING_BOX   BR_MODU_VERTEX_POSITIONS
#define BR_MODU_MATERIALS      BR_MODU_FACE_MATERIALS

/*
 * Values for BrModelApplyMap()
 */
enum br_apply_map_types {
    BR_APPLYMAP_PLANE,
    BR_APPLYMAP_SPHERE,
    BR_APPLYMAP_CYLINDER,
    BR_APPLYMAP_DISC,
    BR_APPLYMAP_NONE
};

/*
 * Axis values for BrModelFitMap()
 */
enum br_fitmap_axis {
    BR_FITMAP_PLUS_X,
    BR_FITMAP_PLUS_Y,
    BR_FITMAP_PLUS_Z,
    BR_FITMAP_MINUS_X,
    BR_FITMAP_MINUS_Y,
    BR_FITMAP_MINUS_Z
};

#endif

/*
 * Local Variables:
 * tab-width: 4
 * End:
 */
