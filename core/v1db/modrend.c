/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: modrend.c 1.2 1998/07/14 13:38:44 johng Exp $
 * $Locker: $
 *
 * Model rendering
 */
#include "v1db.h"
#include "shortcut.h"
#include "formats.h"

static void renderAll(br_model *model, br_material *material, br_token type, int on_screen)
{
    if(model->stored) {
        if(on_screen == BRT_ACCEPT)
            GeometryStoredRenderOnScreen(model->stored, v1db.renderer, type);
        else
            GeometryStoredRender(model->stored, v1db.renderer, type);

    } else if(model->prepared) {
        if(on_screen == BRT_ACCEPT)
            GeometryV1ModelRenderOnScreen(v1db.format_model, v1db.renderer, model->prepared, material->stored, type);
        else
            GeometryV1ModelRender(v1db.format_model, v1db.renderer, model->prepared, material->stored, type);
    }
}

/*
 * Render model faces
 */
static void renderFaces(br_actor *actor, br_model *model, br_material *material, void *render_data, br_uint_8 style, int on_screen)
{
    renderAll(model, material, BRT_TRIANGLE, on_screen);
}

/*
 * Render model edges
 */
static void renderEdges(br_actor *actor, br_model *model, br_material *material, void *render_data, br_uint_8 style, int on_screen)
{
    renderAll(model, material, BRT_LINE, on_screen);
}

/*
 * Render model points
 */
static void renderPoints(br_actor *actor, br_model *model, br_material *material, void *render_data, br_uint_8 style, int on_screen)
{
    renderAll(model, material, BRT_POINT, on_screen);
}

/*
 * Render function for BR_RSTYLE_NONE
 */
static void nullRender(br_actor *actor, br_model *model, br_material *material, void *render_data, br_uint_8 style, int on_screen)
{
}

/*
 * The immediate entry for a v1model format whose driver has no immediate path of its own:
 * store the model the caller prepared, draw it through the stored path, drop the store
 * again. glrend and vkrend advertise BRT_STORED_ONLY_B and implement it this way, so their
 * store and their stored draw are the only driver-specific parts; softrend keeps a real
 * immediate path of its own.
 *
 * The store goes through the object's own storedNew() rather than a driver-specific
 * allocate, which is what leaves the set of storable models to the driver: a store that
 * hands back nothing is a type the driver does not take.
 */
static br_error renderImmediate(br_geometry_v1_model *self, br_renderer *renderer, struct v11model *model,
                                br_renderer_state_stored *default_state, br_token type, br_boolean on_screen)
{
    br_geometry_stored *sg = NULL;
    br_error            err;
    br_boolean          pushed = BR_FALSE;
    br_boolean          forced = BR_FALSE;
    br_value            hidden = {0};

    err = GeometryV1ModelStoredNew(self, renderer, &sg, model, type, NULL);

    if(err != BRE_OK || sg == NULL) {
        BrLogWarn("V1DB", "immediate geometry: the driver did not store this model (%d); nothing was drawn", (int)type);
        return BRE_FAIL;
    }

    /*
     * A group with no stored state of its own falls back to default_state; restoring it
     * into the current state is what the stored path's per-group copy does. A push that
     * fails has nothing to restore, so there is nothing to pop either.
     */
    if(default_state != NULL) {
        if(RendererStatePush(renderer, BR_STATE_STORED_PARTS_MASK) == BRE_OK) {
            RendererStateRestore(renderer, default_state, BR_STATE_STORED_PARTS_MASK);
            pushed = BR_TRUE;
        }
    }

    /*
     * Immediate means now. A renderer under bucket-sort does not draw the store below, it
     * puts it in an order table and draws it when the scene flushes - by which time this
     * call has freed it. A scene leaves the hidden-surface part sorted, so it is turned off
     * around the draw and put back afterwards.
     */
    if(RendererPartQuery(renderer, BRT_HIDDEN_SURFACE, 0, &hidden, BRT_TYPE_T) == BRE_OK && hidden.t != BRT_NONE) {
        RendererPartSet(renderer, BRT_HIDDEN_SURFACE, 0, BRT_TYPE_T, (br_value){.t = BRT_NONE});
        forced = BR_TRUE;
    }

    err = on_screen ? GeometryStoredRenderOnScreen(sg, renderer, type) : GeometryStoredRender(sg, renderer, type);

    if(forced)
        RendererPartSet(renderer, BRT_HIDDEN_SURFACE, 0, BRT_TYPE_T, hidden);

    if(pushed)
        RendererStatePop(renderer, BR_STATE_STORED_PARTS_MASK);

    ObjectFree(sg);

    return err;
}

