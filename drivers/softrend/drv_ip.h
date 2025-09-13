/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: drv_ip.h 1.4 1998/11/13 16:23:35 jon Exp $
 * $Locker: $
 *
 * Prototypes for functions internal to driver
 */
#ifndef _DRV_IP_H_
#define _DRV_IP_H_

#ifndef NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

/*
 * object.c
 */
const char *BR_CMETHOD_DECL(br_object_soft, identifier)(br_object *self);
br_device  *BR_CMETHOD_DECL(br_object_soft, device)(br_object *self);

/*
 * device.c
 */
br_device *DeviceSoftAllocate(char *identifier);

/*
 * rendtype.c
 */
br_renderer_facility *RendererFacilitySoftAllocate(br_device *dev, char *identifier);

/*
 * renderer.c
 */
extern const br_token RendererPartsTokens[];

br_renderer *RendererSoftAllocate(br_device *dev, br_renderer_facility *type, br_primitive_library *prims);

/*
 * rendstat.c
 */
br_error CheckPrimitiveState(br_renderer *self);

br_error BR_CMETHOD_DECL(br_renderer_soft, partSet)(br_renderer *self, br_token part, br_int_32 index, br_token t, br_value value);
br_error BR_CMETHOD_DECL(br_renderer_soft, partSetMany)(br_renderer *self, br_token part, br_int_32 index, br_token_value *tv, br_int_32 *pcount);
br_error BR_CMETHOD_DECL(br_renderer_soft, partQuery)(br_renderer *self, br_token part, br_int_32 index, void *pvalue, br_token t);
br_error BR_CMETHOD_DECL(br_renderer_soft, partQueryBuffer)(br_renderer *self, br_token part, br_int_32 index, void *pvalue, void *buffer,
                                                            br_size_t buffer_size, br_token t);
br_error BR_CMETHOD_DECL(br_renderer_soft, partQueryMany)(br_renderer *self, br_token part, br_int_32 index, br_token_value *tv,
                                                          void *extra, br_size_t extra_size, br_int_32 *pcount);
br_error BR_CMETHOD_DECL(br_renderer_soft, partQueryManySize)(br_renderer *self, br_token part, br_int_32 index, br_size_t *pextra_size,
                                                              br_token_value *tv);
br_error BR_CMETHOD_DECL(br_renderer_soft, partQueryAll)(br_renderer *self, br_token part, br_int_32 index, br_token_value *buffer,
                                                         br_size_t buffer_size);
br_error BR_CMETHOD_DECL(br_renderer_soft, partQueryAllSize)(br_renderer *self, br_token part, br_int_32 index, br_size_t *psize);

br_error BR_CMETHOD_DECL(br_renderer_soft, partQueryCapability)(br_renderer *self, br_token part, br_int_32 index, br_token_value *buffer,
                                                                br_size_t buffer_size);

br_error BR_CMETHOD_DECL(br_renderer_soft, partIndexQuery)(br_renderer *self, br_token part, br_int_32 *pnindex);

br_error BR_CMETHOD_DECL(br_renderer_soft, stateQueryPerformance)(br_renderer *self, br_uint_32 *speed);

br_error BR_CMETHOD_DECL(br_renderer_soft, stateMask)(br_renderer *self, br_uint_32 *mask, br_token *parts, int n_parts);
br_error BR_CMETHOD_DECL(br_renderer_soft, stateDefault)(br_renderer *self, br_uint_32 mask);

br_error StateCopy(state_all *dest, state_all *src, br_uint_32 copy_mask, void *res);

br_error BR_CMETHOD_DECL(br_renderer_soft, modelMulF)(br_renderer *self, br_matrix34_f *m);
br_error BR_CMETHOD_DECL(br_renderer_soft, modelMulX)(br_renderer *self, br_matrix34_x *m);
br_error BR_CMETHOD_DECL(br_renderer_soft, modelPopPushMulF)(br_renderer *self, br_matrix34_f *m);
br_error BR_CMETHOD_DECL(br_renderer_soft, modelPopPushMulX)(br_renderer *self, br_matrix34_x *m);
br_error BR_CMETHOD_DECL(br_renderer_soft, modelInvert)(br_renderer *self);

