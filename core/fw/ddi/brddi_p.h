/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brddi_p.h 1.3 1998/11/12 13:14:55 johng Exp $
 * $Locker: $
 *
 * Prototypes for functions that are exposed to device drivers
 */
#ifndef _BRDDI_P_H_
#define _BRDDI_P_H_
#ifndef _NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

BR_API br_boolean BR_RESIDENT_ENTRY BrNamePatternMatch(const char *p, const char *s);

/*
 * Debugging printf
 */
BR_API int BR_RESIDENT_ENTRY BrLogPrintf(const char *fmt, ...);

/*
 * resource.c
 */
BR_API br_uint_32 BR_RESIDENT_ENTRY BrResCheck(void *vres, int no_tag);
BR_API void BR_RESIDENT_ENTRY       BrResDump(void *vres, br_putline_cbfn *putline, void *arg);

/*
 * brexcept.c
 */
BR_API br_exception_handler *BR_RESIDENT_ENTRY _BrExceptionBegin(void);
BR_API void BR_RESIDENT_ENTRY                  _BrExceptionEnd(void);
BR_API void BR_RESIDENT_ENTRY                  _BrExceptionThrow(br_int_32 type, void *value);
BR_API br_exception BR_RESIDENT_ENTRY          _BrExceptionValueFetch(br_exception type, void **evp);
BR_API void *BR_RESIDENT_ENTRY                 _BrExceptionResource(void);

/*
 * error.c
 */
BR_API void BR_RESIDENT_ENTRY     BrLastErrorSet(br_error type, void *value);
BR_API br_error BR_RESIDENT_ENTRY BrLastErrorGet(void **valuep);

/*
 * token.c
 */
BR_API br_token BR_RESIDENT_ENTRY  BrTokenCreate(const char *identifier, br_token type);
BR_API char *BR_RESIDENT_ENTRY     BrTokenIdentifier(br_token t);
BR_API br_token BR_RESIDENT_ENTRY  BrTokenType(br_token t);
BR_API br_int_32 BR_RESIDENT_ENTRY BrTokenCount(const char *pattern);
BR_API br_size_t BR_RESIDENT_ENTRY BrTokenSize(br_token t);
BR_API br_token BR_RESIDENT_ENTRY  BrTokenFind(const char *pattern);
BR_API br_int_32 BR_RESIDENT_ENTRY BrTokenFindMany(const char *pattern, br_token *tokens, br_int_32 max_tokens);

/*
 * tokenval.c
 */
BR_API br_tv_template *BR_RESIDENT_ENTRY BrTVTemplateAllocate(void *res, br_tv_template_entry *entries, int n_entries);

BR_API void BR_RESIDENT_ENTRY BrTVTemplateFree(br_tv_template *t);

BR_API br_error BR_RESIDENT_ENTRY BrTokenValueQuery(void *pvalue, void *extra, br_size_t extra_size, br_token t,
                                                    void *block, br_tv_template *_template);

BR_API br_error BR_RESIDENT_ENTRY BrTokenValueQueryMany(br_token_value *tv, void *extra, br_size_t extra_size,
                                                        br_int_32 *pcount, void *block, br_tv_template *_template);

BR_API br_error BR_RESIDENT_ENTRY BrTokenValueQueryManySize(br_size_t *psize, br_token_value *tv, void *block,
                                                            br_tv_template *_template);

BR_API br_error BR_RESIDENT_ENTRY BrTokenValueQueryAll(br_token_value *buffer, br_size_t buffer_size, void *block,
                                                       br_tv_template *_template);

BR_API br_error BR_RESIDENT_ENTRY BrTokenValueQueryAllSize(br_size_t *psize, void *block, br_tv_template *_template);

/* NB: value doesn't have to be a pointer. A pointer just happens to be large enough.
 * It should really be br_value. */
BR_API br_error BR_RESIDENT_ENTRY BrTokenValueSet(void *mem, br_uint_32 *pcombined_mask, br_token t, br_value value,
                                                  br_tv_template *_template);

BR_API br_error BR_RESIDENT_ENTRY BrTokenValueSetMany(void *mem, br_int_32 *pcount, br_uint_32 *pcombined_mask,
                                                      br_token_value *tv, br_tv_template *_template);

BR_API void BR_RESIDENT_ENTRY BrTokenValueDump(br_token_value *tv, const char *prefix, br_putline_cbfn *putline, void *arg);
BR_API br_error BR_RESIDENT_ENTRY BrStringToTokenValue(br_token_value *buffer, br_size_t buffer_size, const char *str);
BR_API br_boolean BR_RESIDENT_ENTRY BrTokenValueCompare(br_token_value *tv1, br_token_value *tv2);
BR_API br_boolean BR_RESIDENT_ENTRY BrTokenValueComparePartial(br_token_value *tv1, br_token_value *tv2,
                                                               const br_token *insignificant);

BR_API br_value BR_RESIDENT_ENTRY BrTokenValueVaArg(br_token token, va_list *ap);

/*
 * diag.c
 */
BR_API void BR_RESIDENT_ENTRY _BrAssert(const char *condition, const char *file, unsigned line);
BR_API void BR_RESIDENT_ENTRY _BrUAssert(const char *condition, const char *file, unsigned line);

/*
 * lexer.c
 */
BR_API struct br_lexer *BR_RESIDENT_ENTRY     BrLexerAllocate(const struct br_lexer_keyword *keywords, int nkeywords);
BR_API void BR_RESIDENT_ENTRY                 BrLexerFree(struct br_lexer *l);
BR_API char BR_RESIDENT_ENTRY                 BrLexerCommentSet(struct br_lexer *l, char eol_comment);
BR_API br_lexer_error_cbfn *BR_RESIDENT_ENTRY BrLexerErrorSet(struct br_lexer *l, br_lexer_error_cbfn *error);
BR_API br_error BR_RESIDENT_ENTRY             BrLexerPushFile(struct br_lexer *l, const char *file);
BR_API br_error BR_RESIDENT_ENTRY             BrLexerPushString(struct br_lexer *l, const char *string, char *name);
BR_API struct br_lexer_source *BR_RESIDENT_ENTRY BrLexerPop(struct br_lexer *l);
BR_API br_error BR_RESIDENT_ENTRY BrLexerDumpSet(struct br_lexer *l, br_putline_cbfn *putline, void *putline_arg);
BR_API void BR_RESIDENT_ENTRY     BrLexerTokenError(struct br_lexer *l, br_lexer_token_id t);
BR_API void BR_RESIDENT_ENTRY     BrLexerPosition(struct br_lexer *l, char *buf, br_size_t buf_size);

BR_API br_int_32 BR_RESIDENT_ENTRY   BrParseInteger(struct br_lexer *l);
BR_API br_float BR_RESIDENT_ENTRY    BrParseFloat(struct br_lexer *l);
BR_API br_fixed_ls BR_RESIDENT_ENTRY BrParseFixed(struct br_lexer *l);

BR_API br_int_32 BR_RESIDENT_ENTRY BrParseVectorFloat(struct br_lexer *l, br_float *v, br_int_32 max);
BR_API br_int_32 BR_RESIDENT_ENTRY BrParseVectorFixed(struct br_lexer *l, br_fixed_ls *v, br_int_32 max);

BR_API br_int_32 BR_RESIDENT_ENTRY BrParseMatrixFloat(struct br_lexer *l, br_float *m, br_int_32 width, br_int_32 max_h);
BR_API br_int_32 BR_RESIDENT_ENTRY BrParseMatrixFixed(struct br_lexer *l, br_fixed_ls *m, br_int_32 width, br_int_32 max_h);

#define BrParseScalar BrParseFloat
#define BrParseVector BrParseVectorFloat
#define BrParseMatrix BrParseMatrixFloat

/*
 * object.c
 */
BR_API br_error BR_CMETHOD_DECL(br_object, query)(struct br_object *self, void *pvalue, br_token t);
BR_API br_error BR_CMETHOD_DECL(br_object, queryBuffer)(struct br_object *self, void *pvalue, void *buffer,
                                                        br_size_t buffer_size, br_token t);
BR_API br_error BR_CMETHOD_DECL(br_object, queryMany)(struct br_object *self, br_token_value *tv, void *extra,
                                                      br_size_t extra_size, br_int_32 *pcount);
BR_API br_error BR_CMETHOD_DECL(br_object, queryManySize)(struct br_object *self, br_size_t *pextra_size, br_token_value *tv);
BR_API br_error BR_CMETHOD_DECL(br_object, queryAll)(struct br_object *self, br_token_value *buffer, br_size_t buffer_size);
BR_API br_error BR_CMETHOD_DECL(br_object, queryAllSize)(struct br_object *self, br_size_t *psize);

