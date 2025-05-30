/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: vector.c 1.1 1997/12/10 16:41:23 jon Exp $
 * $Locker: $
 *
 */

#include "brender.h"
#include "shortcut.h"
#include "brassert.h"

/* TODO: make public */
#define BR_CLAMP(v, a, b) (((v) < (a)) ? (a) : (((v) > (b)) ? (b) : (v)))

/**
 ** 2D Vectors
 **/

/*
 * v1=v2
 */
void BR_PUBLIC_ENTRY BrVector2Copy(br_vector2 *v1, const br_vector2 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = v2->v[0];
    v1->v[1] = v2->v[1];
}

/*
 * v1=(s1,s2)
 */
void BR_PUBLIC_ENTRY BrVector2Set(br_vector2 *v1, br_scalar s1, br_scalar s2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);

    v1->v[0] = s1;
    v1->v[1] = s2;
}

void BR_PUBLIC_ENTRY BrVector2SetInt(br_vector2 *v1, int i1, int i2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);

    v1->v[0] = BrIntToScalar(i1);
    v1->v[1] = BrIntToScalar(i2);
}

void BR_PUBLIC_ENTRY BrVector2SetFloat(br_vector2 *v1, float f1, float f2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);

    v1->v[0] = BrFloatToScalar(f1);
    v1->v[1] = BrFloatToScalar(f2);
}

/*
 * v1=-v2
 */
void BR_PUBLIC_ENTRY BrVector2Negate(br_vector2 *v1, const br_vector2 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = -v2->v[0];
    v1->v[1] = -v2->v[1];
}

/*
 * v1=v2+v3
 */
void BR_PUBLIC_ENTRY BrVector2Add(br_vector2 *v1, const br_vector2 *v2, const br_vector2 *v3)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Left Hand Source Vector is NULL", v2 != NULL);
    UASSERT_MESSAGE("Right Hand Source Vector is NULL", v3 != NULL);

    v1->v[0] = v2->v[0] + v3->v[0];
    v1->v[1] = v2->v[1] + v3->v[1];
}

/*
 * v1+=v2
 */
void BR_PUBLIC_ENTRY BrVector2Accumulate(br_vector2 *v1, const br_vector2 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] += v2->v[0];
    v1->v[1] += v2->v[1];
}

/*
 * v1+=v2*scalar
 */
void BR_PUBLIC_ENTRY BrVector2AccumulateScale(br_vector2 *v1, const br_vector2 *v2, br_scalar s)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] += BR_MUL(v2->v[0], s);
    v1->v[1] += BR_MUL(v2->v[1], s);
}

/*
 * v1=v2-v3
 */
void BR_PUBLIC_ENTRY BrVector2Sub(br_vector2 *v1, const br_vector2 *v2, const br_vector2 *v3)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Left Hand Source Vector is NULL", v2 != NULL);
    UASSERT_MESSAGE("Right Hand Source Vector is NULL", v3 != NULL);

    v1->v[0] = v2->v[0] - v3->v[0];
    v1->v[1] = v2->v[1] - v3->v[1];
}

/*
 * v1=v2*scalar
 */
void BR_PUBLIC_ENTRY BrVector2Scale(br_vector2 *v1, const br_vector2 *v2, br_scalar s)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BR_MUL(v2->v[0], (s));
    v1->v[1] = BR_MUL(v2->v[1], (s));
}

/*
 * v1=v2/scalar
 */
void BR_PUBLIC_ENTRY BrVector2InvScale(br_vector2 *v1, const br_vector2 *v2, br_scalar s)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BR_DIV(v2->v[0], (s));
    v1->v[1] = BR_DIV(v2->v[1], (s));
}

/*
 *  = v1.v2
 */
br_scalar BR_PUBLIC_ENTRY BrVector2Dot(br_vector2 *v1, const br_vector2 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    return BR_MAC2(v1->v[0], v2->v[0], v1->v[1], v2->v[1]);
}

/*
 *  = |v1|
 */
