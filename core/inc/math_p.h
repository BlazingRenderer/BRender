/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: math_p.h 1.7 1998/10/16 10:33:15 johng Exp $
 * $Locker: $
 *
 * Public function prototypes for BRender maths support
 */
#ifndef _MATH_P_H_
#define _MATH_P_H_

#ifndef _NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

/**
 ** Fixed Point
 **/

/* result = abs(a)
 */
/**
 * \brief Return the equivalent of abs(a).
 */
br_fixed_ls BrFixedAbs(br_fixed_ls a);

/* result = a*b
 */
/**
 * \brief Return the equivalent of a*b.
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedMul(br_fixed_ls a, br_fixed_ls b);

/* result = a*b + c*d
 */
/**
 * \brief Return the equivalent of a*b + c*d.
 */
br_fixed_ls BrFixedMac2(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d);

/* result = a*b + c*d + e*f
 */
/**
 * \brief Return the equivalent of a*b + c*d + e*f.
 */
br_fixed_ls BrFixedMac3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e, br_fixed_ls f);

/* result = a*b + c*d + e*f + g*h
 */
/**
 * \brief Return the equivalent of a*b + c*d + e*f + g*h.
 */
br_fixed_ls BrFixedMac4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e, br_fixed_ls f, br_fixed_ls g, br_fixed_ls h);

/* result = a*a
 */
/**
 * \brief Return the equivalent of a*a.
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr(br_fixed_ls a);

/* result = a*a + b*b
 */
/**
 * \brief Return the equivalent of a*a + b*b.
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr2(br_fixed_ls a, br_fixed_ls b);

/* result = a*a + b*b + c*c
 */
/**
 * \brief Return the equivalent of a*a + b*b + c*c.
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c);
/* result = a*a + b*b + c*c + d*d
 */
/**
 * \brief Return the equivalent of a*a + b*b + c*c + d*d.
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d);

/* result = sqrt(a*a + b*b)
 */
/**
 * \brief Return the equivalent of sqrt(a*a + b*b).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedLength2(br_fixed_ls a, br_fixed_ls b);

/* result = sqrt(a*a + b*b + c*c)
 */
/**
 * \brief Return the equivalent of sqrt(a*a + b*b + c*c).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedLength3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c);

/* result = sqrt(a*a + b*b + c*c + d*d)
 */
/**
 * \brief Return the equivalent of sqrt(a*a + b*b + c*c + d*d).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedLength4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d);

/* result = 1/sqrt(a*a + b*b) (low precision)
 */
/**
 * \brief Return the equivalent of 1/sqrt(a*a + b*b) (low precision).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedRLength2(br_fixed_ls a, br_fixed_ls b);

/* result = 1/sqrt(a*a + b*b + c*c) (low precision)
 */
/**
 * \brief Return the equivalent of 1/sqrt(a*a + b*b + c*c) (low precision).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedRLength3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c);

/* result = 1/sqrt(a*a + b*b + c*c + d*d) (low precision)
 */
/**
 * \brief Return the equivalent of 1/sqrt(a*a + b*b + c*c + d*d) (low precision).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedRLength4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d);

/* result = a/b
 */
/**
 * \brief Return the equivalent of a/b.
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedDiv(br_fixed_ls a, br_fixed_ls b);

/* result = a/b * 2^31
 */
/**
 * \brief Return the equivalent of a/b * 2^31.
 *
 * Both operands are positive; the reference is an unsigned 32-bit divide of
 * the 64 bit numerator (a << 31) (see g386ifix.h/wtcifix.h and fixed386.asm).
 */
br_fixed_ls BR_ASM_CALL BrFixedDivF(br_fixed_ls a, br_fixed_ls b);

/* result = a/b (rounded towards 0)
 */
/**
 * \brief Return the equivalent of a/b (rounded towards zero).
 *
 * A negative numerator is biased by 2^16-1 before the divide, matching the x86
 * reference; for a >= 0 this is the same as BrFixedDiv().
 */
br_fixed_ls BR_ASM_CALL BrFixedDivR(br_fixed_ls a, br_fixed_ls b);

/* result = a*b/c
 */
/**
 * \brief Return the equivalent of a*b/c.
 */
br_fixed_ls BR_ASM_CALL BrFixedMulDiv(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c);

/* result = (a*b + c*d)/e
 */
/**
 * \brief Return the equivalent of (a*b + c*d)/e.
 */
br_fixed_ls BR_ASM_CALL BrFixedMac2Div(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e);

/* result = (a*b + c*d + e*f)/g
 */
/**
 * \brief Return the equivalent of (a*b + c*d + e*f)/g.
 */
br_fixed_ls BR_ASM_CALL BrFixedMac3Div(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e, br_fixed_ls f, br_fixed_ls g);

/* result = (a*b + c*d + e*f + g*h)/i
 */
/**
 * \brief Return the equivalent of (a*b + c*d + e*f + g*h)/i.
 */
br_fixed_ls BR_ASM_CALL BrFixedMac4Div(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e, br_fixed_ls f,
                                       br_fixed_ls g, br_fixed_ls h, br_fixed_ls i);
/* result = 1.0/a
 */
/**
 * \brief Return the equivalent of 1.0/a.
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedRcp(br_fixed_ls a);

/*
 * Various combinations with fractions
 */

/* result = a*b + c*d - a & c are fractions
 */
/**
 * \brief Return the equivalent of a*b + c*d (a & c are fractions).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedFMac2(br_fixed_lsf a, br_fixed_ls b, br_fixed_lsf c, br_fixed_ls d);

/* result = a*b + c*d + e*f - a,c & e are fractions
 */
/**
 * \brief Return the equivalent of a*b + c*d + e*f (a, c & e are fractions).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedFMac3(br_fixed_lsf a, br_fixed_ls b, br_fixed_lsf c, br_fixed_ls d, br_fixed_lsf e, br_fixed_ls f);

/* result = a*b + c*d + e*f + g*h (a,c,e,g are fractions)
 */
/**
 * \brief Return the equivalent of a*b + c*d + e*f + g*h (a, c, e, & g are fractions).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedFMac4(br_fixed_lsf a, br_fixed_ls b, br_fixed_lsf c, br_fixed_ls d, br_fixed_lsf e, br_fixed_ls f,
                                         br_fixed_lsf g, br_fixed_ls h);

/*
 * Misc. support  functions
 */
/**
 * \brief Return the equivalent of sin(a) (see br_angle).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedSin(br_fixed_luf a);

/**
 * \brief Return the equivalent of cos(a) (see br_angle).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedCos(br_fixed_luf a);

/**
 * \brief Return the equivalent of asin(s) (see br_angle).
 */
br_fixed_luf BR_ASM_CALL BrFixedASin(br_fixed_ls s);

/**
 * \brief Return the equivalent of acos(c) (see br_angle).
 */
br_fixed_luf BR_ASM_CALL BrFixedACos(br_fixed_ls c);

/**
 * \brief Return the equivalent of atan2(x,y) (see br_angle).
 */
br_fixed_luf BR_PUBLIC_ENTRY BrFixedATan2(br_fixed_ls y, br_fixed_ls x);

/**
 * \brief Return the equivalent of atan2(a) (low precision) (see br_angle).
 *
 * Low precision BrFixedATan2(): the arctan table lookup is replaced by the
 * linear approximation atan(r) ~= PI/4 * r, so each octant contributes or
 * subtracts r >> 3 (r being the 0.16 fractional part of |y|/|x| or |x|/|y|).
 */
br_fixed_luf BR_ASM_CALL BrFixedATan2Fast(br_fixed_ls x, br_fixed_ls y);

/**
 * \brief Return the equivalent of sqrt(a).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqrt(br_fixed_ls a);
/**
 * \brief Return the equivalent of pow(a,b).
 */
br_fixed_ls BR_PUBLIC_ENTRY BrFixedPow(br_fixed_ls a, br_fixed_ls b);

br_scalar BR_PUBLIC_ENTRY BrScalarLerp(br_scalar a, br_scalar b, br_scalar t);

/*
 * Integer sqrt functions
 */
br_uint_32 BR_ASM_CALL BrSqrt(br_uint_32 a);
br_uint_32 BR_ASM_CALL BrFastSqrt(br_uint_32 a);
br_uint_32 BR_ASM_CALL BrFastRSqrt(br_uint_32 a);

/*
 * 3x4 Matrix ops.
 */
/**
 * \brief Copy a matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow B\f$
 *
 * \param A A pointer to the destination matrix (may be the same as source – though redundant).
 * \param b A pointer to the source matrix.
 */
void BR_PUBLIC_ENTRY BrMatrix34Copy(br_matrix34 *A, const br_matrix34 *b);
/**
 * \brief Multiply two matrices together and place the result in a third matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow BC\f$
 *
 * \param A A pointer to the destination matrix (must be different from both sources).
 * \param B Pointer to the left hand source matrix.
 * \param C Pointer to the right hand source matrix.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} b_{00} & b_{01} & b_{02} & 0 \\ b_{10} & b_{11} & b_{12} & 0 \\ b_{20} & b_{21} & b_{22} & 0 \\ b_{30} & b_{31} & b_{32} & 1 \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & 0 \\ c_{10} & c_{11} & c_{12} & 0 \\ c_{20} & c_{21} & c_{22} & 0 \\ c_{30} & c_{31} & c_{32} & 1 \end{pmatrix}
 * = \begin{pmatrix}
 * b_{00}c_{00} + b_{01}c_{10} + b_{02}c_{20} & b_{00}c_{01} + b_{01}c_{11} + b_{02}c_{21} & b_{00}c_{02} + b_{01}c_{12} + b_{02}c_{22} & 0 \\
 * b_{10}c_{00} + b_{11}c_{10} + b_{12}c_{20} & b_{10}c_{01} + b_{11}c_{11} + b_{12}c_{21} & b_{10}c_{02} + b_{11}c_{12} + b_{12}c_{22} & 0 \\
 * b_{20}c_{00} + b_{21}c_{10} + b_{22}c_{20} & b_{20}c_{01} + b_{21}c_{11} + b_{22}c_{21} & b_{20}c_{02} + b_{21}c_{12} + b_{22}c_{22} & 0 \\
 * b_{30}c_{00} + b_{31}c_{10} + b_{32}c_{20} + c_{30} & b_{30}c_{01} + b_{31}c_{11} + b_{32}c_{21} + c_{31} & b_{30}c_{02} + b_{31}c_{12} + b_{32}c_{22} + c_{32} & 1
 * \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix34Pre(), BrMatrix34Post()
 */
void BR_PUBLIC_ENTRY BrMatrix34Mul(br_matrix34 *A, const br_matrix34 *B, const br_matrix34 *C);
/**
 * \brief Pre-multiply one matrix by another.
 *
 * Equivalent to the expression: \f$A \Leftarrow BA\f$
 *
 * \param mat A pointer to the subject matrix (may be same as B).
 * \param A   A pointer to the pre-multiplying matrix.
 *
 * \sa BrMatrix34Post(), BrMatrix34Mul()
 */
void BR_PUBLIC_ENTRY BrMatrix34Pre(br_matrix34 *mat, const br_matrix34 *A);
/**
 * \brief Post-multiply one matrix by another.
 *
 * Equivalent to the expression: \f$A \Leftarrow AB\f$
 *
 * \param mat A pointer to the subject matrix (may be same as B).
 * \param A   A pointer to the post-multiplying matrix.
 *
 * \sa BrMatrix34Pre(), BrMatrix34Mul()
 */
void BR_PUBLIC_ENTRY BrMatrix34Post(br_matrix34 *mat, const br_matrix34 *A);

/**
 * \brief Set the specified matrix to the identity transformation matrix.
 *
 * Equivalent to: \f$M \Leftarrow I = \begin{pmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 *
 * \post Stores the identity matrix at the destination.
 */
void BR_PUBLIC_ENTRY BrMatrix34Identity(br_matrix34 *mat);

/**
 * \brief Set the specified matrix to a matrix representing a rotation about the x axis though a
 *        specified angle.
 *
 * Equivalent to: \f$M \Leftarrow R_{\theta X} = \begin{pmatrix} 1 & 0 & 0 & 0 \\ 0 & \cos(\theta_X) & \sin(\theta_X) & 0 \\ 0 & -\sin(\theta_X) & \cos(\theta_X) & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param rx  Rotation about the x axis.
 *
 * \sa BrMatrix34PreRotateX(), BrMatrix34PostRotateX()
 */
void BR_PUBLIC_ENTRY BrMatrix34RotateX(br_matrix34 *mat, br_angle rx);
/**
 * \brief Pre-multiply a matrix by an x axis rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow R_{\theta x} M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param rx  The angle about the x axis used to form the rotation matrix. A positive angle
 *            represents a clockwise rotation (looking toward the origin).
 *
 * \sa BrMatrix34PostRotateX(), BrMatrix34RotateX()
 */