br_error BR_CMETHOD_DECL(br_renderer_soft, boundsTestF)(br_renderer *self, br_token *r, br_bounds3_f *bounds);
br_error BR_CMETHOD_DECL(br_renderer_soft, boundsTestX)(br_renderer *self, br_token *r, br_bounds3_x *bounds);

br_error BR_CMETHOD_DECL(br_renderer_soft, statePush)(br_renderer *self, br_uint_32 mask);
br_error BR_CMETHOD_DECL(br_renderer_soft, statePop)(br_renderer *self, br_uint_32 mask);
br_error BR_CMETHOD_DECL(br_renderer_soft, stateSave)(br_renderer *self, br_renderer_state_stored *save, br_uint_32 mask);
br_error BR_CMETHOD_DECL(br_renderer_soft, stateRestore)(br_renderer *self, br_renderer_state_stored *save, br_uint_32 mask);

br_error BR_CMETHOD_DECL(br_renderer_soft, stateDefault)(br_renderer *self, br_uint_32 mask);

br_error BR_CMETHOD_DECL(br_renderer_soft, stateMask)(br_renderer *self, br_uint_32 *mask, br_token *parts, int n_parts);

/*
 * sstate.c
 */
br_renderer_state_stored *RendererStateStoredSoftAllocate(br_renderer *renderer, state_all *base_state, br_uint_32 m, br_token_value *tv);

br_error StateCopyToStored(br_renderer_state_stored *dest, state_all *src, br_uint_32 copy_mask, void *res);

br_error StateCopyFromStored(state_all *dest, br_renderer_state_stored *src, br_uint_32 copy_mask, void *res);

/*
 * state.c
 */
br_error        StateInitialise(state_all *state);
br_tv_template *FindStateTemplate(br_renderer *self, state_all **state, br_token part, br_int_32 index);
void            TemplateActions(state_all *state, br_token part, br_int_32 index, br_uint_32 mask);
void            TouchModelToView(br_renderer *self);

/*
 * genrend.c
 */
void GEOMETRY_CALL Vertex_ClearFlags(br_geometry *self, br_renderer *renderer);
void GEOMETRY_CALL ScratchFree(br_geometry *self, br_renderer *renderer);

void VertexGeometryFns(br_renderer *renderer, geometry_fn *prim_outcode);

/*
 * gprim.c
 */
br_geometry_primitives *GeometryPrimitivesAllocate(br_renderer_facility *type, char *id);

/*
 * gv1model.c
 */
br_geometry_v1_model *GeometryV1ModelAllocate(br_renderer_facility *type, char *id);

/*
 * gv1buckt.c
 */
br_geometry_v1_buckets *GeometryV1BucketsAllocate(br_renderer_facility *type, char *id);

/*
 * lighting.c
 */
br_geometry_lighting *GeometryLightingAllocate(br_renderer_facility *type, char *id);

/*
 * v1model.c
 */
br_error BR_CMETHOD_DECL(br_geometry_v1_model_soft, render)(br_geometry_v1_model *self, br_renderer *renderer, struct v11model_f *model,
                                                            br_renderer_state_stored *default_state, br_token type);

br_error BR_CMETHOD_DECL(br_geometry_v1_model_soft, renderOnScreen)(br_geometry_v1_model *self, br_renderer *renderer, struct v11model_f *model,
                                                                    br_renderer_state_stored *default_state, br_token type);

/*
 * convert.c
 */
void BR_ASM_CALL RenderConvert1(brp_block *block, brp_vertex *v0);
void BR_ASM_CALL RenderConvert2(brp_block *block, brp_vertex *v0, brp_vertex *v1);
void BR_ASM_CALL RenderConvert3(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2);
void BR_ASM_CALL RenderConvert4(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, brp_vertex *v3);

void ConvertVertex(brp_vertex *dest, brp_vertex *src);

/*
 * clip.c
 */