br_scalar BR_PUBLIC_ENTRY BrVector2Length(br_vector2 *v1)
{
    UASSERT_MESSAGE("Subject Vector is NULL", v1 != NULL);

    return BR_LENGTH2(v1->v[0], v1->v[1]);
}

/*
 *  = |v1| * |v1|
 */
br_scalar BR_PUBLIC_ENTRY BrVector2LengthSquared(br_vector2 *v1)
{
    UASSERT_MESSAGE("Subject Vector is NULL", v1 != NULL);

    return BR_SQR2(v1->v[0], v1->v[1]);
}

/*
 * v1 = v2/|v2|, or (0, 0) if |v2| == 0
 */
br_boolean BR_PUBLIC_ENTRY BrVector2Normalise0(br_vector2 *v1, const br_vector2 *v2)
{
    br_scalar scale;

    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    scale = BR_LENGTH2(v2->v[0], v2->v[1]);

    if(scale > BR_SCALAR_EPSILON * 2) {
        scale = BR_RCP(scale);

        v1->v[0] = BR_MUL(v2->v[0], scale);
        v1->v[1] = BR_MUL(v2->v[1], scale);

        return BR_TRUE;
    } else {
        v1->v[0] = BR_SCALAR(0.0);
        v1->v[1] = BR_SCALAR(0.0);

        return BR_FALSE;
    }
}

/*
 * v1 = v2/|v2|, or (1, 0) if |v2| == 0
 */
void BR_PUBLIC_ENTRY BrVector2Normalise(br_vector2 *v1, const br_vector2 *v2)
{
    if(!BrVector2Normalise0(v1, v2))
        v1->v[0] = BR_SCALAR(1.0);
}

/*
 * v1 = clamp(v2, min, max)
 */
void BR_PUBLIC_ENTRY BrVector2Clamp(br_vector2 *v1, const br_vector2 *v2, br_scalar min, br_scalar max)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BR_CLAMP(v2->v[0], min, max);
    v1->v[1] = BR_CLAMP(v2->v[1], min, max);
}

/**
 ** 3D VECTORS
 **/

/*
 * v1=v2
 */
void BR_PUBLIC_ENTRY BrVector3Copy(br_vector3 *v1, const br_vector3 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = v2->v[0];
    v1->v[1] = v2->v[1];
    v1->v[2] = v2->v[2];
}

/*
 * v1=(s1,s2,s3)
 */
void BR_PUBLIC_ENTRY BrVector3Set(br_vector3 *v1, br_scalar s1, br_scalar s2, br_scalar s3)
{
    UASSERT_MESSAGE("Subject Vector is NULL", v1 != NULL);

    v1->v[0] = s1;
    v1->v[1] = s2;
    v1->v[2] = s3;
}

void BR_PUBLIC_ENTRY BrVector3SetInt(br_vector3 *v1, int i1, int i2, int i3)
{
    UASSERT_MESSAGE("Subject Vector is NULL", v1 != NULL);

    v1->v[0] = BrIntToScalar(i1);
    v1->v[1] = BrIntToScalar(i2);
    v1->v[2] = BrIntToScalar(i3);
}

void BR_PUBLIC_ENTRY BrVector3SetFloat(br_vector3 *v1, float f1, float f2, float f3)
{
    UASSERT_MESSAGE("Subject Vector is NULL", v1 != NULL);

    v1->v[0] = BrFloatToScalar(f1);
    v1->v[1] = BrFloatToScalar(f2);
    v1->v[2] = BrFloatToScalar(f3);
}

/*
 * v1=(colour.r / 255, colour.g / 255, colour.b / 255)
 */
void BR_PUBLIC_ENTRY BrVector3ColourSet(br_vector3 *v1, br_colour colour)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);

    v1->v[0] = BR_DIV(BR_RED(colour), 255.0f);
    v1->v[1] = BR_DIV(BR_GRN(colour), 255.0f);
    v1->v[2] = BR_DIV(BR_BLU(colour), 255.0f);
}

/*
 * v1=-v2
 */
