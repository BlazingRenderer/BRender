/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: light.h 1.3 1998/06/17 16:11:46 jon Exp $
 * $Locker: $
 *
 * Definitons for a light
 */
#ifndef _LIGHT_H_
#define _LIGHT_H_

enum {
    /*
     * Type of light
     */
    BR_LIGHT_TYPE    = 0x0003,
    BR_LIGHT_POINT   = 0x0000,
    BR_LIGHT_DIRECT  = 0x0001,
    BR_LIGHT_SPOT    = 0x0002,
    BR_LIGHT_AMBIENT = 0x0003,

    /*
     * Flag indicating that calculations are done in view space
     */
    BR_LIGHT_VIEW = 0x0004,

    /*
     * Flag indicating that linear falloff should be used
     */
    BR_LIGHT_LINEAR_FALLOFF = 0x0008,
};

/*
 * Definition of cutoff volumes - vertex is lit if it falls within any of
 * a number of convex volumes, and partially lit if it falls within the
 * falloff distance from any
 */
typedef struct br_light_volume {

    br_scalar         falloff_distance;
    br_convex_region *regions;
    br_uint_32        nregions;

} br_light_volume;

/**
 * \brief This structure is used to specify the properties of light actors.
 *
 * See Light Actors.
 */
typedef struct br_light {
    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * If `identifier` is set by BrActorLoad() or BrActorLoadMany() it will have been constructed using
     * BrResStrDup().
     */
    char *identifier;

    /**
     * \brief This member defines the type of the light.
     *
     * Note that different light types can affect rendering performance in different ways. Pre-lighting
     * textures is the fastest means of lighting objects in a scene, ambient lighting comes next, then
     * the three active lighting methods: direct, point and spot lights (in order of increasing
     * processing requirements). The more lights in a scene (the maximum number of enabled light actors
     * in a scene is defined by the symbol BR_MAX_LIGHTS), the more computation required. The three
     * active light types are described in the following table:
     *
     * \li BR_LIGHT_DIRECT — A directed light source. The light is infinitely distant and shines along
     *     the negative z axis of the light actor.
     * \li BR_LIGHT_POINT — A point light source, radiating in all directions.
     * \li BR_LIGHT_SPOT — A spot light, with both spatial location and direction. A spot light has a
     *     cone of illumination and shines along the negative z axis of the light actor.
     *
     * The `type` member also encodes a flag value `BR_LIGHT_VIEW` which can be combined with any of the
     * three light types using the 'inclusive-or' operation. When the flag is present, lighting
     * calculations are performed in view space rather than model space. This is slower, but prevents
     * odd anomalies if non-uniform scalings are used on models. The bit mask value `BR_LIGHT_TYPE` can
     * be used to extract the light type using the 'and' operation.
     */
    br_uint_8 type;

    /**
     * \brief If the rendering engine supports it, a light may be of a specific colour.
     *
     * Typically supported when rendering in 'true colour'.
     */
    br_colour colour;

    /**
     * \brief Constant attenuation component.
     *
     * Light intensity is inversely proportional to this value.
     */
    br_scalar attenuation_c;
    /**
     * \brief Linear attenuation factor.
     *
     * Light intensity is inversely proportional to distance factored by this value. It only applies to
     * point and spot lights.
     */
    br_scalar attenuation_l;
    /**
     * \brief Quadratic attenuation factor.
     *
     * Light intensity is inversely proportional to distance factored by the square of this value. It
     * only applies to point and spot lights.
     */
    br_scalar attenuation_q;

    /**
     * \brief The outer cone of a spot light is defined by the angle between the cone's axis and its
     *        circumference.
     *
     * This represents the region within which surfaces will be partially lit (subject to attenuation).
     * The light level falls off linearly from the fully lit level at the inner cone to zero at the
     * limit of the outer cone. The angles are clamped as cone_inner ≤ cone_outer ≤ 0.5 (180°, π rad).
     */
    br_angle cone_outer;
    /**
     * \brief The inner cone of a spot light is defined by the angle between the cone's axis and its
     *        circumference.
     *
     * This represents the region within which surfaces will be fully lit (subject to attenuation).
     */
    br_angle cone_inner;

    /*
     * Sphere radii for linear falloff and cutoff of normally attenuated lights
     */
    br_scalar radius_outer;
    br_scalar radius_inner;

    /*
     * Cutoff volumes
     */
    br_light_volume volume;
    /**
     * \brief This member may be used by the application for its own purposes.
     *
     * It is initialised to NULL upon allocation (if allocated by BRender), and not accessed by BRender
     * thereafter.
     */
    void           *user;

} br_light;

#endif
