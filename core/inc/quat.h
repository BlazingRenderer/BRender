/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: quat.h 1.1 1997/12/10 16:41:19 jon Exp $
 * $Locker: $
 *
 * Type descibing a unit quaternion
 */
#ifndef _QUAT_H_
#define _QUAT_H_

/**
 * \brief The quaternion is an extension of the complex number, with the addition of two further
 *        imaginary components.
 *
 * BRender restricts itself to using unit quaternions, which have the useful property of being able
 * to easily represent a 3D transform consisting of a rotation about an arbitrary vector.
 *
 * The x, y and z components of the quaternion hold the elements of this vector scaled to a length
 * equal to the sine of half the angle of rotation. The w component holds the cosine of half the
 * angle. The magnitude of a unit quaternion is of course 1 \f$(w^2+x^2+y^2+z^2=1)\f$.
 *
 * The quaternion is written thus: \f[w + xi + yj + zk\f]
 *
 * \note Do not confuse the quaternion with 3D vectors or homogenous co-ordinates, i.e. br_quat and
 * br_vector4 have no relationship.
 */
typedef struct br_quat {
    /**
     * \brief First imaginary component of quaternion.
     *
     * Represents the x axis component of the vector about which the rotation occurs.
     */
    br_scalar x;
    /**
     * \brief Second imaginary component of quaternion.
     *
     * Represents the y axis component of the vector about which the rotation occurs.
     */
    br_scalar y;
    /**
     * \brief Third imaginary component of quaternion.
     *
     * Represents the z axis component of the vector about which the rotation occurs.
     */
    br_scalar z;
    /**
     * \brief Real component of quaternion.
     *
     * Represents the cosine of half the rotation about the (i,j,k) vector component of the quaternion.
     */
    br_scalar w;
} br_quat;

// clang-format off
#define BR_QUAT(x,y,z,w) {BR_SCALAR(x),BR_SCALAR(y),BR_SCALAR(z),BR_SCALAR(w)}
// clang-format on

#endif