/*
 * objectc.c
 */
BR_API void *BR_RESIDENT_ENTRY BrObjectListAllocate(void *res);
BR_API br_error BR_CMETHOD_DECL(br_object_container, addFront)(struct br_object_container *self, struct br_object *ph);
BR_API br_error BR_CMETHOD_DECL(br_object_container, removeFront)(struct br_object_container *self, struct br_object **ph);
BR_API br_error BR_CMETHOD_DECL(br_object_container, remove)(struct br_object_container *self, struct br_object *ph);
BR_API br_error BR_CMETHOD_DECL(br_object_container, find)(struct br_object_container *self, struct br_object **ph,
                                                           br_token type, const char *pattern, br_token_value *tv);
BR_API br_error BR_CMETHOD_DECL(br_object_container, findMany)(struct br_object_container *self, struct br_object **objects,
                                                               br_int_32 max_objects, br_int_32 *num_objects,
                                                               br_token type, const char *pattern, br_token_value *tv);
BR_API br_error BR_CMETHOD_DECL(br_object_container, count)(struct br_object_container *self, br_int_32 *pcount,
                                                            br_token type, const char *pattern, br_token_value *tv);

BR_API void *BR_CMETHOD_DECL(br_object_container, tokensMatchBegin)(struct br_object_container *self, br_token t,
                                                                    br_token_value *tv);
BR_API br_boolean BR_CMETHOD_DECL(br_object_container, tokensMatch)(struct br_object_container *self,
                                                                    struct br_object *h, void *arg);
BR_API void BR_CMETHOD_DECL(br_object_container, tokensMatchEnd)(struct br_object_container *self, void *arg);
BR_API const br_tv_match_info *BR_CMETHOD_DECL(br_object_container, tokensMatchInfoQuery)(struct br_object_container *self);

BR_API br_error BR_RESIDENT_ENTRY BrObjectContainerFree(struct br_object_container *self, br_token type, char *pattern,
                                                        br_token_value *tv);

/*
 * image.c
 */
BR_API struct br_image *BR_RESIDENT_ENTRY BrImageReference(const char *name);
BR_API void BR_RESIDENT_ENTRY             BrImageDereference(struct br_image *image);
BR_API void *BR_RESIDENT_ENTRY            BrImageLookupName(struct br_image *img, char *name, br_uint_32 hint);
BR_API void *BR_RESIDENT_ENTRY            BrImageLookupOrdinal(struct br_image *img, br_uint_32 ordinal);

BR_API br_boolean BR_RESIDENT_ENTRY BrImageAdd(br_image *ing);
BR_API br_boolean BR_RESIDENT_ENTRY BrImageRemove(br_image *img);

/*
 * Registry lists
 */
BR_API void *BrRegistryClear(br_registry *reg);
BR_API int   BrRegistryAddMany(br_registry *reg, void **item, int n);
BR_API int   BrRegistryRemoveMany(br_registry *reg, void **item, int n);
BR_API void *BrRegistryFind(br_registry *reg, const char *pattern);
BR_API int   BrRegistryFindMany(br_registry *reg, const char *pattern, void **item, int n);
BR_API int   BrRegistryCount(br_registry *reg, const char *pattern);
BR_API int   BrRegistryEnum(br_registry *reg, const char *pattern, br_enum_cbfn *callback, void *arg);

BR_API void *BrRegistryNew(br_registry *reg);
BR_API void *BrRegistryAdd(br_registry *reg, void *item);
BR_API void *BrRegistryRemove(br_registry *reg, void *item);

BR_API void *BrRegistryNewStatic(br_registry *reg, br_registry_entry *base, int limit);
BR_API void *BrRegistryAddStatic(br_registry *reg, br_registry_entry *base, void *item);
BR_API void *BrRegistryRemoveStatic(br_registry *reg, void *item);

/*
 * Nasty hack for finding out the last pixelmap that DevBegin() was called with
 */
BR_API br_pixelmap *BrDevLastBeginQuery(void);
BR_API void         BrDevLastBeginSet(br_pixelmap *pm);

#ifdef __cplusplus
};
#endif
#endif
#endif
