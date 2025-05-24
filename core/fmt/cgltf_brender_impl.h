/*
 * BRender-specific modifications to cgltf.
 *
 * Try to keep these out of the cgltf.h as much as possible to ease updating.
 */
#ifndef CGLTF_BRENDER_IMPL_H_
#define CGLTF_BRENDER_IMPL_H_

#if !defined(CGLTF_IMPLEMENTATION)
#error This must only be included from cgltf.h
#endif

static int cgltf_parse_json_brender_light_mappings(cgltf_options *options, jsmntok_t const *tokens, int i, const uint8_t *json_chunk,
                                                   cgltf_brender_light **out_light)
{
    CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

    int size = tokens[i].size;
    ++i;

    for(int j = 0; j < size; ++j) {
        CGLTF_CHECK_KEY(tokens[i]);

        if(cgltf_json_strcmp(tokens + i, json_chunk, "light") == 0) {
            ++i;
            CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_PRIMITIVE);
            *out_light = CGLTF_PTRINDEX(cgltf_brender_light, cgltf_json_to_int(tokens + i, json_chunk));
            ++i;
        } else {
            i = cgltf_skip_json(tokens, i + 1);
        }

        if(i < 0) {
            return i;
        }
    }

    return i;
}

static int cgltf_parse_json_brender_material_mappings(cgltf_options *options, jsmntok_t const *tokens, int i, const uint8_t *json_chunk,
                                                      cgltf_brender_material **out_material)
{
    CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

    int size = tokens[i].size;
    ++i;

    for(int j = 0; j < size; ++j) {
        CGLTF_CHECK_KEY(tokens[i]);

        if(cgltf_json_strcmp(tokens + i, json_chunk, "material") == 0) {
            ++i;
            CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_PRIMITIVE);
            *out_material = CGLTF_PTRINDEX(cgltf_brender_material, cgltf_json_to_int(tokens + i, json_chunk));
            ++i;
        } else {
            i = cgltf_skip_json(tokens, i + 1);
        }

        if(i < 0) {
            return i;
        }
    }

    return i;
}

static int cgltf_parse_json_brender_light(cgltf_options *options, jsmntok_t const *tokens, int i, const uint8_t *json_chunk,
                                          cgltf_brender_light *out_light)
{
    CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

    out_light->type          = cgltf_brender_light_type_direct;
    out_light->colour[0]     = 1.0f;
    out_light->colour[1]     = 1.0f;
    out_light->colour[2]     = 1.0f;
    out_light->attenuation_c = 1.0f;
    out_light->attenuation_l = 0.0f;
    out_light->attenuation_q = 0.0f;
    out_light->cone_outer    = 15.0f / 360.0f;
    out_light->cone_inner    = 10.0f / 360.0f;
    out_light->radius_outer  = 0.0f;
    out_light->radius_inner  = 0.0f;

    int size = tokens[i].size;
    ++i;

    for(int j = 0; j < size; ++j) {
        CGLTF_CHECK_KEY(tokens[i]);

        if(cgltf_json_strcmp(tokens + i, json_chunk, "identifier") == 0) {
            i = cgltf_parse_json_string(options, tokens, i + 1, json_chunk, &out_light->identifier);
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "type") == 0) {
            ++i;
            if(cgltf_json_strcmp(tokens + i, json_chunk, "point") == 0) {
                out_light->type = cgltf_brender_light_type_point;
            } else if(cgltf_json_strcmp(tokens + i, json_chunk, "direct") == 0) {
                out_light->type = cgltf_brender_light_type_direct;
            } else if(cgltf_json_strcmp(tokens + i, json_chunk, "spot") == 0) {
                out_light->type = cgltf_brender_light_type_spot;
            } else if(cgltf_json_strcmp(tokens + i, json_chunk, "ambient") == 0) {
                out_light->type = cgltf_brender_light_type_ambient;
            }
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "view_space") == 0) {
            ++i;
            out_light->view_space = cgltf_json_to_bool(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "linear_falloff") == 0) {
            ++i;
            out_light->linear_falloff = cgltf_json_to_bool(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "colour") == 0) {
            i = cgltf_parse_json_float_array(tokens, i + 1, json_chunk, out_light->colour, 3);
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "attenuation_c") == 0) {
            ++i;
            out_light->attenuation_c = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "attenuation_l") == 0) {
            ++i;
            out_light->attenuation_l = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "attenuation_l") == 0) {
            ++i;
            out_light->attenuation_q = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "cone_outer") == 0) {
            ++i;
            out_light->cone_outer = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "cone_inner") == 0) {
            ++i;
            out_light->cone_inner = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "radius_outer") == 0) {
            ++i;
            out_light->radius_outer = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "radius_inner") == 0) {
            ++i;
            out_light->radius_inner = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else {
            i = cgltf_skip_json(tokens, i + 1);
        }

        if(i < 0) {
            return i;
        }
    }

    return i;
}

