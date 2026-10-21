/*
 * Immediate 2D drawing.
 *
 * A batch is a v11model built here and handed to the renderer's immediate geometry
 * path, the same one a caller reaches with a prepared model. The state setup is
 * device-neutral; the drivers only have to implement the immediate render.
 */
#include "v1db.h"
#include "brassert.h"
#include "formats.h"

/*
 * Everything a batch changes, and therefore everything it restores. glrend and vkrend
 * copy the whole stack entry on a push, so this is about softrend's masked copy.
 */
#define BR2D_STATE_MASK BR_STATE_ALL

static br_boolean    br2d_active;
static br_boolean    br2d_scene;
static br_vertex_2d *br2d_vertices;
static br_uint_32    br2d_count;
static br_uint_32    br2d_capacity;
static br_rectangle  br2d_clip;
static br_boolean    br2d_has_clip;

/*
 * The model's parallel arrays, in one block that is grown to fit the largest batch drawn
 * and then kept, as the vertex buffer above is. They hold nothing between batches, so a
 * grown block is not copied across.
 *
 * One block rather than five allocations because the five were pure overhead: at 768
 * vertices they cost 1.6us of a 145us batch, and the sizes are known together.
 */
typedef struct br2d_arrays {
    br_uint_8      *block;
    br_uint_32      capacity;
    br_vector3     *position;
    br_vector2     *map;
    br_vector3     *normal;
    br_colour      *colours;
    br_vector3_u16 *vertex_numbers;
} br2d_arrays;

static br2d_arrays br2d_batch_arrays;

/*
 * Region sizes are rounded to eight bytes, which is enough alignment for every array type
 * here and keeps the slicing in one place.
 */
static br_size_t br2d_align(br_size_t n)
{
    return (n + 7) & ~(br_size_t)7;
}

static br_size_t br2d_arrays_size(br_uint_32 capacity)
{
    return br2d_align(capacity * sizeof(br_vector3)) + br2d_align(capacity * sizeof(br_vector2)) + br2d_align(capacity * sizeof(br_vector3)) +
           br2d_align(capacity * sizeof(br_colour)) + br2d_align(capacity * sizeof(br_vector3_u16));
}

static br_boolean br2d_arrays_reserve(br_uint_32 nvertices)
{
    br_uint_32 capacity = br2d_batch_arrays.capacity;
    br_uint_8 *block, *old;
    br_size_t  offset;

    if(nvertices <= capacity)
        return BR_TRUE;

    while(capacity < nvertices)
        capacity = capacity != 0 ? capacity * 2 : 256;

    if((block = BrResAllocate(NULL, br2d_arrays_size(capacity), BR_MEMORY_APPLICATION)) == NULL)
        return BR_FALSE;

    offset = 0;

    br2d_batch_arrays.position = (br_vector3 *)(block + offset);
    offset += br2d_align(capacity * sizeof(br_vector3));

    br2d_batch_arrays.map = (br_vector2 *)(block + offset);
    offset += br2d_align(capacity * sizeof(br_vector2));

    br2d_batch_arrays.normal = (br_vector3 *)(block + offset);
    offset += br2d_align(capacity * sizeof(br_vector3));

    br2d_batch_arrays.colours = (br_colour *)(block + offset);
    offset += br2d_align(capacity * sizeof(br_colour));

    br2d_batch_arrays.vertex_numbers = (br_vector3_u16 *)(block + offset);

    old                        = br2d_batch_arrays.block;
    br2d_batch_arrays.block    = block;
    br2d_batch_arrays.capacity = capacity;

    if(old != NULL)
        BrResFree(old);

    return BR_TRUE;
}

/*
 * Vertices are collected as they are submitted and turned into the model's parallel
 * arrays when the batch is drawn, because that is the layout the renderer wants.
 */
static br_boolean br2d_reserve(br_uint_32 nvertices)
{
    br_uint_32    capacity = br2d_capacity;
    br_vertex_2d *vertices;

    if(nvertices <= capacity)
        return BR_TRUE;

    while(capacity < nvertices)
        capacity = capacity != 0 ? capacity * 2 : 256;

    if((vertices = BrResAllocate(NULL, capacity * sizeof(*vertices), BR_MEMORY_APPLICATION)) == NULL)
        return BR_FALSE;

    if(br2d_vertices != NULL) {
        BrMemCpy(vertices, br2d_vertices, br2d_count * sizeof(*vertices));
        BrResFree(br2d_vertices);
    }

    br2d_vertices = vertices;
    br2d_capacity = capacity;

    return BR_TRUE;
}

static void append_vertex(const br_vertex_2d *v)
{
    br2d_vertices[br2d_count++] = *v;
}

static br_vertex_2d lerp_vertex(const br_vertex_2d *a, const br_vertex_2d *b, br_scalar t)
{
    br_vertex_2d out;

    out.position.v[0] = BR_ADD(a->position.v[0], BR_MUL(BR_SUB(b->position.v[0], a->position.v[0]), t));
    out.position.v[1] = BR_ADD(a->position.v[1], BR_MUL(BR_SUB(b->position.v[1], a->position.v[1]), t));
    out.map.v[0]      = BR_ADD(a->map.v[0], BR_MUL(BR_SUB(b->map.v[0], a->map.v[0]), t));
    out.map.v[1]      = BR_ADD(a->map.v[1], BR_MUL(BR_SUB(b->map.v[1], a->map.v[1]), t));
    out.colour        = a->colour;

    return out;
}

/*
 * Sutherland-Hodgman against one axis-aligned edge. `keep_greater` selects which side
 * survives; the vertex count stays small because a convex polygon gains at most one
 * vertex per plane.
 */
static br_uint_32 clip_edge(const br_vertex_2d *in, br_uint_32 n, br_vertex_2d *out, int axis, br_scalar bound,
                            br_boolean keep_greater)
{
    br_uint_32 m = 0;

    for(br_uint_32 i = 0; i < n; ++i) {
        const br_vertex_2d *a = in + i;
        const br_vertex_2d *b = in + (i + 1) % n;

        br_scalar da = BR_SUB(a->position.v[axis], bound);
        br_scalar db = BR_SUB(b->position.v[axis], bound);

        br_boolean ia = keep_greater ? da >= BR_SCALAR(0) : da <= BR_SCALAR(0);
        br_boolean ib = keep_greater ? db >= BR_SCALAR(0) : db <= BR_SCALAR(0);

        if(ia)
            out[m++] = *a;

        if(ia != ib) {
            br_scalar t = BR_DIV(da, BR_SUB(da, db));

            out[m++] = lerp_vertex(a, b, t);
        }
    }

    return m;
}

static void append_triangle(const br_vertex_2d *v0, const br_vertex_2d *v1, const br_vertex_2d *v2)
{
    br_vertex_2d a[8], b[8];
    const br_vertex_2d *in = a;
    br_vertex_2d       *out = b;
    br_uint_32          n = 3;

    if(!br2d_has_clip) {
        append_vertex(v0);
        append_vertex(v1);
        append_vertex(v2);
        return;
    }

    a[0] = *v0;
    a[1] = *v1;
    a[2] = *v2;

    const br_scalar left   = br2d_clip.x;
    const br_scalar right  = BR_ADD(br2d_clip.x, br2d_clip.w);
    const br_scalar top    = br2d_clip.y;
    const br_scalar bottom = BR_ADD(br2d_clip.y, br2d_clip.h);

    for(int edge = 0; edge < 4 && n > 0; ++edge) {
        br_uint_32 m;

        switch(edge) {
            case 0:
                m = clip_edge(in, n, out, 0, left, BR_TRUE);
                break;
            case 1:
                m = clip_edge(in, n, out, 0, right, BR_FALSE);
                break;
            case 2:
                m = clip_edge(in, n, out, 1, top, BR_TRUE);
                break;
            default:
                m = clip_edge(in, n, out, 1, bottom, BR_FALSE);
                break;
        }

        n = m;

        if(in == a) {
            in  = b;
            out = a;
        } else {
            in  = a;
            out = b;
        }
    }

    /* Fan-triangulate the clipped convex polygon. */
    for(br_uint_32 i = 1; i + 1 < n; ++i) {
        append_vertex(in + 0);
        append_vertex(in + i);
        append_vertex(in + i + 1);
    }
}

static void set_2d_state(br_pixelmap *destination)
{
    br_renderer    *r = v1db.renderer;
    br_matrix4      projection;
    br_matrix23     map_identity;
    br_matrix34     identity;
    const br_int_32 width  = destination->width;
    const br_int_32 height = destination->height;

    /*
     * A batch names what it draws into and keeps no depth buffer, so it does not depend on
     * a scene having set the renderer's output first, and it composites over whatever is
     * already in the destination.
     */
    RendererPartSet(r, BRT_OUTPUT, 0, BRT_COLOUR_BUFFER_O, (br_value){.o = (br_object *)destination});
    RendererPartSet(r, BRT_OUTPUT, 0, BRT_DEPTH_BUFFER_O, (br_value){.o = NULL});

    BrMatrix34Identity(&identity);

    /* y is inverted, so that y = 0 is the top of the screen, as the API says. */
    BrMatrix4Orthographic(&projection, BR_SCALAR(0), BrIntToScalar(width), BrIntToScalar(height), BR_SCALAR(0), BR_SCALAR(-1),
                          BR_SCALAR(1));

    RendererPartSet(r, BRT_MATRIX, 0, BRT_MODEL_TO_VIEW_M34_F, (br_value){.m34 = &identity});
    RendererPartSet(r, BRT_MATRIX, 0, BRT_VIEW_TO_SCREEN_M4_F, (br_value){.m4 = &projection});
    RendererPartSet(r, BRT_MATRIX, 0, BRT_VIEW_TO_SCREEN_HINT_T, (br_value){.t = BRT_PARALLEL});

    /*
     * A vertex's map coordinate is the coordinate to sample with, so the transform the
     * mapping source reads it through is the identity. The caller's material may carry a
     * real one, and it arrived with the surface part Br2DBegin() restored.
     */
    BrMatrix23Identity(&map_identity);
    RendererPartSet(r, BRT_SURFACE, 0, BRT_AS_MATRIX23_SCALAR(MAP_MATRIX), (br_value){.m23 = &map_identity});

    /*
     * No hidden-surface deferral. The batch is one group, and a group that the renderer
     * decided to defer would be sorted per triangle against an order table this pass does
     * not have.
     */
    RendererPartSet(r, BRT_HIDDEN_SURFACE, 0, BRT_TYPE_T, (br_value){.t = BRT_NONE});
    RendererPartSet(r, BRT_HIDDEN_SURFACE, 0, BRT_DIVERT_T, (br_value){.t = BRT_NONE});

    /*
     * No depth either: the batch draws in submission order, over whatever is already
     * there. Everything else it draws with - the colour map, the colour and opacity
     * sources, blending, culling, shading, and the map's filtering and addressing - is the
     * caller's material, which Br2DBegin() restored before this.
     */
    RendererPartSet(r, BRT_PRIMITIVE, 0, BRT_DEPTH_TEST_T, (br_value){.t = BRT_ALWAYS});
    RendererPartSet(r, BRT_PRIMITIVE, 0, BRT_DEPTH_WRITE_B, (br_value){.b = BR_FALSE});

    for(br_int_32 i = 0; i < BR_MAX_CLIP_PLANES; ++i)
        RendererPartSet(r, BRT_CLIP, i, BRT_TYPE_T, (br_value){.t = BRT_NONE});
}

/*
 * Turn the collected vertices into the model the immediate path draws. The renderer copies
 * what it needs into its own buffers before returning, so the arrays are free to be the same
 * block for the next batch.
 */
static void draw_batch(void)
{
    br_uint_32      nvertices = br2d_count;
    br_uint_32      nfaces    = nvertices / 3;
    br_vector3     *position;
    br_vector2     *map;
    br_vector3     *normal;
    br_colour      *colours;
    br_vector3_u16 *vertex_numbers;
    struct v11group group = {0};
    struct v11model model = {0};

    if(nvertices == 0)
        return;

    /*
     * A v11group's counts are 16 bit, so a batch larger than one can hold is refused rather
     * than truncated: the low 16 bits' worth would draw and the rest would vanish, which is
     * a wrong picture instead of a missing one. Splitting the batch across several models is
     * a change to what the caller asked for and is not done here.
     */
    if(nvertices > 0xFFFF) {
        BrLogError("2D", "Br2DEnd(): %u vertices, and a v11group holds %u, so the batch was not drawn.", nvertices,
                   0xFFFFu);
        return;
    }

    if(!br2d_arrays_reserve(nvertices))
        return;

    position       = br2d_batch_arrays.position;
    map            = br2d_batch_arrays.map;
    normal         = br2d_batch_arrays.normal;
    colours        = br2d_batch_arrays.colours;
    vertex_numbers = br2d_batch_arrays.vertex_numbers;

    BrVector3Set(&model.bounds.min, BR_SCALAR_MAX, BR_SCALAR_MAX, BR_SCALAR_MAX);
    BrVector3Set(&model.bounds.max, -BR_SCALAR_MAX, -BR_SCALAR_MAX, -BR_SCALAR_MAX);

    for(br_uint_32 i = 0; i < nvertices; ++i) {
        const br_vertex_2d *v = br2d_vertices + i;

        position[i].v[0] = v->position.v[0];
        position[i].v[1] = v->position.v[1];
        position[i].v[2] = BR_SCALAR(0);

        map[i]       = v->map;
        colours[i]   = v->colour;
        BrVector3Set(&normal[i], BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(0));

        if(position[i].v[0] < model.bounds.min.v[0])
            model.bounds.min.v[0] = position[i].v[0];
        if(position[i].v[1] < model.bounds.min.v[1])
            model.bounds.min.v[1] = position[i].v[1];
        if(position[i].v[0] > model.bounds.max.v[0])
            model.bounds.max.v[0] = position[i].v[0];
        if(position[i].v[1] > model.bounds.max.v[1])
            model.bounds.max.v[1] = position[i].v[1];
    }

    for(br_uint_32 f = 0; f < nfaces; ++f) {
        vertex_numbers[f].v[0] = (br_uint_16)(f * 3 + 0);
        vertex_numbers[f].v[1] = (br_uint_16)(f * 3 + 1);
        vertex_numbers[f].v[2] = (br_uint_16)(f * 3 + 2);
    }

    group.position       = position;
    group.map            = map;
    group.normal         = normal;
    group.vertex_colours = colours;
    group.vertex_numbers = vertex_numbers;
    group.nvertices      = (br_uint_16)nvertices;
    group.nfaces         = (br_uint_16)nfaces;

    model.size            = sizeof(model);
    model.ngroups         = 1;
    model.groups          = &group;
    BrVector3Set(&model.pivot, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(0));
    model.radius = BR_SCALAR(0);
    BrVector3Set(&model.centre, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(0));
    model.centred_radius = BR_SCALAR(0);

    GeometryV1ModelRender(v1db.format_model, v1db.renderer, &model, NULL, BRT_TRIANGLE);
}


void BR_PUBLIC_ENTRY Br2DBegin(br_pixelmap *destination, br_material *material, const br_rectangle *clip)
{
    UASSERT_MESSAGE("Br2DBegin() called with no renderer", v1db.renderer != NULL);
    UASSERT_MESSAGE("Br2DBegin() is not re-entrant", br2d_active == BR_FALSE);
    UASSERT_MESSAGE("Br2DBegin() needs a destination", destination != NULL);
    UASSERT_MESSAGE("Br2DBegin() needs a material", material != NULL);

    /*
     * A material's state is prepared against a renderer - BrMaterialAdd() does it when a
     * material is registered - and a batch restores what is already there rather than
     * building it, as the scene path does. An unprepared material would leave the batch
     * drawing with whatever state the renderer happened to have.
     */
    UASSERT_MESSAGE("Br2DBegin() needs a prepared material: BrMaterialAdd() or BrMaterialUpdate() it first",
                    material->stored != NULL);

    RendererStatePush(v1db.renderer, BR2D_STATE_MASK);
    RendererStateRestore(v1db.renderer, material->stored, BR_STATE_PRIMITIVE | BR_STATE_SURFACE | BR_STATE_CULL);
    set_2d_state(destination);

    /*
     * A batch outside a scene needs the scene's per-frame setup (framebuffer, viewport,
     * shader); one inside a scene - after BrZbSceneRenderBegin() - must not touch it.
     */
    br2d_scene = (v1db.rendering == RENDERING_NONE) ? BR_TRUE : BR_FALSE;

    if(br2d_scene)
        RendererSceneBegin(v1db.renderer);

    br2d_has_clip = clip != NULL;

    if(clip != NULL)
        br2d_clip = *clip;

    br2d_count  = 0;
    br2d_active = BR_TRUE;
}
void BR_PUBLIC_ENTRY Br2DVertices(const br_vertex_2d *vertices, br_int_32 nvertices)
{
    UASSERT_MESSAGE("Br2DVertices() called outside a batch", br2d_active == BR_TRUE);
    UASSERT_MESSAGE("Br2DVertices() needs a multiple of three vertices", nvertices > 0 && nvertices % 3 == 0);

    /*
     * A clipped triangle can become up to seven vertices, so reserve for the worst case
     * rather than growing inside the loop.
     */
    if(!br2d_reserve(br2d_count + (br_uint_32)nvertices * 3))
        BR_ERROR0("Br2DVertices() could not allocate");

    for(br_int_32 i = 0; i < nvertices; i += 3)
        append_triangle(vertices + i, vertices + i + 1, vertices + i + 2);
}

void BR_PUBLIC_ENTRY Br2DEnd(void)
{
    UASSERT_MESSAGE("Br2DEnd() called outside a batch", br2d_active == BR_TRUE);

    draw_batch();

    if(br2d_scene)
        RendererSceneEnd(v1db.renderer);

    RendererStatePop(v1db.renderer, BR2D_STATE_MASK);

    br2d_active = BR_FALSE;
    br2d_count  = 0;
}
