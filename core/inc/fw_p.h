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
BR_API br_error BR_PUBLIC_ENTRY BrBegin(void);
BR_API br_error BR_PUBLIC_ENTRY BrEnd(void);

/*
 * Framework Setup
 */
BR_API br_error BR_PUBLIC_ENTRY BrFwBegin(void);
BR_API br_error BR_PUBLIC_ENTRY BrFwEnd(void);

/*
 * Resource class handling
 */
BR_API br_resource_class *BR_PUBLIC_ENTRY     BrResClassAdd(br_resource_class *pixelmap);
BR_API br_resource_class *BR_PUBLIC_ENTRY     BrResClassRemove(br_resource_class *pixelmap);
BR_API br_resource_class *BR_PUBLIC_ENTRY     BrResClassFind(const char *pattern);
typedef br_resource_class *BR_CALLBACK        br_resclass_find_cbfn(const char *name);
BR_API br_resclass_find_cbfn *BR_PUBLIC_ENTRY BrResClassFindHook(br_resclass_find_cbfn *hook);
BR_API br_uint_32 BR_PUBLIC_ENTRY             BrResClassAddMany(br_resource_class **items, int n);
BR_API br_uint_32 BR_PUBLIC_ENTRY             BrResClassRemoveMany(br_resource_class **items, int n);
BR_API br_uint_32 BR_PUBLIC_ENTRY BrResClassFindMany(const char *pattern, br_resource_class **items, int max);
BR_API br_uint_32 BR_PUBLIC_ENTRY BrResClassCount(const char *pattern);

typedef br_uint_32 BR_CALLBACK br_resclass_enum_cbfn(br_resource_class *item, void *arg);

BR_API br_uint_32 BR_PUBLIC_ENTRY BrResClassEnum(const char *pattern, br_resclass_enum_cbfn *callback, void *arg);

BR_API const char *BR_RESIDENT_ENTRY BrResClassIdentifier(br_uint_8 res_class);

/*
 * Block pool allocator
 */
BR_API br_pool *BR_PUBLIC_ENTRY BrPoolAllocate(int block_size, int chunk_size, br_uint_8 mem_type);
BR_API void BR_PUBLIC_ENTRY     BrPoolFree(br_pool *pool);

BR_API void *BR_PUBLIC_ENTRY BrPoolBlockAllocate(struct br_pool *pool);
BR_API void BR_PUBLIC_ENTRY  BrPoolBlockFree(struct br_pool *pool, void *block);

BR_API void BR_PUBLIC_ENTRY BrPoolEmpty(struct br_pool *pool);

/*
 * Byte swapping
 */
BR_API br_uint_32 BR_RESIDENT_ENTRY BrSwap32(br_uint_32 l);
BR_API br_uint_16 BR_RESIDENT_ENTRY BrSwap16(br_uint_16 s);
BR_API br_float BR_RESIDENT_ENTRY   BrSwapFloat(br_float f);
BR_API void *BR_RESIDENT_ENTRY      BrSwapBlock(void *block, int count, int size);

/*
 * Misc. support
 */
typedef int BR_CALLBACK       br_qsort_cbfn(const void *, const void *);
BR_API void BR_RESIDENT_ENTRY BrQsort(void *basep, unsigned int nelems, unsigned int size, br_qsort_cbfn *comp);

typedef int BR_CALLBACK        br_bsearch_cbfn(const void *, const void *);
BR_API void *BR_RESIDENT_ENTRY BrBSearch(const void *key, const void *base, unsigned int nmemb, unsigned int size,
                                         br_bsearch_cbfn *comp);

/*
 * Diagnostic generation
 */
BR_API void BR_RESIDENT_ENTRY BrFailure(const char *s, ...);
BR_API void BR_RESIDENT_ENTRY BrWarning(const char *s, ...);
BR_API void BR_RESIDENT_ENTRY BrFatal(const char *name, int line, char *s, ...);

/*
 * Debug Break
 */
BR_API void BR_PUBLIC_ENTRY BrDebugBreak(void);

/*
 * Set new handlers
 */
BR_API br_diaghandler *BR_PUBLIC_ENTRY BrDiagHandlerSet(br_diaghandler *newdh);
BR_API br_filesystem *BR_PUBLIC_ENTRY  BrFilesystemSet(br_filesystem *newfs);
BR_API br_allocator *BR_PUBLIC_ENTRY   BrAllocatorSet(br_allocator *newal);
BR_API br_loghandler *BR_PUBLIC_ENTRY  BrLogHandlerSet(br_loghandler *newlh);

/*
 * Backwards compatibility
 */
#define BrErrorHandlerSet BrDiagHandlerSet

/*
 * Generic file IO
 */
BR_API br_uint_32 BR_PUBLIC_ENTRY BrFileAttributes(void);

BR_API void *BR_PUBLIC_ENTRY BrFileOpenRead(const char *name, br_size_t n_magics, br_mode_test_cbfn *mode_test, int *mode_result);
BR_API void *BR_PUBLIC_ENTRY     BrFileOpenWrite(const char *name, int text);
BR_API void BR_PUBLIC_ENTRY      BrFileClose(void *f);
BR_API int BR_PUBLIC_ENTRY       BrFileEof(void *f);
BR_API int BR_PUBLIC_ENTRY       BrFileGetChar(void *f);
BR_API void BR_PUBLIC_ENTRY      BrFilePutChar(int c, void *f);
BR_API br_size_t BR_PUBLIC_ENTRY BrFileRead(void *buf, br_size_t size, br_size_t n, void *f);
BR_API br_size_t BR_PUBLIC_ENTRY BrFileWrite(const void *buf, br_size_t size, br_size_t n, void *f);
BR_API br_size_t BR_PUBLIC_ENTRY BrFileGetLine(char *buf, br_size_t buf_len, void *f);
BR_API void BR_PUBLIC_ENTRY      BrFilePutLine(const char *buf, void *f);
BR_API void BR_PUBLIC_ENTRY      BrFileAdvance(long int count, void *f);
BR_API void *BR_PUBLIC_ENTRY     BrFileLoad(void *res, const char *name, br_size_t *size);

BR_API int BR_PUBLIC_ENTRY BrFilePrintf(void *f, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);

BR_API br_int_32 BR_RESIDENT_ENTRY BrSprintf(char *buf, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);
BR_API br_int_32 BR_RESIDENT_ENTRY BrSprintfN(char *buf, br_size_t buf_size, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(3, 4);

BR_API br_int_32 BR_RESIDENT_ENTRY BrSScanf(char *str, const char *fmt, ...) BR_SCANF_ATTRIBUTE(2, 3);

/*
 * Data file output type (one of BR_FS_MODE_xxx)
 */
BR_API int BR_PUBLIC_ENTRY BrWriteModeSet(int text);

/*
 * Generic memory allocation
 */
BR_API void *BR_RESIDENT_ENTRY     BrMemAllocate(br_size_t size, br_uint_8 type);
BR_API void BR_RESIDENT_ENTRY      BrMemFree(void *block);
BR_API br_size_t BR_RESIDENT_ENTRY BrMemInquire(br_uint_8 type);
BR_API br_int_32 BR_RESIDENT_ENTRY BrMemAlign(br_uint_8 type);

#if 0
BR_API void * BR_RESIDENT_ENTRY BrMemAllocateAlign(br_size_t size, br_uint_8 type, br_int_32 align);
#endif

BR_API char *BR_RESIDENT_ENTRY BrMemStrDup(const char *str);
BR_API void *BR_RESIDENT_ENTRY BrMemCalloc(int nelems, br_size_t size, br_uint_8 type);

/*
 * Resource allocation
 */
BR_API void *BR_RESIDENT_ENTRY BrResAllocate(void *vparent, br_size_t size, br_uint_8 res_class);
BR_API void BR_RESIDENT_ENTRY  BrResFree(void *vres);
BR_API void BR_RESIDENT_ENTRY  BrResFreeNoCallback(void *vres);
BR_API char *BR_RESIDENT_ENTRY BrResStrDup(void *vparent, const char *str);
BR_API char *BR_RESIDENT_ENTRY BrResVSprintf(void *vparent, const char *fmt, va_list ap);
BR_API char *BR_RESIDENT_ENTRY BrResSprintf(void *vparent, const char *fmt, ...) BR_PRINTF_ATTRIBUTE(2, 3);

BR_API void *BR_RESIDENT_ENTRY      BrResAdd(void *vparent, void *vres);
BR_API void *BR_RESIDENT_ENTRY      BrResRemove(void *vres);
BR_API br_uint_8 BR_RESIDENT_ENTRY  BrResClass(void *vres);
BR_API br_size_t BR_RESIDENT_ENTRY  BrResSize(void *vres);
BR_API br_size_t BR_RESIDENT_ENTRY  BrResSizeTotal(void *vres);
typedef br_size_t BR_CALLBACK       br_resenum_cbfn(void *vres, void *arg);
BR_API br_size_t BR_RESIDENT_ENTRY  BrResChildEnum(void *vres, br_resenum_cbfn *callback, void *arg);
BR_API br_boolean BR_RESIDENT_ENTRY BrResIsChild(void *vparent, void *vchild);

/*
 * Block operations (XXX should be made private)
 */
BR_API void BR_ASM_CALL BrBlockFill(void *dest_ptr, int value, int dwords);
BR_API void BR_ASM_CALL BrBlockCopy(void *dest_ptr, void *src_ptr, int dwords);

#if BR_HAS_FAR
BR_API void BR_ASM_CALL BrFarBlockCopy(void __far *dest_ptr, void *src_ptr, int dwords);
#endif

/*
 * Scratchpad buffer allocation - Currenty, only one allocation
 * may be outstanding at any time
 */
BR_API void *BR_RESIDENT_ENTRY     BrScratchAllocate(br_size_t size);
BR_API void BR_RESIDENT_ENTRY      BrScratchFree(void *scratch);
BR_API br_size_t BR_RESIDENT_ENTRY BrScratchInquire(void);
BR_API void BR_RESIDENT_ENTRY      BrScratchFlush(void);

/*
 * A seperate, fixed size scratch buffer, typically used as a scratch string workspace
 *
 * At least 512 bytes in size
 */
BR_API br_size_t BR_RESIDENT_ENTRY BrScratchStringSize(void);
BR_API char *BR_RESIDENT_ENTRY     BrScratchString(void);

/*
 * Error retrieval
 */
BR_API br_error BR_RESIDENT_ENTRY    BrGetLastError(void **valuep);
BR_API const char *BR_RESIDENT_ENTRY BrStrError(br_error error);

/*
 * Device mangement
 */
struct br_device;

typedef struct br_device *BR_EXPORT br_device_begin_fn(const char *arguments);

BR_API br_error BR_RESIDENT_ENTRY BrDevAdd(struct br_device **pdev, const char *image, const char *args);
BR_API br_error BR_RESIDENT_ENTRY BrDevCheckAdd(struct br_device **pdev, const char *image, const char *args);

BR_API br_error BR_RESIDENT_ENTRY BrDevAddStatic(struct br_device **pdev, br_device_begin_fn *begin, const char *args);
BR_API br_error BR_RESIDENT_ENTRY BrDevAddConfig(const char *config);
BR_API br_error BR_RESIDENT_ENTRY BrDevRemove(struct br_device *dev);

BR_API br_error BR_RESIDENT_ENTRY BrDevicesQuery(struct br_device **devices, br_int_32 *ndevices, br_int_32 max_devices);

BR_API br_error BR_RESIDENT_ENTRY BrDevFindMany(struct br_device **devices, br_int_32 *ndevices, br_int_32 max_devices,
                                                const char *pattern);
BR_API br_error BR_RESIDENT_ENTRY BrDevCount(br_int_32 *ndevices, const char *pattern);
BR_API br_error BR_RESIDENT_ENTRY BrDevFind(struct br_device **pdev, const char *pattern);

BR_API br_error BR_RESIDENT_ENTRY BrDevContainedFind(struct br_object **ph, br_token type, const char *pattern,
                                                     br_token_value *tv);
BR_API br_error BR_RESIDENT_ENTRY BrDevContainedFindMany(struct br_object **objects, br_int_32 max_objects,
                                                         br_int_32 *pnum_objects, br_token type, const char *pattern,
                                                         br_token_value *tv);
BR_API br_error BR_RESIDENT_ENTRY BrDevContainedCount(br_int_32 *pcount, br_token type, const char *pattern,
                                                      br_token_value *tv);

BR_API br_error BR_PUBLIC_ENTRY     BrDevBegin(br_pixelmap **ppm, const char *setup_string);
BR_API br_error BR_PUBLIC_ENTRY     BrDevBeginVar(br_pixelmap **ppm, const char *device_name, ...);
BR_API br_error BR_PUBLIC_ENTRY     BrDevBeginTV(br_pixelmap **ppm, const char *device_name, br_token_value *tv);
BR_API br_pixelmap *BR_PUBLIC_ENTRY BrDevBeginOld(const char *setup_string);
BR_API void BR_PUBLIC_ENTRY         BrDevEndOld(void);
BR_API void BR_PUBLIC_ENTRY         BrDevPaletteSetOld(br_pixelmap *pm);
BR_API void BR_PUBLIC_ENTRY         BrDevPaletteSetEntryOld(int i, br_colour colour);

/*
 * Callback function invoked when a device is enumerated
 */
typedef br_boolean BR_CALLBACK br_device_enum_cbfn(const char *identifer, br_uint_32 version, const char *creator,
                                                   const char *title, const char *product, const char *product_version,
                                                   void *args);

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
BR_API br_error BR_PUBLIC_ENTRY BrDeviceEnum(br_device_enum_cbfn *cbfn, void *args);
BR_API br_error BR_PUBLIC_ENTRY BrOutputFacilityEnum(const char *name, br_outfcty_enum_cbfn *cbfn, void *args);

struct br_renderer_facility;

BR_API br_error BR_RESIDENT_ENTRY BrRendererFacilityFind(struct br_renderer_facility **prf,
                                                         struct br_device_pixelmap *destination, br_token scalar_type);

BR_API br_error BR_RESIDENT_ENTRY BrRendererFacilityListFind(struct br_renderer_facility **prf, br_int_32 *num_rf,
                                                             br_int_32 max_rf, struct br_device_pixelmap *destination,
                                                             br_token scalar_type);

struct br_primitive_library;

BR_API br_error BR_RESIDENT_ENTRY BrPrimitiveLibraryFind(struct br_primitive_library **ppl,
                                                         struct br_device_pixelmap *destination, br_token scalar_type);

BR_API br_error BR_RESIDENT_ENTRY BrPrimitiveLibraryListFind(struct br_primitive_library **ppl, br_int_32 *num_pl,
                                                             br_int_32 max_pl, struct br_device_pixelmap *destination,
                                                             br_token scalar_type);

BR_API br_error BR_RESIDENT_ENTRY BrGeometryFormatFind(struct br_geometry **pgf, struct br_renderer *renderer,
                                                       struct br_renderer_facility *renderer_facility,
                                                       br_token scalar_type, br_token format_type);

/*
 * lists
 */

BR_API void BR_RESIDENT_ENTRY BrNewList(br_list *list);
BR_API void BR_RESIDENT_ENTRY BrAddHead(br_list *list, br_node *node);
BR_API void BR_RESIDENT_ENTRY BrAddTail(br_list *list, br_node *node);

BR_API br_node *BR_RESIDENT_ENTRY BrRemHead(br_list *list);
BR_API br_node *BR_RESIDENT_ENTRY BrRemTail(br_list *list);

BR_API void BR_RESIDENT_ENTRY     BrInsert(br_list *list, br_node *here, br_node *node);
BR_API br_node *BR_RESIDENT_ENTRY BrRemove(br_node *node);

BR_API void BR_RESIDENT_ENTRY            BrSimpleNewList(br_simple_list *list);
BR_API void BR_RESIDENT_ENTRY            BrSimpleAddHead(br_simple_list *list, br_simple_node *node);
BR_API br_simple_node *BR_RESIDENT_ENTRY BrSimpleRemHead(br_simple_list *list);
BR_API void BR_RESIDENT_ENTRY BrSimpleInsert(br_simple_list *list, br_simple_node *here, br_simple_node *node);
BR_API br_simple_node *BR_RESIDENT_ENTRY BrSimpleRemove(br_simple_node *node);

/*
 * Logging routines.
 */
BR_API br_uint_8 BR_RESIDENT_ENTRY BrLogSetLevel(br_uint_8 level);
BR_API br_uint_8 BR_RESIDENT_ENTRY BrLogGetLevel(void);

BR_API void BR_RESIDENT_ENTRY BrLogV(br_uint_8 level, const char *component, const char *fmt, va_list ap);
BR_API void BR_RESIDENT_ENTRY BrLog(br_uint_8 level, const char *component, const char *fmt, ...);
BR_API void BR_RESIDENT_ENTRY BrLogTrace(const char *component, const char *fmt, ...);
BR_API void BR_RESIDENT_ENTRY BrLogDebug(const char *component, const char *fmt, ...);
BR_API void BR_RESIDENT_ENTRY BrLogInfo(const char *component, const char *fmt, ...);
BR_API void BR_RESIDENT_ENTRY BrLogWarn(const char *component, const char *fmt, ...);
BR_API void BR_RESIDENT_ENTRY BrLogError(const char *component, const char *fmt, ...);

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif
