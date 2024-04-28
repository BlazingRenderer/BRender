#include "drv.h"

void StateGLInit(state_all *state, void *res)
{
    state->res = res;

    StateGLInitMatrix(state);
    StateGLInitClip(state);
    StateGLInitCull(state);
    StateGLInitSurface(state);
    StateGLInitPrimitive(state);
    StateGLInitOutput(state);
    StateGLInitHidden(state);
    StateGLInitLight(state);

    state->current = state->stack;
    StateGLCopy(state->current, &state->default_, ~0u);
}

void StateGLCopy(state_stack *dst, state_stack *src, uint32_t mask)
{
    /* Restrict the copy mask to the valid parts. */
    mask &= src->valid;

    /* Merge the valid mask into the destination. */
    dst->valid |= mask;

    if(mask & MASK_STATE_MATRIX)
        dst->matrix = src->matrix;

    if(mask & MASK_STATE_CLIP) {
        for(int i = 0; i < MAX_STATE_CLIP_PLANES; ++i)
            dst->clip[i] = src->clip[i];
    }

    if(mask & MASK_STATE_CULL)
        dst->cull = src->cull;

    if(mask & MASK_STATE_SURFACE)
        dst->surface = src->surface;

    if(mask & MASK_STATE_PRIMITIVE)
        dst->prim = src->prim;

    if(mask & MASK_STATE_OUTPUT)
        dst->output = src->output;

    if(mask & MASK_STATE_LIGHT)
        for(int i = 0; i < MAX_STATE_LIGHTS; ++i)
            dst->light[i] = src->light[i];
}

br_boolean StateGLPush(state_all *state, uint32_t mask)
{
    if(state->top >= MAX_STATE_STACK)
        return BR_FALSE;

    state_stack *old = state->current;
    ++state->top;
    ++state->current;

    // StateGLCopy(state->current, old, mask);
    *state->current = *old;
    return BR_TRUE;
}

br_boolean StateGLPop(state_all *state, uint32_t mask)
{
    if(state->top <= 0)
        return BR_FALSE;

    state_stack *old = state->current;
    --state->top;
    --state->current;

    // StateGLCopy(&state->current, sp, mask);
    return BR_TRUE;
}

void StateGLDefault(state_all *state, uint32_t mask)
{
    StateGLCopy(state->current, &state->default_, mask);
}

// d3drend:state.c:452 void TemplateActions(struct state_all *state, br_token part, br_int_32 index, br_uint_32 mask)
void StateGLTemplateActions(state_all *state, uint32_t mask)
{
    // if(mask & TM_CLEAR_M2V_HINT)
    //     state->current.matrix.model_to_view_hint = BRT_NONE;
    //
    // if(mask & TM_CLEAR_V2S_HINT)
    //     state->current.matrix.view_to_screen_hint = BRT_NONE;
}

// d3drend:state.c:351 struct br_tv_template * FindStateTemplate(struct br_renderer *self, struct state_all **state,
// br_token part, br_int_32 index)
struct br_tv_template *StateGLGetStateTemplate(state_all *state, br_token part, br_int_32 index)
{
    switch(part) {
        case BRT_MATRIX:
            return state->templates.matrix;

        case BRT_CLIP:
            if(index >= MAX_STATE_CLIP_PLANES)
                return NULL;

            return state->templates.clip[index];

        case BRT_CULL:
            return state->templates.cull;

        case BRT_SURFACE:
            return state->templates.surface;

        case BRT_PRIMITIVE:
            return state->templates.prim;

        case BRT_HIDDEN_SURFACE:
            return state->templates.hidden;
        case BRT_LIGHT:
            if(index >= MAX_STATE_LIGHTS)
                return NULL;

            return state->templates.light[index];

        case BRT_OUTPUT:
            return state->templates.output;

        case BRT_ENABLE:
        case BRT_BOUNDS:
        default:
            break;
    }

    return NULL;
}
