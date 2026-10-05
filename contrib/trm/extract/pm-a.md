## BrPixelmapFill

page: 281
declaration: void BrPixelmapFill(br_pixelmap* dat, br_uint_32 colour)
description: Fill a pixel map with a given value.
arguments:
- br_pixelmap * dat — A pointer to the pixel map to be filled.
- br_uint_32 colour — Value to set each pixel to.

## BrPixelmapRectangleFill

page: 281
declaration: void BrPixelmapRectangleFill(br_pixelmap* dst, br_int_16 x, br_int_16 y, br_uint_16 w, br_uint_16 h, br_uint_32 colour)
description: Fill a rectangular window in a pixel map with a given value.
arguments:
- br_pixelmap * dst — A pointer to the destination pixel map.
- br_int_16 x, y — Co-ordinates of the rectangle's top left corner.
- br_uint_16 w, h — Rectangle width and height (in pixels).
- br_uint_32 colour — Value to set each pixel to.
example:
br_int_16 x,y;
br_uint_16 w,h;
br_pixelmap *offscreen;
...
BrPixelmapRectangleFill(offscreen,x,y,w,h,0);

## BrPixelmapLine

page: 281
declaration: void BrPixelmapLine(br_pixelmap* dst, br_int_16 x1, br_int_16 y1, br_int_16 x2, br_int_16 y2, br_uint_32 colour)
description: Draw a line in a pixel map between (x1,y1) and (x2,y2), clipping it to the edges of the pixel map if necessary.
arguments:
- br_pixelmap * dst — A pointer to the destination pixel map.
- br_int_16 x1,y1,x2,y2 — Co-ordinates of the line's endpoints.
- br_uint_32 colour — Value to set each pixel in the line to.

## BrPixelmapPixelSet

page: 282
declaration: void BrPixelmapPixelSet(br_pixelmap* dst, br_int_16 x, br_int_16 y, br_uint_32 colour)
description: Set a pixel to a given value.
arguments:
- br_pixelmap * dst — A pointer to the destination pixel map.
- br_int_16 x,y — Pixel co-ordinates.
- br_uint_32 colour — Pixel value.

## BrPixelmapPixelGet

page: 282
declaration: br_uint_32 BrPixelmapPixelGet(const br_pixelmap* src, br_int_16 x, br_int_16 y)
description: Get the value of a particular pixel
arguments:
- const br_pixelmap * src — A pointer to the source pixel map from which to read the pixel.
- br_int_16 x,y — Pixel co-ordinates.
result: br_uint_32 colour — Pixel value. If the point is off-screen, zero will be returned.
remarks: Some device oriented pixel maps may not support read operations.

## BrPixelmapText

page: 283
declaration: void BrPixelmapText(br_pixelmap* dst, br_int_16 x, br_int_16 y, br_uint_32 colour, const br_font* font, const char* text)
description: Write a string into a pixel map in a given font.
arguments:
- br_pixelmap * dst — A pointer to the destination pixel map.
- br_int_16 x, y — Co-ordinates of text's top left corner.
- br_uint_32 colour — Value to set each text pixel to.
- const br_font * font — A pointer to a BRender font, or NULL for the default font. The following pointers are available: BrFontFixed3x5, BrFontProp4x6, BrFontProp7x9. See brfont.h for precise declaration.
- const char * text — A string.
example:
br_uint_32 colour;
br_pixelmap *pmap;
...
BrPixelmapText(pmap,0,0,colour,BrFontProp7x9,"Example text...");

## BrPixelmapTextF

page: 283
declaration: void BrPixelmapTextF(br_pixelmap* dst, br_int_16 x, br_int_16 y, br_uint_32 colour, const br_font* font, const char* fmt, ...)
description: Write a 'printf' formatted string into a pixel map in a given font. The function will accept format strings and arguments just as for the standard printf() function.
arguments:
- br_pixelmap * dst — A pointer to the destination pixel map.
- br_int_16 x, y — Co-ordinates of text's top left corner.
- br_uint_32 colour — Value to set each text pixel to.
- const br_font * font — A pointer to a BRender font, or NULL for the default font. The following pointers are available: BrFontFixed3x5, BrFontProp4x6, BrFontProp7x9. See brfont.h for precise declaration.
- const char * fmt — A format string (as for printf()).
example:
br_uint_32 colour;
br_pixelmap *pmap;
...
BrPixelmapTextF
( pmap,0,0,255,NULL,"Frames/Sec = %16g Polys/Sec = %16g"
, TIMING_FRAMES/((end_time-start_time)/(double)CLOCK_RATE)
, total_faces/((end_time-start_time)/(double)CLOCK_RATE)
);

## BrPixelmapTextWidth

