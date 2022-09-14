#include "drv.h"
#include "glstate_ip.h"

GLSTATE_API void GLSTATE_Init(HGLSTATE hState, void *resAnchor)
{
	hState->resourceAnchor = resAnchor;

	GLSTATEI_InitMatrix(hState);
	GLSTATEI_InitCull(hState);
	GLSTATEI_InitSurface(hState);
	GLSTATEI_InitPrimitive(hState);
	GLSTATEI_InitOutput(hState);
	GLSTATEI_InitHidden(hState);
	GLSTATEI_InitLight(hState);
	GLSTATEI_InitGlobal(hState);

	hState->current = hState->stack;
	GLSTATE_Copy(hState->current, &hState->default_, ~0u);
}

GLSTATE_API void GLSTATE_Copy(HGLSTATE_STACK hDest, HGLSTATE_STACK hSrc, uint32_t mask)
{
	/* Restrict the copy mask to the valid parts. */
	mask &= hSrc->valid;

	/* Merge the valid mask into the destination. */
	hDest->valid |= mask;

	if(mask & GLSTATE_MASK_MATRIX)
		hDest->matrix = hSrc->matrix;

	if(mask & GLSTATE_MASK_CULL)
		hDest->cull = hSrc->cull;

	if(mask & GLSTATE_MASK_SURFACE)
		hDest->surface = hSrc->surface;

	if(mask & GLSTATE_MASK_PRIMITIVE)
		hDest->prim = hSrc->prim;

	if(mask & GLSTATE_MASK_OUTPUT)
		hDest->output = hSrc->output;

	if(mask & GLSTATE_MASK_LIGHT) for(int i = 0; i < GLSTATE_MAX_LIGHTS; ++i)
		hDest->light[i] = hSrc->light[i];
}

GLSTATE_API br_boolean GLSTATE_Push(HGLSTATE hState, uint32_t mask)
{
	if(hState->top >= GLSTATE_MAX_COUNT)
		return BR_FALSE;

	HGLSTATE_STACK old = hState->current;
	++hState->top;
	++hState->current;

	//GLSTATE_Copy(hState->current, old, mask);
	*hState->current = *old;
	return BR_TRUE;
}

GLSTATE_API br_boolean GLSTATE_Pop(HGLSTATE hState, uint32_t mask)
{
	if(hState->top <= 0)
		return BR_FALSE;

	HGLSTATE_STACK old = hState->current;
	--hState->top;
	--hState->current;

	//GLSTATE_Copy(&hState->current, sp, mask);
	return BR_TRUE;
}

GLSTATE_API void GLSTATE_Default(HGLSTATE hState, uint32_t mask)
{
	GLSTATE_Copy(hState->current, &hState->default_, mask);
}

// d3drend:state.c:452 void TemplateActions(struct state_all *state, br_token part, br_int_32 index, br_uint_32 mask)
GLSTATE_API void GLSTATE_TemplateActions(HGLSTATE hState, uint32_t mask)
{
	//if(mask & TM_CLEAR_M2V_HINT)
	//	hState->current.matrix.model_to_view_hint = BRT_NONE;
	//
	//if(mask & TM_CLEAR_V2S_HINT)
	//	hState->current.matrix.view_to_screen_hint = BRT_NONE;

}

// d3drend:state.c:351 struct br_tv_template * FindStateTemplate(struct br_renderer *self, struct state_all **state, br_token part, br_int_32 index)
GLSTATE_API struct br_tv_template *GLSTATE_GetStateTemplate(HGLSTATE hState, br_token part, br_int_32 index)
{
	switch(part)
	{
		case BRT_MATRIX:
			return hState->templates.matrix;

		case BRT_CULL:
			return hState->templates.cull;

		case BRT_SURFACE:
			return hState->templates.surface;

		case BRT_PRIMITIVE:
			return hState->templates.prim;

		case BRT_ENABLE:
			break;

		case BRT_HIDDEN_SURFACE:
			return hState->templates.hidden;
		case BRT_LIGHT:
			if(index >= GLSTATE_MAX_LIGHTS)
				return NULL;

			return hState->templates.light[index];

		case BRT_BOUNDS:
			break;
		case BRT_CLIP:
			break;
		case BRT_OUTPUT:
			return hState->templates.output;
	    case BRT_GLOBAL:
	        return hState->templates.global;

	}

	return NULL;
}
