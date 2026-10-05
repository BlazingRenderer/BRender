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
see_also: BrMapUpdate() 287, BrMapAddMany() 287, BrPixelmapLoad() 301, BrMapFind() 295, BrMapRemove() 288.

## BrMapAddMany

page: 287
declaration: br_uint_32 BrMapAddMany(br_pixelmap* const* pixelmaps, int n)
description: Add a number of texture maps to the registry, updating them as necessary.
arguments:
- br_pixelmap * const * pixelmaps — A pointer to an array of pointers to texture maps.
- int n — Number of texture maps to add to the registry.
result: br_uint_32 — Returns the number of texture maps added successfully.
see_also: BrMapUpdate() 287, BrMapAdd() 287, BrMapRemove() 288, BrMapRemoveMany() 288.

## BrMapUpdate

page: 287
declaration: void BrMapUpdate(br_pixelmap* pixelmap, br_uint_16 flags)
description: Update a texture map.
arguments:
- br_pixelmap * pixelmap — A pointer to a texture map.
- br_uint_16 flags — Texture map update flags. In general, BR_MAPU_ALL should be used.
see_also: BrMapAdd() 287.

## BrMapRemove

page: 287
declaration: br_pixelmap* BrMapRemove(br_pixelmap* pixelmap)
description: Remove a texture map from the registry.
arguments:
- br_pixelmap * pixelmap — A pointer to a texture map.
result: br_pixelmap * — Returns a pointer to the item removed.
see_also: BrMapAdd() 287.

## BrMapRemoveMany

page: 288
declaration: br_uint_32 BrMapRemoveMany(br_pixelmap* const* pixelmaps, int n)
description: Remove a number of texture maps from the registry.
arguments:
- br_pixelmap * const * pixelmaps — A pointer to an array of pointers to texture maps.
- int n — Number of texture maps to remove from the registry.
result: br_uint_32 — Returns the number of texture maps removed successfully.
see_also: BrMapAddMany() 287.

## BrTableAdd

page: 288
declaration: br_pixelmap* BrTableAdd(br_pixelmap* pixelmap)
description: Add a shade table to the registry, updating it as necessary. All shade tables must be added to the registry before they are used subsequently.
arguments:
- br_pixelmap * pixelmap — A pointer to a shade table.
result: br_pixelmap * — Returns a pointer to the added shade table, else NULL if unsuccessful.
see_also: BrTableUpdate() 289, BrTableAddMany() 289, BrPixelmapLoad() 301, BrTableFind() 298, BrTableRemove() 289.

## BrTableAddMany

page: 288
declaration: br_uint_32 BrTableAddMany(br_pixelmap* const* pixelmaps, int n)
description: Add a number of shade tables to the registry, updating them as necessary.
arguments:
- br_pixelmap * const * pixelmaps — A pointer to an array of pointers to shade tables.
- int n — Number of shade tables to add to the registry.
result: br_uint_32 — Returns the number of shade tables added successfully.
see_also: BrTableUpdate() 289, BrTableAdd() 288, BrTableRemove() 289, BrTableRemoveMany() 290.

## BrTableUpdate

page: 289
declaration: void BrTableUpdate(br_pixelmap* pixelmap, br_uint_16 flags)
description: Update a shade table.
arguments:
- br_pixelmap * pixelmap — A pointer to a shade table.
- br_uint_16 flags — Shade table update flags. In general, BR_TABU_ALL should be used.
see_also: BrTableAdd() 288.

## BrTableRemove

page: 289
declaration: br_pixelmap* BrTableRemove(br_pixelmap* pixelmap)
description: Remove a shade table from the registry.
arguments:
- br_pixelmap * pixelmap — A pointer to a shade table.
result: br_pixelmap * — Returns a pointer to the shade table removed.
see_also: BrTableAdd() 288.

## BrTableRemoveMany

page: 289
declaration: br_uint_32 BrTableRemoveMany(br_pixelmap* const* pixelmaps, int n)
description: Remove a number of shade tables from the registry.
arguments:
- br_pixelmap * const * pixelmaps — A pointer to an array of pointers to shade tables.
- int n — Number of shade tables to remove from the registry.
result: br_uint_32 — Returns the number of shade tables removed successfully.
see_also: BrTableAddMany() 289.

## BrPixelmapAllocate

page: 290
declaration: br_pixelmap* BrPixelmapAllocate(br_uint_8 type, br_uint_16 w, br_uint_16 h, void* pixels, int flags)
description: Allocate a new pixel map.
arguments:
- br_uint_8 type — Pixel map type.
- br_uint_16 w — Width in pixels.
- br_uint_16 h — Height in pixels.
- void * pixels — A pointer to an existing block of memory. If NULL, the pixel memory is allocated automatically using BrResAllocate() 48.

  The calculation obtaining the minimum size required for the block of memory is non-obvious. It is row_bytes*h,, and row_bytes is not simply w*bits-per-pixel/8. To determine row_bytes, it is probably simplest to call this function with a dummy, non-NULL pointer (address of an automatic, say), and record the value of row_bytes returned in the br_pixelmap 277 (free it immediately afterwards). The memory pointed to by pixels is not accessed by this function.
- int flags — Supply either BR_PMAF_NORMAL or BR_PMAF_INVERTED. If the latter, the function will automatically set the pixels member of the returned pixel map (whether supplied or not) to be at the start of the final row of pixel map memory (row_bytes is negative).

  | Flags | Meaning |
  | --- | --- |
  | BR_PMAF_NORMAL | Pixel map ordinate x increases, and y decreases as addresses within the memory block ascend. |
  | BR_PMAF_INVERTED | Pixel map ordinate x increases, and y increases as addresses within the memory block ascend. |
result: br_pixelmap * — Returns a pointer to the new pixel map, or NULL if unsuccessful.

## BrPixelmapAllocateSub

page: 291
declaration: br_pixelmap* BrPixelmapAllocateSub(br_pixelmap* pm, br_uint_16 x, br_uint_16 y, br_uint_16 w, br_uint_16 h)
description: Allocate a pixel map as part of an existing pixel map. The new pixel map is clipped to the existing pixel map.
arguments:
- br_pixelmap * pm — A pointer to an existing pixel map.
- br_uint_16 x, y — Co-ordinates of the top left of the new pixel map in the existing pixel map.
- br_uint_16 w, h — Width and height of the new pixel map.
result: br_pixelmap * — Returns a pointer to the new pixel map, or NULL if unsuccessful.
remarks: The effective origin is set that of the existing pixel map.

## BrPixelmapMatch

page: 292
declaration: br_pixelmap* BrPixelmapMatch(const br_pixelmap* src, int match_type)
description: Given a pixel map, allocate either a depth buffer or an off-screen colour buffer with the same dimensions.
arguments:
- const br_pixelmap * src — A pointer to the source pixel map.
- int match_type — The type of matching pixel map required.

  | Match Type | Match Method |
  | --- | --- |
  | BR_PMMATCH_OFFSCREEN | Create a pixel map of the same type and dimensions. |
  | BR_PMMATCH_DEPTH_16 | Create an appropriate 16 bit depth buffer. |
result: br_pixelmap * — Returns a pointer to the matching pixel map.

## BrPixelmapClone

page: 292
declaration: br_pixelmap* BrPixelmapClone(const br_pixelmap* src)
description: Create a pixel map of the same type and dimensions and copy the pixel data.
arguments:
- const br_pixelmap * src — A pointer to the source pixel map.
result: br_pixelmap * — Returns a pointer to the new pixel map.
example:

  br_pixelmap *image, *working_copy;
  ...
  image = BrPixelmapLoad("backdrop.pix");
  working_copy = BrPixelmapClone(image);

## BrPixelmapFree

page: 292
declaration: void BrPixelmapFree(br_pixelmap* pmap)
description: Deallocate a pixel map and any associated memory.
arguments:
- br_pixelmap * pmap — A pointer to a pixel map.

## BrPixelmapPixelSize

page: 293
declaration: br_uint_16 BrPixelmapPixelSize(const br_pixelmap* pm)
description: Find the pixel size for a given pixel map.
arguments:
- const br_pixelmap * pm — A pointer to a pixel map.
result: br_uint_16 — Returns the size of each pixel, in bits.

## BrPixelmapChannels

page: 293
declaration: br_uint_16 BrPixelmapChannels(const br_pixelmap* pm)
description: Find the channels available for a given pixel map.
arguments:
- const br_pixelmap * pm — A pointer to a pixel map.
result: br_uint_16 — Returns a mask giving the available channels, being a combination of the following bit value symbols:

  BR_PMCHAN_INDEX
  BR_PMCHAN_RGB
  BR_PMCHAN_DEPTH
  BR_PMCHAN_ALPHA
  BR_PMCHAN_YUV

## BrMapCount

page: 293
declaration: br_uint_32 BrMapCount(const char* pattern)
description: Count the number of registered texture maps whose names match a given search pattern. The search pattern can include the standard wild cards `*` and `?`.
arguments:
- const char * pattern — Search pattern.
result: br_uint_32 — Returns the number of texture maps matching the search pattern.
see_also: BrMapEnum() 294, BrMapFind() 295.

## BrMapEnum

page: 294
declaration: br_uint_32 BrMapEnum(const char* pattern, br_map_enum_cbfn* callback, void* arg)
description: Calls a call-back function for every texture map matching a given search pattern. The call-back is passed a pointer to each matching item, and its second argument is an optional pointer supplied by the user. The search pattern can include the standard wild cards "*" and "?". The call-back itself returns a br_uint_32 358 value. The enumeration will halt at any stage if the return value is non-zero.
arguments:
- const char * pattern — Search pattern.
- br_map_enum_cbfn * callback — A pointer to a call-back function.
- void * arg — An optional argument to pass to the call-back function.
result: br_uint_32 — Returns the first non-zero call-back return value, or zero if all matching texture maps are enumerated.
example:

  br_uint_32 BR_CALLBACK test_callback(br_pixelmap* map, void* arg)
  { br_uint_32 count;
  ...
      return(count);
  }
  ...
  { br_uint_32 enum;
  ...
      enum = BrMapEnum("map",&test_callback,NULL);
  }

## BrMapFind

page: 295
declaration: br_pixelmap* BrMapFind(const char* pattern)
description: Find a texture map in the registry by name. A call-back function can be setup to be called if the search is unsuccessful. The search pattern can include the standard wild cards `*` and `?`.
arguments:
- const char * pattern — Search pattern.
result: br_pixelmap * — Returns a pointer to the texture map if found, otherwise NULL. If a call-back exists and is called, the call-back's return value is returned.
see_also: BrMapFindHook() 295, BrMapFindMany() 295.

## BrMapFindMany

page: 295
declaration: br_uint_32 BrMapFindMany(const char* pattern, br_pixelmap** pixelmaps, int max)
description: Find a number of texture maps in the registry by name. The search pattern can include the standard wild cards `*` and `?`.
arguments:
- const char * pattern — Search pattern.
- br_pixelmap ** pixelmaps — A pointer to an array of pointers to texture maps.
- int max — Maximum number of texture maps to find.
result: br_uint_32 — Returns the number of texture maps found. The pointer array is filled with pointers to the found texture maps.
see_also: BrMapFind() 295, BrMapFindHook() 295.

## BrMapFindHook

page: 295
declaration: br_map_find_cbfn* BrMapFindHook(br_map_find_cbfn* hook)
description: Functions to set up a call-back.
arguments:
- br_map_find_cbfn * hook — A pointer to a call-back function.
effects: If BrMapFind() 295 is unsuccessful and a call-back has been set up, the call-back is passed the search pattern as its only argument. The call-back should then return a pointer to a substitute or default texture map.

  For example, a call-back could be set up to return a default texture map if the desired texture map cannot be found in the registry.

  The function BrMapFindFailedLoad() 296 is provided and will probably be sufficient in many cases.
result: br_map_find_cbfn * — Returns a pointer to the old call-back function.
example:

  br_map BR_CALLBACK * test_callback(char* pattern)
  { br_map* default_map;
  ...
      return(default_map);
  }
  ...
  { br_map* map;
  ...
      BrMapFindHook(&test_callback);
      map = BrMapFind("non_existent_map");
  }
see_also: BrMapFindFailedLoad() 296.

## BrMapFindFailedLoad

page: 296
declaration: br_pixelmap* BrMapFindFailedLoad(const char* name)
description: This function is provided as a suitable function to supply to BrMapFindHook() 295.
arguments:
- const char * name — The name supplied to BrMapFind() 295.
effects: Attempts to load the texture map from the filing system using name as the filename. Searches in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined). If successful, sets this name as the identifier of the loaded texture map and adds the texture map to the registry.
result: br_pixelmap * — Returns a pointer to the texture map, if found, else NULL.
example:

  BrMapFindHook(BrMapFindFailedLoad);

## Notes

24 entries transcribed (pages 286–296):

- BrPixelmapRectangleCopy (286)
- BrMapAdd (286)
- BrMapAddMany (287)
- BrMapUpdate (287)
- BrMapRemove (287)
- BrMapRemoveMany (288)
- BrTableAdd (288)
- BrTableAddMany (288)
- BrTableUpdate (289)
- BrTableRemove (289)
- BrTableRemoveMany (289)
- BrPixelmapAllocate (290)
- BrPixelmapAllocateSub (291)
- BrPixelmapMatch (292)
- BrPixelmapClone (292)
- BrPixelmapFree (292)
- BrPixelmapPixelSize (293)
- BrPixelmapChannels (293)
- BrMapCount (293)
- BrMapEnum (294)
- BrMapFind (295)
- BrMapFindMany (295)
- BrMapFindHook (295)
- BrMapFindFailedLoad (296)

Points for the reader:

- The images are pages 292–302 of the PDF (printed 286–296). Every entry whose heading appears in this range is included; none of them ran past page 296, so no `[continues]` markers were needed. No entry in these images began on an earlier page.
- `BrPixelmapRectangleCopy`, `BrMapAdd`, `BrMapRemove`, `BrTableAddMany`, `BrTableRemoveMany`, `BrPixelmapAllocate`, `BrPixelmapFree`, `BrMapCount` and `BrMapFindHook` each span a page break; their fields are stitched from both pages.
- The type heading for the whole range is `br_pixelmap`, but a number of entries are registry functions for texture maps (`BrMap*`) and shade tables (`BrTable*`) — these are documented here, not in the `br_actor`/`br_model`/`br_table` sections.
- `BrPixelmapAllocate`'s `pixels` description contains the manual's own typographical quirk `row_bytes*h,,` (two commas), kept verbatim.
- Code examples use typographic (curly) quotes in the manual; I have transcribed them with straight quotes so they can be pasted as C literals.
- Subscript page references after `Br*()` cross-references (e.g. `BrResAllocate() 48`, `br_pixelmap 277`, `br_uint_32 358`) are the manual's own page pointers, kept as-is.
- Tables under `Arguments`/`Result` (`BR_PMAF_*` in `BrPixelmapAllocate`, `BR_PMMATCH_*` in `BrPixelmapMatch`) are reproduced as Markdown tables inside the field.
- Every `Declaration:` line is the manual's; note `BrPixelmapRectangleCopy` and `BrPixelmapAllocate` are split over two lines in the manual and joined here.
- No illegible glyphs; nothing skipped.