void BR_PUBLIC_ENTRY BrVector3Negate(br_vector3 *v1, const br_vector3 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v1 != NULL);

    v1->v[0] = -v2->v[0];
    v1->v[1] = -v2->v[1];
    v1->v[2] = -v2->v[2];
}

/*
 * v1=v2+v3
 */
void BR_PUBLIC_ENTRY BrVector3Add(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Left hand Source Vector is NULL", v2 != NULL);
    UASSERT_MESSAGE("Right Hand Source Vector is NULL", v3 != NULL);

    v1->v[0] = v2->v[0] + v3->v[0];
    v1->v[1] = v2->v[1] + v3->v[1];
    v1->v[2] = v2->v[2] + v3->v[2];
}

/*
 * v1+=v2
 */
void BR_PUBLIC_ENTRY BrVector3Accumulate(br_vector3 *v1, const br_vector3 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] += v2->v[0];
    v1->v[1] += v2->v[1];
    v1->v[2] += v2->v[2];
}

/*
 * v1+=v2
 */
void BR_PUBLIC_ENTRY BrVector3AccumulateF(br_vector3 *v1, const br_fvector3 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] += v2->v[0];
    v1->v[1] += v2->v[1];
    v1->v[2] += v2->v[2];
}

/*
 * v1+=v2*scalar
 */
void BR_PUBLIC_ENTRY BrVector3AccumulateScale(br_vector3 *v1, const br_vector3 *v2, br_scalar s)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] += BR_MUL(v2->v[0], s);
    v1->v[1] += BR_MUL(v2->v[1], s);
    v1->v[2] += BR_MUL(v2->v[2], s);
}

/*
 * v1=v2-v3
 */
void BR_PUBLIC_ENTRY BrVector3Sub(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Left hand Source Vector is NULL", v2 != NULL);
    UASSERT_MESSAGE("Right hand Source Vector is NULL", v3 != NULL);

    v1->v[0] = v2->v[0] - v3->v[0];
    v1->v[1] = v2->v[1] - v3->v[1];
    v1->v[2] = v2->v[2] - v3->v[2];
}

/*
 * v1 = v2*v3, element-wise.
 */
void BR_PUBLIC_ENTRY BrVector3Mul(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Left hand Source Vector is NULL", v2 != NULL);
    UASSERT_MESSAGE("Right hand Source Vector is NULL", v3 != NULL);

    v1->v[0] = BR_MUL(v2->v[0], v3->v[0]);
    v1->v[1] = BR_MUL(v2->v[1], v3->v[1]);
    v1->v[2] = BR_MUL(v2->v[2], v3->v[2]);
}

/*
 * v1=v2*scalar
 */
void BR_PUBLIC_ENTRY BrVector3Scale(br_vector3 *v1, const br_vector3 *v2, br_scalar s)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BR_MUL(v2->v[0], (s));
    v1->v[1] = BR_MUL(v2->v[1], (s));
    v1->v[2] = BR_MUL(v2->v[2], (s));
}

/*
 * v1=v2/scalar
 */
void BR_PUBLIC_ENTRY BrVector3InvScale(br_vector3 *v1, const br_vector3 *v2, br_scalar s)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BR_DIV(v2->v[0], (s));
    v1->v[1] = BR_DIV(v2->v[1], (s));
    v1->v[2] = BR_DIV(v2->v[2], (s));
}

/*
 *  = v1.v2
 */
br_scalar BR_PUBLIC_ENTRY BrVector3Dot(const br_vector3 *v1, const br_vector3 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    return BR_MAC3(v1->v[0], v2->v[0], v1->v[1], v2->v[1], v1->v[2], v2->v[2]);
}

/*
 * v1 = v2 x v3
 */
void BR_PUBLIC_ENTRY BrVector3Cross(br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Left hand Source Vector is NULL", v2 != NULL);
    UASSERT_MESSAGE("Right hand Source Vector is NULL", v3 != NULL);
    UASSERT_MESSAGE("Destination vector may not be source", v1 != v2);
    UASSERT_MESSAGE("Destination vector may not be source", v1 != v3);

    v1->v[0] = BR_MUL(v2->v[1], v3->v[2]) - BR_MUL(v2->v[2], v3->v[1]);
    v1->v[1] = BR_MUL(v2->v[2], v3->v[0]) - BR_MUL(v2->v[0], v3->v[2]);
    v1->v[2] = BR_MUL(v2->v[0], v3->v[1]) - BR_MUL(v2->v[1], v3->v[0]);
}

/*
 *  = |v1|
 */
br_scalar BR_PUBLIC_ENTRY BrVector3Length(br_vector3 *v1)
{
    UASSERT_MESSAGE("Subject Vector is NULL", v1 != NULL);

    return BR_LENGTH3(v1->v[0], v1->v[1], v1->v[2]);
}

/*
 * = |v1| * |v1|
 */
br_scalar BR_PUBLIC_ENTRY BrVector3LengthSquared(br_vector3 *v1)
{
    UASSERT_MESSAGE("Subject Vector is NULL", v1 != NULL);

    return BR_SQR3(v1->v[0], v1->v[1], v1->v[2]);
}

/*
 * v1 = v2/|v2|, or (0, 0, 0) if |v2| == 0
 */
br_boolean BR_PUBLIC_ENTRY BrVector3Normalise0(br_vector3 *v1, const br_vector3 *v2)
{
    br_scalar scale;

    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    scale = BR_LENGTH3(v2->v[0], v2->v[1], v2->v[2]);

    if(scale > BR_SCALAR_EPSILON * 2) {
        scale = BR_RCP(scale);

        v1->v[0] = BR_MUL(v2->v[0], scale);
        v1->v[1] = BR_MUL(v2->v[1], scale);
        v1->v[2] = BR_MUL(v2->v[2], scale);

        return BR_TRUE;
    } else {
        v1->v[0] = BR_SCALAR(0.0);
        v1->v[1] = BR_SCALAR(0.0);
        v1->v[2] = BR_SCALAR(0.0);

        return BR_FALSE;
    }
}

/*
 * v1 = v2/|v2|, or (1, 0, 0) if |v2| == 0
 */
void BR_PUBLIC_ENTRY BrVector3Normalise(br_vector3 *v1, const br_vector3 *v2)
{
    if(BrVector3Normalise0(v1, v2) != BR_TRUE)
        v1->v[0] = BR_SCALAR(1.0);
}

/*
 * v1 = v2/|v2|
 */
void BR_PUBLIC_ENTRY BrVector3NormaliseQuick(br_vector3 *v1, const br_vector3 *v2)
{
    br_scalar scale;

    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    scale = BR_RCP(BR_LENGTH3(v2->v[0], v2->v[1], v2->v[2]));
    BrVector3Scale(v1, v2, scale);
}

/*
 * v1 = v2/|v2| (low precision)
 */
void BR_PUBLIC_ENTRY BrVector3NormaliseLP(br_vector3 *v1, const br_vector3 *v2)
{
    br_scalar scale;

    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    scale = BR_RLENGTH3(v2->v[0], v2->v[1], v2->v[2]);

    if(scale != BR_SCALAR(0.0)) {
        v1->v[0] = BR_MUL(v2->v[0], scale);
        v1->v[1] = BR_MUL(v2->v[1], scale);
        v1->v[2] = BR_MUL(v2->v[2], scale);
    }
}

/*
 * v1 = clamp(v2, min, max)
 */
void BR_PUBLIC_ENTRY BrVector3Clamp(br_vector3 *v1, const br_vector3 *v2, br_scalar min, br_scalar max)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BR_CLAMP(v2->v[0], min, max);
    v1->v[1] = BR_CLAMP(v2->v[1], min, max);
    v1->v[2] = BR_CLAMP(v2->v[2], min, max);
}

/**
 ** 4D Vectors
 **/

/*
 * v1=(s1,s2,s3,s4)
 */
void BR_PUBLIC_ENTRY BrVector4Set(br_vector4 *v1, br_scalar s1, br_scalar s2, br_scalar s3, br_scalar s4)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);

    v1->v[0] = s1;
    v1->v[1] = s2;
    v1->v[2] = s3;
    v1->v[3] = s4;
}

