#include <limits.h>
#include <string.h>
#include <brender.h>

#include "brassert.h"

#include "cbase64.h"
#include "brstb.h"
#include "cgltf.h"

typedef struct br_gltf_load_state {
    cgltf_data     *data;
    br_fmt_results *results;

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
    (void)file_options;

    if((*data = BrFileLoad(memory_options->user_data, path, size)) == NULL) {
        return cgltf_result_io_error;
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

    if(strncmp(uri, "data:", 5) != 0)
        return NULL;

    comma = strchr(uri, ',');
    if(!(comma && comma - uri >= 7 && strncmp(comma - 7, ";base64", 7) == 0))
        return NULL;

    in_len = strlen(comma + 1);
    if((out_len = cbase64_calc_decoded_length(comma + 1, in_len)) == 0)
        return NULL;

    data_out = BrResAllocate(res, out_len, BR_MEMORY_APPLICATION);

    cbase64_init_decodestate(&ds);
    cbase64_decode_block(comma + 1, in_len, data_out, &ds);

    *size = out_len;
    return data_out;
}

static br_pixelmap *load_pixelmap(void *res, const cgltf_image *image)
{
    void        *raw_data, *pixels;
    br_pixelmap *pixelmap, *tmp;
    size_t       size;
    int          x, y, c;

    if((raw_data = unbuild_data_url(res, image->uri, &size)) == NULL)
        return NULL;

    if(size > INT_MAX) {
        BrResFree(raw_data);
        return NULL;
    }

    if((pixels = stbi_load_from_memory(raw_data, (int)size, &x, &y, &c, 4)) == NULL) {
        BrResFree(raw_data);
        return NULL;
    }

    BrResFree(raw_data);

    tmp             = BrPixelmapAllocate(BR_PMT_RGBA_8888_ARR, x, y, pixels, BR_PMAF_NORMAL);
    tmp->identifier = (char *)image->name; /* NB: This is safe, the clone below will copy it. */

    pixelmap = BrPixelmapCloneTyped(tmp, BR_PMT_RGBA_8888);

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
             * We don't support anything else.
             */
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

    return model;
}

static void fill_material(br_material *mat, const cgltf_brender_material *br_material, const cgltf_data *data, br_pixelmap **pixelmaps)
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

br_fmt_results *BR_PUBLIC_ENTRY BrFmtGLTFActorLoadMany(const char *name)
{
    cgltf_data         *data;
    br_gltf_load_state *state;
    br_fmt_results     *results;

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

    state = BrResAllocate(NULL, sizeof(br_gltf_load_state), BR_MEMORY_SCRATCH);
    BrMemSet(state, 0, sizeof(br_gltf_load_state));

    opts.memory.user_data = state;

    if(cgltf_parse_file(&opts, name, &data) != cgltf_result_success) {
        BrResFree(state);
        return NULL;
    }

    state->data = data;

    if(cgltf_load_buffers(&opts, data, ".") != cgltf_result_success) {
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
    results->cameras  = BrResAllocate(results, results->ncameras * sizeof(br_actor *), BR_MEMORY_FMT_RESULTS);

    results->nlights = data->lights_count;
    results->lights  = BrResAllocate(results, results->nlights * sizeof(br_light *), BR_MEMORY_FMT_RESULTS);

    results->nmaterials = data->brender_materials_count;
    results->materials  = BrResAllocate(results, results->nmaterials * sizeof(br_material *), BR_MEMORY_FMT_RESULTS);

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
    for(br_size_t i = 0; i < data->brender_materials_count; ++i) {
        results->materials[i] = BrMaterialAllocate(NULL);
        fill_material(results->materials[i], data->brender_materials + i, data, results->pixelmaps);
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
     * Fill in the result camera/light arrays.
     *
     * We have to use the actor list for this as we need the type_data pointer and
     * cgltf_* struct doesn't have a backref to its node.
     *
     * Post: results->cameras is filled.
     *       results->lights is filled.
     */
    for(br_size_t i = 0; i < data->nodes_count; ++i) {
        const cgltf_node *node = data->nodes + i;
        br_actor         *a    = state->all_actors[i];

        switch(a->type) {
            case BR_ACTOR_CAMERA:
                results->cameras[cgltf_camera_index(data, node->camera)] = a->type_data;
                break;

            case BR_ACTOR_LIGHT:
                /* FIXME: non-brender-lights? */
                results->lights[node->brender_light - data->brender_lights] = a->type_data;

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
     * Fill-in the cameras.
     *
     * Post: br_camera::* is filled.
     */
    for(br_size_t i = 0; i < data->cameras_count; ++i) {
        fill_camera(results->cameras[i], data->cameras + i);
    }

    /*
     * Fill-in the lights.
     *
     * Post: br_light::* is filled.
     */
    for(br_size_t i = 0; i < data->brender_lights_count; ++i) {
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

    BrResRemove(results);
    BrResFree(state);
    return results;
}
