/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: fw_p.h 1.2 1998/11/12 13:19:14 johng Exp $
 * $Locker: $
 *
 * Public function prototypes for BRender framework
    Last change:  MIP  29 Nov 96    5:50 pm
 */
#ifndef _FW_P_H_
#define _FW_P_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

/*
 * Global setup
 */
br_error BR_PUBLIC_ENTRY BrBegin(void);
br_error BR_PUBLIC_ENTRY BrEnd(void);

/*
 * Framework Setup
 */
br_error BR_PUBLIC_ENTRY BrFwBegin(void);
br_error BR_PUBLIC_ENTRY BrFwEnd(void);

/*
 * Resource class handling
 */
br_resource_class *BR_PUBLIC_ENTRY     BrResClassAdd(br_resource_class *pixelmap);
br_resource_class *BR_PUBLIC_ENTRY     BrResClassRemove(br_resource_class *pixelmap);
br_resource_class *BR_PUBLIC_ENTRY     BrResClassFind(const char *pattern);
typedef br_resource_class *BR_CALLBACK br_resclass_find_cbfn(const char *name);
br_resclass_find_cbfn *BR_PUBLIC_ENTRY BrResClassFindHook(br_resclass_find_cbfn *hook);
br_uint_32 BR_PUBLIC_ENTRY             BrResClassAddMany(br_resource_class **items, int n);
br_uint_32 BR_PUBLIC_ENTRY             BrResClassRemoveMany(br_resource_class **items, int n);
br_uint_32 BR_PUBLIC_ENTRY             BrResClassFindMany(const char *pattern, br_resource_class **items, int max);
br_uint_32 BR_PUBLIC_ENTRY             BrResClassCount(const char *pattern);

typedef br_uint_32 BR_CALLBACK br_resclass_enum_cbfn(br_resource_class *item, void *arg);

br_uint_32 BR_PUBLIC_ENTRY BrResClassEnum(const char *pattern, br_resclass_enum_cbfn *callback, void *arg);

const char *BR_RESIDENT_ENTRY BrResClassIdentifier(br_uint_8 res_class);

/*
 * Block pool allocator
 */
br_pool *BR_PUBLIC_ENTRY BrPoolAllocate(int block_size, int chunk_size, br_uint_8 mem_type);
void BR_PUBLIC_ENTRY     BrPoolFree(br_pool *pool);

void *BR_PUBLIC_ENTRY BrPoolBlockAllocate(struct br_pool *pool);
void BR_PUBLIC_ENTRY  BrPoolBlockFree(struct br_pool *pool, void *block);

void BR_PUBLIC_ENTRY BrPoolEmpty(struct br_pool *pool);

/*
 * Byte swapping
 */
br_uint_32 BR_RESIDENT_ENTRY BrSwap32(br_uint_32 l);
br_uint_16 BR_RESIDENT_ENTRY BrSwap16(br_uint_16 s);
br_float BR_RESIDENT_ENTRY   BrSwapFloat(br_float f);
void *BR_RESIDENT_ENTRY      BrSwapBlock(void *block, int count, int size);

/*
 * Misc. support
 */
typedef int BR_CALLBACK br_qsort_cbfn(const void *, const void *);
void BR_RESIDENT_ENTRY  BrQsort(void *basep, unsigned int nelems, unsigned int size, br_qsort_cbfn *comp);

typedef int BR_CALLBACK br_bsearch_cbfn(const void *, const void *);
void *BR_RESIDENT_ENTRY BrBSearch(const void *key, const void *base, unsigned int nmemb, unsigned int size, br_bsearch_cbfn *comp);

/*
 * Diagnostic generation
 */
void BR_RESIDENT_ENTRY BrFailure(const char *s, ...);
void BR_RESIDENT_ENTRY BrWarning(const char *s, ...);
void BR_RESIDENT_ENTRY BrFatal(const char *name, int line, char *s, ...);

/*
 * Debug Break
 */
void BR_PUBLIC_ENTRY BrDebugBreak(void);

/*
 * Set new handlers
 */
br_diaghandler *BR_PUBLIC_ENTRY BrDiagHandlerSet(br_diaghandler *newdh);
br_filesystem *BR_PUBLIC_ENTRY  BrFilesystemSet(br_filesystem *newfs);
br_allocator *BR_PUBLIC_ENTRY   BrAllocatorSet(br_allocator *newal);
br_loghandler *BR_PUBLIC_ENTRY  BrLogHandlerSet(br_loghandler *newlh);

/*
 * Backwards compatibility
 */
#define BrErrorHandlerSet BrDiagHandlerSet

/*
 * Generic file IO
 */
br_uint_32 BR_PUBLIC_ENTRY BrFileAttributes(void);

void *BR_PUBLIC_ENTRY     BrFileOpenRead(const char *name, br_size_t n_magics, br_mode_test_cbfn *mode_test, int *mode_result);
void *BR_PUBLIC_ENTRY     BrFileOpenWrite(const char *name, int text);
void BR_PUBLIC_ENTRY      BrFileClose(void *f);
int BR_PUBLIC_ENTRY       BrFileEof(void *f);
int BR_PUBLIC_ENTRY       BrFileGetChar(void *f);
void BR_PUBLIC_ENTRY      BrFilePutChar(int c, void *f);
br_size_t BR_PUBLIC_ENTRY BrFileRead(void *buf, br_size_t size, br_size_t n, void *f);
br_size_t BR_PUBLIC_ENTRY BrFileWrite(const void *buf, br_size_t size, br_size_t n, void *f);
br_size_t BR_PUBLIC_ENTRY BrFileGetLine(char *buf, br_size_t buf_len, void *f);
void BR_PUBLIC_ENTRY      BrFilePutLine(const char *buf, void *f);
void BR_PUBLIC_ENTRY      BrFileAdvance(long int count, void *f);
void *BR_PUBLIC_ENTRY     BrFileLoad(void *res, const char *name, br_size_t *size);

int BR_PUBLIC_ENTRY BrFilePrintf(void *f, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);

