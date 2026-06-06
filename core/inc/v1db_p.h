/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: v1db_p.h 1.8 1998/11/12 13:19:16 johng Exp $
 * $Locker: $
 *
 * Public function prototypes for BRender V1 database
    Last change:  TN    9 Apr 97    4:40 pm
 */
#ifndef _V1DB_P_H_
#define _V1DB_P_H_

/*
 * Setup - overide framework BrBegin/End with our own wrappers
 */
#define BrBegin         BrV1dbBeginWrapper
#define BrEnd           BrV1dbEndWrapper

#define BR_BOUNDS_MIN_X 0
#define BR_BOUNDS_MIN_Y 1
#define BR_BOUNDS_MAX_X 2
#define BR_BOUNDS_MAX_Y 3

/*
 * Callback function invoked when an actor is
 * rendered
 */
typedef void BR_CALLBACK br_renderbounds_cbfn(br_actor *actor, br_model *model, br_material *material, void *render_data, br_uint_8 style,
                                              br_matrix4 *model_to_screen, br_int_32 bounds[4]);

/*
 * Callback function invoked when a z-sort primitive is generated
 */
typedef void BR_CALLBACK br_primitive_cbfn(br_primitive *primitive, br_actor *actor, br_model *model, br_material *material,
                                           br_order_table *order_table, br_scalar *z);

#ifndef _NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

br_error BR_PUBLIC_ENTRY BrV1dbBeginWrapper(void);
br_error BR_PUBLIC_ENTRY BrV1dbEndWrapper(void);

br_error BR_PUBLIC_ENTRY            BrV1dbRendererBegin(struct br_device_pixelmap *destination, struct br_renderer *renderer);
struct br_renderer *BR_PUBLIC_ENTRY BrV1dbRendererQuery(void);
br_error BR_PUBLIC_ENTRY            BrV1dbRendererEnd(void);

/*
 * XXX - All the Add/Remove/Find/Count/Enum calls could ( when !(DEBUG | PARANOID))
 * be #defined in terms of the core Registry fns. with appropriate casts.
 */
/*
 * Material Handling
 */
br_material *BR_PUBLIC_ENTRY           BrMaterialAdd(br_material *material);
br_material *BR_PUBLIC_ENTRY           BrMaterialRemove(br_material *material);
br_material *BR_PUBLIC_ENTRY           BrMaterialFind(const char *pattern);
typedef br_material *BR_CALLBACK       br_material_find_cbfn(const char *name);
br_material_find_cbfn *BR_PUBLIC_ENTRY BrMaterialFindHook(br_material_find_cbfn *hook);
br_uint_32 BR_PUBLIC_ENTRY             BrMaterialAddMany(br_material **items, int n);
br_uint_32 BR_PUBLIC_ENTRY             BrMaterialRemoveMany(br_material **items, int n);
br_uint_32 BR_PUBLIC_ENTRY             BrMaterialFindMany(const char *pattern, br_material **items, int max);
br_uint_32 BR_PUBLIC_ENTRY             BrMaterialCount(const char *pattern);

typedef br_uint_32 BR_CALLBACK br_material_enum_cbfn(br_material *item, void *arg);
br_uint_32 BR_PUBLIC_ENTRY     BrMaterialEnum(const char *pattern, br_material_enum_cbfn *callback, void *arg);

void BR_PUBLIC_ENTRY BrMaterialUpdate(br_material *material, br_uint_16 flags);

br_material *BR_PUBLIC_ENTRY BrMaterialAllocate(const char *name);
void BR_PUBLIC_ENTRY         BrMaterialFree(br_material *m);

/*
 * Model Handling
 */
br_model *BR_PUBLIC_ENTRY           BrModelAdd(br_model *model);
br_model *BR_PUBLIC_ENTRY           BrModelRemove(br_model *model);
br_model *BR_PUBLIC_ENTRY           BrModelFind(const char *pattern);
typedef br_model *BR_CALLBACK       br_model_find_cbfn(const char *name);
br_model_find_cbfn *BR_PUBLIC_ENTRY BrModelFindHook(br_model_find_cbfn *hook);
br_uint_32 BR_PUBLIC_ENTRY          BrModelAddMany(br_model **items, int n);
br_uint_32 BR_PUBLIC_ENTRY          BrModelRemoveMany(br_model **items, int n);
br_uint_32 BR_PUBLIC_ENTRY          BrModelFindMany(const char *pattern, br_model **items, int max);
br_uint_32 BR_PUBLIC_ENTRY          BrModelCount(const char *pattern);

typedef br_uint_32 BR_CALLBACK br_model_enum_cbfn(br_model *item, void *arg);

br_uint_32 BR_PUBLIC_ENTRY BrModelEnum(const char *pattern, br_model_enum_cbfn *callback, void *arg);

void BR_PUBLIC_ENTRY BrModelUpdate(br_model *model, br_uint_16 flags);

void BR_PUBLIC_ENTRY         BrModelApplyMap(br_model *model, int map_type, br_matrix34 *xform);
br_matrix34 *BR_PUBLIC_ENTRY BrModelFitMap(br_model *model, int axis_0, int axis_1, br_matrix34 *transform);

