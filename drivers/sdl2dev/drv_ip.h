/*
 * Prototypes for functions internal to driver
 */
#ifndef _DRV_IP_H_
#define _DRV_IP_H_

#ifndef NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

/*
 * driver.c
 */

/*
 * Hidden token a pixelmap may expose to show it's backed by an SDL_Surface.
 */
extern br_token brt_sdl_surface_h;

/*
 * device.c
 */
br_device *DeviceSDL2Allocate(const char *identifier);

/*
 * outfcty.c
 */
br_error            SDLToBRenderPixelFormat(Uint32 format, br_int_32 *bpp, br_uint_8 *type);
br_error            BRenderToSDLPixelFormat(br_uint_8 type, Uint32 *format, br_int_32 *bpp);
br_output_facility *OutputFacilitySDL2CreateGeneric(br_device *dev);

/*
 * devpixmp.c
 */

br_device_pixelmap *DevicePixelmapSDL2Allocate(br_device *dev, br_output_facility *outfcty, SDL_Window *window,
                                               SDL_Surface *surface, br_boolean owned);
br_boolean          DevicePixelmapSDL2IsOurs(const br_pixelmap *pm);
void               *DevicePixelmapSDLMemAddress(br_device_pixelmap *self, br_int_32 x, br_int_32 y);

/*
 * devclut.c
 */
br_device_clut *DeviceClutSDL2Allocate(br_device_pixelmap *pm, const char *identifier);

br_boolean DeviceClutSDL2IsOurs(const br_device_clut *pm);

/*
 * surface.c
 */
br_clip_result DevicePixelmapSDL2PointClip(SDL_Point *out, const br_point *in, const br_pixelmap *pm);
br_clip_result DevicePixelmapSDL2LineClip(SDL_Point *s_out, SDL_Point *e_out, const br_point *s_in,
                                          const br_point *e_in, const br_pixelmap *pm);
br_clip_result DevicePixelmapSDL2RectangleClip(SDL_Rect *out, const br_rectangle *in, const br_pixelmap *pm);
br_clip_result DevicePixelmapSDL2RectangleClipTwo(SDL_Rect *r_out, SDL_Point *p_out, const br_rectangle *r_in,
                                                  const br_point *p_in, const br_pixelmap *pm_dst, const br_pixelmap *pm_src);

/*
 * If the pixelmap exposes an SDL surface, get it.
 * If ignore_locked is BR_TRUE, the surface will be returned even if it's locked.
 * NB: The reason for this is that only WE should be doing the locking. If it's locked,
 *     something else is using it.
 */
SDL_Surface *DevicePixelmapSDL2GetSurface(br_pixelmap *pm, br_boolean ignore_locked);

/*
 * If the pixelmap has a CLUT, get it.
 */
br_device_clut *DevicePixelmapSDL2GetCLUT(br_pixelmap *pm);

br_error DeviceSDL2SetPalette(SDL_Palette *pal, br_int_32 index, br_int_32 count, const br_colour *entries, br_boolean has_alpha);
br_error DeviceSDL2SetPaletteFromCLUT(SDL_Palette *pal, br_device_clut *clut);
br_error DeviceSDL2SetPaletteFromMap(SDL_Palette *pal, const br_pixelmap *map);
br_error DeviceClutSDL2CopyToSurface(SDL_Surface *surf, br_pixelmap *pm, br_device_clut *fallback);

/*
 * Try very hard to blit a surface using SDL.
 */
br_error DevicePixelmapSDL2BlitSurface(br_pixelmap *src, SDL_Rect *sr, br_pixelmap *dst, SDL_Rect *dr, br_sdl_blit_cbfn blit);

/*
 * glrend.c
 */
br_error    DevicePixelmapSDL2CreateGL(const pixelmap_new_tokens *pt, br_device_pixelmap **ppmap);
SDL_Window *DevicePixelmapSDL2GetWindowGL(br_pixelmap *pm);

#ifdef __cplusplus
};
#endif

#endif
#endif