br_error BR_CMETHOD_DECL(br_geometry_v1_model_gen, render)(struct br_geometry_v1_model *self, struct br_renderer *renderer,
                                                           struct v11model *model, struct br_renderer_state_stored *default_state, br_token type)
{
    return renderImmediate(self, renderer, model, default_state, type, BR_FALSE);
}

br_error BR_CMETHOD_DECL(br_geometry_v1_model_gen, renderOnScreen)(struct br_geometry_v1_model *self, struct br_renderer *renderer,
                                                                   struct v11model *model, struct br_renderer_state_stored *default_state,
                                                                   br_token type)
{
    return renderImmediate(self, renderer, model, default_state, type, BR_TRUE);
}

/*
 * A pre-preocessed static cuboid model that we can fill in with
 * appropriate vertex values to represent a bounding box
 *
 * Another way to do this is to use a cube model, and prepend a scale+xform
 * that matches it to the bounds. I would prefer that the user only has to do
 * this when they themselves have created a funny transform.
 */

/*
 * 8 Vertices
 */

// clang-format off
static br_vector3 bounds_position[] = {
    BR_VECTOR3(-1,-1,-1),
    BR_VECTOR3(-1,-1, 1),
    BR_VECTOR3(-1, 1,-1),
    BR_VECTOR3(-1, 1, 1),
    BR_VECTOR3( 1,-1,-1),
    BR_VECTOR3( 1,-1, 1),
    BR_VECTOR3( 1, 1,-1),
    BR_VECTOR3( 1, 1, 1),
};

static br_vector2 bounds_map[] = {
    BR_VECTOR2(0,0),
    BR_VECTOR2(0,0),
    BR_VECTOR2(0,1),
    BR_VECTOR2(0,1),
    BR_VECTOR2(1,0),
    BR_VECTOR2(1,0),
    BR_VECTOR2(1,1),
    BR_VECTOR2(1,1),
};

static br_vector3 bounds_normal[] = {
    BR_VECTOR3(-0.666,-0.333,-0.666),
    BR_VECTOR3(-0.333,-0.666, 0.666),
    BR_VECTOR3(-0.408, 0.816,-0.408),
    BR_VECTOR3(-0.816, 0.408, 0.408),
    BR_VECTOR3( 0.408,-0.816,-0.408),
    BR_VECTOR3( 0.816,-0.408, 0.408),
    BR_VECTOR3( 0.666, 0.333,-0.666),
    BR_VECTOR3( 0.333, 0.666, 0.666),
};

/*
 * 12 Faces
 */
static br_vector3_u16 bounds_vertex_numbers[] = {
    {5, 6, 7},
    {5, 4, 6},
    {7, 6, 2},
    {7, 2, 3},
    {1, 5, 7},
    {1, 7, 3},
    {3, 0, 1},
    {3, 2, 0},
    {1, 0, 4},
    {1, 4, 5},
    {0, 6, 4},
    {0, 2, 6},
};

static br_vector3_u16 bounds_edges[]={
    { 0,13,14},
    { 3, 4, 0},
    {13, 6, 0},
    { 0, 5,16},
    {11,14, 0},
    { 0,16,12},
    { 0, 8,12},
    { 5, 1, 0},
    { 8, 9, 0},
    { 0, 3,11},
    { 0, 4, 9},
    { 1, 6, 0}
};

static br_vector4 bounds_plane_equation[] = {
    BR_VECTOR4( 1, 0, 0,1),
    BR_VECTOR4( 1, 0, 0,1),
    BR_VECTOR4( 0, 1, 0,1),
    BR_VECTOR4( 0, 1, 0,1),
    BR_VECTOR4( 0, 0, 1,1),
    BR_VECTOR4( 0, 0, 1,1),
    BR_VECTOR4(-1, 0, 0,1),
    BR_VECTOR4(-1, 0, 0,1),
    BR_VECTOR4( 0,-1, 0,1),
    BR_VECTOR4( 0,-1, 0,1),
    BR_VECTOR4( 0, 0,-1,1),
    BR_VECTOR4( 0, 0,-1,1)
};
// clang-format on

static br_colour bounds_colours[12];

/*
 * 1 Group
 */
// clang-format off
static struct v11group bounds_face_groups[] = {
    {
        .stored         = NULL,
        .vertex_numbers = bounds_vertex_numbers,
        .edges          = bounds_edges,
        .eqn            = bounds_plane_equation,
        .face_colours   = bounds_colours,
        .face_user      = NULL,
        .face_flags     = 0,
        .position       = bounds_position,
        .map            = bounds_map,
        .normal         = bounds_normal,
        .vertex_colours = bounds_colours,
        .vertex_index   = NULL,
        .vertex_user    = NULL,
        .nfaces         = 12,
        .nvertices      =  8,
        .nedges         = 18,
    },
};
// clang-format on

static struct v11model bounds_prepared = {
    .size    = 0,
    .flags   = 0,
    .ngroups = 1,
    .pivot   = BR_VECTOR3(0, 0, 0),
    .groups  = bounds_face_groups,
};

static struct br_model bounds_model = {
    ._reserved    = 0,
    .identifier   = "Bounds",
    .vertices     = NULL,
    .faces        = NULL,
    .nvertices    = 0,
    .nfaces       = 0,
    .pivot        = BR_VECTOR3(0, 0, 0),
    .flags        = 0,
    .custom       = NULL,
    .user         = NULL,
    .crease_angle = 0,
    .radius       = 0,
    .bounds       = {0},
    .prepared     = &bounds_prepared,
    .stored       = NULL,
};

/*
 * Fills in above cuboid mesh that represents the given bounding box
 */
static struct br_model *makeMeshFromBounds(br_bounds *b)
{
    int i;

    /*
     * Fill in vertices
     */
    BrVector3Set(&bounds_position[0], b->min.v[X], b->min.v[Y], b->min.v[Z]);
    BrVector3Set(&bounds_position[1], b->min.v[X], b->min.v[Y], b->max.v[Z]);
    BrVector3Set(&bounds_position[2], b->min.v[X], b->max.v[Y], b->min.v[Z]);
    BrVector3Set(&bounds_position[3], b->min.v[X], b->max.v[Y], b->max.v[Z]);
    BrVector3Set(&bounds_position[4], b->max.v[X], b->min.v[Y], b->min.v[Z]);
    BrVector3Set(&bounds_position[5], b->max.v[X], b->min.v[Y], b->max.v[Z]);
    BrVector3Set(&bounds_position[6], b->max.v[X], b->max.v[Y], b->min.v[Z]);
    BrVector3Set(&bounds_position[7], b->max.v[X], b->max.v[Y], b->max.v[Z]);

    /*
     * Fill in plane equations of faces
     */
    for(i = 0; i < 3; i++) {
        bounds_plane_equation[i * 2].v[3] = bounds_plane_equation[1 + i * 2].v[3] = b->max.v[i];
        bounds_plane_equation[6 + i * 2].v[3] = bounds_plane_equation[7 + i * 2].v[3] = -b->min.v[i];
    }

    /*
     * Fill in bounds
     */
    bounds_model.bounds = *b;

    return &bounds_model;
}

/*
 * Render bounding box points
 */
static void boundingBoxRenderPoints(br_actor *actor, br_model *model, br_material *material, void *render_data, br_uint_8 style, int on_screen)
{
    renderAll(makeMeshFromBounds(&model->bounds), material, BRT_POINT, on_screen);
}

/*
 * Render bounding box edges
 */
static void boundingBoxRenderEdges(br_actor *actor, br_model *model, br_material *material, void *render_data, br_uint_8 style, int on_screen)
{
    renderAll(makeMeshFromBounds(&model->bounds), material, BRT_LINE, on_screen);
}

/*
 * Render bounding box faces
 */
static void boundingBoxRenderFaces(br_actor *actor, br_model *model, br_material *material, void *render_data, br_uint_8 style, int on_screen)
{
    renderAll(makeMeshFromBounds(&model->bounds), material, BRT_TRIANGLE, on_screen);
}

/*
 * Table of rendering functions indexed by style
 */
br_render_style_call_fn *const RenderStyleCalls[] = {
    [BR_RSTYLE_DEFAULT]         = renderFaces,
    [BR_RSTYLE_NONE]            = nullRender,
    [BR_RSTYLE_POINTS]          = renderPoints,
    [BR_RSTYLE_EDGES]           = renderEdges,
    [BR_RSTYLE_FACES]           = renderFaces,
    [BR_RSTYLE_BOUNDING_POINTS] = boundingBoxRenderPoints,
    [BR_RSTYLE_BOUNDING_EDGES]  = boundingBoxRenderEdges,
    [BR_RSTYLE_BOUNDING_FACES]  = boundingBoxRenderFaces,
};