br_model *BR_PUBLIC_ENTRY BrModelAllocate(const char *name, int nvertices, int nfaces);
void BR_PUBLIC_ENTRY      BrModelFree(br_model *m);

/*
 * New primitive handling
 */

br_primitive_list *BR_PUBLIC_ENTRY BrPrimitiveListAllocate(br_uint_32 prim_type, br_uint_16 num_prims);
br_uint_32 BR_PUBLIC_ENTRY         BrModelAddPrimitiveList(br_model *model, br_primitive_list *primitive_list);

/*
 * Texture handling
 */
br_pixelmap *BR_PUBLIC_ENTRY      BrMapAdd(br_pixelmap *pixelmap);
br_pixelmap *BR_PUBLIC_ENTRY      BrMapRemove(br_pixelmap *pixelmap);
br_pixelmap *BR_PUBLIC_ENTRY      BrMapFind(const char *pattern);
typedef br_pixelmap *BR_CALLBACK  br_map_find_cbfn(const char *name);
br_map_find_cbfn *BR_PUBLIC_ENTRY BrMapFindHook(br_map_find_cbfn *hook);
br_uint_32 BR_PUBLIC_ENTRY        BrMapAddMany(br_pixelmap **items, int n);
br_uint_32 BR_PUBLIC_ENTRY        BrMapRemoveMany(br_pixelmap **items, int n);
br_uint_32 BR_PUBLIC_ENTRY        BrMapFindMany(const char *pattern, br_pixelmap **items, int max);
br_uint_32 BR_PUBLIC_ENTRY        BrMapCount(const char *pattern);

typedef br_uint_32 BR_CALLBACK br_map_enum_cbfn(br_pixelmap *item, void *arg);
br_uint_32 BR_PUBLIC_ENTRY     BrMapEnum(const char *pattern, br_map_enum_cbfn *callback, void *arg);

void BR_PUBLIC_ENTRY BrMapUpdate(br_pixelmap *item, br_uint_16 flags);

/*
 * Index lighting table handling
 */
br_pixelmap *BR_PUBLIC_ENTRY        BrTableAdd(br_pixelmap *pixelmap);
br_pixelmap *BR_PUBLIC_ENTRY        BrTableRemove(br_pixelmap *pixelmap);
br_pixelmap *BR_PUBLIC_ENTRY        BrTableFind(const char *pattern);
typedef br_pixelmap *BR_CALLBACK    br_table_find_cbfn(const char *name);
br_table_find_cbfn *BR_PUBLIC_ENTRY BrTableFindHook(br_table_find_cbfn *hook);
br_uint_32 BR_PUBLIC_ENTRY          BrTableAddMany(br_pixelmap **items, int n);
br_uint_32 BR_PUBLIC_ENTRY          BrTableRemoveMany(br_pixelmap **items, int n);
br_uint_32 BR_PUBLIC_ENTRY          BrTableFindMany(const char *pattern, br_pixelmap **items, int max);
br_uint_32 BR_PUBLIC_ENTRY          BrTableCount(const char *pattern);

typedef br_uint_32 BR_CALLBACK br_table_enum_cbfn(br_pixelmap *item, void *arg);

br_uint_32 BR_PUBLIC_ENTRY BrTableEnum(const char *pattern, br_table_enum_cbfn *callback, void *arg);

void BR_PUBLIC_ENTRY BrTableUpdate(br_pixelmap *item, br_uint_16 flags);

/*
 * Actor Handling
 */
typedef br_uint_32 BR_CALLBACK br_actor_enum_cbfn(br_actor *mat, void *arg);
br_uint_32 BR_PUBLIC_ENTRY     BrActorEnum(br_actor *parent, br_actor_enum_cbfn *callback, void *arg);

br_actor *BR_PUBLIC_ENTRY  BrActorAdd(br_actor *parent, br_actor *a);
br_actor *BR_PUBLIC_ENTRY  BrActorAddNoRenumber(br_actor *parent, br_actor *a);
br_actor *BR_PUBLIC_ENTRY  BrActorRemove(br_actor *a);
br_actor *BR_PUBLIC_ENTRY  BrActorRemoveNoRenumber(br_actor *a);
void BR_PUBLIC_ENTRY       BrActorRelink(br_actor *parent, br_actor *actor);
br_uint_16 BR_PUBLIC_ENTRY BrActorToActorMatrix34(br_matrix34 *m, br_actor *a, br_actor *b);
void BR_PUBLIC_ENTRY       BrActorToScreenMatrix4(br_matrix4 *m, br_actor *a, br_actor *camera);

br_actor *BR_PUBLIC_ENTRY BrActorAllocate(br_uint_8 actor_type, void *type_data);
void BR_PUBLIC_ENTRY      BrActorFree(br_actor *a);

br_uint_32 BR_PUBLIC_ENTRY BrActorSearchMany(br_actor *root, char *pattern, br_actor **actors, int max);
br_actor *BR_PUBLIC_ENTRY  BrActorSearch(br_actor *root, char *pattern);

br_bounds *BR_PUBLIC_ENTRY BrActorToBounds(br_bounds *b, br_actor *ap);

/*
 * File operations
 */
