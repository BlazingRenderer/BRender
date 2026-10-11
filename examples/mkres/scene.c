/*
 * Scene fixtures for the light cull.
 *
 * The engine culls lights that cannot reach a model's bounding sphere. The
 * radius half of that is covered by the checked-in scenery; the spot/angle half
 * is not, because that scenery contains no spot lights (the game this engine is
 * used by does not use them).
 *
 * Each fixture isolates one property, so that a failure names itself:
 *
 *   scene-spot-hit        one spot aimed at the cube    - must NOT be culled
 *   scene-spot-miss       one spot aimed away           - must be culled
 *   scene-spot-hit-miss   both, so one run covers both directions
 *   scene-radius-near     a point light that reaches    - must NOT be culled
 *   scene-radius-far      a point light that cannot     - must be culled
 *   scene-scaled          a non-uniformly scaled cube with a model-space light
 *   scene-view-space      a BR_LIGHT_VIEW light
 *
 * A light shines along its local -Z (TRM: a light "shines along the negative z
 * axis of the light actor"), so aiming one is purely a matter of its actor
 * transform. The distance and cone are chosen so that the aimed-at and
 * aimed-away cases are unambiguous: the cube is 1x1x1 at the origin (bounding
 * radius ~0.866), the light sits 3 units away, and the cone is 40 degrees.
 *
 * The ambient light exists so that a dropped light reads as a dark cube rather
 * than a black frame - a black frame would not distinguish "the light was culled"
 * from "nothing rendered".
 */
#include <stdio.h>
#include <brender.h>

/* cube.c */
br_model *mkres_make_cube(const char *name);

/*
 * Bounding radius of the cube built by mkres_make_cube(): 1x1x1 about the origin.
 */
#define SCENE_CUBE_RADIUS      BR_SCALAR(0.8660254)

#define SCENE_LIGHT_DISTANCE   BR_SCALAR(3.0)
#define SCENE_CAMERA_DISTANCE  BR_SCALAR(6.0)
#define SCENE_LIGHT_CONE_OUTER BR_ANGLE_DEG(40)
#define SCENE_LIGHT_CONE_INNER BR_ANGLE_DEG(20)

/*
 * 3.0 reaches the cube at distance 3; 1.5 cannot.
 */
#define SCENE_RADIUS_REACHES BR_SCALAR(3.0)
#define SCENE_RADIUS_SHORT   BR_SCALAR(1.5)

static br_actor *scene_actor(br_actor *parent, br_uint_8 type, const char *name)
{
    br_actor *a;

    if((a = BrActorAllocate(type, NULL)) == NULL)
        return NULL;

    a->identifier = BrResStrDup(a, name);
    BrActorAdd(parent, a);

    return a;
}

/*
 * Place an actor and orient it so that its local -Z points at the origin if
 * `at_origin`, or straight past it otherwise. Both cameras and lights use -Z as
 * forward: a light "shines along the negative z axis of the light actor", and a
 * camera looks down its -Z as well.
 */
static void scene_place_and_aim(br_actor *a, br_scalar x, br_scalar y, br_scalar z, br_boolean at_origin)
{
    br_matrix34 m;

    if(at_origin)
        BrMatrix34Identity(&m);
    else
        BrMatrix34RotateX(&m, BR_ANGLE_DEG(180));

    m.m[3][0] = x;
    m.m[3][1] = y;
    m.m[3][2] = z;
    m.m[3][3] = BR_SCALAR(1.0);

    a->t.type  = BR_TRANSFORM_MATRIX34;
    a->t.t.mat = m;
}

/*
 * As scene_place_and_aim, but turned about Y so the light's -Z picks up a
 * component on a scaled axis. A direction that lies purely along Z transforms
 * identically under the transpose and the inverse, so a straight-on fixture
 * cannot exercise the model-space direction transform at all.
 */
static void scene_place_and_turn(br_actor *a, br_scalar x, br_scalar y, br_scalar z, br_scalar angle_y)
{
    br_matrix34 m;

    BrMatrix34RotateY(&m, angle_y);

    m.m[3][0] = x;
    m.m[3][1] = y;
    m.m[3][2] = z;
    m.m[3][3] = BR_SCALAR(1.0);

    a->t.type  = BR_TRANSFORM_MATRIX34;
    a->t.t.mat = m;
}

static br_actor *scene_add_ambient_col(br_actor *world, br_colour colour)
{
    br_actor *a;
    br_light *l;

    if((a = scene_actor(world, BR_ACTOR_LIGHT, "ambient")) == NULL)
        return NULL;

    l = a->type_data;

    l->identifier    = a->identifier;
    l->type          = BR_LIGHT_AMBIENT;
    l->colour        = colour;
    l->attenuation_c = BR_SCALAR(1.0);

    return a;
}

static br_actor *scene_add_ambient(br_actor *world)
{
    return scene_add_ambient_col(world, BR_COLOUR_RGB(48, 48, 48));
}

/*
 * A light 3 units from the origin, aimed either at it or away. `radius_outer`
 * of zero leaves radius culling off, which isolates the angle test.
 */
static br_actor *scene_add_light_col(br_actor *world, const char *name, br_uint_8 type, br_colour colour, br_scalar radius_outer, br_boolean at_origin)
{
    br_actor *a;
    br_light *l;

    if((a = scene_actor(world, BR_ACTOR_LIGHT, name)) == NULL)
        return NULL;

    l = a->type_data;

    l->identifier    = a->identifier;
    l->type          = type;
    l->colour        = colour;
    l->attenuation_c = BR_SCALAR(1.0);
    l->cone_outer    = SCENE_LIGHT_CONE_OUTER;
    l->cone_inner    = SCENE_LIGHT_CONE_INNER;
    l->radius_outer  = radius_outer;
    l->radius_inner  = BR_SCALAR(0.0);

    /*
     * radius_outer alone doesn't turn radius culling on - BrSetupLights derives
     * it from the flag, so a fixture that sets one without the other would test
     * nothing.
     */
    if(radius_outer != BR_SCALAR(0.0))
        l->type |= BR_LIGHT_LINEAR_FALLOFF;

    scene_place_and_aim(a, BR_SCALAR(0.0), BR_SCALAR(0.0), SCENE_LIGHT_DISTANCE, at_origin);

    return a;
}

static br_actor *scene_add_light(br_actor *world, const char *name, br_uint_8 type, br_scalar radius_outer, br_boolean at_origin)
{
    return scene_add_light_col(world, name, type, BR_COLOUR_RGB(255, 255, 255), radius_outer, at_origin);
}

static br_actor *scene_add_light_turned(br_actor *world, const char *name, br_uint_8 type, br_scalar radius_outer, br_scalar angle_y)
{
    br_actor *a;
    br_light *l;

    if((a = scene_actor(world, BR_ACTOR_LIGHT, name)) == NULL)
        return NULL;

    l = a->type_data;

    l->identifier    = a->identifier;
    l->type          = type;
    l->colour        = BR_COLOUR_RGB(255, 255, 255);
    l->attenuation_c = BR_SCALAR(1.0);
    l->cone_outer    = SCENE_LIGHT_CONE_OUTER;
    l->cone_inner    = SCENE_LIGHT_CONE_INNER;
    l->radius_outer  = radius_outer;
    l->radius_inner  = BR_SCALAR(0.0);

    if(radius_outer != BR_SCALAR(0.0))
        l->type |= BR_LIGHT_LINEAR_FALLOFF;

    scene_place_and_turn(a, BR_SCALAR(0.0), BR_SCALAR(0.0), SCENE_LIGHT_DISTANCE, angle_y);

    return a;
}

/*
 * The scene needs a camera of its own: gltfview focuses its editor camera on the
 * first BR_ACTOR_CAMERA it finds, and with none present it falls back to a
 * default view that does not frame a unit cube at all.
 */
static br_actor *scene_add_camera(br_actor *world)
{
    br_actor  *a;
    br_camera *c;

    if((a = scene_actor(world, BR_ACTOR_CAMERA, "camera")) == NULL)
        return NULL;

    c = a->type_data;

    c->type          = BR_CAMERA_PERSPECTIVE_FOV;
    c->field_of_view = BR_ANGLE_DEG(45);
    c->hither_z      = BR_SCALAR(0.1);
    c->yon_z         = BR_SCALAR(100.0);

    scene_place_and_aim(a, BR_SCALAR(0.0), BR_SCALAR(0.0), SCENE_CAMERA_DISTANCE, BR_TRUE);

    return a;
}

static br_actor *scene_world_amb(br_model *cube, br_material *material, br_scalar scale_x, br_colour ambient)
{
    br_actor *world, *a;

    if((world = BrActorAllocate(BR_ACTOR_NONE, NULL)) == NULL)
        return NULL;

    world->identifier = BrResStrDup(world, "world");

    if((a = scene_actor(world, BR_ACTOR_MODEL, "cube")) == NULL) {
        BrActorFree(world);
        return NULL;
    }

    a->model    = cube;
    a->material = material;

    if(scale_x != BR_SCALAR(1.0)) {
        br_matrix34 m;

        BrMatrix34Scale(&m, scale_x, BR_SCALAR(1.0), BR_SCALAR(1.0));

        a->t.type  = BR_TRANSFORM_MATRIX34;
        a->t.t.mat = m;
    }

    if(scene_add_camera(world) == NULL) {
        BrActorFree(world);
        return NULL;
    }

    if(scene_add_ambient_col(world, ambient) == NULL) {
        BrActorFree(world);
        return NULL;
    }

    return world;
}

static br_actor *scene_world(br_model *cube, br_material *material, br_scalar scale_x)
{
    return scene_world_amb(cube, material, scale_x, BR_COLOUR_RGB(48, 48, 48));
}

/*
 * Narrow the world camera's depth range. The fog fixture needs it: the indexed
 * fog level is the high byte of the interpolated z, so the default 0.1..100
 * range puts the whole cube inside two or three levels and the fog table has
 * nothing to say.
 */
static void scene_set_camera_range(br_actor *world, br_scalar hither, br_scalar yon)
{
    br_actor  *a;
    br_camera *cam;

    for(a = world->children; a != NULL; a = a->next) {
        if(a->type != BR_ACTOR_CAMERA)
            continue;

        cam = a->type_data;

        cam->hither_z = hither;
        cam->yon_z    = yon;
        break;
    }
}

static br_error scene_save(const char *name, br_actor *world)
{
    br_actor *actors[1];
    br_error  r;

    if(world == NULL) {
        fprintf(stderr, "failed to build scene %s\n", name);
        return BRE_FAIL;
    }

    actors[0] = world;

    r = BrFmtGLTFActorSaveMany(name, actors, 1);

    BrActorFree(world);

    if(r != BRE_OK)
        fprintf(stderr, "failed to save %s\n", name);

    return r;
}

static br_material *scene_material(const char *name)
{
    br_material *m;

    if((m = BrMaterialAllocate(name)) == NULL)
        return NULL;

    m->colour = BR_COLOUR_RGB(200, 200, 200);
    m->flags |= BR_MATF_LIGHT;

    BrMaterialAdd(m);

    return m;
}

static br_material *scene_material_ex(const char *name, br_colour colour, br_uint_32 flags, br_scalar ka, br_scalar kd, br_scalar ks, br_scalar power)
{
    br_material *m;

    if((m = BrMaterialAllocate(name)) == NULL)
        return NULL;

    m->colour = colour;
    m->flags  = flags;
    m->ka     = ka;
    m->kd     = kd;
    m->ks     = ks;
    m->power  = power;

    BrMaterialAdd(m);

    return m;
}

/*
 * One half-space (x >= 0) in the light actor's own frame. The rig places the
 * light on +Z with identity rotation, so this is model x >= 0 and it cuts the
 * visible +Z face down the middle. The smooth material is deliberate: the
 * volume is evaluated where the lighting is, and a flat-shaded face is lit from
 * a single point, so the fade would not show across it.
 */
static br_vector4       light_volume_planes[1]  = {{{BR_SCALAR(1.0), BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(0.0)}}};
static br_convex_region light_volume_regions[1] = {{light_volume_planes, 1}};

static br_actor *scene_add_volume_light(br_actor *world, br_scalar falloff_distance)
{
    br_actor *a = scene_add_light(world, "point", BR_LIGHT_POINT, BR_SCALAR(0.0), BR_TRUE);
    br_light *l;

    if(a == NULL)
        return NULL;

    l = a->type_data;

    l->volume.falloff_distance = falloff_distance;
    l->volume.regions          = light_volume_regions;
    l->volume.nregions         = 1;

    return a;
}

static br_material *scene_light_volume_material(void)
{
    return scene_material_ex("scene-light-volume-material", BR_COLOUR_RGB(200, 200, 200), BR_MATF_LIGHT | BR_MATF_SMOOTH,
                             BR_SCALAR(0.1), BR_SCALAR(0.7), BR_SCALAR(0.0), BR_SCALAR(20.0));
}

/*
 * ------------------------------------------------------------------
 * Render-feature fixtures.
 *
 * The light-cull fixtures above each isolate one lighting rule. These do the
 * same for the rasteriser's feature set: each one is a plain cube differing from
 * a neighbour by exactly one piece of material state, so a path that silently
 * stops firing moves one frame instead of none.
 *
 * They exist because the nine checked-in reference scenes reach only a
 * fraction of the block set: on all nine of them the matcher selects the
 * *unlit* textured blocks, so the shaded, fogged, blended, decal and dithered
 * families have no witness anywhere in the tree.
 *
 *   scene-flat / scene-smooth   constant vs interpolated intensity
 *   scene-textured              an indexed texture, no lookup table
 *   scene-textured-shade        + an index_shade ramp
 *   scene-persp                 the perspective-correct texture block
 *   scene-persp-shade           + an index_shade ramp
 *   scene-tex-arb               a non-power-of-two map (arbitrary width)
 *   scene-tex-32                32x32 map, the z-sorted power-of-two
 *                               perspective block's own size
 *   scene-shade                 the untextured shade-table block
 *   scene-decal                 decal through a shade table
 *   scene-fog                   the indexed fog table
 *   scene-blend                 the indexed blend table
 *   scene-dither                dithered_map
 *   scene-alpha                 opacity < 255, for the RGB blend path
 *   scene-shade-rgb555/565      the RGB-output shade table, one per output
 *                               type, each in an interpolated and a flat form
 *
 * The indexed tables are INDEX_8 pixelmaps with no palette. Their bytes are
 * indices into the table itself, not colours, and the rasterisers read them
 * directly; a palette would be a lie about what the data is. The RGB-output
 * shade tables are the exception: their type must equal the output format, so
 * scene-shade-rgb555/565 carry an RGB pixelmap whose samples are colours.
 */

