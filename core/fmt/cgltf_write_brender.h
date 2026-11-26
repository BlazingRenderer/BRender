#ifndef CGLTF_WRITE_BRENDER_H_
#define CGLTF_WRITE_BRENDER_H_

#if !defined(CGLTF_WRITE_IMPLEMENTATION)
#error This must only be included from cgltf_write.h
#endif

#include "matflag.h"

static const char *cgltf_str_from_brender_light_type(cgltf_brender_light_type light_type)
{
    switch(light_type) {
        case cgltf_brender_light_type_point:
            return "point";
        case cgltf_brender_light_type_direct:
            return "direct";
        case cgltf_brender_light_type_spot:
            return "spot";
        case cgltf_brender_light_type_ambient:
            return "ambient";
        default:
            return NULL;
    }
}

static void cgltf_write_brender_light(cgltf_write_context *context, const cgltf_brender_light *br_light)
{
    context->extension_flags |= CGLTF_EXTENSION_FLAG_BR_LIGHTS;

    cgltf_write_line(context, "{");
    cgltf_write_strprop(context, "identifier", br_light->identifier);

    cgltf_write_strprop(context, "type", cgltf_str_from_brender_light_type(br_light->type));

    cgltf_write_boolprop_optional(context, "view_space", br_light->view_space, false);
    cgltf_write_boolprop_optional(context, "linear_falloff", br_light->linear_falloff, false);

    cgltf_write_floatarrayprop(context, "colour", br_light->colour, 3);

    cgltf_write_floatprop(context, "attenuation_c", br_light->attenuation_c, 1.0f);
    cgltf_write_floatprop(context, "attenuation_l", br_light->attenuation_l, 0.0f);
    cgltf_write_floatprop(context, "attenuation_q", br_light->attenuation_q, 0.0f);

    if(br_light->type == cgltf_brender_light_type_spot) {
        cgltf_write_floatprop(context, "cone_outer", br_light->cone_outer, 15.0f / 360.0f);
        cgltf_write_floatprop(context, "cone_inner", br_light->cone_inner, 10.0f / 360.0f);
    }

    if(br_light->linear_falloff) {
        cgltf_write_floatprop(context, "radius_outer", br_light->radius_outer, 0.0f);
        cgltf_write_floatprop(context, "radius_inner", br_light->radius_inner, 0.0f);
    }

    if(br_light->regions_count > 0) {
        cgltf_write_line(context, "\"volume\": {");
        cgltf_write_floatprop(context, "falloff_distance", br_light->falloff_distance, 0.0f);

        cgltf_write_line(context, "\"regions\": [");
        for(cgltf_size i = 0; i < br_light->regions_count; ++i) {
            const cgltf_brender_light_region *region = br_light->regions + i;

            cgltf_write_line(context, "[");
            for(cgltf_size j = 0; j < region->planes_count; ++j) {
                const cgltf_float *plane = region->planes[j];

                cgltf_write_indent(context);
                CGLTF_SPRINTF("[ %.*g, %.*g, %.*g, %.*g ]", CGLTF_DECIMAL_DIG, plane[0], CGLTF_DECIMAL_DIG, plane[1], CGLTF_DECIMAL_DIG,
                              plane[2], CGLTF_DECIMAL_DIG, plane[3]);
                context->needs_comma = 1;
            }
            cgltf_write_line(context, "]");
        }
        cgltf_write_line(context, "]");

        cgltf_write_line(context, "}");
    }

    cgltf_write_line(context, "}");
}

/*
 * cgltf_write_intprop() but doesn't exclude the default value.
 */
static void cgltf_write_brender_intprop(cgltf_write_context *context, const char *label, int val)
{
    cgltf_write_indent(context);
    CGLTF_SPRINTF("\"%s\": %d", label, val);
    context->needs_comma = 1;
}

/*
 * cgltf_write_floatprop() but doesn't exclude the default value.
 */
static void cgltf_write_brender_floatprop(cgltf_write_context *context, const char *label, float val)
{
    cgltf_write_indent(context);
    CGLTF_SPRINTF("\"%s\": %.*g", label, CGLTF_DECIMAL_DIG, val);
    context->needs_comma = 1;

    if(context->cursor) {
        char *decimal_comma = strchr(context->cursor - context->tmp, ',');

        if(decimal_comma)
            *decimal_comma = '.';
    }
}

static void cgltf_write_brender_material_flags(cgltf_write_context *context, br_uint_64 flags)
{
    const br_material_flag *flag;
    br_uint_64              named   = 0;
    int                     written = 0;

    cgltf_write_indent(context);
    CGLTF_SPRINTF("\"flags\": [");

    for(flag = MaterialFlagNames; flag->name; flag++) {
        if(!flag->canonical || !(flags & flag->value))
            continue;

        CGLTF_SPRINTF("%s\"%s\"", written++ ? ", " : "", flag->name);
        named |= flag->value;
    }

    CGLTF_SPRINTF("]");
    context->needs_comma = 1;

    /*
     * A bit that no row names has no spelling to write. Say so: a flag the
     * engine acts on and this file does not carry is not a rounding error, and
     * it is the one thing a name cannot express that the old number could.
     */
    if(flags & ~named)
        BrLogWarn("GLTF", "material flags %#llx: the bits %#llx have no name and are not written", (unsigned long long)flags,
                  (unsigned long long)(flags & ~named));
}

static void cgltf_write_brender_material(cgltf_write_context *context, const cgltf_brender_material *br_material)
{
    context->extension_flags |= CGLTF_EXTENSION_FLAG_BR_MATERIALS;

    cgltf_write_line(context, "{");

    cgltf_write_strprop(context, "identifier", br_material->identifier);

    cgltf_write_floatarrayprop(context, "colour", br_material->colour, 3);

    cgltf_write_brender_floatprop(context, "opacity", br_material->opacity);

    cgltf_write_brender_floatprop(context, "ka", br_material->ka);
    cgltf_write_brender_floatprop(context, "kd", br_material->kd);
    cgltf_write_brender_floatprop(context, "ks", br_material->ks);
    cgltf_write_brender_floatprop(context, "power", br_material->power);

    cgltf_write_brender_material_flags(context, br_material->flags);
    cgltf_write_brender_intprop(context, "mode", br_material->mode);

    cgltf_write_floatarrayprop(context, "map_transform", br_material->map_transform, 6);

    cgltf_write_brender_intprop(context, "index_base", br_material->index_base);
    cgltf_write_brender_intprop(context, "index_range", br_material->index_range);

    if(br_material->colour_map) {
        CGLTF_WRITE_IDXPROP("colour_map", br_material->colour_map, context->data->images);
    }

    if(br_material->bump_map) {
        CGLTF_WRITE_IDXPROP("bump_map", br_material->bump_map, context->data->images);
    }

    if(br_material->screendoor) {
        CGLTF_WRITE_IDXPROP("screendoor", br_material->screendoor, context->data->images);
    }

    if(br_material->index_shade) {
        CGLTF_WRITE_IDXPROP("index_shade", br_material->index_shade, context->data->images);
    }

    if(br_material->index_blend) {
        CGLTF_WRITE_IDXPROP("index_blend", br_material->index_blend, context->data->images);
    }

    if(br_material->index_fog) {
        CGLTF_WRITE_IDXPROP("index_fog", br_material->index_fog, context->data->images);
    }

    cgltf_write_brender_floatprop(context, "fog_min", br_material->fog_min);
    cgltf_write_brender_floatprop(context, "fog_max", br_material->fog_max);

    cgltf_write_floatarrayprop(context, "fog_colour", br_material->fog_colour, 3);

    cgltf_write_brender_intprop(context, "subdivide_tolerance", br_material->subdivide_tolerance);

    cgltf_write_brender_floatprop(context, "depth_bias", br_material->depth_bias);

    cgltf_write_brender_floatprop(context, "bump_scale", br_material->bump_scale);

    cgltf_write_floatarrayprop(context, "bump_map_transform", br_material->bump_map_transform, 6);

    cgltf_write_brender_floatprop(context, "bump_depth", br_material->bump_depth);

    cgltf_write_line(context, "}");
}

#endif /* CGLTF_WRITE_BRENDER_H_ */
