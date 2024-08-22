/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: pm_ip.h 1.3 1998/09/02 13:41:56 johng Exp $
 * $Locker: $
 *
 * Prototypes for functions internal to pixelmap support
 */
#ifndef _PM_IP_H_
#define _PM_IP_H_
#ifndef NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

/*
 * pixelmap.c
 */
br_uint_16 BR_RESIDENT_ENTRY BrPixelmapFileSize(br_pixelmap *pm);

/*
 * pmmem.c
 */
struct br_device_pixelmap *DevicePixelmapMemAllocate(br_uint_8 type, br_uint_16 w, br_uint_16 h, void *pixels, int flags);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, validSource)(struct br_device_pixelmap *self, br_boolean *bp, br_object *h);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, resize)(struct br_device_pixelmap *self, br_int_32 width, br_int_32 height);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, match)(struct br_device_pixelmap  *self,
                                                        struct br_device_pixelmap **newpm, br_token_value *tv);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, allocateSub)(struct br_device_pixelmap  *self,
                                                              struct br_device_pixelmap **newpm, br_rectangle *rect);

/*
 * Operations on whole pixemap (with versions that include a hint dirty rectangle)
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyTo)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyFrom)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, fill)(struct br_device_pixelmap *self, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, doubleBuffer)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyDirty)(struct br_device_pixelmap *self, struct br_device_pixelmap *src,
                                                            br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyToDirty)(struct br_device_pixelmap *self, struct br_device_pixelmap *src,
                                                              br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyFromDirty)(struct br_device_pixelmap *self,
                                                                struct br_device_pixelmap *src, br_rectangle *dirty,
                                                                br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, fillDirty)(struct br_device_pixelmap *self, br_uint_32 colour,
                                                            br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, doubleBufferDirty)(struct br_device_pixelmap *self,
                                                                    struct br_device_pixelmap *src, br_rectangle *dirty,
                                                                    br_int_32 num_rects);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangle)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                            br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangle2)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                             br_uint_32 colour_tl, br_uint_32 colour_br);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleCopyTo)(struct br_device_pixelmap *self, br_point *p,
                                                                  struct br_device_pixelmap *mem, br_rectangle *r);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleCopyFrom)(struct br_device_pixelmap *self, br_point *p,
                                                                    struct br_device_pixelmap *mem, br_rectangle *r);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleStretchCopyTo)(struct br_device_pixelmap *self, br_rectangle *d,
                                                                         struct br_device_pixelmap *mem, br_rectangle *s);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleStretchCopyFrom)(struct br_device_pixelmap *self, br_rectangle *d,
                                                                           struct br_device_pixelmap *mem, br_rectangle *s);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rectangleFill)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                                br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, pixelSet)(struct br_device_pixelmap *self, br_point *point, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, line)(struct br_device_pixelmap *self, br_point *s, br_point *e,
                                                       br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, copyBits)(struct br_device_pixelmap *self, br_point *point,
                                                           const br_uint_8 *src, br_uint_16 s_stride,
                                                           br_rectangle *bit_rect, br_uint_32 colour);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rowSize)(struct br_device_pixelmap *self, br_size_t *sizep);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rowQuery)(struct br_device_pixelmap *self, void *buffer,
                                                           br_size_t buffer_size, br_uint_32 row);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, rowSet)(struct br_device_pixelmap *self, void *buffer,
                                                         br_size_t buffer_size, br_uint_32 row);

/*
 * Reading back
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, pixelQuery)(struct br_device_pixelmap *self, br_uint_32 *pcolour,
                                                             br_point *point);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, pixelAddressQuery)(struct br_device_pixelmap *self, void **pptr,
                                                                    br_uint_32 *pqual, br_point *point);

/*
 * Updating the pixelmap
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, pixelAddressSet)(struct br_device_pixelmap *self, void *ptr, br_uint_32 *qual);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, originSet)(struct br_device_pixelmap *self, br_point *p);

/*
 * General information
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, wordBytesQuery)(struct br_device_pixelmap *self, br_size_t *bytes);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, pixelBitsQuery)(struct br_device_pixelmap *self, br_uint_32 *pbits);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, channelsQuery)(struct br_device_pixelmap *self, br_uint_32 *pchannels);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, flush)(struct br_device_pixelmap *self);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, synchronise)(struct br_device_pixelmap *self, br_token sync_type,
                                                              br_boolean block);

br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, directLock)(struct br_device_pixelmap *self, br_boolean block);
br_error BR_CMETHOD_DECL(br_device_pixelmap_mem, directUnlock)(struct br_device_pixelmap *self);

/*
 * pmgen.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, copy)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, copyTo)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, copyFrom)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, fill)(struct br_device_pixelmap *self, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, doubleBuffer)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, copyDirty)(struct br_device_pixelmap *self, struct br_device_pixelmap *src,
                                                            br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, copyToDirty)(struct br_device_pixelmap *self, struct br_device_pixelmap *src,
                                                              br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, copyFromDirty)(struct br_device_pixelmap *self,
                                                                struct br_device_pixelmap *src, br_rectangle *dirty,
                                                                br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, fillDirty)(struct br_device_pixelmap *self, br_uint_32 colour,
                                                            br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, doubleBufferDirty)(struct br_device_pixelmap *self,
                                                                    struct br_device_pixelmap *src, br_rectangle *dirty,
                                                                    br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, rectangle)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                            br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, rectangle2)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                             br_uint_32 colour_tl, br_uint_32 colour_br);

br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, line)(struct br_device_pixelmap *self, br_point *s, br_point *e,
                                                       br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, text)(struct br_device_pixelmap *self, br_point *point, br_font *font,
                                                       const char *text, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, textBounds)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                             br_font *font, const char *text);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, copyBits)(struct br_device_pixelmap *self, br_point *point,
                                                           const br_uint_8 *src, br_uint_16 s_stride,
                                                           br_rectangle *bit_rect, br_uint_32 colour);

br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, originSet)(struct br_device_pixelmap *self, br_point *p);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, flush)(struct br_device_pixelmap *self);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, synchronise)(struct br_device_pixelmap *self, br_token sync_type,
                                                              br_boolean block);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, directLock)(struct br_device_pixelmap *self, br_boolean block);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, directUnlock)(struct br_device_pixelmap *self);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, getControls)(struct br_device_pixelmap *self, br_display_controls *controls);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, setControls)(struct br_device_pixelmap *self, br_display_controls *controls);
br_error BR_CMETHOD_DECL(br_device_pixelmap_gen, handleWindowEvent)(struct br_device_pixelmap *self, void *arg);

/*
 * pmnull.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, validSource)(struct br_device_pixelmap *self, br_boolean *bp, br_object *h);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, resize)(struct br_device_pixelmap *self, br_int_32 width, br_int_32 height);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, copyTo)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, copyFrom)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, fill)(struct br_device_pixelmap *self, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, doubleBuffer)(struct br_device_pixelmap *self,
                                                                struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, copyDirty)(struct br_device_pixelmap *self, struct br_device_pixelmap *src,
                                                             br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, copyToDirty)(struct br_device_pixelmap *self, struct br_device_pixelmap *src,
                                                               br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, copyFromDirty)(struct br_device_pixelmap *self,
                                                                 struct br_device_pixelmap *src, br_rectangle *dirty,
                                                                 br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, fillDirty)(struct br_device_pixelmap *self, br_uint_32 colour,
                                                             br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, doubleBufferDirty)(struct br_device_pixelmap *self,
                                                                     struct br_device_pixelmap *src,
                                                                     br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, rectangleCopyTo)(struct br_device_pixelmap *self, br_point *p,
                                                                   struct br_device_pixelmap *src, br_rectangle *r);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, rectangleCopyFrom)(struct br_device_pixelmap *self, br_point *p,
                                                                     struct br_device_pixelmap *mem, br_rectangle *r);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, rectangleStretchCopyTo)(struct br_device_pixelmap *self, br_rectangle *d,
                                                                          struct br_device_pixelmap *mem, br_rectangle *s);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, rectangleStretchCopyFrom)(struct br_device_pixelmap *self, br_rectangle *d,
                                                                            struct br_device_pixelmap *mem, br_rectangle *s);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, rectangleFill)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                                 br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, pixelSet)(struct br_device_pixelmap *self, br_point *p, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, copyBits)(struct br_device_pixelmap *self, br_point *point,
                                                            const br_uint_8 *src, br_uint_16 s_stride,
                                                            br_rectangle *bit_rect, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, pixelQuery)(struct br_device_pixelmap *self, br_uint_32 *pcolour,
                                                              br_point *p);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, pixelAddressQuery)(struct br_device_pixelmap *self, void **pptr,
                                                                     br_uint_32 *pqual, br_point *p);
br_error BR_CMETHOD_DECL(br_device_pixelmap_null, pixelAddressSet)(struct br_device_pixelmap *self, void *ptr, br_uint_32 *qual);

/*
 * pmfail.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, allocateSub)(struct br_device_pixelmap  *self,
                                                               struct br_device_pixelmap **newpm, br_rectangle *rect);

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copy)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyTo)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyFrom)(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, fill)(struct br_device_pixelmap *self, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, doubleBuffer)(struct br_device_pixelmap *self,
                                                                struct br_device_pixelmap *src);

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyDirty)(struct br_device_pixelmap *self, struct br_device_pixelmap *src,
                                                             br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyToDirty)(struct br_device_pixelmap *self, struct br_device_pixelmap *src,
                                                               br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyFromDirty)(struct br_device_pixelmap *self,
                                                                 struct br_device_pixelmap *src, br_rectangle *dirty,
                                                                 br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, fillDirty)(struct br_device_pixelmap *self, br_uint_32 colour,
                                                             br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, doubleBufferDirty)(struct br_device_pixelmap *self,
                                                                     struct br_device_pixelmap *src,
                                                                     br_rectangle *dirty, br_int_32 num_rects);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangle)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                             br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangle2)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                              br_uint_32 colour_tl, br_uint_32 colour_br);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangleCopyTo)(struct br_device_pixelmap *self, br_point *p,
                                                                   struct br_device_pixelmap *src, br_rectangle *r);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangleCopyFrom)(struct br_device_pixelmap *self, br_point *p,
                                                                     struct br_device_pixelmap *dest, br_rectangle *r);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail,
                         rectangleStretchCopyTo)(struct br_device_pixelmap *self, br_rectangle *destinationRectangle,
                                                 struct br_device_pixelmap *src, br_rectangle *sourceRectangle);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail,
                         rectangleStretchCopyFrom)(struct br_device_pixelmap *_self, br_rectangle *d,
                                                   struct br_device_pixelmap *dest, br_rectangle *s);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangleFill)(struct br_device_pixelmap *self, br_rectangle *rect,
                                                                 br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelSet)(struct br_device_pixelmap *self, br_point *p, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, line)(struct br_device_pixelmap *self, br_point *s, br_point *e,
                                                        br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyBits)(struct br_device_pixelmap *_self, br_point *point,
                                                            const br_uint_8 *src, br_uint_16 s_stride,
                                                            br_rectangle *bit_rect, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, text)(struct br_device_pixelmap *self, br_point *point, br_font *font,
                                                        const char *text, br_uint_32 colour);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rowSize)(struct br_device_pixelmap *self, br_size_t *sizep);

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rowQuery)(struct br_device_pixelmap *self, void *buffer,
                                                            br_size_t buffer_size, br_uint_32 row);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rowSet)(struct br_device_pixelmap *self, void *buffer,
                                                          br_size_t buffer_size, br_uint_32 row);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelQuery)(struct br_device_pixelmap *self, br_uint_32 *pcolour,
                                                              br_point *p);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelAddressQuery)(struct br_device_pixelmap *self, void **pptr,
                                                                     br_uint_32 *pqual, br_point *p);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelAddressSet)(struct br_device_pixelmap *self, void *ptr, br_uint_32 *qual);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, flush)(struct br_device_pixelmap *self);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, synchronise)(struct br_device_pixelmap *self, br_token sync_type,
                                                               br_boolean block);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, directLock)(struct br_device_pixelmap *self, br_boolean block);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, directUnlock)(struct br_device_pixelmap *self);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, getControls)(struct br_device_pixelmap *self, br_display_controls *controls);
br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, setControls)(struct br_device_pixelmap *self, br_display_controls *controls);

/*
 * gencopy.c
 */
br_error GeneralRectangleCopy(struct br_device_pixelmap *dest, br_point *p, struct br_device_pixelmap *src, br_rectangle *r);

br_error GeneralCopy(struct br_device_pixelmap *dst, struct br_device_pixelmap *src);

br_error GeneralRectangleStretchCopy(struct br_device_pixelmap *dst, br_rectangle *d, struct br_device_pixelmap *src,
                                     br_rectangle *s);

/*
 * genclip.c
 */
br_clip_result BR_RESIDENT_ENTRY PixelmapPointClip(br_point *out, const br_point *in, const br_pixelmap *pm);
br_clip_result BR_RESIDENT_ENTRY PixelmapLineClip(br_point *s_out, br_point *e_out, const br_point *s_in,
                                                  const br_point *e_in, const br_pixelmap *pm);
br_clip_result BR_RESIDENT_ENTRY PixelmapRectangleClip(br_rectangle *out, const br_rectangle *in, const br_pixelmap *pm);
br_clip_result BR_RESIDENT_ENTRY PixelmapRectangleClipTwo(br_rectangle *r_out, br_point *p_out,
                                                          const br_rectangle *r_in, const br_point *p_in,
                                                          const br_pixelmap *pm_dst, const br_pixelmap *pm_src);
br_clip_result BR_RESIDENT_ENTRY PixelmapRectanglesClipTwo(br_rectangle *s_out, br_rectangle *d_out,
                                                           const br_rectangle *s_in, const br_rectangle *d_in,
                                                           const br_pixelmap *pm_dst, const br_pixelmap *pm_src);
