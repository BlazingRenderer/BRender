/*
 * The material witness: every field the file formats carry, written and read back.
 *
 * The corpus only ever *loads* glTF, and the `fixtures` check regenerates glTF without reading
 * one back into a material, so neither the `.mat` nor the `.scr` reader had any gate. Both have
 * silently lost a field: a bump-map reference a loader's chunk table had no entry for, and a
 * default that lived in the reader's initialiser after the writer stopped writing the field.
 *
 * The files never reach the disk - BrMemFileInstall gives the file layer one in-memory file - and
 * no device is involved, so this runs in every configuration and in both floating-point classes.
 *
 * Every value below is chosen so that the script's `%f` round-trips it exactly; a format that
 * prints fewer digits than it parses is a different witness's problem.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <brender.h>
#include <brddi.h>
#include "brmemfs.h"

#include "rt_demo.h"
#include "materialwitness.h"

static int mw_failures;

#define MW_FAIL(...)                   \
    do {                               \
        BrLogError("RT", __VA_ARGS__); \
        mw_failures++;                 \
    } while(0)

/*
 * Everything that is not a default. `flags` uses only canonical names, because the script writer
 * emits those and nothing else: a bit with no name is not representable there.
 */
static br_error mw_populate(br_material *m, br_pixelmap *map)
{
    m->identifier  = BrResStrDup(m, "material-witness");
    m->colour      = BR_COLOUR_RGB(0x40, 0x60, 0x80);
    m->opacity     = 200;
    m->ka          = BR_UFRACTION(0.25);
    m->kd          = BR_UFRACTION(0.5);
    m->ks          = BR_UFRACTION(0.75);
    m->power       = BR_SCALAR(20.0);
    m->flags       = BR_MATF_LIGHT | BR_MATF_SMOOTH | BR_MATF_PERSPECTIVE | BR_MATF_SMOOTH_ALPHA;
    m->mode        = BR_MATM_DEPTH_TEST_LE | BR_MATM_BLEND_MODE_STANDARD;
    m->index_base  = 16;
    m->index_range = 200;

    m->colour_map = map;
    m->bump_map   = map;
    m->bump_scale = BR_SCALAR(0.5);

    /* A negative depth: the conversion that clamps it to zero is exactly what this catches. */
    m->bump_depth = BR_FRACTION(-0.25);

    BrMatrix23Identity(&m->map_transform);
    m->map_transform.m[0][0] = BR_SCALAR(0.5);
    m->map_transform.m[1][1] = BR_SCALAR(2.0);
    m->map_transform.m[2][0] = BR_SCALAR(0.25);

    BrMatrix23Identity(&m->bump_map_transform);
    m->bump_map_transform.m[0][0] = BR_SCALAR(2.0);
    m->bump_map_transform.m[2][1] = BR_SCALAR(0.5);

    m->fog_min             = BR_SCALAR(1.0);
    m->fog_max             = BR_SCALAR(4.0);
    m->fog_colour          = BR_COLOUR_RGB(0x11, 0x22, 0x33);
    m->subdivide_tolerance = 8;

    return BRE_OK;
}

static void mw_compare_matrix(const br_matrix23 *a, const br_matrix23 *b, const char *field, const char *what)
{
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 2; ++j)
            if(a->m[i][j] != b->m[i][j])
                MW_FAIL("%s: %s did not survive: [%d][%d] %f became %f.", field, what, i, j, (double)BrScalarToFloat(a->m[i][j]),
                        (double)BrScalarToFloat(b->m[i][j]));
}

/* The fields both formats carry, compared field by field. */
static void mw_compare_common(const br_material *a, const br_material *b, const char *what)
{
    /* 24 bits: the binary format stores RGB, and the script writes three numbers. */
    if((a->colour & 0x00ffffff) != (b->colour & 0x00ffffff))
        MW_FAIL("%s: colour %06x became %06x.", what, (unsigned)a->colour & 0x00ffffff, (unsigned)b->colour & 0x00ffffff);
    if(a->opacity != b->opacity)
        MW_FAIL("%s: opacity %u became %u.", what, a->opacity, b->opacity);
    if(a->ka != b->ka || a->kd != b->kd || a->ks != b->ks)
        MW_FAIL("%s: ka/kd/ks did not survive.", what);
    if(a->power != b->power)
        MW_FAIL("%s: power did not survive.", what);
    if(a->flags != b->flags)
        MW_FAIL("%s: flags %x became %x.", what, (unsigned)a->flags, (unsigned)b->flags);
    if(a->index_base != b->index_base || a->index_range != b->index_range)
        MW_FAIL("%s: index_base/range did not survive.", what);
    if(a->colour_map != b->colour_map)
        MW_FAIL("%s: colour_map did not survive (it must come back as the same registered map).", what);
    if(a->bump_map != b->bump_map)
        MW_FAIL("%s: bump_map did not survive (it must come back as the same registered map).", what);
    if(a->bump_scale != b->bump_scale)
        MW_FAIL("%s: bump_scale did not survive.", what);
    if(a->bump_depth != b->bump_depth)
        MW_FAIL("%s: bump_depth %f became %f.", what, (double)BrScalarToFloat(BrFractionToScalar(a->bump_depth)),
                (double)BrScalarToFloat(BrFractionToScalar(b->bump_depth)));

    mw_compare_matrix(&a->map_transform, &b->map_transform, what, "map_transform");
    mw_compare_matrix(&a->bump_map_transform, &b->bump_map_transform, what, "bump_map_transform");
}

/* The binary format carries the mode; the script has no property for it. */
static void mw_compare_binary(const br_material *a, const br_material *b)
{
    mw_compare_common(a, b, "the binary format");

    if(a->mode != b->mode)
        MW_FAIL("the binary format: mode %u became %u.", a->mode, b->mode);
    if(a->fog_min != b->fog_min || a->fog_max != b->fog_max || (a->fog_colour & 0x00ffffff) != (b->fog_colour & 0x00ffffff))
        MW_FAIL("the binary format: the fog fields did not survive: min/max %f/%f became %f/%f, colour %06x became %06x.",
                (double)BrScalarToFloat(a->fog_min), (double)BrScalarToFloat(a->fog_max), (double)BrScalarToFloat(b->fog_min),
                (double)BrScalarToFloat(b->fog_max), (unsigned)a->fog_colour & 0x00ffffff, (unsigned)b->fog_colour & 0x00ffffff);
    if(a->subdivide_tolerance != b->subdivide_tolerance)
        MW_FAIL("the binary format: subdivide_tolerance did not survive.");
}

static void mw_compare_script(const br_material *a, const br_material *b)
{
    /* The script has no property for mode, fog or subdivide_tolerance: they are its limits. */
    mw_compare_common(a, b, "the material script");
}

/* One round trip through one pair of save/load entry points, in the memory filesystem. */
static br_error mw_round_trip(const char  *filename, br_error (*save)(const char *, br_material *), br_material *(*load)(const char *),
                              br_material *source, br_material **out)
{
    memfile_store  store = {0};
    br_filesystem *previous;
    br_material   *loaded;

    previous     = BrMemFileInstall(&store);
    br_error err = save(filename, source);
    BrMemFileRestore(previous);

    if(err != BRE_OK || store.data == NULL || store.size == 0) {
        BrLogError("RT", "material witness: could not save %s to the memory filesystem.", filename);
        return BRE_FAIL;
    }

    previous = BrMemFileInstall(&store);
    loaded   = load(filename);
    BrMemFileRestore(previous);

    if(loaded == NULL) {
        BrLogError("RT", "material witness: could not load %s back.", filename);
        return BRE_FAIL;
    }

    *out = loaded;
    return BRE_OK;
}

/* Both save entry points return a count, not a br_error. */
static br_error mw_save_binary(const char *filename, br_material *m)
{
    return BrMaterialSave(filename, m) == 1 ? BRE_OK : BRE_FAIL;
}

static br_error mw_save_script(const char *filename, br_material *m)
{
    return BrFmtScriptMaterialSave(filename, m) == 1 ? BRE_OK : BRE_FAIL;
}

/* The buffer holds three float32 positions followed by TEXCOORD_0 and TEXCOORD_1. */
static const char mw_gltf_fixture
    [] = "{\"asset\":{\"version\":\"2.0\"},\"extensionsUsed\":[\"KHR_texture_transform\"],"
         "\"scene\":0,\"scenes\":[{\"nodes\":[0]}],\"nodes\":[{\"mesh\":0}],"
         "\"meshes\":[{\"primitives\":[{\"attributes\":{\"POSITION\":0,\"TEXCOORD_1\":2,\"TEXCOORD_0\":1},\"material\":0}]}],"
         "\"materials\":[{\"name\":\"%s\",%s}],\"textures\":[{\"source\":0},{\"source\":1}],"
         "\"images\":[{\"uri\":\"data:image/png;base64,"
         "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAADElEQVR4nGM4oSECAALAAQUE/EAbAAAAAElFTkSuQmCC\"},"
         "{\"uri\":\"data:image/png;base64,"
         "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAADElEQVR4nGNoOPAeAAPzAjDczs/qAAAAAElFTkSuQmCC\"}],"
         "\"buffers\":[{\"byteLength\":84,\"uri\":\"data:application/octet-stream;base64,"
         "AACAvwAAgL8AAAAAAACAPwAAgL8AAAAAAAAAAAAAgD8AAAAAAAAAPgAAgD4AAMA+AAAAPwAAAD8AAEA/AABgPwAAAD4AAEA/AADAPgAAAD8AACA/\"}],"
         "\"bufferViews\":[{\"buffer\":0,\"byteLength\":84,\"target\":34962}],"
         "\"accessors\":[{\"bufferView\":0,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\",\"min\":[-1,-1,0],\"max\":[1,1,0]},"
         "{\"bufferView\":0,\"byteOffset\":36,\"componentType\":5126,\"count\":3,\"type\":\"VEC2\"},"
         "{\"bufferView\":0,\"byteOffset\":60,\"componentType\":5126,\"count\":3,\"type\":\"VEC2\"}]}";

#define MW_BASE(properties)   "\"pbrMetallicRoughness\":{\"baseColorTexture\":{\"index\":0" properties "}},"
#define MW_NORMAL(properties) "\"normalTexture\":{\"index\":1,\"scale\":0.5" properties "}"
#define MW_TRANSFORM(fields)  ",\"extensions\":{\"KHR_texture_transform\":{" fields "}}"
#define MW_UV1                ",\"texCoord\":1"
#define MW_IDENTITY \
    {               \
        {0, 0}, \
        {1, 0}, \
        {0, 1} \
}

/* Expected absolute UVs at the origin and at the two unit axes, after the glTF texture transform. */
static const struct mw_gltf_case {
    const char *name, *material;
    int         uv_set;
    br_boolean  has_base, has_bump;
    float       base_uv[3][2], bump_uv[3][2];
} mw_gltf_cases[] = {
    {
     .name     = "shared set",
     .material = MW_BASE(MW_UV1) MW_NORMAL(MW_UV1),
     .uv_set   = 1,
     .has_base = BR_TRUE,
     .has_bump = BR_TRUE,
     .base_uv  = MW_IDENTITY,
     .bump_uv  = MW_IDENTITY,
     },
    {
     .name     = "normal only",
     .material = MW_NORMAL(MW_UV1),
     .uv_set   = 1,
     .has_bump = BR_TRUE,
     .base_uv  = MW_IDENTITY,
     .bump_uv  = MW_IDENTITY,
     },
    {
     .name     = "different sets",
     .material = MW_BASE("") MW_NORMAL(MW_UV1),
     .has_base = BR_TRUE,
     .base_uv  = MW_IDENTITY,
     },
    {
     .name     = "texCoord overrides",
     .material = MW_BASE(MW_TRANSFORM("\"texCoord\":1,\"scale\":[0.5,0.25],\"rotation\":1.57079632679,\"offset\":[0.25,0.5]"))
            MW_NORMAL(MW_TRANSFORM("\"texCoord\":1")),
     .uv_set   = 1,
     .has_base = BR_TRUE,
     .has_bump = BR_TRUE,
     .base_uv  = {{0.25f, 0.5f}, {0.25f, 0.0f}, {0.5f, 0.5f}},
     .bump_uv  = MW_IDENTITY,
     },
    {
     .name     = "independent transforms",
     .material = MW_BASE(MW_UV1 MW_TRANSFORM("\"scale\":[-1.5,0.75],\"offset\":[0.25,-0.5]"))
            MW_NORMAL(MW_UV1 MW_TRANSFORM("\"scale\":[2,0.5],\"rotation\":-1.57079632679,\"offset\":[-0.125,0.375]")),
     .uv_set   = 1,
     .has_base = BR_TRUE,
     .has_bump = BR_TRUE,
     .base_uv  = {{0.25f, -0.5f}, {-1.25f, -0.5f}, {0.25f, 0.25f}},
     .bump_uv  = {{-0.125f, 0.375f}, {-0.125f, 2.375f}, {-0.625f, 0.375f}},
     },
    {
     .name     = "normal without transform",
     .material = MW_BASE(MW_TRANSFORM("\"scale\":[2,0.5],\"offset\":[0.25,0.125]")) MW_NORMAL(""),
     .has_base = BR_TRUE,
     .has_bump = BR_TRUE,
     .base_uv  = {{0.25f, 0.125f}, {2.25f, 0.125f}, {0.25f, 0.625f}},
     .bump_uv  = MW_IDENTITY,
     },
    {
     .name     = "missing set",
     .material = MW_NORMAL(",\"texCoord\":2"),
     .base_uv  = MW_IDENTITY,
     },
    {
     .name     = "singular base transform",
     .material = MW_BASE(MW_TRANSFORM("\"scale\":[0,1]")) MW_NORMAL(""),
     .has_base = BR_TRUE,
     .base_uv  = {{0, 0}, {0, 0}, {0, 1}},
     },
};

static void mw_gltf_assert_uv(const char *name, const char *map, int point, const br_vector2 *actual, const float expected[2])
{
    float u = BrScalarToFloat(actual->v[0]), v = BrScalarToFloat(actual->v[1]);
    if(!(fabsf(u - expected[0]) <= 1e-4f && fabsf(v - expected[1]) <= 1e-4f))
        MW_FAIL("glTF %s: %s point %d mapped to (%g, %g), expected (%g, %g).", name, map, point, u, v, expected[0], expected[1]);
}

static void mw_gltf_check(const struct mw_gltf_case *c, const br_fmt_results *results)
{
    static const float uv[2][3][2] = {
        {{0.125f, 0.25f},  {0.375f, 0.5f},  {0.5f, 0.75f} },
        {{0.875f, 0.125f}, {0.75f, 0.375f}, {0.5f, 0.625f}}
    };
    static const br_vector2 basis[] = {{{0, 0}}, {{BR_SCALAR(1.0), 0}}, {{0, BR_SCALAR(1.0)}}};

    if(results->nmaterials != 1 || results->materials[0] == NULL || results->nmodels != 1 || results->models[0] == NULL ||
       results->models[0]->nvertices != 3 || results->npixelmaps != 2 || results->pixelmaps[0] == NULL || results->pixelmaps[1] == NULL) {
        MW_FAIL("glTF %s: missing material, model vertices, or images.", c->name);
        return;
    }

    const br_material *mat = results->materials[0];
    if(mat->colour_map != (c->has_base ? results->pixelmaps[0] : NULL))
        MW_FAIL("glTF %s: wrong base colour image.", c->name);
    if(mat->bump_map != (c->has_bump ? results->pixelmaps[1] : NULL))
        MW_FAIL("glTF %s: wrong normal image.", c->name);
    if(c->has_bump && mat->bump_scale != BR_SCALAR(0.5))
        MW_FAIL("glTF %s: normal scale did not survive.", c->name);

    for(int i = 0; i < 3; ++i) {
        br_vector2 mapped, bump;
        mw_gltf_assert_uv(c->name, "vertex UV", i, &results->models[0]->vertices[i].map, uv[c->uv_set][i]);
        BrMatrix23ApplyP(&mapped, &basis[i], &mat->map_transform);
        mw_gltf_assert_uv(c->name, "base transform", i, &mapped, c->base_uv[i]);
        if(c->has_bump) {
            BrMatrix23ApplyP(&bump, &basis[i], &mat->bump_map_transform);
            BrMatrix23ApplyP(&mapped, &bump, &mat->map_transform);
            mw_gltf_assert_uv(c->name, "normal transform", i, &mapped, c->bump_uv[i]);
        }
    }
}

static void mw_gltf_witness(void)
{
    for(br_size_t i = 0; i < BR_ASIZE(mw_gltf_cases); ++i) {
        const struct mw_gltf_case *c    = mw_gltf_cases + i;
        char                      *json = BrResSprintf(NULL, mw_gltf_fixture, c->name, c->material);
        if(json == NULL) {
            MW_FAIL("glTF %s: could not allocate fixture.", c->name);
            continue;
        }

        memfile_store   store    = {.filename = "witness.gltf", .data = (br_uint_8 *)json, .size = BrStrLen(json)};
        br_filesystem  *previous = BrMemFileInstall(&store);
        br_fmt_results *results  = BrFmtGLTFActorLoadMany(store.filename, NULL);
        BrMemFileRestore(previous);
        BrResFree(json);

        if(results == NULL) {
            MW_FAIL("glTF %s: could not load fixture.", c->name);
            continue;
        }

        mw_gltf_check(c, results);
        for(br_size_t j = 0; j < results->nactors; ++j)
            if(results->actors[j] != NULL)
                BrActorFree(results->actors[j]);
        for(br_size_t j = 0; j < results->nmodels; ++j)
            if(results->models[j] != NULL)
                BrModelFree(results->models[j]);
        for(br_size_t j = 0; j < results->nmaterials; ++j)
            if(results->materials[j] != NULL)
                BrMaterialFree(results->materials[j]);
        BrResFree(results);
    }
}

br_error rt_material_witness_run(rt_demo *demo)
{
    br_pixelmap *map = NULL;
    br_material *pop = NULL, *back = NULL;
    br_material *plain = NULL, *plain_back = NULL;
    br_error     result = BRE_FAIL;

    (void)demo;
    mw_failures = 0;

    if((map = BrPixelmapAllocate(BR_PMT_RGBA_8888, 2, 2, NULL, BR_PMAF_NORMAL)) == NULL) {
        BrLogError("RT", "material witness: could not allocate the map.");
        return BRE_FAIL;
    }

    BrPixelmapPixelSet(map, 0, 0, BR_COLOUR_RGBA(1, 2, 3, 255));
    BrPixelmapPixelSet(map, 1, 0, BR_COLOUR_RGBA(4, 5, 6, 255));
    BrPixelmapPixelSet(map, 0, 1, BR_COLOUR_RGBA(7, 8, 9, 255));
    BrPixelmapPixelSet(map, 1, 1, BR_COLOUR_RGBA(10, 11, 12, 255));
    map->identifier = BrResStrDup(map, "material-witness-map");
    BrMapUpdate(map, BR_MAPU_ALL);
    BrMapAdd(map);

    /*
     * A material with every field away from its default, and one with every field at it. The
     * second is the case that catches a writer that omits a default and a reader whose copy of
     * that default has drifted.
     */
    if((pop = BrMaterialAllocate("material-witness")) == NULL || (plain = BrMaterialAllocate("material-witness-default")) == NULL) {
        BrLogError("RT", "material witness: could not allocate the materials.");
        goto out;
    }

    if(mw_populate(pop, map) != BRE_OK)
        goto out;

    /* The binary format. */
    if(mw_round_trip("witness.mat", mw_save_binary, BrMaterialLoad, pop, &back) != BRE_OK)
        goto out;
    mw_compare_binary(pop, back);
    BrLogInfo("RT", "material witness: the binary format carried %s.", mw_failures == 0 ? "every field" : "not every field");

    /* And the script. */
    BrMaterialFree(back);
    back = NULL;
    if(mw_round_trip("witness.scr", mw_save_script, BrFmtScriptMaterialLoad, pop, &back) != BRE_OK)
        goto out;
    mw_compare_script(pop, back);

    /* The all-defaults material, both ways. */
    BrMaterialFree(back);
    back = NULL;
    if(mw_round_trip("witness-default.mat", mw_save_binary, BrMaterialLoad, plain, &plain_back) != BRE_OK)
        goto out;
    mw_compare_binary(plain, plain_back);
    BrMaterialFree(plain_back);
    plain_back = NULL;
    if(mw_round_trip("witness-default.scr", mw_save_script, BrFmtScriptMaterialLoad, plain, &plain_back) != BRE_OK)
        goto out;
    mw_compare_script(plain, plain_back);

    mw_gltf_witness();
    if(mw_failures == 0)
        BrLogInfo("RT", "material witness passed (.mat and .scr round trips and glTF texture mapping).");

    result = mw_failures == 0 ? BRE_OK : BRE_FAIL;

out:
    if(back != NULL)
        BrMaterialFree(back);
    if(plain_back != NULL)
        BrMaterialFree(plain_back);
    if(pop != NULL)
        BrMaterialFree(pop);
    if(plain != NULL)
        BrMaterialFree(plain);
    if(map != NULL) {
        BrMapRemove(map);
        BrPixelmapFree(map);
    }
    return result;
}
