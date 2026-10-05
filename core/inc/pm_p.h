/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: pm_p.h 1.3 1998/11/06 14:54:25 jon Exp $
 * $Locker: $
 *
 * Public function prototypes for BRender pixelmap support
 */
#ifndef _PM_P_H_
#define _PM_P_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

/*
 * Pixelmap support setup
 */
void BR_PUBLIC_ENTRY BrPixelmapBegin(void);
void BR_PUBLIC_ENTRY BrPixelmapEnd(void);

/*
 * Pixelmap management
 */
/**
 * \brief Allocate a new pixel map.
 *
 * \param type   Pixel map type.
 * \param w      Width in pixels.
 * \param h      Height in pixels.
 * \param pixels A pointer to an existing block of memory. If NULL, the pixel memory is allocated
 *               automatically using BrResAllocate(). The calculation obtaining the minimum size
 *               required for the block of memory is non-obvious: it is row_bytes * h, and row_bytes
 *               is not simply w * bits-per-pixel / 8. To determine row_bytes, it is probably
 *               simplest to call this function with a dummy, non-NULL pointer (the address of an
 *               automatic, say), and record the value of row_bytes returned in the br_pixelmap
 *               (free it immediately afterwards). The memory pointed to by \p pixels is not
 *               accessed by this function.
 * \param flags  Supply either BR_PMAF_NORMAL or BR_PMAF_INVERTED. If the latter, the function will
 *               automatically set the pixels member of the returned pixel map (whether supplied or
 *               not) to be at the start of the final row of pixel map memory (row_bytes is
 *               negative).
 *               \li BR_PMAF_NORMAL - pixel map ordinate x increases, and y decreases, as addresses
 *                   within the memory block ascend.
 *               \li BR_PMAF_INVERTED - pixel map ordinate x increases, and y increases, as
 *                   addresses within the memory block ascend.
 *
 * \return A pointer to the new pixel map, or NULL if unsuccessful.
 */
br_pixelmap *BR_RESIDENT_ENTRY BrPixelmapAllocate(br_uint_8 type, br_int_32 w, br_int_32 h, void *pixels, int flags);
/**
 * \brief Find the pixel size for a given pixel map.
 *
 * \param pm A pointer to a pixel map.
 *
 * \return The size of each pixel, in bits.
 */
br_uint_16 BR_RESIDENT_ENTRY   BrPixelmapPixelSize(br_pixelmap *pm);
/**
 * \brief Find the channels available for a given pixel map.
 *
 * \param pm A pointer to a pixel map.
 *
 * \return A mask giving the available channels, being a combination of BR_PMCHAN_INDEX,
 *         BR_PMCHAN_RGB, BR_PMCHAN_DEPTH, BR_PMCHAN_ALPHA and BR_PMCHAN_YUV.
 */
br_uint_16 BR_RESIDENT_ENTRY   BrPixelmapChannels(br_pixelmap *pm);

/**
 * \brief Allocate a pixel map as part of an existing pixel map.
 *
 * The new pixel map is clipped to the existing pixel map.
 *
 * \param pm A pointer to an existing pixel map.
 * \param x  X co-ordinate of the top left of the new pixel map in the existing pixel map.
 * \param y  Y co-ordinate of the top left of the new pixel map in the existing pixel map.
 * \param w  Width of the new pixel map.
 * \param h  Height of the new pixel map.
 *
 * \return A pointer to the new pixel map, or NULL if unsuccessful.
 *
 * \remark The effective origin is set that of the existing pixel map.
 */
br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapAllocateSub(br_pixelmap *pm, br_int_32 x, br_int_32 y, br_int_32 w, br_int_32 h);

/**
 * \brief Deallocate a pixel map and any associated memory.
 *
 * \param pm A pointer to a pixel map.
 */
void BR_PUBLIC_ENTRY BrPixelmapFree(br_pixelmap *pm);

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapResize(br_pixelmap *pm, br_int_32 width, br_int_32 height);

