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

static br_actor *scene_add_light_turned(br_actor *world, const char *name, br_uint_8 type, br_scalar radius_outer,
                                        br_scalar angle_y)
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

    return r;
}
