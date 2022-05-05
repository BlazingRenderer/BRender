#ifndef _SSTATE_HPP_
#define _SSTATE_HPP_

#include "drv.hpp"
#include "glstate.hpp"

/* Private state of geometry format */
typedef struct br_renderer_state_stored_gl
{
	/* Dispatch table */
	const struct br_renderer_state_stored_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Pointer to owning device */
	struct br_device_gl *device;

	/* Saved state */

	/* Pointer to renderer that this state is asociated with */
	br_renderer_gl *renderer;

	GLSTATE_STACK state;

	//struct state_cache cache;

	//br_timestamp timestamp_copy_cache;
	//br_timestamp timestamp_cache;

} br_renderer_state_stored_gl;

template<>
inline constexpr bool is_object_v<br_renderer_state_stored_gl> = true;

#endif



