/*
 * Support routines for rendering models
 */
#include <string.h>
#include "drv.h"
#include "brassert.h"

void DeviceGLExtractPrimitiveState(const state_stack *state, br_primitive_state_info_gl *info, GLuint tex_white)
{
    const state_primitive *prim = &state->prim;

    // clang-format off
    *info = (br_primitive_state_info_gl){
        .sampler = {
            .filter_min     = GL_NEAREST,
            .filter_mag     = GL_NEAREST,
            .wrap_s         = GL_REPEAT,
            .wrap_t         = GL_REPEAT,
        },
        .is_blended         = BR_FALSE,
        .is_filtered        = BR_FALSE,
        .is_indexed         = BR_FALSE,
        .disable_colour_key = BR_FALSE,
        .write_colour       = BR_TRUE,
    };
    // clang-format on

    if(!(state->valid & MASK_STATE_PRIMITIVE))
        return;

    info->is_blended         = (prim->flags & PRIMF_BLEND) != 0;
    info->disable_colour_key = (prim->flags & PRIMF_COLOUR_KEY) == 0;
    info->write_colour       = (prim->flags & PRIMF_COLOUR_WRITE) != 0;
    info->is_indexed         = prim->colour_map ? (prim->colour_map->fmt->indexed != 0) : 0;

    if(prim->colour_map != NULL) {
        const br_buffer_stored *stored = prim->colour_map;

        info->is_indexed = stored->fmt->indexed;
        info->colour_map = BufferStoredGLGetTexture(stored);

        if(info->is_indexed) {
            info->colour_palette = BufferStoredGLGetCLUTTexture(stored, NULL, tex_white);
        } else {
            info->colour_palette = 0;
        }

        info->is_blended = info->is_blended || stored->fmt->blended != 0;
    } else {
        info->is_indexed     = BR_FALSE;
        info->colour_map     = tex_white;
        info->colour_palette = 0;
    }

    if(prim->filter == BRT_LINEAR && prim->mip_filter == BRT_LINEAR) {
        info->sampler.filter_min = GL_LINEAR_MIPMAP_LINEAR;
        info->sampler.filter_mag = GL_LINEAR;

        info->is_filtered = 1;
    } else if(prim->filter == BRT_LINEAR && prim->mip_filter == BRT_NONE) {
        info->sampler.filter_min = GL_LINEAR;
        info->sampler.filter_mag = GL_LINEAR;

        info->is_filtered = 1;
    } else if(prim->filter == BRT_NONE && prim->mip_filter == BRT_LINEAR) {
        info->sampler.filter_min = GL_NEAREST_MIPMAP_NEAREST;
        info->sampler.filter_mag = GL_NEAREST;

        info->is_filtered = 0;
    } else if(prim->filter == BRT_NONE && prim->mip_filter == BRT_NONE) {
        info->sampler.filter_min = GL_NEAREST;
        info->sampler.filter_mag = GL_NEAREST;

        info->is_filtered = 0;
    } else {
        assert(0);
    }

    if(info->is_indexed) {
        info->sampler.filter_min = GL_NEAREST;
        info->sampler.filter_mag = GL_NEAREST;
    }

    /*
     * Apply wrapping.
     */
    switch(prim->map_width_limit) {
        case BRT_WRAP:
        default:
            info->sampler.wrap_s = GL_REPEAT;
            break;

        case BRT_CLAMP:
            info->sampler.wrap_s = GL_CLAMP_TO_EDGE;
            break;

        case BRT_MIRROR:
            info->sampler.wrap_s = GL_MIRRORED_REPEAT;
            break;
    }

    switch(prim->map_height_limit) {
        case BRT_WRAP:
        default:
            info->sampler.wrap_t = GL_REPEAT;
            break;

        case BRT_CLAMP:
            info->sampler.wrap_t = GL_CLAMP_TO_EDGE;
            break;

        case BRT_MIRROR:
            info->sampler.wrap_t = GL_MIRRORED_REPEAT;
            break;
    }
}

static void apply_blend_mode(state_stack *self)
{
    /* C_result = (C_source * F_Source) + (C_dest * F_dest) */

    /* NB: srcAlpha and dstAlpha are all GL_ONE and GL_ZERO respectively. */
    switch(self->prim.blend_mode) {
        default:
            /* fallthrough */
        case BRT_BLEND_STANDARD:
            /* fallthrough */
        case BRT_BLEND_DIMMED:
            /*
             * 3dfx blending mode = 1
             * Colour = (alpha * src) + ((1 - alpha) * dest)
             * Alpha  = (1     * src) + (0           * dest)
             */
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
            break;

        case BRT_BLEND_SUMMED:
            /*
             * 3fdx blending mode = 4
             * Colour = (alpha * src) + (1 * dest)
             * Alpha  = (1     * src) + (0 * dest)
             */
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
            break;

        case BRT_BLEND_PREMULTIPLIED:
            /*
             * 3dfx qblending mode = 2
             * Colour = (1 * src) + ((1 - alpha) * dest)
             * Alpha  = (1 * src) + (0           * dest)
             */
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
            break;
    }
}

static void apply_depth_properties(state_stack *state, uint32_t states)
{
    br_boolean depth_valid = BR_TRUE; /* Defaulting to BR_TRUE to keep existing behaviour. */
    GLenum     depth_test  = GL_NONE;

    /* Only use the states we want (if valid). */
    states = state->valid & states;

    if(states & MASK_STATE_OUTPUT) {
        depth_valid = state->output.depth != NULL;
    }

    if(states & MASK_STATE_SURFACE) {
        if(state->surface.force_front || state->surface.force_back)
            depth_test = GL_FALSE;
        else
            depth_test = GL_TRUE;
    }

    if(depth_valid == BR_TRUE) {
        if(depth_test == GL_TRUE)
            glEnable(GL_DEPTH_TEST);
        else if(depth_test == GL_FALSE)
            glDisable(GL_DEPTH_TEST);
    }

    if(states & MASK_STATE_PRIMITIVE) {
        GLboolean depthMask;
        GLenum depthFunc;

        if(state->prim.flags & PRIMF_DEPTH_WRITE)
            depthMask = GL_TRUE;
        else
            depthMask = GL_FALSE;

        switch(state->prim.depth_test) {
            case BRT_LESS:
                depthFunc = GL_LESS;
                break;
            case BRT_GREATER:
                depthFunc = GL_GREATER;
                break;
            case BRT_LESS_OR_EQUAL:
                depthFunc = GL_LEQUAL;
                break;
            case BRT_GREATER_OR_EQUAL:
                depthFunc = GL_GEQUAL;
                break;
            case BRT_EQUAL:
                depthFunc = GL_EQUAL;
                break;
            case BRT_NOT_EQUAL:
                depthFunc = GL_NOTEQUAL;
                break;
            case BRT_NEVER:
                depthFunc = GL_NEVER;
                break;
            case BRT_ALWAYS:
                depthFunc = GL_ALWAYS;
                break;
            default:
                depthFunc = GL_LESS;
        }

        if(depth_valid) {
            glDepthFunc(depthFunc);
            glDepthMask(depthMask);
        }
    }
}

static void apply_stored_properties(HVIDEO hVideo, br_renderer *renderer, state_stack *state, uint32_t states, br_boolean *unlit,
                                    shader_data_model *model, GLuint tex_default)
{
    /* Only use the states we want (if valid). */
    states = state->valid & states;

    if(states & MASK_STATE_CULL) {
        /*
         * Apply culling states. These are a bit confusing:
         * BRT_ONE_SIDED - Simple, cull back faces. From BRT_ONE_SIDED.
         *
         * BRT_TWO_SIDED - This means the face is two-sided, not to cull
         *                 both sides. From BR_MATF_TWO_SIDED. In the .3ds file
         *                 format, the "two sided" flag means the material is
         *                 visible from the back, or "not culled". fmt/load3ds.c
         *                 sets BR_MATF_TWO_SIDED if this is set, so assume this is
         *                 the correct behaviour.
         *
         * BRT_NONE      - Confusing, this is set if the material has
         *                 BR_MATF_ALWAYS_VISIBLE, but is overridden if
         *                 BR_MATF_TWO_SIDED is set. Assume it means the same
         *                 as BR_MATF_TWO_SIDED.
         */
        switch(state->cull.type) {
            case BRT_ONE_SIDED:
            default: /* Default BRender policy, so default. */
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                break;

            case BRT_TWO_SIDED:
            case BRT_NONE:
                glDisable(GL_CULL_FACE);
                break;
        }
    }

    *unlit = BR_FALSE;
    if(states & MASK_STATE_SURFACE) {
        if(state->surface.colour_source == BRT_SURFACE) {
            br_uint_32 colour = state->surface.colour;
            float      r      = BR_RED(colour) / 255.0f;
            float      g      = BR_GRN(colour) / 255.0f;
            float      b      = BR_BLU(colour) / 255.0f;
            BrVector4Set(&model->surface_colour, r, g, b, state->surface.opacity);
        } else {
            BrVector4Set(&model->surface_colour, 1.0f, 1.0f, 1.0f, state->surface.opacity);
        }

        model->ka    = state->surface.ka;
        model->ks    = state->surface.ks;
        model->kd    = state->surface.kd;
        model->power = state->surface.power;

        switch(state->surface.mapping_source) {
            case BRT_GEOMETRY_MAP:
            default:
                model->uv_source = 0;
                break;

            case BRT_ENVIRONMENT_LOCAL:
                model->uv_source = 1;
                break;

            case BRT_ENVIRONMENT_INFINITE:
                model->uv_source = 2;
                break;
        }

        BrMatrix4Copy23(&model->map_transform, &state->surface.map_transform);

        *unlit = !state->surface.lighting;
    }

    {
        br_primitive_state_info_gl info;
        DeviceGLExtractPrimitiveState(state, &info, tex_default);

        model->disable_colour_key = info.disable_colour_key;
        model->is_indexed         = info.is_indexed;
        model->is_filtered        = info.is_filtered;

        if(info.write_colour) {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        } else {
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        }

        glBindSampler(0, RendererGLGetSampler(renderer, &info.sampler));

        if(info.is_indexed) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, info.colour_map);
            glUniform1i(hVideo->brenderProgram.uniforms.main_texture, hVideo->brenderProgram.mainTextureBinding);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, info.colour_palette);
            glUniform1i(hVideo->brenderProgram.uniforms.index_texture, hVideo->brenderProgram.indexTextureBinding);
        } else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, info.colour_map);
            glUniform1i(hVideo->brenderProgram.uniforms.main_texture, hVideo->brenderProgram.mainTextureBinding);
        }

        if(info.is_blended) {
            glEnable(GL_BLEND);
            apply_blend_mode(state);
        } else {
            glDisable(GL_BLEND);
        }
    }
}