/*
 * The texture's two indices are far apart on purpose: the PPM dump of an
 * INDEX_8 frame goes through a grey ramp built from the material's index band,
 * and a fixture whose two texels land on adjacent indices cannot be read.
 */
#define SCENE_TEX_INDEX_A 32
#define SCENE_TEX_INDEX_B 224
#define SCENE_TEX_SIZE    64
#define SCENE_TEX_CELLS   16
#define SCENE_P256_SIZE   256
#define SCENE_TEX32_SIZE  32
#define SCENE_TEX32_SCALE BR_SCALAR(3.0)
#define SCENE_ARB_WIDTH   96
#define SCENE_ARB_HEIGHT  48
#define SCENE_ARB_CELLS   12
#define SCENE_TABLE_WIDTH 256
#define SCENE_TABLE_ROWS  256

/*
 * A band of its own for the blend table's output: see scene_blend_table().
 */
#define SCENE_BLEND_BASE 64

/*
 * The feature fixtures are built on the same cube as everything else, scaled up
 * by the same non-uniform-forcing factor the scene-scaled fixtures use. It is
 * not cosmetic: pentprim's perspective setup falls back to the affine rasteriser
 * when a triangle's w range is narrow (`SETUP_FLOAT_CHECK_PERSPECTIVE_CHEAT`,
 * gated on wrange*srange < wmin*4), and at the default scale the whole cube is
 * inside that threshold - so a perspective fixture would silently test nothing
 * but the affine path.
 */
#define SCENE_FX_SCALE          BR_SCALAR(1.5)
#define SCENE_NEAR_LIGHT_RADIUS BR_SCALAR(5.0)

/*
 * The scale the RGB-output, arbitrary-width shade-table fixtures are drawn at,
 * measured rather than chosen. The trapezium's u/v correction loops pull the
 * span's first texel address back towards the span's own walk, and on the 1.5x
 * feature cube that correction absorbs the wrong seed entirely: a rebuild that
 * reintroduces the old packing leaves a 1.5x fixture's checksum unmoved. Only
 * at 3.0x does the error survive. scene-tex-32 carries the same measurement for
 * the 32x32 block; this is the same trap on the arbitrary-width mapper, so the
 * fixtures that reach its CORRECT cells have to be at least this large.
 */
#define SCENE_ARB_SCALE BR_SCALAR(3.0)

/*
 * The backdrop the blend fixtures draw behind their cubes, so that the indexed
 * destination blend has colour to mix with rather than the clear colour. The
 * cube model is drawn wide and shallow and pushed back: scale_x/y of 60 make
 * the front face far larger than the projected frame at its depth, and the
 * small scale_z keeps it flat against the camera, while the z of -20 seats it
 * behind the cubes (whose back face is near z = -2.6 at the 3x p256 scale) yet
 * inside the scene camera's 0.1..100 range. The front face sits at z = -19, and
 * the widest frame half-width there is the 1280x720 viewport's, 0.7365 * 25 =
 * 18.4, well inside the 30 the scale covers.
 *
 * The fogged blend fixtures cannot use that place: their fog tables need the
 * narrowed 4..8 camera range (scene_fx_world_range), whose far plane is at
 * z = -2, so a backdrop at -20 is clipped away entirely. Those scenes seat
 * theirs just inside that plane instead - a thin slab at z = -1.6, behind the
 * 1.5x cubes' own back face near z = -1.2 - and at a smaller scale, since the
 * plane is far closer to the camera: at 7.6 out the 320x240 viewport's frame
 * half-width is 0.4142 * 7.6 * 4/3 = 4.2, inside the 6 the scale covers.
 */
#define SCENE_BACKDROP_SCALE_XY BR_SCALAR(60.0)
#define SCENE_BACKDROP_SCALE_Z  BR_SCALAR(2.0)
#define SCENE_BACKDROP_Z        BR_SCALAR(-20.0)

/* The fogged blend fixtures' place; see the note above. */
#define SCENE_BACKDROP_FOG_SCALE_XY BR_SCALAR(12.0)
#define SCENE_BACKDROP_FOG_SCALE_Z  BR_SCALAR(0.25)
#define SCENE_BACKDROP_FOG_Z        BR_SCALAR(-1.6)

/*
 * The index the backdrop's primitive resolves to on an INDEX_8 output, and so
 * the row of the blend table the blend reads: SP_BLEND_INDEX is
 * blend_table[(dst_index << 8) | src_index], and dst_index is whatever the
 * backdrop left in the colour buffer. An unlit, untextured primitive on the
 * indexed path writes its material's index_base and discards the material's RGB
 * colour, so the index is the only lever the backdrop has over the blend.
 * BrMaterialAllocate()'s default, 10, is a near-black grey whose table row is a
 * few steps from the clear colour's row 0, which is why the red backdrop moved
 * no cube pixel by more than five steps and the blend stayed unreadable.
 *
 * 224 is one of the texture's own palette entries: scene_texture() paints it
 * the orange (220,120,32), and the grey ramp the 8bpp export resolves indices
 * through makes it the brightest of the two saturated entries that palette
 * carries. Its blend-table row is 64 + ((224 + src) >> 1) against the default's
 * 64 + ((10 + src) >> 1), and 224 - 10 is even, so a blend moves its source by
 * exactly (224 - 10) >> 1 = 107 index steps whatever shade path produced it -
 * measured on every cube pixel of all ten scenes. Where a second cube blends
 * over the first, its destination is the first layer's output and it moves by
 * half that. The unblended control keeps its own cube pixels, and the backdrop
 * itself moves 10 -> 224 in the same frame.
 *
 * The p256 fixtures keep the default index, not this one: their blend is the
 * RGB 50/50 destination blend the opacity knob reaches, not the indexed table,
 * and their four fixtures are the guard that only the ten indexed blend scenes
 * moved.
 */
#define SCENE_BACKDROP_INDEX     224
#define SCENE_BACKDROP_INDEX_RGB 10

/*
 * Every lookup table is indexed as (row * 256) + column whatever its height, so
 * the width is not a free parameter. An empty table is still a valid table; the
 * builders below fill one in.
 */