brp_vertex *FaceClip(br_renderer *self, brp_vertex *clip_in, br_uint_32 mask, br_uint_32 codes, int n, int *n_out);
void        ClippedRenderTriangles(br_renderer *renderer, brp_block *block, brp_vertex *cp_in, int n, br_uint_16 (*fp_vertices)[3],
                                   br_uint_16 (*fp_edges)[3]);
br_boolean  ClipLine(br_renderer *self, brp_vertex *out, brp_vertex *v0, brp_vertex *v1, br_uint_32 mask, br_uint_32 codes);
void        ClippedRenderLine(br_renderer *renderer, brp_block *block, brp_vertex *cp_in);

/*
 * setup.c
 */
br_error CacheUpdate(br_renderer *self);
void     StaticCacheUpdate_PerScene(br_renderer *self);
void     StaticCacheUpdate_PerModel(br_renderer *self);
br_error CacheClear(void);
void     ViewToModelUpdate(br_renderer *self);
void     ModelToScreenUpdate(br_renderer *self);

void ActiveLightsFind(br_renderer *self);
void ActiveLightsUpdate(br_renderer *self);

br_int_32 GenerateSurfaceFunctions(br_renderer *self, surface_fn **fns, br_uint_32 mask);

br_uint_32 ComponentMaskToSlots(br_uint_32 cm);

void GeometryFunctionReset(br_renderer *renderer);
void GeometryFunctionAdd(br_renderer *renderer, geometry_fn *fn);
void GeometryFunctionOnScreenAdd(br_renderer *renderer, geometry_fn *fn);
void GeometryFunctionBothAdd(br_renderer *renderer, geometry_fn *fn);

void PrimBlockReset(br_renderer *renderer);
void PrimBlockAdd(br_renderer *renderer, brp_render_fn *fn);
void PrimBlockOnScreenAdd(br_renderer *renderer, brp_render_fn *fn);
void PrimBlockAddBoth(br_renderer *renderer, brp_render_fn *fn);

/*
 * onscreen.c
 */
br_token OnScreenCheck(br_renderer *self, br_matrix4 *model_to_screen, br_bounds3 *bounds);

/*
 * light8.c
 */
void SURFACE_CALL SurfaceIndexZero(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour gcolour, br_scalar *comp);
void SURFACE_CALL SurfaceIndexUnlit(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour gcolour, br_scalar *comp);
void SURFACE_CALL SurfaceIndexLit(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour gcolour, br_scalar *comp);
void SURFACE_CALL SurfaceIndexLitSimple(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour gcolour, br_scalar *comp);
void ActiveLightAccumulateIndexSet(struct active_light *alp);

void SURFACE_CALL SurfaceIndexLit1MD(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);

/*
 * light24.c
 */
void SURFACE_CALL SurfaceColourZero(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour gcolour, br_scalar *comp);
void SURFACE_CALL SurfaceColourUnlit(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour gcolour, br_scalar *comp);
void SURFACE_CALL SurfaceColourLit(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour gcolour, br_scalar *comp);
void SURFACE_CALL SurfaceColourLitPrelit(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour gcolour, br_scalar *comp);
void SURFACE_CALL SurfaceColourLitSimple(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour gcolour, br_scalar *comp);
void ActiveLightAccumulateColourSet(struct active_light *alp);

/*
 * mapping.c
 */
void SURFACE_CALL SurfaceMapEnvironmentInfinite(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *normal, br_colour colour,
                                                br_scalar *comp);
void SURFACE_CALL SurfaceMapEnvironmentLocal(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *normal, br_colour colour,
                                             br_scalar *comp);
void SURFACE_CALL SurfaceMapGeometryX(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);
void SURFACE_CALL SurfaceMapGeometryY(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);
void SURFACE_CALL SurfaceMapGeometryZ(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);
void SURFACE_CALL SurfaceMapGeometryMap(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);
void SURFACE_CALL SurfaceMapGeometryMapScale(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);
void SURFACE_CALL SurfaceMapGeometryMapScaleTranslate(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour,
                                                      br_scalar *comp);
void SURFACE_CALL SurfaceMapGeometryMapCopy(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);
void SURFACE_CALL SurfaceMapGeometryMapShift(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);