static void apply_state(br_renderer *renderer)
{
    state_cache              *cache  = &renderer->state.cache;
    br_device_pixelmap       *screen = renderer->pixelmap->screen;
    HVIDEO                    hVideo = &screen->asFront.video;
    br_boolean                unlit;
    shader_data_model         model;

    /* Update the per-model cache (matrices and lights) */
    StateGLUpdateModel(cache, &renderer->state.current->matrix);

#if DEBUG
    { /* Check that sceneBegin() actually did it's shit. */

        /* Program */
        GLint p;
        glGetIntegerv(GL_CURRENT_PROGRAM, &p);
        ASSERT(p == hVideo->brenderProgram.program);

        /* FBO */
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &p);
        ASSERT(p == renderer->state.current->output.colour->asBack.glFbo);

        /* Model UBO */
        glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &p);
        ASSERT(p == hVideo->brenderProgram.uboModel);
    }
#endif

    model.projection         = cache->model.p;
    model.model_view         = cache->model.mv;
    model.mvp                = cache->model.mvp;
    model.normal_matrix      = cache->model.normal;
    model.environment_matrix = cache->model.environment;
    model.eye_m              = cache->model.eye_m;

    /* NB: Flag is never set */
    // int model_lit = self->model->flags & V11MODF_LIT;

    unlit = BR_TRUE;
    apply_stored_properties(hVideo, renderer, renderer->state.current, MASK_STATE_STORED, &unlit, &model, screen->asFront.tex_white);
    apply_depth_properties(renderer->state.current, MASK_STATE_STORED | MASK_STATE_OUTPUT);

    model.unlit = (br_uint_32)unlit;
    BrVector4Set(&model.clear_colour, renderer->pixelmap->asBack.clearColour[0], renderer->pixelmap->asBack.clearColour[1],
                 renderer->pixelmap->asBack.clearColour[2], renderer->pixelmap->asBack.clearColour[3]);

    glBufferData(GL_UNIFORM_BUFFER, sizeof(model), &model, GL_STATIC_DRAW);

}

void RendererGLRenderGroup(br_renderer *self, br_geometry_stored *stored, const gl_groupinfo *groupinfo)
{
    apply_state(self);

    glBindVertexArray(stored->gl_vao);
    glDrawElements(GL_TRIANGLES, groupinfo->count, GL_UNSIGNED_SHORT, groupinfo->offset);

    self->stats.face_group_count++;
    self->stats.triangles_rendered_count += groupinfo->group->nfaces;
    self->stats.triangles_drawn_count += groupinfo->group->nfaces;
    self->stats.vertices_rendered_count += groupinfo->group->nfaces * 3;
}

void RendererGLRenderTri(br_renderer *self, br_uintptr_t offset, const gl_groupinfo *groupinfo)
{
    apply_state(self);

    glBindVertexArray(self->trans.vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->trans.vbo);
    glDrawArrays(GL_TRIANGLES, (GLint)offset, 3);

    self->stats.face_group_count++;
    self->stats.triangles_rendered_count += 1;
    self->stats.triangles_drawn_count += 1;
    self->stats.vertices_rendered_count += 3;
}