static br_pixelmap *scene_table_alloc(const char *name, int rows)
{
    br_pixelmap *pm;

    if((pm = BrPixelmapAllocate(BR_PMT_INDEX_8, SCENE_TABLE_WIDTH, rows, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, name);
    BrMemSet(pm->pixels, 0, (br_size_t)pm->row_bytes * (br_size_t)pm->height);

    BrMapAdd(pm);

    return pm;
}

/*
 * A checkerboard texture with a palette, which is what the glTF exporter needs
 * in order to carry it as an image at all. Index 0 is left unused: the indexed
 * rasterisers treat a texel of 0 as transparent, and a fixture that is mostly
 * holes would not show whether the texel fetch worked.
 *
 * `cells` is the number of squares across; the two textured fixtures use
 * different counts so that they cannot land on the same pattern on screen for
 * reasons that have nothing to do with the address mode being tested.
 */
static br_pixelmap *scene_texture(const char *name, int width, int height, int cells)
{
    br_pixelmap *pm, *pal;
    br_uint_8   *pixels;
    int          cell_w = width / cells;
    int          cell_h = height / cells;

    if((pal = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    pal->identifier = BrResStrDup(pal, name);

    for(int i = 0; i < 256; ++i)
        ((br_colour *)pal->pixels)[i] = BR_COLOUR_RGB(i, i, i);

    /*
     * Two separable hues, so a channel-order or channel-drop failure names
     * itself instead of reading as a grey shift.
     */
    ((br_colour *)pal->pixels)[SCENE_TEX_INDEX_A] = BR_COLOUR_RGB(32, 160, 200);
    ((br_colour *)pal->pixels)[SCENE_TEX_INDEX_B] = BR_COLOUR_RGB(220, 120, 32);

    if((pm = BrPixelmapAllocate(BR_PMT_INDEX_8, width, height, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, name);
    pixels         = pm->pixels;

    for(int y = 0; y < height; ++y)
        for(int x = 0; x < width; ++x)
            pixels[(y * pm->row_bytes) + x] = (((x / cell_w) + (y / cell_h)) & 1) ? SCENE_TEX_INDEX_A : SCENE_TEX_INDEX_B;

    pm->map = pal;

    BrMapAdd(pal);
    BrMapAdd(pm);

    return pm;
}

/*
 * A 32x32 texture whose every row differs from every other and whose every
 * column does too, so that a texel fetched from the wrong row or column cannot
 * land on the same index and hide itself. The 32x32 z-sorted block's defect is
 * a wrong seed address, and the checkerboard above repeats every few rows: a
 * fixture that sampled it would still match the stored reference, which is the
 * failure mode the fixture exists to rule out.
 *
 * Every byte is in 1..255 (index 0 is the rasterisers' transparent texel), and
 * the palette is the identity grey ramp, which is what lets the glTF exporter
 * carry the indices through a PNG unchanged.
 */
static br_pixelmap *scene_texture_rows(const char *name, int width, int height)
{
    br_pixelmap *pm, *pal;
    br_uint_8   *pixels;

    if((pal = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    pal->identifier = BrResStrDup(pal, name);

    for(int i = 0; i < 256; ++i)
        ((br_colour *)pal->pixels)[i] = BR_COLOUR_RGB(i, i, i);

    if((pm = BrPixelmapAllocate(BR_PMT_INDEX_8, width, height, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, name);
    pixels         = pm->pixels;

    for(int y = 0; y < height; ++y)
        for(int x = 0; x < width; ++x)
            pixels[(y * pm->row_bytes) + x] = (br_uint_8)(1 + ((x * 3 + y * 7) % 255));

    pm->map = pal;

    BrMapAdd(pal);
    BrMapAdd(pm);

    return pm;
}

/*
 * The shade table is indexed shade[intensity][texel]: row = surface intensity,
 * column = the texture index, and the byte is the output index.
 *
 * The ramp is in the intensity and the texel index is carried through with a
 * fixed lift, which is what a real ramp between palette entries does. It matters
 * that neither axis can drive the output to zero for a column the texture
 * actually uses: an INDEX_8 frame treats index 0 as transparent, so a table that
 * collapses some of the texture to zero would read as the texture fetch being
 * broken.
 */
static br_pixelmap *scene_shade_table(const char *name)
{
    br_pixelmap *pm;
    br_uint_8   *pixels;

    if((pm = scene_table_alloc(name, SCENE_TABLE_ROWS)) == NULL)
        return NULL;

    pixels = pm->pixels;

    for(int row = 0; row < SCENE_TABLE_ROWS; ++row)
        for(int col = 0; col < SCENE_TABLE_WIDTH; ++col) {
            int v = ((row * 96) / 255) + (col * 32);

            if(v > 255)
                v = 255;

            pixels[(row * pm->row_bytes) + col] = (br_uint_8)v;
        }

    return pm;
}

/*
 * The untextured shade-table block has no texel to index with, so its "column"
 * is derived from the same intensity the row is, and the table is a remap of the
 * intensity onto the output range. That is all the untextured block can use. The
 * remap is deliberately not the identity: a table that reproduces the unshaded
 * frame exactly would make "the table was bound" unpublishable.
 */
static br_pixelmap *scene_shade_ramp(const char *name)
{
    br_pixelmap *pm;
    br_uint_8   *pixels;

    if((pm = scene_table_alloc(name, SCENE_TABLE_ROWS)) == NULL)
        return NULL;

    pixels = pm->pixels;

    for(int row = 0; row < SCENE_TABLE_ROWS; ++row)
        for(int col = 0; col < SCENE_TABLE_WIDTH; ++col)
            pixels[(row * pm->row_bytes) + col] = (br_uint_8)((row / 2) + 32);

    return pm;
}

/*
 * A shade table whose samples are an output colour rather than an index: the
 * RGB-output half of the shade-table family. A shade table's type must equal the
 * output format, and the RGB-output kernel writes the table's sample straight to
 * the colour buffer (`out = shade[(intensity << 8) | texel]`), so the table is
 * one of the three RGB pixel types, not INDEX_8.
 *
 * The sample carries two axes so that the frame can be read: the red channel is
 * the intensity (the row), and the green channel is the texel index (the column).
 * A table that varied only with the intensity would be indistinguishable from the
 * untextured intensity path, and a dropped table read would look like a working
 * one.
 *
 * The samples are packed here rather than through BrPixelmapPixelSet(): that
 * writes a br_colour truncated to the pixel's byte width, which for a 16-bit
 * type keeps only the low two bytes of 0x00RRGGBB - the blue and green channels -
 * and silently drops the red one. The packing below is the layout the rasterisers
 * read and write (ScalarsToRGB15/16 for the 16-bit types).
 */
static void scene_shade_table_rgb_set(br_pixelmap *pm, int x, int y, br_uint_8 type, br_uint_8 r, br_uint_8 g, br_uint_8 b)
{
    br_uint_8 *p = (br_uint_8 *)pm->pixels + (br_size_t)y * pm->row_bytes + (br_size_t)x * ((type == BR_PMT_RGB_888) ? 3 : 2);

    switch(type) {
        case BR_PMT_RGB_555: {
            br_uint_16 v = (br_uint_16)(((br_uint_16)(r >> 3) << 10) | ((br_uint_16)(g >> 3) << 5) | (br_uint_16)(b >> 3));

            p[0] = (br_uint_8)v;
            p[1] = (br_uint_8)(v >> 8);
            break;
        }

        case BR_PMT_RGB_565: {
            br_uint_16 v = (br_uint_16)(((br_uint_16)(r >> 3) << 11) | ((br_uint_16)(g >> 2) << 5) | (br_uint_16)(b >> 3));

            p[0] = (br_uint_8)v;
            p[1] = (br_uint_8)(v >> 8);
            break;
        }

        default:
            /* RGB_888 is byte-ordered B,G,R in memory. */
            p[0] = b;
            p[1] = g;
            p[2] = r;
            break;
    }
}

static br_pixelmap *scene_shade_table_rgb(const char *name, br_uint_8 type)
{
    br_pixelmap *pm;

    if((pm = BrPixelmapAllocate(type, SCENE_TABLE_WIDTH, SCENE_TABLE_ROWS, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, name);

    for(int row = 0; row < SCENE_TABLE_ROWS; ++row)
        for(int col = 0; col < SCENE_TABLE_WIDTH; ++col)
            scene_shade_table_rgb_set(pm, col, row, type, (br_uint_8)row, (br_uint_8)col, 96);

    BrMapAdd(pm);

    return pm;
}

/*
 * A colour map typed to an RGB output: the map a 555/565/888 textured block
 * samples directly. It has no palette, because the rasteriser reads the map's
 * own words and a palette would be an indirection the block does not perform.
 *
 * It exists because the block's texture type is the output type. infogen.pl's
 * shared_texture() sets a plain `texture` block's texture_type from the block's
 * colour type, so the 15bpp block that a textured, unshaded
 * primitive selects requires a BR_PMT_RGB_555 map. Every colour map the corpus
 * had was INDEX_8 (either with a palette or as a marked table), so nothing could
 * ever satisfy that requirement. The two hues are scene_texture()'s, so a
 * channel-order or channel-drop failure reads the same way on either map.
 */
static br_pixelmap *scene_texture_rgb(const char *name, br_uint_8 type, int width, int height, int cells)
{
    br_pixelmap *pm;
    int          cell_w = width / cells;
    int          cell_h = height / cells;

    if((pm = BrPixelmapAllocate(type, width, height, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, name);

    for(int y = 0; y < height; ++y)
        for(int x = 0; x < width; ++x) {
            br_boolean a = (((x / cell_w) + (y / cell_h)) & 1) != 0;

            /* RGB_555 is byte-ordered low,high; RGB_565 the same; RGB_888 B,G,R. */
            scene_shade_table_rgb_set(pm, x, y, type, a ? 32 : 220, a ? 160 : 120, a ? 200 : 32);
        }

    BrMapAdd(pm);

    return pm;
}

/*
 * The shade-table fixtures' texture: an INDEX_8 map with no palette, so its two
 * indices survive the glTF round trip exactly (the exporter's index8 marker)
 * instead of being requantised to whatever indices the loader's palette chose.
 * That keeps the shade table's column - the texel byte - readable in the frame,
 * and it is honest about what the data is: the RGB-output shade kernel reads the
 * texel byte as the table's column and never expands a palette. Index 0 is left
 * unused for the same reason as scene_texture().
 */
static br_pixelmap *scene_shade_texture_size(const char *name, int size)
{
    br_pixelmap *pm;
    br_uint_8   *pixels;
    const int    cell = size / SCENE_TEX_CELLS;

    if((pm = BrPixelmapAllocate(BR_PMT_INDEX_8, size, size, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    pm->identifier = BrResStrDup(pm, name);
    pixels         = pm->pixels;

    for(int y = 0; y < size; ++y)
        for(int x = 0; x < size; ++x)
            pixels[(y * pm->row_bytes) + x] = (((x / cell) + (y / cell)) & 1) ? SCENE_TEX_INDEX_A : SCENE_TEX_INDEX_B;

    BrMapAdd(pm);

    return pm;
}

static br_pixelmap *scene_shade_texture(const char *name)
{
    return scene_shade_texture_size(name, SCENE_TEX_SIZE);
}

/*
 * Blend is indexed blend[dst][src]: row = the pixel already in the colour
 * buffer, column = the shaded texel. The byte is a palette index, not a colour,
 * so the table is free to name a band of its own - and it has to, because a
 * plain average of the two or three indices a fixture's texture occupies lands
 * back on one of them and a blended pixel would be indistinguishable from an
 * unblended one.
 */
static br_pixelmap *scene_blend_table(const char *name)
{
    br_pixelmap *pm;
    br_uint_8   *pixels;

    if((pm = scene_table_alloc(name, SCENE_TABLE_ROWS)) == NULL)
        return NULL;

    pixels = pm->pixels;

    for(int row = 0; row < SCENE_TABLE_ROWS; ++row)
        for(int col = 0; col < SCENE_TABLE_WIDTH; ++col) {
            int v = SCENE_BLEND_BASE + ((row + col) >> 1);

            if(v > 255)
                v = 255;

            pixels[(row * pm->row_bytes) + col] = (br_uint_8)v;
        }

    return pm;
}

/*
 * Fog is indexed fog[level][shaded]; the level is the high byte of the
 * interpolated z and the ramp is linear over the whole byte, so how much fog a
 * pixel gets is decided by the fixture's camera range rather than by the table.
 * It fades towards a non-zero index so that a fully fogged pixel still counts as
 * drawn - index 0 is the transparent end of the palette.
 */
#define SCENE_FOG_INDEX 48

static br_pixelmap *scene_fog_table(const char *name)
{
    br_pixelmap *pm;
    br_uint_8   *pixels;

    if((pm = scene_table_alloc(name, SCENE_TABLE_ROWS)) == NULL)
        return NULL;

    pixels = pm->pixels;

    for(int row = 0; row < SCENE_TABLE_ROWS; ++row)
        for(int col = 0; col < SCENE_TABLE_WIDTH; ++col)
            pixels[(row * pm->row_bytes) + col] = (br_uint_8)(((col * (255 - row)) + (SCENE_FOG_INDEX * row)) / 255);

    return pm;
}

/*
 * A point light close enough to the cube that the intensity varies within a
 * single face. That is what makes flat and interpolated shading produce
 * different frames: with a light far enough away each face gets one value and
 * the two shading modes agree by construction.
 */
static br_actor *scene_add_near_light(br_actor *world)
{
    br_actor *a;
    br_light *l;

    if((a = scene_actor(world, BR_ACTOR_LIGHT, "near")) == NULL)
        return NULL;

    l = a->type_data;

    l->identifier    = a->identifier;
    l->type          = BR_LIGHT_POINT | BR_LIGHT_LINEAR_FALLOFF;
    l->colour        = BR_COLOUR_RGB(255, 255, 255);
    l->attenuation_c = BR_SCALAR(1.0);
    l->radius_inner  = BR_SCALAR(0.0);
    l->radius_outer  = SCENE_NEAR_LIGHT_RADIUS;

    scene_place_and_aim(a, BR_SCALAR(2.0), BR_SCALAR(2.0), BR_SCALAR(2.0), BR_TRUE);

    return a;
}

/*
 * The feature fixtures share one lit cube, one light and one index band, so that
 * the only thing that differs between two of them is the one named in the
 * comment above it.
 */
/*
 * The whole index byte, so that a PPM dump of the frame - which resolves the
 * colour buffer through a grey ramp built from the material's index band - is
 * the index buffer and can be read directly.
 */
#define SCENE_FX_BASE  0
#define SCENE_FX_RANGE 255

static br_material *scene_fx_material(const char *name, br_uint_32 flags)
{
    br_material *m;

    if((m = scene_material_ex(name, BR_COLOUR_RGB(200, 200, 200), flags, BR_SCALAR(0.1), BR_SCALAR(0.7), BR_SCALAR(0.0), BR_SCALAR(20.0))) == NULL)
        return NULL;

    m->index_base  = SCENE_FX_BASE;
    m->index_range = SCENE_FX_RANGE;

    return m;
}

/*
 * The cube is turned so that no visible face is parallel to the screen. A
 * fronto-parallel face has a constant w across it, which makes the perspective
 * setup's cheat fire and the affine rasteriser render it - so an unturned cube
 * cannot tell the affine and perspective texture paths apart.
 *
 * `scale` is almost always SCENE_FX_SCALE. scene-tex-32 enlarges the cube: the
 * 32x32 z-sorted perspective block's defect is in the texel address its first
 * span starts from, and that address is corrected by the trapezium's u/v
 * correction loops when the projected texel fractions are small - which they
 * are on the 1.5x cube. A cube large enough that the spans start with large
 * fractions is what leaves the error in place. See scene-tex-32 below.
 */
static br_actor *scene_fx_world_range(br_model *cube, br_material *mat, br_scalar scale, br_scalar hither, br_scalar yon)
{
    br_actor   *world = scene_world(cube, mat, BR_SCALAR(1.0));
    br_actor   *a;
    br_matrix34 m;

    if(world == NULL)
        return NULL;

    for(a = world->children; a != NULL; a = a->next)
        if(a->type == BR_ACTOR_MODEL)
            break;

    if(a != NULL) {
        BrMatrix34Scale(&m, scale, scale, scale);
        BrMatrix34PostRotateX(&m, BR_ANGLE_DEG(20));
        BrMatrix34PostRotateY(&m, BR_ANGLE_DEG(30));

        a->t.type  = BR_TRANSFORM_MATRIX34;
        a->t.t.mat = m;
    }

    scene_add_near_light(world);
    scene_set_camera_range(world, hither, yon);

    return world;
}

static br_actor *scene_fx_world(br_model *cube, br_material *mat)
{
    return scene_fx_world_range(cube, mat, SCENE_FX_SCALE, BR_SCALAR(0.1), BR_SCALAR(100.0));
}

/*
 * scene_fx_world() with a render style on the cube actor, for the topology
 * witnesses below.
 *
 * The style goes on the model actor rather than the world so that the camera
 * and the light actors keep the default; the renderer inherits a style down the
 * tree, so either would reach the cube.
 */
static br_actor *scene_fx_world_topology(br_model *cube, br_material *mat, br_uint_8 render_style)
{
    br_actor *world = scene_fx_world(cube, mat);

    if(world == NULL)
        return NULL;

    for(br_actor *a = world->children; a != NULL; a = a->next) {
        if(a->type == BR_ACTOR_MODEL) {
            a->render_style = render_style;
            break;
        }
    }

    return world;
}

/*
 * scene_fx_world() with two cube actors sharing one material: the first drawn
 * as edges, the second as points. The two sit at the same transform and differ
 * only in render_style, so one material state witnesses both topologies in a
 * single scene - which is what makes the line/point fixtures five material
 * states rather than ten.
 *
 * The point actor copies the edge actor's transform rather than taking the
 * identity: at the identity it would be a unit cube at the origin, i.e. a
 * different shape in a different place, and the frame would no longer be the
 * same rig scene-edges and scene-points use.
 *
 * Both actors also draw at the same place when the style is dropped, so a
 * fixture whose render_style never reached the loader renders as scene-flat
 * does - which is what the scene-edges/scene-points relation asserts today.
 */
static br_actor *scene_fx_world_topologies(br_model *cube, br_material *mat)
{
    br_actor *world = scene_fx_world(cube, mat);
    br_actor *edges = NULL, *points;

    if(world == NULL)
        return NULL;

    for(br_actor *a = world->children; a != NULL; a = a->next) {
        if(a->type == BR_ACTOR_MODEL) {
            edges = a;
            break;
        }
    }

    if(edges == NULL)
        return world;

    edges->render_style = BR_RSTYLE_EDGES;

    if((points = scene_actor(world, BR_ACTOR_MODEL, "cube-points")) == NULL)
        return world;

    points->model        = cube;
    points->material     = mat;
    points->render_style = BR_RSTYLE_POINTS;
    points->t            = edges->t;

    return world;
}

/*
 * A cube at an explicit scale and place, named so that a several-cube scene
 * can say why each is there. scene_fx_add_cube() is this at the feature scale.
 */
static void scene_fx_add_cube_named(br_actor *world, br_model *cube, br_material *mat, const char *name, br_scalar scale, br_scalar x,
                                    br_scalar y, br_scalar z)
{
    br_actor   *a;
    br_matrix34 m;

    if((a = scene_actor(world, BR_ACTOR_MODEL, name)) == NULL)
        return;

    a->model    = cube;
    a->material = mat;

    BrMatrix34Scale(&m, scale, scale, scale);
    BrMatrix34PostRotateX(&m, BR_ANGLE_DEG(-15));
    BrMatrix34PostTranslate(&m, x, y, z);

    a->t.type  = BR_TRANSFORM_MATRIX34;
    a->t.t.mat = m;
}

/*
 * A second cube, nearer the camera and offset to one side so that it overlaps
 * the first on screen. The indexed blend reads the pixel already in the colour
 * buffer, and with z buffering a single convex model writes every pixel exactly
 * once - so a blend has to have something in front of it to mix with. Without
 * this the fixture would render identically to scene-textured.
 */
static void scene_fx_add_cube(br_actor *world, br_model *cube, br_material *mat, br_scalar x, br_scalar y, br_scalar z)
{
    scene_fx_add_cube_named(world, cube, mat, "cube-2", SCENE_FX_SCALE, x, y, z);
}

/*
 * ------------------------------------------------------------------
 * The MMX fixture family.
 *
 * pentprim walks the MMX 555/565 table before the general one, and that table
 * carries a family the corpus never reaches: the dithered, screendoor and
 * dithered-screendoor twins of the textured, flat, gouraud and untextured
 * blocks.
 *
 * Three properties of the family force the design:
 *
 *  - The screendoor level is the material alpha byte (`UNPACK_SCREENDOOR_ALPHA`,
 *    `_c >> 28`). At the default opacity 255 that byte is zero, the mask row is
 *    sixteen zero words, and the kernel is entered and writes nothing - so every
 *    screendoor cube carries an opacity below 255.
 *  - The untextured blocks match only when no colour map is bound, and the
 *    texture-only ones only when the material does not modulate - which means
 *    unlit *and* white, because BRT_MODULATE_B is set by the material colour as
 *    well as by BR_MATF_LIGHT.
 *  - Each flat/gouraud pair differs by BR_MATF_SMOOTH alone, and each
 *    affine/perspective pair by BR_MATF_PERSPECTIVE alone.
 *
 * One scene per cube state, not one scene holding several: a scene has one
 * checksum, so a scene of four states says only that one of the four changed.
 *
 * The cube, turn, lights and camera are the existing single-cube rig
 * (`scene_fx_world`), so the constraints the older feature fixtures carry are
 * inherited unchanged.
 */

/*
 * How far below full opacity a screendoor material is set. The level is the top
 * nibble of the material alpha byte, quantised into the mask rows, and any
 * value in 1..254 draws; 128 gives a half-density frame that is unmistakably
 * not the blank one 255 produces.
 */
#define SCENE_MMX_OPACITY 128

#define SCENE_MMX_GREY    BR_COLOUR_RGB(200, 200, 200)
#define SCENE_MMX_WHITE   BR_COLOUR_RGB(255, 255, 255)

/*
 * One cube state. `flags` is the material's whole BR_MATF_* set; the screendoor
 * family is the only one whose `opacity` is not 255, and `textured` selects
 * whether a colour map is bound - the untextured blocks match only when none is.
 */
typedef struct scene_mmx_fixture {
    const char *name;
    br_uint_32  flags;
    br_colour   colour;
    br_uint_8   opacity;
    br_boolean  textured;
} scene_mmx_fixture;

/*
 * The MMX kernel family, grouped by shading: untextured, texture-only, flat
 * coloured, gouraud. Within a group each state is a `_D` (dithered), `_S`
 * (screendoor) or `_SD` (both) variant, and the textured groups add the
 * affine/perspective split. The scene names spell the groups `rgb` (untextured),
 * `uv` (texture-only), `uvc` (flat-coloured) and `uvrgb` (gouraud).
 *
 * The columns are hand-aligned, so the table is held out of clang-format:
 * AlignArrayOfStructures would strip the padding and reflow the long rows.
 */
// clang-format off
static const scene_mmx_fixture scene_mmx_fixtures[] = {
    /* Untextured: no map at all. */
    {.name = "scene-mmx-rgb-dither-smooth", .flags = BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = 255, .textured = BR_FALSE},
    {.name = "scene-mmx-rgb-dither-flat", .flags = BR_MATF_LIGHT | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = 255, .textured = BR_FALSE},
    {.name = "scene-mmx-rgb-screen-flat", .flags = BR_MATF_LIGHT, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_FALSE},
    {.name = "scene-mmx-rgb-ditherscreen-smooth", .flags = BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_FALSE},
    {.name = "scene-mmx-rgb-ditherscreen-flat", .flags = BR_MATF_LIGHT | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_FALSE},

    /* Texture-only: unlit white, so PRIMF_MODULATE is clear. */
    {.name = "scene-mmx-uv-dither-persp", .flags = BR_MATF_PERSPECTIVE | BR_MATF_DITHER, .colour = SCENE_MMX_WHITE, .opacity = 255, .textured = BR_TRUE},
    {.name = "scene-mmx-uv-dither-affine", .flags = BR_MATF_DITHER, .colour = SCENE_MMX_WHITE, .opacity = 255, .textured = BR_TRUE},
    {.name = "scene-mmx-uv-screen-persp", .flags = BR_MATF_PERSPECTIVE, .colour = SCENE_MMX_WHITE, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},
    {.name = "scene-mmx-uv-screen-affine", .flags = 0, .colour = SCENE_MMX_WHITE, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},
    {.name = "scene-mmx-uv-ditherscreen-persp", .flags = BR_MATF_PERSPECTIVE | BR_MATF_DITHER, .colour = SCENE_MMX_WHITE, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},
    {.name = "scene-mmx-uv-ditherscreen-affine", .flags = BR_MATF_DITHER, .colour = SCENE_MMX_WHITE, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},

    /* Flat-coloured texture. */
    {.name = "scene-mmx-uvc-persp", .flags = BR_MATF_LIGHT | BR_MATF_PERSPECTIVE, .colour = SCENE_MMX_GREY, .opacity = 255, .textured = BR_TRUE},
    {.name = "scene-mmx-uvc-dither-persp", .flags = BR_MATF_LIGHT | BR_MATF_PERSPECTIVE | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = 255, .textured = BR_TRUE},
    {.name = "scene-mmx-uvc-dither-affine", .flags = BR_MATF_LIGHT | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = 255, .textured = BR_TRUE},
    {.name = "scene-mmx-uvc-screen-persp", .flags = BR_MATF_LIGHT | BR_MATF_PERSPECTIVE, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},
    {.name = "scene-mmx-uvc-screen-affine", .flags = BR_MATF_LIGHT, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},
    {.name = "scene-mmx-uvc-ditherscreen-persp", .flags = BR_MATF_LIGHT | BR_MATF_PERSPECTIVE | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},
    {.name = "scene-mmx-uvc-ditherscreen-affine", .flags = BR_MATF_LIGHT | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},

    /* Gouraud-textured. */
    {.name = "scene-mmx-uvrgb-dither-affine", .flags = BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = 255, .textured = BR_TRUE},
    {.name = "scene-mmx-uvrgb-screen-persp", .flags = BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},
    {.name = "scene-mmx-uvrgb-ditherscreen-persp", .flags = BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},
    {.name = "scene-mmx-uvrgb-ditherscreen-affine", .flags = BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_DITHER, .colour = SCENE_MMX_GREY, .opacity = SCENE_MMX_OPACITY, .textured = BR_TRUE},
};
// clang-format on

static br_error scene_make_mmx_fixtures(br_model *cube)
{
    br_pixelmap *map;
    br_error     r = BRE_OK;

    if((map = scene_texture("scene-mmx-map", SCENE_TEX_SIZE, SCENE_TEX_SIZE, SCENE_TEX_CELLS)) == NULL) {
        fprintf(stderr, "failed to allocate the mmx fixture map\n");
        return BRE_FAIL;
    }

    for(size_t i = 0; i < BR_ASIZE(scene_mmx_fixtures); ++i) {
        const scene_mmx_fixture *fx = &scene_mmx_fixtures[i];
        char                     mat[128];
        char                     gltf[128];
        br_material             *m;
        br_actor                *world;

        snprintf(mat, sizeof(mat), "%s-material", fx->name);
        snprintf(gltf, sizeof(gltf), "%s.gltf", fx->name);

        if((m = scene_material_ex(mat, fx->colour, fx->flags, BR_SCALAR(0.1), BR_SCALAR(0.7), BR_SCALAR(0.0), BR_SCALAR(20.0))) == NULL)
            return BRE_FAIL;

        m->index_base  = SCENE_FX_BASE;
        m->index_range = SCENE_FX_RANGE;
        m->opacity     = fx->opacity;
        m->colour_map  = fx->textured ? map : NULL;

        if((world = scene_fx_world(cube, m)) == NULL || scene_save(gltf, world) != BRE_OK)
            r = BRE_FAIL;
    }

    return r;
}

/*
 * ------------------------------------------------------------------
 * The line and point material fixtures.
 *
 * One scene witnesses both topologies: render_style is per actor, so a scene
 * can hold an edge actor and a point actor sharing one material
 * (scene_fx_world_topologies).
 *
 *   scene-lines-plain         lit flat, no map            - the control
 *   scene-lines-gouraud       + BR_MATF_SMOOTH
 *   scene-lines-map           lit flat, indexed map + shade table
 *   scene-lines-map-gouraud   + BR_MATF_SMOOTH
 *   scene-lines-plain-unlit   unlit white, no map        - the second control
 *   scene-lines-map-unlit     unlit white, indexed map
 *   scene-lines-rgb555        unlit white, 555-typed map - the TEX_555 witness
 *   scene-lines-rgb565        unlit white, 565-typed map - the TEX_565 witness
 *
 * Two properties decide whether these draw the block they name:
 *
 *  - The gouraud fixtures need the near light scene_fx_world() adds. A light
 *    far enough away gives each face one intensity, and the flat and
 *    interpolated kernels then produce the same frame.
 *  - The unlit fixtures must be unlit *and* white: a texture-only block is one
 *    whose entry carries no MODULATE requirement, and PRIMF_MODULATE is set by
 *    the material colour as well as by BR_MATF_LIGHT, so a lit grey material
 *    matches the shaded block instead.
 *
 * The map is scene-tex-arb's shape, an indexed checkerboard of arbitrary width
 * (96x48): the line and point family has no power-of-two blocks, so every one
 * of these states is an ADDR_DIVIDE tuple and a power-of-two map would reach
 * nothing. The palette carries it through the 24bpp run, where the loader
 * re-types an indexed map as an RGB_888 one.
 *
 * The indexed shade table is bound only where a tree entry requires one: the
 * line/point INTERP_I/TEX_I8 and CONST_I/TEX_I8 blocks declare shade_type
 * INDEX_8, and pentprim's LineRenderPITI reads work.shade_table unguarded, so a
 * scene without one would match and then read a NULL pointer.
 *
 * The index band is not scene_fx_material()'s: an indexed frame treats index 0
 * as transparent and an unlit untextured primitive's whole intensity is the
 * band's base, so SCENE_FX_BASE's 0 draws nothing at 8bpp. A base above zero
 * keeps the control visible, and these fixtures carry one.
 */
#define SCENE_LINES_BASE  16
#define SCENE_LINES_RANGE 224
static br_material *scene_lines_material(const char *name, br_colour colour, br_uint_32 flags, br_pixelmap *map, br_pixelmap *shade)
{
    br_material *m;

    if((m = scene_material_ex(name, colour, flags, BR_SCALAR(0.1), BR_SCALAR(0.7), BR_SCALAR(0.0), BR_SCALAR(20.0))) == NULL)
        return NULL;

    m->index_base  = SCENE_LINES_BASE;
    m->index_range = SCENE_LINES_RANGE;
    m->colour_map  = map;
    m->index_shade = shade;

    return m;
}

static br_error scene_make_line_fixtures(br_model *cube)
{
    br_pixelmap *map, *shade;
    br_error     r = BRE_OK;

    if((map = scene_texture("scene-lines-map", SCENE_ARB_WIDTH, SCENE_ARB_HEIGHT, SCENE_ARB_CELLS)) == NULL ||
       (shade = scene_shade_table("scene-lines-shade-table")) == NULL) {
        fprintf(stderr, "failed to allocate the line fixture resources\n");
        return BRE_FAIL;
    }

    /* The controls: the plain material each of the fixtures below is one knob away from. */
    if(scene_save("scene-lines-plain.gltf",
                  scene_fx_world_topologies(cube, scene_lines_material("scene-lines-plain-material", BR_COLOUR_RGB(200, 200, 200),
                                                                       BR_MATF_LIGHT, NULL, NULL))) != BRE_OK)
        r = BRE_FAIL;

    if(scene_save("scene-lines-plain-unlit.gltf",
                  scene_fx_world_topologies(cube, scene_lines_material("scene-lines-plain-unlit-material", BR_COLOUR_RGB(255, 255, 255), 0,
                                                                       NULL, NULL))) != BRE_OK)
        r = BRE_FAIL;

    /* Constant intensity, no map: scene-lines-plain plus BR_MATF_SMOOTH. */
    if(scene_save("scene-lines-gouraud.gltf",
                  scene_fx_world_topologies(cube, scene_lines_material("scene-lines-gouraud-material", BR_COLOUR_RGB(200, 200, 200),
                                                                       BR_MATF_LIGHT | BR_MATF_SMOOTH, NULL, NULL))) != BRE_OK)
        r = BRE_FAIL;

    /* Constant intensity, indexed map and shade table: scene-lines-plain plus the map. */
    if(scene_save("scene-lines-map.gltf",
                  scene_fx_world_topologies(cube, scene_lines_material("scene-lines-map-material", BR_COLOUR_RGB(200, 200, 200),
                                                                       BR_MATF_LIGHT, map, shade))) != BRE_OK)
        r = BRE_FAIL;

    /* Interpolated intensity through the same map: scene-lines-map plus BR_MATF_SMOOTH. */
    if(scene_save("scene-lines-map-gouraud.gltf",
                  scene_fx_world_topologies(cube, scene_lines_material("scene-lines-map-gouraud-material", BR_COLOUR_RGB(200, 200, 200),
                                                                       BR_MATF_LIGHT | BR_MATF_SMOOTH, map, shade))) != BRE_OK)
        r = BRE_FAIL;

    /* Texture-only: scene-lines-plain-unlit plus the map, and no shade table. */
    if(scene_save("scene-lines-map-unlit.gltf",
                  scene_fx_world_topologies(
                      cube, scene_lines_material("scene-lines-map-unlit-material", BR_COLOUR_RGB(255, 255, 255), 0, map, NULL))) != BRE_OK)
        r = BRE_FAIL;

    /*
     * The 555/565-typed maps. The line/point TEX_555 and TEX_565 entries require
     * nothing but the topology and the map's type - no match flags at all - so
     * these are scene-lines-map-unlit's material with the map typed to the
     * output, and no shade table. Unlit is what keeps the walk on them: a lit
     * material sets PRIMF_MODULATE, which the earlier untextured blocks that
     * require it would take first.
     *
     * The map stays arbitrary-width: this family has no power-of-two blocks, so
     * the entries are all ADDR_DIVIDE and a power-of-two map would reach the
     * untextured twin instead.
     */
    {
        br_pixelmap *map555 = scene_texture_rgb("scene-lines-rgb555-map", BR_PMT_RGB_555, SCENE_ARB_WIDTH, SCENE_ARB_HEIGHT, SCENE_ARB_CELLS);
        br_pixelmap *map565 = scene_texture_rgb("scene-lines-rgb565-map", BR_PMT_RGB_565, SCENE_ARB_WIDTH, SCENE_ARB_HEIGHT, SCENE_ARB_CELLS);

        if(map555 == NULL || map565 == NULL)
            return BRE_FAIL;

        if(scene_save("scene-lines-rgb555.gltf",
                      scene_fx_world_topologies(cube, scene_lines_material("scene-lines-rgb555-material", BR_COLOUR_RGB(255, 255, 255), 0,
                                                                           map555, NULL))) != BRE_OK)
            r = BRE_FAIL;

        if(scene_save("scene-lines-rgb565.gltf",
                      scene_fx_world_topologies(cube, scene_lines_material("scene-lines-rgb565-material", BR_COLOUR_RGB(255, 255, 255), 0,
                                                                           map565, NULL))) != BRE_OK)
            r = BRE_FAIL;
    }

    return r;
}

/*
 * ------------------------------------------------------------------
 * The backdrop the blend scenes draw behind their cubes.
 *
 * The blend the blend fixtures reach is a mix against the *destination* buffer,
 * and against the black clear colour that only halves the cube's own colour - a
 * darker copy of the non-blend twin, which no human reads as blending. A
 * saturated red drawn underneath makes the same pixels half red on the RGB
 * outputs, an obvious mixture - the same cube pixels, but a colour a person
 * reads at a glance. It is unlit so its colour is the material's and does not
 * vary with the near light, untextured so the mixture is between two flat
 * colours, and opaque whatever the cubes' opacity is: a backdrop that itself
 * blended would mix with black and defeat the point.
 *
 * On an INDEX_8 output there is no colour to see: the primitive's RGB is
 * discarded and the destination buffer holds an index, so the backdrop's lever
 * is its index_base - see SCENE_BACKDROP_INDEX. The actor and the material are
 * the same on both paths.
 *
 * The actor is a wide, shallow, unturned cube rather than the turned one
 * scene_fx_add_cube_named() makes, seated at the place SCENE_BACKDROP_* names.
 * It is added after the cubes, which puts it at the head of the world's child
 * list and so draws it first - the blend needs it in the colour buffer before
 * the cube that reads it. Under z-sorting its depth puts it first regardless.
 *
 * This is legibility, not coverage: the census already witnesses every blend
 * tuple these scenes reach, and no block selection moves. The backdrop only
 * gives a reader's eye the index the destination blend is reading.
 */
static br_material *scene_backdrop_material(const char *name, br_uint_8 index)
{
    br_material *m;

    if((m = scene_material_ex(name, BR_COLOUR_RGB(255, 0, 0), 0, BR_SCALAR(0.1), BR_SCALAR(0.7), BR_SCALAR(0.0), BR_SCALAR(20.0))) == NULL)
        return NULL;

    m->opacity    = 255;
    m->index_base = index;

    return m;
}

static void scene_backdrop_actor(br_actor *world, br_model *cube, br_material *mat, const char *name, br_scalar scale_xy, br_scalar scale_z,
                                 br_scalar z)
{
    br_actor   *a;
    br_matrix34 m;

    if((a = scene_actor(world, BR_ACTOR_MODEL, name)) == NULL)
        return;

    a->model    = cube;
    a->material = mat;

    BrMatrix34Scale(&m, scale_xy, scale_xy, scale_z);
    BrMatrix34PostTranslate(&m, BR_SCALAR(0), BR_SCALAR(0), z);

    a->t.type  = BR_TRANSFORM_MATRIX34;
    a->t.t.mat = m;
}

/*
 * Add the backdrop for a scene named `scene`, at the given place, resolving to
 * index `index` on an INDEX_8 output. The material is named after the scene, so
 * each fixture carries its own and a diff of two fixtures still says which
 * scene drew it.
 */
static br_error scene_add_backdrop(br_actor *world, br_model *cube, const char *scene, br_uint_8 index, br_scalar scale_xy,
                                   br_scalar scale_z, br_scalar z)
{
    char         name[128];
    br_material *mat;

    snprintf(name, sizeof(name), "%s-backdrop-material", scene);

    if((mat = scene_backdrop_material(name, index)) == NULL)
        return BRE_FAIL;

    scene_backdrop_actor(world, cube, mat, "backdrop", scale_xy, scale_z, z);

    return BRE_OK;
}

/*
 * ------------------------------------------------------------------
 * The RGB-output arbitrary-width shade-table fixtures.
 *
 * prim_t24's RGB-typed shade-table blocks - out = shade[(intensity << 8) |
 * texel] - have a witness at 888 in scene-shade-rgb888, and prim_t15/t16's at
 * 555 and 565 in scene-shade-rgb555/565. All three are drawn at the 1.5x
 * feature scale, where the arbitrary-width trapezium's u/v correction absorbs
 * the difference between the CORRECT and AFFINE cells entirely, so every
 * CORRECT x DIVIDE cell is still unwitnessed and the flat 888 one is
 * unwitnessed beside them. These fixtures are the same material at
 * SCENE_ARB_SCALE, and they are three scenes:
 *
 *   scene-shade-arb-flat          constant intensity, no perspective flag
 *   scene-shade-arb-flat-persp    + BR_MATF_PERSPECTIVE
 *   scene-shade-arb-smooth-persp  + BR_MATF_SMOOTH as well
 *
 * A shade table's type must equal the output format (shared_texture() sets the
 * block's shade type from its colour type, and the matcher
 * tests shade_type against the bound table's), and a scene renders at every
 * output format in turn. So each scene carries three cubes - 555, 565 and 888 -
 * each with its own material and its own table, and each is the witness at its
 * own --bpp while the other two fall through to the untextured block. That is
 * measured rather than assumed: one scene's log names the 555 cell at 15bpp,
 * the 565 at 16 and the 888 at 24.
 *
 * The map is an INDEX_8 map with no palette, the shape scene-shade-rgb888
 * already uses. It has to stay INDEX_8 at every output: the blocks all require
 * texture type INDEX_8, and the loader re-types an indexed map that carries a
 * palette - so a palette here would leave the 24bpp run on the untextured
 * block. DIVIDE is then forced rather than chosen: the arbitrary-width family
 * is the only one of these cells the tables emit for 555/565 at all, and 888
 * has no SHIFT variant of any of them.
 *
 * The 565 and 888 cubes are offset from the centre one because three cubes
 * drawn at SCENE_ARB_SCALE overlap in the middle of the frame; the offsets keep
 * each cube's own pixels - and so each cube's own response to the perspective
 * flag - readable in the scene's checksum.
 *
 * The four p256 scenes additionally carry a backdrop, which the three arb
 * scenes do not - hence the parameter and the byte-identical arb regeneration
 * that guards it. The p256 scenes are the ones with a -blend twin, and their
 * blend is the reason the backdrop exists; see scene_add_backdrop().
 */
static br_error scene_make_rgb_shade(br_model *cube, const char *name, const char *mapname, int size, br_uint_32 flags, br_uint_8 opacity,
                                     br_boolean backdrop)
{
    br_uint_8    types[3] = {BR_PMT_RGB_555, BR_PMT_RGB_565, BR_PMT_RGB_888};
    br_scalar    xs[3]    = {BR_SCALAR(0.0), BR_SCALAR(-2.6), BR_SCALAR(2.6)};
    br_pixelmap *map      = scene_shade_texture_size(mapname, size);
    br_material *mats[3];
    br_actor    *world;
    char         gltf[128];

    if(map == NULL)
        return BRE_FAIL;

    for(int k = 0; k < 3; ++k) {
        char mname[64];

        snprintf(mname, sizeof(mname), "%s-%u-material", name, (unsigned)types[k]);

        if((mats[k] = scene_fx_material(mname, BR_MATF_LIGHT | flags)) == NULL)
            return BRE_FAIL;

        mats[k]->colour_map  = map;
        mats[k]->index_shade = scene_shade_table_rgb(mname, types[k]);
        mats[k]->opacity     = opacity;
    }

    world = scene_fx_world_range(cube, mats[0], SCENE_ARB_SCALE, BR_SCALAR(0.1), BR_SCALAR(100.0));

    if(world != NULL)
        scene_fx_add_cube_named(world, cube, mats[1], "cube-565", SCENE_ARB_SCALE, xs[1], BR_SCALAR(0), BR_SCALAR(0));

    if(world != NULL)
        scene_fx_add_cube_named(world, cube, mats[2], "cube-888", SCENE_ARB_SCALE, xs[2], BR_SCALAR(0), BR_SCALAR(0));

    if(backdrop && world != NULL) {
        if(scene_add_backdrop(world, cube, name, SCENE_BACKDROP_INDEX_RGB, SCENE_BACKDROP_SCALE_XY, SCENE_BACKDROP_SCALE_Z, SCENE_BACKDROP_Z) != BRE_OK)
            return BRE_FAIL;
    }

    snprintf(gltf, sizeof(gltf), "%s.gltf", name);

    return scene_save(gltf, world);
}

static br_error scene_make_rgb_shade_fixtures(br_model *cube)
{
    br_error r = BRE_OK;

    if(scene_make_rgb_shade(cube, "scene-shade-arb-flat", "scene-shade-arb-map", SCENE_TEX_SIZE, 0, 255, BR_FALSE) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_rgb_shade(cube, "scene-shade-arb-flat-persp", "scene-shade-arb-map", SCENE_TEX_SIZE, BR_MATF_PERSPECTIVE, 255, BR_FALSE) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_rgb_shade(cube, "scene-shade-arb-smooth-persp", "scene-shade-arb-map", SCENE_TEX_SIZE,
                            BR_MATF_PERSPECTIVE | BR_MATF_SMOOTH, 255, BR_FALSE) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_rgb_shade(cube, "scene-shade-p256-flat-persp", "scene-shade-p256-map", SCENE_P256_SIZE, BR_MATF_PERSPECTIVE, 255, BR_TRUE) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_rgb_shade(cube, "scene-shade-p256-smooth-persp", "scene-shade-p256-map", SCENE_P256_SIZE,
                            BR_MATF_PERSPECTIVE | BR_MATF_SMOOTH, 255, BR_TRUE) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_rgb_shade(cube, "scene-shade-p256-flat-persp-blend", "scene-shade-p256-map", SCENE_P256_SIZE, BR_MATF_PERSPECTIVE, 128,
                            BR_TRUE) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_rgb_shade(cube, "scene-shade-p256-smooth-persp-blend", "scene-shade-p256-map", SCENE_P256_SIZE,
                            BR_MATF_PERSPECTIVE | BR_MATF_SMOOTH, 128, BR_TRUE) != BRE_OK)
        r = BRE_FAIL;

    return r;
}

/*
 * ------------------------------------------------------------------
 * The INDEX_8 ROP cross-product fixtures.
 *
 * One fixture per cell of the cross-product of shading mode x addressing x
 * perspective for the blend, fog, decal and blend+fog operators, plus the two
 * plain `DIVIDE` columns and the two flat untextured blocks: one family of
 * question - which of the three indexed ROP tables the walk binds, in which
 * order, and through which intensity path.
 *
 * prim_t8.ifg lists the fogged-blended sections first, then blended, then
 * fogged, then decal, then plain, and each section requires the table it names;
 * the walk takes the first entry whose requirements the bound state meets. A
 * blend table bound alongside a fog table therefore selects the combined block.
 *
 * The axes:
 *
 *  - `depth` is the run, not the scene: rendertest renders every fixture with
 *    and without a depth buffer. The z-sorted half has no blend, fog or
 *    corrected-decal entries, so those cells are witnessed by the z-buffered
 *    run alone.
 *  - `shade` is the intensity path: a bound INDEX_8 shade table with
 *    BR_MATF_SMOOTH is INTERP_I, without it CONST_I, and no shade table at all
 *    is the texture-only NONE. The constant and interpolated entries require a
 *    shade table, so a fixture without one reaches NONE even when lit.
 *  - `addr` is the map's shape: a power-of-two map of the entry's own size is
 *    SHIFT, anything else is DIVIDE. That is what the three map sizes here are
 *    for.
 *  - `persp` is BR_MATF_PERSPECTIVE, `blend` is a bound index_blend table or
 *    BR_MATF_DECAL, and `fog` is a bound index_fog table.
 *
 * Every scene is the same rig - the feature cube, the near light and the turned
 * model that make the flat/gouraud and affine/corrected cells distinguishable -
 * so each differs from its neighbour by one knob. Where only the shading mode
 * separates cubes they share a scene, since each cube still selects its own
 * entry. A scene has one checksum, so a moved checksum says one of the cubes
 * moved, not which; the cubes are placed so a PPM dump separates them.
 *
 * The fog cells use scene-fog's narrowed 4..8 camera range: the indexed fog
 * level is the high byte of the interpolated z, and the default 0.1..100 range
 * puts the whole cube inside two or three levels. Every cell uses the near
 * light the feature rig adds, so that the flat and gouraud cells do not agree
 * by construction.
 */

/* scene-fog's camera range; see the note above. */
#define SCENE_ROP_FOG_HITHER BR_SCALAR(4.0)
#define SCENE_ROP_FOG_YON    BR_SCALAR(8.0)

/* The 256x256 map the affine power-of-two fog and the z-sorted decal cells need. */
#define SCENE_ROP_MAP_256 256

/*
 * The shading modes a scene's cubes can be drawn in, in the order the scenes
 * place them: the interpolated, constant-intensity and texture-only cells of the
 * shade axis. `shaded` binds the scene's INDEX_8 shade table, and `flags` is the
 * material's whole BR_MATF_* set on top of BR_MATF_LIGHT and the scene's own.
 */
typedef struct scene_rop_shade {
    const char *suffix;
    br_uint_32  flags;
    br_boolean  shaded;
} scene_rop_shade;

/* A scene's bit for each entry of scene_rop_shades. */
#define SCENE_ROP_SMOOTH 1
#define SCENE_ROP_FLAT   2
#define SCENE_ROP_TEX    4

/* clang-format off */
static const scene_rop_shade scene_rop_shades[] = {
    {.suffix = "smooth", .flags = BR_MATF_SMOOTH, .shaded = BR_TRUE},
    {.suffix = "flat",   .flags = 0,              .shaded = BR_TRUE},
    {.suffix = "tex",    .flags = 0,              .shaded = BR_FALSE},
};
/* clang-format on */

/*
 * Where the cubes of a scene sit, by how many there are. None of these overlap:
 * the cube is 1.5 units across on a camera six units out, so 1.3 apart is clear
 * of it and the three-cube row is spread to the frame's edges. The placement is
 * what lets a PPM dump - and so a reader - separate the states one scene's
 * checksum covers. The first cube is moved to its place after the rig is built,
 * because the rig puts it at the origin.
 */
static const br_scalar scene_rop_x1[] = {BR_SCALAR(0.0)};
static const br_scalar scene_rop_x2[] = {BR_SCALAR(-1.3), BR_SCALAR(1.3)};
static const br_scalar scene_rop_x3[] = {BR_SCALAR(-2.0), BR_SCALAR(0.0), BR_SCALAR(2.0)};

static const br_scalar *scene_rop_row(int n)
{
    switch(n) {
        case 2:
            return scene_rop_x2;
        case 3:
            return scene_rop_x3;
        default:
            return scene_rop_x1;
    }
}

/*
 * One cube of a ROP scene: the scene's map and the tables its entry of
 * scene_rop_shades names, plus the material flags the scene shares.
 */
static br_material *scene_rop_material(const char *scene, const scene_rop_shade *shade, br_uint_32 flags, br_pixelmap *map,
                                       br_pixelmap *shade_table, br_pixelmap *blend, br_pixelmap *fog)
{
    br_material *m;
    char         name[160];

    snprintf(name, sizeof(name), "%s-%s-material", scene, shade->suffix);

    if((m = scene_material_ex(name, BR_COLOUR_RGB(200, 200, 200), BR_MATF_LIGHT | shade->flags | flags, BR_SCALAR(0.1), BR_SCALAR(0.7),
                              BR_SCALAR(0.0), BR_SCALAR(20.0))) == NULL)
        return NULL;

    m->index_base  = SCENE_FX_BASE;
    m->index_range = SCENE_FX_RANGE;
    m->colour_map  = map;
    m->index_shade = shade->shaded ? shade_table : NULL;
    m->index_blend = blend;
    m->index_fog   = fog;

    /*
     * The fog table only reaches the frame if the material asks for local fog,
     * which is how scene-fog binds it. The range is the narrowed one: the
     * default puts the whole cube inside two or three fog levels.
     */
    if(fog != NULL) {
        m->fog_min = SCENE_ROP_FOG_HITHER;
        m->fog_max = SCENE_ROP_FOG_YON;
    }

    return m;
}

/*
 * A ROP scene. `cubes` selects the entries of scene_rop_shades it draws, `flags`
 * is what they share (BR_MATF_PERSPECTIVE, BR_MATF_DECAL), and the three tables
 * are bound to every cube the mask selects or to none - a fixture binds the
 * table it is named for and drops the ones it is a one-knob variant of.
 */
static br_error scene_rop_build(const char *name, br_model *cube, br_uint_8 cubes, br_uint_32 flags, br_pixelmap *map, br_pixelmap *shade,
                                br_pixelmap *blend, br_pixelmap *fog, br_boolean backdrop)
{
    br_material     *mats[BR_ASIZE(scene_rop_shades)];
    const br_scalar *xs;
    br_actor        *world;
    char             gltf[192];
    int              n = 0;

    for(int i = 0; i < BR_ASIZE(scene_rop_shades); ++i)
        if(cubes & (1u << i)) {
            if((mats[n] = scene_rop_material(name, &scene_rop_shades[i], flags, map, shade, blend, fog)) == NULL)
                return BRE_FAIL;
            ++n;
        }

    world = scene_fx_world_range(cube, mats[0], SCENE_FX_SCALE, fog != NULL ? SCENE_ROP_FOG_HITHER : BR_SCALAR(0.1),
                                 fog != NULL ? SCENE_ROP_FOG_YON : BR_SCALAR(100.0));

    if(world == NULL)
        return BRE_FAIL;

    xs = scene_rop_row(n);

    for(br_actor *a = world->children; a != NULL; a = a->next) {
        if(a->type != BR_ACTOR_MODEL)
            continue;

        BrMatrix34PostTranslate(&a->t.t.mat, xs[0], BR_SCALAR(0), BR_SCALAR(0));
        break;
    }

    for(int i = 1; i < n; ++i) {
        char suffix[64];

        snprintf(suffix, sizeof(suffix), "cube-%d", i);
        scene_fx_add_cube_named(world, cube, mats[i], suffix, SCENE_FX_SCALE, xs[i], BR_SCALAR(0), BR_SCALAR(0));
    }

    /*
     * The red backdrop the blend is read against, seated behind the cubes and
     * so drawn first. The fogged scenes bind a fog table and so run at the
     * narrowed 4..8 camera range, where only the near place is inside the far
     * plane; the rest use the wide one; see the SCENE_BACKDROP_* note above.
     */
    if(backdrop && scene_add_backdrop(world, cube, name, SCENE_BACKDROP_INDEX, fog != NULL ? SCENE_BACKDROP_FOG_SCALE_XY : SCENE_BACKDROP_SCALE_XY,
                                      fog != NULL ? SCENE_BACKDROP_FOG_SCALE_Z : SCENE_BACKDROP_SCALE_Z,
                                      fog != NULL ? SCENE_BACKDROP_FOG_Z : SCENE_BACKDROP_Z) != BRE_OK)
        return BRE_FAIL;

    snprintf(gltf, sizeof(gltf), "%s.gltf", name);

    return scene_save(gltf, world);
}

/*
 * The ROP fixtures. Each row is one scene: the map its cubes bind, the flags
 * they share, which of the three shading cubes it carries, and which tables are
 * bound. The rows are grouped by which operator is under test, and within a
 * group each neighbour differs by one knob - the perspective flag, the
 * addressing mode, or one table.
 *
 *   blend+fog   the ROP order as a combination; the only cells where the three
 *               operators meet in one primitive
 *   blend       one table, so a wrong order cannot show
 *   fog         scene-fog's neighbours: the corrected cells it never reached,
 *               the affine cells its 64 map cannot select, the flat twin of the
 *               untextured block it does, and the texture-only cube of the
 *               affine cell it carries the two shade-table cubes of
 *   plain       the two DIVIDE columns no fixture reached, the flat untextured
 *               shade-table block, and the z-sorted affine pair at the one size
 *               the table emits them at
 *   decal       scene-decal's neighbour at the arbitrary width, and the flat
 *               twin of that neighbour
 *
 * A row draws the cubes the note above does not exclude, so a family whose cell
 * was in dispute is carried by the cube that cube is: each scene stays a
 * one-knob neighbour of the others.
 *
 * The columns are hand-aligned, so the table is held out of clang-format:
 * AlignArrayOfStructures would strip the padding and reflow the long rows.
 */
// clang-format off
typedef struct scene_rop_fixture {
    const char *name;
    br_uint_8   map;       /* which map the cubes bind: 0 = 64x64, 1 = 256x256, 2 = arbitrary width, 3 = none */
    br_uint_32  flags;     /* shared by the scene's cubes, on top of BR_MATF_LIGHT */
    br_uint_8   cubes;     /* which of scene_rop_shades are drawn */
    br_boolean  blend;     /* bind the blend table */
    br_boolean  fog;       /* bind the fog table */
    br_boolean  backdrop;  /* draw the red backdrop the blend is read against */
} scene_rop_fixture;

/*
 * Only the rows that bind the blend table carry the backdrop: the indexed blend
 * reads the pixel already in the colour buffer, and against the clear colour
 * that only halves the cube's own colour. The fog-only, plain and decal rows
 * bind no blend and are left alone, and so are the scene-fog-* and
 * scene-decal-* families elsewhere. The three blendfog rows take the near place
 * because their fog tables narrow the camera; see scene_rop_build().
 */
static const scene_rop_fixture scene_rop_fixtures[] = {
    /* Blend and fog at once. */
    {.name = "scene-blendfog-p2",        .map = 0, .flags = BR_MATF_PERSPECTIVE, .cubes = 7, .blend = BR_TRUE, .fog = BR_TRUE,  .backdrop = BR_TRUE},
    {.name = "scene-blendfog-arb-persp", .map = 2, .flags = BR_MATF_PERSPECTIVE, .cubes = 7, .blend = BR_TRUE, .fog = BR_TRUE,  .backdrop = BR_TRUE},
    {.name = "scene-blendfog-arb",       .map = 2, .flags = 0,                   .cubes = 7, .blend = BR_TRUE, .fog = BR_TRUE,  .backdrop = BR_TRUE},

    /* Blend alone. */
    {.name = "scene-blend-p2-persp",     .map = 0, .flags = BR_MATF_PERSPECTIVE, .cubes = 7, .blend = BR_TRUE, .fog = BR_FALSE, .backdrop = BR_TRUE},
    {.name = "scene-blend-p2-smooth",    .map = 0, .flags = 0,                   .cubes = SCENE_ROP_SMOOTH, .blend = BR_TRUE, .fog = BR_FALSE, .backdrop = BR_TRUE},
    {.name = "scene-blend-p2-flat",      .map = 0, .flags = 0,                   .cubes = SCENE_ROP_FLAT, .blend = BR_TRUE, .fog = BR_FALSE, .backdrop = BR_TRUE},
    {.name = "scene-blend-arb-persp",    .map = 2, .flags = BR_MATF_PERSPECTIVE, .cubes = 7, .blend = BR_TRUE, .fog = BR_FALSE, .backdrop = BR_TRUE},
    {.name = "scene-blend-arb",          .map = 2, .flags = 0,                   .cubes = 7, .blend = BR_TRUE, .fog = BR_FALSE, .backdrop = BR_TRUE},

    /* Fog alone. */
    {.name = "scene-fog-p2-persp",       .map = 1, .flags = BR_MATF_PERSPECTIVE, .cubes = 7, .blend = BR_FALSE, .fog = BR_TRUE},
    {.name = "scene-fog-p2",             .map = 1, .flags = 0,                   .cubes = 3, .blend = BR_FALSE, .fog = BR_TRUE},
    {.name = "scene-fog-p2-tex",         .map = 1, .flags = 0,                   .cubes = SCENE_ROP_TEX, .blend = BR_FALSE, .fog = BR_TRUE},
    {.name = "scene-fog-arb-persp",      .map = 2, .flags = BR_MATF_PERSPECTIVE, .cubes = 7, .blend = BR_FALSE, .fog = BR_TRUE},
    {.name = "scene-fog-arb",            .map = 2, .flags = 0,                   .cubes = 7, .blend = BR_FALSE, .fog = BR_TRUE},
    {.name = "scene-fog-flat",           .map = 3, .flags = 0,                   .cubes = SCENE_ROP_TEX, .blend = BR_FALSE, .fog = BR_TRUE},

    /* Plain: no ROP table at all. */
    {.name = "scene-idx-arb-persp",      .map = 2, .flags = BR_MATF_PERSPECTIVE, .cubes = 3, .blend = BR_FALSE, .fog = BR_FALSE},
    {.name = "scene-idx-arb",            .map = 2, .flags = 0,                   .cubes = 3, .blend = BR_FALSE, .fog = BR_FALSE},
    {.name = "scene-idx-p2-256",         .map = 1, .flags = 0,                   .cubes = 3, .blend = BR_FALSE, .fog = BR_FALSE},

    /* Decal. */
    {.name = "scene-decal-p2-256",       .map = 1, .flags = BR_MATF_DECAL,       .cubes = 3, .blend = BR_FALSE, .fog = BR_FALSE},
    {.name = "scene-decal-arb",          .map = 2, .flags = BR_MATF_DECAL,       .cubes = SCENE_ROP_SMOOTH, .blend = BR_FALSE, .fog = BR_FALSE},
    {.name = "scene-decal-arb-flat",     .map = 2, .flags = BR_MATF_DECAL,       .cubes = SCENE_ROP_FLAT, .blend = BR_FALSE, .fog = BR_FALSE},
};
// clang-format on

static br_error scene_make_rop_fixtures(br_model *cube)
{
    br_pixelmap *maps[4]; /* the fourth is the unmapped fixture's absent map */
    br_pixelmap *shade, *blend, *fog;
    br_error     r = BRE_OK;

    maps[3] = NULL;

    if((maps[0] = scene_texture("scene-rop-map-64", SCENE_TEX_SIZE, SCENE_TEX_SIZE, SCENE_TEX_CELLS)) == NULL ||
       (maps[1] = scene_texture_rows("scene-rop-map-256", SCENE_ROP_MAP_256, SCENE_ROP_MAP_256)) == NULL ||
       (maps[2] = scene_texture("scene-rop-map-arb", SCENE_ARB_WIDTH, SCENE_ARB_HEIGHT, SCENE_ARB_CELLS)) == NULL ||
       (shade = scene_shade_table("scene-rop-shade-table")) == NULL || (blend = scene_blend_table("scene-rop-blend-table")) == NULL ||
       (fog = scene_fog_table("scene-rop-fog-table")) == NULL) {
        fprintf(stderr, "failed to allocate the ROP fixture resources\n");
        return BRE_FAIL;
    }

    for(size_t i = 0; i < BR_ASIZE(scene_rop_fixtures); ++i) {
        const scene_rop_fixture *fx = &scene_rop_fixtures[i];

        if(scene_rop_build(fx->name, cube, fx->cubes, fx->flags, maps[fx->map], shade, fx->blend ? blend : NULL, fx->fog ? fog : NULL,
                           fx->backdrop) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * The flat twin of the untextured shade-table block. scene-shade is the same
     * rig with BR_MATF_SMOOTH, which selects the interpolated block; without the
     * flag the walk reaches the constant-intensity one. The empty index band is
     * what makes the untextured family reachable at all: the table's rows are
     * its intensity and its columns its output, and with index_range zero the
     * block reads range_zero's row rather than the intensity ramp.
     */
    {
        br_material *mat  = scene_fx_material("scene-shade-flat-material", BR_MATF_LIGHT);
        br_pixelmap *ramp = scene_shade_ramp("scene-shade-flat-ramp-table");

        if(ramp == NULL)
            return BRE_FAIL;

        mat->index_shade = ramp;
        mat->index_base  = 0;
        mat->index_range = 0;

        if(scene_save("scene-shade-flat.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    return r;
}

/*
 * The render-feature fixtures. `cube` is the model built by mkres_make_cube(),
 * which has both per-face map coordinates in 0..1 and a per-face normal.
 */
static br_error scene_make_feature_fixtures(br_model *cube)
{
    br_pixelmap *tex64, *tex32, *tex_arb, *shade, *shade_ramp, *blend, *fog;
    br_actor    *world;
    br_error     r = BRE_OK;

    if((tex64 = scene_texture("scene-textured-map", SCENE_TEX_SIZE, SCENE_TEX_SIZE, SCENE_TEX_CELLS)) == NULL ||
       (tex32 = scene_texture_rows("scene-tex-32-map", SCENE_TEX32_SIZE, SCENE_TEX32_SIZE)) == NULL ||
       (tex_arb = scene_texture("scene-tex-arb-map", SCENE_ARB_WIDTH, SCENE_ARB_HEIGHT, SCENE_ARB_CELLS)) == NULL ||
       (shade = scene_shade_table("scene-shade-table")) == NULL || (shade_ramp = scene_shade_ramp("scene-shade-ramp-table")) == NULL ||
       (blend = scene_blend_table("scene-blend-table")) == NULL || (fog = scene_fog_table("scene-fog-table")) == NULL) {
        fprintf(stderr, "failed to allocate feature fixture resources\n");
        return BRE_FAIL;
    }

    /* Constant intensity. */
    {
        br_material *mat = scene_fx_material("scene-flat-material", BR_MATF_LIGHT);

        if(scene_save("scene-flat.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /* Interpolated intensity: scene-flat with BR_MATF_SMOOTH and nothing else. */
    {
        br_material *mat = scene_fx_material("scene-smooth-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

        if(scene_save("scene-smooth.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /* An indexed texture and no lookup table. */
    {
        br_material *mat = scene_fx_material("scene-textured-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

        mat->colour_map = tex64;

        if(scene_save("scene-textured.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /* The same texture with an index_shade ramp in front of it. */
    {
        br_material *mat = scene_fx_material("scene-textured-shade-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

        mat->colour_map  = tex64;
        mat->index_shade = shade;

        if(scene_save("scene-textured-shade.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * The perspective-correct texture path. None of the checked-in scenes reach
     * it: their materials are perspective, but they only ever match the *unlit*
     * perspective block, because the intensity blocks require a shade table to
     * be bound before the matcher will consider them at all.
     */
    {
        br_material *mat = scene_fx_material("scene-persp-material", BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE);

        mat->colour_map = tex64;

        if(scene_save("scene-persp.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    {
        br_material *mat = scene_fx_material("scene-persp-shade-material", BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE);

        mat->colour_map  = tex64;
        mat->index_shade = shade;

        if(scene_save("scene-persp-shade.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * A map whose dimensions are not powers of two. pentprim has no dimension
     * block for it, so it falls to the arbitrary-width rasteriser - the path the
     * forest scene's background uses.
     */
    {
        br_material *mat = scene_fx_material("scene-tex-arb-material", BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE);

        mat->colour_map = tex_arb;

        if(scene_save("scene-tex-arb.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * The 32x32 power-of-two perspective block, which only the nine reference
     * scenes reached. pentprim's z-sorted TriangleRender_PT_I8_32 packs the
     * texel its first span starts from as if the map were 64 wide (shl cl,2 /
     * shr ecx,2 / and ecx,63*65), where every other size in both families packs
     * its own width; a 32x32 map makes that read a row or two away, so the
     * fixture is the near miss the nine-scene PNG diff had to find.
     *
     * The rig is scene-persp's - same cube, same light, same perspective flag -
     * with a 32x32 map instead of a 64x64 one, so the map dimension is what
     * selects the block and the map size is what selects the 32 block over the
     * 64 one. The perspective flag is what selects the perspective entry rather
     * than the affine twin; both are power-of-two 32 blocks.
     *
     * The cube is drawn at SCENE_TEX32_SCALE rather than SCENE_FX_SCALE, and
     * that too is measured: the trapezium corrects the seed address against the
     * span's own u/v walk, and on the 1.5x cube the correction absorbs the
     * wrong seed entirely - rebuilding pentprim with the old 63*65 packing
     * leaves the 1.5x fixture's checksum unmoved. At 3.0x the spans start with
     * fractions large enough that the error survives, and the same rebuild moves
     * the frame.
     */
    {
        br_material *mat = scene_fx_material("scene-tex-32-material", BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE);

        mat->colour_map = tex32;

        if(scene_save("scene-tex-32.gltf", scene_fx_world_range(cube, mat, SCENE_TEX32_SCALE, BR_SCALAR(0.1), BR_SCALAR(100.0))) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * The untextured shade-table block, which is the only user of range_zero -
     * and range_zero is the whole of index_range == 0, so this fixture has to
     * leave the index band empty for the block to be reachable at all.
     */
    {
        br_material *mat = scene_fx_material("scene-shade-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

        mat->index_shade = shade_ramp;
        mat->index_base  = 0;
        mat->index_range = 0;

        if(scene_save("scene-shade.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /* Decal: the texel index replaces the shade index rather than being modulated by it. */
    {
        br_material *mat = scene_fx_material("scene-decal-material", BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_DECAL);

        mat->colour_map  = tex64;
        mat->index_shade = shade;

        if(scene_save("scene-decal.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * Indexed fog. The camera's depth range is narrowed to 4..8, close around the
     * cube, so that the fog level - the high byte of z - actually varies across
     * the geometry rather than taking one value over the whole model.
     */
    {
        br_material *mat = scene_fx_material("scene-fog-material", BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_FOG_LOCAL);

        mat->index_fog  = fog;
        mat->fog_min    = BR_SCALAR(4.0);
        mat->fog_max    = BR_SCALAR(8.0);
        mat->fog_colour = BR_COLOUR_RGB(160, 160, 160);

        if(scene_save("scene-fog.gltf", scene_fx_world_range(cube, mat, SCENE_FX_SCALE, BR_SCALAR(4.0), BR_SCALAR(8.0))) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * Indexed blend. No BR_MATF_SMOOTH, so the blend is applied to the raw texel:
     * the shaded-and-blended combination needs a shade table as well, and one
     * thing at a time is what makes a failure name itself.
     *
     * The backdrop behind the two cubes is what the blend mixes against - see
     * the note above scene_backdrop_material().
     */
    {
        br_material *mat = scene_fx_material("scene-blend-material", BR_MATF_LIGHT | BR_MATF_BLEND);

        mat->colour_map  = tex64;
        mat->index_blend = blend;

        world = scene_fx_world(cube, mat);

        if(world != NULL)
            scene_fx_add_cube(world, cube, mat, BR_SCALAR(0.9), BR_SCALAR(0.0), BR_SCALAR(1.0));

        if(world != NULL && scene_add_backdrop(world, cube, "scene-blend", SCENE_BACKDROP_INDEX, SCENE_BACKDROP_SCALE_XY,
                                               SCENE_BACKDROP_SCALE_Z, SCENE_BACKDROP_Z) != BRE_OK)
            r = BRE_FAIL;

        if(scene_save("scene-blend.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * The control for scene-blend: the same two cubes and the same texture, with
     * no blend table bound. Without it the blend relation would be comparing two
     * differently-shaped scenes and would pass whether the table was read or
     * not.
     *
     * It gets the backdrop too, or the relation would stop being readable: a
     * plain cube over black beside a blended one over red is two changes, not
     * the one knob the pair exists to isolate.
     */
    {
        br_material *mat = scene_fx_material("scene-blend-off-material", BR_MATF_LIGHT);

        mat->colour_map = tex64;

        world = scene_fx_world(cube, mat);

        if(world != NULL)
            scene_fx_add_cube(world, cube, mat, BR_SCALAR(0.9), BR_SCALAR(0.0), BR_SCALAR(1.0));

        if(world != NULL && scene_add_backdrop(world, cube, "scene-blend-off", SCENE_BACKDROP_INDEX, SCENE_BACKDROP_SCALE_XY,
                                               SCENE_BACKDROP_SCALE_Z, SCENE_BACKDROP_Z) != BRE_OK)
            r = BRE_FAIL;

        if(scene_save("scene-blend-off.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * Dithered texture coordinates. The dithered_map blocks are the only users of
     * BR_MATF_DITHER, and they also want a 64/128/256/1024 map and the
     * perspective flag, so this differs from scene-persp by the flag alone.
     */
    {
        br_material *mat = scene_fx_material("scene-dither-material", BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE | BR_MATF_DITHER);

        mat->colour_map = tex64;

        if(scene_save("scene-dither.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * Partial opacity. Inert on an INDEX_8 buffer; on 15/16/24bpp it is what
     * selects an alpha or screendoor block, and it needs no table of its own.
     */
    {
        br_material *mat = scene_fx_material("scene-alpha-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

        mat->colour_map = tex64;
        mat->opacity    = 128;

        if(scene_save("scene-alpha.gltf", scene_fx_world(cube, mat)) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * The RGB-output shade table: scene-textured-shade's rig, but the table is
     * typed to the output format and its sample goes straight to the colour
     * buffer (`out = shade[(intensity << 8) | texel]`) instead of being an index
     * into the palette. A shade table's type must equal the output, so the
     * fixtures carry one table each and each is only a witness at its own --bpp;
     * a table typed for 555 cannot match a 565 output, and vice versa. The
     * smooth/flat pair is the interpolated/constant split, which is a different
     * kernel name even though the walk is shared - see scene-flat/scene-smooth.
     */
    {
        br_pixelmap *shade555  = scene_shade_table_rgb("scene-shade-rgb555-table", BR_PMT_RGB_555);
        br_pixelmap *shade565  = scene_shade_table_rgb("scene-shade-rgb565-table", BR_PMT_RGB_565);
        br_pixelmap *shade_tex = scene_shade_texture("scene-shade-rgb-map");

        if(shade555 == NULL || shade565 == NULL || shade_tex == NULL)
            return BRE_FAIL;

        {
            br_material *mat = scene_fx_material("scene-shade-rgb555-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

            mat->colour_map  = shade_tex;
            mat->index_shade = shade555;

            if(scene_save("scene-shade-rgb555.gltf", scene_fx_world(cube, mat)) != BRE_OK)
                r = BRE_FAIL;
        }

        {
            br_material *mat = scene_fx_material("scene-shade-rgb555-flat-material", BR_MATF_LIGHT);

            mat->colour_map  = shade_tex;
            mat->index_shade = shade555;

            if(scene_save("scene-shade-rgb555-flat.gltf", scene_fx_world(cube, mat)) != BRE_OK)
                r = BRE_FAIL;
        }

        {
            br_material *mat = scene_fx_material("scene-shade-rgb565-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

            mat->colour_map  = shade_tex;
            mat->index_shade = shade565;

            if(scene_save("scene-shade-rgb565.gltf", scene_fx_world(cube, mat)) != BRE_OK)
                r = BRE_FAIL;
        }

        {
            br_material *mat = scene_fx_material("scene-shade-rgb565-flat-material", BR_MATF_LIGHT);

            mat->colour_map  = shade_tex;
            mat->index_shade = shade565;

            if(scene_save("scene-shade-rgb565-flat.gltf", scene_fx_world(cube, mat)) != BRE_OK)
                r = BRE_FAIL;
        }
    }

    /*
     * The RGB-typed colour maps, which no earlier fixture reached. The map's
     * type is not decoration - a block's texture type is its
     * output type (shared_texture() sets texture_type from colour_type), so a
     * textured, unshaded 15bpp primitive with a BR_PMT_RGB_555 map selects
     * TriangleRenderPITA15 and nothing else. No
     * fixture could reach it before, because the only colour maps in the tree
     * were INDEX_8; the brender=rgb555/rgb565 glTF markers are what let one be
     * authored and read back as itself.
     *
     * Reachable only in the Z-sort mode, and that is a property of the block set
     * rather than of the fixture: with a depth buffer the MMX table is walked
     * first, its textured rows all require an INDEX_8 map with a palette, and
     * its untextured rows match any 555/565 triangle - so a 555 map is never
     * sampled at all and the primitive is drawn untextured.
     * The Z-sort tables have no MMX half, so the general prm_t15 walk reaches
     * TriangleRenderPITA15 there.
     *
     * The map is the arbitrary-width (96x48) one scene-tex-arb uses, so it does
     * not satisfy any of the power-of-two textureNxN requirements and the walk
     * reaches the bare `texture` block. The material is smooth and perspective
     * correct, matching scene-persp's rig, so the selected entry is the
     * perspective-subdivide one (rather than the affine twin).
     */
    {
        br_pixelmap *tex555 = scene_texture_rgb("scene-tex-rgb555-map", BR_PMT_RGB_555, SCENE_ARB_WIDTH, SCENE_ARB_HEIGHT, SCENE_ARB_CELLS);
        br_pixelmap *tex565 = scene_texture_rgb("scene-tex-rgb565-map", BR_PMT_RGB_565, SCENE_ARB_WIDTH, SCENE_ARB_HEIGHT, SCENE_ARB_CELLS);

        if(tex555 == NULL || tex565 == NULL)
            return BRE_FAIL;

        {
            br_material *mat = scene_fx_material("scene-tex-rgb555-material", BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE);

            mat->colour_map = tex555;

            if(scene_save("scene-tex-rgb555.gltf", scene_fx_world(cube, mat)) != BRE_OK)
                r = BRE_FAIL;
        }

        {
            br_material *mat = scene_fx_material("scene-tex-rgb565-material", BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE);

            mat->colour_map = tex565;

            if(scene_save("scene-tex-rgb565.gltf", scene_fx_world(cube, mat)) != BRE_OK)
                r = BRE_FAIL;
        }

        /*
         * The affine twins: scene-tex-rgb555/565 with BR_MATF_PERSPECTIVE dropped
         * and nothing else changed. The CORRECT cells are witnessed by the
         * perspective fixtures above; the AFFINE cells of the same block are not,
         * because every fixture that reaches this family carries the flag. The
         * pair is what makes the flag's effect visible in the checksum rather
         * than only in the block the walk selects.
         */
        {
            br_material *mat = scene_fx_material("scene-tex-rgb555-affine-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

            mat->colour_map = tex555;

            if(scene_save("scene-tex-rgb555-affine.gltf", scene_fx_world(cube, mat)) != BRE_OK)
                r = BRE_FAIL;
        }

        {
            br_material *mat = scene_fx_material("scene-tex-rgb565-affine-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

            mat->colour_map = tex565;

            if(scene_save("scene-tex-rgb565-affine.gltf", scene_fx_world(cube, mat)) != BRE_OK)
                r = BRE_FAIL;
        }
    }

    /*
     * Family B: a textured primitive with an RGB-typed shade table.
     *
     * The shape is prim_t24.ifg:8, TriangleRenderPIZ2TIA_RGB_888 - a z-buffered
     * textured primitive that looks its fragment up in the bound shade table
     * (`out = shade[(intensity << 8) | texel]`). 24bpp is what makes it
     * reachable: pentprim has no MMX table for an 888 output, so the general
     * prim_t24 walk reaches this block, while the 555/565 twins are intercepted
     * by the MMX table's untextured rows first. The table is RGB_888 because a
     * shade table's type must equal the output (shared_texture(), and the
     * matcher's shade_type test).
     *
     * The material is deliberately bright: ka 0.5, kd 1.0. scene_fx_material's
     * kd 0.7 keeps the surface intensity to about half of the table's 256 rows,
     * and the shape's defect - the reader indexes an RGB_888 table at a four-byte
     * stride where the table's own row_bytes is three, so `idx * 4` leaves the
     * allocation at idx 49152, i.e. at an intensity of 192 - only shows itself
     * near the top of the range. Full diffuse plus a mid ambient puts the lit
     * faces just past it, so the fixture reaches the end of the table and past
     * it, which is what makes it a witness rather than a neighbour of one.
     */
    {
        br_pixelmap *shade888  = scene_shade_table_rgb("scene-shade-rgb888-table", BR_PMT_RGB_888);
        br_pixelmap *shade_tex = scene_shade_texture("scene-shade-rgb888-map");

        if(shade888 == NULL || shade_tex == NULL)
            return BRE_FAIL;

        {
            br_material *mat = scene_fx_material("scene-shade-rgb888-material", BR_MATF_LIGHT | BR_MATF_SMOOTH);

            mat->ka = BR_SCALAR(0.5);
            mat->kd = BR_SCALAR(1.0);

            mat->colour_map  = shade_tex;
            mat->index_shade = shade888;

            if(scene_save("scene-shade-rgb888.gltf", scene_fx_world(cube, mat)) != BRE_OK)
                r = BRE_FAIL;
        }
    }

    /*
     * The topology witnesses.
     *
     * The topology the renderer draws comes from br_actor::render_style, and no
     * glTF path carried it until the BR_actors extension: the loader never set
     * it and the writer never wrote it, so every actor loaded from a .gltf was
     * BR_RSTYLE_DEFAULT and the blocks that draw points and lines (RP_TOP_POINT,
     * RP_TOP_LINE) had no witness anywhere in the corpus.
     *
     * Each fixture is scene-flat's rig with render_style set on the cube actor
     * and nothing else changed - the same cube, turn, material flags, light and
     * camera - so the round trip is witnessed by the frame itself: if the
     * extension is dropped the actor falls back to BR_RSTYLE_DEFAULT and the
     * frame is byte-identical to scene-flat.
     */
    {
        br_material *mat = scene_fx_material("scene-edges-material", BR_MATF_LIGHT);

        if(scene_save("scene-edges.gltf", scene_fx_world_topology(cube, mat, BR_RSTYLE_EDGES)) != BRE_OK)
            r = BRE_FAIL;
    }

    {
        br_material *mat = scene_fx_material("scene-points-material", BR_MATF_LIGHT);

        if(scene_save("scene-points.gltf", scene_fx_world_topology(cube, mat, BR_RSTYLE_POINTS)) != BRE_OK)
            r = BRE_FAIL;
    }

    return r;
}

br_error mkres_make_scenes(void)
{
    br_model    *cube;
    br_material *material;
    br_actor    *world;
    br_error     r = BRE_OK;

    if((cube = mkres_make_cube("cube")) == NULL) {
        fprintf(stderr, "failed to allocate cube model\n");
        return BRE_FAIL;
    }

    if((material = scene_material("scene-material")) == NULL) {
        fprintf(stderr, "failed to allocate scene material\n");
        return BRE_FAIL;
    }

    /* One spot, aimed at the cube. */
    world = scene_world(cube, material, BR_SCALAR(1.0));

    if(world != NULL)
        scene_add_light(world, "spot", BR_LIGHT_SPOT, BR_SCALAR(0.0), BR_TRUE);

    if(scene_save("scene-spot-hit.gltf", world) != BRE_OK)
        r = BRE_FAIL;

    /* One spot, aimed away. */
    world = scene_world(cube, material, BR_SCALAR(1.0));

    if(world != NULL)
        scene_add_light(world, "spot", BR_LIGHT_SPOT, BR_SCALAR(0.0), BR_FALSE);

    if(scene_save("scene-spot-miss.gltf", world) != BRE_OK)
        r = BRE_FAIL;

    /*
     * Both at once: a single render covers the "must keep" and the "must drop"
     * direction, and a run that only got one of them right cannot pass.
     */
    world = scene_world(cube, material, BR_SCALAR(1.0));

    if(world != NULL) {
        scene_add_light(world, "spot-hit", BR_LIGHT_SPOT, BR_SCALAR(0.0), BR_TRUE);
        scene_add_light(world, "spot-miss", BR_LIGHT_SPOT, BR_SCALAR(0.0), BR_FALSE);
    }

    if(scene_save("scene-spot-hit-miss.gltf", world) != BRE_OK)
        r = BRE_FAIL;

    /* A point light that can reach the cube, and one that cannot. */
    world = scene_world(cube, material, BR_SCALAR(1.0));

    if(world != NULL)
        scene_add_light(world, "point", BR_LIGHT_POINT, SCENE_RADIUS_REACHES, BR_TRUE);

    if(scene_save("scene-radius-near.gltf", world) != BRE_OK)
        r = BRE_FAIL;

    world = scene_world(cube, material, BR_SCALAR(1.0));

    if(world != NULL)
        scene_add_light(world, "point", BR_LIGHT_POINT, SCENE_RADIUS_SHORT, BR_TRUE);

    if(scene_save("scene-radius-far.gltf", world) != BRE_OK)
        r = BRE_FAIL;

    /*
     * A non-uniformly scaled model: the extent that matters is the model's
     * radius scaled into view space, not the model-space radius.
     */
    world = scene_world(cube, material, BR_SCALAR(3.0));

    if(world != NULL)
        scene_add_light(world, "point", BR_LIGHT_POINT, SCENE_RADIUS_REACHES, BR_TRUE);

    if(scene_save("scene-scaled.gltf", world) != BRE_OK)
        r = BRE_FAIL;

    /* A view-space light, which takes a different shading path. */
    world = scene_world(cube, material, BR_SCALAR(1.0));

    if(world != NULL)
        scene_add_light(world, "spot", BR_LIGHT_SPOT | BR_LIGHT_VIEW, BR_SCALAR(0.0), BR_TRUE);

    if(scene_save("scene-view-space.gltf", world) != BRE_OK)
        r = BRE_FAIL;

    /*
     * Distinct per-channel material and light, so a channel-order or
     * per-channel failure names itself instead of reading as a grey shift.
     */
    {
        br_material *col = scene_material_ex("scene-colour-material", BR_COLOUR_RGB(204, 102, 26), BR_MATF_LIGHT, BR_SCALAR(0.1),
                                             BR_SCALAR(0.7), BR_SCALAR(0.0), BR_SCALAR(20.0));

        world = scene_world(cube, col, BR_SCALAR(1.0));

        if(world != NULL)
            scene_add_light_col(world, "point", BR_LIGHT_POINT, BR_COLOUR_RGB(230, 128, 51), SCENE_RADIUS_REACHES, BR_TRUE);

        if(scene_save("scene-colour.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /* Coloured ambient on its own, so the ambient path is isolated. */
    {
        world = scene_world_amb(cube, material, BR_SCALAR(1.0), BR_COLOUR_RGB(51, 13, 102));

        if(scene_save("scene-colour-ambient.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /* Two coloured lights: the render is their sum, so dropping one moves it. */
    {
        world = scene_world(cube, material, BR_SCALAR(1.0));

        if(world != NULL) {
            scene_add_light_col(world, "red", BR_LIGHT_POINT, BR_COLOUR_RGB(255, 13, 13), SCENE_RADIUS_REACHES, BR_TRUE);
            scene_add_light_col(world, "blue", BR_LIGHT_POINT, BR_COLOUR_RGB(13, 26, 255), SCENE_RADIUS_REACHES, BR_TRUE);
        }

        if(scene_save("scene-colour-two.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /* A directional light, which none of the original fixtures had. */
    {
        world = scene_world(cube, material, BR_SCALAR(1.0));

        if(world != NULL)
            scene_add_light(world, "sun", BR_LIGHT_DIRECT, BR_SCALAR(0.0), BR_TRUE);

        if(scene_save("scene-directional.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    {
        world = scene_world(cube, material, BR_SCALAR(1.0));

        if(world != NULL)
            scene_add_light(world, "sun", BR_LIGHT_DIRECT, BR_SCALAR(0.0), BR_FALSE);

        if(scene_save("scene-directional-miss.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * Same light and geometry as scene-directional, differing only in ks, so
     * a specular path that does nothing reads as no change.
     */
    {
        br_material *spec = scene_material_ex("scene-specular-material", BR_COLOUR_RGB(200, 200, 200), BR_MATF_LIGHT, BR_SCALAR(0.1),
                                              BR_SCALAR(0.7), BR_SCALAR(0.5), BR_SCALAR(20.0));

        world = scene_world(cube, spec, BR_SCALAR(1.0));

        if(world != NULL)
            scene_add_light(world, "sun", BR_LIGHT_DIRECT, BR_SCALAR(0.0), BR_TRUE);

        if(scene_save("scene-specular.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * Unlit: an added light must not change it. scene-unlit and
     * scene-unlit-plain must therefore be identical.
     */
    {
        br_material *unlit = scene_material_ex("scene-unlit-material", BR_COLOUR_RGB(200, 200, 200), 0, BR_SCALAR(0.1), BR_SCALAR(0.7),
                                               BR_SCALAR(0.0), BR_SCALAR(20.0));

        world = scene_world(cube, unlit, BR_SCALAR(1.0));

        if(world != NULL)
            scene_add_light(world, "sun", BR_LIGHT_DIRECT, BR_SCALAR(0.0), BR_TRUE);

        if(scene_save("scene-unlit.gltf", world) != BRE_OK)
            r = BRE_FAIL;

        world = scene_world(cube, unlit, BR_SCALAR(1.0));

        if(scene_save("scene-unlit-plain.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * A spot light on a non-uniformly scaled model: the direction transform,
     * where scene-scaled only covers the position transform. The light is
     * straight down -Z, so this does not reach the transpose.
     */
    {
        world = scene_world(cube, material, BR_SCALAR(3.0));

        if(world != NULL)
            scene_add_light(world, "spot", BR_LIGHT_SPOT, BR_SCALAR(0.0), BR_TRUE);

        if(scene_save("scene-scale-spot.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * The same scaled model with the light off the Z axis, so its direction
     * now has a component on the scaled axis and the model-space direction
     * transform is actually exercised.
     */
    {
        world = scene_world(cube, material, BR_SCALAR(3.0));

        if(world != NULL)
            scene_add_light_turned(world, "sun", BR_LIGHT_DIRECT, BR_SCALAR(0.0), BR_ANGLE_DEG(30));

        if(scene_save("scene-scale-direction.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    {
        world = scene_world(cube, material, BR_SCALAR(3.0));

        if(world != NULL)
            scene_add_light_turned(world, "spot", BR_LIGHT_SPOT, BR_SCALAR(0.0), BR_ANGLE_DEG(30));

        if(scene_save("scene-scale-spot-off.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    /*
     * Cutoff volumes. A vertex inside a region is lit fully; beyond the
     * boundary it fades linearly over `falloff_distance` to nothing. `-cut` is
     * the same region with no fade at all (the hard cutoff path), and `-off`
     * carries no volume, so the volume is the only difference between them.
     *
     * The volume is applied on the colour lighting path only: the indexed
     * (8bpp) path has no volume variants, so at INDEX_8 all three render
     * identically.
     */
    {
        br_material *lvm = scene_light_volume_material();

        world = scene_world(cube, lvm, BR_SCALAR(1.0));

        if(world != NULL)
            scene_add_volume_light(world, BR_SCALAR(1.0));

        if(scene_save("scene-light-volume.gltf", world) != BRE_OK)
            r = BRE_FAIL;

        world = scene_world(cube, lvm, BR_SCALAR(1.0));

        if(world != NULL)
            scene_add_volume_light(world, BR_SCALAR(0.0));

        if(scene_save("scene-light-volume-cut.gltf", world) != BRE_OK)
            r = BRE_FAIL;

        world = scene_world(cube, lvm, BR_SCALAR(1.0));

        if(world != NULL)
            scene_add_light(world, "point", BR_LIGHT_POINT, BR_SCALAR(0.0), BR_TRUE);

        if(scene_save("scene-light-volume-off.gltf", world) != BRE_OK)
            r = BRE_FAIL;
    }

    if(scene_make_feature_fixtures(cube) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_rgb_shade_fixtures(cube) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_line_fixtures(cube) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_rop_fixtures(cube) != BRE_OK)
        r = BRE_FAIL;

    if(scene_make_mmx_fixtures(cube) != BRE_OK)
        r = BRE_FAIL;

    return r;
}