br_model *BR_PUBLIC_ENTRY  BrModelLoad(const char *filename);
br_uint_32 BR_PUBLIC_ENTRY BrModelSave(const char *filename, br_model *model);
br_uint_32 BR_PUBLIC_ENTRY BrModelLoadMany(const char *filename, br_model **models, br_uint_16 num);
br_uint_32 BR_PUBLIC_ENTRY BrModelSaveMany(const char *filename, br_model **models, br_uint_16 num);
br_error BR_PUBLIC_ENTRY   BrModelFileCount(const char *filename, br_uint_16 *num);

br_material *BR_PUBLIC_ENTRY BrMaterialLoad(const char *filename);
br_uint_32 BR_PUBLIC_ENTRY   BrMaterialSave(const char *filename, br_material *material);
br_uint_32 BR_PUBLIC_ENTRY   BrMaterialLoadMany(const char *filename, br_material **materials, br_uint_16 num);
br_uint_32 BR_PUBLIC_ENTRY   BrMaterialSaveMany(const char *filename, br_material **materials, br_uint_16 num);
br_error BR_PUBLIC_ENTRY     BrMaterialFileCount(const char *filename, br_uint_16 *num);

br_actor *BR_PUBLIC_ENTRY  BrActorLoad(const char *filename);
br_uint_32 BR_PUBLIC_ENTRY BrActorSave(const char *filename, br_actor *actor);
br_uint_32 BR_PUBLIC_ENTRY BrActorLoadMany(const char *filename, br_actor **actors, br_uint_16 num);
br_uint_32 BR_PUBLIC_ENTRY BrActorSaveMany(const char *filename, br_actor **actors, br_uint_16 num);
br_error BR_PUBLIC_ENTRY   BrActorFileCount(const char *filename, br_uint_16 *num);

/*
 * Lights
 */
void BR_PUBLIC_ENTRY BrLightEnable(br_actor *l);
void BR_PUBLIC_ENTRY BrLightDisable(br_actor *l);

/*
 * Environment
 */
br_actor *BR_PUBLIC_ENTRY BrEnvironmentSet(br_actor *a);

/*
 * Clip planes
 */
void BR_PUBLIC_ENTRY BrClipPlaneEnable(br_actor *cp);
void BR_PUBLIC_ENTRY BrClipPlaneDisable(br_actor *cp);

/*
 * Horizon planes
 */
void BR_PUBLIC_ENTRY BrHorizonPlaneEnable(br_actor *h);
void BR_PUBLIC_ENTRY BrHorizonPlaneDisable(br_actor *h);

/*
 * Picking
 */
typedef int BR_CALLBACK br_pick2d_cbfn(br_actor *a, br_model *model, br_material *material, br_vector3 *ray_pos, br_vector3 *ray_dir,
                                       br_scalar t_near, br_scalar t_far, void *arg);

int BR_PUBLIC_ENTRY BrScenePick2D(br_actor *world, br_actor *camera, br_pixelmap *viewport, int pick_x, int pick_y,
                                  br_pick2d_cbfn *callback, void *arg);

typedef int BR_CALLBACK br_pick3d_cbfn(br_actor *a, br_model *model, br_material *material, br_matrix34 *transform, br_bounds *bounds, void *arg);

int BR_PUBLIC_ENTRY BrScenePick3D(br_actor *world, br_actor *actor, br_bounds *bounds, br_pick3d_cbfn *callback, void *arg);

typedef int BR_CALLBACK br_modelpick2d_cbfn(br_model *model, br_material *material, br_vector3 *ray_pos, br_vector3 *ray_dir, br_scalar t,
                                            int face, int edge, int vertex, br_vector3 *p, br_vector2 *map, void *arg);

int BR_PUBLIC_ENTRY BrModelPick2D(br_model *model, br_material *material, br_vector3 *ray_pos, br_vector3 *ray_dir, br_scalar t_near,
                                  br_scalar t_far, br_modelpick2d_cbfn *callback, void *arg);

/*
 * Custom calback support
 */
br_uint_32 BR_PUBLIC_ENTRY BrOnScreenCheck(br_bounds3 *bounds);

br_uint_8 BR_PUBLIC_ENTRY  BrOriginToScreenXY(br_vector2 *screen);
br_uint_32 BR_PUBLIC_ENTRY BrOriginToScreenXYZO(br_vector3 *screen);

br_uint_8 BR_PUBLIC_ENTRY  BrPointToScreenXY(br_vector2 *screen, br_vector3 *point);
br_uint_32 BR_PUBLIC_ENTRY BrPointToScreenXYZO(br_vector3 *screen, br_vector3 *point);

void BR_PUBLIC_ENTRY BrPointToScreenXYMany(br_vector2 *screens, br_vector3 *points, br_uint_32 npoints);
void BR_PUBLIC_ENTRY BrPointToScreenXYZOMany(br_vector3 *screens, br_uint_32 *outcodes, br_vector3 *points, br_uint_32 npoints);

void BR_PUBLIC_ENTRY BrSceneModelLight(br_model *model, br_material *default_material, br_actor *root, br_actor *a);