br_clip_result BR_RESIDENT_ENTRY PixelmapCopyBitsClip(br_rectangle *r_out, br_point *p_out, const br_rectangle *r_in,
                                                      const br_point *p_in, const br_pixelmap *pm_dst);

#include "memloops.h"

/*
 * memloops.asm
 */
br_uint_16 BR_ASM_CALL _GetSysQual(void);

/*
 * pixelmap.c
 */
br_uint_16 BR_RESIDENT_ENTRY BrPixelmapFileSize(br_pixelmap *pm);

/*
 * pmdspth.c
 */
br_error DispatchCopy(struct br_device_pixelmap *self, struct br_device_pixelmap *src);
br_error DispatchRectangleCopy(struct br_device_pixelmap *self, struct br_point *p, struct br_device_pixelmap *src,
                               struct br_rectangle *r);
br_error DispatchRectangleStretchCopy(struct br_device_pixelmap *self, struct br_rectangle *d,
                                      struct br_device_pixelmap *src, struct br_rectangle *s);

/*
 * mipsupt.c
 */
br_size_t BrPixelmapMipMemorySize(int dimension, br_uint_8 type);

/*
 * pmsetup.c
 */

struct br_pixelmap_state *BR_RESIDENT_ENTRY PixelmapState(void);

#ifdef __cplusplus
};
#endif
#endif
#endif