br_int_32 BR_RESIDENT_ENTRY BrSprintf(char *buf, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
br_int_32 BR_RESIDENT_ENTRY BrSprintfN(char *buf, br_size_t buf_size, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(3, 4);

br_int_32 BR_RESIDENT_ENTRY BrSScanf(char *str, const char *fmt, ...) BR_SCANF_ATTRIBUTE(2, 3);

/*
 * Data file output type (one of BR_FS_MODE_xxx)
 */
int BR_PUBLIC_ENTRY BrWriteModeSet(int text);

/*
 * Generic memory allocation
 */
void *BR_RESIDENT_ENTRY     BrMemAllocate(br_size_t size, br_uint_8 type);
void *BR_RESIDENT_ENTRY     BrMemReallocate(void *block, br_size_t size, br_uint_8 type);
void BR_RESIDENT_ENTRY      BrMemFree(void *block);
br_size_t BR_RESIDENT_ENTRY BrMemInquire(br_uint_8 type);
br_int_32 BR_RESIDENT_ENTRY BrMemAlign(br_uint_8 type);

#if 0
void * BR_RESIDENT_ENTRY BrMemAllocateAlign(br_size_t size, br_uint_8 type, br_int_32 align);
#endif

char *BR_RESIDENT_ENTRY BrMemStrDup(const char *str);
void *BR_RESIDENT_ENTRY BrMemCalloc(br_size_t nelems, br_size_t size, br_uint_8 type);

/*
 * Resource allocation
 */
void *BR_RESIDENT_ENTRY BrResAllocate(void *vparent, br_size_t size, br_uint_8 res_class);
void BR_RESIDENT_ENTRY  BrResFree(void *vres);
void BR_RESIDENT_ENTRY  BrResFreeNoCallback(void *vres);
char *BR_RESIDENT_ENTRY BrResStrDup(void *vparent, const char *str);
char *BR_RESIDENT_ENTRY BrResVSprintf(void *vparent, const char *fmt, va_list ap);
char *BR_RESIDENT_ENTRY BrResSprintf(void *vparent, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);

void *BR_RESIDENT_ENTRY       BrResAdd(void *vparent, void *vres);
void *BR_RESIDENT_ENTRY       BrResRemove(void *vres);
br_uint_8 BR_RESIDENT_ENTRY   BrResClass(void *vres);
br_size_t BR_RESIDENT_ENTRY   BrResSize(void *vres);
br_size_t BR_RESIDENT_ENTRY   BrResSizeTotal(void *vres);
typedef br_size_t BR_CALLBACK br_resenum_cbfn(void *vres, void *arg);
br_size_t BR_RESIDENT_ENTRY   BrResChildEnum(void *vres, br_resenum_cbfn *callback, void *arg);
br_boolean BR_RESIDENT_ENTRY  BrResIsChild(void *vparent, void *vchild);

/*
 * Block operations (XXX should be made private)
 */
void BR_ASM_CALL BrBlockFill(void *dest_ptr, int value, int dwords);
void BR_ASM_CALL BrBlockCopy(void *dest_ptr, void *src_ptr, int dwords);

#if BR_HAS_FAR
void BR_ASM_CALL BrFarBlockCopy(void __far *dest_ptr, void *src_ptr, int dwords);
#endif

/*
 * Scratchpad buffer allocation - Currenty, only one allocation
 * may be outstanding at any time
 */
void *BR_RESIDENT_ENTRY     BrScratchAllocate(br_size_t size);
void BR_RESIDENT_ENTRY      BrScratchFree(void *scratch);
br_size_t BR_RESIDENT_ENTRY BrScratchInquire(void);
void BR_RESIDENT_ENTRY      BrScratchFlush(void);

/*
 * A seperate, fixed size scratch buffer, typically used as a scratch string workspace
 *
 * At least 512 bytes in size
 */
br_size_t BR_RESIDENT_ENTRY BrScratchStringSize(void);
char *BR_RESIDENT_ENTRY     BrScratchString(void);

/*
 * Error retrieval
 */
br_error BR_RESIDENT_ENTRY    BrGetLastError(void **valuep);
const char *BR_RESIDENT_ENTRY BrStrError(br_error error);

/*
 * Device mangement
 */
struct br_device;

typedef struct br_device *BR_EXPORT br_device_begin_fn(const char *arguments);

br_error BR_RESIDENT_ENTRY BrDevAdd(struct br_device **pdev, const char *image, const char *args);
br_error BR_RESIDENT_ENTRY BrDevCheckAdd(struct br_device **pdev, const char *image, const char *args);

br_error BR_RESIDENT_ENTRY BrDevAddStatic(struct br_device **pdev, br_device_begin_fn *begin, const char *args);
br_error BR_RESIDENT_ENTRY BrDevAddConfig(const char *config);
br_error BR_RESIDENT_ENTRY BrDevRemove(struct br_device *dev);

br_error BR_RESIDENT_ENTRY BrDevicesQuery(struct br_device **devices, br_int_32 *ndevices, br_int_32 max_devices);

br_error BR_RESIDENT_ENTRY BrDevFindMany(struct br_device **devices, br_int_32 *ndevices, br_int_32 max_devices, const char *pattern);
br_error BR_RESIDENT_ENTRY BrDevCount(br_int_32 *ndevices, const char *pattern);
br_error BR_RESIDENT_ENTRY BrDevFind(struct br_device **pdev, const char *pattern);

br_error BR_RESIDENT_ENTRY BrDevContainedFind(struct br_object **ph, br_token type, const char *pattern, br_token_value *tv);
br_error BR_RESIDENT_ENTRY BrDevContainedFindMany(struct br_object **objects, br_int_32 max_objects, br_int_32 *pnum_objects, br_token type,
                                                  const char *pattern, br_token_value *tv);
br_error BR_RESIDENT_ENTRY BrDevContainedCount(br_int_32 *pcount, br_token type, const char *pattern, br_token_value *tv);

br_error BR_PUBLIC_ENTRY     BrDevBegin(br_pixelmap **ppm, const char *setup_string);
br_error BR_PUBLIC_ENTRY     BrDevBeginVar(br_pixelmap **ppm, const char *device_name, ...);
br_error BR_PUBLIC_ENTRY     BrDevBeginTV(br_pixelmap **ppm, const char *device_name, br_token_value *tv);
br_pixelmap *BR_PUBLIC_ENTRY BrDevBeginOld(const char *setup_string);
void BR_PUBLIC_ENTRY         BrDevEndOld(void);
void BR_PUBLIC_ENTRY         BrDevPaletteSetOld(br_pixelmap *pm);
void BR_PUBLIC_ENTRY         BrDevPaletteSetEntryOld(int i, br_colour colour);

/*
 * Callback function invoked when a device is enumerated
 */
typedef br_boolean BR_CALLBACK br_device_enum_cbfn(const char *identifer, br_uint_32 version, const char *creator, const char *title,
                                                   const char *product, const char *product_version, void *args);

/*
 * Callback function invoked when an output facility is enumerated
 */
typedef struct br_outfcty_desc {
    br_int_32  width;
    br_int_32  width_min;
    br_int_32  width_max;
    br_int_32  height;
    br_int_32  height_min;
    br_int_32  height_max;
    br_uint_8  pmtype;
    br_int_32  pmbits;
    br_boolean indexed;
    br_boolean fullscreen;

    /*
     * A pointer to the output facility.  Note that this is only guaranteed
     * to remain valid within the callback routine, and is is strictly
     * for the purposes of gathering further information
     */
    struct br_output_facility *output_facility;

} br_outfcty_desc;

typedef br_boolean BR_CALLBACK br_outfcty_enum_cbfn(const char *identifier, br_outfcty_desc *desc, void *args);

struct br_device_pixelmap;
struct br_renderer;
struct br_geometry;
struct br_lexer_source;
struct br_lexer;

/*
 * Enumeration routines.
 */
br_error BR_PUBLIC_ENTRY BrDeviceEnum(br_device_enum_cbfn *cbfn, void *args);
br_error BR_PUBLIC_ENTRY BrOutputFacilityEnum(const char *name, br_outfcty_enum_cbfn *cbfn, void *args);

struct br_renderer_facility;

br_error BR_RESIDENT_ENTRY BrRendererFacilityFind(struct br_renderer_facility **prf, struct br_device_pixelmap *destination, br_token scalar_type);

br_error BR_RESIDENT_ENTRY BrRendererFacilityListFind(struct br_renderer_facility **prf, br_int_32 *num_rf, br_int_32 max_rf,
                                                      struct br_device_pixelmap *destination, br_token scalar_type);

struct br_primitive_library;

br_error BR_RESIDENT_ENTRY BrPrimitiveLibraryFind(struct br_primitive_library **ppl, struct br_device_pixelmap *destination, br_token scalar_type);

br_error BR_RESIDENT_ENTRY BrPrimitiveLibraryListFind(struct br_primitive_library **ppl, br_int_32 *num_pl, br_int_32 max_pl,
                                                      struct br_device_pixelmap *destination, br_token scalar_type);

br_error BR_RESIDENT_ENTRY BrGeometryFormatFind(struct br_geometry **pgf, struct br_renderer *renderer,
                                                struct br_renderer_facility *renderer_facility, br_token scalar_type, br_token format_type);

/*
 * lists
 */

void BR_RESIDENT_ENTRY BrNewList(br_list *list);
void BR_RESIDENT_ENTRY BrAddHead(br_list *list, br_node *node);
void BR_RESIDENT_ENTRY BrAddTail(br_list *list, br_node *node);

br_node *BR_RESIDENT_ENTRY BrRemHead(br_list *list);
br_node *BR_RESIDENT_ENTRY BrRemTail(br_list *list);

void BR_RESIDENT_ENTRY     BrInsert(br_list *list, br_node *here, br_node *node);
br_node *BR_RESIDENT_ENTRY BrRemove(br_node *node);

void BR_RESIDENT_ENTRY            BrSimpleNewList(br_simple_list *list);
void BR_RESIDENT_ENTRY            BrSimpleAddHead(br_simple_list *list, br_simple_node *node);
br_simple_node *BR_RESIDENT_ENTRY BrSimpleRemHead(br_simple_list *list);
void BR_RESIDENT_ENTRY            BrSimpleInsert(br_simple_list *list, br_simple_node *here, br_simple_node *node);
br_simple_node *BR_RESIDENT_ENTRY BrSimpleRemove(br_simple_node *node);

/*
 * Hash
 */
br_hash BR_RESIDENT_ENTRY BrHash(const void *data, size_t size);
br_hash BR_RESIDENT_ENTRY BrHashString(const char *s);

/*
 * Hash Map.
 */

/**
 * @brief Manually hash a key.
 *
 * @param   hm The hash map instance. May not be NULL.
 * @param   key A pointer to the key.
 * @return  The hash of the provided key. This will never be #BR_INVALID_HASH.
 */
br_hash BR_RESIDENT_ENTRY BrHashMapHash(const br_hashmap *hm, const void *key);

/**
 * @brief Compare two keys for equality.
 *
 * @param   hm The hash map instance. May not be NULL.
 * @param   a A pointer to the first key.
 * @param   b A pointer to the second key.
 * @return  If the keys are equal, returns BR_TRUE, or BR_FALSE if they're not.
 */
br_boolean BR_RESIDENT_ENTRY BrHashMapCompare(const br_hashmap *hm, const void *a, const void *b);

br_hashmap *BR_RESIDENT_ENTRY BrHashMapAllocate(void *vparent, br_hashmap_hash_cbfn *hash, br_hashmap_compare_cbfn *compare);
int BR_RESIDENT_ENTRY         BrHashMapClear(br_hashmap *hm);

void BR_RESIDENT_ENTRY BrHashMapSetResizePolicy(br_hashmap *hm, br_hashmap_resize_policy policy);

/**
 * @brief Configure the minimum and maximum load factors of the hash map.
 *
 * @param   hm      The hash map instance.
 * @param   min_num The numerator of the minimum load factor.
 * @param   min_den The denominator of the minimum load factor.
 * @param   max_num The numerator of the maximum load factor.
 * @param   max_den The denominator of the maximum load factor.
 * @return  On success, returns 0. If the function fails, it returns a negative error value.
 *          The function can fail under the following conditions:
 *          - `hm` is NULL.
 *          - One or both of the numerators or denominators are 0.
 *          - One of both of the numerators are greater than or equal to its
 *            respective denominator.
 *          - The minimum load factor is greater than the maximum load factor.
 */
int BR_RESIDENT_ENTRY BrHashMapSetLoadFactor(br_hashmap *hm, uint16_t min_num, uint16_t min_den, uint16_t max_num, uint16_t max_den);

/**
 * @brief Reset a hash map to its default state, releasing all memory.
 *
 * It may be used as if `BrHashMapAllocate()` has just been called.
 *
 * @param hm The hash map instance. Must not be NULL.
 */
void BR_RESIDENT_ENTRY BrHashMapReset(br_hashmap *hm);

/**
 * @brief Free a hash map and all contents.
 *
 * @param hm The hash map instance. Must not be NULL.
 *
 * @remark If attached to a parent resource, this call may be omitted.
 */
void BR_RESIDENT_ENTRY BrHashMapFree(br_hashmap *hm);

/**
 * @brief Resize a hash map so it can hold `nelem` elements.
 *
 * @param   hm The hash map instance. Must not be NULL.
 * @param   nelem The number of elements this map must be able to hold.
 *          This cannot be less than the current number of elements in the map.
 * @return
 *
 * @remark  This is *not* affected by the current resize policy.
 */
int BR_RESIDENT_ENTRY        BrHashMapResize(br_hashmap *hm, br_size_t nelem);
int BR_RESIDENT_ENTRY        BrHashMapInsert(br_hashmap *hm, const void *key, void *value);
void *BR_RESIDENT_ENTRY      BrHashMapFindByHash(const br_hashmap *hm, br_hash hash);
void *BR_RESIDENT_ENTRY      BrHashMapFind(const br_hashmap *hm, const void *key);
br_boolean BR_RESIDENT_ENTRY BrHashMapUpdate(br_hashmap *hm, const void *key, void *value);
void *BR_RESIDENT_ENTRY      BrHashMapRemove(br_hashmap *hm, const void *key);
br_size_t BR_RESIDENT_ENTRY  BrHashMapSize(const br_hashmap *hm);

int BR_RESIDENT_ENTRY BrHashMapEnumerate(const br_hashmap *hm, br_hashmap_enum_cbfn proc, void *user);

br_hash BR_RESIDENT_ENTRY    BrHashMapDefaultHash(const void *k);
br_boolean BR_RESIDENT_ENTRY BrHashMapDefaultCompare(const void *a, const void *b);

br_hash BR_RESIDENT_ENTRY    BrHashMapStringHash(const void *s);
br_boolean BR_RESIDENT_ENTRY BrHashMapStringCompare(const void *a, const void *b);

/*
 * Logging routines.
 */
br_uint_8 BR_RESIDENT_ENTRY BrLogSetLevel(br_uint_8 level);
br_uint_8 BR_RESIDENT_ENTRY BrLogGetLevel(void);

void BR_RESIDENT_ENTRY BrLogV(br_uint_8 level, const char *component, const char *fmt, va_list ap);
void BR_RESIDENT_ENTRY BrLog(br_uint_8 level, const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(3, 4);
void BR_RESIDENT_ENTRY BrLogTrace(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
void BR_RESIDENT_ENTRY BrLogDebug(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
void BR_RESIDENT_ENTRY BrLogInfo(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
void BR_RESIDENT_ENTRY BrLogWarn(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
void BR_RESIDENT_ENTRY BrLogError(const char *component, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif
