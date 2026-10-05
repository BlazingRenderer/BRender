# BRender TRM — function reference transcript (printed pages 295–305)

## BrMapFind

page: 295
declaration: br_pixelmap* BrMapFind(const char* pattern)
description: Find a texture map in the registry by name. A call-back function can be setup to be called if the search is unsuccessful. The search pattern can include the standard wild cards '*' and '?'.
arguments:
- const char * pattern — Search pattern.
result: br_pixelmap * — Returns a pointer to the texture map if found, otherwise NULL. If a call-back exists and is called, the call-back's return value is returned.
see_also: BrMapFindHook() 295, BrMapFindMany() 295

## BrMapFindMany

page: 295
declaration: br_uint_32 BrMapFindMany(const char* pattern, br_pixelmap** pixelmaps, int max)
description: Find a number of texture maps in the registry by name. The search pattern can include the standard wild cards '*' and '?'.
arguments:
- const char * pattern — Search pattern.
- br_pixelmap ** pixelmaps — A pointer to an array of pointers to texture maps.
- int max — Maximum number of texture maps to find.
result: br_uint_32 — Returns the number of texture maps found. The pointer array is filled with pointers to the found texture maps.
see_also: BrMapFind() 295, BrMapFindHook() 295

## BrMapFindHook

page: 295 (continues to 296)
declaration: br_map_find_cbfn* BrMapFindHook(br_map_find_cbfn* hook)
description: Functions to set up a call-back.
arguments:
- br_map_find_cbfn * hook — A pointer to a call-back function.
effects: If BrMapFind() 295 is unsuccessful and a call-back has been set up, the call-back is passed the search pattern as its only argument. The call-back should then return a pointer to a substitute or default texture map. For example, a call-back could be set up to return a default texture map if the desired texture map cannot be found in the registry. The function BrMapFindFailedLoad() 296 is provided and will probably be sufficient in many cases.
result: br_map_find_cbfn * — Returns a pointer to the old call-back function.
example:
```
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
```
see_also: BrMapFindFailedLoad() 296

## BrMapFindFailedLoad

page: 296
declaration: br_pixelmap* BrMapFindFailedLoad(const char* name)
description: This function is provided as a suitable function to supply to BrMapFindHook() 295.
arguments:
- const char * name — The name supplied to BrMapFind() 295.
effects: Attempts to load the texture map from the filing system using name as the filename. Searches in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined). If successful, sets this name as the identifier of the loaded texture map and adds the texture map to the registry.
result: br_pixelmap * — Returns a pointer to the texture map, if found, else NULL.
example: BrMapFindHook(BrMapFindFailedLoad);

## BrTableCount

page: 297
declaration: br_uint_32 BrTableCount(const char* pattern)
description: Count the number of registered shade tables whose names match a given search pattern. The search pattern can include the standard wild cards '*' and '?'.
arguments:
- const char * pattern — Search pattern.
result: br_uint_32 — Returns the number of shade tables matching the search pattern.
see_also: BrTableEnum() 297, BrTableFind() 298

## BrTableEnum

page: 297
declaration: br_uint_32 BrTableEnum(const char* pattern, br_table_enum_cbfn* callback, void* arg)
description: Calls a call-back function for every shade table matching a given search pattern. The call-back is passed a pointer to each matching shade table, and its second argument is an optional pointer supplied by the user. The search pattern can include the standard wild cards '*' and '?'. The call-back itself returns a br_uint_32 388 value. The enumeration will halt at any stage if the return value is non-zero.
arguments:
- const char * pattern — Search pattern.
- br_table_enum_cbfn * callback — A pointer to a call-back function.
- void * arg — An optional argument to pass to the call-back function.
result: br_uint_32 — Returns the first non-zero call-back return value, or zero if all matching shade tables are enumerated.
example:
```
br_uint_32 BR_CALLBACK test_callback(br_pixelmap* table, void* arg)
{ br_uint_32 count;
...
    return(count);
}
...
{ br_uint_32 enum;
...
    enum = BrTableEnum("Table",&test_callback,NULL);
}
```

## BrTableFind

page: 298
declaration: br_pixelmap* BrTableFind(const char* pattern)
description: Find a shade table in the registry by name. A call-back function can be setup to be called if the search is unsuccessful. The search pattern can include the standard wild cards '*' and '?'.
arguments:
- const char * pattern — Search pattern.
result: br_pixelmap * — Returns a pointer to the shade table if found, otherwise NULL. If a call-back exists and is called, the call-back's return value is returned.
see_also: BrTableFindHook() 299, BrTableFindMany() 298

## BrTableFindMany

page: 298
declaration: br_uint_32 BrTableFindMany(const char* pattern, br_pixelmap** pixelmaps, int max)
description: Find a number of shade tables in the registry by name. The search pattern can include the standard wild cards '*' and '?'.
arguments:
- const char * pattern — Search pattern.
- br_pixelmap ** pixelmaps — A pointer to an array of pointers to shade tables.
- int max — Maximum number of shade tables to find.
result: br_uint_32 — Returns the number of shade tables found. The pointer array is filled with pointers to the found shade tables.
see_also: BrTableFind() 298, BrTableFindHook() 299

## BrTableFindHook

page: 299
declaration: br_table_find_cbfn* BrTableFindHook(br_table_find_cbfn* hook)
description: Functions to set up a call-back. If BrTableFind() 298 is unsuccessful and a call-back has been set up, the call-back it is passed the search pattern as its only argument. The call-back should then return a pointer to a substitute or default shade table. For example, a call-back could be set up to return a default shade table if the desired shade table cannot be found in the registry. The function BrTableFindFailedLoad() 299 is provided and will probably be sufficient in many cases.
arguments:
- br_table_find_cbfn * hook — A pointer to a call-back function.
result: br_table_find_cbfn * — Returns a pointer to the old call-back function.
example:
```
br_table BR_CALLBACK * test_callback(char* pattern)
{ br_table* default_table;
...
    return(default_table);
}
...
{ br_table* table;
...
    BrTableFindHook(&test_callback);
    table = BrTableFind("non_existent_table");
}
```
see_also: BrTableFindFailedLoad() 299

## BrTableFindFailedLoad

page: 299 (continues to 300)
declaration: br_pixelmap* BrTableFindFailedLoad(const char* name)
description: This function is provided as a suitable function to supply to BrTableFindHook() 299.
arguments:
- const char * name — The name supplied to BrTableFind() 298.
effects: Attempts to load the shade table from the filing system using name as the filename. Searches in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined). If successful, sets this name as the identifier of the loaded shade table and adds the shade table to the registry.
result: br_pixelmap * — Returns a pointer to the shade table, if found, else NULL.
example: BrTableFindHook(BrTableFindFailedLoad);

## BrPixelmapFileCount

page: 301
declaration: br_uint_32 BrPixelmapFileCount(const char* filename, br_uint_16* num)
description: Locate a given file and count the number of pixel maps in it.
arguments:
- const char * filename — Name of the file containing the pixel maps to count.
- br_uint_16 * num — Pointer to the variable in which to store the number of pixel maps counted in the file. If NULL, the file will still be located and appropriate success returned, but no count will be made.
effects: Searches for filename, if no path specified with file looks in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined). If a file is found, will count the number of pixel maps stored in it.
result: br_uint_32 — Returns zero if the file was found (even if it is not a pixel map file), non-zero otherwise.

## BrPixelmapLoad

page: 301
declaration: br_pixelmap* BrPixelmapLoad(const char* filename)
description: Load a pixel map. Note that they are not added to the registry.
arguments:
- const char * filename — Name of the file containing the pixel map to load.
effects: Searches for filename, if no path specified with file looks in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined).
result: br_pixelmap * — Returns a pointer to the loaded pixel map, or NULL if unsuccessful.
see_also: BrPixelmapLoadMany() 301, BrPixelmapSave() 304, BrMapAdd() 287, BrTableAdd() 288

## BrPixelmapLoadMany

page: 301 (continues to 302)
declaration: br_uint_32 BrPixelmapLoadMany(const char* filename, br_pixelmap** pixelmaps, br_uint_16 num)
description: Load a number of pixel maps. Note that they are not added to the registry.
arguments:
- const char * filename — Name of the file containing the pixel maps to load.
- br_pixelmap ** pixelmaps — A non-NULL pointer to an array of pointers to pixel maps.
- br_uint_16 num — Maximum number of pixel maps to load.
effects: Searches for filename, if no path specified with file looks in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined).
result: br_uint_32 — Returns the number of pixel maps loaded successfully. The pointer array is filled with pointers to the loaded pixel maps.
see_also: See BrPixelmapFileCount() 247 to determine the number of pixel maps in a file.

## BrFmtBMPLoad

page: 302
declaration: br_pixelmap* BrFmtBMPLoad(const char* name, br_uint_32 flags)
description: Load a pixel map in the BMP format.
arguments:
- const char * name — Name of the file containing the pixel map.
- br_uint_32 flags — Either BR_PMT_RGBX_888 or BR_PMT_RGBA_8888, when the source pixel map uses 32 bits per pixel. Zero otherwise.
effects: Searches for filename, if no path specified with file looks in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined).
result: br_pixelmap * — Returns a pointer to the loaded pixel map.

## BrFmtGIFLoad

page: 302 (continues to 303)
declaration: br_pixelmap* BrFmtGIFLoad(const char* name, br_uint_32 flags)
description: Load a pixel map in the GIF format.
arguments:
- const char * name — Name of the file containing the pixel map.
- br_uint_32 flags — Either BR_PMT_RGBX_888 or BR_PMT_RGBA_8888, when the source pixel map uses 32 bits per pixel. Zero otherwise.
effects: Searches for filename, if no path specified with file looks in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined).
result: br_pixelmap * — Returns a pointer to the loaded pixel map.

## BrFmtIFFLoad

page: 303
declaration: br_pixelmap* BrFmtIFFLoad(const char* name, br_uint_32 flags)
description: Load a pixel map in the IFF format.
arguments:
- const char * name — Name of the file containing the pixel map.
- br_uint_32 flags — Either BR_PMT_RGBX_888 or BR_PMT_RGBA_8888, when the source pixel map uses 32 bits per pixel. Zero otherwise.
effects: Searches for filename, if no path specified with file looks in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined).
result: br_pixelmap * — Returns a pointer to the loaded pixel map.

## BrFmtTGALoad

page: 303 (continues to 304)
declaration: br_pixelmap* BrFmtTGALoad(const char* name, br_uint_32 flags)
description: Load a pixel map in the TGA format.
arguments:
- const char * name — Name of the file containing the pixel map.
- br_uint_32 flags — Either BR_PMT_RGBX_888 or BR_PMT_RGBA_8888, when the source pixel map uses 32 bits per pixel. Zero otherwise.
effects: Searches for filename, if no path specified with file looks in current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if defined).
result: br_pixelmap * — Returns a pointer to the loaded pixel map.

## BrPixelmapSave

page: 304
declaration: br_uint_32 BrPixelmapSave(const char* filename, const br_pixelmap* pixelmap)
description: Save a pixel map to a file.
arguments:
- const char * filename — Name of the file to save the pixel map to.
- const br_pixelmap * pixelmap — A pointer to a pixel map.
effects: Writes the pixel map to a file.*
result: br_uint_32 — Returns NULL if the pixel map could not be saved.
notes: '*' Any existing file of the same name is overwritten.

## BrPixelmapSaveMany

page: 304
declaration: br_uint_32 BrPixelmapSaveMany(const char* filename, const br_pixelmap* const* pixelmaps, br_uint_16 num)
description: Save a number of pixel maps to a file.
arguments:
- const char * filename — Name of the file to save the pixel maps to.
- const br_pixelmap * const * pixelmaps — A pointer to an array of pointers to pixel maps. If NULL, all registered texture maps and shade tables are saved (irrespective of num).
- br_uint_16 num — Number of pixel maps to save.
effects: Writes the pixel maps to a file.†
result: br_uint_32 — Returns the number of pixel maps saved successfully.
notes: '†' Any existing file of the same name is overwritten.

## Notes

- Transcribed 19 function entries: BrMapFind, BrMapFindMany, BrMapFindHook,
  BrMapFindFailedLoad, BrTableCount, BrTableEnum, BrTableFind, BrTableFindMany,
  BrTableFindHook, BrTableFindFailedLoad, BrPixelmapFileCount, BrPixelmapLoad,
  BrPixelmapLoadMany, BrFmtBMPLoad, BrFmtGIFLoad, BrFmtIFFLoad, BrFmtTGALoad,
  BrPixelmapSave, BrPixelmapSaveMany.
- Printed page 305 (image pg-311.png) is blank — it is the end of the br_pixelmap
  section. No entries there.
- Four entries start in this range and continue past the last visible line of a
  page; I read the following page and transcribed the whole entry, noting the
  continuation in the `page:` line (BrMapFindHook 295→296, BrTableFindFailedLoad
  299→300, BrPixelmapLoadMany 301→302, BrFmtGIFLoad 302→303, BrFmtTGALoad
  303→304).
- I added an `example:` field for entries that carry an `Example:` block, and a
  `notes:` field for the two page footnotes on page 304; neither is in the
  supplied template, but dropping them would have lost manual content.
- Trailing numbers after a function name (e.g. `BrMapFind() 295`,
  `br_uint_32 388`) are the manual's subscripts — cross-reference page numbers.
  They are reproduced as printed.
- The section is entered at printed page 295 mid-way through the `br_pixelmap`
  type section; earlier pixelmap functions (BrPixelmapAllocate etc.) live on
  earlier pages and are outside this range.
- All field text is verbatim from the manual, including its grammar quirks
  ("can be setup to be called", "the call-back it is passed the search pattern").