void BR_PUBLIC_ENTRY BrModelToScreenQuery(br_matrix4 *dest);
void BR_PUBLIC_ENTRY BrModelToViewQuery(br_matrix34 *dest);

/**
 * \brief Convert z buffer depth [0, 0xFFFFFFFF] to screen z [-32,768.0, +~32,768).
 *
 * \param depth_z A 32 bit value read from the z buffer pixel map.
 * \param camera  A non-NULL pointer to the camera being used for rendering, i.e. relevant to
 *                the depth values used int z buffer.
 *
 * \return Returns the corresponding screen z value.
 *
 * \sa BrScreenZToCamera(), BrScreenXYZToCamera()
 */
br_scalar BR_PUBLIC_ENTRY  BrZbDepthToScreenZ(br_uint_32 depth_z, const br_camera *camera);

/**
 * \brief Convert screen z [-32,768.0, +32,768) to z buffer depth [0, 0xFFFFFFFF].
 *
 * \param sz     A screen z value as obtained from functions such as BrOriginToScreenXYZO().
 * \param camera A non-NULL pointer to the camera being used for rendering, i.e. relevant to
 *               the depth values used in the z buffer.
 *
 * \return A 32 bit depth value suitable for writing to a z buffer pixel map.
 *
 * \sa BrScreenXYZToCamera(), BrScreenZToCamera()
 */
br_uint_32 BR_PUBLIC_ENTRY BrZbScreenZToDepth(br_scalar sz, const br_camera *camera);

/**
 * \brief Convert z sort depth [-hither_z, +yon_z] to screen z [-32,768.0, +32,768).
 *
 * \param depth_z A depth value read from an order table or obtained from a z sort primitive callback.
 * \param camera  A non-NULL pointer to the camera being used for rendering, i.e. relevant to the
 *                depth values used in primitives and order tables.
 *
 * \return The corresponding screen z value.
 *
 * \sa BrScreenZToCamera(), BrScreenXYZToCamera()
 */
br_scalar BR_PUBLIC_ENTRY BrZsDepthToScreenZ(br_scalar depth_z, const br_camera *camera);

/**
 * \brief Convert screen z [-32,768.0, +32,768) to z sort depth [-hither_z, +yon_z].
 *
 * \param sz     A screen z value as obtained from functions such as BrOriginToScreenXYZO().
 * \param camera A non-NULL pointer to the camera being used for rendering, i.e. relevant to the
 *               depth values used in primitives and order tables.
 *
 * \return A depth value suitable for writing to an order table or comparing with values of z
 *         obtained from a z sort primitive callback.
 *
 * \sa BrScreenZToCamera(), BrScreenXYZToCamera()
 */
br_scalar BR_PUBLIC_ENTRY BrZsScreenZToDepth(br_scalar sz, const br_camera *camera);

/**
 * \brief Convert screen z [-32,768.0,+32,767.9] to view z [-hither_z,-yon_z].
 *
 * \param camera Pointer to camera actor.
 * \param sz     Screen z value, e.g. as returned by
 *               BrOriginToScreenXYZO().
 *
 * \return Returns the corresponding z value in the camera actor's co-ordinate space (view space).
 */
br_scalar BR_PUBLIC_ENTRY BrScreenZToCamera(const br_actor *camera, br_scalar sz);

/**
 * \brief Convert a point in screen space to a point in a camera actor's
 *        co-ordinate space (view space) (compare with BrPointToScreenXYZO()).
 *
 * Computes the x & y co-ordinates in screen space, and together with the z co-ordinate
 * applies the inverse projection transform, and store the resulting vector at \p point.
 *
 * \param point         A non-NULL pointer to the vector to hold the converted point in camera space.
 * \param camera        A non-NULL pointer to the camera actor into whose co-ordinate space the point is to be converted.
 * \param screen_buffer A non-NULL pointer to the screen buffer to which the x & y co-ordinates apply.
 * \param x             X co-ordinate of pixel.
 * \param y             Y co-ordinate of pixel.
 * \param sz            Screen z co-ordinate.
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZbBegin() & BrZbEnd().
 *
 * \par Example
 * \code{.c}
 * br_vector3 p;
 * br_uint_32 depth = BrPixelmapGet(&my_depth_buffer, x, y);
 * br_scalar  sz    = BrZbDepthToScreenZ(depth, &my_camera);
 * BrScreenXYZToCamera(&p, &my_camera, &my_screen_buffer, x, y, sz);
 * \endcode
 *
 * \sa BrPointToScreenXYZO(), BrOriginToScreenXYZO(), BrMatrix4Perspective()
 */
void BR_PUBLIC_ENTRY      BrScreenXYZToCamera(br_vector3 *point, const br_actor *camera, const br_pixelmap *screen_buffer, br_int_16 x,
                                              br_int_16 y, br_scalar sz);

br_error BR_PUBLIC_ENTRY BrLightModelCull(br_actor *light);

/*
 * Utility "FindFailed" callbacks that can be used to automaticaly load
 * models/materials/maps/tables from the filesystem
 */
br_pixelmap *BR_CALLBACK BrMapFindFailedLoad(const char *name);
br_pixelmap *BR_CALLBACK BrTableFindFailedLoad(const char *name);
br_model *BR_CALLBACK    BrModelFindFailedLoad(const char *name);
br_material *BR_CALLBACK BrMaterialFindFailedLoad(const char *name);

/*
 * Backwards comaptibility
 */
#define BrModelPrepare    BrModelUpdate
#define BrMaterialPrepare BrMaterialUpdate
#define BrMapPrepare      BrMapUpdate
#define BrTablePrepare    BrTableUpdate

/*
 * Rendering - General
 */
void BR_PUBLIC_ENTRY BrRendererBegin(br_pixelmap *destination, struct br_renderer_facility *renderer_facility,
                                     struct br_primitive_library *primitive_library, void *primitive_heap, br_uint_32 primitive_heap_size);

void BR_PUBLIC_ENTRY BrRendererEnd(void);

void BR_PUBLIC_ENTRY BrRendererFrameBegin(void);
void BR_PUBLIC_ENTRY BrRendererFrameEnd(void);

void BR_PUBLIC_ENTRY BrRendererFocusLossBegin(void);
void BR_PUBLIC_ENTRY BrRendererFocusLossEnd(void);

/*
 * Renderering - Z Buffer
 */

/**
 * \brief Initialise the Z-buffer renderer.
 *
 * This is a rendering engine which utilises a depth buffer (a pixel map matching the colour buffer)
 * containing z values for each pixel, which can be used to determine whether another pixel at the same
 * position should be drawn over the existing one.
 *
 * \param colour_type Pixel map type of buffer to render into.
 * \param depth_type  Pixel map type of Z-buffer.
 *
 * \pre Between BrBegin() & BrEnd(). The registry is empty. No rendering engine is currently enabled.
 *
 * \post Checks that the specified colour and depth types can be supported,
 *       initialises registry for this renderer.
 *
 * \sa BrBegin(), BrEnd()
 *
 */
void BR_PUBLIC_ENTRY BrZbBegin(br_uint_8 colour_type, br_uint_8 depth_type);

/**
 * \brief Close down the Z-Buffer renderer.
 *
 * \pre Between BrBegin() & BrEnd().
 *      BrZbBegin() has been called, and BrZbEnd(), has not yet been called since.
 *      The registry is empty.
 *      No other rendering engine is currently enabled.
 *
 * \post Releases resources used by the Z-Buffer renderer.
 *
 * \sa BrZbBegin()
 */
void BR_PUBLIC_ENTRY BrZbEnd(void);

/**
 * \brief Set up a new scene to be rendered using the Z-Buffer renderer, processing the camera,
 *        lights and environment.
 *
 * Enter rendering state, prepare for destination buffers, preprocess view, screen and environment
 * transforms, preprocess enabled lights, handle environment actor, preprocess enabled clip planes.
 *
 * \param world         A non-NULL pointer to the root actor of a scene.
 * \param camera        A non-NULL pointer to a camera actor that is a descendant of the root actor.
 * \param colour_buffer A non-NULL pointer to the pixel map to render the scene into, whose type is
 *                      \p colour_type as supplied to BrZbBegin().
 * \param depth_buffer  A non-NULL pointer to the pixel map to be used as a depth buffer whose type is
 *                      \p depth_type as supplied to BrZbBegin().
 *                      It must have the same width and height as the colour buffer. See BrPixelmapMatch().
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZbBegin() & BrZbEnd(). Not currently rendering.
 *
 * \remark The colour buffer and depth buffer should not be texture maps (or even shade tables),
 *         though they can of course be subsequently added as such once the rendering has completed.
 *
 * \sa BrZbSceneRenderAdd(), BrZbSceneRenderEnd(), BrZbRenderBoundsCallbackSet(), BrZbModelRender().
 */
void BR_PUBLIC_ENTRY BrZbSceneRenderBegin(br_actor *world, br_actor *camera, br_pixelmap *colour_buffer, br_pixelmap *depth_buffer);

void BR_PUBLIC_ENTRY BrZbSceneRenderContinue(br_actor *world, br_actor *camera, br_pixelmap *colour_buffer, br_pixelmap *depth_buffer);

/**
 * \brief Include an actor (and its descendents) of the \p world in the current rendering.
 *
 * \param tree A non-NULL pointer to an actor hierarchy, which must be descendent of
 *             the \p world hierarchy supplied to BrZbSceneRenderBegin().
 *
 * \pre Between BrBegin() and BrEnd(). Between BrZbBegin() and BrZbEnd().
 *      Currently rendering, i.e. between BrZbSceneRenderBegin() and BrZbSceneRenderEnd().
 *      Not within a custom model render callback or render bounds callback.
 *
 * \post Add actor to the list of actors to be rendered.
 *
 * \remark Whether rendering takes place during this function or sometime before the return
 *         of BrZbSceneRenderEnd() is undefined. When custom model render and render bounds
 *         callback functions are called is similarly undefined.
 *
 * \sa BrZbSceneRenderBegin(), BrZbSceneRenderEnd(), BrZbRenderBoundsCallbackSet(), BrZbModelRender()
 */
void BR_PUBLIC_ENTRY BrZbSceneRenderAdd(br_actor *tree);

