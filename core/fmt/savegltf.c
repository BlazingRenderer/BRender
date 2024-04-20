#include <string.h>
#include <brender.h>
#include <brddi.h>
#include "formats.h"

#include "cbase64.h"
#include "cgltf_write.h"

enum {
    BRGLTF_BUFFER_POSN   = 0,
    BRGLTF_BUFFER_NORM   = 1,
    BRGLTF_BUFFER_UV     = 2,
    BRGLTF_BUFFER_COLOUR = 3,
    BRGLTF_BUFFER_INDEX  = 4,
    BRGLTF_BUFFER_COUNT  = 5, /* (Position, Normal, UV, Colour, Index) */
};

static char *build_buffer_uri(const cgltf_buffer *buffer, void *res)
{
    const char          prefix[] = "data:application/octet-stream;base64,";
    unsigned int        len      = cbase64_calc_encoded_length((unsigned int)buffer->size);
    cbase64_encodestate es;
    char               *code_out;
    char               *code_out_end;

    code_out = BrResAllocate(res, sizeof(prefix) + len + 1, BR_MEMORY_APPLICATION);
    memcpy(code_out, prefix, sizeof(prefix));

    code_out_end = stpcpy(code_out, prefix);

    cbase64_init_encodestate(&es);
    code_out_end += cbase64_encode_block(buffer->data, buffer->size, code_out_end, &es);
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

br_error BrFmtGLTFSave(const char *name, const br_model *mp, void *res)
{
    const struct v11model *v11m;
    cgltf_mesh            *mesh;
    // clang-format off
    cgltf_options opts = {
        .type   = cgltf_file_type_gltf,
        .memory = (cgltf_memory_options) {
            .alloc_func = cgltf_alloc_br,
            .free_func  = cgltf_free_br,
        },
    };
    // clang-format on

    /*
     * We use the prepared version of the model because all the face
     * groups and things have already been sorted.
     */
    if((v11m = mp->prepared) == NULL)
        return BRE_FAIL;

    res                   = BrResAllocate(res, 0, BR_MEMORY_ANCHOR);
    opts.memory.user_data = res;

    cgltf_data *data      = BrResAllocate(res, sizeof(cgltf_data), BR_MEMORY_APPLICATION);
    data->asset.generator = BrResSprintf(data, "BRender %d.%d.%d", (__BRENDER__ & 0xff0000) >> 16,
                                         (__BRENDER__ & 0x00ff00) >> 8, (__BRENDER__ & 0x0000ff) >> 0);
    data->asset.version   = "2.0";

    /*
     * NB: This is a lot of buffers because we're effectively only dumping the internal state.
     * If you want to optimise this, either DIY or use an GLTF external optimiser.
     */
    data->buffers_count = BRGLTF_BUFFER_COUNT * v11m->ngroups; /* (Position, Normal, UV, Colour, Index) * ngroups */
    data->buffers       = BrResAllocate(data, data->buffers_count * sizeof(cgltf_buffer), BR_MEMORY_APPLICATION);

    data->buffer_views_count = data->buffers_count;
    data->buffer_views = BrResAllocate(data, data->buffer_views_count * sizeof(cgltf_buffer_view), BR_MEMORY_APPLICATION);

    data->accessors_count = data->buffers_count;
    data->accessors       = BrResAllocate(data, data->accessors_count * sizeof(cgltf_accessor), BR_MEMORY_APPLICATION);

    data->meshes_count = 1;
    data->meshes       = BrResAllocate(data, data->meshes_count * sizeof(cgltf_mesh), BR_MEMORY_APPLICATION);

    mesh = data->meshes + 0; /* One mesh, many primitives. */

    mesh->primitives_count = v11m->ngroups; /* (Position, Normal, UV, Colour) */
    mesh->primitives = BrResAllocate(data->meshes, mesh->primitives_count * sizeof(cgltf_primitive), BR_MEMORY_APPLICATION);

    for(unsigned int g = 0; g < v11m->ngroups; ++g) {
        const struct v11group *gp            = v11m->groups + g;
        cgltf_buffer          *base_buffer   = data->buffers + (BRGLTF_BUFFER_COUNT * g);
        cgltf_buffer_view     *base_view     = data->buffer_views + (BRGLTF_BUFFER_COUNT * g);
        cgltf_accessor        *base_accessor = data->accessors + (BRGLTF_BUFFER_COUNT * g);
        cgltf_primitive       *prims         = mesh->primitives + g;

        prims->attributes_count = 4;
        prims->attributes       = BrResAllocate(mesh->primitives, prims->attributes_count * sizeof(cgltf_attribute),
                                                BR_MEMORY_APPLICATION);

        /*
         * Positions
         */
        {
            cgltf_buffer      *buffer   = base_buffer + BRGLTF_BUFFER_POSN;
            cgltf_buffer_view *view     = base_view + BRGLTF_BUFFER_POSN;
            cgltf_accessor    *accessor = base_accessor + BRGLTF_BUFFER_POSN;
            cgltf_attribute   *attrib   = prims->attributes + BRGLTF_BUFFER_POSN;

            buffer->name = BrResSprintf(data->buffers, "group %u positions", g);
            buffer->size = sizeof(br_vector3) * gp->nvertices;
            buffer->data = gp->position;
            buffer->uri  = build_buffer_uri(buffer, data->buffers);

            view->name   = buffer->name;
            view->offset = 0;
            view->size   = buffer->size;
            view->buffer = buffer;

            accessor->buffer_view    = view;
            accessor->offset         = 0;
            accessor->component_type = cgltf_component_type_r_32f;
            accessor->count          = gp->nvertices;
            accessor->type           = cgltf_type_vec3;

            /* FIXME: Calculate the actual bounds instead of using the global ones. */
            accessor->has_min = 1;
            accessor->min[0]  = v11m->bounds.min.v[0];
            accessor->min[1]  = v11m->bounds.min.v[1];
            accessor->min[2]  = v11m->bounds.min.v[2];

            accessor->has_max = 1;
            accessor->max[0]  = v11m->bounds.max.v[0];
            accessor->max[1]  = v11m->bounds.max.v[1];
            accessor->max[2]  = v11m->bounds.max.v[2];

            attrib->name = "POSITION";
            attrib->data = accessor;
        }

        /*
         * Normals
         */
        {
            cgltf_buffer      *buffer   = base_buffer + BRGLTF_BUFFER_NORM;
            cgltf_buffer_view *view     = base_view + BRGLTF_BUFFER_NORM;
            cgltf_accessor    *accessor = base_accessor + BRGLTF_BUFFER_NORM;
            cgltf_attribute   *attrib   = prims->attributes + BRGLTF_BUFFER_NORM;

            buffer->name = BrResSprintf(data->buffers, "group %u normals", g);
            buffer->size = sizeof(br_vector3) * gp->nvertices;
            buffer->data = gp->normal;
            buffer->uri  = build_buffer_uri(buffer, data->buffers);

            view->name   = buffer->name;
            view->offset = 0;
            view->size   = buffer->size;
            view->buffer = buffer;

            accessor->buffer_view    = view;
            accessor->offset         = 0;
            accessor->component_type = cgltf_component_type_r_32f;
            accessor->count          = gp->nvertices;
            accessor->type           = cgltf_type_vec3;

            attrib->name = "NORMAL";
            attrib->data = accessor;
        }

        /*
         * UVs
         */
        {
            cgltf_buffer      *buffer   = base_buffer + BRGLTF_BUFFER_UV;
            cgltf_buffer_view *view     = base_view + BRGLTF_BUFFER_UV;
            cgltf_accessor    *accessor = base_accessor + BRGLTF_BUFFER_UV;
            cgltf_attribute   *attrib   = prims->attributes + BRGLTF_BUFFER_UV;

            buffer->name = BrResSprintf(data->buffers, "group %u uvs", g);
            buffer->size = sizeof(br_vector2) * gp->nvertices;
            buffer->data = gp->map;
            buffer->uri  = build_buffer_uri(buffer, data->buffers);

            view->name   = buffer->name;
            view->offset = 0;
            view->size   = buffer->size;
            view->buffer = buffer;

            accessor->buffer_view    = view;
            accessor->offset         = 0;
            accessor->component_type = cgltf_component_type_r_32f;
            accessor->count          = gp->nvertices;
            accessor->type           = cgltf_type_vec2;

            attrib->name = "TEXCOORD_0";
            attrib->data = accessor;
        }

        /*
         * Colours
         */
        {
            cgltf_buffer      *buffer   = base_buffer + BRGLTF_BUFFER_COLOUR;
            cgltf_buffer_view *view     = base_view + BRGLTF_BUFFER_COLOUR;
            cgltf_accessor    *accessor = base_accessor + BRGLTF_BUFFER_COLOUR;
            cgltf_attribute   *attrib   = prims->attributes + BRGLTF_BUFFER_COLOUR;
            buffer->name                = BrResSprintf(data->buffers, "group %u colours", g);
            buffer->size                = sizeof(br_vector3_f) * gp->nvertices;
            buffer->data                = BrResAllocate(data->buffers, buffer->size, BR_MEMORY_APPLICATION);
            for(int i = 0; i < gp->nvertices; ++i) {
                br_vector3_f *cp = (br_vector3_f *)buffer->data + i;
                cp->v[0]         = (float)BR_RED(gp->vertex_colours[i]) / 255.0f;
                cp->v[1]         = (float)BR_GRN(gp->vertex_colours[i]) / 255.0f;
                cp->v[2]         = (float)BR_BLU(gp->vertex_colours[i]) / 255.0f;
            }
            buffer->uri = build_buffer_uri(buffer, data->buffers);

            view->name   = buffer->name;
            view->offset = 0;
            view->size   = buffer->size;
            view->buffer = buffer;

            accessor->buffer_view    = view;
            accessor->offset         = 0;
            accessor->component_type = cgltf_component_type_r_32f;
            accessor->count          = gp->nvertices;
            accessor->type           = cgltf_type_vec3;

            attrib->name = "COLOR_0";
            attrib->data = accessor;
        }

        /*
         * Indices
         */
        {
            cgltf_buffer      *buffer   = base_buffer + BRGLTF_BUFFER_INDEX;
            cgltf_buffer_view *view     = base_view + BRGLTF_BUFFER_INDEX;
            cgltf_accessor    *accessor = base_accessor + BRGLTF_BUFFER_INDEX;

            buffer->name = BrResSprintf(data->buffers, "group %u indices", g);
            buffer->size = 3 * gp->nfaces * sizeof(br_uint_16);
            buffer->data = gp->vertex_numbers;
            buffer->uri  = build_buffer_uri(buffer, data->buffers);

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
    }

    data->nodes_count = 1;
    data->nodes       = BrResAllocate(data, data->nodes_count * sizeof(cgltf_node), BR_MEMORY_APPLICATION);
    cgltf_node *node  = data->nodes + 0;
    node->mesh        = mesh;

    data->scenes_count = 1;
    data->scenes       = BrResAllocate(data, data->scenes_count * sizeof(cgltf_scene), BR_MEMORY_APPLICATION);
    data->scene        = data->scenes + 0;

    cgltf_scene *scene = data->scenes + 0;

    scene->nodes_count = 1;
    scene->nodes       = &data->nodes;

    cgltf_result r = cgltf_write_brfile(res, &opts, name, data);
    BrResFree(res);

    if(r != cgltf_result_success)
        return BRE_FAIL;

    return BRE_OK;
}
