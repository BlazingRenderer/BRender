#ifndef _DRV_HPP_
#define _DRV_HPP_

#include <brddi.h>
#include <token.h>
#include <tokenmap.h>
#include "device.hpp"
#include "object.hpp"

/*
* Macros that expand to the first two arguments of a template entry
* Builtin or device token
*/
#define BRT(t)	(br_token)BRT_##t

#undef ObjectDevice
#undef ObjectFree
#undef ObjectIdentifier
#undef ObjectIsType
#undef ObjectQueryManySize
#undef ObjectQueryMany

#undef ObjectContainerAddFront
#undef ObjectContainerRemove

template<typename T> inline constexpr bool is_object_v					= false;
template<> inline constexpr bool is_object_v<br_object>					= true;
template<> inline constexpr bool is_object_v<br_object_gl>				= true;
template<> inline constexpr bool is_object_v<br_device>					= true;
template<> inline constexpr bool is_object_v<br_device_gl>				= true;
template<> inline constexpr bool is_object_v<br_pixelmap>				= true;
template<> inline constexpr bool is_object_v<br_device_pixelmap>		= true;
template<> inline constexpr bool is_object_v<br_device_pixelmap_gl>		= true;
template<> inline constexpr bool is_object_v<br_buffer_stored>			= true;
template<> inline constexpr bool is_object_v<br_output_facility>		= true;
template<> inline constexpr bool is_object_v<br_output_facility_gl>		= true;
template<> inline constexpr bool is_object_v<br_renderer_facility>		= true;
template<> inline constexpr bool is_object_v<br_renderer_facility_gl>	= true;
template<> inline constexpr bool is_object_v<br_geometry_v1_buckets>	= true;
template<> inline constexpr bool is_object_v<br_geometry_primitives>	= true;
template<> inline constexpr bool is_object_v<br_geometry_v1_model>		= true;
template<> inline constexpr bool is_object_v<br_geometry_stored>		= true;
template<> inline constexpr bool is_object_v<br_renderer>				= true;
template<> inline constexpr bool is_object_v<br_renderer_state_stored>	= true;
template<> inline constexpr bool is_object_v<br_geometry_lighting>		= true;

template<typename T, typename = std::enable_if_t<is_object_v<T>>>
inline static br_device *ObjectDevice(T *self)
{
	return reinterpret_cast<br_object*>(self)->dispatch->_device(reinterpret_cast<br_object*>(self));
}

template<typename T, typename = std::enable_if_t<is_object_v<T>>>
inline static void ObjectFree(T *self)
{
	return reinterpret_cast<br_object*>(self)->dispatch->_free(reinterpret_cast<br_object*>(self));
}

template<typename T, typename = std::enable_if_t<is_object_v<T>>>
inline static const char *ObjectIdentifier(T *self)
{
	return reinterpret_cast<br_object*>(self)->dispatch->_identifier(reinterpret_cast<br_object*>(self));
}

template<typename T, typename = std::enable_if_t<is_object_v<T>>>
inline static br_boolean ObjectIsType(T *self, br_token t)
{
	return reinterpret_cast<br_object*>(self)->dispatch->_isType(reinterpret_cast<br_object*>(self), t);
}

template<typename T, typename = std::enable_if_t<is_object_v<T>>>
inline static br_error ObjectQueryManySize(T *self, br_size_t *s, br_token_value *tv)
{
	return reinterpret_cast<br_object_container*>(self)->dispatch->_queryManySize(
		reinterpret_cast<br_object*>(self), s, tv
	);
}

template<typename T, typename = std::enable_if_t<is_object_v<T>>>
inline static br_error ObjectQueryMany(T *self, br_token_value *tv, void *buffer, br_size_t buffer_size, br_int_32 *pcount)
{
	return reinterpret_cast<br_object_container*>(self)->dispatch->_queryMany(
		reinterpret_cast<br_object*>(self), tv, buffer, buffer_size, pcount
	);
}

template<typename T, typename U, typename = std::enable_if_t<is_object_v<T>>, typename = std::enable_if_t<is_object_v<U>>>
inline static br_error ObjectContainerAddFront(T *self, U *ph)
{
	return reinterpret_cast<br_object_container*>(self)->dispatch->_addFront(
		reinterpret_cast<br_object_container*>(self),
		reinterpret_cast<br_object*>(ph)
	);
}

template<typename T, typename U, typename = std::enable_if_t<is_object_v<T>>, typename = std::enable_if_t<is_object_v<U>>>
inline static br_error ObjectContainerRemove(T *self, U *ph)
{
	return reinterpret_cast<br_object_container*>(self)->dispatch->_remove(
		reinterpret_cast<br_object_container*>(self),
		reinterpret_cast<br_object*>(ph)
	);
}

inline static void RendererGLSceneBegin(br_renderer *self)
{
	return self->dispatch->__reserved0(reinterpret_cast<br_object*>(self));
}

inline static void RendererGLSceneEnd(br_renderer *self)
{
	return self->dispatch->__reserved1(reinterpret_cast<br_object*>(self));
}

template<typename T>
T *BrResAllocate(void *vparent, br_uint_8 res_class)
{
	return reinterpret_cast<T*>(::BrResAllocate(vparent, sizeof(T), res_class));
}


#endif /* _DRV_HPP_ */