/*
 * v1=(colour.r / 255, colour.g / 255, colour.b / 255, colour.a / 255)
 */
void BR_PUBLIC_ENTRY BrVector4ColourSet(br_vector4 *v1, br_colour colour)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);

    v1->v[0] = BR_DIV(BR_RED(colour), 255.0f);
    v1->v[1] = BR_DIV(BR_GRN(colour), 255.0f);
    v1->v[2] = BR_DIV(BR_BLU(colour), 255.0f);
    v1->v[3] = BR_DIV(BR_ALPHA(colour), 255.0f);
}

/*
 *  = v1.v2
 */
br_scalar BR_PUBLIC_ENTRY BrVector4Dot(const br_vector4 *v1, const br_vector4 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    return BR_MAC4(v1->v[0], v2->v[0], v1->v[1], v2->v[1], v1->v[2], v2->v[2], v1->v[3], v2->v[3]);
}

/*
 * v1=v2
 */
void BR_PUBLIC_ENTRY BrVector4Copy(br_vector4 *v1, const br_vector4 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = v2->v[0];
    v1->v[1] = v2->v[1];
    v1->v[2] = v2->v[2];
    v1->v[3] = v2->v[3];
}

/*
 * v1 = (v2.x, v2.y, v2.z, w)
 */
void BR_PUBLIC_ENTRY BrVector4Copy3(br_vector4 *v1, const br_vector3 *v2, br_scalar w)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = v2->v[0];
    v1->v[1] = v2->v[1];
    v1->v[2] = v2->v[2];
    v1->v[3] = w;
}

/*
 * v1=-v2
 */
void BR_PUBLIC_ENTRY BrVector4Negate(br_vector4 *v1, const br_vector4 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v1 != NULL);

    v1->v[0] = -v2->v[0];
    v1->v[1] = -v2->v[1];
    v1->v[2] = -v2->v[2];
    v1->v[3] = -v2->v[3];
}

/*
 * v1=v2*scalar
 */
void BR_PUBLIC_ENTRY BrVector4Scale(br_vector4 *v1, const br_vector4 *v2, br_scalar s)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BR_MUL(v2->v[0], (s));
    v1->v[1] = BR_MUL(v2->v[1], (s));
    v1->v[2] = BR_MUL(v2->v[2], (s));
    v1->v[3] = BR_MUL(v2->v[3], (s));
}

/*
 * v1 = v2/|v2|, or (0, 0, 0, 0) if |v2| == 0
 */
br_boolean BR_PUBLIC_ENTRY BrVector4Normalise0(br_vector4 *v1, const br_vector4 *v2)
{
    br_scalar scale;

    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    scale = BR_LENGTH4(v2->v[0], v2->v[1], v2->v[2], v2->v[3]);

    if(scale > BR_SCALAR_EPSILON * 2) {
        scale = BR_RCP(scale);

        v1->v[0] = BR_MUL(v2->v[0], scale);
        v1->v[1] = BR_MUL(v2->v[1], scale);
        v1->v[2] = BR_MUL(v2->v[2], scale);
        v1->v[3] = BR_MUL(v2->v[3], scale);

        return BR_TRUE;
    } else {
        v1->v[0] = BR_SCALAR(0.0);
        v1->v[1] = BR_SCALAR(0.0);
        v1->v[2] = BR_SCALAR(0.0);
        v1->v[3] = BR_SCALAR(0.0);

        return BR_FALSE;
    }
}

/*
 * v1 = v2/|v2|, or (1, 0, 0, 0) if |v2| == 0
 */
void BR_PUBLIC_ENTRY BrVector4Normalise(br_vector4 *v1, const br_vector4 *v2)
{
    if(!BrVector4Normalise0(v1, v2))
        v1->v[0] = BR_SCALAR(1.0);
}

/*
 * v1+=v2
 */
