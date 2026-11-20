#include "brender.h"
#include "matflag.h"

/*
 * Do not reorder, it will mess up the ordering when writing .scr files.
 */
const br_material_flag MaterialFlagNames[] = {
    {.name = "light",               .value = BR_MATF_LIGHT,               .canonical = BR_TRUE },
    {.name = "prelit",              .value = BR_MATF_PRELIT,              .canonical = BR_TRUE },
    {.name = "environment",         .value = BR_MATF_ENVIRONMENT_I,       .canonical = BR_TRUE },
    {.name = "environment_local",   .value = BR_MATF_ENVIRONMENT_L,       .canonical = BR_TRUE },
    {.name = "perspective",         .value = BR_MATF_PERSPECTIVE,         .canonical = BR_TRUE },
    {.name = "decal",               .value = BR_MATF_DECAL,               .canonical = BR_TRUE },
    {.name = "i_from_u",            .value = BR_MATF_I_FROM_U,            .canonical = BR_TRUE },
    {.name = "i_from_v",            .value = BR_MATF_I_FROM_V,            .canonical = BR_TRUE },
    {.name = "u_from_i",            .value = BR_MATF_U_FROM_I,            .canonical = BR_TRUE },
    {.name = "v_from_i",            .value = BR_MATF_V_FROM_I,            .canonical = BR_TRUE },
    {.name = "always_visible",      .value = BR_MATF_ALWAYS_VISIBLE,      .canonical = BR_TRUE },
    {.name = "two_sided",           .value = BR_MATF_TWO_SIDED,           .canonical = BR_TRUE },
    {.name = "force_front",         .value = BR_MATF_FORCE_FRONT,         .canonical = BR_TRUE },
    {.name = "dither",              .value = BR_MATF_DITHER,              .canonical = BR_TRUE },
    {.name = "map_antialiasing",    .value = BR_MATF_MAP_ANTIALIASING,    .canonical = BR_TRUE },
    {.name = "map_interpolation",   .value = BR_MATF_MAP_INTERPOLATION,   .canonical = BR_TRUE },
    {.name = "mip_interpolation",   .value = BR_MATF_MIP_INTERPOLATION,   .canonical = BR_TRUE },
    {.name = "fog_local",           .value = BR_MATF_FOG_LOCAL,           .canonical = BR_TRUE },
    {.name = "subdivide",           .value = BR_MATF_SUBDIVIDE,           .canonical = BR_TRUE },
    {.name = "quad_mapping",        .value = BR_MATF_QUAD_MAPPING,        .canonical = BR_TRUE },
    {.name = "force_back",          .value = BR_MATF_FORCE_BACK,          .canonical = BR_TRUE },
    {.name = "inhibit_depth_write", .value = BR_MATF_INHIBIT_DEPTH_WRITE, .canonical = BR_TRUE },
    {.name = "prealpha",            .value = BR_MATF_PREALPHA,            .canonical = BR_TRUE },
    {.name = "disable_colour_key",  .value = BR_MATF_DISABLE_COLOUR_KEY,  .canonical = BR_TRUE },
    {.name = "smooth_alpha",        .value = BR_MATF_SMOOTH_ALPHA,        .canonical = BR_TRUE },
    {.name = "blend",               .value = BR_MATF_BLEND,               .canonical = BR_TRUE },
    {.name = NULL,                  .value = 0,                           .canonical = BR_FALSE},
};
