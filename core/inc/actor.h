/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: actor.h 1.2 1998/06/09 17:29:25 johng Exp $
 * $Locker: $
 *
 * Definitons for an Actor
    Last change:  TN   15 Apr 97    4:14 pm
 */
#ifndef _ACTOR_H_
#define _ACTOR_H_

/**
 ** Definition of base actor structure
 **/

/*
 * Basic types of actor
 */
enum {
    BR_ACTOR_NONE,
    BR_ACTOR_MODEL,
    BR_ACTOR_LIGHT,
    BR_ACTOR_CAMERA,
    _BR_ACTOR_RESERVED,
    BR_ACTOR_BOUNDS,
    BR_ACTOR_BOUNDS_CORRECT,
    BR_ACTOR_CLIP_PLANE,
    BR_ACTOR_HORIZON_PLANE,
    BR_ACTOR_MAX
};

/*
 * Render styles - an actor inherits it's style from the most _distant_
 * ancestor included in this traversal that does not have default set
 * (unlike model & material which are inherited from the nearest ancestor)
 */
enum {
    BR_RSTYLE_DEFAULT,
    BR_RSTYLE_NONE,
    BR_RSTYLE_POINTS,
    BR_RSTYLE_EDGES,
    BR_RSTYLE_FACES,
    BR_RSTYLE_BOUNDING_POINTS,
    BR_RSTYLE_BOUNDING_EDGES,
    BR_RSTYLE_BOUNDING_FACES,
    BR_RSTYLE_ANTIALIASED_LINES,
    BR_RSTYLE_ANTIALIASED_FACES,
    BR_RSTYLE_MAX
};

/**
 * \brief The basic unit of scene construction.
 *
 * The br_actor object is designed to facilitate hierarchical relationships between elements of a
 * scene, particularly in terms of position and orientation. See The Actor, page 12, in the
 * structured description for further details.
 */
