#include <string.h>
#include <brender.h>
#include <brddi.h>
#include "formats.h"

#include "cbase64.h"
#include "brstb.h"
#include "cgltf_write.h"

#pragma pack(push, 1)
typedef struct br_gltf_save_sampler_key {
    int filter_min;
    int filter_mag;
    int wrap_s;
    int wrap_t;
} br_gltf_save_sampler_key;
#pragma pack(pop)

static br_hash hash_sampler_key(const void *key)
{
    return BrHash(key, sizeof(br_gltf_save_sampler_key));
}

static int compare_sampler_key(const void *a, const void *b)
{
    return BrMemCmp(a, b, sizeof(br_gltf_save_sampler_key)) == 0;
}

typedef struct br_gltf_save_state {
    cgltf_data *data;
    br_hashmap *actor_map;    /**< br_actor                 -> cgltf_node     */
    br_hashmap *model_map;    /**< br_model                 -> cgltf_mesh     */
    br_hashmap *material_map; /**< br_material              -> cgltf_material */
    br_hashmap *stored_map;   /**< br_material::stored      -> br_material    */
    br_hashmap *pixelmap_map; /**< br_pixelmap              -> cgltf_image    */
    br_hashmap *sampler_map;  /**< br_gltf_save_sampler_key -> cgltf_sampler  */

    br_size_t next_node;
    br_size_t next_camera;
    br_size_t next_light;
    br_size_t next_brender_light;
    br_size_t next_mesh;
    br_size_t next_material;
    br_size_t next_texture;
    br_size_t next_sampler;
    br_size_t next_buffer;
    br_size_t next_image;
} br_gltf_save_state;

static char *build_buffer_uri(const cgltf_buffer *buffer, void *res)
{
    const char          prefix[] = "data:application/octet-stream;base64,";
    unsigned int        len      = cbase64_calc_encoded_length((unsigned int)buffer->size);
    cbase64_encodestate es;
    char               *code_out;
    char               *code_out_end;

    code_out = BrResAllocate(res, sizeof(prefix) + len + 1, BR_MEMORY_APPLICATION);
    memcpy(code_out, prefix, sizeof(prefix));

    code_out_end = BrStpCpy(code_out, prefix);

    cbase64_init_encodestate(&es);
    code_out_end += cbase64_encode_block(buffer->data, buffer->size, code_out_end, &es);
    (void)cbase64_encode_blockend(code_out_end, &es);

    return code_out;
}

static char *build_png_uri(void *data, br_size_t size, void *res)
{
    const char          prefix[] = "data:image/png;base64,";
    unsigned int        len      = cbase64_calc_encoded_length((unsigned int)size);
    cbase64_encodestate es;
    char               *code_out;
    char               *code_out_end;

    code_out = BrResAllocate(res, sizeof(prefix) + len + 1, BR_MEMORY_APPLICATION);
    memcpy(code_out, prefix, sizeof(prefix));

    code_out_end = BrStpCpy(code_out, prefix);

    cbase64_init_encodestate(&es);
    code_out_end += cbase64_encode_block(data, (unsigned int)size, code_out_end, &es);
    (void)cbase64_encode_blockend(code_out_end, &es);

    return code_out;
}

static void *cgltf_alloc_br(void *user, cgltf_size size)
{
    return BrResAllocate(user, size, BR_MEMORY_SCRATCH);
}

static void cgltf_free_br(void *user, void *ptr)
{
    (void)user;
    BrResFree(ptr);
}

static cgltf_result cgltf_write_brfile(void *res, const cgltf_options *options, const char *path, const cgltf_data *data)
{
    cgltf_size expected, actual;
    char      *buffer;
    void      *fp;

    /*
     * We only support GLTF.
     */
    if(options->type != cgltf_file_type_gltf)
        return cgltf_result_invalid_options;

    expected = cgltf_write(options, NULL, 0, data);
    buffer   = BrResAllocate(res, expected, BR_MEMORY_APPLICATION);
    actual   = cgltf_write(options, buffer, expected, data);

    if(expected != actual) {
        BrResFree(buffer);
        return cgltf_result_data_too_short;
    }

    if((fp = BrFileOpenWrite(path, 0)) == NULL) {
        BrResFree(buffer);
        return cgltf_result_file_not_found;
    }

    if(BrFileWrite(buffer, actual - 1, 1, fp) != 1) {
        BrResFree(buffer);
        BrFileClose(fp);
        return cgltf_result_io_error;
    }

    BrResFree(buffer);
    BrFileClose(fp);
    return cgltf_result_success;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
static void build_actor_lookup(br_hashmap *hm, br_actor *root)
{
    BrHashMapInsert(hm, root, NULL);

    for(br_actor *a = root->children; a != NULL; a = a->next) {
        build_actor_lookup(hm, a);
    }
}
#pragma clang diagnostic pop

static int fixup_actor_map(const void *key, void *value, br_hash hash, void *user)
{
    const br_actor     *actor = key;
    br_gltf_save_state *state = user;

    (void)value;
    (void)hash;

    /* Technically modifying the map during iteration, but we're just updating the value so it's fine. */
    BrHashMapInsert(state->actor_map, actor, state->data->nodes + state->next_node++);
    return 0;
}

static int gather_models_and_materials(const void *key, void *value, br_hash hash, void *user)
{
    const br_actor     *actor = key;
    br_gltf_save_state *state = user;

    (void)value;
    (void)hash;

    /*
     * Gather all the scene materials.
     */
    if(actor->material != NULL) {
        BrHashMapInsert(state->material_map, actor->material, NULL);
    }

    switch(actor->type) {
        case BR_ACTOR_CAMERA: {
            ++state->data->cameras_count;
            break;
        }

        case BR_ACTOR_LIGHT: {
            ++state->data->lights_count;
            ++state->data->brender_lights_count;
            break;
        }

        case BR_ACTOR_MODEL: {
            /*
             * Gather the model and per-face materials.
             * FIXME: use the default model if NULL
             */
            if(actor->model != NULL) {
                BrHashMapInsert(state->model_map, actor->model, NULL);

                for(br_size_t f = 0; f < actor->model->nfaces; ++f) {
                    const br_face *fp = actor->model->faces + f;

                    if(fp->material != NULL) {
                        BrHashMapInsert(state->material_map, fp->material, NULL);
                    }
                }
            }

            break;
        }
    }

    return 0;
}

static int count_textures(const void *key, void *value, br_hash hash, void *user)
{
    const br_material  *mat   = key;
    br_gltf_save_state *state = user;

    (void)value;
    (void)hash;

    /*
     * If materials don't have a colour map, don't create a texture for them.
     * Using an empty one is undefined behaviour (e.g. Blender samples (0, 0, 0, 0), meaning
     * the material colour multiplication is completely ignored.
     */
    if(mat->colour_map != NULL)
        ++state->data->textures_count;

    return 0;
}

static br_gltf_save_sampler_key material_to_sampler_key(const br_material *mat)
{
    enum {
        NEAREST                = 9728,
        LINEAR                 = 9729,
        NEAREST_MIPMAP_NEAREST = 9984,
        LINEAR_MIPMAP_NEAREST  = 9985,
        NEAREST_MIPMAP_LINEAR  = 9986,
        LINEAR_MIPMAP_LINEAR   = 9987,

        CLAMP_TO_EDGE   = 33071,
        MIRRORED_REPEAT = 33648,
        REPEAT          = 10497,
    };

    br_gltf_save_sampler_key key = {
        .filter_mag = NEAREST,
        .filter_min = NEAREST,
        .wrap_s     = REPEAT,
        .wrap_t     = REPEAT,
    };

    br_boolean map_filtering = (mat->flags & BR_MATF_MAP_INTERPOLATION) != 0;
    br_boolean mip_filtering = (mat->flags & BR_MATF_MIP_INTERPOLATION) != 0;

    if(map_filtering && mip_filtering) {
        key.filter_min = LINEAR_MIPMAP_LINEAR;
        key.filter_mag = LINEAR;
    } else if(map_filtering && !mip_filtering) {
        key.filter_min = LINEAR;
        key.filter_mag = LINEAR;
    } else if(!map_filtering && mip_filtering) {
        key.filter_min = NEAREST_MIPMAP_NEAREST;
        key.filter_mag = NEAREST;
    } else if(!map_filtering && !mip_filtering) {
        key.filter_min = NEAREST;
        key.filter_mag = NEAREST;
    }

    switch(mat->mode & BR_MATM_MAP_WIDTH_LIMIT_MASK) {
        case BR_MATM_MAP_WIDTH_LIMIT_WRAP:
            key.wrap_s = REPEAT;
            break;

        case BR_MATM_MAP_WIDTH_LIMIT_CLAMP:
            key.wrap_s = CLAMP_TO_EDGE;
            break;

        case BR_MATM_MAP_WIDTH_LIMIT_MIRROR:
            key.wrap_s = MIRRORED_REPEAT;
            break;
    }

    switch(mat->mode & BR_MATM_MAP_HEIGHT_LIMIT_MASK) {
        case BR_MATM_MAP_HEIGHT_LIMIT_WRAP:
            key.wrap_t = REPEAT;
            break;

        case BR_MATM_MAP_HEIGHT_LIMIT_CLAMP:
            key.wrap_t = CLAMP_TO_EDGE;
            break;

        case BR_MATM_MAP_HEIGHT_LIMIT_MIRROR:
            key.wrap_t = MIRRORED_REPEAT;
            break;
    }

    return key;
}

static int gather_samplers(const void *key, void *value, br_hash hash, void *user)
{
    const br_material       *mat         = key;
    br_gltf_save_state      *state       = user;
    br_gltf_save_sampler_key sampler_key = material_to_sampler_key(mat);
    cgltf_sampler           *sampler;

    (void)value;
    (void)hash;

    /*
     * We're hashing manually to find the key because we don't want our local copy
     * to be inserted into the map. Only allocate a new key if we have to.
     */
    sampler = BrHashMapFindByHash(state->sampler_map, hash_sampler_key(&sampler_key));
    if(sampler == NULL) {
        br_gltf_save_sampler_key *new_key = BrResAllocate(state->sampler_map, sizeof(br_gltf_save_sampler_key), BR_MEMORY_SCRATCH);
        *new_key                          = sampler_key;
        BrHashMapInsert(state->sampler_map, new_key, (void *)1); /* FIXME: using a dummy, non-NULL value here. */
    }

    return 0;
}

static int gather_pixelmaps(const void *key, void *value, br_hash hash, void *user)
{
    const br_material  *mat   = key;
    br_gltf_save_state *state = user;

    // clang-format off
    const br_pixelmap *maps[] = {
        mat->colour_map,
        mat->screendoor,
        mat->index_shade,
        mat->index_blend,
        mat->index_fog,
    };
    // clang-format on

    (void)value;
    (void)hash;
    (void)user;

    for(br_size_t i = 0; i < BR_ASIZE(maps); ++i) {
        br_size_t last_size = BrHashMapSize(state->pixelmap_map);

        if(maps[i] != NULL) {
            BrHashMapInsert(state->pixelmap_map, mat->colour_map, NULL);

            if(BrHashMapSize(state->pixelmap_map) != last_size) {
                ++state->data->images_count;
            }
        }
    }

    return 0;
}

static void fill_buf_position(const struct v11model *v11m, const struct v11group *gp, cgltf_buffer *buffer, cgltf_buffer_view *view,
                              cgltf_accessor *accessor, cgltf_attribute *attrib, void *res)
{
    buffer->name = BrResSprintf(res, "group %lu positions", gp - v11m->groups);
    buffer->size = sizeof(br_vector3) * gp->nvertices;
    buffer->data = gp->position;
    buffer->uri  = build_buffer_uri(buffer, res);

    view->name   = buffer->name;
    view->offset = 0;
    view->size   = buffer->size;
    view->buffer = buffer;
    view->type   = cgltf_buffer_view_type_vertices;

    accessor->buffer_view    = view;
    accessor->offset         = 0;
    accessor->component_type = cgltf_component_type_r_32f;
    accessor->count          = gp->nvertices;
    accessor->type           = cgltf_type_vec3;

    /*
     * Recalculate the bounding box, the v11m one is a copy of the whole model's.
     */
    accessor->has_min = 1;
    accessor->min[0]  = INFINITY;
    accessor->min[1]  = INFINITY;
    accessor->min[2]  = INFINITY;

    accessor->has_max = 1;
    accessor->max[0]  = -INFINITY;
    accessor->max[1]  = -INFINITY;
    accessor->max[2]  = -INFINITY;

    for(br_size_t p = 0; p < gp->nvertices; ++p) {
        for(br_size_t i = 0; i < 3; ++i) {
            float f = BrScalarToFloat(gp->position[p].v[i]);

            if(f < accessor->min[i])
                accessor->min[i] = f;

            if(f > accessor->max[i])
                accessor->max[i] = f;
        }
    }

    attrib->name = "POSITION";
    attrib->data = accessor;
}

static void fill_buf_normal(const struct v11model *v11m, const struct v11group *gp, cgltf_buffer *buffer, cgltf_buffer_view *view,
                            cgltf_accessor *accessor, cgltf_attribute *attrib, void *res)
{
    buffer->name = BrResSprintf(res, "group %lu normals", gp - v11m->groups);
    buffer->size = sizeof(br_vector3) * gp->nvertices;
    buffer->data = gp->normal;
    buffer->uri  = build_buffer_uri(buffer, res);

    view->name   = buffer->name;
    view->offset = 0;
    view->size   = buffer->size;
    view->buffer = buffer;
    view->type   = cgltf_buffer_view_type_vertices;

    accessor->buffer_view    = view;
    accessor->offset         = 0;
    accessor->component_type = cgltf_component_type_r_32f;
    accessor->count          = gp->nvertices;
    accessor->type           = cgltf_type_vec3;

    attrib->name = "NORMAL";
    attrib->data = accessor;
}

static void fill_buf_uv(const struct v11model *v11m, const struct v11group *gp, cgltf_buffer *buffer, cgltf_buffer_view *view,
                        cgltf_accessor *accessor, cgltf_attribute *attrib, void *res)
{
    buffer->name = BrResSprintf(res, "group %lu uvs", gp - v11m->groups);
    buffer->size = sizeof(br_vector2) * gp->nvertices;
    buffer->data = gp->map;
    buffer->uri  = build_buffer_uri(buffer, res);

    view->name   = buffer->name;
    view->offset = 0;
    view->size   = buffer->size;
    view->buffer = buffer;
    view->type   = cgltf_buffer_view_type_vertices;

    accessor->buffer_view    = view;
    accessor->offset         = 0;
    accessor->component_type = cgltf_component_type_r_32f;
    accessor->count          = gp->nvertices;
    accessor->type           = cgltf_type_vec2;

    attrib->name = "TEXCOORD_0";
    attrib->data = accessor;
}

static void fill_buf_colour(const struct v11model *v11m, const struct v11group *gp, cgltf_buffer *buffer, cgltf_buffer_view *view,
                            cgltf_accessor *accessor, cgltf_attribute *attrib, void *res)
{
    buffer->name = BrResSprintf(res, "group %lu colours", gp - v11m->groups);
    buffer->size = sizeof(br_vector3_f) * gp->nvertices;
    buffer->data = BrResAllocate(res, buffer->size, BR_MEMORY_APPLICATION);
    for(int i = 0; i < gp->nvertices; ++i) {
        br_vector3_f *cp = (br_vector3_f *)buffer->data + i;
        cp->v[0]         = (float)BR_RED(gp->vertex_colours[i]) / 255.0f;
        cp->v[1]         = (float)BR_GRN(gp->vertex_colours[i]) / 255.0f;
        cp->v[2]         = (float)BR_BLU(gp->vertex_colours[i]) / 255.0f;
    }
    buffer->uri = build_buffer_uri(buffer, res);

    view->name   = buffer->name;
    view->offset = 0;
    view->size   = buffer->size;
    view->buffer = buffer;
    view->type   = cgltf_buffer_view_type_vertices;

    accessor->buffer_view    = view;
    accessor->offset         = 0;
    accessor->component_type = cgltf_component_type_r_32f;
    accessor->count          = gp->nvertices;
    accessor->type           = cgltf_type_vec3;

    attrib->name = "COLOR_0";
    attrib->data = accessor;
}

static void fill_buf_index(const struct v11model *v11m, const struct v11group *gp, cgltf_buffer *buffer, cgltf_buffer_view *view,
                           cgltf_accessor *accessor, cgltf_primitive *prims, void *res)
{
    buffer->name = BrResSprintf(res, "group %lu indices", gp - v11m->groups);
    buffer->size = 3 * gp->nfaces * sizeof(br_uint_16);
    buffer->data = gp->vertex_numbers;
    buffer->uri  = build_buffer_uri(buffer, res);

    view->name   = buffer->name;
    view->offset = 0;
    view->size   = buffer->size;
    view->buffer = buffer;
    view->type   = cgltf_buffer_view_type_indices;

    accessor->buffer_view    = view;
    accessor->offset         = 0;
    accessor->component_type = cgltf_component_type_r_16u;
    accessor->count          = gp->nfaces * 3;
    accessor->type           = cgltf_type_scalar;

    prims->type    = cgltf_primitive_type_triangles;
    prims->indices = accessor;
}

typedef void(attribute_cbfn)(const struct v11model *v11m, const struct v11group *gp, cgltf_buffer *buffer, cgltf_buffer_view *view,
                             cgltf_accessor *accessor, cgltf_attribute *attribute, void *res);

// clang-format off
static attribute_cbfn *const attribute_handlers[] = {
    fill_buf_position,
    fill_buf_normal,
    fill_buf_uv,
    fill_buf_colour,
};
// clang-format on

static int count_model_buffers(const void *key, void *value, br_hash hash, void *user)
{
    const br_model        *model    = key;
    br_gltf_save_state    *state    = user;
    const struct v11model *v11m     = model->prepared;
    br_size_t              nbuffers = BR_ASIZE(attribute_handlers) + 1; /* attribs + index */

    (void)value;
    (void)hash;

    state->data->buffers_count += nbuffers * v11m->ngroups;
    state->data->buffer_views_count += nbuffers * v11m->ngroups;
    state->data->accessors_count += nbuffers * v11m->ngroups;

    return 0;
}

static int add_reverse_material_entry(const void *key, void *value, br_hash hash, void *user)
{
    const br_material  *mat   = key;
    br_gltf_save_state *state = user;

    (void)value;
    (void)hash;

    BrHashMapInsert(state->stored_map, mat->stored, (void *)mat);
    return 0;
}

static int fixup_model_map(const void *key, void *value, br_hash hash, void *user)
{
    const br_model     *model = key;
    br_gltf_save_state *state = user;

    (void)value;
    (void)hash;

    /* Technically modifying the map during iteration, but we're just updating the value so it's fine. */
    BrHashMapInsert(state->model_map, model, state->data->meshes + state->next_mesh++);
    return 0;
}

static int fixup_material_map(const void *key, void *value, br_hash hash, void *user)
{
    const br_material  *mat   = key;
    br_gltf_save_state *state = user;

    (void)value;
    (void)hash;

    /* Technically modifying the map during iteration, but we're just updating the value so it's fine. */
    BrHashMapInsert(state->material_map, mat, state->data->materials + state->next_material++);
    return 0;
}

static int fixup_sampler_map(const void *key, void *value, br_hash hash, void *user)
{
    br_gltf_save_state *state = user;

    (void)value;
    (void)hash;

    /* Technically modifying the map during iteration, but we're just updating the value so it's fine. */
    BrHashMapInsert(state->sampler_map, key, state->data->samplers + state->next_sampler++);
    return 0;
}

static int fill_sampler(const void *key, void *value, br_hash hash, void *user)
{
    const br_gltf_save_sampler_key *sampler_key = key;
    cgltf_sampler                  *sampler     = value;

    (void)hash;
    (void)user;

    sampler->min_filter = sampler_key->filter_min;
    sampler->mag_filter = sampler_key->filter_mag;
    sampler->wrap_s     = sampler_key->wrap_s;
    sampler->wrap_t     = sampler_key->wrap_t;
    return 0;
}

static int fixup_pixelmap_map(const void *key, void *value, br_hash hash, void *user)
{
    const br_pixelmap  *pm    = key;
    br_gltf_save_state *state = user;

    (void)value;
    (void)hash;

    /* Technically modifying the map during iteration, but we're just updating the value so it's fine. */
    BrHashMapInsert(state->pixelmap_map, pm, state->data->images + state->next_image++);
    return 0;
}

typedef struct br_gltf_save_stbi {
    br_size_t  size;
    br_size_t  cursor;
    br_uint_8 *data;
} br_gltf_save_stbi;

static void write_stbi_count(void *context, void *data, int size)
{
    br_gltf_save_stbi *state = context;
    (void)data;
    state->size += size;
}

static void write_stbi_actual(void *context, void *data, int size)
{
    br_gltf_save_stbi *state = context;
    BrMemCpy(state->data + state->cursor, data, size);
    state->cursor += size;
}

static int fill_pixelmap(const void *key, void *value, br_hash hash, void *user)
{
    const br_pixelmap  *pm    = key;
    cgltf_image        *image = value;
    br_gltf_save_state *state = user;
    br_pixelmap        *pm2;

    (void)hash;

    image->name = pm->identifier;

    /*
     * This code is cursed.
     */
    pm2 = BrPixelmapCloneTyped((br_pixelmap *)pm, BR_PMT_RGBA_8888_ARR);
    if(pm2 != NULL) {
        br_gltf_save_stbi *sstate;

        sstate  = BrResAllocate(state, sizeof(br_gltf_save_stbi), BR_MEMORY_SCRATCH);
        *sstate = (br_gltf_save_stbi){
            .size   = 0,
            .cursor = 0,
            .data   = NULL,
        };

        stbi_write_png_to_func(write_stbi_count, sstate, pm2->width, pm2->height, 4, pm2->pixels, pm2->row_bytes);
        sstate->data = BrResAllocate(state, sstate->size, BR_MEMORY_SCRATCH);
        stbi_write_png_to_func(write_stbi_actual, sstate, pm2->width, pm2->height, 4, pm2->pixels, pm2->row_bytes);

        BrPixelmapFree(pm2);

        image->uri = build_png_uri(sstate->data, sstate->size, state->data);

        BrResFree(sstate);
    } else {
        /*
         * FIXME: what do here?
         */
        image->uri = "data:image/png;base64,";
    }

    return 0;
}

static int build_node_links(const void *key, void *value, br_hash hash, void *user)
{
    const br_actor     *actor = key;
    cgltf_node         *node  = value;
    br_gltf_save_state *state = user;

    if(actor->material != NULL) {
        cgltf_material *material = BrHashMapFind(state->material_map, actor->material);
        br_size_t       index    = material - state->data->materials;
        node->brender_material   = state->data->brender_materials + index;
    }

    (void)hash;

    switch(actor->type) {
        case BR_ACTOR_CAMERA:
            node->camera = state->data->cameras + state->next_camera++;
            break;

        case BR_ACTOR_LIGHT:
            node->light         = state->data->lights + state->next_light++;
            node->brender_light = state->data->brender_lights + state->next_brender_light++;
            break;

        case BR_ACTOR_MODEL:
            /*
             * FIXME: use the default model if NULL
             */
            if(actor->model != NULL) {
                node->mesh = BrHashMapFind(state->model_map, actor->model);
            }
            break;
    }

    return 0;
}

int fill_hierarchy(const void *key, void *value, br_hash hash, void *user)
{
    const br_actor     *actor        = key;
    cgltf_node         *node         = value;
    br_gltf_save_state *state        = user;
    size_t              num_children = 0;

    (void)hash;

    for(br_actor *a = actor->children; a != NULL; a = a->next) {
        ++num_children;
    }

    if(actor->parent != NULL) {
        node->parent = BrHashMapFind(state->actor_map, actor->parent);
    }

    if(num_children > 0) {
        size_t i;

        node->children_count = num_children;
        node->children       = BrResAllocate(state, sizeof(cgltf_node *) * num_children, BR_MEMORY_SCRATCH);

        i = 0;
        for(br_actor *a = actor->children; a != NULL; a = a->next) {
            node->children[i] = BrHashMapFind(state->actor_map, a);
            ++i;
        }
    }

    return 0;
}

static void fill_actor_name_actual(const br_actor *root, cgltf_node *node)
{
    node->name = root->identifier;
}

static int fill_actor_name(const void *key, void *value, br_hash hash, void *user)
{
    (void)hash;
    (void)user;

    fill_actor_name_actual(key, value);
    return 0;
}

static void fill_transform_actual(const br_actor *root, cgltf_node *node)
{
    br_matrix4 identity;
    BrMatrix4Identity(&identity);

    switch(root->t.type) {
        case BR_TRANSFORM_LOOK_UP:
        case BR_TRANSFORM_MATRIX34:
        case BR_TRANSFORM_MATRIX34_LP: {
            br_matrix34 mat34;
            br_matrix4  mat44;
            BrTransformToMatrix34(&mat34, &root->t);
            BrMatrix4Copy34(&mat44, &root->t.t.mat);

            /*
             * Only set the matrix if it's non-identity. The validator complains otherwise.
             */
            if(BrMemCmp(&mat44, &identity, sizeof(br_matrix4)) != 0) {
                node->has_matrix = true;

                for(int j = 0; j < 4; ++j) {
                    for(int i = 0; i < 4; ++i) {
                        node->matrix[(j * 4) + i] = BrScalarToFloat(mat44.m[j][i]);
                    }
                }
            }
            break;
        }

        case BR_TRANSFORM_QUAT: {
            br_quat q          = root->t.t.quat.q;
            node->rotation[0]  = BrScalarToFloat(q.x);
            node->rotation[1]  = BrScalarToFloat(q.y);
            node->rotation[2]  = BrScalarToFloat(q.z);
            node->rotation[3]  = BrScalarToFloat(q.w);
            node->has_rotation = true;
            break;
        }

        case BR_TRANSFORM_EULER: {
            br_quat q;
            BrEulerToQuat(&q, &root->t.t.euler.e);
            node->rotation[0]  = BrScalarToFloat(q.x);
            node->rotation[1]  = BrScalarToFloat(q.y);
            node->rotation[2]  = BrScalarToFloat(q.z);
            node->rotation[3]  = BrScalarToFloat(q.w);
            node->has_rotation = true;
            break;
        }

        case BR_TRANSFORM_TRANSLATION: {
            node->translation[0]  = BrScalarToFloat(root->t.t.translate.t.v[0]);
            node->translation[1]  = BrScalarToFloat(root->t.t.translate.t.v[1]);
            node->translation[2]  = BrScalarToFloat(root->t.t.translate.t.v[2]);
            node->has_translation = true;
            break;
        }

        case BR_TRANSFORM_IDENTITY:
        default:
            break;
    }
}

static int fill_transform(const void *key, void *value, br_hash hash, void *user)
{
    (void)hash;
    (void)user;

    fill_transform_actual(key, value);
    return 0;
}

#include <float.h>
static float atten_to_range(float c, float l, float q)
{
    const float eta     = 0.01f;
    const float inv_eta = 1.0f / eta;

    if(q == 0.0f) {
        if(l == 0.0f)
            return FLT_MAX;

        float d = (inv_eta - c) / l;
        return d > 0.0f ? d : 0.0f;
    }

    float discriminant = (l * l) - (4.0f * q * (c - inv_eta));
    if(discriminant < 0.0f)
        return 0.0f;

    return (-l * sqrtf(discriminant)) / (2.0f * q);
}

static void fill_br_light(const br_light *light_data, cgltf_brender_light *br_light)
{
    switch(light_data->type & BR_LIGHT_TYPE) {
        case BR_LIGHT_DIRECT:
            br_light->type = cgltf_brender_light_type_direct;
            break;

        case BR_LIGHT_POINT:
            br_light->type = cgltf_brender_light_type_point;
            break;

        case BR_LIGHT_SPOT:
            br_light->type = cgltf_brender_light_type_spot;
            break;

        case BR_LIGHT_AMBIENT:
            br_light->type = cgltf_brender_light_type_ambient;
            break;

        default:
            br_light->type = cgltf_brender_light_type_invalid;
            break;
    }

    br_light->identifier     = light_data->identifier;
    br_light->view_space     = (light_data->type & BR_LIGHT_VIEW) != 0;
    br_light->linear_falloff = (light_data->type & BR_LIGHT_LINEAR_FALLOFF) != 0;
    br_light->colour[0]      = BR_RED(light_data->colour) / 255.0f;
    br_light->colour[1]      = BR_GRN(light_data->colour) / 255.0f;
    br_light->colour[2]      = BR_BLU(light_data->colour) / 255.0f;
    br_light->attenuation_c  = BrScalarToFloat(light_data->attenuation_c);
    br_light->attenuation_l  = BrScalarToFloat(light_data->attenuation_l);
    br_light->attenuation_q  = BrScalarToFloat(light_data->attenuation_q);
    br_light->cone_inner     = BrAngleToScalar(light_data->cone_inner);
    br_light->cone_outer     = BrAngleToScalar(light_data->cone_outer);
    br_light->radius_inner   = BrAngleToScalar(light_data->radius_inner);
    br_light->radius_outer   = BrAngleToScalar(light_data->radius_outer);
}

static void fill_camera(const br_camera *camera_data, cgltf_camera *camera)
{
    switch(camera_data->type) {
        case BR_CAMERA_PARALLEL:
        case BR_CAMERA_PARALLEL_OLD:
            camera->type                    = cgltf_camera_type_orthographic;
            camera->data.orthographic.xmag  = BrScalarToFloat(camera_data->width);
            camera->data.orthographic.ymag  = BrScalarToFloat(camera_data->height);
            camera->data.orthographic.znear = BrScalarToFloat(camera_data->hither_z);
            camera->data.orthographic.zfar  = BrScalarToFloat(camera_data->yon_z);
            break;

        case BR_CAMERA_PERSPECTIVE_WHD:
            /* Unknown */
            break;

        case BR_CAMERA_PERSPECTIVE_FOV:
        case BR_CAMERA_PERSPECTIVE_FOV_OLD:
            camera->type                              = cgltf_camera_type_perspective;
            camera->data.perspective.aspect_ratio     = BrScalarToFloat(camera_data->aspect);
            camera->data.perspective.has_aspect_ratio = BR_TRUE;
            camera->data.perspective.yfov             = BrAngleToRadian(camera_data->field_of_view);
            camera->data.perspective.znear            = BrScalarToFloat(camera_data->hither_z);
            camera->data.perspective.zfar             = BrScalarToFloat(camera_data->yon_z);
            camera->data.perspective.has_zfar         = BR_TRUE;
            break;
    }
}

static void fill_light(const br_light *light_data, cgltf_light *light)
{
    light->color[0]  = BR_RED(light_data->colour) / 255.0f;
    light->color[1]  = BR_GRN(light_data->colour) / 255.0f;
    light->color[2]  = BR_BLU(light_data->colour) / 255.0f;
    light->intensity = 1.0f / BrScalarToFloat(light_data->attenuation_c);

    switch(light_data->type & BR_LIGHT_TYPE) {
        case BR_LIGHT_DIRECT:
            light->type = cgltf_light_type_directional;
            break;

        case BR_LIGHT_POINT:
            light->type = cgltf_light_type_point;

            /*
             * FIXME: This doesn't match BRender's lighting model. At all.
             */
            if(light_data->type & BR_LIGHT_LINEAR_FALLOFF) {
                light->range = light_data->radius_outer;
            } else {
                light->range = atten_to_range(BrScalarToFloat(light_data->attenuation_c), BrScalarToFloat(light_data->attenuation_l),
                                              BrScalarToFloat(light_data->attenuation_q));
            }

            break;

        case BR_LIGHT_SPOT:
            light->type                  = cgltf_light_type_spot;
            light->spot_inner_cone_angle = BrAngleToRadian(BrAngleToScalar(light_data->cone_inner));
            light->spot_outer_cone_angle = BrAngleToRadian(BrAngleToScalar(light_data->cone_outer));

            /*
             * FIXME: This doesn't match BRender's lighting model. At all.
             */
            if(light_data->type & BR_LIGHT_LINEAR_FALLOFF) {
                light->range = light_data->radius_outer;
            } else {
                light->range = atten_to_range(BrScalarToFloat(light_data->attenuation_c), BrScalarToFloat(light_data->attenuation_l),
                                              BrScalarToFloat(light_data->attenuation_q));
            }
            break;

        case BR_LIGHT_AMBIENT:
            /*
             * Not supported in GLTF, but we want to keep the indices matching brender_lights,
             * so add a "dummy" light.
             */
            light->type      = cgltf_light_type_point;
            light->intensity = 0.0f;
            light->color[0]  = 1.0f;
            light->color[1]  = 1.0f;
            light->color[2]  = 1.0f;
            light->range     = 1.0f;
            break;
    }
}

static void fill_actor_types_actual(const br_actor *root, cgltf_node *node)
{
    /*
     * NB: This assumes all the fields have already been set.
     */

    switch(root->type) {
        case BR_ACTOR_NONE:
        case BR_ACTOR_MODEL:
            break;

        case BR_ACTOR_LIGHT: {
            const br_light *light_data = root->type_data;

            fill_br_light(light_data, node->brender_light);

            /*
             * KHR_lights_punctual doesn't support ambient lights.
             */
            fill_light(light_data, node->light);
            if((light_data->type & BR_LIGHT_TYPE) == BR_LIGHT_AMBIENT) {
                node->light = NULL;
            }

            break;
        }

        case BR_ACTOR_CAMERA:
            fill_camera(root->type_data, node->camera);
            break;

        case BR_ACTOR_BOUNDS:
        case BR_ACTOR_BOUNDS_CORRECT:
        case BR_ACTOR_CLIP_PLANE:
        case BR_ACTOR_HORIZON_PLANE:
        default:
            break;
    }
}

static int fill_actor_types(const void *key, void *value, br_hash hash, void *user)
{
    (void)hash;
    (void)user;

    fill_actor_types_actual(key, value);
    return 0;
}

static void fill_mesh_actual(const br_model *model, cgltf_mesh *mesh, cgltf_data *data, size_t *offset)
{
    const struct v11model *v11m        = model->prepared;
    size_t                 next_buffer = *offset;

    mesh->name = model->identifier;

    mesh->primitives_count = v11m->ngroups;
    mesh->primitives       = BrResAllocate(data->meshes, mesh->primitives_count * sizeof(cgltf_primitive), BR_MEMORY_SCRATCH);

    for(size_t g = 0; g < v11m->ngroups; ++g) {
        const struct v11group *gp   = v11m->groups + g;
        cgltf_primitive       *prim = mesh->primitives + g;

        prim->attributes_count = BR_ASIZE(attribute_handlers);
        prim->attributes       = BrResAllocate(data->meshes, prim->attributes_count * sizeof(cgltf_attribute), BR_MEMORY_SCRATCH);

        for(int a = 0; a < prim->attributes_count; ++a) {
            attribute_handlers[a](v11m, gp, data->buffers + next_buffer, data->buffer_views + next_buffer, data->accessors + next_buffer,
                                  prim->attributes + a, data);
            ++next_buffer;
        }

        fill_buf_index(v11m, gp, data->buffers + next_buffer, data->buffer_views + next_buffer, data->accessors + next_buffer, prim, data);
        ++next_buffer;
    }

    *offset = next_buffer;
}

static int fill_mesh(const void *key, void *value, br_hash hash, void *user)
{
    br_gltf_save_state *state = user;

    (void)hash;

    fill_mesh_actual(key, value, state->data, &state->next_buffer);
    return 0;
}

static int fill_material(const void *key, void *value, br_hash hash, void *user)
{
    const br_material  *mat      = key;
    cgltf_material     *material = value;
    br_gltf_save_state *state    = user;

    (void)hash;

    material->name = mat->identifier;

    /*
     * This is 100%, completely wrong. Have you tried mapping Blinn-Phong to PBR?
     */
    material->has_pbr_metallic_roughness = true;

    /* TODO: Figure out how to decompose map_transform. */
    // material->pbr_metallic_roughness.base_color_texture.has_transform = true;

    if(mat->colour_map != NULL) {
        cgltf_texture           *texture     = state->data->textures + state->next_texture++;
        br_gltf_save_sampler_key sampler_key = material_to_sampler_key(mat);

        texture->name    = mat->identifier;
        texture->sampler = BrHashMapFind(state->sampler_map, &sampler_key);
        texture->image   = BrHashMapFind(state->pixelmap_map, mat->colour_map);

        material->pbr_metallic_roughness.base_color_texture.texture = texture;
    }

    material->pbr_metallic_roughness.base_color_factor[0] = BR_RED(mat->colour) / 255.0f;
    material->pbr_metallic_roughness.base_color_factor[1] = BR_GRN(mat->colour) / 255.0f;
    material->pbr_metallic_roughness.base_color_factor[2] = BR_BLU(mat->colour) / 255.0f;
    material->pbr_metallic_roughness.base_color_factor[3] = (float)mat->opacity / 255.0f;

    material->pbr_metallic_roughness.roughness_factor = 1.0f;
    material->pbr_metallic_roughness.metallic_factor  = 0.0f;

    if(mat->flags & (BR_MATF_ENVIRONMENT_I | BR_MATF_ENVIRONMENT_L))
        material->pbr_metallic_roughness.metallic_factor = 1.0f;

    material->has_ior = true;
    material->ior.ior = 1.0f;

    /*
     * GLTF really has some limitations.
     */
    if(mat->mode & BR_MATM_BLEND_MODE_MASK) {
        material->alpha_mode = cgltf_alpha_mode_blend;
    } else {
        material->alpha_mode = cgltf_alpha_mode_opaque;
    }
    return 0;
}

static int fill_br_material(const void *key, void *value, br_hash hash, void *user)
{
    const br_material      *mat         = key;
    br_gltf_save_state     *state       = user;
    size_t                  index       = (cgltf_material *)value - state->data->materials;
    cgltf_brender_material *br_material = state->data->brender_materials + index;

    (void)hash;

    /*
     * We have no direct br_material -> cgltf_brender_material mapping, but the indices
     * are the same as the regular ones, so we can calculate it.
     */

    br_material->identifier = mat->identifier;

    br_material->colour[0] = BR_RED(mat->colour) / 255.0f;
    br_material->colour[1] = BR_GRN(mat->colour) / 255.0f;
    br_material->colour[2] = BR_BLU(mat->colour) / 255.0f;

    br_material->opacity = (float)mat->opacity / 255.0f;

    br_material->ka    = BrScalarToFloat(mat->ka);
    br_material->kd    = BrScalarToFloat(mat->kd);
    br_material->ks    = BrScalarToFloat(mat->ks);
    br_material->power = BrScalarToFloat(mat->power);

    br_material->flags = (cgltf_int)mat->flags;
    br_material->mode  = (cgltf_int)mat->mode;

    br_material->map_transform[0] = BrScalarToFloat(mat->map_transform.m[0][0]);
    br_material->map_transform[1] = BrScalarToFloat(mat->map_transform.m[0][1]);
    br_material->map_transform[2] = BrScalarToFloat(mat->map_transform.m[1][0]);
    br_material->map_transform[3] = BrScalarToFloat(mat->map_transform.m[1][1]);
    br_material->map_transform[4] = BrScalarToFloat(mat->map_transform.m[2][0]);
    br_material->map_transform[5] = BrScalarToFloat(mat->map_transform.m[2][1]);

    br_material->index_base  = mat->index_base;
    br_material->index_range = mat->index_range;

    br_material->colour_map  = BrHashMapFind(state->pixelmap_map, mat->colour_map);
    br_material->screendoor  = BrHashMapFind(state->pixelmap_map, mat->screendoor);
    br_material->index_shade = BrHashMapFind(state->pixelmap_map, mat->index_shade);
    br_material->index_blend = BrHashMapFind(state->pixelmap_map, mat->index_blend);
    br_material->index_fog   = BrHashMapFind(state->pixelmap_map, mat->index_fog);

    br_material->fog_min = BrScalarToFloat(mat->fog_min);
    br_material->fog_max = BrScalarToFloat(mat->fog_max);

    br_material->fog_colour[0] = BR_RED(mat->fog_colour) / 255.0f;
    br_material->fog_colour[1] = BR_GRN(mat->fog_colour) / 255.0f;
    br_material->fog_colour[2] = BR_BLU(mat->fog_colour) / 255.0f;

    br_material->subdivide_tolerance = mat->subdivide_tolerance;

    br_material->depth_bias = BrScalarToFloat(mat->depth_bias);
    return 0;
}

static int fill_mesh_materials(const void *key, void *value, br_hash hash, void *user)
{
    const br_model        *model = key;
    const struct v11model *v11m  = model->prepared;
    cgltf_mesh            *mesh  = value;
    br_gltf_save_state    *state = user;

    (void)hash;

    for(size_t g = 0; g < v11m->ngroups; ++g) {
        br_material *mat = BrHashMapFind(state->stored_map, v11m->groups[g].stored);
        if(mat != NULL) {
            cgltf_material *material             = BrHashMapFind(state->material_map, mat);
            mesh->primitives[g].material         = material;
            mesh->primitives[g].brender_material = state->data->brender_materials + (material - state->data->materials);
        }
    }
    return 0;
}

br_error BR_PUBLIC_ENTRY BrFmtGLTFActorSave(const char *name, br_actor *actor)
{
    return BrFmtGLTFActorSaveMany(name, &actor, 1);
}

br_error BR_PUBLIC_ENTRY BrFmtGLTFActorSaveMany(const char *name, br_actor **actors, br_size_t num)
{
    cgltf_data         *data;
    br_gltf_save_state *state;

    // clang-format off
    cgltf_options opts = {
        .type   = cgltf_file_type_gltf,
        .memory = (cgltf_memory_options) {
            .alloc_func = cgltf_alloc_br,
            .free_func  = cgltf_free_br,
        },
    };
    // clang-format on

    state = BrResAllocate(NULL, sizeof(br_gltf_save_state), BR_MEMORY_SCRATCH);
    BrMemSet(state, 0, sizeof(br_gltf_save_state));

    opts.memory.user_data = state;

    data = state->data = BrResAllocate(state, sizeof(cgltf_data), BR_MEMORY_SCRATCH);

    // clang-format off
    data->asset.generator = BrResSprintf(data, "BRender %d.%d.%d (DDI %d.%d.%d)",
        (__BRENDER__     & 0xFF0000) >> 16,
        (__BRENDER__     & 0x00FF00) >>  8,
        (__BRENDER__     & 0x0000FF) >>  0,
        (__BRENDER_DDI__ & 0xFF0000) >> 16,
        (__BRENDER_DDI__ & 0x00FF00) >>  8,
        (__BRENDER_DDI__ & 0x0000FF) >>  0
    );
    // clang-format on

    data->asset.version = "2.0";

    state->data         = data;
    state->actor_map    = BrHashMapAllocate(state, BrHashMapDefaultHash, BrHashMapDefaultCompare);
    state->model_map    = BrHashMapAllocate(state, BrHashMapDefaultHash, BrHashMapDefaultCompare);
    state->material_map = BrHashMapAllocate(state, BrHashMapDefaultHash, BrHashMapDefaultCompare);
    state->stored_map   = BrHashMapAllocate(state, BrHashMapDefaultHash, BrHashMapDefaultCompare);
    state->pixelmap_map = BrHashMapAllocate(state, BrHashMapDefaultHash, BrHashMapDefaultCompare);
    state->sampler_map  = BrHashMapAllocate(state, hash_sampler_key, compare_sampler_key);

    /*
     * Gather the actors.
     *
     * Post: state->actor_map is populated with (<actor>, NULL).
     *
     * NB: This is the only recursion here, please keep it that way.
     */
    for(br_size_t i = 0; i < num; ++i) {
        build_actor_lookup(state->actor_map, actors[i]);
    }

    /*
     * Allocate space for the nodes.
     */
    data->nodes_count = BrHashMapSize(state->actor_map);
    data->nodes       = BrResAllocate(data, data->nodes_count * sizeof(cgltf_node), BR_MEMORY_SCRATCH);

    /*
     * Build the actor -> cgltf_node* lookup table.
     *
     * Post: state->actor_map is populated with (<actor>, <node>)
     */
    BrHashMapEnumerate(state->actor_map, fixup_actor_map, state);

    /*
     * Allocate space for the scenes, and fill in the nodes.
     */
    data->scenes_count = num;
    data->scenes       = BrResAllocate(data, data->scenes_count * sizeof(cgltf_scene), BR_MEMORY_SCRATCH);

    for(br_size_t i = 0; i < num; ++i) {
        cgltf_scene *scene = data->scenes + i;

        scene->name        = actors[i]->identifier;
        scene->nodes_count = 1;
        scene->nodes       = BrResAllocate(data, scene->nodes_count * sizeof(cgltf_node *), BR_MEMORY_SCRATCH);
        scene->nodes[0]    = BrHashMapFind(state->actor_map, actors[i]);
    }

    /*
     * Fill in the node parent<->child relationships.
     */
    BrHashMapEnumerate(state->actor_map, fill_hierarchy, state);

    /*
     * Gather all the referenced models and materials.
     *
     * Post: state->model_map is populated with (<model>, NULL).
     *       state->material_map is populated with (<material>, NULL).
     *       state->data->cameras_count is set.
     *       state->data->lights_count is set.
     *       state->data->brender_lights_count is set.
     */
    BrHashMapEnumerate(state->actor_map, gather_models_and_materials, state);

    /*
     * Using our materials list, count the no. textures.
     *
     * Post: state->date->textures_count is set.
     */
    BrHashMapEnumerate(state->material_map, count_textures, state);

    /*
     * Using our materials list, gather our samplers.
     *
     * Post: state->sampler_map is populated with (<sampler_key>, (void*)1).
     */
    BrHashMapEnumerate(state->material_map, gather_samplers, state);

    /*
     * Using our materials list, gather the referenced pixelmaps.
     *
     * Post: state->pixelmap_map is populated with (<pixelmap>, NULL)
     *       state->data->images_count is updated.
     *       state->data->buffers_count is updated.
     *       state->data->buffer_views_count is updated.
     */
    BrHashMapEnumerate(state->material_map, gather_pixelmaps, state);

    /*
     * Count tne no. buffers/views/accessors required for the meshes.
     *
     * Post: state->data->buffers_count is updated.
     *       state->data->buffer_views_count is updated.
     *       state->data->accessors_count is updated.
     */
    BrHashMapEnumerate(state->model_map, count_model_buffers, state);

    /*
     * Build the br_material::stored -> br_material lookup.
     *
     * Post: state->material_map is populated.
     */
    BrHashMapEnumerate(state->material_map, add_reverse_material_entry, state);

    /*
     * Allocate things.
     */
    data->meshes_count            = BrHashMapSize(state->model_map);
    data->materials_count         = BrHashMapSize(state->material_map);
    data->brender_materials_count = BrHashMapSize(state->material_map);
    data->samplers_count          = BrHashMapSize(state->sampler_map);

    data->cameras           = BrResAllocate(data, data->cameras_count * sizeof(cgltf_camera), BR_MEMORY_SCRATCH);
    data->lights            = BrResAllocate(data, data->lights_count * sizeof(cgltf_light), BR_MEMORY_SCRATCH);
    data->meshes            = BrResAllocate(data, data->meshes_count * sizeof(cgltf_mesh), BR_MEMORY_SCRATCH);
    data->materials         = BrResAllocate(data, data->materials_count * sizeof(cgltf_material), BR_MEMORY_SCRATCH);
    data->textures          = BrResAllocate(data, data->textures_count * sizeof(cgltf_texture), BR_MEMORY_SCRATCH);
    data->samplers          = BrResAllocate(data, data->samplers_count * sizeof(cgltf_sampler), BR_MEMORY_SCRATCH);
    data->buffers           = BrResAllocate(data, data->buffers_count * sizeof(cgltf_buffer), BR_MEMORY_SCRATCH);
    data->buffer_views      = BrResAllocate(data, data->buffer_views_count * sizeof(cgltf_buffer_view), BR_MEMORY_SCRATCH);
    data->accessors         = BrResAllocate(data, data->accessors_count * sizeof(cgltf_accessor), BR_MEMORY_SCRATCH);
    data->images            = BrResAllocate(data, data->images_count * sizeof(cgltf_image), BR_MEMORY_SCRATCH);
    data->brender_lights    = BrResAllocate(data, data->brender_lights_count * sizeof(cgltf_brender_light), BR_MEMORY_SCRATCH);
    data->brender_materials = BrResAllocate(data, data->brender_materials_count * sizeof(cgltf_brender_material), BR_MEMORY_SCRATCH);

    /*
     * Fixup the model map.
     *
     * Post: state->model_map's values are filled.
     */
    BrHashMapEnumerate(state->model_map, fixup_model_map, state);

    /*
     * Fixup the material map.
     *
     * Post: state->material_map's values are filled.
     */
    BrHashMapEnumerate(state->material_map, fixup_material_map, state);

    /*
     * Fixup the sampler map.
     *
     * Post: state->sampler_map's values are filled.
     */
    BrHashMapEnumerate(state->sampler_map, fixup_sampler_map, state);

    /*
     * Fixup the pixelmap map.
     *
     * Post: state->pixelmap_map's values are filled.
     */
    BrHashMapEnumerate(state->pixelmap_map, fixup_pixelmap_map, state);

    /*
     * Fill the pixelmaps.
     *
     * Post: state->data->images are filled.
     */
    BrHashMapEnumerate(state->pixelmap_map, fill_pixelmap, state);

    /*
     * Build node links.
     *
     * Post: cgltf_node::camera is set, if necessary.
     *       cgltf_node::light is set, if necessary.
     *       cgltf_node::brender_light is set, if necessary.
     *       cgltf_node::brender_material is set, if necessary.
     *       cgltf_node::mesh is set, if necessary.
     */
    BrHashMapEnumerate(state->actor_map, build_node_links, state);

    /*
     * Fill the meshes.
     *
     * Post: state->next_buffer moved to end
     *       Mesh primitives/attributes/buffers/buffer_views/accessors filled.
     */
    BrHashMapEnumerate(state->model_map, fill_mesh, state);

    /*
     * Fill the mesh materials.
     *
     * Post: cgltf_mesh::material is set to the face group's material.
     *
     * FIXME: This doesn't handle br_actor::material override if NULL.
     */
    BrHashMapEnumerate(state->model_map, fill_mesh_materials, state);

    /*
     * Fill the samplers.
     *
     * Post: state->data->samplers is filled.
     */
    BrHashMapEnumerate(state->sampler_map, fill_sampler, state);

    /*
     * Fill the materials as best we can.
     *
     * Post: state->data->materials is filled.
     *       state->data->textures for each material are filled.
     */
    BrHashMapEnumerate(state->material_map, fill_material, state);

    /*
     * Fill the BR_materials extension data.
     *
     * Post: state->data->brender_materials is filled.
     */
    BrHashMapEnumerate(state->material_map, fill_br_material, state);

    /*
     * Fill actor transforms.
     */
    BrHashMapEnumerate(state->actor_map, fill_actor_name, state);
    BrHashMapEnumerate(state->actor_map, fill_transform, state);

    /*
     * Fill the type-specific actor stuff.
     *
     * Post: cgltf_node::brender_light is filled.
     *       cgltf_node::light is filled.
     *       cgltf_node::camera is filled.
     */
    BrHashMapEnumerate(state->actor_map, fill_actor_types, state);

    cgltf_result r = cgltf_write_brfile(state, &opts, name, data);
    BrResFree(state);

    if(r != cgltf_result_success)
        return BRE_FAIL;

    return BRE_OK;
}

br_error BR_PUBLIC_ENTRY BrFmtGLTFModelSaveMany(const char *name, br_model **models, br_size_t num)
{
    br_actor **actors;
    br_error   r;

    actors = BrResAllocate(NULL, num * sizeof(br_actor *), BR_MEMORY_SCRATCH);
    if(actors == NULL)
        return BRE_NO_MEMORY;

    for(br_size_t i = 0; i < num; ++i) {
        br_actor *a   = BrActorAllocate(BR_ACTOR_MODEL, NULL);
        a->identifier = models[i]->identifier;
        a->t.type     = BR_TRANSFORM_IDENTITY;
        a->model      = models[i];
    }

    r = BrFmtGLTFActorSaveMany(name, actors, num);

    for(br_size_t i = 0; i < num; ++i) {
        BrActorFree(actors[i]);
    }

    BrResFree(actors);

    return r;
}

br_error BR_PUBLIC_ENTRY BrFmtGLTFModelSave(const char *name, br_model *model)
{
    return BrFmtGLTFModelSaveMany(name, &model, 1);
}
