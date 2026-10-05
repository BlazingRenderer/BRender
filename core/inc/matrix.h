/*
 * Copyright (c) 1992,1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: matrix.h 1.1 1997/12/10 16:41:18 jon Exp $
 * $Locker: $
 *
 * Structrures describing matrices
 */
#ifndef _MATRIX_H_
#define _MATRIX_H_

/**
 * \brief A two column, three row, scalar array, used as a 2D affine matrix, typically for texture
 *        map transformations (translation, scaling, shearing, rotation).
 *
 * Functions are provided to allow it be used as though it were an integral type. It has the
 * following form:
 *
 * \f[
 * \begin{pmatrix}
 * m_{00} & m_{01} \\
 * m_{10} & m_{11} \\
 * m_{20} & m_{21} \\
 * \end{pmatrix}
 * \f]
 *
 * Note that this is effectively used as a 3x3 matrix, but omitting the redundant, third column for
 * storage purposes. Thus:
 *
 * \f[
 * \begin{pmatrix}
 * m_{00} & m_{01} & 0 \\
 * m_{10} & m_{11} & 0 \\
 * m_{20} & m_{21} & 1 \\
 * \end{pmatrix}
 * \f]
 *
 * It is applied to homogenous 2D co-ordinates, which similarly omit the third element for sake of
 * economy.
 *
 * It can be noted that the bottom row has a translational effect. Also note, that the matrix
 * determinant represents the area change effected.
 */
typedef struct br_matrix23 {
    /**
     * \brief Each element of the matrix can be freely and individually accessed.
     *
     * This matrix can also be thought of as an array of three br_vector2 structures, e.g. br_vector2
     * m[3]. Thus m[row] can be cast as (br_vector2 *).
     */
    br_scalar m[3][2];
} br_matrix23;

typedef struct br_matrix23_f {
    br_float m[3][2];
} br_matrix23_f;

typedef struct br_matrix23_x {
    br_fixed_ls m[3][2];
} br_matrix23_x;

/**
 * \brief A three column, four row, scalar array, used as a 3D affine matrix for general purpose 3D
 *        transformations (translation, scaling, shearing, rotation).
 *
 * Functions are provided to allow it be used as though it were an integral type. It has the
 * following form:
 *
 * \f[
 * \begin{pmatrix}
 * m_{00} & m_{01} & m_{02} \\
 * m_{10} & m_{11} & m_{12} \\
 * m_{20} & m_{21} & m_{22} \\
 * m_{30} & m_{31} & m_{32} \\
 * \end{pmatrix}
 * \f]
 *
 * Note that this is effectively used as a 4x4 matrix, but omitting the redundant, fourth column for
 * storage purposes. Thus:
 *
 * \f[
 * \begin{pmatrix}
 * m_{00} & m_{01} & m_{02} & 0 \\
 * m_{10} & m_{11} & m_{12} & 0 \\
 * m_{20} & m_{21} & m_{22} & 0 \\
 * m_{30} & m_{31} & m_{32} & 1 \\
 * \end{pmatrix}
 * \f]
 *
 * It is applied to homogenous 3D co-ordinates, which similarly omit the fourth element for sake of
 * economy.
 *
 * It can be noted that the bottom row has a translational effect. Also note, that the matrix
 * determinant represents the volume change effected.
 */
typedef struct br_matrix34 {
    /**
     * \brief Each element of the matrix can be freely and individually accessed.
     *
     * This matrix can also be thought of as an array of four br_vector3 structures, e.g. br_vector3
     * m[4]. Thus m[row] can be cast as (br_vector3 *).
     */
    br_scalar m[4][3];
} br_matrix34;

typedef struct br_matrix34_f {
    br_float m[4][3];
} br_matrix34_f;

typedef struct br_matrix34_x {
    br_fixed_ls m[4][3];
} br_matrix34_x;

/**
 * \brief A four column, four row, scalar array, used as a 3D affine matrix for general purpose 3D
 *        transformations (translation, scaling, shearing, rotation).
 *
 * Functions are provided to allow it be used as though it were an integral type. It has the
 * following form:
 *
 * \f[
 * \begin{pmatrix}
 * m_{00} & m_{01} & m_{02} & m_{03} \\
 * m_{10} & m_{11} & m_{12} & m_{13} \\
 * m_{20} & m_{21} & m_{22} & m_{23} \\
 * m_{30} & m_{31} & m_{32} & m_{33} \\
 * \end{pmatrix}
 * \f]
 *
 * It can be noted that the bottom row has a translational effect. Also note, that the matrix
 * determinant represents the volume change effected.
 */
typedef struct br_matrix4 {
    /**
     * \brief Each element of the matrix can be freely and individually accessed.
     *
     * This matrix can also be thought of as an array of four br_vector4 structures, e.g. br_vector4
     * m[4]. Thus m[row] can be cast as (br_vector4*).
     */
    br_scalar m[4][4];
} br_matrix4;

typedef struct br_matrix4_f {
    br_float m[4][4];
} br_matrix4_f;

typedef struct br_matrix4_x {
    br_fixed_ls m[4][4];
} br_matrix4_x;

#endif