static int cgltf_parse_json_brender_lights(cgltf_options *options, jsmntok_t const *tokens, int i, const uint8_t *json_chunk, cgltf_data *out_data)
{
    i = cgltf_parse_json_array(options, tokens, i, json_chunk, sizeof(cgltf_brender_light), (void **)&out_data->brender_lights,
                               &out_data->brender_lights_count);
    if(i < 0) {
        return i;
    }

    for(cgltf_size j = 0; j < out_data->brender_lights_count; ++j) {
        i = cgltf_parse_json_brender_light(options, tokens, i, json_chunk, &out_data->brender_lights[j]);
        if(i < 0) {
            return i;
        }
    }
    return i;
}

static int cgltf_parse_json_brender_material(cgltf_options *options, jsmntok_t const *tokens, int i, const uint8_t *json_chunk,
                                             cgltf_brender_material *out_material)
{
    CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

    out_material->colour[0] = 1.0f;
    out_material->colour[1] = 1.0f;
    out_material->colour[2] = 1.0f;

    out_material->opacity = 1.0f;
    out_material->ka      = 1.0f;
    out_material->kd      = 0.7f;
    out_material->ks      = 0.0f;
    out_material->power   = 20.0f;

    out_material->flags = 1;
    out_material->mode  = 4;

    out_material->map_transform[0] = 1.0f;
    out_material->map_transform[3] = 1.0f;

    out_material->index_base  = 10;
    out_material->index_range = 31;

    out_material->fog_min = 0.0f;
    out_material->fog_max = 0.0f;

    out_material->fog_colour[0] = 0.0f;
    out_material->fog_colour[1] = 0.0f;
    out_material->fog_colour[2] = 0.0f;

    out_material->subdivide_tolerance = 0;
    out_material->depth_bias          = 0.0f;

    int size = tokens[i].size;
    ++i;

    for(int j = 0; j < size; ++j) {
        CGLTF_CHECK_KEY(tokens[i]);

        if(cgltf_json_strcmp(tokens + i, json_chunk, "identifier") == 0) {
            i = cgltf_parse_json_string(options, tokens, i + 1, json_chunk, &out_material->identifier);
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "colour") == 0) {
            i = cgltf_parse_json_float_array(tokens, i + 1, json_chunk, out_material->colour, 3);
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "opacity") == 0) {
            ++i;
            out_material->opacity = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "ka") == 0) {
            ++i;
            out_material->ka = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "kd") == 0) {
            ++i;
            out_material->kd = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "ks") == 0) {
            ++i;
            out_material->ks = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "power") == 0) {
            ++i;
            out_material->power = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "flags") == 0) {
            ++i;
            out_material->flags = cgltf_json_to_int(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "mode") == 0) {
            ++i;
            out_material->mode = cgltf_json_to_int(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "map_transform") == 0) {
            i = cgltf_parse_json_float_array(tokens, i + 1, json_chunk, out_material->map_transform, 6);
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "index_base") == 0) {
            ++i;
            out_material->index_base = cgltf_json_to_int(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "index_range") == 0) {
            ++i;
            out_material->index_range = cgltf_json_to_int(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "colour_map") == 0) {
            ++i;
            out_material->colour_map = CGLTF_PTRINDEX(cgltf_image, cgltf_json_to_int(tokens + i, json_chunk));
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "screendoor") == 0) {
            ++i;
            out_material->screendoor = CGLTF_PTRINDEX(cgltf_image, cgltf_json_to_int(tokens + i, json_chunk));
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "index_shade") == 0) {
            ++i;
            out_material->index_shade = CGLTF_PTRINDEX(cgltf_image, cgltf_json_to_int(tokens + i, json_chunk));
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "index_blend") == 0) {
            ++i;
            out_material->index_blend = CGLTF_PTRINDEX(cgltf_image, cgltf_json_to_int(tokens + i, json_chunk));
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "index_fog") == 0) {
            ++i;
            out_material->index_fog = CGLTF_PTRINDEX(cgltf_image, cgltf_json_to_int(tokens + i, json_chunk));
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "fog_min") == 0) {
            ++i;
            out_material->fog_min = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "fog_max") == 0) {
            ++i;
            out_material->fog_max = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "fog_colour") == 0) {
            i = cgltf_parse_json_float_array(tokens, i + 1, json_chunk, out_material->fog_colour, 3);
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "subdivide_tolerance") == 0) {
            ++i;
            out_material->subdivide_tolerance = cgltf_json_to_int(tokens + i, json_chunk);
            ++i;
        } else if(cgltf_json_strcmp(tokens + i, json_chunk, "depth_bias") == 0) {
            ++i;
            out_material->depth_bias = cgltf_json_to_float(tokens + i, json_chunk);
            ++i;
        } else {
            i = cgltf_skip_json(tokens, i + 1);
        }

        if(i < 0) {
            return i;
        }
    }

    return i;
}