void BR_ASM_CALL OpTriangleMapQuad(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                   br_uint_16 (*fp_edges)[3], br_vector4 *fp_eqn, struct temp_face *tfp);

/*
 * alpha.c
 */
void SURFACE_CALL SurfaceAlpha(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);
void SURFACE_CALL SurfaceAlphaPrealpha(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);

/*
 * depth.c
 */
void SURFACE_CALL SurfaceLinearDepth(br_renderer *self, br_vector3 *p, br_vector2 *map, br_vector3 *n, br_colour colour, br_scalar *comp);

/*
 * faceops.c
 */
void BR_ASM_CALL OpTriangleClip(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                br_uint_16 (*fp_edges)[3], br_vector4 *fp_eqn, struct temp_face *tfp);
void BR_ASM_CALL OpTriangleClipConstantSurf(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                            br_uint_16 (*fp_edges)[3], br_vector4 *fp_eqn, struct temp_face *tfp);
void BR_ASM_CALL OpTriangleConstantSurf(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                        br_uint_16 (*fp_edges)[3], br_vector4 *fp_eqn, struct temp_face *tfp);
void BR_ASM_CALL OpTriangleTwoSidedConstantSurf(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                                br_uint_16 (*fp_edges)[3], br_vector4 *fp_eqn, struct temp_face *tfp);

void BR_ASM_CALL OpTriangleMappingWrapFix(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                          br_uint_16 (*fp_edges)[3], br_vector4 *fp_eqn, struct temp_face *tfp);

void BR_ASM_CALL OpTriangleRelightTwoSided(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                           br_uint_16 (*fp_edges)[3], br_vector4 *fp_eqn, struct temp_face *tfp);

void BR_ASM_CALL OpTriangleToLines(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                   br_uint_16 (*fp_edges)[3]);
void BR_ASM_CALL OpTriangleReplicateConstant(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                             br_uint_16 (*fp_edges)[3]);
void BR_ASM_CALL OpTriangleReplicateConstantI(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2,
                                              br_uint_16 (*fp_vertices)[3], br_uint_16 (*fp_edges)[3]);
void BR_ASM_CALL OpTriangleReplicateConstantRGB(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2,
                                                br_uint_16 (*fp_vertices)[3], br_uint_16 (*fp_edges)[3]);
void BR_ASM_CALL OpTriangleToPoints(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3]);
void BR_ASM_CALL OpTriangleToPoints_OS(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3]);
void BR_ASM_CALL OpLineClip(brp_block *block, brp_vertex *v0, brp_vertex *v1);

void BR_ASM_CALL OpTriangleSubdivide(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                     br_uint_16 (*fp_edges)[3], br_vector4 *fp_eqn, struct temp_face *tfp);

void BR_ASM_CALL OpTriangleSubdivideOnScreen(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2, br_uint_16 (*fp_vertices)[3],
                                             br_uint_16 (*fp_edges)[3], br_vector4 *fp_eqn, struct temp_face *tfp);

void SubdivideSetThreshold(br_int_32 subdivide_tolerance);

/*
 * heap.c
 */
void BR_ASM_CALL OpHeapAddPoint(brp_block *block, brp_vertex *v0);
void BR_ASM_CALL OpHeapAddLine(brp_block *block, brp_vertex *v0, brp_vertex *v1);
void BR_ASM_CALL OpHeapAddTriangle(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2);

void BR_ASM_CALL OpHeapAddPointConvert(brp_block *block, brp_vertex *v0);
void BR_ASM_CALL OpHeapAddLineConvert(brp_block *block, brp_vertex *v0, brp_vertex *v1);
void BR_ASM_CALL OpHeapAddTriangleConvert(brp_block *block, brp_vertex *v0, brp_vertex *v1, brp_vertex *v2);

/*
 * Platform dependant downcoded geometry support
 */
#ifdef __386__
#endif

/* Rampant optimism ...
 */

#ifdef __MIPS__
#endif

#ifdef __ALPHA__
#endif

#ifdef __PPC__
#endif

#ifdef __680X0__
#endif

#ifdef __cplusplus
};
#endif

#endif
#endif
