/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: transfrm.h 1.1 1997/12/10 16:41:20 jon Exp $
 * $Locker: $
 *
 * Structure describing an affine transform from one coordinate space
 * to another
 */
#ifndef _TRANSFRM_H_
#define _TRANSFRM_H_

/*
 * Type of actor position
 */
enum {
    BR_TRANSFORM_MATRIX34,
    BR_TRANSFORM_MATRIX34_LP,
    BR_TRANSFORM_QUAT,
    BR_TRANSFORM_EULER,
    BR_TRANSFORM_LOOK_UP,
    BR_TRANSFORM_TRANSLATION,
    BR_TRANSFORM_IDENTITY,
    BR_TRANSFORM_MAX
};

/**
 * \brief This is BRender's generic transformation type, primarily used to specify a transformation
 *        from one actor's space to another's.
 *
 * In an actor it represents the transform to be applied to co-ordinates (such as of a model) in its
 * space to bring them into the co-ordinate space of its parent.
 */
typedef struct br_transform {

    /**
     * \brief This member defines which other members of the transform structure have meaning.
     *
     * It should never be modified directly except for initialisation purposes. Refer to
     * BrTransformToTransform() for details of how to convert from one transform to another. This member
     * may have any one of the following values:
     *
     * \li BR_TRANSFORM_IDENTITY — The transform is the identity.
     * \li BR_TRANSFORM_TRANSLATION — The transform is a translation only (held in t.translate.t).
     * \li BR_TRANSFORM_EULER — The transform is represented by a Euler angle set (t.euler.e) and a
     *     translation (t.euler.t).
     * \li BR_TRANSFORM_LOOK_UP — The transform is represented by a look-at vector (t.look_up.look), an
     *     up vector (t.look_up.up) and a translation (t.look_up.t).
     * \li BR_TRANSFORM_QUAT — The transform is represented by a quaternion (t.quat.q) and a translation
     *     (t.quat.t).
     * \li BR_TRANSFORM_MATRIX34 — The transform is represented by a 3x4 affine matrix (t.mat), which is
     *     the most general representation.
     * \li BR_TRANSFORM_MATRIX34_LP — The transform is represented by a 3x4 length preserving matrix
     *     (t.mat).
     */
    br_uint_16 type;

    /*
     * Union of the various means of describing a transform -
     * these are explicity arrranged so that any exlicit transform
     * will always be available as br_transform.t.translate
     */
    union {
        /**
         * \brief This member contains the 3D affine matrix representing the entire transform.
         */
        br_matrix34 mat;

        /*
         * Euler angles and translation
         */
        struct {
            /**
             * \brief This member contains the vector representing the rotation components of the transform.
             */
            br_euler   e;
            br_scalar  _pad[7];
            /**
             * \brief This member contains the vector representing the translation component of the transform.
             */
            br_vector3 t;
        } euler;

        /*
         * Unit quaternion and translation
         */
        struct {
            /**
             * \brief This member contains the vector representing the rotation component of the transform.
             */
            br_quat    q;
            br_scalar  _pad[5];
            /**
             * \brief This member contains the vector representing the translation component of the transform.
             */
            br_vector3 t;
        } quat;

        /*
         * Lookat vector, up vector and translation
         */
        struct {
            /**
             * \brief This member contains the vector that defines the rotation of the negative z axis.
             */
            br_vector3 look;
            /**
             * \brief This member contains the vector that defines the rotation about the look vector.
             */
            br_vector3 up;
            br_scalar  _pad[3];
            /**
             * \brief This member contains the vector representing the translation component of the transform.
             */
            br_vector3 t;
        } look_up;

        /*
         * Just a translation
         */
        struct {
            br_scalar  _pad[9];
            /**
             * \brief This member only has meaning for this transform type.
             *
             * It contains the vector representing the translation. (Footnote: However, upon inspection of
             * br_transform it can be seen that it is effective for all transforms apart from the identity. Use
             * of this feature is for internal use only.)
             */
            br_vector3 t;
        } translate;
    } t;
} br_transform;

#endif