void BR_PUBLIC_ENTRY BrMatrix34PreRotateX(br_matrix34 *mat, br_angle rx);
/**
 * \brief Post-multiply a matrix by an x axis rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MR_{\theta x}\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param rx  The angle about the x axis used to form the rotation matrix. A positive angle
 *            represents a clockwise rotation (looking towards the origin).
 *
 * \sa BrMatrix34PreRotateX(), BrMatrix34RotateX()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostRotateX(br_matrix34 *mat, br_angle rx);

/**
 * \brief Set the specified matrix to a matrix representing a rotation about the y axis though a
 *        specified angle.
 *
 * Equivalent to: \f$M \Leftarrow R_{\theta Y} = \begin{pmatrix} \cos(\theta_Y) & 0 & -\sin(\theta_Y) & 0 \\ 0 & 1 & 0 & 0 \\ \sin(\theta_Y) & 0 & \cos(\theta_Y) & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param ry  Rotation about the y axis.
 *
 * \sa BrMatrix34PreRotateY(), BrMatrix34PostRotateY()
 */
void BR_PUBLIC_ENTRY BrMatrix34RotateY(br_matrix34 *mat, br_angle ry);
/**
 * \brief Pre-multiply a matrix by a y axis, rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow R_{\theta y} M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param ry  The angle about the y axis used to form the rotation matrix. A positive angle
 *            represents a clockwise rotation (looking towards the origin).
 *
 * \sa BrMatrix34PostRotateY(), BrMatrix34RotateY()
 */
void BR_PUBLIC_ENTRY BrMatrix34PreRotateY(br_matrix34 *mat, br_angle ry);
/**
 * \brief Post-multiply a matrix by a y axis, rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow M R_{\theta Y}\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param ry  The angle about the y axis used to form the rotation matrix. A positive angle
 *            represents a clockwise rotation (looking towards the origin).
 *
 * \sa BrMatrix34PreRotateY(), BrMatrix34RotateY()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostRotateY(br_matrix34 *mat, br_angle ry);

/**
 * \brief Set the specified matrix to a matrix representing a rotation about the z axis though a
 *        specified angle.
 *
 * Equivalent to: \f$M \Leftarrow R_{\theta Z} = \begin{pmatrix} \cos(\theta_Z) & \sin(\theta_Z) & 0 & 0 \\ -\sin(\theta_Z) & \cos(\theta_Z) & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param rz  Rotation about the z axis.
 *
 * \sa BrMatrix34PreRotateZ(), BrMatrix34PostRotateZ()
 */
void BR_PUBLIC_ENTRY BrMatrix34RotateZ(br_matrix34 *mat, br_angle rz);
/**
 * \brief Pre-multiply a matrix by a z axis, rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow R_{\theta z} M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param rz  The angle about the z axis used to form the rotation matrix. A positive angle
 *            represents a clockwise rotation (looking towards the origin).
 *
 * \sa BrMatrix34PostRotateZ(), BrMatrix34RotateZ()
 */
void BR_PUBLIC_ENTRY BrMatrix34PreRotateZ(br_matrix34 *mat, br_angle rz);
/**
 * \brief Post-multiply a matrix by a z axis, rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow M R_{\theta Z}\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param rz  The angle about the z axis used to form the rotation matrix. A positive angle
 *            represents a clockwise rotation (looking towards the origin).
 *
 * \sa BrMatrix34PreRotateZ(), BrMatrix34RotateZ()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostRotateZ(br_matrix34 *mat, br_angle rz);

/**
 * \brief Set the specified matrix to a matrix representing a rotation about a given axis vector
 *        though a specified angle.
 *
 * Equivalent to: \f$M \Leftarrow R_{\theta a}\f$
 *
 * \param mat  A pointer to the destination matrix.
 * \param r    Rotation about the specified axis vector.
 * \param axis The arbitrary (normalised) axis vector about which the rotation occurs.
 *
 * \sa BrMatrix34PreRotate(), BrMatrix34PostRotate()
 */
void BR_PUBLIC_ENTRY BrMatrix34Rotate(br_matrix34 *mat, br_angle r, const br_vector3 *axis);
/**
 * \brief Pre-multiply a matrix by a vector specified axis, rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow R_{\theta a} M\f$
 *
 * \param mat  A pointer to the subject matrix.
 * \param r    The angle about the specified axis used to form the rotation matrix. A positive angle
 *             represents a clockwise rotation (with the vector pointing at you).
 * \param axis The arbitrary (normalised) axis vector about which the rotation occurs.
 *
 * \sa BrMatrix34PostRotate(), BrMatrix34Rotate()
 */
void BR_PUBLIC_ENTRY BrMatrix34PreRotate(br_matrix34 *mat, br_angle r, const br_vector3 *axis);
/**
 * \brief Post-multiply a matrix by a vector specified axis, rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MR_{\theta a}\f$
 *
 * \param mat  A pointer to the subject matrix.
 * \param r    The angle about the specified axis used to form the rotation matrix.
 * \param axis The arbitrary (normalised) axis vector about which the rotation occurs.
 *
 * \sa BrMatrix34PreRotate(), BrMatrix34Rotate()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostRotate(br_matrix34 *mat, br_angle r, const br_vector3 *axis);

/**
 * \brief Set the specified matrix to a matrix representing a specific translation.
 *
 * Equivalent to: \f$M \Leftarrow T_{xyz} = \begin{pmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ d_x & d_y & d_z & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param x   Translation component along the x axis.
 * \param y   Translation component along the y axis.
 * \param z   Translation component along the z axis.
 *
 * \sa BrMatrix34PreTranslate(), BrMatrix34PostTranslate()
 */
void BR_PUBLIC_ENTRY BrMatrix34Translate(br_matrix34 *mat, br_scalar x, br_scalar y, br_scalar z);
/**
 * \brief Pre-multiply a matrix by a translation transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow T_{xyz} M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param x   The x axis component used to form the translation matrix.
 * \param y   The y axis component used to form the translation matrix.
 * \param z   The z axis component used to form the translation matrix.
 *
 * \sa BrMatrix34PostTranslate(), BrMatrix34Translate()
 */
void BR_PUBLIC_ENTRY BrMatrix34PreTranslate(br_matrix34 *mat, br_scalar x, br_scalar y, br_scalar z);
/**
 * \brief Post-multiply a matrix by a translation transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MT_{xyz}\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param x   The x axis component used to form the translation matrix.
 * \param y   The y axis component used to form the translation matrix.
 * \param z   The z axis component used to form the translation matrix.
 *
 * \sa BrMatrix34PreTranslate(), BrMatrix34Translate()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostTranslate(br_matrix34 *mat, br_scalar x, br_scalar y, br_scalar z);

/**
 * \brief Set the specified matrix to a matrix representing a specific scaling.
 *
 * Equivalent to: \f$M \Leftarrow S_{xyz} = \begin{pmatrix} s_x & 0 & 0 & 0 \\ 0 & s_y & 0 & 0 \\ 0 & 0 & s_z & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param sx  Scaling component along the x axis.
 * \param sy  Scaling component along the y axis.
 * \param sz  Scaling component along the z axis.
 *
 * \sa BrMatrix34PreScale(), BrMatrix34PostScale()
 */
void BR_PUBLIC_ENTRY BrMatrix34Scale(br_matrix34 *mat, br_scalar sx, br_scalar sy, br_scalar sz);
/**
 * \brief Pre-multiply a matrix by a scaling transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow S_{xyz} M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Scaling component along the x axis.
 * \param sy  Scaling component along the y axis.
 * \param sz  Scaling component along the z axis.
 *
 * \sa BrMatrix34PostScale(), BrMatrix34Scale()
 */
void BR_PUBLIC_ENTRY BrMatrix34PreScale(br_matrix34 *mat, br_scalar sx, br_scalar sy, br_scalar sz);

/**
 * \brief Post-multiply a matrix by a scaling transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MS_{xyz}\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Scaling component along the x axis.
 * \param sy  Scaling component along the y axis.
 * \param sz  Scaling component along the z axis.
 *
 * \sa BrMatrix34PreScale(), BrMatrix34Scale()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostScale(br_matrix34 *mat, br_scalar sx, br_scalar sy, br_scalar sz);

/**
 * \brief Set the specified matrix to a matrix representing a shear, invariant along the x axis.
 *
 * Thus values of y and z co-ordinates will be scaled in proportion to the value of the x
 * co-ordinate. Equivalent to: \f$M \Leftarrow Z_X = \begin{pmatrix} 1 & d_y & d_z & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param sy  Shear factor by which the x co-ordinate is included in the transformed y co-ordinate.
 * \param sz  Shear factor by which the x co-ordinate is included in the transformed z co-ordinate.
 *
 * \sa BrMatrix34PreShearX(), BrMatrix34PostShearX()
 */
void BR_PUBLIC_ENTRY BrMatrix34ShearX(br_matrix34 *mat, br_scalar sy, br_scalar sz);
/**
 * \brief Pre-multiply a matrix by an x invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow Z_x M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sy  Shear factor by which the x co-ordinate is included in the transformed y co-ordinate.
 * \param sz  Shear factor by which the x co-ordinate is included in the transformed z co-ordinate.
 *
 * \sa BrMatrix34PostShearX(), BrMatrix34ShearX()
 */
void BR_PUBLIC_ENTRY BrMatrix34PreShearX(br_matrix34 *mat, br_scalar sy, br_scalar sz);
/**
 * \brief Post-multiply a matrix by an x invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MZ_x\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sy  Shear factor by which the x co-ordinate is included in the transformed y co-ordinate.
 * \param sz  Shear factor by which the x co-ordinate is included in the transformed z co-ordinate.
 *
 * \sa BrMatrix34PreShearX(), BrMatrix34ShearX()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostShearX(br_matrix34 *mat, br_scalar sy, br_scalar sz);

/**
 * \brief Set the specified matrix to a matrix representing a shear, invariant along the y axis.
 *
 * Thus values of x and z co-ordinates will be scaled in proportion to the value of the y
 * co-ordinate. Equivalent to: \f$M \Leftarrow Z_Y = \begin{pmatrix} 1 & 0 & 0 & 0 \\ s_x & 1 & s_z & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param sx  Shear factor by which the y co-ordinate is included in the transformed x co-ordinate.
 * \param sz  Shear factor by which the y co-ordinate is included in the transformed z co-ordinate.
 *
 * \sa BrMatrix34PreShearY(), BrMatrix34PostShearY()
 */
void BR_PUBLIC_ENTRY BrMatrix34ShearY(br_matrix34 *mat, br_scalar sx, br_scalar sz);
/**
 * \brief Pre-multiply a matrix by a y invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow Z_y M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Shear factor by which the y co-ordinate is included in the transformed x co-ordinate.
 * \param sz  Shear factor by which the y co-ordinate is included in the transformed z co-ordinate.
 *
 * \sa BrMatrix34PostShearY(), BrMatrix34ShearY()
 */
void BR_PUBLIC_ENTRY BrMatrix34PreShearY(br_matrix34 *mat, br_scalar sx, br_scalar sz);
/**
 * \brief Post-multiply a matrix by a y invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MZ_y\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Shear factor by which the y co-ordinate is included in the transformed x co-ordinate.
 * \param sz  Shear factor by which the y co-ordinate is included in the transformed z co-ordinate.
 *
 * \sa BrMatrix34PreShearY(), BrMatrix34ShearY()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostShearY(br_matrix34 *mat, br_scalar sx, br_scalar sz);

/**
 * \brief Set the specified matrix to a matrix representing a shear, invariant along the z axis.
 *
 * Thus values of x and y co-ordinates will be scaled in proportion to the value of the z
 * co-ordinate. Equivalent to: \f$M \Leftarrow Z_z = \begin{pmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ s_x & s_y & 1 & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param sx  Shear factor by which the z co-ordinate is included in the transformed x co-ordinate.
 * \param sy  Shear factor by which the z co-ordinate is included in the transformed y co-ordinate.
 *
 * \sa BrMatrix34PreShearZ(), BrMatrix34PostShearZ()
 */
void BR_PUBLIC_ENTRY BrMatrix34ShearZ(br_matrix34 *mat, br_scalar sx, br_scalar sy);
/**
 * \brief Pre-multiply a matrix by a z invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow Z_z M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Shear factor by which the z co-ordinate is included in the transformed x co-ordinate.
 * \param sy  Shear factor by which the z co-ordinate is included in the transformed y co-ordinate.
 *
 * \sa BrMatrix34PostShearZ(), BrMatrix34ShearZ()
 */