/**
 * \brief Complete the specification of actors to be rendered in a scene, and their rendering.
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZbBegin() & BrZbEnd().
 *      Currently rendering, i.e. after BrZbSceneRenderBegin().
 *      Not within a custom model render callback or render bounds callback.
 *
 * \post By the time this function returns, the scene as specified in terms of world root,
 *       camera and sub-trees, will have been rendered to the output buffers.
 *
 * \remark Whether rendering takes place during this function or sometime
 *         before the return of BrZbSceneRenderEnd() is undefined.
 *         When custom model render and render bounds callback functions are called is
 *         similarly undefined.
 *
 * \sa BrZbSceneRenderBegin(), BrZbSceneRenderAdd(), BrZbRenderBoundsCallbackSet(),
 *     BrZbModelRender().
 */
void BR_PUBLIC_ENTRY BrZbSceneRenderEnd(void);

/**
 * \brief All-in-one function to render a scene using the Z-Buffer renderer.
 *
 * \param world         A non-NULL pointer to the root actor of a scene.
 * \param camera        A non-NULL pointer to a camera actor that is a descendant of
 *                      the root actor.
 * \param colour_buffer A non-NULL pointer to the pixel map to render the scene into,
 *                      whose type matches \p colour_type supplied to BrZbBegin().
 * \param depth_buffer  A non-NULL pointer to the pixel map to be used as a depth
 *                      buffer whose type matches \p depth_type as supplied to
 *                      BrZbBegin(). It must have the same width and height
 *                      as the colour buffer. See BrPixelmapMatch().
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZbBegin() & BrZbEnd().
 *      Not currently rendering.
 *
 * \post Equivalent to a call of BrZbSceneRenderBegin() followed by BrZbSceneRenderAdd()
 *       and BrZbSceneRenderEnd().
 *
 * \remark The colour buffer and depth buffer should not be texture maps (or even shade tables), though
 *         they can of course be subsequently added as such once the rendering has completed.
 *
 * \sa BrZbRenderBoundsCallbackSet(), BrZbModelRender()
 */
void BR_PUBLIC_ENTRY BrZbSceneRender(br_actor *world, br_actor *camera, br_pixelmap *colour_buffer, br_pixelmap *depth_buffer);

/*
 * Used within custom model callbacks to render other models
 */
void BR_PUBLIC_ENTRY BrZbModelRender(br_actor *actor, br_model *model, br_material *material, br_uint_8 style, int on_screen, int use_custom);

/**
 * \brief Set the callback function invoked for each rendered actor.
 *
 * For example, a callback can be set up to log those rectangles in the colour buffer
 * that have been written to (dirty rectangle flagging).
 *
 * \param new_cbfn A pointer to the new callback function. Specify the old callback
 *                 function when a function callback is not required.
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZbBegin() and BrZbEnd().
 *      Not currently rendering.
 *
 * \post Defines a function that will be called during rendering for each model that
 *       affects the output buffers.
 *
 * \return Returns a pointer to the old callback function.
 *
 * \remark Exactly when the callback gets called is undefined except that it will be
 *         sometime between BrZbSceneRenderBegin() and BrZbSceneRenderEnd().
 *         It is also not necessarily associated with a particular point in the rendering process.
 *
 * \sa br_renderbounds_cbfn, br_model_custom_cbfn.
 */
br_renderbounds_cbfn *BR_PUBLIC_ENTRY BrZbRenderBoundsCallbackSet(br_renderbounds_cbfn *new_cbfn);

/*
 * Renderering - Z Sort
 */

/**
 * \brief Initialise the Z-sort renderer.
 *
 * This is a rendering engine which uses a bucket sort to determine the order in which primitives
 * (faces, lines, points) should be rendered.
 *
 * \param colour_type Pixel map type of buffer to render into.
 * \param primitive   Non-NULL pointer to an allocated block of memory to be used as a heap to hold
 *                    rendered (i.e. not back faces) primitives and referenced vertices generated during
 *                    rendering.
 * \param size        Size of the primitive heap.
 *                    To ensure that everything is rendered completely, this should be large enough for
 *                    the renderer to store the temporary data structures used to hold details of each
 *                    rendered primitive and the transformed vertices indexed by them.
 *                    A primitive can be anything from a point to a triangle (possibly a quad).
 *                    For the purposes of estimation you should allow 26 bytes for each primitive and 64
 *                    bytes for each unique vertex.
 *                    The number of unique vertices can be calculated from the number of primitives,
 *                    according to how vertices are shared by the models' faces.
 *                    At best there will be an average of one vertex per face primitive (a tetrahedron
 *                    -- four faces, four vertices), at worst there will be an average of three vertices
 *                    per face primitive (a scene of independent triangles).
 *
 *                    Note that clipping is likely to increase the number of vertices.
 *
 *                    If you estimate there is an upper limit of about 1,000 faces (of front facing
 *                    surfaces) in a rendering sequence with an average of 2.5 vertices per triangular
 *                    face (incude increases due to clipping) then the value of this member should be
 *                    \f$1,000 \times (26 + 2.6 \times 64)\f$, i.e. \f$186,000\f$ bytes, call it 200KB.
 *
 * \pre Between BrBegin() & BrEnd(). The registry is empty. No rendering engine is currently enabled.
 *
 * \post Checks that the specified colour types can be supported, initialises registry for this renderer.
 *
 * \remark If \p size is insufficiently large, some primitives will be omitted from the rendering.
 *         In borderline cases, these are likely to be faces of models towards the end of the
 *         traversal of the actor hierarchy (which may manifest as deterioration of models in a particular
 *         lineage).
 *
 * \sa BrZsEnd(), BrZbBegin()
 */
