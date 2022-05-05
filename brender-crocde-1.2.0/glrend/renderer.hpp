#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "drv.hpp"
#include "glstate.hpp"

#include <bitset>
#include <functional>
#include <set>
constexpr size_t GLREND_MAX_STORED_MODEL_GROUPS = 64;

using grflags = std::bitset<GLREND_MAX_STORED_MODEL_GROUPS>;

enum class render_mode
{
	force_back = 0,
	opaque = 1,
	transparent = 2,
	force_front = 3,
	max = 4
};

struct br_geometry_stored_gl;
typedef struct br_renderer_gl_deferred
{
	/* Distance from the view */
	float distance;
	/* State snapshot */
	GLSTATE_STACK snapshot;
	/* Stored geometry */
	br_geometry_stored_gl *stored;
	/* Defer flags. */
	grflags flags[static_cast<int>(render_mode::max)];
} br_renderer_gl_deferred;

namespace std {
template<> struct greater<br_renderer_gl_deferred>
{
	bool operator()(const br_renderer_gl_deferred& a, const br_renderer_gl_deferred& b) const;
};

}

/*
* Renderer - must be compatible with br_renderer_state
*/
typedef struct br_renderer_gl
{
	/* Dispatch table */
	const struct br_renderer_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Pointer to owning device */
	br_device_gl *device;

	/* List of objects associated with this renderer */
	void *object_list;

	br_device_pixelmap_gl *pixelmap;
	br_renderer_facility_gl *renderer_facility;

	GLSTATE state;

	using deferred_set = std::multiset<br_renderer_gl_deferred, std::greater<br_renderer_gl_deferred>>;
	deferred_set geom;

	bool has_begun;

} br_renderer_gl;

template<>
inline constexpr bool is_object_v<br_renderer_gl> = true;

#endif /* _RENDERER_H_ */