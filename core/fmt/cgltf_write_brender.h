#ifndef CGLTF_WRITE_BRENDER_H_
#define CGLTF_WRITE_BRENDER_H_

#if !defined(CGLTF_WRITE_IMPLEMENTATION)
#error This must only be included from cgltf_write.h
#endif

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

    cgltf_write_line(context, "}");
}

static void cgltf_write_brender_material(cgltf_write_context *context, const cgltf_brender_material *br_material)
{
    context->extension_flags |= CGLTF_EXTENSION_FLAG_BR_MATERIALS;

    cgltf_write_line(context, "{");

    cgltf_write_strprop(context, "identifier", br_material->identifier);

    if(cgltf_check_floatarray(br_material->colour, 3, 1.0f)) {
        cgltf_write_floatarrayprop(context, "colour", br_material->colour, 3);
    }

    cgltf_write_floatprop(context, "opacity", br_material->opacity, 1.0f);

    cgltf_write_floatprop(context, "ka", br_material->ka, 0.1f);
    cgltf_write_floatprop(context, "kd", br_material->kd, 0.7f);
    cgltf_write_floatprop(context, "ks", br_material->ks, 0.0f);
    cgltf_write_floatprop(context, "power", br_material->power, 20.0f);

    cgltf_write_intprop(context, "flags", br_material->flags, 1);
    cgltf_write_intprop(context, "mode", br_material->mode, 4);

    cgltf_write_floatarrayprop(context, "map_transform", br_material->map_transform, 6);

    cgltf_write_intprop(context, "index_base", br_material->index_base, 10);
    cgltf_write_intprop(context, "index_range", br_material->index_range, 31);

    if(br_material->colour_map) {
        CGLTF_WRITE_IDXPROP("colour_map", br_material->colour_map, context->data->images);
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

    cgltf_write_floatprop(context, "fog_min", br_material->fog_min, 0.0f);
    cgltf_write_floatprop(context, "fog_max", br_material->fog_max, 0.0f);

    if(cgltf_check_floatarray(br_material->fog_colour, 3, 0.0f)) {
        cgltf_write_floatarrayprop(context, "fog_colour", br_material->fog_colour, 3);
    }

    cgltf_write_intprop(context, "subdivide_tolerance", br_material->subdivide_tolerance, 0);

    cgltf_write_floatprop(context, "depth_bias", br_material->depth_bias, 0.0f);

    cgltf_write_line(context, "}");
}

#endif /* CGLTF_WRITE_BRENDER_H_ */