void BR_PUBLIC_ENTRY BrZsBegin(br_uint_8 colour_type, void *primitive, br_uint_32 size);
void BR_PUBLIC_ENTRY BrZsEnd(void);

void BR_PUBLIC_ENTRY BrZsSceneRenderBegin(br_actor *world, br_actor *camera, br_pixelmap *colour_buffer);

void BR_PUBLIC_ENTRY BrZsSceneRenderContinue(br_actor *world, br_actor *camera, br_pixelmap *colour_buffer);

/**
 * \brief Include an actor (and its descendants) of the \p world in the current rendering.
 *
 * \param tree A non-NULL pointer to an actor hierarchy, which must be a descendant of the
 *             \p world hierarchy supplied to BrZsSceneRenderBegin().
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZsBegin() & BrZsEnd().
 *      Currently rendering, i.e. between BrZsSceneRenderBegin() and BrZsSceneRenderEnd().
 *      Not within a custom model render callback, render bounds callback, or primitive callback.
 *
 * \post Add actor to list of actors to be rendered.
 *
 * \remark Whether rendering takes place during this function or sometime before the return
 *         of BrZsSceneRenderEnd() is undefined.
 *         When custom model render, render bounds and primitive callback functions are called
 *         is similarly undefined.
 *
 * \sa BrZsSceneRenderBegin(), BrZsSceneRenderEnd(), BrZsRenderBoundsCallbackSet(),
 *     BrZsPrimitiveCallbackSet(), BrZsModelRender()
 */
void BR_PUBLIC_ENTRY BrZsSceneRenderAdd(br_actor *tree);

/**
 * \brief Complete the specification of actors to be rendered in a scene, and their rendering.
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZsBegin() & BrZsEnd().
 *      Currently rendering, i.e. after BrZsSceneRenderBegin().
 *      Not within a custom model render callback, render bounds callback, or primitive call-back.
 *
 * \post By the time this function ends, the scene as specified in the terms of world root,
 *       camera and sub-trees, will have been rendered to the output buffer.
 *
 * \remark Whether rendering takes places during this function or sometime before the return of
 *         BrZsSceneRenderEnd() is undefined.
 *         When custom model render, render bounds, and primitive callback functions are called
 *         is similarly undefined.
 *
 * \sa BrZsSceneRenderBegin(), BrZsSceneRenderAdd(), BrZsRenderBoundsCallbackSet(),
 *     BrZsPrimitiveCallbackSet(), BrZsModelRender().
 */
void BR_PUBLIC_ENTRY BrZsSceneRenderEnd(void);

/**
 * \brief All-in-one function to render a scene using the Z-Sort renderer.
 *
 * \param world         A non-NULL pointer to the root actor of a scene.
 * \param camera        A non-NULL pointer to a camera actor that is a descendant of
 *                      the root actor.
 * \param colour_buffer A non-NULL pointer to the pixel map to render the scene into,
 *                      whose type is \p colour_type as supplied to BrZsBegin().
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZsBegin() & BrZsEnd(). Not currently rendering.
 *
 * \post Equivalent to a call of BrZsSceneRenderBegin() followed by BrZsSceneRenderAdd()
 *       and BrZsSceneRenderEnd().
 *
 * \remark The colour buffer should not be a texture map (or even a shade table), though
 *         it can of course be subsequently added as such once the rendering has completed.
 *
 * \sa BrZsRenderBoundsCallbackSet(), BrZsModelRender(), BrZsPrimitiveCallbackSet().
 */
void BR_PUBLIC_ENTRY BrZsSceneRender(br_actor *world, br_actor *camera, br_pixelmap *colour_buffer);

/*
 * Used within custom model callbacks to render other models
 */
void BR_PUBLIC_ENTRY BrZsModelRender(br_actor *actor, br_model *model, br_material *material, br_order_table *order_table, br_uint_8 style,
                                     int on_screen, int use_custom);

/**
 * \brief Set the callback function invoked for each rendered actor.
 *
 * For example, a callback can be set up to log those rectangles in the colour buffer that have
 * been written to (dirty rectangle flagging).
 *
 * \param new_cbfn A pointer to the new callback function. Specify the old callback function
 *                 when a function callback is not required.
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZsBegin() & BrZsEnd(). Not currently rendering.
 *
 * \post Defines a function that will be called during rendering for each model that affects the
 *       output buffer.
 *
 * \return Returns a pointer to the old callback function.
 *
 * \remark The actor order table will be supplied to the call-back function.
 *
 * \remark Exactly when the callback function gets called is undefined except that it will be
 *         sometime between BrZsSceneRenderBegin() and BrZsSceneRenderEnd().
 *         It is also not necessarily associated with a particular point in the rendering process.
 *
 * \sa br_renderbounds_cbfn, br_model_custom_cbfn.
 */
