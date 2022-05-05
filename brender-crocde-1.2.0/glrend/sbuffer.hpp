#ifndef _SBUFFER_HPP_
#define _SBUFFER_HPP_

#include "drv.hpp"

/* Flags */
enum
{
	SBUFF_SHARED		= 0x0001,	/* Data is shared with user */
	SBUFF_ALLOCONLOAD	= 0x0002,	/* Contents must be copied on use */
	SBUFF_LOADED		= 0x0004,	/* Contents have been loaded */
};


typedef struct br_buffer_stored_gl
{
	/* Dispatch table */
	const struct br_buffer_stored_dispatch *dispatch;

	/* Standard object identifier */
	const char *identifier;

	/* Pointer to owning device */
	struct br_device_gl *device;

	br_renderer_gl *renderer;

	/* Flags */
	br_uint_32 flags;

	/* Stored copy of the source flags. */
	br_uint_16 source_flags;

	/* Source pixelmap */
	br_pixelmap *source;

	GLuint glTex;
	///*
	//* Surface and texture interface representing the source version
	//*
	//* The texture interface is only used for ALLOCONLOAD textures
	//*/
	//LPDIRECTDRAWSURFACE2 source_surface;
	//LPDIRECT3DTEXTURE2 source_texture;
	//
	///*
	//* Surface and texture interface representing the stored version
	//*/
	//LPDIRECTDRAWSURFACE2 stored_surface;
	//LPDIRECT3DTEXTURE2 stored_texture;
	//
	///*
	//* Texture handle, and a timestamp for when it was set
	//*/
	//D3DTEXTUREHANDLE handle;

} br_buffer_stored_gl;

template<>
inline constexpr bool is_object_v<br_buffer_stored_gl> = true;

#endif