static int cgltf_parse_json_brender_materials(cgltf_options *options, jsmntok_t const *tokens, int i, const uint8_t *json_chunk, cgltf_data *out_data)
{
    i = cgltf_parse_json_array(options, tokens, i, json_chunk, sizeof(cgltf_brender_material), (void **)&out_data->brender_materials,
                               &out_data->brender_materials_count);
    if(i < 0) {
        return i;
    }

    for(cgltf_size j = 0; j < out_data->brender_materials_count; ++j) {
        i = cgltf_parse_json_brender_material(options, tokens, i, json_chunk, &out_data->brender_materials[j]);
        if(i < 0) {
            return i;
        }
    }
    return i;
}

static int cgltf_parse_BR_lights(cgltf_options *options, jsmntok_t const *tokens, int i, const uint8_t *json_chunk, cgltf_data *out_data)
{
    ++i;

    CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

    int data_size = tokens[i].size;
    ++i;

    for(int m = 0; m < data_size; ++m) {
        CGLTF_CHECK_KEY(tokens[i]);

        if(cgltf_json_strcmp(tokens + i, json_chunk, "lights") == 0) {
            i = cgltf_parse_json_brender_lights(options, tokens, i + 1, json_chunk, out_data);
        } else {
            i = cgltf_skip_json(tokens, i + 1);
        }

        if(i < 0) {
            return i;
        }
    }

    return i;
}

static int cgltf_parse_BR_materials(cgltf_options *options, jsmntok_t const *tokens, int i, const uint8_t *json_chunk, cgltf_data *out_data)
{
    ++i;

    CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

    int data_size = tokens[i].size;
    ++i;

    for(int m = 0; m < data_size; ++m) {
        CGLTF_CHECK_KEY(tokens[i]);

        if(cgltf_json_strcmp(tokens + i, json_chunk, "materials") == 0) {
            i = cgltf_parse_json_brender_materials(options, tokens, i + 1, json_chunk, out_data);
        } else {
            i = cgltf_skip_json(tokens, i + 1);
        }

        if(i < 0) {
            return i;
        }
    }

    return i;
}

#endif /* CGLTF_BRENDER_IMPL_H_ */
