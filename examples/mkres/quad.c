#include <brender.h>

static br_vertex quad_vertices[4] = {
    {.p = BR_VECTOR3(-0.5f, -0.5f, 0.0f), .n = BR_VECTOR3(0.0f, 0.0f, -1.0f), .map = BR_VECTOR2(0.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3( 0.5f, -0.5f, 0.0f), .n = BR_VECTOR3(0.0f, 0.0f, -1.0f), .map = BR_VECTOR2(1.0f, 0.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3(-0.5f,  0.5f, 0.0f), .n = BR_VECTOR3(0.0f, 0.0f, -1.0f), .map = BR_VECTOR2(0.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},
    {.p = BR_VECTOR3( 0.5f,  0.5f, 0.0f), .n = BR_VECTOR3(0.0f, 0.0f, -1.0f), .map = BR_VECTOR2(1.0f, 1.0f), .red = 255, .grn = 255, .blu = 255},
};

static br_face quad_faces[2] = {
    {.vertices = {3, 2, 0}},
    {.vertices = {1, 3, 0}},
};

br_model *mkres_make_quad(const char *name)
{
    br_model *quad;

    if((quad = BrModelAllocate(name, 0, 0)) == NULL)
        return NULL;

    /* LHS -> RHS */
    quad->flags |= BR_MODF_CUSTOM_NORMALS;
    for(int i = 0; i < BR_ASIZE(quad_vertices); ++i) {
        quad_vertices[i].n.v[2] = -quad_vertices[i].n.v[2];

        quad_vertices[i].p.v[2] = -quad_vertices[i].p.v[2];
    }

    quad->nvertices = BR_ASIZE(quad_vertices);
    quad->vertices  = quad_vertices;

    quad->nfaces = BR_ASIZE(quad_faces);
    quad->faces  = quad_faces;

    /* Required so the engine doesn't free our static memory. */
    quad->flags |= BR_MODF_UPDATEABLE;

    BrModelAdd(quad);
    return quad;
}