page: 284
declaration: br_uint_16 BrPixelmapTextWidth(const br_pixelmap* dst, const br_font* font, const char* text)
description: Find the width of a string for a given font and pixel map.
arguments:
- const br_pixelmap * dst — A pointer to a pixel map.
- const br_font * font — A pointer to a BRender font, or NULL for the default font.
- const char * text — A string.
result: br_uint_16 — Returns the string width in pixels. If the text argument is NULL, the width of one character is returned.
see_also: BrPixelmapText() 283

## BrPixelmapTextHeight

page: 285
declaration: br_uint_16 BrPixelmapTextHeight(const br_pixelmap* dst, const br_font* font)
description: Find the height of a font for a given pixel map.
arguments:
- const br_pixelmap * dst — A pointer to a pixel map.
- const br_font * font — A pointer to a BRender font, or NULL for the default font.
result: Returns the font height in pixels.
see_also: BrPixelmapText() 283

## BrPixelmapCopy

page: 285
declaration: void BrPixelmapCopy(br_pixelmap* dst, const br_pixelmap* src)
description: Copy the data in one pixel map to another. The source and destination pixel maps must have the same type and dimensions.
arguments:
- br_pixelmap * dst — A pointer to the destination pixel map.
- const br_pixelmap * src — A pointer to the source pixel map.
example:
br_pixelmap *offscreen, *backdrop;
...
BrPixelmapCopy(offscreen, backdrop);

## BrPixelmapRectangleCopy

page: 286
declaration: void BrPixelmapRectangleCopy (br_pixelmap* dst, br_int_16 dx, br_int_16 dy, const br_pixelmap* src, br_int_16 sx, br_int_16 sy, br_uint_16 w, br_uint_16 h)
description: Copy a rectangular window from one pixel map to another.
arguments:
- br_pixelmap * dst — A pointer to the destination pixel map.
- br_int_16 dx, dy — Co-ordinates of the destination rectangle's top left corner.
- const br_pixelmap * src — A pointer to the source pixel map.
- br_int_16 sx, sy — Co-ordinates of the source rectangle's top left corner.
- br_uint_16 w, h — Rectangle width and height (in pixels).

## BrMapAdd

page: 286
declaration: br_pixelmap* BrMapAdd(br_pixelmap* pixelmap)
description: Add a texture map to the registry, updating it as necessary. All texture maps must be added to the registry before they are subsequently involved in rendering.
arguments:
- br_pixelmap * pixelmap — A pointer to a texture map.
result: br_pixelmap * — Returns a pointer to the added texture map, else NULL if unsuccessful.
see_also: BrMapUpdate() 287, BrMapAddMany() 287, BrPixelmapLoad() 301, BrMapFind() 295, BrMapRemove() 288

## BrMapAddMany

page: 287
declaration: br_uint_32 BrMapAddMany(br_pixelmap* const* pixelmaps, int n)
description: Add a number of texture maps to the registry, updating them as necessary.
arguments:
- br_pixelmap * const * pixelmaps — A pointer to an array of pointers to texture maps.
- int n — Number of texture maps to add to the registry.
result: br_uint_32 — Returns the number of texture maps added successfully.
see_also: BrMapUpdate() 287, BrMapAdd() 287, BrMapRemove() 288, BrMapRemoveMany() 288

## BrMapUpdate

page: 287
declaration: void BrMapUpdate(br_pixelmap* pixelmap, br_uint_16 flags)
description: Update a texture map.
arguments:
- br_pixelmap * pixelmap — A pointer to a texture map.
- br_uint_16 flags — Texture map update flags. In general, BR_MAPU_ALL should be used.
see_also: BrMapAdd() 287

## BrMapRemove

page: 287
declaration: br_pixelmap* BrMapRemove(br_pixelmap* pixelmap)
description: Remove a texture map from the registry.
[continues]

## Notes

- Transcribed 15 function entries.
- The brief's template lists description/declaration/arguments/preconditions/effects/result/remarks/see_also. Several entries in this range also have an `Example:` field in the manual; I have included those verbatim under an `example:` field (deliberate deviation from the exact template, since omitting them would lose manual content). Nothing else was present.
- `BrPixelmapText` and `BrPixelmapTextF` document the `font` argument with a vertical list of available pointers (BrFontFixed3x5 / BrFontProp4x6 / BrFontProp7x9) followed by "See brfont.h for precise declaration." That vertical list has been joined into the same bullet with commas to fit the one-line-per-argument format.
- `BrMapRemove` begins on page 287 but continues onto the following page, which is outside this range; it is marked `[continues]`.
- Pages 277–280 (images pg-283 to pg-286) are the `br_pixelmap` type documentation (structure, members, related functions) with no function reference entries; nothing was transcribed from them.
- No illegible glyphs were encountered; no `[?]` markers were needed.
