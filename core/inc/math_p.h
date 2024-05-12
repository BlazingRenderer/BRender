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
BR_API br_fixed_ls BrFixedAbs(br_fixed_ls a);

/* result = a*b
 */
BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedMul(br_fixed_ls a, br_fixed_ls b);

/* result = a*b + c*d
 */
BR_API br_fixed_ls BrFixedMac2(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d);

/* result = a*b + c*d + e*f
 */
BR_API br_fixed_ls BrFixedMac3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e, br_fixed_ls f);

/* result = a*b + c*d + e*f + g*h
 */
BR_API br_fixed_ls BrFixedMac4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e, br_fixed_ls f,
                               br_fixed_ls g, br_fixed_ls h);

/* result = a*a
 */
BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr(br_fixed_ls a);

/* result = a*a + b*b
 */
BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr2(br_fixed_ls a, br_fixed_ls b);

/* result = a*a + b*b + c*c
 */
BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c);
/* result = a*a + b*b + c*c + d*d
 */
BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d);

/* result = sqrt(a*a + b*b)
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedLength2(br_fixed_ls a, br_fixed_ls b);

/* result = sqrt(a*a + b*b + c*c)
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedLength3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c);

/* result = sqrt(a*a + b*b + c*c + d*d)
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedLength4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d);

/* result = 1/sqrt(a*a + b*b) (low precision)
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedRLength2(br_fixed_ls a, br_fixed_ls b);

/* result = 1/sqrt(a*a + b*b + c*c) (low precision)
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedRLength3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c);

/* result = 1/sqrt(a*a + b*b + c*c + d*d) (low precision)
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedRLength4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d);

/* result = a/b
 */
BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedDiv(br_fixed_ls a, br_fixed_ls b);

/* result = a/b * 2^31
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedDivF(br_fixed_ls a, br_fixed_ls b);

/* result = a/b (rounded towards 0)
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedDivR(br_fixed_ls a, br_fixed_ls b);

/*
 */
BR_API br_fixed_ls BR_ASM_CALL _FixedDivP(br_fixed_ls a, br_fixed_ls b);

/* result = a*b/c
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedMulDiv(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c);

/* result = (a*b + c*d)/e
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedMac2Div(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e);

/* result = (a*b + c*d + e*f)/g
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedMac3Div(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e,
                                              br_fixed_ls f, br_fixed_ls g);

/* result = (a*b + c*d + e*f + g*h)/i
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedMac4Div(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e,
                                              br_fixed_ls f, br_fixed_ls g, br_fixed_ls h, br_fixed_ls i);
/* result = 1.0/a
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedRcp(br_fixed_ls a);

/*
 * Various combinations with fractions
 */

/* result = a*b + c*d - a & c are fractions
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedFMac2(br_fixed_lsf a, br_fixed_ls b, br_fixed_lsf c, br_fixed_ls d);

/* result = a*b + c*d + e*f - a,c & e are fractions
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedFMac3(br_fixed_lsf a, br_fixed_ls b, br_fixed_lsf c, br_fixed_ls d,
                                            br_fixed_lsf e, br_fixed_ls f);

/* result = a*b + c*d + e*f + g*h (a,c,e,g are fractions)
 */
BR_API br_fixed_ls BR_ASM_CALL BrFixedFMac4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e,
                                            br_fixed_ls f, br_fixed_ls g, br_fixed_ls h);

/*
 * Misc. support  functions
 */
BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedSin(br_fixed_luf a);

BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedCos(br_fixed_luf a);

BR_API br_fixed_luf BR_ASM_CALL BrFixedASin(br_fixed_ls s);

BR_API br_fixed_luf BR_ASM_CALL BrFixedACos(br_fixed_ls c);

BR_API br_fixed_luf BR_PUBLIC_ENTRY BrFixedATan2(br_fixed_ls x, br_fixed_ls y);

BR_API br_fixed_luf BR_ASM_CALL BrFixedATan2Fast(br_fixed_ls x, br_fixed_ls y);

BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqrt(br_fixed_ls a);
BR_API br_fixed_ls BR_PUBLIC_ENTRY BrFixedPow(br_fixed_ls a, br_fixed_ls b);

/*
 * Integer sqrt functions
 */
BR_API br_uint_32 BR_ASM_CALL BrSqrt(br_uint_32 a);
BR_API br_uint_32 BR_ASM_CALL BrFastSqrt(br_uint_32 a);
BR_API br_uint_32 BR_ASM_CALL BrFastRSqrt(br_uint_32 a);

/*
 * 3x4 Matrix ops.
 */
BR_API void BR_PUBLIC_ENTRY BrMatrix34Copy(br_matrix34 *A, const br_matrix34 *b);
BR_API void BR_PUBLIC_ENTRY BrMatrix34Mul(br_matrix34 *A, const br_matrix34 *B, const br_matrix34 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix34Pre(br_matrix34 *mat, const br_matrix34 *A);
BR_API void BR_PUBLIC_ENTRY BrMatrix34Post(br_matrix34 *mat, const br_matrix34 *A);

BR_API void BR_PUBLIC_ENTRY BrMatrix34Identity(br_matrix34 *mat);

BR_API void BR_PUBLIC_ENTRY BrMatrix34RotateX(br_matrix34 *mat, br_angle rx);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PreRotateX(br_matrix34 *mat, br_angle rx);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PostRotateX(br_matrix34 *mat, br_angle rx);

BR_API void BR_PUBLIC_ENTRY BrMatrix34RotateY(br_matrix34 *mat, br_angle ry);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PreRotateY(br_matrix34 *mat, br_angle ry);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PostRotateY(br_matrix34 *mat, br_angle ry);

BR_API void BR_PUBLIC_ENTRY BrMatrix34RotateZ(br_matrix34 *mat, br_angle rz);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PreRotateZ(br_matrix34 *mat, br_angle rz);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PostRotateZ(br_matrix34 *mat, br_angle rz);

BR_API void BR_PUBLIC_ENTRY BrMatrix34Rotate(br_matrix34 *mat, br_angle r, const br_vector3 *axis);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PreRotate(br_matrix34 *mat, br_angle r, const br_vector3 *axis);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PostRotate(br_matrix34 *mat, br_angle r, const br_vector3 *axis);

BR_API void BR_PUBLIC_ENTRY BrMatrix34Translate(br_matrix34 *mat, br_scalar x, br_scalar y, br_scalar z);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PreTranslate(br_matrix34 *mat, br_scalar x, br_scalar y, br_scalar z);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PostTranslate(br_matrix34 *mat, br_scalar x, br_scalar y, br_scalar z);

BR_API void BR_PUBLIC_ENTRY BrMatrix34Scale(br_matrix34 *mat, br_scalar sx, br_scalar sy, br_scalar sz);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PreScale(br_matrix34 *mat, br_scalar sx, br_scalar sy, br_scalar sz);

BR_API void BR_PUBLIC_ENTRY BrMatrix34PostScale(br_matrix34 *mat, br_scalar sx, br_scalar sy, br_scalar sz);

BR_API void BR_PUBLIC_ENTRY BrMatrix34ShearX(br_matrix34 *mat, br_scalar sy, br_scalar sz);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PreShearX(br_matrix34 *mat, br_scalar sy, br_scalar sz);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PostShearX(br_matrix34 *mat, br_scalar sy, br_scalar sz);

BR_API void BR_PUBLIC_ENTRY BrMatrix34ShearY(br_matrix34 *mat, br_scalar sx, br_scalar sz);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PreShearY(br_matrix34 *mat, br_scalar sx, br_scalar sz);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PostShearY(br_matrix34 *mat, br_scalar sx, br_scalar sz);

BR_API void BR_PUBLIC_ENTRY BrMatrix34ShearZ(br_matrix34 *mat, br_scalar sx, br_scalar sy);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PreShearZ(br_matrix34 *mat, br_scalar sx, br_scalar sy);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PostShearZ(br_matrix34 *mat, br_scalar sx, br_scalar sy);

BR_API void BR_PUBLIC_ENTRY BrMatrix34ApplyV(br_vector3 *A, const br_vector3 *B, const br_matrix34 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix34ApplyP(br_vector3 *A, const br_vector3 *B, const br_matrix34 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix34Apply(br_vector3 *A, const br_vector4 *B, const br_matrix34 *C);

BR_API void BR_PUBLIC_ENTRY BrMatrix34TApplyV(br_vector3 *A, const br_vector3 *B, const br_matrix34 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix34TApplyP(br_vector3 *A, const br_vector3 *B, const br_matrix34 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix34TApply(br_vector4 *A, const br_vector4 *B, const br_matrix34 *C);

BR_API br_scalar BR_PUBLIC_ENTRY BrMatrix34Inverse(br_matrix34 *out, const br_matrix34 *in);
BR_API void BR_PUBLIC_ENTRY      BrMatrix34LPInverse(br_matrix34 *A, const br_matrix34 *B);
BR_API void BR_PUBLIC_ENTRY      BrMatrix34LPNormalise(br_matrix34 *A, const br_matrix34 *B);

BR_API void BR_PUBLIC_ENTRY BrMatrix34RollingBall(br_matrix34 *mat, int dx, int dy, int radius);

BR_API br_matrix34 *BR_PUBLIC_ENTRY BrBoundsToMatrix34(br_matrix34 *mat, const br_bounds *bounds);

BR_API void BR_PUBLIC_ENTRY BrMatrix34ApplyBounds(br_bounds *A, const br_bounds *B, const br_matrix34 *C);

BR_API void BR_PUBLIC_ENTRY BrMatrix34ApplyPlaneEquation(br_vector4 *A, const br_vector4 *B, const br_matrix34 *C);

BR_API void BR_PUBLIC_ENTRY BrMatrix34FixedToFloat(br_matrix34_f *A, const br_matrix34_x *B);

/*
 * 4x4 Matrix ops.
 */
BR_API void BR_PUBLIC_ENTRY BrMatrix4Copy(br_matrix4 *A, const br_matrix4 *B);
BR_API void BR_PUBLIC_ENTRY BrMatrix4Mul(br_matrix4 *A, const br_matrix4 *B, const br_matrix4 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix4Identity(br_matrix4 *mat);
BR_API void BR_PUBLIC_ENTRY BrMatrix4Scale(br_matrix4 *mat, br_scalar sx, br_scalar sy, br_scalar sz);

BR_API void BR_PUBLIC_ENTRY      BrMatrix4Transpose(br_matrix4 *A);
BR_API br_scalar BR_PUBLIC_ENTRY BrMatrix4Inverse(br_matrix4 *A, const br_matrix4 *B);
BR_API void BR_PUBLIC_ENTRY      BrMatrix4Adjoint(br_matrix4 *A, const br_matrix4 *B);
BR_API br_scalar BR_PUBLIC_ENTRY BrMatrix4Determinant(const br_matrix4 *mat);

BR_API void BR_PUBLIC_ENTRY BrMatrix4Perspective(br_matrix4 *mat, br_angle field_of_view, br_scalar aspect,
                                                 br_scalar hither, br_scalar yon);

BR_API void BR_PUBLIC_ENTRY BrMatrix4Orthographic(br_matrix4 *mat, br_scalar left, br_scalar right, br_scalar bottom,
                                                  br_scalar top, br_scalar hither, br_scalar yon);

BR_API void BR_PUBLIC_ENTRY BrMatrix4ApplyV(br_vector4 *A, const br_vector3 *B, const br_matrix4 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix4ApplyP(br_vector4 *A, const br_vector3 *B, const br_matrix4 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix4Apply(br_vector4 *A, const br_vector4 *B, const br_matrix4 *C);

BR_API void BR_PUBLIC_ENTRY BrMatrix4TApplyV(br_vector4 *A, const br_vector3 *B, const br_matrix4 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix4TApplyP(br_vector4 *A, const br_vector3 *B, const br_matrix4 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix4TApply(br_vector4 *A, const br_vector4 *B, const br_matrix4 *C);

BR_API void BR_PUBLIC_ENTRY BrMatrix4Copy23(br_matrix4 *A, const br_matrix23 *B);

BR_API void BR_PUBLIC_ENTRY BrMatrix4Pre34(br_matrix4 *A, const br_matrix34 *B);
BR_API void BR_PUBLIC_ENTRY BrMatrix4Mul34(br_matrix4 *A, const br_matrix34 *B, const br_matrix4 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix4Copy34(br_matrix4 *A, const br_matrix34 *B);
BR_API void BR_PUBLIC_ENTRY BrMatrix34Copy4(br_matrix34 *A, const br_matrix4 *B);

/**
 ** 2D Vectors
 **/
BR_API void BR_PUBLIC_ENTRY      BrVector2Copy(br_vector2 *v1, const br_vector2 *v2);
BR_API void BR_PUBLIC_ENTRY      BrVector2Set(br_vector2 *v1, br_scalar s1, br_scalar s2);
BR_API void BR_PUBLIC_ENTRY      BrVector2SetInt(br_vector2 *v1, int i1, int i2);
BR_API void BR_PUBLIC_ENTRY      BrVector2SetFloat(br_vector2 *v1, float f1, float f2);
BR_API void BR_PUBLIC_ENTRY      BrVector2Negate(br_vector2 *v1, const br_vector2 *v2);
BR_API void BR_PUBLIC_ENTRY      BrVector2Add(br_vector2 *v1, const br_vector2 *v2, const br_vector2 *v3);
BR_API void BR_PUBLIC_ENTRY      BrVector2Accumulate(br_vector2 *v1, const br_vector2 *v2);
BR_API void BR_PUBLIC_ENTRY      BrVector2AccumulateScale(br_vector2 *v1, const br_vector2 *v2, br_scalar s);
BR_API void BR_PUBLIC_ENTRY      BrVector2Sub(br_vector2 *v1, const br_vector2 *v2, const br_vector2 *v3);
BR_API void BR_PUBLIC_ENTRY      BrVector2Scale(br_vector2 *v1, const br_vector2 *v2, br_scalar s);
BR_API void BR_PUBLIC_ENTRY      BrVector2InvScale(br_vector2 *v1, const br_vector2 *v2, br_scalar s);
BR_API br_scalar BR_PUBLIC_ENTRY BrVector2Dot(br_vector2 *v1, const br_vector2 *v2);
BR_API br_scalar BR_PUBLIC_ENTRY BrVector2Length(br_vector2 *v1);
BR_API br_scalar BR_PUBLIC_ENTRY BrVector2LengthSquared(br_vector2 *v1);

/**
 ** 3D VECTORS
 **/
BR_API void BR_PUBLIC_ENTRY      BrVector3Copy(br_vector3 *v1, const br_vector3 *v2);
BR_API void BR_PUBLIC_ENTRY      BrVector3Set(br_vector3 *v1, br_scalar s1, br_scalar s2, br_scalar s3);
BR_API void BR_PUBLIC_ENTRY      BrVector3SetInt(br_vector3 *v1, int i1, int i2, int i3);
BR_API void BR_PUBLIC_ENTRY      BrVector3SetFloat(br_vector3 *v1, float f1, float f2, float f3);
BR_API void BR_PUBLIC_ENTRY      BrVector3Negate(br_vector3 *v1, const br_vector3 *v2);
BR_API void BR_PUBLIC_ENTRY      BrVector3Add(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3);
BR_API void BR_PUBLIC_ENTRY      BrVector3Accumulate(br_vector3 *v1, const br_vector3 *v2);
BR_API void BR_PUBLIC_ENTRY      BrVector3AccumulateF(br_vector3 *v1, const br_fvector3 *v2);
BR_API void BR_PUBLIC_ENTRY      BrVector3AccumulateScale(br_vector3 *v1, const br_vector3 *v2, br_scalar s);
BR_API void BR_PUBLIC_ENTRY      BrVector3Sub(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3);
BR_API void BR_PUBLIC_ENTRY      BrVector3Mul(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3);
BR_API void BR_PUBLIC_ENTRY      BrVector3Scale(br_vector3 *v1, const br_vector3 *v2, br_scalar s);
BR_API void BR_PUBLIC_ENTRY      BrVector3InvScale(br_vector3 *v1, const br_vector3 *v2, br_scalar s);
BR_API br_scalar BR_PUBLIC_ENTRY BrVector3Dot(const br_vector3 *v1, const br_vector3 *v2);
BR_API void BR_PUBLIC_ENTRY      BrVector3Cross(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3);
BR_API br_scalar BR_PUBLIC_ENTRY BrVector3Length(br_vector3 *v1);
BR_API br_scalar BR_PUBLIC_ENTRY BrVector3LengthSquared(br_vector3 *v1);

BR_API br_boolean BR_PUBLIC_ENTRY BrVector3Normalise0(br_vector3 *v1, const br_vector3 *v2);
BR_API void BR_PUBLIC_ENTRY       BrVector3Normalise(br_vector3 *v1, const br_vector3 *v2);
BR_API void BR_PUBLIC_ENTRY       BrVector3NormaliseQuick(br_vector3 *v1, const br_vector3 *v2);
BR_API void BR_PUBLIC_ENTRY       BrVector3NormaliseLP(br_vector3 *v1, const br_vector3 *v2);

BR_API br_int_32 BR_PUBLIC_ENTRY BrPlaneEquation(br_vector4 *eqn, const br_vector3 *v0, const br_vector3 *v1,
                                                 const br_vector3 *v2);

/*
 * 2D vectors
 */
BR_API br_boolean BR_PUBLIC_ENTRY BrVector2Normalise0(br_vector2 *v1, const br_vector2 *v2);
BR_API void BR_PUBLIC_ENTRY       BrVector2Normalise(br_vector2 *v1, const br_vector2 *v2);

/**
 ** 4D Vectors
 **/
BR_API void BR_PUBLIC_ENTRY       BrVector4Set(br_vector4 *v1, br_scalar s1, br_scalar s2, br_scalar s3, br_scalar s4);
BR_API br_scalar BR_PUBLIC_ENTRY  BrVector4Dot(br_vector4 *v1, const br_vector4 *v2);
BR_API void BR_PUBLIC_ENTRY       BrVector4Copy(br_vector4 *v1, const br_vector4 *v2);
BR_API void BR_PUBLIC_ENTRY       BrVector4Negate(br_vector4 *v1, const br_vector4 *v2);
BR_API void BR_PUBLIC_ENTRY       BrVector4Scale(br_vector4 *v1, const br_vector4 *v2, br_scalar s);
BR_API br_boolean BR_PUBLIC_ENTRY BrVector4Normalise0(br_vector4 *v1, const br_vector4 *v2);
BR_API void BR_PUBLIC_ENTRY       BrVector4Normalise(br_vector4 *v1, const br_vector4 *v2);

/*
 * Euler Angles
 */
BR_API br_matrix34 *BR_PUBLIC_ENTRY BrEulerToMatrix34(br_matrix34 *mat, const br_euler *euler);
BR_API br_euler *BR_PUBLIC_ENTRY    BrMatrix34ToEuler(br_euler *euler, const br_matrix34 *mat);

BR_API br_matrix4 *BR_PUBLIC_ENTRY BrEulerToMatrix4(br_matrix4 *mat, const br_euler *src);
BR_API br_euler *BR_PUBLIC_ENTRY   BrMatrix4ToEuler(br_euler *dest, const br_matrix4 *mat);

BR_API br_quat *BR_PUBLIC_ENTRY  BrEulerToQuat(br_quat *quat, const br_euler *euler);
BR_API br_euler *BR_PUBLIC_ENTRY BrQuatToEuler(br_euler *euler, const br_quat *quat);

/*
 * Quaternions
 */
BR_API br_quat *BR_PUBLIC_ENTRY BrQuatMul(br_quat *q, const br_quat *l, const br_quat *r);
BR_API br_quat *BR_PUBLIC_ENTRY BrQuatNormalise(br_quat *q, const br_quat *qq);
BR_API br_quat *BR_PUBLIC_ENTRY BrQuatInvert(br_quat *q, const br_quat *qq);

BR_API br_quat *BR_PUBLIC_ENTRY BrQuatSlerp(br_quat *q, const br_quat *l, const br_quat *r, br_scalar t, br_int_16 spins);

BR_API br_matrix34 *BR_PUBLIC_ENTRY BrQuatToMatrix34(br_matrix34 *mat, const br_quat *q);
BR_API br_quat *BR_PUBLIC_ENTRY     BrMatrix34ToQuat(br_quat *q, const br_matrix34 *mat);

BR_API br_matrix4 *BR_PUBLIC_ENTRY BrQuatToMatrix4(br_matrix4 *mat, const br_quat *q);
BR_API br_quat *BR_PUBLIC_ENTRY    BrMatrix4ToQuat(br_quat *q, const br_matrix4 *mat);

/*
 * Transforms
 */
BR_API void BR_PUBLIC_ENTRY BrTransformToMatrix34(br_matrix34 *mat, const br_transform *xform);
BR_API void BR_PUBLIC_ENTRY BrMatrix34ToTransform(br_transform *xform, const br_matrix34 *mat);
BR_API void BR_PUBLIC_ENTRY BrTransformToTransform(br_transform *dest, const br_transform *src);

BR_API void BR_PUBLIC_ENTRY BrMatrix34PreTransform(br_matrix34 *mat, const br_transform *xform);
BR_API void BR_PUBLIC_ENTRY BrMatrix34PostTransform(br_matrix34 *mat, const br_transform *xform);
BR_API void BR_PUBLIC_ENTRY BrMatrix4PreTransform(br_matrix4 *mat, const br_transform *xform);

/*
 * 2x3 Matrix ops.
 */
BR_API void BR_PUBLIC_ENTRY BrMatrix23Copy(br_matrix23 *A, const br_matrix23 *b);
BR_API void BR_PUBLIC_ENTRY BrMatrix23Mul(br_matrix23 *A, const br_matrix23 *B, const br_matrix23 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix23Pre(br_matrix23 *mat, const br_matrix23 *A);
BR_API void BR_PUBLIC_ENTRY BrMatrix23Post(br_matrix23 *mat, const br_matrix23 *A);

BR_API void BR_PUBLIC_ENTRY BrMatrix23Identity(br_matrix23 *mat);

BR_API void BR_PUBLIC_ENTRY BrMatrix23Rotate(br_matrix23 *mat, br_angle rz);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PreRotate(br_matrix23 *mat, br_angle rz);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PostRotate(br_matrix23 *mat, br_angle rz);

BR_API void BR_PUBLIC_ENTRY BrMatrix23Translate(br_matrix23 *mat, br_scalar x, br_scalar y);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PreTranslate(br_matrix23 *mat, br_scalar x, br_scalar y);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PostTranslate(br_matrix23 *mat, br_scalar x, br_scalar y);

BR_API void BR_PUBLIC_ENTRY BrMatrix23Scale(br_matrix23 *mat, br_scalar sx, br_scalar sy);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PreScale(br_matrix23 *mat, br_scalar sx, br_scalar sy);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PostScale(br_matrix23 *mat, br_scalar sx, br_scalar sy);

BR_API void BR_PUBLIC_ENTRY BrMatrix23ShearX(br_matrix23 *mat, br_scalar sy);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PreShearX(br_matrix23 *mat, br_scalar sy);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PostShearX(br_matrix23 *mat, br_scalar sy);

BR_API void BR_PUBLIC_ENTRY BrMatrix23ShearY(br_matrix23 *mat, br_scalar sx);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PreShearY(br_matrix23 *mat, br_scalar sx);
BR_API void BR_PUBLIC_ENTRY BrMatrix23PostShearY(br_matrix23 *mat, br_scalar sx);

BR_API void BR_PUBLIC_ENTRY BrMatrix23ApplyV(br_vector2 *A, const br_vector2 *B, const br_matrix23 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix23ApplyP(br_vector2 *A, const br_vector2 *B, const br_matrix23 *C);

BR_API void BR_PUBLIC_ENTRY BrMatrix23TApplyV(br_vector2 *A, const br_vector2 *B, const br_matrix23 *C);
BR_API void BR_PUBLIC_ENTRY BrMatrix23TApplyP(br_vector2 *A, const br_vector2 *B, const br_matrix23 *C);

BR_API br_scalar BR_PUBLIC_ENTRY BrMatrix23Inverse(br_matrix23 *out, const br_matrix23 *in);
BR_API void BR_PUBLIC_ENTRY      BrMatrix23LPInverse(br_matrix23 *A, const br_matrix23 *B);
BR_API void BR_PUBLIC_ENTRY      BrMatrix23LPNormalise(br_matrix23 *A, const br_matrix23 *B);

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
