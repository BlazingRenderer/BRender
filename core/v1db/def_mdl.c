/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: def_mdl.c 1.2 1998/05/20 11:57:10 jon Exp $
 * $Locker: $
 *
 * default model for when an actor is created - a cube of the default
 * material
 */
#include "brender.h"

/*
 * 8 Vertices
 */
static br_vertex default_model_vertices[] = {
    {BR_VECTOR3(-1.0, -1.0, 1.0),  BR_VECTOR2(0.00, 0.99)}, /*    0 */
    {BR_VECTOR3(1.0,  -1.0, 1.0),  BR_VECTOR2(0.99, 0.99)}, /*    1 */
    {BR_VECTOR3(1.0,  1.0,  1.0),  BR_VECTOR2(0.99, 0.99)}, /*    2 */
    {BR_VECTOR3(-1.0, 1.0,  1.0),  BR_VECTOR2(0.00, 0.99)}, /*    3 */
    {BR_VECTOR3(-1.0, -1.0, -1.0), BR_VECTOR2(0.00, 0.00)}, /*    4 */
    {BR_VECTOR3(1.0,  -1.0, -1.0), BR_VECTOR2(0.99, 0.00)}, /*    5 */
    {BR_VECTOR3(1.0,  1.0,  -1.0), BR_VECTOR2(0.99, 0.00)}, /*    6 */
    {BR_VECTOR3(-1.0, 1.0,  -1.0), BR_VECTOR2(0.00, 0.00)}, /*    7 */
};

/*
 * 12 Faces
 */
static br_face default_model_faces[] = {
    {{0, 1, 2}, 1, NULL, 0, 0, 0, 0, 4}, /*    0 */
    {{0, 2, 3}, 1, NULL, 0, 0, 0, 0, 1}, /*    1 */
    {{0, 4, 5}, 1, NULL, 0, 0, 0, 0, 4}, /*    2 */
    {{0, 5, 1}, 1, NULL, 0, 0, 0, 0, 1}, /*    3 */
    {{1, 5, 6}, 1, NULL, 0, 0, 0, 0, 4}, /*    4 */
    {{1, 6, 2}, 1, NULL, 0, 0, 0, 0, 1}, /*    5 */
    {{2, 6, 7}, 1, NULL, 0, 0, 0, 0, 4}, /*    6 */
    {{2, 7, 3}, 1, NULL, 0, 0, 0, 0, 1}, /*    7 */
    {{3, 7, 4}, 1, NULL, 0, 0, 0, 0, 4}, /*    8 */
    {{3, 4, 0}, 1, NULL, 0, 0, 0, 0, 1}, /*    9 */
    {{4, 7, 6}, 1, NULL, 0, 0, 0, 0, 4}, /*   10 */
    {{4, 6, 5}, 1, NULL, 0, 0, 0, 0, 1}, /*   11 */
};

br_model _BrDefaultModel = {
    ._reserved    = 0,
    .identifier   = "default_model",
    .vertices     = default_model_vertices,
    .faces        = default_model_faces,
    .nvertices    = 8,
    .nfaces       = 12,
    .pivot        = BR_VECTOR3(0.0, 0.0, 0.0),
    .flags        = BR_MODF_UPDATEABLE,
    .custom       = NULL,
    .user         = NULL,
    .crease_angle = BR_ANGLE_DEG(90),
    .radius       = BR_SCALAR(1.732050808),
    .bounds       = {{BR_SCALAR(-1.0), BR_SCALAR(-1.0), BR_SCALAR(-1.0)}, {BR_SCALAR(1.0), BR_SCALAR(1.0), BR_SCALAR(1.0)}}
};