/**
 * \brief Given a pixel map, allocate either a depth buffer or an off-screen colour buffer with the
 *        same dimensions.
 *
 * \param src        A pointer to the source pixel map.
 * \param match_type The type of matching pixel map required.
 *                   \li BR_PMMATCH_OFFSCREEN - create a pixel map of the same type and dimensions.
 *                   \li BR_PMMATCH_DEPTH_16 - create an appropriate 16 bit depth buffer.
 *
 * \return A pointer to the matching pixel map.
 */
br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapMatch(br_pixelmap *src, br_uint_8 match_type);

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapMatchSized(br_pixelmap *src, br_uint_8 match_type, br_int_32 width, br_int_32 height);

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapMatchTyped(br_pixelmap *src, br_uint_8 match_type, br_uint_8 pixelmap_type);

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapMatchTypedSized(br_pixelmap *src, br_uint_8 match_type, br_uint_8 pixelmap_type, br_int_32 width,
                                                       br_int_32 height);

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapMatchTV(br_pixelmap *src, br_token_value *tv);

/**
 * \brief Create a pixel map of the same type and dimensions and copy the pixel data.
 *
 * \param src A pointer to the source pixel map.
 *
 * \return A pointer to the new pixel map.
 *
 * \par Example
 * \code{.c}
 * br_pixelmap *image, *working_copy;
 * ...
 * image = BrPixelmapLoad("backdrop.pix");
 * working_copy = BrPixelmapClone(image);
 * \endcode
 */
br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapClone(br_pixelmap *src);
br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapCloneTyped(br_pixelmap *src, br_uint_8 type);
br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapConvert(br_pixelmap *src, br_uint_8 type, const br_pixelmap_convert_options *opts);

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapDirectLock(br_pixelmap *src, br_boolean block);
br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapDirectUnlock(br_pixelmap *src);

/*
 * Pixelmap operations
 */
/**
 * \brief Fill a pixel map with a given value.
 *
 * \param dst    A pointer to the pixel map to be filled.
 * \param colour Value to set each pixel to.
 */
void BR_PUBLIC_ENTRY BrPixelmapFill(br_pixelmap *dst, br_uint_32 colour);

void BR_PUBLIC_ENTRY BrPixelmapRectangle(br_pixelmap *dst, br_int_32 x, br_int_32 y, br_int_32 w, br_int_32 h, br_uint_32 colour);
void BR_PUBLIC_ENTRY BrPixelmapRectangle2(br_pixelmap *dst, br_int_32 x, br_int_32 y, br_int_32 w, br_int_32 h, br_uint_32 colour1,
                                          br_uint_32 colour2);
/**
 * \brief Copy a rectangular window from one pixel map to another.
 *
 * \param dst A pointer to the destination pixel map.
 * \param dx  X co-ordinate of the destination rectangle's top left corner.
 * \param dy  Y co-ordinate of the destination rectangle's top left corner.
 * \param src A pointer to the source pixel map.
 * \param sx  X co-ordinate of the source rectangle's top left corner.
 * \param sy  Y co-ordinate of the source rectangle's top left corner.
 * \param w   Rectangle width (in pixels).
 * \param h   Rectangle height (in pixels).
 */
void BR_PUBLIC_ENTRY BrPixelmapRectangleCopy(br_pixelmap *dst, br_int_32 dx, br_int_32 dy, br_pixelmap *src, br_int_32 sx, br_int_32 sy,
                                             br_int_32 w, br_int_32 h);
void BR_PUBLIC_ENTRY BrPixelmapRectangleStretchCopy(br_pixelmap *dst, br_int_32 dx, br_int_32 dy, br_int_32 dw, br_int_32 dh,
                                                    br_pixelmap *src, br_int_32 sx, br_int_32 sy, br_int_32 sw, br_int_32 sh);
/**
 * \brief Fill a rectangular window in a pixel map with a given value.
 *
 * \param dst    A pointer to the destination pixel map.
 * \param x      X co-ordinate of the rectangle's top left corner.
 * \param y      Y co-ordinate of the rectangle's top left corner.
 * \param w      Rectangle width (in pixels).
 * \param h      Rectangle height (in pixels).
 * \param colour Value to set each pixel to.
 *
 * \par Example
 * \code{.c}
 * br_int_16 x,y;
 * br_uint_16 w,h;
 * br_pixelmap *offscreen;
 * ...
 * BrPixelmapRectangleFill(offscreen,x,y,w,h,0);
 * \endcode
 */
void BR_PUBLIC_ENTRY BrPixelmapRectangleFill(br_pixelmap *dst, br_int_32 x, br_int_32 y, br_int_32 w, br_int_32 h, br_uint_32 colour);

/**
 * \brief Copy a rectangular window of data from one pixel map to the same position in another pixel
 *        map.
 *
 * This function is intended to be used in conjunction with a rendering call-back to copy those
 * regions of a pixel map that have been rendered to.
 *
 * \param dst A pointer to the destination pixel map.
 * \param src A pointer to the source pixel map.
 * \param x   Co-ordinates of the rectangle's top left corner.
 * \param y   Co-ordinates of the rectangle's top left corner.
 * \param w   Rectangle width and height (in pixels).
 * \param h   Rectangle width and height (in pixels).
 *
 * \pre The source and destination pixel maps must have the same type and dimensions.
 *
 * \post Copies an area of pixels from the source to the destination pixel map, such that each pixel
 *       in the specified rectangle is copied.
 *
 * \remark The actual area copied depends upon the platform and implementation, but this function is
 *         intended to provide the fastest way of copying a particular rectangle. It is possible
 *         that this could be as large as the entire pixel map. Hopefully, most redundant, repeated
 *         calls would be ignored.
 *
 * \sa BrZbRenderBoundsCallbackSet()
 *
 * \par Example
 * \code{.c}
 * br_int_16 drx, dry;
 * br_uint_16 drw, drh;
 * br_pixelmap* offscreen;
 * br_pixelmap* backdrop;
 * ...
 * BrPixelmapDirtyRectangleCopy(offscreen, backdrop,drx,dry,drw,drh);
 * \endcode
 */
void BR_PUBLIC_ENTRY BrPixelmapDirtyRectangleCopy(br_pixelmap *dst, br_pixelmap *src, br_int_32 x, br_int_32 y, br_int_32 w, br_int_32 h);
void BR_PUBLIC_ENTRY BrPixelmapDirtyRectangleClear(br_pixelmap *dst, br_int_32 x, br_int_32 y, br_int_32 w, br_int_32 h, br_uint_32 colour);
void BR_PUBLIC_ENTRY BrPixelmapDirtyRectangleDoubleBuffer(br_pixelmap *dst, br_pixelmap *src, br_int_32 x, br_int_32 y, br_int_32 w, br_int_32 h);

/*
 * Backwards compatibility
 */
#define BrPixelmapDirtyRectangleFill BrPixelmapDirtyRectangleClear

/**
 * \brief Set a pixel to a given value.
 *
 * \param dst    A pointer to the destination pixel map.
 * \param x      X co-ordinate of the pixel.
 * \param y      Y co-ordinate of the pixel.
 * \param colour Pixel value.
 */
void BR_PUBLIC_ENTRY       BrPixelmapPixelSet(br_pixelmap *dst, br_int_32 x, br_int_32 y, br_uint_32 colour);
/**
 * \brief Get the value of a particular pixel.
 *
 * \param dst A pointer to the source pixel map from which to read the pixel.
 * \param x   X co-ordinate of the pixel.
 * \param y   Y co-ordinate of the pixel.
 *
 * \return The pixel value. If the point is off-screen, zero will be returned.
 *
 * \remark Some device oriented pixel maps may not support read operations.
 */
br_uint_32 BR_PUBLIC_ENTRY BrPixelmapPixelGet(br_pixelmap *dst, br_int_32 x, br_int_32 y);
/**
 * \brief Copy the data in one pixel map to another.
 *
 * The source and destination pixel maps must have the same type and dimensions.
 *
 * \param dst A pointer to the destination pixel map.
 * \param src A pointer to the source pixel map.
 *
 * \par Example
 * \code{.c}
 * br_pixelmap *offscreen, *backdrop;
 * ...
 * BrPixelmapCopy(offscreen, backdrop);
 * \endcode
 */
