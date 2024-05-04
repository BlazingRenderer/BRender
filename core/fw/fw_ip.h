/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: fw_ip.h 1.1 1997/12/10 16:41:07 jon Exp $
 * $Locker: $
 *
 * Prototypes for functions internal to framework
 */
#ifndef _FW_IP_H_
#define _FW_IP_H_
#ifndef _NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

/*
 * pool.c
 */
void BR_ASM_CALLBACK BrPoolAddChunk(br_pool *pool);

/*
 * Debugging writes
 */
int BR_PUBLIC_ENTRY BrLogWrite(void *buffer, br_size_t s, br_size_t n);

/*
 * file.c
 */
void BR_CALLBACK _BrFileFree(void *res, br_uint_8 res_class, br_size_t size);

/*
 * token.c
 */
void     BrTokenBegin(void);
br_token BrTokenFindType(br_token *ptype, const char *base, const br_token *types, br_size_t ntypes);

/*
 * tokenval.cpp
 */
br_fixed_ls *BR_RESIDENT_ENTRY BrTVConvertFloatToFixed(br_fixed_ls **pextra, const br_float *src, br_size_t count,
                                                       br_size_t *pextra_space);

br_float *BR_RESIDENT_ENTRY BrTVConvertFixedToFloat(br_float **pextra, const br_fixed_ls *src, br_size_t count,
                                                    br_size_t *pextra_space);

br_uint_32 *BR_RESIDENT_ENTRY BrTVConvertLongCopy(br_uint_32 **pextra, const br_uint_32 *src, br_size_t count,
                                                  br_size_t *pextra_space);

br_uintptr_t *BR_RESIDENT_ENTRY BrTVConvertPtrCopy(br_uintptr_t **pextra, const br_uintptr_t *src, br_size_t count,
                                                   br_size_t *pextra_space);

/*
 * object.c
 */
void BR_CALLBACK _BrObjectFree(void *res, br_uint_8 res_class, br_size_t size);

/*
 * image.c
 */
void BR_CALLBACK _BrImageFree(void *res, br_uint_8 res_class, br_size_t size);
br_image        *ImageLoad(const char *name);

/*
 * brbhook.c
 */
void BR_CALLBACK _BrBeginHook(void);
void BR_CALLBACK _BrEndHook(void);

#ifdef __cplusplus
};
#endif
#endif
#endif
