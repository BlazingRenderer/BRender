/*
 * Support routines for rendering models
 */
#include <string.h>
#include "drv.h"
#include "brassert.h"

/* Handy for debugging Croc. */
static int is_croc_dome(HGLSTATE_STACK state)
{
    br_buffer_stored *stored;
    br_pixelmap      *pm;

    if(!(state->valid & GLSTATE_MASK_PRIMITIVE))
        return 0;

    if((stored = state->prim.colour_map) == NULL)
        return 0;

    if((pm = stored->source) == NULL)
        return 0;

    if(pm->identifier == NULL)
        return 0;

    return strstr(pm->identifier, "DOMEATLAS") == pm->identifier;
}

static void apply_stored_properties(HVIDEO hVideo, HGLSTATE_STACK state, uint32_t states,
                                    br_boolean *unlit, HGLSTD140_MODEL_DATA hModel)
{
    glDisable(GL_BLEND);

    /* Only use the states we want (if valid). */
    states = state->valid & states;

    if(states & GLSTATE_MASK_CULL) {
        switch(state->cull.type) {
            case BRT_ONE_SIDED:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                break;
            case BRT_TWO_SIDED:
                /* Don't cull, is handled in the shader. */
            case BRT_NONE:
            default:
                glDisable(GL_CULL_FACE);
                break;
        }
    }

    *unlit = BR_FALSE;
    if(states & GLSTATE_MASK_SURFACE) {
        if(state->surface.force_front || state->surface.force_back)
            glDisable(GL_DEPTH_TEST);
        else
            glEnable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);

        if(state->surface.colour_source == BRT_SURFACE) {
            br_uint_32 colour = state->surface.colour;
            float      r      = BR_RED(colour) / 255.0f;
            float      g      = BR_GRN(colour) / 255.0f;
            float      b      = BR_BLU(colour) / 255.0f;
            BrVector4Set(&hModel->surface_colour, r, g, b, state->surface.opacity);
        } else {
            BrVector4Set(&hModel->surface_colour, 1.0f, 1.0f, 1.0f, state->surface.opacity);
        }

        hModel->ka    = state->surface.ka;
        hModel->ks    = state->surface.ks;
        hModel->kd    = state->surface.kd;
        hModel->power = state->surface.power;

        /* ambient_{r,g,b} are multiplied by the colour values. */
        BrVector4Set(&hModel->ambient_colour,
                     state->surface.ambient_r, state->surface.ambient_g,
                     state->surface.ambient_g, state->surface.opacity);

        *unlit = !state->surface.lighting;
    }

    if(states & GLSTATE_MASK_PRIMITIVE) {
        if(state->prim.flags & PRIMF_DEPTH_WRITE)
            glDepthMask(GL_TRUE);
        else
            glDepthMask(GL_FALSE);

        if(state->prim.flags & PRIMF_COLOUR_WRITE)
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        else
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        if(state->prim.flags & PRIMF_BLEND)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);

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
            default:
                depthFunc = GL_LESS;
        }
        glDepthFunc(depthFunc);

        if(state->prim.colour_map) {
            glBindTexture(GL_TEXTURE_2D, state->prim.colour_map->gl_tex);
            glUniform1i(hVideo->brenderProgram.uniforms.main_texture, hVideo->brenderProgram.mainTextureBinding);

            //if(state->prim.colour_map->source->flags & BR_PMF_KEYED_TRANSPARENCY)
            //{
            //	//BrDebugBreak();
            //}
        } else {
            glBindTexture(GL_TEXTURE_2D, hVideo->texture.white);
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

        //if(state->prim.colour_map)
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
        //}

        /* This seems to be the only way to see if something needs blending :/ */
        if(state->prim.index_blend != NULL) {
            glEnable(GL_BLEND);

            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
}

void StoredGLRenderGroup(br_geometry_stored *self, br_renderer *renderer, const gl_groupinfo *groupinfo)
{
    HGLCACHE                 hCache  = &renderer->state.cache;
    HVIDEO                   hVideo  = &self->device->video;
    br_renderer_state_stored *stored = groupinfo->stored;
    br_boolean               unlit;
    GLSTD140_MODEL_DATA      model;

    /* Update the per-model cache (matrices and lights) */
    GLCACHE_UpdateModel(hCache, &renderer->state.current->matrix);

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

    model.projection    = *(br_matrix4_f *)&hCache->model.p;
    model.model_view    = *(br_matrix4_f *)&hCache->model.mv;
    model.mvp           = *(br_matrix4_f *)&hCache->model.mvp;
    model.normal_matrix = *(br_matrix4_f *)&hCache->model.normal;

    glBindVertexArray(self->gl_vao);

    /* NB: Flag is never set */
    // int model_lit = self->model->flags & V11MODF_LIT;

    unlit = BR_TRUE;
    if(stored) {
        apply_stored_properties(hVideo, &stored->state,
                                GLSTATE_MASK_PRIMITIVE | GLSTATE_MASK_SURFACE | GLSTATE_MASK_CULL,
                                &unlit, &model);
    } else {
        /* If there's no stored state, apply all states from global. */
        apply_stored_properties(hVideo, renderer->state.current, ~0u, &unlit, &model);
    }

    model.unlit = (br_uint_32)unlit;
    BrVector4Set(&model.clear_colour,
                 renderer->pixelmap->asBack.clearColour[0],
                 renderer->pixelmap->asBack.clearColour[1],
                 renderer->pixelmap->asBack.clearColour[2],
                 renderer->pixelmap->asBack.clearColour[3]
    );

    glBufferData(GL_UNIFORM_BUFFER, sizeof(model), &model, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, groupinfo->count, GL_UNSIGNED_SHORT, groupinfo->offset);

    renderer->stats.face_group_count++;
    renderer->stats.triangles_rendered_count += groupinfo->group->nfaces;
    renderer->stats.triangles_drawn_count    += groupinfo->group->nfaces;
    renderer->stats.vertices_rendered_count  += groupinfo->group->nfaces * 3;
}
