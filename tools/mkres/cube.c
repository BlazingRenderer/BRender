#include <brender.h>

static br_vertex cube_vertices[24] = {
    {.p = BR_VECTOR3(0.5f,  -0.5f, -0.5f), .n = BR_VECTOR3(0.0f,  0.0f,  -1.0f), .map = BR_VECTOR2(1.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, -0.5f, -0.5f), .n = BR_VECTOR3(0.0f,  0.0f,  -1.0f), .map = BR_VECTOR2(0.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(0.5f,  0.5f,  -0.5f), .n = BR_VECTOR3(0.0f,  0.0f,  -1.0f), .map = BR_VECTOR2(1.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, 0.5f,  -0.5f), .n = BR_VECTOR3(0.0f,  0.0f,  -1.0f), .map = BR_VECTOR2(0.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},

    {.p = BR_VECTOR3(0.5f,  0.5f,  -0.5f), .n = BR_VECTOR3(0.0f,  1.0f,  0.0f),  .map = BR_VECTOR2(1.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, 0.5f,  -0.5f), .n = BR_VECTOR3(0.0f,  1.0f,  0.0f),  .map = BR_VECTOR2(0.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(0.5f,  0.5f,  0.5f),  .n = BR_VECTOR3(0.0f,  1.0f,  0.0f),  .map = BR_VECTOR2(1.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, 0.5f,  0.5f),  .n = BR_VECTOR3(0.0f,  1.0f,  0.0f),  .map = BR_VECTOR2(0.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},

    {.p = BR_VECTOR3(-0.5f, -0.5f, 0.5f),  .n = BR_VECTOR3(0.0f,  0.0f,  1.0f),  .map = BR_VECTOR2(1.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(0.5f,  -0.5f, 0.5f),  .n = BR_VECTOR3(0.0f,  0.0f,  1.0f),  .map = BR_VECTOR2(0.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, 0.5f,  0.5f),  .n = BR_VECTOR3(0.0f,  0.0f,  1.0f),  .map = BR_VECTOR2(1.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(0.5f,  0.5f,  0.5f),  .n = BR_VECTOR3(0.0f,  0.0f,  1.0f),  .map = BR_VECTOR2(0.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},

    {.p = BR_VECTOR3(0.5f,  -0.5f, 0.5f),  .n = BR_VECTOR3(0.0f,  -1.0f, 0.0f),  .map = BR_VECTOR2(1.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, -0.5f, 0.5f),  .n = BR_VECTOR3(0.0f,  -1.0f, 0.0f),  .map = BR_VECTOR2(0.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(0.5f,  -0.5f, -0.5f), .n = BR_VECTOR3(0.0f,  -1.0f, 0.0f),  .map = BR_VECTOR2(1.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, -0.5f, -0.5f), .n = BR_VECTOR3(0.0f,  -1.0f, 0.0f),  .map = BR_VECTOR2(0.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},

    {.p = BR_VECTOR3(-0.5f, -0.5f, -0.5f), .n = BR_VECTOR3(-1.0f, 0.0f,  0.0f),  .map = BR_VECTOR2(1.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, -0.5f, 0.5f),  .n = BR_VECTOR3(-1.0f, 0.0f,  0.0f),  .map = BR_VECTOR2(0.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, 0.5f,  -0.5f), .n = BR_VECTOR3(-1.0f, 0.0f,  0.0f),  .map = BR_VECTOR2(1.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f, 0.5f,  0.5f),  .n = BR_VECTOR3(-1.0f, 0.0f,  0.0f),  .map = BR_VECTOR2(0.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},

    {.p = BR_VECTOR3(0.5f,  -0.5f, 0.5f),  .n = BR_VECTOR3(1.0f,  0.0f,  0.0f),  .map = BR_VECTOR2(1.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(0.5f,  -0.5f, -0.5f), .n = BR_VECTOR3(1.0f,  0.0f,  0.0f),  .map = BR_VECTOR2(0.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(0.5f,  0.5f,  0.5f),  .n = BR_VECTOR3(1.0f,  0.0f,  0.0f),  .map = BR_VECTOR2(1.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(0.5f,  0.5f,  -0.5f), .n = BR_VECTOR3(1.0f,  0.0f,  0.0f),  .map = BR_VECTOR2(0.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},
};

// clang-format off
static br_face cube_faces[12] = {
    {.vertices = { 0,  2,  3}},
    {.vertices = { 0,  3,  1}},
    {.vertices = { 4,  6,  7}},
    {.vertices = { 4,  7,  5}},
    {.vertices = { 8, 10, 11}},
    {.vertices = { 8, 11,  9}},
    {.vertices = {12, 14, 15}},
    {.vertices = {12, 15, 13}},
    {.vertices = {16, 18, 19}},
    {.vertices = {16, 19, 17}},
    {.vertices = {20, 22, 23}},
    {.vertices = {20, 23, 21}},
};
// clang-format on

br_model *mkres_make_cube(const char *name)
{
    br_model *cube;

    if((cube = BrModelAllocate(name, 0, 0)) == NULL)
        return NULL;

    /* LHS -> RHS */
    cube->flags |= BR_MODF_CUSTOM_NORMALS;
    for(int i = 0; i < BR_ASIZE(cube_vertices); ++i) {
        cube_vertices[i].n.v[2] = -cube_vertices[i].n.v[2];

        cube_vertices[i].p.v[2] = -cube_vertices[i].p.v[2];
    }

    cube->nvertices = BR_ASIZE(cube_vertices);
    cube->vertices  = cube_vertices;

    cube->nfaces = BR_ASIZE(cube_faces);
    cube->faces  = cube_faces;

    /* Required so the engine doesn't free our static memory. */
    cube->flags |= BR_MODF_UPDATEABLE;

    BrModelAdd(cube);
    return cube;
}