void BR_PUBLIC_ENTRY       BrPixelmapCopy(br_pixelmap *dst, br_pixelmap *src);
/**
 * \brief Draw a line in a pixel map between (x1,y1) and (x2,y2), clipping it to the edges of the
 *        pixel map if necessary.
 *
 * \param dst    A pointer to the destination pixel map.
 * \param x1     X co-ordinate of the line's first endpoint.
 * \param y1     Y co-ordinate of the line's first endpoint.
 * \param x2     X co-ordinate of the line's second endpoint.
 * \param y2     Y co-ordinate of the line's second endpoint.
 * \param colour Value to set each pixel in the line to.
 */
void BR_PUBLIC_ENTRY       BrPixelmapLine(br_pixelmap *dst, br_int_32 x1, br_int_32 y1, br_int_32 x2, br_int_32 y2, br_uint_32 colour);
/**
 * \brief Write a string into a pixel map in a given font.
 *
 * \param dst    A pointer to the destination pixel map.
 * \param x      X co-ordinate of the text's top left corner.
 * \param y      Y co-ordinate of the text's top left corner.
 * \param colour Value to set each text pixel to.
 * \param font   A pointer to a BRender font, or NULL for the default font. BrFontFixed3x5,
 *               BrFontProp4x6 and BrFontProp7x9 are available; see brfont.h for their precise
 *               declaration.
 * \param text   A string.
 *
 * \par Example
 * \code{.c}
 * br_uint_32 colour;
 * br_pixelmap *pmap;
 * ...
 * BrPixelmapText(pmap,0,0,colour,BrFontProp7x9,"Example text...");
 * \endcode
 */
void BR_PUBLIC_ENTRY       BrPixelmapText(br_pixelmap *dst, br_int_32 x, br_int_32 y, br_uint_32 colour, br_font *font, const char *text);
/**
 * \brief Write a printf-formatted string into a pixel map in a given font.
 *
 * The function will accept format strings and arguments just as for the standard printf()
 * function.
 *
 * \param dst    A pointer to the destination pixel map.
 * \param x      X co-ordinate of the text's top left corner.
 * \param y      Y co-ordinate of the text's top left corner.
 * \param colour Value to set each text pixel to.
 * \param font   A pointer to a BRender font, or NULL for the default font. BrFontFixed3x5,
 *               BrFontProp4x6 and BrFontProp7x9 are available; see brfont.h for their precise
 *               declaration.
 * \param fmt    A format string (as for printf()).
 */
void BR_PUBLIC_ENTRY BrPixelmapTextF(br_pixelmap *dst, br_int_32 x, br_int_32 y, br_uint_32 colour, br_font *font, const char *fmt, ...);

void BR_PUBLIC_ENTRY BrPixelmapCopyBits(br_pixelmap *dst, br_int_32 x, br_int_32 y, br_uint_8 *src, br_int_32 s_stride, br_int_32 start_bit,
                                        br_int_32 end_bit, br_int_32 nrows, br_uint_32 colour);

/**
 * \brief Find the width of a string for a given font and pixel map.
 *
 * \param dst  A pointer to a pixel map.
 * \param font A pointer to a BRender font, or NULL for the default font.
 * \param text A string.
 *
 * \return The string width in pixels. If \p text is NULL, the width of one character is returned.
 *
 * \sa BrPixelmapText()
 */
br_uint_16 BR_PUBLIC_ENTRY BrPixelmapTextWidth(br_pixelmap *dst, br_font *font, const char *text);
/**
 * \brief Find the height of a font for a given pixel map.
 *
 * \param dst  A pointer to a pixel map.
 * \param font A pointer to a BRender font, or NULL for the default font.
 *
 * \return The font height in pixels.
 *
 * \sa BrPixelmapText()
 */
br_uint_16 BR_PUBLIC_ENTRY BrPixelmapTextHeight(br_pixelmap *dst, br_font *font);

/**
 * \brief If the destination pixel map relates to a device, for example a graphics hardware screen,
 *        then the source 'off-screen' pixel map is copied to the destination pixel map at a
 *        suitable moment.
 *
 * If the source is an off screen pixel map (created using BrPixelmapMatch(...,
 * BR_PMMATCH_OFFSCREEN)) then it is swapped with the destination pixel map instead. Otherwise, the
 * function is equivalent to BrPixelmapCopy() and the source pixel map is copied to the destination
 * pixel map.
 *
 * \param dst A pointer to the destination pixel map.
 * \param src A pointer to the source pixel map.
 *
 * \post If the destination is a device that supports a 'wait for vertical retrace' function, a copy
 *       or swap will be performed pending that event. If the destination is a device that supports
 *       double buffering and the source pixel map is an off-screen secondary buffer, the
 *       destination and source will be switched to use each other's buffers.
 *
 * \remark Returns immediately, but will cause further rendering or calls of this function to block
 *         until the copy or swap has been completed.
 */
void BR_PUBLIC_ENTRY BrPixelmapDoubleBuffer(br_pixelmap *dst, br_pixelmap *src);

void BR_PUBLIC_ENTRY BrPixelmapPaletteSet(br_pixelmap *pm, br_pixelmap *pal);
void BR_PUBLIC_ENTRY BrPixelmapPaletteEntrySet(br_pixelmap *pm, br_int_32 i, br_colour colour);
void BR_PUBLIC_ENTRY BrPixelmapPaletteEntrySetMany(br_pixelmap *pm, br_int_32 index, br_int_32 ncolours, br_colour *colours);

/*
 * Backwards compatibility
 */
#define BrPixelmapPlot BrPixelmapPixelSet

/**
 * \brief Load a pixel map.
 *
 * The pixel map is not added to the registry.
 *
 * \param filename Name of the file containing the pixel map to load.
 *
 * \post Searches for \p filename; if no path is specified, the current directory is searched
 *       first, then the directories listed in BRENDER_PATH (if defined).
 *
 * \return A pointer to the loaded pixel map, or NULL if unsuccessful.
 *
 * \sa BrPixelmapLoadMany(), BrPixelmapSave(), BrMapAdd(), BrTableAdd()
 */
br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapLoad(const char *filename);
/**
 * \brief Save a pixel map to a file.
 *
 * \param filename Name of the file to save the pixel map to.
 * \param pixelmap A pointer to a pixel map.
 *
 * \post Writes the pixel map to a file. Any existing file of the same name is overwritten.
 *
 * \return One if the pixel map was saved successfully, zero otherwise.
 */
br_uint_32 BR_PUBLIC_ENTRY   BrPixelmapSave(const char *filename, br_pixelmap *pixelmap);
/**
 * \brief Load a number of pixel maps.
 *
 * The pixel maps are not added to the registry.
 *
 * \param filename  Name of the file containing the pixel maps to load.
 * \param pixelmaps A non-NULL pointer to an array of pointers to pixel maps.
 * \param num       Maximum number of pixel maps to load.
 *
 * \post Searches for \p filename; if no path is specified, the current directory is searched
 *       first, then the directories listed in BRENDER_PATH (if defined).
 *
 * \return The number of pixel maps loaded successfully. The pointer array is filled with pointers
 *         to the loaded pixel maps.
 *
 * \sa BrPixelmapLoad()
 */
br_uint_32 BR_PUBLIC_ENTRY   BrPixelmapLoadMany(const char *filename, br_pixelmap **pixelmaps, br_uint_16 num);
/**
 * \brief Save a number of pixel maps to a file.
 *
 * \param filename  Name of the file to save the pixel maps to.
 * \param pixelmaps A pointer to an array of pointers to pixel maps. If NULL, all registered texture
 *                  maps and shade tables are saved (irrespective of \p num).
 * \param num       Number of pixel maps to save.
 *
 * \post Writes the pixel maps to a file. Any existing file of the same name is overwritten.
 *
 * \return The number of pixel maps saved successfully.
 */
br_uint_32 BR_PUBLIC_ENTRY   BrPixelmapSaveMany(const char *filename, br_pixelmap **pixelmaps, br_uint_16 num);

/*
 * Scaling (XXX interface will change)
 */
void BR_PUBLIC_ENTRY BrScaleBegin(void);
void BR_PUBLIC_ENTRY BrScaleEnd(void);

/*
 * scale pixelmap src to new_x,new_y with filter width fwidth
 * src must be BR_PMT_RGB_888
 */
br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapScale(br_pixelmap *src, br_uint_32 new_x, br_uint_32 new_y, float fwidth);

/*
 * Qunatization  (XXX interface will change)
 */
void BR_PUBLIC_ENTRY BrQuantBegin(void);
void BR_PUBLIC_ENTRY BrQuantEnd(void);

/*
 * add array of <size> rgb values to quantizer
 */
void BR_PUBLIC_ENTRY BrQuantAddColours(br_uint_8 *colours, br_uint_32 size);

/*
 * make optimum palette
 */
void BR_PUBLIC_ENTRY BrQuantMakePalette(int base, int num_entries, br_pixelmap *palette);

/*
 * given palette with base and num_entries, setup internal map
 */
void BR_PUBLIC_ENTRY BrQuantPrepareMapping(int base, int num_entries, br_pixelmap *palette);

/*
 * get <size> index values for array of <size> rgb values
 */
void BR_PUBLIC_ENTRY BrQuantMapColours(int base, br_uint_8 *colours, br_uint_8 *mapped_colours, int size);

/*
 * Produce a pixelmap containing mip levels from a pixelmap.
 */

br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapMakeMipMap(br_pixelmap *source, br_uint_32 destinationType, br_pixelmap *palette, br_uint_32 base,
                                                  br_uint_32 range, br_uint_32 quantizationMethod);

br_error BR_PUBLIC_ENTRY BrPixelmapGetControls(br_pixelmap *pm, br_display_controls *controls);
br_error BR_PUBLIC_ENTRY BrPixelmapSetControls(br_pixelmap *pm, br_display_controls *controls);

br_error BR_PUBLIC_ENTRY BrPixelmapHandleWindowEvent(br_pixelmap *src, void *arg);

/*
 * Convert an BR_PMT_INDEX_8 pixelmap to a BR_PMT_RGBA_8888 or BR_PMT_RGBX_888
 * pixelmap, depending on keyed transparency.
 */
br_pixelmap *BR_PUBLIC_ENTRY BrPixelmapDeCLUT(br_pixelmap *src);

/**
 * \brief Resize a colour and depth buffer to match their attached screen, falling back to recreation if
 *        resizing fails.
 *
 * \param screen A pointer to the screen. Must not be NULL.
 * \param colour A pointer to receive the new colour pixelmap. Must not be NULL.
 *               If the pixelmap is recreated, it will be matched to the screen's current pixel format.
 *               If \c *colour is NULL, recreation is forced.
 *
 * \param depth A pointer to receive the new depth pixelmap. Must not be NULL.
 *              If the pixelmap is recreated, it will be a depth buffer containing at least 16-bit precision.
 *              If \c *depth is NULL, recreation is forced.
 *
 * \return Returns BRE_OK on success, or a BRE_* error code on failure.
 */
br_error BR_PUBLIC_ENTRY BrPixelmapResizeBuffers(br_pixelmap *screen, br_pixelmap **colour, br_pixelmap **depth);

/**
 * \brief Resize a colour and depth buffer to match their attached screen, falling back to recreation if
 *        resizing fails.
 *
 * \param screen A pointer to the screen. Must not be NULL.
 * \param colour A pointer to receive the new colour pixelmap. Must not be NULL.
 *               If the pixelmap is recreated, it will be matched to the screen's current pixel format.
 *               If \c *colour is NULL, recreation is forced.
 *
 * \param depth A pointer to receive the new depth pixelmap. Must not be NULL.
 *              If the pixelmap is recreated, it will be a depth buffer containing at least 16-bit precision.
 *              If \c *depth is NULL, recreation is forced.
 * \param tv    A br_token_value list containing additional options. Supported tokens are:
 *              - \c BRT_MSAA_SAMPLES_I32
 *              - \c BRT_WIDTH_I32 (defaults to \c screen->width)
 *              - \c BRT_HEIGHT_I32 (defaults to \c screen->height)
 *              - \c BRT_PIXEL_TYPE_U8 (colour buffer, defaults to \c screen->type)
 *              - \c BRT_PIXEL_BITS_I32 (depth buffer)
 *
 * \return Returns BRE_OK on success, or a BRE_* error code on failure.
 */
br_error BR_PUBLIC_ENTRY BrPixelmapResizeBuffersTV(br_pixelmap *screen, br_pixelmap **colour, br_pixelmap **depth, const br_token_value *tv);

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif
