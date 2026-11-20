/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: savescr.c 1.2 1998/03/05 20:24:07 jon Exp $
 * $Locker: $
 *
 * Save a Material Script file
 */
#include "brender.h"
#include "fmt.h"
#include "matflag.h"

/*
 * The default material used to load a script
 */
extern const br_material _DefaultScriptMaterial;

static const struct {
    const char *name;
    int         value;
} DepthTestNames[] = {
    {"gt", BR_MATM_DEPTH_TEST_GT},
    {"ge", BR_MATM_DEPTH_TEST_GE},
    {"eq", BR_MATM_DEPTH_TEST_EQ},
    {"ne", BR_MATM_DEPTH_TEST_NE},
    {"le", BR_MATM_DEPTH_TEST_LE},
    {"lt", BR_MATM_DEPTH_TEST_LT},
    {"nv", BR_MATM_DEPTH_TEST_NV},
    {"al", BR_MATM_DEPTH_TEST_AL},
};

static const struct {
    const char *name;
    int         value;
} BlendModeNames[] = {
    {"standard",      BR_MATM_BLEND_MODE_STANDARD     },
    {"summed",        BR_MATM_BLEND_MODE_SUMMED       },
    {"dimmed",        BR_MATM_BLEND_MODE_DIMMED       },
    {"premultiplied", BR_MATM_BLEND_MODE_PREMULTIPLIED},
};

static const struct {
    const char *name;
    int         value;
} WidthLimitNames[] = {
    {"wrap",   BR_MATM_MAP_WIDTH_LIMIT_WRAP  },
    {"clamp",  BR_MATM_MAP_WIDTH_LIMIT_CLAMP },
    {"mirror", BR_MATM_MAP_WIDTH_LIMIT_MIRROR},
};

static const struct {
    const char *name;
    int         value;
} HeightLimitNames[] = {
    {"wrap",   BR_MATM_MAP_HEIGHT_LIMIT_WRAP  },
    {"clamp",  BR_MATM_MAP_HEIGHT_LIMIT_CLAMP },
    {"mirror", BR_MATM_MAP_HEIGHT_LIMIT_MIRROR},
};

static const struct {
    const char *name;
    int         value;
} ShadingModeNames[] = {
    {.name = "flat",    .value = BR_MATM_SHADING_MODE_FLAT   },
    {.name = "gouraud", .value = BR_MATM_SHADING_MODE_GOURAUD},
    {.name = "phong",   .value = BR_MATM_SHADING_MODE_PHONG  },
};

static void WriteScriptMaterial(br_material *mat, void *df)
{
    const br_material_flag *f;
    int                     i, j;

    BrFilePrintf(df, "\nmaterial = [\n");

    if(mat->identifier)
        BrFilePrintf(df, "    identifier = \"%s\";\n", mat->identifier);

    /*
     * Flags
     */
    if(mat->flags != _DefaultScriptMaterial.flags) {
        BrFilePrintf(df, "    flags = [");

        for(j = 0, f = MaterialFlagNames; f->name; f++)
            if(f->canonical && (mat->flags & f->value)) {
                BrFilePrintf(df, "%s%s", j ? ", " : "", f->name);
                j++;
            }

        BrFilePrintf(df, "];\n");
    }

    /*
     * Lighting parameters
     */
    if(mat->colour != _DefaultScriptMaterial.colour)
        BrFilePrintf(df, "    colour = [%d,%d,%d];\n", BR_RED(mat->colour), BR_GRN(mat->colour), BR_BLU(mat->colour));

    if(mat->opacity != _DefaultScriptMaterial.opacity)
        BrFilePrintf(df, "    opacity = %d;\n", mat->opacity);

    if(mat->ka != _DefaultScriptMaterial.ka)
        BrFilePrintf(df, "    ambient = %f;\n", BrScalarToFloat(BrUFractionToScalar(mat->ka)));

    if(mat->kd != _DefaultScriptMaterial.kd)
        BrFilePrintf(df, "    diffuse = %f;\n", BrScalarToFloat(BrUFractionToScalar(mat->kd)));

    if(mat->ks != _DefaultScriptMaterial.ks)
        BrFilePrintf(df, "    specular = %f;\n", BrScalarToFloat(BrUFractionToScalar(mat->ks)));

    if(mat->power != _DefaultScriptMaterial.power)
        BrFilePrintf(df, "    power = %f;\n", BrScalarToFloat(mat->power));

    if(mat->index_base != _DefaultScriptMaterial.index_base)
        BrFilePrintf(df, "    index_base = %d;\n", mat->index_base);

    if(mat->index_range != _DefaultScriptMaterial.index_range)
        BrFilePrintf(df, "    index_range = %d;\n", mat->index_range);

    /*
     * Map Transform
     */
    for(i = 0; i < 3; i++)
        if(mat->map_transform.m[i][0] != _DefaultScriptMaterial.map_transform.m[i][0] ||
           mat->map_transform.m[i][1] != _DefaultScriptMaterial.map_transform.m[i][1])
            break;

    if(i < 3) {
        /*
         * Matrices were different
         */
        BrFilePrintf(df, "    map_transform = [\n");
        for(i = 0; i < 3; i++)
            BrFilePrintf(df, "                     [%f,%f]%s\n", BrScalarToFloat(mat->map_transform.m[i][0]),
                         BrScalarToFloat(mat->map_transform.m[i][1]), i != 2 ? "," : "");
        BrFilePrintf(df, "                    ];\n");
    }

    if((mat->mode & BR_MATM_DEPTH_TEST_MASK) != (_DefaultScriptMaterial.mode & BR_MATM_DEPTH_TEST_MASK))
        for(i = 0; i < BR_ASIZE(DepthTestNames); i++)
            if((mat->mode & BR_MATM_DEPTH_TEST_MASK) == DepthTestNames[i].value) {
                BrFilePrintf(df, "    depth_test = %s;\n", DepthTestNames[i].name);
                break;
            }

    if((mat->mode & BR_MATM_BLEND_MODE_MASK) != (_DefaultScriptMaterial.mode & BR_MATM_BLEND_MODE_MASK))
        for(i = 0; i < BR_ASIZE(BlendModeNames); i++)
            if((mat->mode & BR_MATM_BLEND_MODE_MASK) == BlendModeNames[i].value) {
                BrFilePrintf(df, "    blend_mode = %s;\n", BlendModeNames[i].name);
                break;
            }

    if((mat->mode & BR_MATM_MAP_WIDTH_LIMIT_MASK) != (_DefaultScriptMaterial.mode & BR_MATM_MAP_WIDTH_LIMIT_MASK))
        for(i = 0; i < BR_ASIZE(WidthLimitNames); i++)
            if((mat->mode & BR_MATM_MAP_WIDTH_LIMIT_MASK) == WidthLimitNames[i].value) {
                BrFilePrintf(df, "    map_width_limit = %s;\n", WidthLimitNames[i].name);
                break;
            }

    if((mat->mode & BR_MATM_MAP_HEIGHT_LIMIT_MASK) != (_DefaultScriptMaterial.mode & BR_MATM_MAP_HEIGHT_LIMIT_MASK))
        for(i = 0; i < BR_ASIZE(HeightLimitNames); i++)
            if((mat->mode & BR_MATM_MAP_HEIGHT_LIMIT_MASK) == HeightLimitNames[i].value) {
                BrFilePrintf(df, "    map_height_limit = %s;\n", HeightLimitNames[i].name);
                break;
            }

    if((mat->mode & BR_MATM_SHADING_MODE_MASK) != (_DefaultScriptMaterial.mode & BR_MATM_SHADING_MODE_MASK)) {
        for(i = 0; i < BR_ASIZE(ShadingModeNames); i++) {
            if((mat->mode & BR_MATM_SHADING_MODE_MASK) == ShadingModeNames[i].value) {
                BrFilePrintf(df, "    shading_mode = %s;\n", ShadingModeNames[i].name);
                break;
            }
        }
    } else if(mat->flags & BR_MATF_SMOOTH) {
        for(i = 0; i < BR_ASIZE(ShadingModeNames); i++) {
            if(BR_MATM_SHADING_MODE_GOURAUD == ShadingModeNames[i].value) {
                BrFilePrintf(df, "    shading_mode = %s;\n", ShadingModeNames[i].name);
                break;
            }
        }
    }

    /*
     * Maps and Tables
     */
    if(mat->colour_map && mat->colour_map->identifier)
        BrFilePrintf(df, "    colour_map = \"%s\";\n", mat->colour_map->identifier);

    if(mat->screendoor && mat->screendoor->identifier)
        BrFilePrintf(df, "    screendoor = \"%s\";\n", mat->screendoor->identifier);

    if(mat->index_shade && mat->index_shade->identifier)
        BrFilePrintf(df, "    index_shade = \"%s\";\n", mat->index_shade->identifier);

    if(mat->index_blend && mat->index_blend->identifier)
        BrFilePrintf(df, "    index_blend = \"%s\";\n", mat->index_blend->identifier);

    BrFilePrintf(df, "];\n");
}

/*
 * Write out a material script
 */
br_uint_32 BR_PUBLIC_ENTRY BrFmtScriptMaterialSaveMany(const char *filename, br_material **materials, br_uint_16 num)
{
    void *df;
    int   i, count;

    /*
     * Open file and write header
     */
    df = BrFileOpenWrite(filename, 1);

    if(df == NULL)
        return 0;

    /*
     * Write a header
     */
    BrFilePutLine("# BRender Material Script", df);
    BrFilePutLine("#", df);

    if(materials) {
        for(i = 0; i < num; i++)
            WriteScriptMaterial(materials[i], df);
        count = num;
    } else {
        BrMaterialEnum(NULL, (br_material_enum_cbfn *)WriteScriptMaterial, df);
        count = BrMaterialCount(NULL);
    }

    BrFileClose(df);

    return count;
}

br_uint_32 BR_PUBLIC_ENTRY BrFmtScriptMaterialSave(const char *filename, br_material *ptr)
{
    return BrFmtScriptMaterialSaveMany(filename, &ptr, 1);
}
