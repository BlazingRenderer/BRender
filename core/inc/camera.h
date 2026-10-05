/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: camera.h 1.1 1997/12/10 16:41:16 jon Exp $
 * $Locker: $
 *
 * Definitons for a camera
 */
#ifndef _CAMERA_H_
#define _CAMERA_H_

enum {
    BR_CAMERA_PARALLEL,
    BR_CAMERA_PERSPECTIVE_FOV,
    BR_CAMERA_PERSPECTIVE_WHD,
    BR_CAMERA_PARALLEL_OLD,
    BR_CAMERA_PERSPECTIVE_FOV_OLD,
};

/*
 * Backwards compatibility
 */
#define BR_CAMERA_PERSPECTIVE     BR_CAMERA_PERSPECTIVE_FOV
#define BR_CAMERA_PERSPECTIVE_OLD BR_CAMERA_PERSPECTIVE_FOV_OLD

/**
 * \brief BRender's camera data structure.
 *
 * See Camera Actors.
 */
typedef struct br_camera {
    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * If identifier is set by BrActorLoad() or BrActorLoadMany() it will have been constructed using
     * BrResStrDup().
     */
    const char *identifier;

    /**
     * \brief Type of camera.
     *
     * Which can be one of the following:
     *
     * \li BR_CAMERA_PARALLEL — A parallel camera. Object size is independent of its distance from the
     *     camera.
     * \li BR_CAMERA_PERSPECTIVE — A standard perspective camera.
     */
    br_uint_8 type;

    /**
     * \brief Field of view, i.e. the angle subtended at the camera between the top and bottom of the
     *        view volume (pyramid).
     *
     * Applies only to BR_CAMERA_PERSPECTIVE_FOV cameras. The value should be greater than zero and
     * less than 180°.
     * Note than this is the full angle, i.e. not the half angle, between the view z axis and top (or
     * bottom) of view volume.
     */
    br_angle field_of_view;

    /**
     * \brief Distance of front of view volume from camera along negative z axis, i.e. -hither_z in view
     *        co-ordinates.
     *
     * The value should be greater than zero.
     */
    br_scalar hither_z;
    /**
     * \brief Distance of back of view volume from camera along negative z axis, i.e. -yon_z in view
     *        co-ordinates.
     *
     * The value should be greater than hither_z.
     */
    br_scalar yon_z;

    /**
     * \brief Scaling factor for width of viewing volume.
     *
     * For perspective cameras, ±1 in y view ordinates is mapped to the height of the output pixel map,
     * and ±aspect in x view ordinates is mapped to the width of the output pixel map. For parallel
     * cameras, the height of the view volume is mapped to the height of the output pixel map, whereas
     * \f$width \times aspect\f$ is mapped to the width of the output pixel map. Correct Aspect: If you want to
     * maintain correctly proportioned images on a screen, then the ratio between the sides of the
     * physical image should be the same as that of the view volume. Therefore, for perspective cameras,
     * aspect should be calculated as the ratio between the physical dimensions of the output pixel map,
     * i.e. its physical width divided by its physical height (both of which may be computed directly in
     * terms of pixel map resolution if pixels are square). For parallel cameras, aspect would have to
     * be defined such that \f$width \times aspect \div height\f$ is the same as the ratio between the physical dimensions
     * of the output pixel map – this is effectively modifying the width of the view volume to neatly
     * fit the pixel map. In this case, aspect is relatively redundant as it might as well be left at
     * unity with just width needing to be changed.
     */
    br_scalar aspect;

    /**
     * \brief Width of view volume (rectangular prism) in world co-ordinates (before aspect is applied)
     *        – the actual width is \f$width \times aspect\f$.
     *
     * Applies only to BR_CAMERA_PARALLEL and BR_CAMERA_PERSPECTIVE_WHD cameras.
     */

    br_scalar width;
    /**
     * \brief Height of view volume (rectangular prism) in world co-ordinates.
     *
     * Applies only to BR_CAMERA_PARALLEL and BR_CAMERA_PERSPECTIVE_WHD cameras.
     */
    br_scalar height;

    /*
     * Distance of projection plane from center of projection
     * (BR_CAMERA_PERSPECTIVE_WHD only)
     */
    br_scalar distance;

    /**
     * \brief This member may be used by the application for its own purposes.
     *
     * It is initialised to NULL upon allocation (if allocated by BRender), and not accessed by BRender
     * thereafter.
     */
    void *user;

} br_camera;

#endif