typedef struct br_actor {
    /**
     * \brief A pointer to the next sibling actor in the linked list of sibling actors (parent's
     *        children).
     *
     * If the actor has no siblings or is the last sibling actor, next is NULL.
     */
    struct br_actor  *next;
    /**
     * \brief A pointer to the previous sibling actor's next member in the linked list of sibling actors
     *        (parent's children).
     *
     * If the first sibling, prev will point to its parent actor's children member. If the root actor,
     * prev is NULL. Given that the next member is the first member of br_actor, prev can be cast to a
     * pointer to the previous actor (where prev points to a next member).
     */
    struct br_actor **prev;

    /**
     * \brief A pointer to the first actor in a linked list of child actors.
     *
     * If the actor has no children, children is NULL.
     */
    struct br_actor *children;

    /**
     * \brief A pointer to the actor's parent actor in an actor hierarchy.
     *
     * If the actor has no parent, i.e. is the root actor of its hierarchy, parent is NULL.
     */
    struct br_actor *parent;

    /**
     * \brief The depth of the actor from the root of the hierarchy. depth is zero for the root actor
     *        and increases by one each generation (indirection through children).
     */
    br_uint_16 depth;

    /**
     * \brief This member defines the type of function required of this actor.
     *
     * The actor's type should contain a value defined by one of the symbols described in the following
     * table.
     *
     * \li BR_ACTOR_NONE — Reference actor - no special behaviour. May be used to define frames of
     *     reference, actor groups, intermediate transforms, inheritable properties, and temporarily
     *     disable an alternative behaviour.
     * \li BR_ACTOR_MODEL — Renderable model - render the specified model in the specified style,
     *     possibly using the default material
     * \li BR_ACTOR_LIGHT — Light source - affects rendering of models using materials that are lit.
     * \li BR_ACTOR_CAMERA — Camera - used to define view point and perspective for rendering output
     *     image.
     * \li BR_ACTOR_BOUNDS — A cuboid bounding box, application defined within the actor's co-ordinate
     *     space. Box completely off screen: rendering of all descendants is disabled. Box partially
     *     on/off screen: descendants subject to 'on screen' check. Box completely on screen:
     *     descendants subject to 'on screen' check.
     * \li BR_ACTOR_BOUNDS_CORRECT — A cuboid bounding box, application defined within the actor's
     *     co-ordinate space. Box completely off screen: rendering of all descendants is disabled. Box
     *     partially on/off screen: descendants subject to 'on screen' check. Box completely on screen:
     *     descendants not subject to 'on screen' check. (Footnote a: Rendering of a model that is off
     *     screen is undefined, and should be considered a fatal error.)
     * \li BR_ACTOR_CLIP_PLANE — The four vector pointed to by type_data defines a three vector unit
     *     normal to a plane whose distance from the origin is represented by the fourth element.
     *     Descendants are clipped against the plane, with the side defined by the normal being 'in
     *     scene'.
     *
     * Although set at initialisation, this member can be changed outside rendering at any time (as long
     * as the other members are set appropriately, and, if a Light or Clip Plane, it has first been
     * disabled (See BrLightDisable() and BrClipPlaneDisable())). During rendering (within call-back
     * functions) change is not recommended - Actors should not be changed to or from Lights or Clip
     * Planes, and the particular Camera supplied for the rendering should not have its type changed.
     */
    br_uint_8 type;

    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * Can be used as a handle to retrieve a pointer to the actor, given only an ancestral actor. Not
     * intended for intensive use. Typically used to collect pointers to actors loaded using
     * BrActorLoad() and BrActorLoadMany(). Also ideal for diagnostic purposes. A non-unique string can
     * be supplied, but which of a set of actors having the same string will be matched by search
     * functions (See BrActorSearch()), is undefined. Also in consideration of searching, it is not
     * recommended that non-alphabetic characters are used, especially Slash ('/'), Asterisk ('*'), and
     * Query ('?'), which are used for pattern matching. This member can be modified by the programmer
     * at any time. If identifier is set by BrActorLoad() or BrActorLoadMany() it will have been
     * constructed using BrResStrDup().
     */
    char *identifier;

    /**
     * \brief The model defines the geometry and any specific face characteristics to be rendered in
     *        this actor's co-ordinate space.
     *
     * This member points to model information, which is used by this actor if it is a model actor, and
     * descendent model actors that inherit it. If NULL, model information is obtained (when required by
     * a model actor) from the previous ancestor (parent) that supplied it. If no ancestor supplies it,
     * a default br_model data structure is used which defines a cube (this is for diagnostic purposes
     * only). Note that an actor does not have to be a model actor in order for it to define a model to
     * be inherited by a descendent model actor.
     */
    br_model *model;

    /**
     * \brief The material is used for a model's faces that don't specify a material.
     *
     * This member points to material information, which is used by this actor if it is a model actor,
     * and descendent model actors that inherit it. If NULL, material information is obtained (when
     * required by a model actor) from the previous ancestor (parent) that supplied it. If no ancestor
     * supplies it, a default br_material data structure is used which defines a flat-shaded grey (this
     * is for diagnostic purposes only). Out of interest, it is currently defined as follows‡:
     *
     * \code{.c}
     *   { "default",
     *     BR_COLOUR_RGB(255,255,255),
     *     255,
     *     BR_UFRACTION(0.10),
     *     BR_UFRACTION(0.70),
     *     BR_UFRACTION(0.0),
     *     BR_SCALAR(20),
     *     BR_MATF_LIGHT,
     *     {{ BR_VECTOR2(1,0),
     *        BR_VECTOR2(0,1),
     *        BR_VECTOR2(0,0),
     *     }},
     *     0, 63,
     *   };
     * \endcode
     *
     * (Footnote on page 80: ‡ Yet subject to change) Note that an actor does not have to be a model
     * actor in order for it to define a material to be inherited by a descendent model actor.
     */
    br_material *material;

    /**
     * \brief This member determines the style of rendering, which is used by this actor if it is a
     *        model actor.
     *
     * If the NULL style (BR_RSTYLE_DEFAULT) is specified, the rendering style is obtained (when
     * required by a model actor) from the first ancestor (from the root) that defines a non-NULL style.
     * If no ancestor defines one, the model is rendered as though BR_RSTYLE_FACES had been specified.
     * This is subtly different from normal inheritance, in that non-default styles set by intervening
     * actors have no effect. It is designed this way to facilitate the use of an attribute primarily
     * intended for highlighting (obviously not one concerned with realism). The render_style member
     * should be set to a value defined by one of the following (enum) symbols:
     *
     * \li BR_RSTYLE_DEFAULT — Uses first non-default rendering style defined in this branch of the
     *     hierarchy (renders faces if no non-default style defined).
     * \li BR_RSTYLE_NONE — Does not render this actor or any of its descendants.
     * \li BR_RSTYLE_POINTS — Renders only the points at each vertex of a face using the model's
     *     material.
     * \li BR_RSTYLE_EDGES — Renders only the points along each edge of a face using the model's
     *     material.(see br_face flags)
     * \li BR_RSTYLE_FACES — Renders all the points across rendered faces using the model's material.
     * \li BR_RSTYLE_BOUNDING_POINTS — Renders only the points at each vertex of the model's bounding
     *     box using the actor's material
     * \li BR_RSTYLE_BOUNDING_EDGES — Renders only the points at along edge of the model's bounding box
     *     using the actor's material
     * \li BR_RSTYLE_BOUNDING_FACES — Renders the faces of the model's bounding box using the actor's
     *     material
     */
    br_uint_8 render_style;

    /*
     * Reference to renderer specific data associated with this actor - NULL will
     * inherit from parent (root inherits default data)
     */
    void *render_data;

    /**
     * \brief This defines the transform to apply to co-ordinates of this actor (e.g. of models), to
     *        convert them to co-ordinates in its parent's co-ordinate system.
     *
     * This is the primary way of defining relative model positions and orientations. Any valid
     * transform may be used. This member is read when rendering and computing actor/actor transforms.
     * It can be modified at any time, although it should be appreciated that this may have
     * unpredictable effects if done during render call-back functions. See BrActorToActorMatrix34() for
     * information on obtaining transforms between actors other than parent and child.
     *
     * \par Example
     * Remember that each actor can be considered to have its own (right handed) co-ordinate system.
     * Consider a point (1,2,3), of a model vertex say. Let's suppose the actor has a transform that
     * just translates by a vector of (1,0,0). In applying this transform we obtain the point (2,2,3),
     * and these are now co-ordinates in the parent's co-ordinate system. Now let's suppose the actor
     * has a transform that also rotates by 90 degrees about the x-axis (+y topples toward us, +z) and
     * is then followed by the translation of (1,0,0). If we applied this transform we'd obtain the
     * point (2,-3,2). If you wished to perform this using BRender functions, you might first use
     * BrTransformToMatrix34() and then BrMatrix34ApplyP().
     */
    br_transform t;

    /**
     * \brief This member is used to refer to additional, type specific data.
     *
     * It should point to data according to the contents of type as shown in the following table:
     *
     * \li BR_ACTOR_NONE — NULL (or application defined)
     * \li BR_ACTOR_MODEL — NULL (or application defined)
     * \li BR_ACTOR_LIGHT — Pointer to an instance of br_light
     * \li BR_ACTOR_CAMERA — Pointer to an instance of br_camera
     * \li BR_ACTOR_BOUNDS — Pointer to an instance of br_bounds
     * \li BR_ACTOR_BOUNDS_CORRECT — Pointer to an instance of br_bounds
     * \li BR_ACTOR_CLIP_PLANE — Pointer to an instance of br_vector4
     *
     * The type_data member should not be NULL except for BR_ACTOR_NONE or BR_ACTOR_MODEL. This member
     * can be changed at any time, though not recommended during rendering.
     *
     * The manual introduces this with a note on the technique: Experienced C programmers will be
     * familiar with the technique of supplying pointers to structures that are actually embodied within
     * others, for the purpose of accessing attached application specific data*. This technique is still
     * a workable way of extending the amount of data within BRender data structures, particularly
     * here†. For example, it may be desired to introduce rotating lights. The corresponding details
     * could be appended to the br_light structure as demonstrated in this example:
     *
     * \code{.c}
     *   typedef struct
     *   { br_light light;
     *     unsigned type;
     *     br_scalar speed;
     *     br_vector3 axis;
     *   } my_light;
     * \endcode
     *
     * (Footnotes on page 80: * For C++ programmers, this is embraced by the language - any derived
     * class may be supplied where a base class is expected. † Though BRender will of course, not be
     * aware of any attached data (so will not save it in BrActorSave(), say).)
     */
    void *type_data;

    /**
     * \brief This member may be used by the application for its own purposes.
     *
     * It is initialised to NULL upon allocation, and not accessed by BRender thereafter.
     */
    void *user;

} br_actor;

#endif