void BR_PUBLIC_ENTRY BrMatrix34PreShearZ(br_matrix34 *mat, br_scalar sx, br_scalar sy);
/**
 * \brief Post-multiply a matrix by a z invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MZ_z\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Shear factor by which the z co-ordinate is included in the transformed x co-ordinate.
 * \param sy  Shear factor by which the z co-ordinate is included in the transformed y co-ordinate.
 *
 * \sa BrMatrix34PreShearZ(), BrMatrix34ShearZ()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostShearZ(br_matrix34 *mat, br_scalar sx, br_scalar sy);

/**
 * \brief Applies a transform to a 3D vector, i.e. as for a point but without translation components
 *        (a vector has no location).
 *
 * Equivalent to the expression: \f$V_A \Leftarrow V_B C\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed vector.
 * \param B A pointer to the source vector, holding the vector to be transformed.
 * \param C A pointer to the transform matrix to be applied.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} x_B & y_B & z_B & 0 \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & 0 \\ c_{10} & c_{11} & c_{12} & 0 \\ c_{20} & c_{21} & c_{22} & 0 \\ c_{30} & c_{31} & c_{32} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} x_B c_{00} + y_B c_{10} + z_B c_{20} + c_{30} & x_B c_{01} + y_B c_{11} + z_B c_{21} + c_{31} & x_B c_{02} + y_B c_{12} + z_B c_{22} + c_{32} & 0 \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix34ApplyV(br_vector3 *A, const br_vector3 *B, const br_matrix34 *C);
/**
 * \brief Applies a transform to a 3D point.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} x_B & y_B & z_B & 1 \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & 0 \\ c_{10} & c_{11} & c_{12} & 0 \\ c_{20} & c_{21} & c_{22} & 0 \\ c_{30} & c_{31} & c_{32} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} x_B c_{00} + y_B c_{10} + z_B c_{20} + c_{30} & x_B c_{01} + y_B c_{11} + z_B c_{21} + c_{31} & x_B c_{02} + y_B c_{12} + z_B c_{22} + c_{32} & 1 \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix34ApplyP(br_vector3 *A, const br_vector3 *B, const br_matrix34 *C);
/**
 * \brief Applies a transform to a 3D point which may have non-unity homogenous co-ordinates.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied.
 *
 * \remark Be aware that the fourth element of the resulting vector is only implicit, and if
 *         required must either be copied (if the destination is actually a br_vector4) or used to
 *         scale down the first three elements. This all depends on the purpose for which this
 *         function is called. The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} x_B & y_B & z_B & w_B \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & 0 \\ c_{10} & c_{11} & c_{12} & 0 \\ c_{20} & c_{21} & c_{22} & 0 \\ c_{30} & c_{31} & c_{32} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} x_B c_{00} + y_B c_{10} + z_B c_{20} + w_B c_{30} & x_B c_{01} + y_B c_{11} + z_B c_{21} + w_B c_{31} & x_B c_{02} + y_B c_{12} + z_B c_{22} + w_B c_{32} & \{ w_B \} \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix34Apply(br_vector3 *A, const br_vector4 *B, const br_matrix34 *C);

/**
 * \brief Applies a transposed transform to a 3D vector, i.e. as for a point but without translation
 *        components (a vector has no location).
 *
 * Equivalent to the expression: \f$V_A \Leftarrow V_B C^{t}\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed vector.
 * \param B A pointer to the source vector, holding the vector to be transformed.
 * \param C A pointer to the transform matrix to be applied transposed - the translation elements
 *          are presumed zero or irrelevant.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & 0 \\ c_{10} & c_{11} & c_{12} & 0 \\ c_{20} & c_{21} & c_{22} & 0 \\ - & - & - & 1 \end{pmatrix}
 * \begin{pmatrix} x_B \\ y_B \\ z_B \\ 0 \end{pmatrix}
 * \equiv \begin{pmatrix} c_{00}x_B + c_{01}y_B + c_{02}z_B \\ c_{10}x_B + c_{11}y_B + c_{12}z_B \\ c_{20}x_B + c_{21}y_B + c_{22}z_B \\ 0 \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix34TApplyV(br_vector3 *A, const br_vector3 *B, const br_matrix34 *C);
/**
 * \brief Applies a transposed transform to a 3D point.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C^{t}\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied transposed - the translation elements
 *          are presumed zero or irrelevant.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & 0 \\ c_{10} & c_{11} & c_{12} & 0 \\ c_{20} & c_{21} & c_{22} & 0 \\ - & - & - & 1 \end{pmatrix}
 * \begin{pmatrix} x_B \\ y_B \\ z_B \\ 1 \end{pmatrix}
 * \equiv \begin{pmatrix} c_{00}x_B + c_{01}y_B + c_{02}z_B \\ c_{10}x_B + c_{11}y_B + c_{12}z_B \\ c_{20}x_B + c_{21}y_B + c_{22}z_B \\ 1 \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix34TApplyP(br_vector3 *A, const br_vector3 *B, const br_matrix34 *C);
/**
 * \brief Applies a transform to a transposed 3D point which may have non-unity homogenous
 *        co-ordinates.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C^{t}\f$
 *
 * \param A A pointer to the destination vector (must be different from source), to hold the
 *          transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied transposed.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & 0 \\ c_{10} & c_{11} & c_{12} & 0 \\ c_{20} & c_{21} & c_{22} & 0 \\ c_{30} & c_{31} & c_{32} & 1 \end{pmatrix}
 * \begin{pmatrix} x_B \\ y_B \\ z_B \\ w_B \end{pmatrix}
 * \equiv \begin{pmatrix} c_{00}x_B + c_{01}y_B + c_{02}z_B \\ c_{10}x_B + c_{11}y_B + c_{12}z_B \\ c_{20}x_B + c_{21}y_B + c_{22}z_B \\ c_{30}x_B + c_{31}y_B + c_{32}z_B + w_B \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix34TApply(br_vector4 *A, const br_vector4 *B, const br_matrix34 *C);

/**
 * \brief Compute the inverse of the supplied 3D affine matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow B^{-1}\f$
 *
 * \param out A pointer to the destination matrix (must be different from source).
 * \param in  A pointer to the source matrix.
 *
 * \return If the inverse exists, the determinant of the source matrix is returned. If there is no
 *         inverse, scalar zero is returned.
 *
 * \remark Remember that while an inverse may be obtained using double precision arithmetic, this
 *         does not necessarily mean that it can using the br_scalar type. This difference is most
 *         marked between fixed and floating point BRender libraries.
 *
 * \sa BrMatrix34LPInverse().
 */
br_scalar BR_PUBLIC_ENTRY BrMatrix34Inverse(br_matrix34 *out, const br_matrix34 *in);
/**
 * \brief Compute the inverse of the supplied length preserving* transformation matrix.
 *
 * The resulting matrix is undefined for non-length preserving matrices. Equivalent to the
 * expression: \f$A_{LP} \Leftarrow B_{LP}^{-1}\f$
 *
 * \param A A pointer to the destination matrix (must be different from source).
 * \param B A pointer to the source matrix.
 *
 * \sa BrMatrix34Inverse().
 */
void BR_PUBLIC_ENTRY      BrMatrix34LPInverse(br_matrix34 *A, const br_matrix34 *B);
/**
 * \brief Normalise a length preserving* matrix.
 *
 * Equivalent to the expression: \f$A_{LP} \Leftarrow Norm(B_{LP})\f$
 *
 * \param A A pointer to the destination matrix, which must not point to the source matrix.
 * \param B A pointer to the source matrix.
 *
 * \post The destination matrix is the source matrix adjusted so that it represents a length
 *       preserving transformation.
 *
 * \remark This function is typically applied to a length preserving matrix which has undergone a
 *         long sequence of operations, to ensure that the final matrix is still truly length
 *         preserving (regardless of rounding errors).
 */
void BR_PUBLIC_ENTRY      BrMatrix34LPNormalise(br_matrix34 *A, const br_matrix34 *B);

/**
 * \brief This function generates a matrix which provides an intuitive way of controlling the
 *        rotation of 3D objects with a mouse or other 2D pointing device.
 *
 * The mouse may be thought of as controlling a horizontal flat surface resting on top of a fixed
 * sphere. As the surface is moved in any direction (but not rotated), so the sphere will rotate.
 *
 * \param mat    A pointer to the destination matrix.
 * \param dx     The amount the 'top surface' has moved in each direction.
 * \param dy     The amount the 'top surface' has moved in each direction.
 * \param radius The radius of the imaginary ball.
 *
 * \post This function calculates the tangent vector (dx,dy) to the sphere of radius radius in 3D,
 *       and uses this to determine the axis of rotation normal to this tangent at the centre of the
 *       sphere, and the angle subtended by the tangent vector. From this, a transform matrix is
 *       created, describing the rotation.
 *
 * \remark The function is expected to be used with frequent samples of movements made with the 2D
 *         manipulator, i.e. a movement of 10cm in one go will produce a smaller rotation (no
 *         greater than 180°) whereas the same movement sampled at 100 intervals will cumulatively
 *         produce a larger rotation (possibly several revolutions). This is unlikely to be a
 *         problem in practice. Note that using the 2D manipulator to describe small circles can
 *         rotate the 3D object about its vertical axis.
 *
 * \sa The book Graphics Gems III, edited by David Kirk, ISBN 0-12-409670-0, Ch.2, Pt.3, 'The
 *     Rolling Ball', Andrew J. Hanson, p51.
 *
 * \par Example
 * \code{.c}
 * int mouse_x,mouse_y;
 * br_matrix34 mat;
 * ...
 * BrMatrix34RollingBall(&mat,-mouse_x,mouse_y,500);
 * \endcode
 */
void BR_PUBLIC_ENTRY BrMatrix34RollingBall(br_matrix34 *mat, int dx, int dy, int radius);

br_matrix34 *BR_PUBLIC_ENTRY BrMatrix34FromTRS(br_matrix34 *mat, const br_vector3 *t, const br_quat *r, const br_vector3 *s);

/**
 * \brief Find the transform that maps a 2 unit cube centred at the origin to the given bounding
 *        box.
 *
 * \param mat    A non-NULL pointer to the destination matrix, into which will be placed the
 *               transform.
 * \param bounds A non-NULL pointer to the bounding box.
 *
 * \post Will calculate the transform required to transform a 2 unit cube centred at the origin to
 *       the given bounding box.
 *
 * \return The pointer to the destination matrix, `mat`, returned for convenience.
 *
 * \remark Typically used to facilitate 3D cursors, e.g. a child model actor with a cursor model
 *         whose co-ordinates are based upon a 2 unit cube, can use this function to define its
 *         transform.
 *
 * \sa br_actor
 */
br_matrix34 *BR_PUBLIC_ENTRY BrBoundsToMatrix34(br_matrix34 *mat, const br_bounds *bounds);

void BR_PUBLIC_ENTRY BrMatrix34ApplyBounds(br_bounds *A, const br_bounds *B, const br_matrix34 *C);

void BR_PUBLIC_ENTRY BrMatrix34ApplyPlaneEquation(br_vector4 *A, const br_vector4 *B, const br_matrix34 *C);

void BR_PUBLIC_ENTRY BrMatrix34FixedToFloat(br_matrix34_f *A, const br_matrix34_x *B);

br_scalar BR_PUBLIC_ENTRY BrMatrix34Determinant3(const br_matrix34 *m);

/*
 * 4x4 Matrix ops.
 */
/**
 * \brief Copy a matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow B\f$
 *
 * \param A A pointer to the destination matrix (may be the same as source - though redundant).
 * \param B A pointer to the source matrix.
 */
void BR_PUBLIC_ENTRY BrMatrix4Copy(br_matrix4 *A, const br_matrix4 *B);
/**
 * \brief Multiply two matrices together and place the result in a third matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow BC\f$
 *
 * \param A A pointer to the destination matrix (must be different from both sources).
 * \param B Pointer to the left hand source matrix.
 * \param C Pointer to the right hand source matrix.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} b_{00} & b_{01} & b_{02} & b_{03} \\ b_{10} & b_{11} & b_{12} & b_{13} \\ b_{20} & b_{21} & b_{22} & b_{23} \\ b_{30} & b_{31} & b_{32} & b_{33} \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & c_{03} \\ c_{10} & c_{11} & c_{12} & c_{13} \\ c_{20} & c_{21} & c_{22} & c_{23} \\ c_{30} & c_{31} & c_{32} & c_{33} \end{pmatrix}
 * \equiv \begin{pmatrix}
 * b_{00}c_{00} + b_{01}c_{10} + b_{02}c_{20} + b_{03}c_{30} & b_{00}c_{01} + b_{01}c_{11} + b_{02}c_{21} + b_{03}c_{31} & b_{00}c_{02} + b_{01}c_{12} + b_{02}c_{22} + b_{03}c_{32} & b_{00}c_{03} + b_{01}c_{13} + b_{02}c_{23} + b_{03}c_{33} \\
 * b_{10}c_{00} + b_{11}c_{10} + b_{12}c_{20} + b_{13}c_{30} & b_{10}c_{01} + b_{11}c_{11} + b_{12}c_{21} + b_{13}c_{31} & b_{10}c_{02} + b_{11}c_{12} + b_{12}c_{22} + b_{13}c_{32} & b_{10}c_{03} + b_{11}c_{13} + b_{12}c_{23} + b_{13}c_{33} \\
 * b_{20}c_{00} + b_{21}c_{10} + b_{22}c_{20} + b_{23}c_{30} & b_{20}c_{01} + b_{21}c_{11} + b_{22}c_{21} + b_{23}c_{31} & b_{20}c_{02} + b_{21}c_{12} + b_{22}c_{22} + b_{23}c_{32} & b_{20}c_{03} + b_{21}c_{13} + b_{22}c_{23} + b_{23}c_{33} \\
 * b_{30}c_{00} + b_{31}c_{10} + b_{32}c_{20} + b_{33}c_{30} & b_{30}c_{01} + b_{31}c_{11} + b_{32}c_{21} + b_{33}c_{31} & b_{30}c_{02} + b_{31}c_{12} + b_{32}c_{22} + b_{33}c_{32} & b_{30}c_{03} + b_{31}c_{13} + b_{32}c_{23} + b_{33}c_{33}
 * \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix4Pre34(), BrMatrix4PreTransform()
 */
void BR_PUBLIC_ENTRY BrMatrix4Mul(br_matrix4 *A, const br_matrix4 *B, const br_matrix4 *C);
/**
 * \brief Set the specified matrix to the identity transformation matrix.
 *
 * Equivalent to: \f$M \Leftarrow I = \begin{pmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 *
 * \post Stores the identity matrix at the destination.
 */
void BR_PUBLIC_ENTRY BrMatrix4Identity(br_matrix4 *mat);
/**
 * \brief Set the specified matrix to a matrix representing a specific scaling.
 *
 * Equivalent to: \f$M \Leftarrow S_{xyz} = \begin{pmatrix} s_x & 0 & 0 & 0 \\ 0 & s_y & 0 & 0 \\ 0 & 0 & s_z & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param sx  Scaling component along the x axis.
 * \param sy  Scaling component along the y axis.
 * \param sz  Scaling component along the z axis.
 */
void BR_PUBLIC_ENTRY BrMatrix4Scale(br_matrix4 *mat, br_scalar sx, br_scalar sy, br_scalar sz);

void BR_PUBLIC_ENTRY      BrMatrix4Transpose(br_matrix4 *A);
/**
 * \brief Compute the inverse of the supplied 3D affine matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow B^{-1}\f$
 *
 * \param A A pointer to the destination matrix (must be different from source).
 * \param B A pointer to the source matrix.
 *
 * \return If the inverse exists, the determinant of the source matrix is returned. If there is no
 *         inverse, scalar zero is returned.
 *
 * \remark Remember that while an inverse may be obtained using double precision arithmetic, this
 *         does not necessarily mean that it can using the br_scalar type. This difference is most
 *         marked between fixed and floating point BRender libraries.
 *
 * \sa BrMatrix4Adjoint(), BrMatrix4Determinant()
 */
br_scalar BR_PUBLIC_ENTRY BrMatrix4Inverse(br_matrix4 *A, const br_matrix4 *B);
/**
 * \brief Find the adjoint of a matrix - the transposed matrix of co-factors.
 *
 * Equivalent to the expression: \f$A \Leftarrow Adjoint(B)\f$
 *
 * \param A A pointer to the destination matrix (may be same as source).
 * \param B A pointer to the source matrix.
 *
 * \sa BrMatrix4Inverse().
 */
void BR_PUBLIC_ENTRY      BrMatrix4Adjoint(br_matrix4 *A, const br_matrix4 *B);
/**
 * \brief Calculate the determinant of a matrix.
 *
 * Equivalent to the expression: \f$|M|\f$
 *
 * \param mat A pointer to the source matrix.
 *
 * \return The determinant of the source matrix.
 *
 * \sa BrMatrix4Inverse().
 */
br_scalar BR_PUBLIC_ENTRY BrMatrix4Determinant(const br_matrix4 *mat);

/**
 * \brief Generate a perspective transformation matrix, that can be used to convert from a camera
 *        actor's co-ordinate space into the homogenous screen space (assuming a centred
 *        projection).
 *
 * This maps the viewing volume into the rendering volume, a cuboid delimited by the homogenous
 * screen co-ordinates (left, bottom, near) (-1,-1,+1) to (+1,+1,-1). The matrix created is
 * equivalent to the following (alpha=field of view):
 * \f[
 * M \Leftarrow Perspective \equiv \begin{pmatrix}
 * \frac{\cot(0.5\alpha)}{\mathrm{aspect}} & 0 & 0 & 0 \\
 * 0 & \cot(0.5\alpha) & 0 & 0 \\
 * 0 & 0 & \frac{z_{yon} + z_{hither}}{z_{yon} - z_{hither}} & -1 \\
 * 0 & 0 & \frac{-2z_{yon}z_{hither}}{z_{yon} - z_{hither}} & 0
 * \end{pmatrix}
 * \f]
 *
 * \param mat           A pointer to the destination matrix to receive the perspective transform.
 * \param field_of_view Field of view, i.e. the angle subtended at the camera between the top and
 *                      bottom of the view volume.
 * \param aspect        Scaling factor for width of viewing volume, i.e. +/-1 in y view ordinates is
 *                      mapped to the height of the output image, and +/-aspect in x view ordinates
 *                      is mapped to the width of the output image.
 * \param hither        z ordinate of front of view volume. The value should be less than zero.
 * \param yon           z ordinate of back of view volume. The value should be less than hither.
 *
 * \post Calculates the camera to screen transformation matrix and stores it at mat.
 *
 * \remark Note that hither and yon are of opposite sign to the comparable values of hither_z and
 *         yon_z that would be specified in br_camera. This is because they are ordinates as opposed
 *         to distances. Note that the front clip plane is at a homogenous screen z ordinate of -1.
 *
 * \sa br_camera, BrActorToScreenMatrix4().
 */
void BR_PUBLIC_ENTRY BrMatrix4Perspective(br_matrix4 *mat, br_angle field_of_view, br_scalar aspect, br_scalar hither, br_scalar yon);

void BR_PUBLIC_ENTRY BrMatrix4Orthographic(br_matrix4 *mat, br_scalar left, br_scalar right, br_scalar bottom, br_scalar top,
                                           br_scalar hither, br_scalar yon);

/**
 * \brief Applies a transform to a 3D vector, i.e. as for a point but without translation components
 *        (a vector has no location).
 *
 * Equivalent to the expression: \f$V_A \Leftarrow V_B C\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed vector.
 * \param B A pointer to the source vector, holding the vector to be transformed.
 * \param C A pointer to the transform matrix to be applied.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} x_B & y_B & z_B & 0 \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & c_{03} \\ c_{10} & c_{11} & c_{12} & c_{13} \\ c_{20} & c_{21} & c_{22} & c_{23} \\ c_{30} & c_{31} & c_{32} & c_{33} \end{pmatrix}
 * \equiv \begin{pmatrix} x_B c_{00} + y_B c_{10} + z_B c_{20} & x_B c_{01} + y_B c_{11} + z_B c_{21} & x_B c_{02} + y_B c_{12} + z_B c_{22} & x_B c_{03} + y_B c_{13} + z_B c_{23} \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix4ApplyV(br_vector4 *A, const br_vector3 *B, const br_matrix4 *C);
/**
 * \brief Applies a transform to a 3D point.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} x_B & y_B & z_B & 1 \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & c_{03} \\ c_{10} & c_{11} & c_{12} & c_{13} \\ c_{20} & c_{21} & c_{22} & c_{23} \\ c_{30} & c_{31} & c_{32} & c_{33} \end{pmatrix}
 * \equiv \begin{pmatrix} x_B c_{00} + y_B c_{10} + z_B c_{20} + c_{30} & x_B c_{01} + y_B c_{11} + z_B c_{21} + c_{31} & x_B c_{02} + y_B c_{12} + z_B c_{22} + c_{32} & x_B c_{03} + y_B c_{13} + z_B c_{23} + c_{33} \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix4ApplyP(br_vector4 *A, const br_vector3 *B, const br_matrix4 *C);
/**
 * \brief Applies a transform to a 3D point which may have non-unity homogenous co-ordinates.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} x_B & y_B & z_B & w_B \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & c_{03} \\ c_{10} & c_{11} & c_{12} & c_{13} \\ c_{20} & c_{21} & c_{22} & c_{23} \\ c_{30} & c_{31} & c_{32} & c_{33} \end{pmatrix}
 * \equiv \begin{pmatrix} x_B c_{00} + y_B c_{10} + z_B c_{20} + w_B c_{30} & x_B c_{01} + y_B c_{11} + z_B c_{21} + w_B c_{31} & x_B c_{02} + y_B c_{12} + z_B c_{22} + w_B c_{32} & x_B c_{03} + y_B c_{13} + z_B c_{23} + w_B c_{33} \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix4Apply(br_vector4 *A, const br_vector4 *B, const br_matrix4 *C);

/**
 * \brief Applies a transposed transform to a 3D vector, i.e. as for a point but without translation
 *        components (a vector has no location).
 *
 * Equivalent to the expression: \f$V_A \Leftarrow V_B C^{t}\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed vector.
 * \param B A pointer to the source vector, holding the vector to be transformed.
 * \param C A pointer to the transform matrix to be applied transposed.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & c_{03} \\ c_{10} & c_{11} & c_{12} & c_{13} \\ c_{20} & c_{21} & c_{22} & c_{23} \\ c_{30} & c_{31} & c_{32} & c_{33} \end{pmatrix}
 * \begin{pmatrix} x_B \\ y_B \\ z_B \\ 0 \end{pmatrix}
 * \equiv \begin{pmatrix} c_{00}x_B + c_{01}y_B + c_{02}z_B \\ c_{10}x_B + c_{11}y_B + c_{12}z_B \\ c_{20}x_B + c_{21}y_B + c_{22}z_B \\ c_{30}x_B + c_{31}y_B + c_{32}z_B \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix4TApplyV(br_vector4 *A, const br_vector3 *B, const br_matrix4 *C);
/**
 * \brief Applies a transposed transform to a 3D point.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C^{t}\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied transposed.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & c_{03} \\ c_{10} & c_{11} & c_{12} & c_{13} \\ c_{20} & c_{21} & c_{22} & c_{23} \\ c_{30} & c_{31} & c_{32} & c_{33} \end{pmatrix}
 * \begin{pmatrix} x_B \\ y_B \\ z_B \\ 1 \end{pmatrix}
 * \equiv \begin{pmatrix} c_{00}x_B + c_{01}y_B + c_{02}z_B + c_{03} \\ c_{10}x_B + c_{11}y_B + c_{12}z_B + c_{13} \\ c_{20}x_B + c_{21}y_B + c_{22}z_B + c_{23} \\ c_{30}x_B + c_{31}y_B + c_{32}z_B + c_{33} \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix4TApplyP(br_vector4 *A, const br_vector3 *B, const br_matrix4 *C);
/**
 * \brief Applies a transform to a transposed 3D point which may have non-unity homogenous
 *        co-ordinates.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C^{t}\f$
 *
 * \param A A pointer to the destination vector (must be different from source), to hold the
 *          transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied transposed.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} c_{00} & c_{01} & c_{02} & c_{03} \\ c_{10} & c_{11} & c_{12} & c_{13} \\ c_{20} & c_{21} & c_{22} & c_{23} \\ c_{30} & c_{31} & c_{32} & c_{33} \end{pmatrix}
 * \begin{pmatrix} x_B \\ y_B \\ z_B \\ w_B \end{pmatrix}
 * \equiv \begin{pmatrix} c_{00}x_B + c_{01}y_B + c_{02}z_B + c_{03}w_B \\ c_{10}x_B + c_{11}y_B + c_{12}z_B + c_{13}w_B \\ c_{20}x_B + c_{21}y_B + c_{22}z_B + c_{23}w_B \\ c_{30}x_B + c_{31}y_B + c_{32}z_B + c_{33}w_B \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix4TApply(br_vector4 *A, const br_vector4 *B, const br_matrix4 *C);

void BR_PUBLIC_ENTRY BrMatrix4Copy23(br_matrix4 *A, const br_matrix23 *B);

/**
 * \brief Pre-multiply one matrix by another.
 *
 * Equivalent to the expression: \f$A \Leftarrow BA\f$
 *
 * \param A A pointer to the subject matrix (may be same as B).
 * \param B A pointer to the pre-multiplying matrix.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} b_{00} & b_{01} & b_{02} & 0 \\ b_{10} & b_{11} & b_{12} & 0 \\ b_{20} & b_{21} & b_{22} & 0 \\ b_{30} & b_{31} & b_{32} & 1 \end{pmatrix}
 * \begin{pmatrix} a_{00} & a_{01} & a_{02} & a_{03} \\ a_{10} & a_{11} & a_{12} & a_{13} \\ a_{20} & a_{21} & a_{22} & a_{23} \\ a_{30} & a_{31} & a_{32} & a_{33} \end{pmatrix}
 * \equiv \begin{pmatrix}
 * b_{00}a_{00} + b_{01}a_{10} + b_{02}a_{20} & b_{00}a_{01} + b_{01}a_{11} + b_{02}a_{21} & b_{00}a_{02} + b_{01}a_{12} + b_{02}a_{22} & b_{00}a_{03} + b_{01}a_{13} + b_{02}a_{23} \\
 * b_{10}a_{00} + b_{11}a_{10} + b_{12}a_{20} & b_{10}a_{01} + b_{11}a_{11} + b_{12}a_{21} & b_{10}a_{02} + b_{11}a_{12} + b_{12}a_{22} & b_{10}a_{03} + b_{11}a_{13} + b_{12}a_{23} \\
 * b_{20}a_{00} + b_{21}a_{10} + b_{22}a_{20} & b_{20}a_{01} + b_{21}a_{11} + b_{22}a_{21} & b_{20}a_{02} + b_{21}a_{12} + b_{22}a_{22} & b_{20}a_{03} + b_{21}a_{13} + b_{22}a_{23} \\
 * b_{30}a_{00} + b_{31}a_{10} + b_{32}a_{20} + a_{30} & b_{30}a_{01} + b_{31}a_{11} + b_{32}a_{21} + a_{31} & b_{30}a_{02} + b_{31}a_{12} + b_{32}a_{22} + a_{32} & b_{30}a_{03} + b_{31}a_{13} + b_{32}a_{23} + a_{33}
 * \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix4Mul().
 */
void BR_PUBLIC_ENTRY BrMatrix4Pre34(br_matrix4 *A, const br_matrix34 *B);
void BR_PUBLIC_ENTRY BrMatrix4Mul34(br_matrix4 *A, const br_matrix34 *B, const br_matrix4 *C);
/**
 * \brief Copy a 3x4 matrix into a 4x4 matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow B\f$
 *
 * \param A A pointer to the destination matrix.
 * \param B A pointer to the source 3x4 matrix.
 *
 * \post The source is copied into the destination, and the fourth column of the destination is set
 *       to the implicit (0,0,0,1) column vector.
 *
 * \sa BrMatrix34Copy4().
 */
void BR_PUBLIC_ENTRY BrMatrix4Copy34(br_matrix4 *A, const br_matrix34 *B);
/**
 * \brief Copy a 4x4 matrix into a 3x4 matrix, discarding right-hand column.
 *
 * Equivalent to the expression: \f$A \Leftarrow B\f$
 *
 * \param A A pointer to the destination matrix.
 * \param B A pointer to the source 4x4 matrix.
 *
 * \sa BrMatrix4Copy34()
 */
void BR_PUBLIC_ENTRY BrMatrix34Copy4(br_matrix34 *A, const br_matrix4 *B);

/**
 ** 2D Vectors
 **/
void BR_PUBLIC_ENTRY      BrVector2Copy(br_vector2 *v1, const br_vector2 *v2);
/**
 * \brief Set a vector with a pair of scalars.
 *
 * \param v1 A pointer to the destination vector.
 * \param s1 The first vector element (x axis component).
 * \param s2 The second vector element (y axis component).
 *
 * \par Example
 * \code{.c}
 * br_vector2 *va;
 *  ...
 *  BrVector2Set(va,BR_SCALAR(1.0),BR_SCALAR(-1.0));
 * \endcode
 */
void BR_PUBLIC_ENTRY      BrVector2Set(br_vector2 *v1, br_scalar s1, br_scalar s2);
/**
 * \brief Set a vector from a pair of standard C integers.
 *
 * \param v1 A pointer to the destination vector.
 * \param i1 The first vector element (x axis component).
 * \param i2 The second vector element (y axis component).
 *
 * \par Example
 * \code{.c}
 * br_vector2 *va;
 *  ...
 *  BrVector2Set(va,1,-1);
 * \endcode
 */
void BR_PUBLIC_ENTRY      BrVector2SetInt(br_vector2 *v1, int i1, int i2);
/**
 * \brief Set a vector from a pair of standard C floating point numbers.
 *
 * \param v1 A pointer to the destination vector.
 * \param f1 The first vector element (x axis component).
 * \param f2 The second vector element (y axis component).
 *
 * \par Example
 * \code{.c}
 * br_vector2 *va;
 *  ...
 *  BrVector2Set(va,1.f,-1.f);
 * \endcode
 */
void BR_PUBLIC_ENTRY      BrVector2SetFloat(br_vector2 *v1, float f1, float f2);
/**
 * \brief Negate a vector and place the result in a second destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow -V_2\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 */
void BR_PUBLIC_ENTRY      BrVector2Negate(br_vector2 *v1, const br_vector2 *v2);
/**
 * \brief Add two vectors and place the result in a third destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_2 + V_3\f$
 *
 * \param v1 A pointer to the destination vector (may be same as either source).
 * \param v2 A pointer to the first vector of the sum.
 * \param v3 A pointer to the second vector of the sum.
 */
void BR_PUBLIC_ENTRY      BrVector2Add(br_vector2 *v1, const br_vector2 *v2, const br_vector2 *v3);
/**
 * \brief Add one vector to another.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_1 + V_2\f$
 *
 * \param v1 A pointer to the accumulating vector (may be same as v2).
 * \param v2 A pointer to the vector to add.
 */
void BR_PUBLIC_ENTRY      BrVector2Accumulate(br_vector2 *v1, const br_vector2 *v2);
void BR_PUBLIC_ENTRY      BrVector2AccumulateScale(br_vector2 *v1, const br_vector2 *v2, br_scalar s);
/**
 * \brief Subtract one vector from another and place the result in a third destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_2 - V_3\f$
 *
 * \param v1 A pointer to the destination vector (may be same as v1 or v2).
 * \param v2 A pointer to the additive vector.
 * \param v3 A pointer to the subtractive vector.
 */
void BR_PUBLIC_ENTRY      BrVector2Sub(br_vector2 *v1, const br_vector2 *v2, const br_vector2 *v3);
/**
 * \brief Scale a vector by a scalar and place the result in a destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow sV_2\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 * \param s  Scale factor.
 */
void BR_PUBLIC_ENTRY      BrVector2Scale(br_vector2 *v1, const br_vector2 *v2, br_scalar s);
/**
 * \brief Scale a vector by the reciprocal of a scalar and place the result in a destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow s^{-1} V_2\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 * \param s  Reciprocal scale factor.
 */
void BR_PUBLIC_ENTRY      BrVector2InvScale(br_vector2 *v1, const br_vector2 *v2, br_scalar s);
/**
 * \brief Calculate the dot product of two vectors.
 *
 * Equivalent to the expression: \f$V_1 \cdot V_2\f$
 *
 * \param v1 Pointer to left hand vector (may be same as v2).
 * \param v2 Pointers to right hand vector.
 *
 * \return Returns the dot product of the two source vectors. Equivalent to: \f$(x_1\ y_1) \cdot (x_2\ y_2) \equiv x_1 x_2 + y_1 y_2\f$
 */
br_scalar BR_PUBLIC_ENTRY BrVector2Dot(br_vector2 *v1, const br_vector2 *v2);
/**
 * \brief Calculate the length of a vector.
 *
 * Equivalent to the expression: \f$|V_1|\f$
 *
 * \param v1 A pointer to the source vector.
 *
 * \return Returns the length of the vector. Equivalent to: \f$|(x_1\ y_1)| \equiv \sqrt{x_1^2 + y_1^2}\f$
 *
 * \sa BrVector2LengthSquared()
 */
br_scalar BR_PUBLIC_ENTRY BrVector2Length(br_vector2 *v1);
/**
 * \brief Calculate the squared length of a vector.
 *
 * Equivalent to the expression: \f$|V_1|^{2} \text{ or } V_1 \cdot V_1\f$
 *
 * \param v1 A pointer to the source vector.
 *
 * \return Returns the squared length of the vector. Equivalent to: \f$|(x_1\ y_1)|^{2} \equiv x_1^2 + y_1^2\f$
 *
 * \sa BrVector2Length()
 */
br_scalar BR_PUBLIC_ENTRY BrVector2LengthSquared(br_vector2 *v1);
void BR_PUBLIC_ENTRY      BrVector2Clamp(br_vector2 *v1, const br_vector2 *v2, br_scalar min, br_scalar max);

/**
 ** 3D VECTORS
 **/
void BR_PUBLIC_ENTRY      BrVector3Copy(br_vector3 *v1, const br_vector3 *v2);
/**
 * \brief Set a vector with a triple of scalars.
 *
 * \param v1 A pointer to the destination vector.
 * \param s1 The first vector element (x axis component).
 * \param s2 The second vector element (y axis component).
 * \param s3 The third vector element (z axis component).
 *
 * \par Example
 * \code{.c}
 *  br_vector3 *va;
 *  ...
 *  BrVector3Set(va,BR_SCALAR(1.0),BR_SCALAR(-1.0),BR_SCALAR(2.0));
 * \endcode
 */
void BR_PUBLIC_ENTRY      BrVector3Set(br_vector3 *v1, br_scalar s1, br_scalar s2, br_scalar s3);
/**
 * \brief Set a vector from a triple of standard C integers.
 *
 * \param v1 A pointer to the destination vector.
 * \param i1 The first vector element (x axis component).
 * \param i2 The second vector element (y axis component).
 * \param i3 The third vector element (z axis component).
 *
 * \par Example
 * \code{.c}
 *  br_vector3 *va;
 *  ...
 *  BrVector3Set(va,1,-1,2);
 * \endcode
 */
void BR_PUBLIC_ENTRY      BrVector3SetInt(br_vector3 *v1, int i1, int i2, int i3);
/**
 * \brief Set a vector from a triple of standard C floating point numbers.
 *
 * \param v1 A pointer to the destination vector.
 * \param f1 The first vector element (x axis component).
 * \param f2 The second vector element (y axis component).
 * \param f3 The third vector element (z axis component).
 *
 * \par Example
 * \code{.c}
 *  br_vector3 *va;
 *  ...
 *  BrVector3Set(va,1.f,-1.f,1.5f);
 * \endcode
 */
void BR_PUBLIC_ENTRY      BrVector3SetFloat(br_vector3 *v1, float f1, float f2, float f3);
/**
 * \brief Negate a vector and place the result in a second destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow -V_2\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 */
void BR_PUBLIC_ENTRY      BrVector3Negate(br_vector3 *v1, const br_vector3 *v2);
/**
 * \brief Add two vectors and place the result in a third destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_2 + V_3\f$
 *
 * \param v1 A pointer to the destination vector (may be same as either source).
 * \param v2 A pointer to the first vector of the sum.
 * \param v3 A pointer to the second vector of the sum.
 */
void BR_PUBLIC_ENTRY      BrVector3Add(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3);
/**
 * \brief Add one vector to another.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_1 + V_2\f$
 *
 * \param v1 A pointer to the accumulating vector (may be same as v2).
 * \param v2 A pointer to the vector to add.
 */
void BR_PUBLIC_ENTRY      BrVector3Accumulate(br_vector3 *v1, const br_vector3 *v2);
void BR_PUBLIC_ENTRY      BrVector3AccumulateF(br_vector3 *v1, const br_fvector3 *v2);
void BR_PUBLIC_ENTRY      BrVector3AccumulateScale(br_vector3 *v1, const br_vector3 *v2, br_scalar s);
/**
 * \brief Subtract one vector from another and place the result in a third destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_2 - V_3\f$
 *
 * \param v1 A pointer to the destination vector (may be same as v2 or v3).
 * \param v2 A pointer to the additive vector.
 * \param v3 A pointer to the subtractive vector.
 */
void BR_PUBLIC_ENTRY      BrVector3Sub(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3);
void BR_PUBLIC_ENTRY      BrVector3Mul(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3);
/**
 * \brief Scale a vector by a scalar and place the result in a destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow sV_2\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 * \param s  Scale factor.
 */
void BR_PUBLIC_ENTRY      BrVector3Scale(br_vector3 *v1, const br_vector3 *v2, br_scalar s);
/**
 * \brief Scale a vector by the reciprocal of a scalar and place the result in a destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow s^{-1} V_2\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 * \param s  Reciprocal scale factor.
 */
void BR_PUBLIC_ENTRY      BrVector3InvScale(br_vector3 *v1, const br_vector3 *v2, br_scalar s);
/**
 * \brief Calculate the dot product of two vectors.
 *
 * Equivalent to the expression: \f$V_1 \cdot V_2\f$
 *
 * \param v1 Pointer to left hand vector (may be same as v2).
 * \param v2 Pointers to right hand vector (may be same as v1).
 *
 * \return Returns the dot product of the two source vectors. Equivalent to: \f$(x_1\ y_1\ z_1) \cdot (x_2\ y_2\ z_2) \equiv x_1 x_2 + y_1 y_2 + z_1 z_2\f$
 */
br_scalar BR_PUBLIC_ENTRY BrVector3Dot(const br_vector3 *v1, const br_vector3 *v2);
/**
 * \brief Calculate the cross product of two vectors and store the result in a destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_2 \times V_3\f$
 *
 * \param v1 Pointer to destination vector (must be different from both v1 and v2).
 * \param v2 Pointer to left hand vector.
 * \param v3 Pointers to right hand vector.
 *
 * \remark The cross product of the two source vectors is equivalent to: \f$(x_2\ y_2\ z_2) \times (x_3\ y_3\ z_3) \equiv (y_2 z_3 - z_2 y_3 \quad z_2 x_3 - x_2 z_3 \quad x_2 y_3 - y_2 x_3)\f$
 */
void BR_PUBLIC_ENTRY      BrVector3Cross(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3);
/**
 * \brief Calculate the length of a vector.
 *
 * Equivalent to the expression: \f$|V_1|\f$
 *
 * \param v1 A pointer to the source vector.
 *
 * \return Returns the length of the vector. Equivalent to: \f$|(x_1\ y_1\ z_1)| \equiv \sqrt{x_1^2 + y_1^2 + z_1^2}\f$
 *
 * \sa BrVector3LengthSquared()
 */
br_scalar BR_PUBLIC_ENTRY BrVector3Length(br_vector3 *v1);
/**
 * \brief Calculate the squared length of a vector.
 *
 * Equivalent to the expression: \f$|V_1|^{2} \text{ or } V_1 \cdot V_1\f$
 *
 * \param v1 A pointer to the source vector.
 *
 * \return Returns the squared length of the vector. Equivalent to: \f$|(x_1\ y_1\ z_1)|^{2} \equiv x_1^2 + y_1^2 + z_1^2\f$
 *
 * \sa BrVector3Length()
 */
br_scalar BR_PUBLIC_ENTRY BrVector3LengthSquared(br_vector3 *v1);

br_boolean BR_PUBLIC_ENTRY BrVector3Normalise0(br_vector3 *v1, const br_vector3 *v2);
/**
 * \brief Normalise a vector and place the result in a destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_2 / |V_2|\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 *
 * \remark If the source vector's length is zero* the unit vector along the x axis is stored at the
 *         destination instead. (footnote: * Or, in the fixed point library, too close to zero, i.e.
 *         less than or equal to 2*BR_SCALAR_EPSILON.)
 */
void BR_PUBLIC_ENTRY       BrVector3Normalise(br_vector3 *v1, const br_vector3 *v2);
/**
 * \brief Normalise a vector with non-zero length and place the result in a destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_2 / |V_2|\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 *
 * \remark No check made for zero length, hence quicker.
 */
void BR_PUBLIC_ENTRY       BrVector3NormaliseQuick(br_vector3 *v1, const br_vector3 *v2);
/**
 * \brief Normalise a vector with non-zero length using low precision* arithmetic and place the
 *        result in a destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow |V_2|^{-1} V_2\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 *
 * \remark No check is made for zero length. The destination vector will be left unchanged if the
 *         reciprocal of length of the vector is zero. (footnote: * Thus LP in the function name –
 *         do not be confused with the use of this mnemonic for 'length preserving'.)
 */
void BR_PUBLIC_ENTRY       BrVector3NormaliseLP(br_vector3 *v1, const br_vector3 *v2);
void BR_PUBLIC_ENTRY       BrVector3ColourSet(br_vector3 *v1, br_colour colour);
void BR_PUBLIC_ENTRY       BrVector3Clamp(br_vector3 *v1, const br_vector3 *v2, br_scalar min, br_scalar max);
void BR_PUBLIC_ENTRY       BrVector3Lerp(br_vector3 *r, const br_vector3 *a, const br_vector3 *b, br_scalar t);

br_int_32 BR_PUBLIC_ENTRY BrPlaneEquation(br_vector4 *eqn, const br_vector3 *v0, const br_vector3 *v1, const br_vector3 *v2);

/*
 * 2D vectors
 */
br_boolean BR_PUBLIC_ENTRY BrVector2Normalise0(br_vector2 *v1, const br_vector2 *v2);
/**
 * \brief Normalise a vector and place the result in a destination vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_2/|V_2|\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 *
 * \remark If the source vector's length is zero* the unit vector along the x axis is stored at the
 *         destination instead.
 */
void BR_PUBLIC_ENTRY       BrVector2Normalise(br_vector2 *v1, const br_vector2 *v2);

/**
 ** 4D Vectors
 **/
void BR_PUBLIC_ENTRY       BrVector4Set(br_vector4 *v1, br_scalar s1, br_scalar s2, br_scalar s3, br_scalar s4);
void BR_PUBLIC_ENTRY       BrVector4ColourSet(br_vector4 *v1, br_colour colour);
/**
 * \brief Calculate the dot product of two vectors.
 *
 * Equivalent to the expression: \f$V_1 \cdot V_2\f$
 *
 * \param v1 Pointer to left hand vector (may be same as source).
 * \param v2 Pointers to right hand vector.
 *
 * \return Returns the dot product of the two source vectors. Equivalent to: \f$(x_1\ y_1\ z_1\ w_1) \cdot (x_2\ y_2\ z_2\ w_2) \equiv x_1 x_2 + y_1 y_2 + z_1 z_2 + w_1 w_2\f$
 */
br_scalar BR_PUBLIC_ENTRY  BrVector4Dot(const br_vector4 *v1, const br_vector4 *v2);
/**
 * \brief Copy a vector.
 *
 * Equivalent to the expression: \f$V_1 \Leftarrow V_2\f$
 *
 * \param v1 A pointer to the destination vector (may be same as source).
 * \param v2 A pointer to the source vector.
 */
void BR_PUBLIC_ENTRY       BrVector4Copy(br_vector4 *v1, const br_vector4 *v2);
void BR_PUBLIC_ENTRY       BrVector4Copy3(br_vector4 *v1, const br_vector3 *v2, br_scalar w);
void BR_PUBLIC_ENTRY       BrVector4Negate(br_vector4 *v1, const br_vector4 *v2);
void BR_PUBLIC_ENTRY       BrVector4Scale(br_vector4 *v1, const br_vector4 *v2, br_scalar s);
br_boolean BR_PUBLIC_ENTRY BrVector4Normalise0(br_vector4 *v1, const br_vector4 *v2);
void BR_PUBLIC_ENTRY       BrVector4Normalise(br_vector4 *v1, const br_vector4 *v2);
void BR_PUBLIC_ENTRY       BrVector4Accumulate(br_vector4 *v1, const br_vector4 *v2);
void BR_PUBLIC_ENTRY       BrVector4AccumulateScale(br_vector4 *v1, const br_vector4 *v2, br_scalar s);
void BR_PUBLIC_ENTRY       BrVector4Clamp(br_vector4 *v1, const br_vector4 *v2, br_scalar min, br_scalar max);

/*
 * Euler Angles
 */
/**
 * \brief Convert a br_euler to a 3D affine matrix, that would have the same transformational
 *        effect.
 *
 * \param mat   A pointer to the destination matrix to receive the conversion.
 * \param euler A pointer to the source Euler angle.
 *
 * \return Returns mat for convenience.
 */
br_matrix34 *BR_PUBLIC_ENTRY BrEulerToMatrix34(br_matrix34 *mat, const br_euler *euler);
/**
 * \brief Convert a 3D affine matrix to a Euler angle set, that would have the same rotational
 *        effect.
 *
 * \param euler A pointer to the destination Euler angle set to receive the conversion. The Euler
 *              angle set's Euler order is used to determine each angle.
 * \param mat   A pointer to the source matrix to convert from.
 *
 * \return Returns euler for convenience.
 *
 * \remark Translation components of the matrix are lost in conversion.
 */
br_euler *BR_PUBLIC_ENTRY    BrMatrix34ToEuler(br_euler *euler, const br_matrix34 *mat);

/**
 * \brief Convert a br_euler to a 3D affine matrix, that would have the same transformational
 *        effect.
 *
 * \param mat A pointer to the destination matrix to receive the conversion.
 * \param src A pointer to the source Euler angle.
 *
 * \return Returns mat for convenience.
 *
 * \remark Equivalent to BrEulerToMatrix34().
 */
br_matrix4 *BR_PUBLIC_ENTRY BrEulerToMatrix4(br_matrix4 *mat, const br_euler *src);
/**
 * \brief Convert a 3D affine matrix to a Euler angle set, that would have the same rotational
 *        effect.
 *
 * \param dest A pointer to the destination Euler angle set to receive the conversion. The Euler
 *             angle set's Euler order is used to determine each angle.
 * \param mat  A pointer to the source matrix to convert from.
 *
 * \return Returns euler for convenience.
 *
 * \remark Translation and projective (fourth column) components of the matrix are lost in
 *         conversion.
 */
br_euler *BR_PUBLIC_ENTRY   BrMatrix4ToEuler(br_euler *dest, const br_matrix4 *mat);

/**
 * \brief Convert a br_euler to a quaternion, that would have the same transformational effect.
 *
 * \param quat  A pointer to the destination quaternion to receive the conversion.
 * \param euler A pointer to the source Euler angle.
 *
 * \return Returns q for convenience.
 */
br_quat *BR_PUBLIC_ENTRY  BrEulerToQuat(br_quat *quat, const br_euler *euler);
/**
 * \brief Convert a unit quaternion to an Euler angle set, that would have the same transformational
 *        effect.
 *
 * \param euler A pointer to the destination Euler angle set to receive the conversion.
 * \param quat  A pointer to the source unit quaternion.
 *
 * \return Returns euler for convenience.
 */
br_euler *BR_PUBLIC_ENTRY BrQuatToEuler(br_euler *euler, const br_quat *quat);

/*
 * Quaternions
 */
/**
 * \brief Multiply two quaternions.
 *
 * \param q A pointer to the destination quaternion (may be same as either source).
 * \param l A pointer to the left hand source quaternion.
 * \param r A pointer to the right hand source quaternion.
 *
 * \post The resultant quaternion is computed as follows:
 * \f[
 * (w_l + x_l i + y_l j + z_l k)(w_r + x_r i + y_r j + z_r k) \equiv w_l w_r - x_l x_r - y_l y_r - z_l z_r + (w_l x_r + x_l w_r + y_l z_r - z_l y_r)i + (w_l y_r + y_l w_r + z_l x_r - x_l z_r)j + (w_l z_r + z_l w_r + x_l y_r - y_l x_r)k
 * \f]
 *
 * \return The destination quaternion pointer is returned as supplied for convenience.
 *
 * \remark Quaternion multiplication has the effect of concatenating the individual transformations
 *         that each represents. It is not commutative.
 */
br_quat *BR_PUBLIC_ENTRY BrQuatMul(br_quat *q, const br_quat *l, const br_quat *r);
/**
 * \brief Normalise a quaternion.
 *
 * \param q  A pointer to the destination quaternion (may be same as source).
 * \param qq A pointer to the source quaternion.
 *
 * \post The destination is each element of the source divided by its magnitude.
 *
 * \remark BRender assumes that only unit quaternions are used, and due to this assumption being
 *         embodied in certain calculations, if non-unit quaternions are used it is likely that
 *         unwanted side effects such as scaling will be introduced. This function ensures that the
 *         quaternion is a unit one, and is intended to be applied to unit quaternions that have
 *         resulted from a large number of intermediate calculations and may have drifted (due to
 *         precision limitations) from unit magnitude.
 */
br_quat *BR_PUBLIC_ENTRY BrQuatNormalise(br_quat *q, const br_quat *qq);
/**
 * \brief Obtain the inverse of a unit quaternion.
 *
 * This is effectively the rotation reversed (about the same vector component).
 *
 * \param q  A pointer to the destination quaternion (can be same as source).
 * \param qq A pointer to the source quaternion.
 *
 * \post The resultant quaternion is computed as follows: \f$(w + xi + yj + zk)^{-1} \equiv w - xi - yj - zk\f$
 *
 * \return The destination quaternion pointer is returned as supplied for convenience.
 */
br_quat *BR_PUBLIC_ENTRY BrQuatInvert(br_quat *q, const br_quat *qq);

/**
 * \brief The 'Slerp' operation (spherical, linear interpolation) interpolates linearly between two
 *        quaternions along the most direct path between the two orientations.
 *
 * \param q     A pointer to the destination quaternion (can be same as either source).
 * \param l     A pointer to the starting quaternion (equivalent to result with t=0).
 * \param r     A pointer to the finishing quaternion (equivalent to result with t=1).
 * \param t     Interpolation parameter in the range [0,1].
 * \param spins Number of extra spins in the interpolated path. A special value of zero causes
 *              interpolation along the shortest path. A special value of -1 causes interpolation
 *              along the longest path.
 *
 * \post Computes the quaternion between l and r corresponding to position t along the interpolated
 *       path between them.
 *
 * \return The destination quaternion pointer is returned as supplied for convenience.
 */
br_quat *BR_PUBLIC_ENTRY BrQuatSlerp(br_quat *q, const br_quat *l, const br_quat *r, br_scalar t, br_int_16 spins);

/**
 * \brief Convert a unit quaternion to a 3D affine matrix, that would have the same transformational
 *        effect.
 *
 * \param mat A pointer to the destination matrix to receive the conversion.
 * \param q   A pointer to the source unit quaternion.
 *
 * \return Returns mat for convenience.
 *
 * \remark The resulting matrix at mat is equivalent to the following:
 * \f[
 * w + xi + yj + zk \equiv \begin{pmatrix}
 * 1 - 4(y^2 + z^2) & 4(xy + wz) & 4(xz - wy) & 0 \\
 * 4(xy - wz) & 1 - 4(x^2 + z^2) & 4(yz + wx) & 0 \\
 * 4(xz + wy) & 4(yz - wx) & 1 - 4(x^2 + y^2) & 0 \\
 * 0 & 0 & 0 & 1
 * \end{pmatrix}
 * \f]
 */
br_matrix34 *BR_PUBLIC_ENTRY BrQuatToMatrix34(br_matrix34 *mat, const br_quat *q);
/**
 * \brief Convert a 3D affine matrix to a quaternion, that would have the same rotational effect.
 *
 * \param q   A pointer to the destination quaternion to receive the conversion.
 * \param mat A pointer to the source matrix to convert from.
 *
 * \return Returns q for convenience.
 *
 * \remark Translation components of the matrix are lost in conversion.
 */
br_quat *BR_PUBLIC_ENTRY     BrMatrix34ToQuat(br_quat *q, const br_matrix34 *mat);

/**
 * \brief Convert a unit quaternion to a 3D affine matrix, that would have the same transformational
 *        effect.
 *
 * \param mat A pointer to the destination matrix to receive the conversion.
 * \param q   A pointer to the source unit quaternion.
 *
 * \return Returns mat for convenience.
 *
 * \sa BrQuatToMatrix34()
 */
br_matrix4 *BR_PUBLIC_ENTRY BrQuatToMatrix4(br_matrix4 *mat, const br_quat *q);
/**
 * \brief Convert a 3D affine matrix to a quaternion, that would have the same rotational effect.
 *
 * \param q   A pointer to the destination quaternion to receive the conversion.
 * \param mat A pointer to the source matrix to convert from.
 *
 * \return Returns q for convenience.
 *
 * \remark Translation and projective (fourth column) components of the matrix are lost in
 *         conversion.
 */
br_quat *BR_PUBLIC_ENTRY    BrMatrix4ToQuat(br_quat *q, const br_matrix4 *mat);

/*
 * Transforms
 */
/**
 * \brief Convert a generic transform to a 3D affine matrix, that would have the same
 *        transformational effect.
 *
 * \param mat   A pointer to the destination matrix to receive the conversion.
 * \param xform A pointer to the source generic transform.
 *
 * \post When the transform is the identity Uses BrMatrix34Identity() to re-initialise the
 *       destination matrix to the identity matrix. When the transform is a translation Uses
 *       BrMatrix34Translate() to re-initialise the destination matrix to a translation matrix. When
 *       the transform is a Euler angle set Calls BrEulerToMatrix34() and copies the transforms'
 *       translation to the respective elements of the destination matrix. When the transform is a
 *       Look-Up The normalised and negated Look vector provides the third row of the matrix. The
 *       normalised cross product of this with the Up vector provides the first row. The cross
 *       product of these two rows provides the second row. The translation vector provides the
 *       fourth and last row. When the transform is a quaternion Calls BrQuatToMatrix34() and copies
 *       the transforms' translation to the respective elements of the destination matrix. When the
 *       transform is a 3x4 matrix Directly copies the matrix from the source transform. When the
 *       transform is a 3x4 length preserving matrix Directly copies the matrix from the source
 *       transform.
 */
void BR_PUBLIC_ENTRY BrTransformToMatrix34(br_matrix34 *mat, const br_transform *xform);
/**
 * \brief Convert a 3D affine matrix into a specific transform, that would have a similar
 *        transformational effect.
 *
 * \param xform A pointer to the destination transform. The type member of the destination transform
 *              is retained and determines the method of conversion.
 * \param mat   A pointer to the source matrix to be converted.
 *
 * \post When the transform is the identity: the destination transform is left unchanged – no
 *       conversion necessary. When the transform is a translation: the translation component of the
 *       matrix (its bottom row) is copied into the translation vector of the transform. When the
 *       transform is a Euler angle set: calls BrMatrix34ToEuler() and copies the matrix's
 *       translation component (its bottom row) into the translation vector of the transform. When
 *       the transform is a Look-Up: the Up vector is left unchanged and should really be set before
 *       rather than after this conversion; copies the third row into the Look vector (if zero
 *       (0,0,0) then (0,1,0) is used instead); the matrix's translation component is copied into
 *       the translation vector of the transform. When the transform is a quaternion: calls
 *       BrMatrix34ToQuat() and copies the matrix's translation component (its bottom row) into the
 *       translation vector of the transform. When the transform is a 3x4 matrix: directly copies
 *       the matrix into the transform. When the transform is a 3x4 length preserving matrix:
 *       directly copies the matrix into the transform and then calls BrMatrix34LPNormalise().
 */
void BR_PUBLIC_ENTRY BrMatrix34ToTransform(br_transform *xform, const br_matrix34 *mat);
/**
 * \brief Convert from one transform to another, that would have the same transformational effect
 *        (where possible).
 *
 * \param dest A pointer to the destination generic transform to receive the conversion. The
 *             destination's type member is unchanged.
 * \param src  A pointer to the source generic transform to be converted.
 *
 * \post When the transforms' types are the same The transform structure is copied entire. When the
 *       destination transform is of matrix type Calls BrTransformToMatrix34() then normalises if
 *       necessary. In other cases Converts the transform via an intermediate 3x4 matrix by first
 *       calling BrTransformToMatrix34() and then BrMatrix34ToTransform().
 *
 * \remark The transformation type in the destination transform must be set before conversion is
 *         performed*. In some cases it may not be possible to preserve all components of a
 *         transformation across the conversion. For example, a conversion of a matrix to a
 *         quaternion would lose any scaling or shearing components. * This also applies to the
 *         order member of Euler transforms.
 */
void BR_PUBLIC_ENTRY BrTransformToTransform(br_transform *dest, const br_transform *src);

/**
 * \brief Pre-multiply a matrix by a generic transform.
 *
 * Equivalent to the expression: \f$M \Leftarrow M_T M\f$
 *
 * \param mat   A pointer to the subject matrix.
 * \param xform The pre-multiplying generic transform.
 *
 * \post The transform is first converted to a general 3x4 transform matrix using
 *       BrTransformToMatrix34() and then applied as a pre-multiplying matrix using BrMatrix34Pre().
 *
 * \sa BrMatrix34PostTransform().
 */
void BR_PUBLIC_ENTRY BrMatrix34PreTransform(br_matrix34 *mat, const br_transform *xform);
/**
 * \brief Post-multiply a matrix by a generic transform.
 *
 * Equivalent to the expression: \f$M \Leftarrow M M_T\f$
 *
 * \param mat   A pointer to the subject matrix.
 * \param xform The post-multiplying generic transform.
 *
 * \post The transform is first converted to a general 3x4 transform matrix using
 *       BrTransformToMatrix34() and then applied as a post-multiplying matrix using
 *       BrMatrix34Post().
 *
 * \sa BrMatrix34PreTransform()
 */
void BR_PUBLIC_ENTRY BrMatrix34PostTransform(br_matrix34 *mat, const br_transform *xform);
/**
 * \brief Pre-multiply a matrix by a generic transform.
 *
 * Equivalent to the expression: \f$M \Leftarrow M_T M\f$
 *
 * \param mat   A pointer to the subject matrix.
 * \param xform The pre-multiplying generic transform.
 *
 * \post The transform is first converted to a general 3x4 transform matrix using
 *       BrTransformToMatrix34() and then applied as a pre-multiplying matrix using
 *       BrMatrix4Pre34().
 */
void BR_PUBLIC_ENTRY BrMatrix4PreTransform(br_matrix4 *mat, const br_transform *xform);

/*
 * 2x3 Matrix ops.
 */
/**
 * \brief Copy a matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow B\f$
 *
 * \param A A pointer to the destination matrix (may be the same as source – though redundant).
 * \param b A pointer to the source matrix.
 */
void BR_PUBLIC_ENTRY BrMatrix23Copy(br_matrix23 *A, const br_matrix23 *b);
/**
 * \brief Multiply two matrices together and place the result in a third matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow BC\f$
 *
 * \param A A pointer to the destination matrix (must be different from both sources).
 * \param B Pointer to the left hand source matrix.
 * \param C Pointer to the right hand source matrix.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} b_{00} & b_{01} & 0 \\ b_{10} & b_{11} & 0 \\ b_{20} & b_{21} & 1 \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & 0 \\ c_{10} & c_{11} & 0 \\ c_{20} & c_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} b_{00}c_{00} + b_{01}c_{10} & b_{00}c_{01} + b_{01}c_{11} & 0 \\ b_{10}c_{00} + b_{11}c_{10} & b_{10}c_{01} + b_{11}c_{11} & 0 \\ b_{20}c_{00} + b_{21}c_{10} + c_{20} & b_{20}c_{01} + b_{21}c_{11} + c_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23Pre(), BrMatrix23Post()
 */
void BR_PUBLIC_ENTRY BrMatrix23Mul(br_matrix23 *A, const br_matrix23 *B, const br_matrix23 *C);
/**
 * \brief Pre-multiply one matrix by another.
 *
 * Equivalent to the expression: \f$A \Leftarrow BA\f$
 *
 * \param mat A pointer to the subject matrix (may be same as B).
 * \param A   A pointer to the pre-multiplying matrix.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} b_{00} & b_{01} & 0 \\ b_{10} & b_{11} & 0 \\ b_{20} & b_{21} & 1 \end{pmatrix}
 * \begin{pmatrix} a_{00} & a_{01} & 0 \\ a_{10} & a_{11} & 0 \\ a_{20} & a_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} b_{00}a_{00} + b_{01}a_{10} & b_{00}a_{01} + b_{01}a_{11} & 0 \\ b_{10}a_{00} + b_{11}a_{10} & b_{10}a_{01} + b_{11}a_{11} & 0 \\ b_{20}a_{00} + b_{21}a_{10} + a_{20} & b_{20}a_{01} + b_{21}a_{11} + a_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23Post(), BrMatrix23Mul()
 */
void BR_PUBLIC_ENTRY BrMatrix23Pre(br_matrix23 *mat, const br_matrix23 *A);
/**
 * \brief Post-multiply one matrix by another.
 *
 * Equivalent to the expression: \f$A \Leftarrow AB\f$
 *
 * \param mat A pointer to the subject matrix (may be same as B).
 * \param A   A pointer to the post-multiplying matrix.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} a_{00} & a_{01} & 0 \\ a_{10} & a_{11} & 0 \\ a_{20} & a_{21} & 1 \end{pmatrix}
 * \begin{pmatrix} b_{00} & b_{01} & 0 \\ b_{10} & b_{11} & 0 \\ b_{20} & b_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} a_{00}b_{00} + a_{01}b_{10} & a_{00}b_{01} + a_{01}b_{11} & 0 \\ a_{10}b_{00} + a_{11}b_{10} & a_{10}b_{01} + a_{11}b_{11} & 0 \\ a_{20}b_{00} + a_{21}b_{10} + b_{20} & a_{20}b_{01} + a_{21}b_{11} + b_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23Pre(), BrMatrix23Mul()
 */
void BR_PUBLIC_ENTRY BrMatrix23Post(br_matrix23 *mat, const br_matrix23 *A);

/**
 * \brief Set the specified matrix to the identity transformation matrix.
 *
 * Equivalent to: \f$M \Leftarrow I \equiv \begin{pmatrix} 1 & 0 & 0 \\ 0 & 1 & 0 \\ 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 *
 * \post Stores the identity matrix at the destination.
 */
void BR_PUBLIC_ENTRY BrMatrix23Identity(br_matrix23 *mat);

/**
 * \brief Set the specified matrix to a matrix representing a rotation about the z axis though a
 *        specified angle.
 *
 * Equivalent to: \f$M \Leftarrow R_{\theta Z} \equiv \begin{pmatrix} \cos\theta_Z & \sin\theta_Z & 0 \\ -\sin\theta_Z & \cos\theta_Z & 0 \\ 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param rz  Clockwise rotation about the z axis.
 *
 * \sa BrMatrix23PreRotate(), BrMatrix23PostRotate()
 */
void BR_PUBLIC_ENTRY BrMatrix23Rotate(br_matrix23 *mat, br_angle rz);
/**
 * \brief Pre-multiply a matrix by a rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow R_{\theta Z} M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param rz  The clockwise angle about the z axis used to form the rotation matrix.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} \cos\theta_Z & \sin\theta_Z & 0 \\ -\sin\theta_Z & \cos\theta_Z & 0 \\ 0 & 0 & 1 \end{pmatrix}
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} m_{00}\cos\theta_Z + m_{10}\sin\theta_Z & m_{01}\cos\theta_Z + m_{11}\sin\theta_Z & 0 \\ -m_{00}\sin\theta_Z + m_{10}\cos\theta_Z & -m_{01}\sin\theta_Z + m_{11}\cos\theta_Z & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PostRotate(), BrMatrix23Rotate()
 */
void BR_PUBLIC_ENTRY BrMatrix23PreRotate(br_matrix23 *mat, br_angle rz);
/**
 * \brief Post-multiply a matrix by a rotational transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MR_{\theta Z}\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param rz  The clockwise angle about the z axis used to form the rotation matrix.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \begin{pmatrix} \cos\theta_Z & \sin\theta_Z & 0 \\ -\sin\theta_Z & \cos\theta_Z & 0 \\ 0 & 0 & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} m_{00}\cos\theta_Z - m_{01}\sin\theta_Z & m_{00}\sin\theta_Z + m_{01}\cos\theta_Z & 0 \\ m_{10}\cos\theta_Z - m_{11}\sin\theta_Z & m_{10}\sin\theta_Z + m_{11}\cos\theta_Z & 0 \\ m_{20}\cos\theta_Z - m_{21}\sin\theta_Z & m_{20}\sin\theta_Z + m_{21}\cos\theta_Z & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PreRotate(), BrMatrix23Rotate()
 */
void BR_PUBLIC_ENTRY BrMatrix23PostRotate(br_matrix23 *mat, br_angle rz);

/**
 * \brief Set the specified matrix to a matrix representing a specific translation.
 *
 * Equivalent to: \f$M \Leftarrow T_{xy} \equiv \begin{pmatrix} 1 & 0 & 0 \\ 0 & 1 & 0 \\ d_x & d_y & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param x   Translation component along the x axis.
 * \param y   Translation component along the y axis.
 *
 * \sa BrMatrix23PreTranslate(), BrMatrix23PostTranslate()
 */
void BR_PUBLIC_ENTRY BrMatrix23Translate(br_matrix23 *mat, br_scalar x, br_scalar y);
/**
 * \brief Pre-multiply a matrix by a translation transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow T_{xy} M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param x   The x axis component used to form the translation matrix.
 * \param y   The y axis component used to form the translation matrix.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} 1 & 0 & 0 \\ 0 & 1 & 0 \\ d_x & d_y & 1 \end{pmatrix}
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ d_x m_{00} + d_y m_{10} + m_{20} & d_x m_{01} + d_y m_{11} + m_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PostTranslate(), BrMatrix23Translate()
 */
void BR_PUBLIC_ENTRY BrMatrix23PreTranslate(br_matrix23 *mat, br_scalar x, br_scalar y);
/**
 * \brief Post-multiply a matrix by a translation transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MT_{xy}\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param x   The x axis component used to form the translation matrix.
 * \param y   The y axis component used to form the translation matrix.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \begin{pmatrix} 1 & 0 & 0 \\ 0 & 1 & 0 \\ d_x & d_y & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} + d_x & m_{21} + d_y & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PreTranslate(), BrMatrix23Translate()
 */
void BR_PUBLIC_ENTRY BrMatrix23PostTranslate(br_matrix23 *mat, br_scalar x, br_scalar y);

/**
 * \brief Set the specified matrix to a matrix representing a specific scaling.
 *
 * Equivalent to: \f$M \Leftarrow S_{xy} \equiv \begin{pmatrix} s_x & 0 & 0 \\ 0 & s_y & 0 \\ 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param sx  Scaling component along the x axis.
 * \param sy  Scaling component along the y axis.
 *
 * \sa BrMatrix23PreScale(), BrMatrix23PostScale()
 */
void BR_PUBLIC_ENTRY BrMatrix23Scale(br_matrix23 *mat, br_scalar sx, br_scalar sy);
/**
 * \brief Pre-multiply a matrix by a scaling transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow S_{xy} M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Scaling component along the x axis.
 * \param sy  Scaling component along the y axis.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} s_x & 0 & 0 \\ 0 & s_y & 0 \\ 0 & 0 & 1 \end{pmatrix}
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} s_x m_{00} & s_x m_{01} & 0 \\ s_y m_{10} & s_y m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PostScale(), BrMatrix23Scale()
 */
void BR_PUBLIC_ENTRY BrMatrix23PreScale(br_matrix23 *mat, br_scalar sx, br_scalar sy);
/**
 * \brief Post-multiply a matrix by a scaling transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MS_{xy}\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Scaling component along the x axis.
 * \param sy  Scaling component along the y axis.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \begin{pmatrix} s_x & 0 & 0 \\ 0 & s_y & 0 \\ 0 & 0 & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} m_{00}s_x & m_{01}s_y & 0 \\ m_{10}s_x & m_{11}s_y & 0 \\ m_{20}s_x & m_{21}s_y & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PreScale(), BrMatrix23Scale()
 */
void BR_PUBLIC_ENTRY BrMatrix23PostScale(br_matrix23 *mat, br_scalar sx, br_scalar sy);

/**
 * \brief Set the specified matrix to a matrix representing a shear, invariant along the x axis.
 *
 * Thus values of y co-ordinates will be scaled in proportion to the value of the x co-ordinate.
 * Equivalent to: \f$M \Leftarrow Z_X \equiv \begin{pmatrix} 1 & s_y & 0 \\ 0 & 1 & 0 \\ 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param sy  Shear factor by which the x co-ordinate is included in the transformed y co-ordinate.
 *
 * \sa BrMatrix23PreShearX(), BrMatrix23PostShearX()
 */
void BR_PUBLIC_ENTRY BrMatrix23ShearX(br_matrix23 *mat, br_scalar sy);
/**
 * \brief Pre-multiply a matrix by an x invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow Z_X M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sy  Shear factor by which the x co-ordinate is included in the transformed y co-ordinate.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} 1 & s_y & 0 \\ 0 & 1 & 0 \\ 0 & 0 & 1 \end{pmatrix}
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} m_{00} + s_y m_{10} & m_{01} + s_y m_{11} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PostShearX(), BrMatrix23ShearX()
 */
void BR_PUBLIC_ENTRY BrMatrix23PreShearX(br_matrix23 *mat, br_scalar sy);
/**
 * \brief Post-multiply a matrix by an x invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MZ_X\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sy  Shear factor by which the x co-ordinate is included in the transformed y co-ordinate.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \begin{pmatrix} 1 & s_y & 0 \\ 0 & 1 & 0 \\ 0 & 0 & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} m_{00} & m_{00}s_y + m_{01} & 0 \\ m_{10} & m_{10}s_y + m_{11} & 0 \\ m_{20} & m_{20}s_y + m_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PreShearX(), BrMatrix23ShearX()
 */
void BR_PUBLIC_ENTRY BrMatrix23PostShearX(br_matrix23 *mat, br_scalar sy);

/**
 * \brief Set the specified matrix to a matrix representing a shear, invariant along the y axis.
 *
 * Thus values of x co-ordinates will be scaled in proportion to the value of the y co-ordinate.
 * Equivalent to: \f$M \Leftarrow Z_Y \equiv \begin{pmatrix} 1 & 0 & 0 \\ s_x & 1 & 0 \\ 0 & 0 & 1 \end{pmatrix}\f$
 *
 * \param mat A pointer to the destination matrix.
 * \param sx  Shear factor by which the y co-ordinate is included in the transformed x co-ordinate.
 *
 * \sa BrMatrix23PreShearY(), BrMatrix23PostShearY()
 */
void BR_PUBLIC_ENTRY BrMatrix23ShearY(br_matrix23 *mat, br_scalar sx);
/**
 * \brief Pre-multiply a matrix by a y invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow Z_y M\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Shear factor by which the y co-ordinate is included in the transformed x co-ordinate.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} 1 & 0 & 0 \\ s_x & 1 & 0 \\ 0 & 0 & 1 \end{pmatrix}
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} m_{00} & m_{01} & 0 \\ s_x m_{00} + m_{10} & s_x m_{01} + m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PostShearY(), BrMatrix23ShearY()
 */
void BR_PUBLIC_ENTRY BrMatrix23PreShearY(br_matrix23 *mat, br_scalar sx);
/**
 * \brief Post-multiply a matrix by a y invariant shearing transform matrix.
 *
 * Equivalent to the expression: \f$M \Leftarrow MZ_Y\f$
 *
 * \param mat A pointer to the subject matrix.
 * \param sx  Shear factor by which the y co-ordinate is included in the transformed x co-ordinate.
 *
 * \remark The result in mat is equivalent to the following:
 * \f[
 * \begin{pmatrix} m_{00} & m_{01} & 0 \\ m_{10} & m_{11} & 0 \\ m_{20} & m_{21} & 1 \end{pmatrix}
 * \begin{pmatrix} 1 & 0 & 0 \\ s_x & 1 & 0 \\ 0 & 0 & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} m_{00} + m_{01}s_x & m_{01} & 0 \\ m_{10} + m_{11}s_x & m_{11} & 0 \\ m_{20} + m_{21}s_x & m_{21} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23PreShearY(), BrMatrix23ShearY()
 */
void BR_PUBLIC_ENTRY BrMatrix23PostShearY(br_matrix23 *mat, br_scalar sx);

/**
 * \brief Applies a transform to a 2D vector, i.e. as for a point but without translation components
 *        (a vector has no location).
 *
 * Equivalent to the expression: \f$V_A \Leftarrow V_B C\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed vector.
 * \param B A pointer to the source vector, holding the vector to be transformed.
 * \param C A pointer to the transform matrix to be applied.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} x_B & y_B & 0 \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & 0 \\ c_{10} & c_{11} & 0 \\ c_{20} & c_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} x_B c_{00} + y_B c_{10} & x_B c_{01} + y_B c_{11} & 0 \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix23ApplyV(br_vector2 *A, const br_vector2 *B, const br_matrix23 *C);
/**
 * \brief Applies a transform to a 2D point.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} x_B & y_B & 1 \end{pmatrix}
 * \begin{pmatrix} c_{00} & c_{01} & 0 \\ c_{10} & c_{11} & 0 \\ c_{20} & c_{21} & 1 \end{pmatrix}
 * \equiv \begin{pmatrix} x_B c_{00} + y_B c_{10} + c_{20} & x_B c_{01} + y_B c_{11} + c_{21} & 1 \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix23ApplyP(br_vector2 *A, const br_vector2 *B, const br_matrix23 *C);

/**
 * \brief Applies a transposed transform to a 2D vector, i.e. as for a point but without translation
 *        components (a vector has no location).
 *
 * Equivalent to the expression: \f$V_A \Leftarrow V_B C^{t}\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed vector.
 * \param B A pointer to the source vector, holding the vector to be transformed.
 * \param C A pointer to the transform matrix to be applied transposed – the translation elements
 *          are presumed zero or irrelevant.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} c_{00} & c_{01} & 0 \\ c_{10} & c_{11} & 0 \\ - & - & 1 \end{pmatrix}
 * \begin{pmatrix} x_B \\ y_B \\ 0 \end{pmatrix}
 * \equiv \begin{pmatrix} c_{00}x_B + c_{01}y_B \\ c_{10}x_B + c_{11}y_B \\ 0 \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix23TApplyV(br_vector2 *A, const br_vector2 *B, const br_matrix23 *C);
/**
 * \brief Applies a transposed transform to a 2D point.
 *
 * Equivalent to the expression: \f$P_A \Leftarrow P_B C^{t}\f$
 *
 * \param A A pointer to the destination vector (must be different from source, and not part of
 *          transform), to hold the transformed point.
 * \param B A pointer to the source vector, holding the point to be transformed.
 * \param C A pointer to the transform matrix to be applied transposed – the translation elements
 *          are presumed zero or irrelevant.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} c_{00} & c_{01} & 0 \\ c_{10} & c_{11} & 0 \\ - & - & 1 \end{pmatrix}
 * \begin{pmatrix} x_B \\ y_B \\ 1 \end{pmatrix}
 * \equiv \begin{pmatrix} c_{00}x_B + c_{01}y_B \\ c_{10}x_B + c_{11}y_B \\ 1 \end{pmatrix}
 * \f]
 */
void BR_PUBLIC_ENTRY BrMatrix23TApplyP(br_vector2 *A, const br_vector2 *B, const br_matrix23 *C);

/**
 * \brief Compute the inverse of the supplied matrix.
 *
 * Equivalent to the expression: \f$A \Leftarrow B^{-1}\f$
 *
 * \param out A pointer to the destination matrix (must be different from source).
 * \param in  A pointer to the source matrix.
 *
 * \return If the inverse exists, the determinant of the source matrix is returned. If there is no
 *         inverse, scalar zero is returned.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} b_{00} & b_{01} & 0 \\ b_{10} & b_{11} & 0 \\ b_{20} & b_{21} & 1 \end{pmatrix}^{-1}
 * \equiv \begin{pmatrix} \frac{b_{11}}{|B|} & \frac{-b_{01}}{|B|} & 0 \\ \frac{-b_{10}}{|B|} & \frac{b_{00}}{|B|} & 0 \\ \frac{b_{10}b_{21} - b_{11}b_{20}}{|B|} & \frac{b_{01}b_{20} - b_{00}b_{21}}{|B|} & 1 \end{pmatrix}
 * \f]
 *
 * \sa BrMatrix23LPInverse()
 */
br_scalar BR_PUBLIC_ENTRY BrMatrix23Inverse(br_matrix23 *out, const br_matrix23 *in);
/**
 * \brief Compute the inverse of the supplied length preserving* transformation matrix.
 *
 * The resulting matrix is undefined for non-length preserving matrices. Equivalent to the
 * expression: \f$A_{LP} \Leftarrow B_{LP}^{-1}\f$
 *
 * \param A A pointer to the destination matrix (must be different from source).
 * \param B A pointer to the source matrix.
 *
 * \remark The result in A is equivalent to the following:
 * \f[
 * \begin{pmatrix} b_{00} & b_{01} & 0 \\ b_{10} & b_{11} & 0 \\ b_{20} & b_{21} & 1 \end{pmatrix}_{LP}^{-1}
 * \equiv \begin{pmatrix} b_{11} & -b_{01} & 0 \\ -b_{10} & b_{00} & 0 \\ b_{10}b_{21} - b_{11}b_{20} & b_{01}b_{20} - b_{00}b_{21} & 1 \end{pmatrix}_{LP}
 * \f]
 *
 * \sa BrMatrix23Inverse()
 */
void BR_PUBLIC_ENTRY      BrMatrix23LPInverse(br_matrix23 *A, const br_matrix23 *B);
/**
 * \brief Normalise a length preserving* matrix.
 *
 * Equivalent to the expression: \f$A_{LP} \Leftarrow Norm(B_{LP})\f$
 *
 * \param A A pointer to the destination matrix, which must not point to the source matrix.
 * \param B A pointer to the source matrix.
 *
 * \post The destination matrix is the source matrix adjusted so that it represents a length
 *       preserving transformation.
 *
 * \remark This function is typically applied to a length preserving matrix which has undergone a
 *         long sequence of operations, to ensure that the final matrix is still truly
 *         length-preserving.
 */
void BR_PUBLIC_ENTRY      BrMatrix23LPNormalise(br_matrix23 *A, const br_matrix23 *B);

/*
 * Backwards compatibility
 */
#define BrMatrix34Transform BrTransformToMatrix34
#define BrTransformTransfer BrTransformToTransform

#ifdef __cplusplus
};
#endif
#endif /* _NO_PROTOTYPES */

#endif