void BR_PUBLIC_ENTRY BrVector4Accumulate(br_vector4 *v1, const br_vector4 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] += v2->v[0];
    v1->v[1] += v2->v[1];
    v1->v[2] += v2->v[2];
    v1->v[3] += v2->v[3];
}

/*
 * v1+=v2*scalar
 */
void BR_PUBLIC_ENTRY BrVector4AccumulateScale(br_vector4 *v1, const br_vector4 *v2, br_scalar s)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] += BR_MUL(v2->v[0], s);
    v1->v[1] += BR_MUL(v2->v[1], s);
    v1->v[2] += BR_MUL(v2->v[2], s);
    v1->v[3] += BR_MUL(v2->v[3], s);
}

/*
 * v1 = clamp(v2, min, max)
 */
void BR_PUBLIC_ENTRY BrVector4Clamp(br_vector4 *v1, const br_vector4 *v2, br_scalar min, br_scalar max)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BR_CLAMP(v2->v[0], min, max);
    v1->v[1] = BR_CLAMP(v2->v[1], min, max);
    v1->v[2] = BR_CLAMP(v2->v[2], min, max);
    v1->v[3] = BR_CLAMP(v2->v[3], min, max);
}

/**
 ** Private ops for fractional vectors
 **/

/*
 *  = v1.v2 (v1 is a unit vector (word components))
 */
br_scalar BrFVector2Dot(br_fvector2 *v1, br_vector2 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    return BR_FMAC2(v1->v[0], v2->v[0], v1->v[1], v2->v[1]);
}

/*
 * v1=v2
 */
void BrFVector3Copy(br_fvector3 *v1, br_vector3 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BrScalarToFraction(v2->v[0]);
    v1->v[1] = BrScalarToFraction(v2->v[1]);
    v1->v[2] = BrScalarToFraction(v2->v[2]);
}

/*
 * v1=v2*scalar
 */
void BrVector3ScaleF(br_vector3 *v1, br_fvector3 *v2, br_scalar s)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = BR_MUL(BrFractionToScalar(v2->v[0]), s);
    v1->v[1] = BR_MUL(BrFractionToScalar(v2->v[1]), s);
    v1->v[2] = BR_MUL(BrFractionToScalar(v2->v[2]), s);
}

/*
 *  = v1.v2 (v1 is a unit vector (word components))
 */
br_scalar BrFVector3Dot(br_fvector3 *v1, br_vector3 *v2)
{
    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    return BR_FMAC3(v1->v[0], v2->v[0], v1->v[1], v2->v[1], v1->v[2], v2->v[2]);
}

/*
 * v1 = v2/|v2|
 */
void BrFVector3Normalise(br_fvector3 *v1, br_vector3 *v2)
{
    br_scalar scale;

    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    scale = BR_LENGTH3(v2->v[0], v2->v[1], v2->v[2]);

    if(scale != BR_SCALAR(0.0)) {
        scale    = BR_DIV(BR_FRACTION(1.0), scale);
        v1->v[0] = (br_fraction)BR_MUL(v2->v[0], scale);
        v1->v[1] = (br_fraction)BR_MUL(v2->v[1], scale);
        v1->v[2] = (br_fraction)BR_MUL(v2->v[2], scale);
    } else {
        v1->v[0] = BR_FRACTION(1.0);
        v1->v[1] = BR_FRACTION(0.0);
        v1->v[2] = BR_FRACTION(0.0);
    }
}

/*
 * v1 = v2/|v2| (low precision)
 */
void BrFVector3NormaliseLP(br_fvector3 *v1, br_vector3 *v2)
{
    br_scalar scale = BrScalarToFraction(BR_RLENGTH3(v1->v[0], v1->v[1], v1->v[2]));

    UASSERT_MESSAGE("Destination Vector is NULL", v1 != NULL);
    UASSERT_MESSAGE("Source Vector is NULL", v2 != NULL);

    v1->v[0] = (br_fraction)BR_MUL(v2->v[0], scale);
    v1->v[1] = (br_fraction)BR_MUL(v2->v[1], scale);
    v1->v[2] = (br_fraction)BR_MUL(v2->v[2], scale);
}