br_renderbounds_cbfn *BR_PUBLIC_ENTRY BrZsRenderBoundsCallbackSet(br_renderbounds_cbfn *new_cbfn);

/**
 * \brief Set the callback function invoked for each primitive generated by the Z-Sort renderer.
 *
 * This callback can be used to perform customised insertion of primitives into order tables.
 *
 * \param new_cbfn A pointer to the new callback function. Specify the old callback function when
 *                 a function callback is not required. NULL will indicate the default callback
 *                 function should be used.
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZsBegin() & BrZsEnd(). Not currently rendering.
 *
 * \post Defines a function that will be called during rendering for each front facing primitive
 *       that is to be inserted into the order table.
 *
 * \return Returns a pointer to the old callback function.
 *
 * \remark Exactly when the callback function gets called is undefined except that it will be
 *         sometime between BrZsSceneRenderBegin() and BrZsSceneRenderEnd().
 *         However, it may be assumed that rendering to the output buffer has not yet commenced.
 *
 * \sa br_primitive_cbfn, br_model_custom_cbfn.
 */
br_primitive_cbfn *BR_PUBLIC_ENTRY BrZsPrimitiveCallbackSet(br_primitive_cbfn *new_cbfn);

struct br_order_table *BR_PUBLIC_ENTRY BrZsOrderTableAllocate(br_uint_16 size, br_uint_32 flags, br_uint_16 type);
void BR_PUBLIC_ENTRY                   BrZsOrderTableFree(br_order_table *order_table);
struct br_order_table *BR_PUBLIC_ENTRY BrZsActorOrderTableSet(struct br_actor *actor, struct br_order_table *order_table);
struct br_order_table *BR_PUBLIC_ENTRY BrZsActorOrderTableGet(struct br_actor *actor);
struct br_order_table *BR_PUBLIC_ENTRY BrZsOrderTableClear(struct br_order_table *order_table);

void BR_PUBLIC_ENTRY BrZsOrderTablePrimitiveInsert(struct br_order_table *order_table, struct br_primitive *primitive, br_uint_16 bucket);

br_uint_16 BR_PUBLIC_ENTRY BrZsPrimitiveBucketSelect(br_scalar *z, br_uint_16 type, br_scalar min_z, br_scalar max_z, br_uint_16 size,
                                                     br_uint_16 sort_type);
void BR_PUBLIC_ENTRY       BrZsOrderTablePrimaryEnable(struct br_order_table *order_table);
void BR_PUBLIC_ENTRY       BrZsOrderTablePrimaryDisable(void);

/*
 * Backwards compatibility
 */
#define BrZbSetRenderBoundsCallback BrZbRenderBoundsCallbackSet
#define BrZsSetRenderBoundsCallback BrZsRenderBoundsCallbackSet
#define BrZsSetPrimitiveCallback    BrZsPrimitiveCallbackSet

/*
 * Order table traversal
 */

typedef void BR_CALLBACK zs_order_table_traversal_cbfn(int primitive_type, ot_vertex *v0, ot_vertex *v1, ot_vertex *v2);
void BR_PUBLIC_ENTRY     ZsOrderTableTraversal(zs_order_table_traversal_cbfn *cbfn);

/*
 * Callback function invoked when a renderer facility is enumerated
 */
typedef struct br_rendfcty_desc {

    br_boolean uses_primitive_library;

    /*
     * A pointer to the renderer facility.  Note that if the enumeration
     * was performed within an output facility enumeration, this is only
     * guaranteed to remain valid within the callback routine.
     */
    struct br_renderer_facility *renderer_facility;

} br_rendfcty_desc;

typedef br_boolean BR_CALLBACK br_rendfcty_enum_cbfn(const char *identifier, br_rendfcty_desc *desc, void *args);

/*
 * Callback function invoked when a primitive library is enumerated
 */
typedef struct br_primlib_desc {

    /*
     * A pointer to the primitive library.  Note that if the enumeration
     * was performed within an output facility enumeration, this is only
     * guaranteed to remain valid within the callback routine.
     */
    struct br_primitive_library *primitive_library;

} br_primlib_desc;

typedef br_boolean BR_CALLBACK br_primlib_enum_cbfn(const char *identifier, br_primlib_desc *desc, void *args);

/*
 * Enumeration routines.
 */
br_error BR_PUBLIC_ENTRY BrRendererFacilityEnum(br_pixelmap *destination, br_rendfcty_enum_cbfn *cbfn, void *args);
br_error BR_PUBLIC_ENTRY BrPrimitiveLibraryEnum(br_pixelmap *destination, br_primlib_enum_cbfn *cbfn, void *args);

/*
 * Animation
 */
void BR_PUBLIC_ENTRY BrAnimationInstanceUpdate(br_animation_instance *inst, br_scalar time);
void BR_PUBLIC_ENTRY BrAnimationInstanceDetach(br_animation_instance *inst);

#ifdef __cplusplus
};
#endif

#endif /* _NO_PROTOTYPES */

#endif
