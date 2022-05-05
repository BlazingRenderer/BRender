#ifndef _SSTATE_HPP_
#define _SSTATE_HPP_


#include "drv.hpp"
#include "glstate.hpp"

/*
* Parts of state that are stored
*/
#define MASK_STATE_STORED (	MASK_STATE_CULL|\
						  	MASK_STATE_SURFACE|\
						  	MASK_STATE_PRIMITIVE|\
						  	MASK_STATE_CACHE)

/* Private state of geometry format */
typedef struct br_renderer_state_stored_gl
{
	/* Dispatch table */
	struct br_renderer_state_stored_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Saved state */

	/* Pointer to renderer that this state is asociated with */
	br_renderer_gl *renderer;

	GLSTATE_STACK state;

	//struct state_cache cache;

	//br_timestamp timestamp_copy_cache;
	//br_timestamp timestamp_cache;

} br_renderer_state_stored_gl;


#endif



