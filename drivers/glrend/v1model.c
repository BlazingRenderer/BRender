/*
 * Support routines for rendering models
 */
#include <string.h>
#include "drv.h"
#include "brassert.h"

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
        if(state->prim.flags & PRIMF_DEPTH_WRITE)
            glDepthMask(GL_TRUE);
        else
            glDepthMask(GL_FALSE);

        GLenum depthFunc;
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
        glDepthFunc(depthFunc);
    }
}

static void apply_stored_properties(HVIDEO hVideo, state_stack *state, uint32_t states, br_boolean *unlit,
                                    shader_data_model *model, GLuint tex_default)
{
    br_boolean blending_on;

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
        glActiveTexture(GL_TEXTURE0);

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

    if(states & MASK_STATE_PRIMITIVE) {
        model->disable_colour_key = !(state->prim.flags & PRIMF_COLOUR_KEY);

        if(state->prim.flags & PRIMF_COLOUR_WRITE)
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        else
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        if(state->prim.colour_map) {
            glBindTexture(GL_TEXTURE_2D, BufferStoredGLGetTexture(state->prim.colour_map));
            glUniform1i(hVideo->brenderProgram.uniforms.main_texture, hVideo->brenderProgram.mainTextureBinding);

            // if(state->prim.colour_map->source->flags & BR_PMF_KEYED_TRANSPARENCY)
            //{
            //	//BrDebugBreak();
            // }
        } else {
            glBindTexture(GL_TEXTURE_2D, tex_default);
            glUniform1i(hVideo->brenderProgram.uniforms.main_texture, hVideo->brenderProgram.mainTextureBinding);
        }

        GLenum  minFilter, magFilter;
        GLfloat maxAnisotropy;
        if(state->prim.filter == BRT_LINEAR && state->prim.mip_filter == BRT_LINEAR) {
            minFilter     = GL_LINEAR_MIPMAP_LINEAR;
            magFilter     = GL_LINEAR;
            maxAnisotropy = hVideo->maxAnisotropy;
        } else if(state->prim.filter == BRT_LINEAR && state->prim.mip_filter == BRT_NONE) {
            minFilter     = GL_LINEAR;
            magFilter     = GL_LINEAR;
            maxAnisotropy = 1.0f;
        } else if(state->prim.filter == BRT_NONE && state->prim.mip_filter == BRT_LINEAR) {
            minFilter     = GL_NEAREST_MIPMAP_NEAREST;
            magFilter     = GL_NEAREST;
            maxAnisotropy = hVideo->maxAnisotropy;
        } else if(state->prim.filter == BRT_NONE && state->prim.mip_filter == BRT_NONE) {
            minFilter     = GL_NEAREST;
            magFilter     = GL_NEAREST;
            maxAnisotropy = 1.0f;
        } else {
            assert(0);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)magFilter);

        if(GLAD_GL_EXT_texture_filter_anisotropic)
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

        // if(state->prim.colour_map)
        //{
        //	if(state->prim.colour_map->source)
        //	{
        //		if(state->prim.colour_map->source->identifier)
        //		{
        //			if(!stricmp("sprkwt02", state->prim.colour_map->source->identifier))
        //			{
        //				int x = 0;
        //			}
        //		}
        //	}
        // }

        blending_on = (state->prim.flags & PRIMF_BLEND) || (state->prim.colour_map != NULL && state->prim.colour_map->blended);
        if(blending_on) {
            glEnable(GL_BLEND);
            apply_blend_mode(state);
        } else {
            glDisable(GL_BLEND);
        }
    }

    apply_depth_properties(state, states);
}

void StoredGLRenderGroup(br_geometry_stored *self, br_renderer *renderer, const gl_groupinfo *groupinfo)
{
    state_cache              *cache  = &renderer->state.cache;
    br_device_pixelmap       *screen = renderer->pixelmap->screen;
    HVIDEO                    hVideo = &screen->asFront.video;
    br_renderer_state_stored *stored = groupinfo->stored;
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

    glBindVertexArray(self->gl_vao);

    /* NB: Flag is never set */
    // int model_lit = self->model->flags & V11MODF_LIT;

    unlit = BR_TRUE;
    if(stored) {
        apply_stored_properties(hVideo, &stored->state, MASK_STATE_PRIMITIVE | MASK_STATE_SURFACE | MASK_STATE_CULL,
                                &unlit, &model, screen->asFront.tex_white);
    } else {
        /* If there's no stored state, apply all states from global. */
        apply_stored_properties(hVideo, renderer->state.current, ~0u, &unlit, &model, screen->asFront.tex_white);
    }

    model.unlit = (br_uint_32)unlit;
    BrVector4Set(&model.clear_colour, renderer->pixelmap->asBack.clearColour[0], renderer->pixelmap->asBack.clearColour[1],
                 renderer->pixelmap->asBack.clearColour[2], renderer->pixelmap->asBack.clearColour[3]);

    glBufferData(GL_UNIFORM_BUFFER, sizeof(model), &model, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, groupinfo->count, GL_UNSIGNED_SHORT, groupinfo->offset);

    renderer->stats.face_group_count++;
    renderer->stats.triangles_rendered_count += groupinfo->group->nfaces;
    renderer->stats.triangles_drawn_count += groupinfo->group->nfaces;
    renderer->stats.vertices_rendered_count += groupinfo->group->nfaces * 3;
}
