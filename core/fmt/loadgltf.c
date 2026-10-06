#include <limits.h>
#include <stdbool.h>
#include <brender.h>

#include "brassert.h"

#include "cbase64.h"
#include "brstb.h"
#include "cgltf.h"

/* Defined in animgltf.c */
br_animation_set      *BrFmtGLTFAnimBuildSet(const cgltf_data *data, br_uint_32 nnodes, void *res_parent);
br_animation_instance *BrFmtGLTFAnimBuildInstance(br_animation_set *set, br_actor **all_actors, br_uint_32 nnodes, void *res_parent);

/*
 * Default GLTF conversion options
 */
static br_gltf_options _BrDefaultGLTFOptions = {
    .base_path = NULL,
    .pm_type   = BR_PMT_RGBA_8888,
};

typedef struct br_gltf_load_state {
    cgltf_data            *data;
    br_fmt_results        *results;
    const br_gltf_options *options;
    const char            *base_path; /**< Directory for resolving relative URIs (with trailing slash). */

    br_actor **all_actors; /**< All actors in the same order as the file. */
} br_gltf_load_state;

static void *cgltf_alloc_br(void *user, cgltf_size size)
{
    return BrResAllocate(user, size, BR_MEMORY_SCRATCH);
}

static void cgltf_free_br(void *user, void *ptr)
{
    if(ptr == NULL)
        return;

    (void)user;
    BrResFree(ptr);
}

static cgltf_result cgltf_load_brfile(const cgltf_memory_options *memory_options, const cgltf_file_options *file_options, const char *path,
                                      cgltf_size *size, void **data)
{
    cgltf_size declared = *size;

    (void)file_options;

    if((*data = BrFileLoad(memory_options->user_data, path, size)) == NULL) {
        return cgltf_result_io_error;
    }

    if(declared != 0 && *size < declared) {
        BrLogError("GLTF",
                   "buffer file \"%s\" is %lu bytes where the file declares %lu; every buffer view inside the declared range would be "
                   "read past the end of the data that was loaded",
                   path, (unsigned long)*size, (unsigned long)declared);
        return cgltf_result_data_too_short;
    }

    return cgltf_result_success;
}

static void cgltf_release_brfile(const cgltf_memory_options *memory_options, const cgltf_file_options *file_options, void *data)
{
    (void)memory_options;
    (void)file_options;

    if(data == NULL)
        return;

    BrResFree(data);
}

typedef struct br_gltf_load_prim_info {
    const cgltf_attribute *attrib_position;
    const cgltf_attribute *attrib_normal;
    const cgltf_attribute *attrib_colour;
    const cgltf_attribute *attrib_texcoord;
    br_size_t              input_index_count;
    br_size_t              input_vertex_count;
    br_size_t              output_vertex_count;
    br_size_t              output_face_count;
    const cgltf_accessor  *indices;
} br_gltf_load_prim_info;

void *unbuild_data_url(void *res, const char *uri, size_t *size)
{
    const char         *comma;
    size_t              in_len, out_len;
    void               *data_out;
    cbase64_decodestate ds;

    if(BrStrNCmp(uri, "data:", 5) != 0)
        return NULL;

    comma = BrStrChr(uri, ',');
    if(!(comma && comma - uri >= 7 && BrStrNCmp(comma - 7, ";base64", 7) == 0))
        return NULL;

    in_len = BrStrLen(comma + 1);
    if((out_len = cbase64_calc_decoded_length(comma + 1, in_len)) == 0)
        return NULL;

    data_out = BrResAllocate(res, out_len, BR_MEMORY_APPLICATION);

    cbase64_init_decodestate(&ds);
    out_len = cbase64_decode_block(comma + 1, in_len, data_out, &ds);

    *size = out_len;
    return data_out;
}

/*
 * A lookup table's samples are carried raw in a marked data URI (see
 * cgltf_brender.h). Return the pixel type the marker names and set *components
 * to the PNG channel count it was written with, or return 0 for an ordinary
 * colour image. The marker names the type rather than the loader inferring it:
 * BR_PMT_RGB_555 and BR_PMT_RGB_565 are both two bytes per sample, and a shade
 * table's type must equal the output, so guessing would collapse them.
 */
static br_uint_8 load_table_type(const char *uri, int *components)
{
    if(BrStrNCmp(uri, CGLTF_BR_INDEX_8_PNG_URI, BR_ASIZE(CGLTF_BR_INDEX_8_PNG_URI) - 1) == 0) {
        *components = 1;
        return BR_PMT_INDEX_8;
    }

    if(BrStrNCmp(uri, CGLTF_BR_RGB_555_PNG_URI, BR_ASIZE(CGLTF_BR_RGB_555_PNG_URI) - 1) == 0) {
        *components = 2;
        return BR_PMT_RGB_555;
    }

    if(BrStrNCmp(uri, CGLTF_BR_RGB_565_PNG_URI, BR_ASIZE(CGLTF_BR_RGB_565_PNG_URI) - 1) == 0) {
        *components = 2;
        return BR_PMT_RGB_565;
    }

    if(BrStrNCmp(uri, CGLTF_BR_RGB_888_PNG_URI, BR_ASIZE(CGLTF_BR_RGB_888_PNG_URI) - 1) == 0) {
        *components = 3;
        return BR_PMT_RGB_888;
    }

    *components = 0;
    return 0;
}

static br_pixelmap *load_pixelmap(br_gltf_load_state *state, const cgltf_image *image)
{
    void        *raw_data, *pixels;
    br_pixelmap *pixelmap, *tmp;
    size_t       size;
    int          x, y, c;
    int          owns_raw   = 0;
    br_uint_8    table_type = 0;
    int          table_comp = 0;

    /*
     * GLB: images stored in buffer views.
     */
    if(image->buffer_view != NULL) {
        raw_data = (void *)cgltf_buffer_view_data(image->buffer_view);
        size     = image->buffer_view->size;
    } else if(image->uri != NULL) {
        /*
         * A lookup table: its payload is the pixelmap's raw samples, not colour,
         * and the marker names the type so the table comes back as the exact
         * pixelmap the renderer needs (see cgltf_brender.h).
         */
        table_type = load_table_type(image->uri, &table_comp);

        /*
         * Try base64 data URI first, then external file.
         */
        if((raw_data = unbuild_data_url(state, image->uri, &size)) != NULL) {
            owns_raw = 1;
        } else {
            char      *path = BrResSprintf(state, "%s%s", state->base_path, image->uri);
            cgltf_size fsize;

            raw_data = BrFileLoad(state, path, &fsize);
            BrResFree(path);
            if(raw_data == NULL)
                return NULL;
            size     = (size_t)fsize;
            owns_raw = 1;
        }
    } else {
        return NULL;
    }

    if(size > INT_MAX) {
        if(owns_raw)
            BrResFree(raw_data);
        return NULL;
    }

    if((pixels = stbi_load_from_memory(raw_data, (int)size, &x, &y, &c, table_type ? table_comp : 4)) == NULL) {
        if(owns_raw)
            BrResFree(raw_data);
        return NULL;
    }

    if(owns_raw)
        BrResFree(raw_data);

    if(table_type) {
        /*
         * Rebuild the lookup table as it was: the marked type, the samples in
         * their own channels, and deliberately no palette. The PNG only carries
         * the samples, so the rows are copied into the row_bytes the engine
         * would have allocated itself.
         */
        size_t row_bytes = (size_t)x * (size_t)table_comp;

        pixelmap = BrPixelmapAllocate(table_type, x, y, NULL, BR_PMAF_NORMAL);

        if(pixelmap != NULL) {
            if(row_bytes > (size_t)pixelmap->row_bytes)
                row_bytes = (size_t)pixelmap->row_bytes;

            for(int row = 0; row < pixelmap->height; ++row) {
                BrMemCpy((br_uint_8 *)pixelmap->pixels + (row * pixelmap->row_bytes),
                         (const br_uint_8 *)pixels + ((size_t)row * x * (size_t)table_comp), row_bytes);
            }

            if(image->name != NULL)
                pixelmap->identifier = BrResStrDup(pixelmap, image->name);
        }

        BrMemFree(pixels);
        return pixelmap;
    }

    tmp             = BrPixelmapAllocate(BR_PMT_RGBA_8888_ARR, x, y, pixels, BR_PMAF_NORMAL);
    tmp->identifier = (char *)image->name; /* NB: This is safe, the clone below will copy it. */

    pixelmap = BrPixelmapCloneTyped(tmp, state->options->pm_type);

    BrPixelmapFree(tmp);
    BrMemFree(pixels);
    return pixelmap;
}

/*
 * Scan our attributes, taking the lowest index of each one.
 *
 * Is this overkill? Probably.
 */
static br_gltf_load_prim_info *filter_primitive_attributes(br_gltf_load_prim_info *info, const cgltf_primitive *prim)
{
    const cgltf_attribute *attrib_position = NULL;
    const cgltf_attribute *attrib_normal   = NULL;
    const cgltf_attribute *attrib_colour   = NULL;
    const cgltf_attribute *attrib_texcoord = NULL;
    int                    index_position  = INT_MAX;
    int                    index_normal    = INT_MAX;
    int                    index_colour    = INT_MAX;
    int                    index_texcoord  = INT_MAX;
    br_size_t              index_count;
    br_size_t              output_vertex_count;
    br_size_t              output_face_count;

    for(br_size_t i = 0; i < prim->attributes_count; ++i) {
        const cgltf_attribute *attrib = prim->attributes + i;

        switch(prim->attributes[i].type) {
            case cgltf_attribute_type_position:
                if(attrib->index < index_position) {
                    attrib_position = attrib;
                    index_position  = attrib->index;
                }
                break;

            case cgltf_attribute_type_normal:
                if(attrib->index < index_normal) {
                    attrib_normal = attrib;
                    index_normal  = attrib->index;
                }
                break;

            case cgltf_attribute_type_color:
                if(attrib->index < index_colour) {
                    attrib_colour = attrib;
                    index_colour  = attrib->index;
                }
                break;

            case cgltf_attribute_type_texcoord:
                if(attrib->index < index_texcoord) {
                    attrib_texcoord = attrib;
                    index_texcoord  = attrib->index;
                }
                break;

            default:
                continue;
        }
    }

    /*
     * NB: cgltf will ensure that all attributes have the same count and that there's at least one.
     */
    index_count = prim->indices ? prim->indices->count : prim->attributes[0].data->count;

    switch(prim->type) {
        case cgltf_primitive_type_triangles: {
            ASSERT(index_count % 3 == 0 && index_count > 0);

            output_vertex_count = prim->attributes[0].data->count;
            output_face_count   = index_count / 3;
            break;
        }

        case cgltf_primitive_type_triangle_strip:
        case cgltf_primitive_type_triangle_fan:
            ASSERT(index_count > 3);
            output_vertex_count = prim->attributes[0].data->count;
            output_face_count   = index_count - 2;
            break;

        default:
            /*
             * Unreachable: check_primitive_modes() rejects a file carrying any
             * other mode before a model is built.
             */
            ASSERT(BR_FALSE);
            output_vertex_count = 0;
            output_face_count   = 0;
            break;
    }

    *info = (br_gltf_load_prim_info){
        .attrib_position     = attrib_position,
        .attrib_normal       = attrib_normal,
        .attrib_colour       = attrib_colour,
        .attrib_texcoord     = attrib_texcoord,
        .input_index_count   = index_count,
        .input_vertex_count  = prim->attributes[0].data->count,
        .output_vertex_count = output_vertex_count,
        .output_face_count   = output_face_count,
        .indices             = prim->indices,
    };
    return info;
}

static br_model *create_model(const cgltf_mesh *mesh, br_gltf_load_state *state)
{
    br_model               *model;
    br_gltf_load_prim_info *primitive_info;
    br_size_t               num_faces    = 0;
    br_size_t               num_vertices = 0;

    primitive_info = BrResAllocate(state, mesh->primitives_count * sizeof(br_gltf_load_prim_info), BR_MEMORY_SCRATCH);

    /*
     * Precalculate everything to get sizing.
     */
    for(br_size_t i = 0; i < mesh->primitives_count; ++i) {
        const cgltf_primitive        *prim = mesh->primitives + i;
        const br_gltf_load_prim_info *info = filter_primitive_attributes(primitive_info + i, prim);
        num_vertices += info->output_vertex_count;
        num_faces += info->output_face_count;
    }

    if(num_vertices > INT_MAX || num_faces > INT_MAX) {
        BrResFree(primitive_info);
        return NULL;
    }

    if(num_vertices == 0 || num_faces == 0) {
        BrResFree(primitive_info);
        return NULL;
    }

    /*
     * Allocate and fill the model.
     */
    model = BrModelAllocate(mesh->name, (int)num_vertices, (int)num_faces);

    br_vertex *vp = model->vertices;
    br_face   *fp = model->faces;

    br_size_t vertex_base = 0;
    for(br_size_t i = 0; i < mesh->primitives_count; ++i) {
        const cgltf_primitive        *prim = mesh->primitives + i;
        const br_gltf_load_prim_info *info = primitive_info + i;

        /*
         * Catch cases of unsupported types (i.e. anything that's not a triangle).
         */
        if(info->output_vertex_count == 0 || info->output_face_count == 0) {
            continue;
        }

        /*
         * Set face materials if we have them.
         */
        if(prim->brender_material != NULL) {
            br_face *base_face = fp;
            for(br_size_t f = 0; f < info->output_face_count; ++f) {
                base_face[f].material = state->results->materials[prim->brender_material - state->data->brender_materials];
            }
        } else if(prim->material != NULL) {
            br_face *base_face = fp;
            for(br_size_t f = 0; f < info->output_face_count; ++f) {
                base_face[f].material = state->results->materials[cgltf_material_index(state->data, prim->material)];
            }
        }

        /*
         * Copy over the vertex data. This is the same regardless of type.
         */
        ASSERT(info->input_vertex_count == info->output_vertex_count);
        for(br_size_t v = 0; v < info->output_vertex_count; ++v, ++vp) {
            if(info->attrib_position != NULL) {
                cgltf_accessor_read_float(info->attrib_position->data, v, vp->p.v, 3);
            }

            if(info->attrib_normal != NULL) {
                cgltf_accessor_read_float(info->attrib_normal->data, v, vp->n.v, 3);
            }

            if(info->attrib_colour != NULL) {
                br_vector3 vv;
                cgltf_accessor_read_float(info->attrib_colour->data, v, vv.v, 3);
                BrVector3Clamp(&vv, &vv, 0.0f, 1.0f);

                vp->red = (br_uint_8)(vv.v[0] * 255.0f);
                vp->grn = (br_uint_8)(vv.v[1] * 255.0f);
                vp->blu = (br_uint_8)(vv.v[2] * 255.0f);
            }

            if(info->attrib_texcoord != NULL) {
                cgltf_accessor_read_float(info->attrib_texcoord->data, v, vp->map.v, 2);
            }
        }

        switch(prim->type) {

            case cgltf_primitive_type_triangles:

                if(info->indices != NULL) {
                    for(br_size_t v = 0; v < info->input_index_count; v += 3, ++fp) {
                        fp->vertices[0] = vertex_base + cgltf_accessor_read_index(info->indices, v + 0);
                        fp->vertices[1] = vertex_base + cgltf_accessor_read_index(info->indices, v + 1);
                        fp->vertices[2] = vertex_base + cgltf_accessor_read_index(info->indices, v + 2);
                    }
                } else {
                    for(br_size_t v = 0; v < info->input_index_count; v += 3, ++fp) {
                        fp->vertices[0] = vertex_base + v + 0;
                        fp->vertices[1] = vertex_base + v + 1;
                        fp->vertices[2] = vertex_base + v + 2;
                    }
                }
                break;

            case cgltf_primitive_type_triangle_strip:

                if(info->indices != NULL) {
                    for(br_uint_32 v = 0; v < info->input_index_count - 2; ++v, ++fp) {
                        if(v & 1u) {
                            fp->vertices[0] = vertex_base + cgltf_accessor_read_index(info->indices, v + 0);
                            fp->vertices[1] = vertex_base + cgltf_accessor_read_index(info->indices, v + 2);
                            fp->vertices[2] = vertex_base + cgltf_accessor_read_index(info->indices, v + 1);
                        } else {
                            fp->vertices[0] = vertex_base + cgltf_accessor_read_index(info->indices, v + 0);
                            fp->vertices[1] = vertex_base + cgltf_accessor_read_index(info->indices, v + 1);
                            fp->vertices[2] = vertex_base + cgltf_accessor_read_index(info->indices, v + 2);
                        }
                    }
                } else {
                    for(br_uint_32 v = 0; v < info->input_index_count - 2; ++v, ++fp) {
                        if(v & 1u) {
                            fp->vertices[0] = vertex_base + v + 0;
                            fp->vertices[1] = vertex_base + v + 2;
                            fp->vertices[2] = vertex_base + v + 1;
                        } else {
                            fp->vertices[0] = vertex_base + v + 0;
                            fp->vertices[1] = vertex_base + v + 1;
                            fp->vertices[2] = vertex_base + v + 2;
                        }
                    }
                }
                break;

            case cgltf_primitive_type_triangle_fan:

                if(info->indices != NULL) {
                    br_size_t center = vertex_base + cgltf_accessor_read_index(info->indices, 0);
                    for(br_uint_32 v = 1; v < info->input_index_count - 1; ++v, ++fp) {
                        fp->vertices[0] = center;
                        fp->vertices[1] = vertex_base + cgltf_accessor_read_index(info->indices, v + 0);
                        fp->vertices[2] = vertex_base + cgltf_accessor_read_index(info->indices, v + 1);
                    }
                } else {
                    for(br_uint_32 v = 1; v < info->input_index_count - 1; ++v, ++fp) {
                        fp->vertices[0] = vertex_base;
                        fp->vertices[1] = vertex_base + v + 0;
                        fp->vertices[2] = vertex_base + v + 1;
                    }
                }

                break;

            default:
                /*
                 * We don't support anything else.
                 */
                continue;
        }

        vertex_base += info->output_vertex_count;
    }

    /*
     * If any primitive had normals, tell BRender to use them
     * instead of regenerating from smoothing groups.
     */
    for(br_size_t i = 0; i < mesh->primitives_count; ++i) {
        if(primitive_info[i].attrib_normal != NULL) {
            model->flags |= BR_MODF_CUSTOM_NORMALS;
            break;
        }
    }

    return model;
}

static float linear_to_srgb(float f)
{
    return (f <= 0.0031308f) ? f * 12.92f : 1.055f * powf(f, 1.0f / 2.4f) - 0.055f;
}

static void fill_material(br_material *mat, const cgltf_material *material, const cgltf_data *data, br_pixelmap **pixelmaps)
{
    if(material->name != NULL) {
        mat->identifier = BrResStrDup(mat, material->name);
    }

    mat->flags |= BR_MATF_LIGHT;
    mat->mode &= ~BR_MATM_SHADING_MODE_MASK;
    mat->mode |= BR_MATM_SHADING_MODE_PHONG;

    if(material->double_sided)
        mat->flags |= BR_MATF_TWO_SIDED;

    if(material->has_pbr_metallic_roughness) {
        const cgltf_pbr_metallic_roughness *pbr = &material->pbr_metallic_roughness;

        /*
         * Base colour factor -> sRGB vertex colour.
         */
        mat->colour = BR_COLOUR_RGBA((uint8_t)(linear_to_srgb(pbr->base_color_factor[0]) * 255.0f + 0.5f),
                                     (uint8_t)(linear_to_srgb(pbr->base_color_factor[1]) * 255.0f + 0.5f),
                                     (uint8_t)(linear_to_srgb(pbr->base_color_factor[2]) * 255.0f + 0.5f),
                                     (uint8_t)(pbr->base_color_factor[3] * 255.0f + 0.5f));

        mat->opacity = (br_uint_8)(pbr->base_color_factor[3] * 255.0f + 0.5f);

        /*
         * Base colour texture -> colour_map.
         */
        if(pbr->base_color_texture.texture != NULL) {
            const cgltf_texture *texture = pbr->base_color_texture.texture;

            if(texture->image != NULL) {
                cgltf_size image_index = cgltf_image_index(data, texture->image);

                if(image_index < data->images_count && pixelmaps[image_index] != NULL) {
                    mat->colour_map = pixelmaps[image_index];
                }
            }

            if(texture->sampler != NULL) {
                const cgltf_sampler *sampler = texture->sampler;

                switch(sampler->wrap_s) {
                    case cgltf_wrap_mode_repeat:
                        mat->mode = (mat->mode & ~BR_MATM_MAP_WIDTH_LIMIT_MASK) | BR_MATM_MAP_WIDTH_LIMIT_WRAP;
                        break;
                    case cgltf_wrap_mode_clamp_to_edge:
                        mat->mode = (mat->mode & ~BR_MATM_MAP_WIDTH_LIMIT_MASK) | BR_MATM_MAP_WIDTH_LIMIT_CLAMP;
                        break;
                    case cgltf_wrap_mode_mirrored_repeat:
                        mat->mode = (mat->mode & ~BR_MATM_MAP_WIDTH_LIMIT_MASK) | BR_MATM_MAP_WIDTH_LIMIT_MIRROR;
                        break;
                    default:
                        break;
                }

                switch(sampler->wrap_t) {
                    case cgltf_wrap_mode_repeat:
                        mat->mode = (mat->mode & ~BR_MATM_MAP_HEIGHT_LIMIT_MASK) | BR_MATM_MAP_HEIGHT_LIMIT_WRAP;
                        break;
                    case cgltf_wrap_mode_clamp_to_edge:
                        mat->mode = (mat->mode & ~BR_MATM_MAP_HEIGHT_LIMIT_MASK) | BR_MATM_MAP_HEIGHT_LIMIT_CLAMP;
                        break;
                    case cgltf_wrap_mode_mirrored_repeat:
                        mat->mode = (mat->mode & ~BR_MATM_MAP_HEIGHT_LIMIT_MASK) | BR_MATM_MAP_HEIGHT_LIMIT_MIRROR;
                        break;
                    default:
                        break;
                }
            }

            if(pbr->base_color_texture.has_transform) {
                const cgltf_texture_transform *xform = &pbr->base_color_texture.transform;

                /*
                 * glTF's KHR_texture_transform defines (in column-vector convention):
                 *   uv' = T * R * S * uv
                 *
                 * In BRender's row-vector convention this becomes:
                 *   uv' = uv * S * R(-angle) * T
                 */
                BrMatrix23Scale(&mat->map_transform, BrFloatToScalar(xform->scale[0]), BrFloatToScalar(xform->scale[1]));
                BrMatrix23PostRotate(&mat->map_transform, -BrRadianToAngle(xform->rotation));
                BrMatrix23PostTranslate(&mat->map_transform, BrFloatToScalar(xform->offset[0]), BrFloatToScalar(xform->offset[1]));
            }
        }

        /*
         * Roughness -> Blinn-Phong power inversion:
         *   roughness = pow(2 / (power + 2), 0.25)
         *   roughness^4 = 2 / (power + 2)
         *   power = (2 / roughness^4) - 2
         *
         * Metallic factor is used as ks directly — 'tis the best
         * we can do without an actual environment/reflection map.
         */
        if(pbr->roughness_factor < 1.0f) {
            float r4    = pbr->roughness_factor * pbr->roughness_factor * pbr->roughness_factor * pbr->roughness_factor;
            float power = (2.0f / r4) - 2.0f;

            if(power < 1.0f)
                power = 1.0f;

            mat->ks    = BrFloatToScalar(pbr->metallic_factor);
            mat->power = BrFloatToScalar(power);
            mat->kd    = BrFloatToScalar(1.0f - pbr->metallic_factor * 0.5f);
            mat->ka    = BrFloatToScalar(0.1f);
        } else {
            mat->ks    = BrFloatToScalar(0.0f);
            mat->power = BrFloatToScalar(0.0f);
            mat->kd    = BrFloatToScalar(0.8f);
            mat->ka    = BrFloatToScalar(0.2f);
        }
    }

    /*
     * Alpha mode.
     */
    switch(material->alpha_mode) {
        case cgltf_alpha_mode_blend:
        case cgltf_alpha_mode_mask:
            mat->flags |= BR_MATF_BLEND;
            break;
        case cgltf_alpha_mode_opaque:
        default:
            break;
    }
}

static void fill_br_material(br_material *mat, const cgltf_brender_material *br_material, const cgltf_data *data, br_pixelmap **pixelmaps)
{
    if(br_material->identifier != NULL) {
        mat->identifier = BrResStrDup(mat, br_material->identifier);
    }

    mat->colour = BR_COLOUR_RGBA((br_uint_8)(br_material->colour[0] * 255.0f), (br_uint_8)(br_material->colour[1] * 255.0f),
                                 (br_uint_8)(br_material->colour[2] * 255.0f), 255);

    mat->opacity = (br_uint_8)(br_material->opacity * 255.0f);
    mat->ka      = BrFloatToScalar(br_material->ka);
    mat->kd      = BrFloatToScalar(br_material->kd);
    mat->ks      = BrFloatToScalar(br_material->ks);
    mat->power   = BrFloatToScalar(br_material->power);

    mat->flags = (br_uint_32)br_material->flags;
    mat->mode  = (br_uint_32)br_material->mode;

    mat->map_transform.m[0][0] = BrFloatToScalar(br_material->map_transform[0]);
    mat->map_transform.m[0][1] = BrFloatToScalar(br_material->map_transform[1]);
    mat->map_transform.m[1][0] = BrFloatToScalar(br_material->map_transform[2]);
    mat->map_transform.m[1][1] = BrFloatToScalar(br_material->map_transform[3]);
    mat->map_transform.m[2][0] = BrFloatToScalar(br_material->map_transform[4]);
    mat->map_transform.m[2][1] = BrFloatToScalar(br_material->map_transform[5]);

    mat->index_base  = br_material->index_base;
    mat->index_range = br_material->index_range;

    if(br_material->colour_map != NULL) {
        mat->colour_map = pixelmaps[cgltf_image_index(data, br_material->colour_map)];
    }

    if(br_material->screendoor != NULL) {
        mat->screendoor = pixelmaps[cgltf_image_index(data, br_material->screendoor)];
    }

    if(br_material->index_shade != NULL) {
        mat->index_shade = pixelmaps[cgltf_image_index(data, br_material->index_shade)];
    }

    if(br_material->index_blend != NULL) {
        mat->index_blend = pixelmaps[cgltf_image_index(data, br_material->index_blend)];
    }

    if(br_material->index_fog != NULL) {
        mat->index_fog = pixelmaps[cgltf_image_index(data, br_material->index_fog)];
    }

    mat->fog_min = BrFloatToScalar(br_material->fog_min);
    mat->fog_max = BrFloatToScalar(br_material->fog_max);

    mat->fog_colour = BR_COLOUR_RGBA((br_uint_8)(br_material->fog_colour[0] * 255.0f), (br_uint_8)(br_material->fog_colour[1] * 255.0f),
                                     (br_uint_8)(br_material->fog_colour[2] * 255.0f), 255);

    mat->subdivide_tolerance = br_material->subdivide_tolerance;
    mat->depth_bias          = br_material->depth_bias;
}

static void read_actor_matrix(br_actor *a, const cgltf_node *node)
{
    /*
     * FIXME: We can't differentiate between (BR_TRANSFORM_LOOK_UP, BR_TRANSFORM_MATRIX34,
     *  and BR_TRANSFORM_MATRIX34) and (BR_TRANSFORM_QUAT, BR_TRANSFORM_EULER).
     */

    /*
     * If the node has a matrix, use BR_TRANSFORM_MATRIX34;
     */
    if(node->has_matrix) {
        br_matrix4 mat44;
        a->t.type = BR_TRANSFORM_MATRIX34;

        for(int m_j = 0; m_j < 4; ++m_j) {
            for(int m_i = 0; m_i < 4; ++m_i) {
                mat44.m[m_j][m_i] = BrFloatToScalar(node->matrix[(m_j * 4) + m_i]);
            }
        }

        BrMatrix34Copy4(&a->t.t.mat, &mat44);
        return;
    }

    /*
     * If we only have a translation, use BR_TRANSFORM_TRANSLATION.
     */
    if(node->has_translation && !node->has_rotation && !node->has_scale) {
        a->t.type = BR_TRANSFORM_TRANSLATION;

        a->t.t.translate.t.v[0] = BrFloatToScalar(node->translation[0]);
        a->t.t.translate.t.v[1] = BrFloatToScalar(node->translation[1]);
        a->t.t.translate.t.v[2] = BrFloatToScalar(node->translation[2]);
        return;
    }

    /*
     * If we only have a rotation, use BR_TRANSFORM_QUAT.
     */
    if(!node->has_translation && node->has_rotation && !node->has_scale) {
        a->t.type = BR_TRANSFORM_QUAT;

        a->t.t.quat.q.x = BrFloatToScalar(node->rotation[0]);
        a->t.t.quat.q.y = BrFloatToScalar(node->rotation[1]);
        a->t.t.quat.q.z = BrFloatToScalar(node->rotation[2]);
        a->t.t.quat.q.w = BrFloatToScalar(node->rotation[3]);
        return;
    }

    /*
     * If we have a combination, generate a matrix.
     *
     * Note that our exporter will never produce this.
     */
    if(node->has_translation || node->has_rotation || node->has_scale) {
        br_vector3  trn, scale;
        br_matrix34 rot;

        if(node->has_translation) {
            trn.v[0] = BrFloatToScalar(node->translation[0]);
            trn.v[1] = BrFloatToScalar(node->translation[1]);
            trn.v[2] = BrFloatToScalar(node->translation[2]);
        } else {
            trn.v[0] = BR_SCALAR(0);
            trn.v[1] = BR_SCALAR(0);
            trn.v[2] = BR_SCALAR(0);
        }

        if(node->has_rotation) {
            br_quat q;
            q.x = BrFloatToScalar(node->rotation[0]);
            q.y = BrFloatToScalar(node->rotation[1]);
            q.z = BrFloatToScalar(node->rotation[2]);
            q.w = BrFloatToScalar(node->rotation[3]);
            BrQuatToMatrix34(&rot, &q);
        } else {
            BrMatrix34Identity(&rot);
        }

        if(node->has_scale) {
            scale.v[0] = BrFloatToScalar(node->scale[0]);
            scale.v[1] = BrFloatToScalar(node->scale[1]);
            scale.v[2] = BrFloatToScalar(node->scale[2]);
        } else {
            scale.v[0] = BR_SCALAR(1);
            scale.v[1] = BR_SCALAR(1);
            scale.v[2] = BR_SCALAR(1);
        }

        /*
         * 3.5.3 Transformations
         * To compose the local transformation matrix, TRS properties MUST be converted
         * to matrices and postmultiplied in the T * R * S order; first the scale is applied
         * to the vertices, then the rotation, and then the translation.
         */
        a->t.type = BR_TRANSFORM_MATRIX34;
        BrMatrix34Scale(&a->t.t.mat, scale.v[0], scale.v[1], scale.v[2]);
        BrMatrix34Post(&a->t.t.mat, &rot);
        BrMatrix34PostTranslate(&a->t.t.mat, trn.v[0], trn.v[1], trn.v[2]);
        return;
    }

    /*
     * Anything else, BR_TRANSFORM_IDENTITY.
     */
    a->t.type = BR_TRANSFORM_IDENTITY;
}

static void fill_actor(br_actor *actor, const cgltf_node *node, const cgltf_data *data, br_material **materials, br_model **models)
{
    if(node->name != NULL) {
        actor->identifier = BrResStrDup(actor, node->name);
    }

    read_actor_matrix(actor, node);

    /*
     * BR_actors carries the actor state a node has no field for; see
     * cgltf_brender.h. A node without the extension keeps the
     * BR_RSTYLE_DEFAULT that BrActorAllocate() left in the field.
     */
    if(node->has_brender_actor) {
        actor->render_style = (br_uint_8)node->brender_actor.render_style;
    }

    if(node->brender_material != NULL) {
        actor->material = materials[node->brender_material - data->brender_materials];
    }

    if(node->mesh != NULL) {
        actor->model = models[cgltf_mesh_index(data, node->mesh)];
    }
}

static br_actor *create_empty_actor(const cgltf_node *node)
{
    br_uint_8 actor_type = BR_ACTOR_NONE;

    /*
     * FIXME: What to do if multiple conditions are true?
     */
    if(node->brender_light != NULL) {
        /* FIXME: Do I handle regular lights too? */
        actor_type = BR_ACTOR_LIGHT;
    } else if(node->camera != NULL) {
        actor_type = BR_ACTOR_CAMERA;
    } else if(node->mesh != NULL) {
        actor_type = BR_ACTOR_MODEL;
    }

    return BrActorAllocate(actor_type, NULL);
}

static void fill_light(br_light *light_data, const cgltf_brender_light *light)
{
    if(light->identifier != NULL) {
        light_data->identifier = BrResStrDup(light_data, light->identifier);
    }

    switch(light->type) {
        default:
        case cgltf_brender_light_type_direct:
            light_data->type = BR_LIGHT_DIRECT;
            break;
        case cgltf_brender_light_type_point:
            light_data->type = BR_LIGHT_POINT;
            break;
        case cgltf_brender_light_type_spot:
            light_data->type = BR_LIGHT_SPOT;
            break;
        case cgltf_brender_light_type_ambient:
            light_data->type = BR_LIGHT_AMBIENT;
            break;
    }

    if(light->view_space) {
        light_data->type |= BR_LIGHT_VIEW;
    }

    if(light->linear_falloff) {
        light_data->type |= BR_LIGHT_LINEAR_FALLOFF;
    }

    light_data->colour = BR_COLOUR_RGBA((br_uint_8)(light->colour[0] * 255.0f), (br_uint_8)(light->colour[1] * 255.0f),
                                        (br_uint_8)(light->colour[2] * 255.0f), 255);

    light_data->attenuation_c = BrFloatToScalar(light->attenuation_c);
    light_data->attenuation_l = BrFloatToScalar(light->attenuation_l);
    light_data->attenuation_q = BrFloatToScalar(light->attenuation_q);

    light_data->cone_outer = BrFloatToScalar(light->cone_outer);
    light_data->cone_inner = BrFloatToScalar(light->cone_inner);

    light_data->radius_outer = BrFloatToScalar(light->radius_outer);
    light_data->radius_inner = BrFloatToScalar(light->radius_inner);
}

static void fill_camera(br_camera *camera_data, const cgltf_camera *camera)
{
    switch(camera->type) {
        default:
        case cgltf_camera_type_perspective:
            camera_data->type = BR_CAMERA_PERSPECTIVE_FOV;

            if(camera->data.perspective.has_aspect_ratio) {
                camera_data->aspect = BrFloatToScalar(camera->data.perspective.aspect_ratio);
            }

            camera_data->field_of_view = BrRadianToAngle(camera->data.perspective.yfov);
            camera_data->hither_z      = BrFloatToScalar(camera->data.perspective.znear);

            if(camera->data.perspective.has_zfar) {
                camera_data->yon_z = BrFloatToScalar(camera->data.perspective.zfar);
            } else {
                // FIXME: this good enough?
                camera_data->yon_z = BR_ADD(camera_data->hither_z, BR_SCALAR(1.0f));
            }

            break;
        case cgltf_camera_type_orthographic:
            camera_data->type = BR_CAMERA_PARALLEL;

            camera_data->width    = BrFloatToScalar(camera->data.orthographic.xmag);
            camera_data->height   = BrFloatToScalar(camera->data.orthographic.ymag);
            camera_data->hither_z = BrFloatToScalar(camera->data.orthographic.znear);
            camera_data->yon_z    = BrFloatToScalar(camera->data.orthographic.zfar);
            break;
    }
}

/* ------------------------------------------------------------------ */
/* File-level validation.                                             */
/* ------------------------------------------------------------------ */

/*
 * The glTF primitive modes this loader can turn into BRender faces.
 *
 * Anything else - points, lines, line strips and loops - has no face
 * representation at all. The switch in filter_primitive_attributes() used to
 * fall through to a zero-sized primitive for those, and a zero-sized primitive
 * is not an empty mesh: create_model() returns NULL, br_actor::model is left
 * NULL, and the renderer then inherits v1db.default_model down the hierarchy
 * - so the actor drew BRender's two-unit default cube
 * in place of the geometry in the file. That is a
 * substitution, not a refusal, and it is silent: the caller sees a scene that
 * loaded and rendered.
 */
static const char *const primitive_mode_names[] = {
    "POINTS", "LINES", "LINE_LOOP", "LINE_STRIP", "TRIANGLES", "TRIANGLE_STRIP", "TRIANGLE_FAN",
};

static const char *primitive_mode_name(cgltf_primitive_type type)
{
    /* cgltf_primitive_type_points is glTF mode 0, so the enum value is the mode plus one. */
    if(type < cgltf_primitive_type_points || type > cgltf_primitive_type_triangle_fan)
        return "unknown";

    return primitive_mode_names[type - cgltf_primitive_type_points];
}

static br_boolean primitive_mode_supported(cgltf_primitive_type type)
{
    return type == cgltf_primitive_type_triangles || type == cgltf_primitive_type_triangle_strip || type == cgltf_primitive_type_triangle_fan;
}

static br_error check_primitive_modes(const cgltf_data *data)
{
    for(br_size_t m = 0; m < data->meshes_count; ++m) {
        const cgltf_mesh *mesh = data->meshes + m;

        for(br_size_t p = 0; p < mesh->primitives_count; ++p) {
            const cgltf_primitive *prim = mesh->primitives + p;

            if(primitive_mode_supported(prim->type))
                continue;

            if(prim->type == cgltf_primitive_type_invalid) {
                BrLogError("GLTF",
                           "mesh \"%s\" primitive %lu has an unrecognised glTF mode (not 0..6), which this loader cannot build faces "
                           "from; only TRIANGLES, TRIANGLE_STRIP and TRIANGLE_FAN are supported",
                           mesh->name != NULL ? mesh->name : "<unnamed>", (unsigned long)p);
            } else {
                BrLogError("GLTF",
                           "mesh \"%s\" primitive %lu has glTF mode %d (%s), which this loader cannot build faces from; only TRIANGLES, "
                           "TRIANGLE_STRIP and TRIANGLE_FAN are supported",
                           mesh->name != NULL ? mesh->name : "<unnamed>", (unsigned long)p, (int)prim->type - 1, primitive_mode_name(prim->type));
            }

            return BRE_FAIL;
        }
    }

    return BRE_OK;
}

/*
 * The count that sizes a model is the primitive's index count, or its first
 * attribute's vertex count when it has no indices, and create_model() fills the
 * model by stepping that same count: three at a time for TRIANGLES, and one at
 * a time for a strip or a fan. The face count it is sized by has to agree with
 * that step, and glTF requires that it does - a multiple of three for
 * TRIANGLES, at least three for a strip or a fan - but nothing enforces either.
 * A count that disagrees is filled past the end of the face array (four indices
 * size one face and fill two), or sizes it from a face count that underflowed,
 * which create_model() sees as too large and refuses, leaving the actor with no
 * model and the renderer's default cube in its place.
 */
static br_error check_primitive_counts(const cgltf_data *data)
{
    for(br_size_t m = 0; m < data->meshes_count; ++m) {
        const cgltf_mesh *mesh = data->meshes + m;

        for(br_size_t p = 0; p < mesh->primitives_count; ++p) {
            const cgltf_primitive *prim = mesh->primitives + p;
            br_size_t              count;

            /*
             * check_primitive_modes() refuses an unsupported mode, and
             * check_primitive_attributes() refuses a primitive with no
             * attributes.
             */
            if(!primitive_mode_supported(prim->type) || prim->attributes_count == 0)
                continue;

            count = prim->indices != NULL ? prim->indices->count : prim->attributes[0].data->count;

            switch(prim->type) {
                case cgltf_primitive_type_triangles:
                    if(count % 3 != 0) {
                        BrLogError("GLTF",
                                   "mesh \"%s\" primitive %lu has %lu %s, which is not a multiple of three; a TRIANGLES primitive is "
                                   "sized for count/3 faces and filled three at a time, so it is filled with one face more than it was "
                                   "sized for",
                                   mesh->name != NULL ? mesh->name : "<unnamed>", (unsigned long)p, (unsigned long)count,
                                   prim->indices != NULL ? "indices" : "vertices");
                        return BRE_FAIL;
                    }

                    if(count == 0) {
                        BrLogError("GLTF",
                                   "mesh \"%s\" primitive %lu has a count of zero; a TRIANGLES primitive is sized for count/3 faces, so it "
                                   "builds no model at all and the renderer draws its default cube in the actor's place",
                                   mesh->name != NULL ? mesh->name : "<unnamed>", (unsigned long)p);
                        return BRE_FAIL;
                    }
                    break;

                case cgltf_primitive_type_triangle_strip:
                case cgltf_primitive_type_triangle_fan:
                    if(count < 3) {
                        BrLogError("GLTF",
                                   "mesh \"%s\" primitive %lu has a count of %lu, fewer than the three a %s needs; its face count is that "
                                   "count less two, which underflowed",
                                   mesh->name != NULL ? mesh->name : "<unnamed>", (unsigned long)p, (unsigned long)count,
                                   primitive_mode_name(prim->type));
                        return BRE_FAIL;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    return BRE_OK;
}

/*
 * The glTF extensions this loader implements - that is, the ones whose state
 * reaches a br_* structure. cgltf parsing an extension is not the same thing:
 * cgltf resolves KHR_lights_punctual into cgltf_node::light and this loader
 * never reads that field, so a file carrying its lights only there would lose
 * them without a word.
 */
static const char *const supported_extensions[] = {
    "BR_actors",
    "BR_lights",
    "BR_materials",
    "KHR_texture_transform",
};

static br_boolean extension_supported(const char *name)
{
    for(size_t i = 0; i < BR_ASIZE(supported_extensions); ++i) {
        if(BrStrCmp(name, supported_extensions[i]) == 0)
            return BR_TRUE;
    }

    return BR_FALSE;
}

static br_error check_extensions(const cgltf_data *data)
{
    /*
     * extensionsRequired is a promise by the file that it cannot be read
     * without the named extensions. One we cannot honour makes the file
     * unrenderable, which is what the field is for, so it is an error rather
     * than something to render around.
     */
    for(br_size_t i = 0; i < data->extensions_required_count; ++i) {
        if(extension_supported(data->extensions_required[i]))
            continue;

        BrLogError("GLTF",
                   "file requires glTF extension \"%s\", which this loader does not implement; refusing to load rather than rendering "
                   "the file without it",
                   data->extensions_required[i]);
        return BRE_FAIL;
    }

    /*
     * extensionsUsed is only a statement of what the file contains. An optional
     * extension may be ignored - but saying so is the difference between
     * ignoring it and losing it.
     */
    for(br_size_t i = 0; i < data->extensions_used_count; ++i) {
        if(extension_supported(data->extensions_used[i]))
            continue;

        BrLogInfo("GLTF", "ignoring glTF extension \"%s\": the file does not require it and this loader does not implement it",
                  data->extensions_used[i]);
    }

    return BRE_OK;
}

/*
 * The BR_materials root array replaces the material table, and glTF's own
 * primitive.material is then read as an index into it (create_model()). In
 * files this writer produces the two arrays are parallel and of equal length,
 * and cgltf has already checked the reference against glTF's own materials
 * array, so only the BR table's shorter length can put the index out of range.
 * That index would read past results->materials and crash model preparation.
 */
static br_error check_material_references(const cgltf_data *data)
{
    if(data->brender_materials_count == 0)
        return BRE_OK;

    for(br_size_t m = 0; m < data->meshes_count; ++m) {
        const cgltf_mesh *mesh = data->meshes + m;

        for(br_size_t p = 0; p < mesh->primitives_count; ++p) {
            const cgltf_primitive *prim = mesh->primitives + p;
            br_size_t              index;

            if(prim->material == NULL)
                continue;

            index = cgltf_material_index(data, prim->material);

            if(index >= data->brender_materials_count) {
                BrLogError("GLTF",
                           "mesh \"%s\" primitive %lu references material %lu, past the %lu in extensions.BR_materials.materials; "
                           "a file carrying BR_materials reads primitive.material as an index into that array",
                           mesh->name != NULL ? mesh->name : "<unnamed>", (unsigned long)p, (unsigned long)index,
                           (unsigned long)data->brender_materials_count);
                return BRE_FAIL;
            }
        }
    }

    return BRE_OK;
}

static br_error check_actor_styles(const cgltf_data *data)
{
    for(br_size_t i = 0; i < data->nodes_count; ++i) {
        const cgltf_node *node = data->nodes + i;
        cgltf_int         style;

        if(!node->has_brender_actor)
            continue;

        style = node->brender_actor.render_style;

        /*
         * render_style indexes RenderStyleCalls[] with no bounds check
         *, and that table only has entries up to
         * BR_RSTYLE_BOUNDING_FACES - the two
         * antialiased styles have no entry. A file that named one would be a
         * call through a NULL or out-of-range pointer, so the value is checked
         * here rather than trusted.
         */
        if(style < BR_RSTYLE_DEFAULT || style > BR_RSTYLE_BOUNDING_FACES) {
            BrLogError("GLTF",
                       "node \"%s\" has BR_actors.render_style %d, outside the range the renderer dispatches "
                       "(BR_RSTYLE_DEFAULT..BR_RSTYLE_BOUNDING_FACES, %d..%d)",
                       node->name != NULL ? node->name : "<unnamed>", (int)style, (int)BR_RSTYLE_DEFAULT, (int)BR_RSTYLE_BOUNDING_FACES);
            return BRE_FAIL;
        }
    }

    return BRE_OK;
}

br_fmt_results *BR_PUBLIC_ENTRY BrFmtGLTFActorLoadMany(const char *name, const br_gltf_options *options)
{
    cgltf_data         *data;
    br_gltf_load_state *state;
    br_fmt_results     *results;
    const char         *base_path;

    // clang-format off
    cgltf_options opts = {
        .type   = cgltf_file_type_invalid,
        .memory = (cgltf_memory_options){
            .alloc_func = cgltf_alloc_br,
            .free_func  = cgltf_free_br,
        },
        .file = (cgltf_file_options){
            .read    = cgltf_load_brfile,
            .release = cgltf_release_brfile,
        },
    };
    // clang-format on

    /*
     * Use default options if none is supplied
     */
    if(options == NULL) {
        options = &_BrDefaultGLTFOptions;
    }

    state = BrResAllocate(NULL, sizeof(br_gltf_load_state), BR_MEMORY_SCRATCH);
    BrMemSet(state, 0, sizeof(br_gltf_load_state));

    base_path = options->base_path;
    if(base_path == NULL || base_path[0] == '\0') {
        base_path = ".";
    }
    base_path        = BrResSprintf(state, "%s/", base_path);
    state->base_path = base_path;

    opts.memory.user_data = state;

    if(cgltf_parse_file(&opts, name, &data) != cgltf_result_success) {
        BrResFree(state);
        return NULL;
    }

    state->data    = data;
    state->options = options;

    /*
     * The checks below are pure and each refuses the file it names before
     * any state is built, so a file that cannot be loaded faithfully never
     * gets part-way in.
     */
    if(check_extensions(data) != BRE_OK || check_primitive_modes(data) != BRE_OK || check_primitive_counts(data) != BRE_OK ||
       check_material_references(data) != BRE_OK || check_actor_styles(data) != BRE_OK) {
        BrResFree(state);
        return NULL;
    }

    if(cgltf_load_buffers(&opts, data, base_path) != cgltf_result_success) {
        BrResFree(state);
        return NULL;
    }

    /*
     * Allocate things.
     */
    results = BrResAllocate(state, sizeof(br_fmt_results), BR_MEMORY_FMT_RESULTS); /* NB: Detached later.*/
    BrMemSet(results, 0, sizeof(br_fmt_results));

    state->results = results;

    results->nmodels = data->meshes_count;
    results->models  = BrResAllocate(results, results->nmodels * sizeof(br_model *), BR_MEMORY_FMT_RESULTS);

    results->ncameras = data->cameras_count;
    results->cameras  = BrResAllocate(results, results->ncameras * sizeof(br_camera *), BR_MEMORY_FMT_RESULTS);

    /*
     * The light table has one entry per BR_lights entry. The KHR light array
     * is a parallel sidecar the writer emits and this reader never reads, so
     * sizing from it produced a table whose entries no node necessarily
     * filled.
     */
    results->nlights = data->brender_lights_count;
    results->lights  = BrResAllocate(results, results->nlights * sizeof(br_light *), BR_MEMORY_FMT_RESULTS);

    if(data->brender_materials_count > 0) {
        results->nmaterials = data->brender_materials_count;
    } else {
        results->nmaterials = data->materials_count;
    }
    results->materials = BrResAllocate(results, results->nmaterials * sizeof(br_material *), BR_MEMORY_FMT_RESULTS);

    results->npixelmaps = data->images_count;
    results->pixelmaps  = BrResAllocate(results, results->npixelmaps * sizeof(br_pixelmap *), BR_MEMORY_FMT_RESULTS);

    results->nactors = data->scene->nodes_count;
    results->actors  = BrResAllocate(results, results->nactors * sizeof(br_actor *), BR_MEMORY_FMT_RESULTS);

    state->all_actors = BrResAllocate(state, data->nodes_count * sizeof(br_actor *), BR_MEMORY_SCRATCH);

    /*
     * Create the pixelmaps.
     *
     * Post: state->results->pixelmaps is filled.
     */
    for(br_size_t i = 0; i < data->images_count; ++i) {
        results->pixelmaps[i] = load_pixelmap(state, data->images + i);
    }

    /*
     * Create the materials.
     *
     * Post: state->results->materials filled.
     */
    if(data->brender_materials_count > 0) {
        for(br_size_t i = 0; i < data->brender_materials_count; ++i) {
            results->materials[i] = BrMaterialAllocate(NULL);
            fill_br_material(results->materials[i], data->brender_materials + i, data, results->pixelmaps);
        }
    } else {
        for(br_size_t i = 0; i < data->materials_count; ++i) {
            results->materials[i] = BrMaterialAllocate(NULL);
            fill_material(results->materials[i], data->materials + i, data, results->pixelmaps);
        }
    }

    for(br_size_t i = 0; i < data->meshes_count; ++i) {
        results->models[i] = create_model(data->meshes + i, state);
    }

    /*
     * Gather all the actors.
     *
     * Post: state->all_actors is filled, new actors created.
     */
    for(br_size_t i = 0; i < data->nodes_count; ++i) {
        state->all_actors[i] = create_empty_actor(data->nodes + i);
    }

    /*
     * Fill in the camera and light actors from the definition their node
     * references.
     *
     * We have to use the actor list for this as we need the type_data pointer and
     * cgltf_* struct doesn't have a backref to its node.
     *
     * Post: the camera and light actors' own type_data is filled.
     */
    for(br_size_t i = 0; i < data->nodes_count; ++i) {
        const cgltf_node *node = data->nodes + i;
        br_actor         *a    = state->all_actors[i];

        switch(a->type) {
            case BR_ACTOR_CAMERA:
                fill_camera(a->type_data, node->camera);
                break;

            case BR_ACTOR_LIGHT:
                /* FIXME: non-brender-lights? */
                fill_light(a->type_data, node->brender_light);

                /*
                 * Seems dodgy, but this is also what the 3ds importer does.
                 */
                BrLightEnable(a);
                break;
        }
    }

    /*
     * Build the hierarchy (in reverse to maintain order).
     *
     * Post: all actor parent/child relationships are created.
     */
    for(br_size_t i = 0; i < data->nodes_count; ++i) {
        const cgltf_node *node = data->nodes + i;
        for(br_size_t c = 0; c < node->children_count; ++c) {
            BrActorAdd(state->all_actors[i], state->all_actors[cgltf_node_index(data, node->children[node->children_count - c - 1])]);
        }
    }

    /*
     * Fill-in the actors.
     *
     * Pre:  results->materials is set.
     * Post: br_actor::identifier is set, if necessary.
     *       br_actor::t is set.
     */
    for(br_size_t i = 0; i < data->nodes_count; ++i) {
        fill_actor(state->all_actors[i], data->nodes + i, data, results->materials, results->models);
    }

    /*
     * Build the camera table.
     *
     * Post: results->cameras is filled, one br_camera per camera in the file. A
     *       camera no node references has no actor behind it, so it is built
     *       here rather than aliased from one.
     */
    for(br_size_t i = 0; i < results->ncameras; ++i) {
        results->cameras[i] = BrResAllocate(results, sizeof(br_camera), BR_MEMORY_CAMERA);
        BrMemSet(results->cameras[i], 0, sizeof(br_camera));
        fill_camera(results->cameras[i], data->cameras + i);
    }

    /*
     * Build the light table.
     *
     * Post: results->lights is filled, one br_light per BR_lights entry. A
     *       definition no node references has no actor behind it, so it is
     *       built here rather than aliased from one.
     */
    for(br_size_t i = 0; i < results->nlights; ++i) {
        results->lights[i] = BrResAllocate(results, sizeof(br_light), BR_MEMORY_LIGHT);
        BrMemSet(results->lights[i], 0, sizeof(br_light));
        fill_light(results->lights[i], data->brender_lights + i);
    }

    /*
     * Set the root actors.
     *
     * Post: results->actors is filled with the top-level actors.
     */
    for(br_size_t i = 0; i < data->scene->nodes_count; ++i) {
        results->actors[i] = state->all_actors[cgltf_node_index(data, data->scene->nodes[i])];
    }

    /*
     * Load animations, if present.
     *
     * Post: results->animation_sets and animation_instances are filled.
     */
    if(data->animations_count > 0) {
        br_animation_set *set = BrFmtGLTFAnimBuildSet(data, data->nodes_count, results);
        if(set != NULL) {
            br_animation_instance *inst = BrFmtGLTFAnimBuildInstance(set, state->all_actors, data->nodes_count, results);
            results->nanimation_sets    = 1;
            results->animation_sets     = BrResAllocate(results, sizeof(br_animation_set *), BR_MEMORY_FMT_RESULTS);
            results->animation_sets[0]  = set;

            if(inst != NULL) {
                results->nanimation_instances   = 1;
                results->animation_instances    = BrResAllocate(results, sizeof(br_animation_instance *), BR_MEMORY_FMT_RESULTS);
                results->animation_instances[0] = inst;
            }
        }
    }

    BrResRemove(results);
    BrResFree(state);
    return results;
}
