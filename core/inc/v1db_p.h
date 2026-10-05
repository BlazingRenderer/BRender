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
/**
 * \brief An application defined call-back function that is called for each rendered model, at some
 *        point during rendering - at precisely what point is undefined, except that its parents
 *        have been processed (though not necessarily rendered).
 *
 * The pass through equivalent for this call-back is to do nothing. The function may call any
 * non-rendering functions available to a model's custom call-back function - see
 * br_model_custom_cbfn.
 *
 * \param actor           Pointer to model actor referencing the model.
 * \param model           Pointer to a model that has affected the rendering.
 * \param material        Pointer to actor's material if defined, or default material otherwise.
 * \param render_data     If the function is called during a rendering performed by the Z-Sort
 *                        renderer this will point to the order table into which the model actor's
 *                        primitives have been inserted.
 * \param style           Actor's rendering style, or default. BRender will not supply
 *                        BR_RSTYLE_BOUNDING_ or BR_RSTYLE_NONE.
 * \param model_to_screen A pointer to a matrix giving the model to screen transformation.
 * \param bounds          An array containing minimum and maximum screen ordinates of pixels that
 *                        would be modified in the process of rendering the model. Use the following
 *                        symbols to obtain each ordinate: BR_BOUNDS_MIN_X = Left-most column in
 *                        which pixels are modified; BR_BOUNDS_MAX_X = Right-most column in which
 *                        pixels are modified; BR_BOUNDS_MIN_Y = Top-most row in which pixels are
 *                        modified; BR_BOUNDS_MAX_Y = Bottom-most row in which pixels are modified.
 *
 * \pre BRender has completed initialisation. Rendering is in progress. The rendering engine has
 *      determined that the model will affect pixels in the output buffers.
 *
 * \post Behaviour is up to the application. Any of the operations described for
 *       br_model_custom_cbfn can be used except Br[Zb|Zs]ModelRender()
 *
 * \remark This function may also be called as a result of calling Br[Zb|Zs]ModelRender() so if you
 *         are highlighting a model's edges say, be careful that you don't accidentally over
 *         recurse. Any other BRender functions may be called from within this call-back with the
 *         following restrictions:
 * \li Don't call any rendering functions.
 * \li Don't modify any light, clip-plane or camera actors.
 * \li Do not access output buffers until rendering has completed.
 * \li Don't change the environment actor.
 * \li For best performance, avoid adding, updating or removing registry items - try to do these
 *     things before rendering.
 * \li Do not modify the actor hierarchy.
 *
 * \sa br_model_custom_cbfn, br_primitive_cbfn, br_pick2d_cbfn, br_pick3d_cbfn.
 *
 * \par Example
 * Possible uses include:
 * \li Dirty Rectangle Tracking (tracking modified areas of the screen pixel map).
 * \li Monitoring the set of model actors currently on screen
 * \li Labelling, selection, highlighting
 * \li Collision detection (not necessarily indicating the best method) Note that a suitable call to
 *     clear the area of a pixel map for the purpose of clearing dirty rectangles only, is as
 *     follows: BrPixelmapDirtyRectangleFill ( pixel_map , bounds[BR_BOUNDS_MIN_X] ,
 *     bounds[BR_BOUNDS_MIN_Y] , bounds[BR_BOUNDS_MAX_X]-bounds[BR_BOUNDS_MIN_X]+1L ,
 *     bounds[BR_BOUNDS_MAX_Y]-bounds[BR_BOUNDS_MIN_Y]+1L , 0 )
 */
typedef void BR_CALLBACK br_renderbounds_cbfn(br_actor *actor, br_model *model, br_material *material, void *render_data, br_uint_8 style,
                                              br_matrix4 *model_to_screen, br_int_32 bounds[4]);

/*
 * Callback function invoked when a z-sort primitive is generated
 */
/**
 * \brief An application defined call-back function that is called at some point during rendering –
 *        at precisely what point is undefined, and whether its children have been processed is also
 *        undefined.
 *
 * The pass through equivalent for this call-back is described in the example. The function may call
 * any non-rendering functions available to a model's custom call-back function – see
 * br_model_custom_cbfn().
 *
 * \param primitive   Primitive to be inserted in order_table.
 * \param actor       Pointer to model actor referencing the model from which the primitive was
 *                    generated.
 * \param model       Pointer to the model from which the primitive was generated.
 * \param material    Pointer to actor's material if defined, or default material otherwise.
 * \param order_table Order table into which the primitive was about to be inserted.
 * \param z           Non-NULL pointer to one or more depth values of the primitive's vertices
 *                    ([−hither_z, −yon_z] in camera co-ordinate space (linearly) mapped to
 *                    [−hither_z,+yon_z]). The number of values pointer to can be determined from
 *                    the type member of primitive.
 *
 * \pre BRender has completed initialisation. Rendering is in progress (by the Z-Sort renderer). The
 *      rendering engine has generated a primitive that faces the viewer.
 *
 * \post Behaviour is up to the application. Any of the operations described for
 *       br_model_custom_cbfn() can be used apart from Br[Zb|Zs]ModelRender().
 *
 * \remark Any other BRender functions may be called from within this call-back with the following
 *         restrictions:
 * \li Don't call any rendering functions, e.g. BrZsSceneRenderAdd().
 * \li Don't modify any light, clip-plane or camera actors.
 * \li Do not access output buffer until rendering has completed.
 * \li Don't change the environment actor.
 * \li For best performance, avoid adding, updating or removing registry items – try to do these
 *     things before rendering.
 * \li Do not modify the actor hierarchy.
 *
 * \sa br_primitive(), br_model_custom_cbfn(), br_pick2d_cbfn(), br_pick3d_cbfn(),
 *     BrZsOrderTablePrimitiveInsert(), BrZsPrimitiveBucketSelect()
 *
 * \par Example
 * \code{.c}
 * The following code demonstrates a primitive call-back function that performs the equivalent of BRender's normal primitive insertion, and so can be used for collection of statistics, say.
 *     void BR_CALLBACK CBFnPrimitive(br_primitive* primitive, br_actor*
 *       actor, const br_model* model, const br_material* material,
 *       br_order_table* order_table, const br_scalar* z)
 *     {
 *         BrZsOrderTablePrimitiveInsert
 *         ( order_table
 *         , primitive
 *         , BrZsPrimitiveBucketSelect
 *             ( z
 *             , primitive->type
 *             , order_table->min_z
 *             , order_table->max_z
 *             , order_table->size
 *             , order_table->type
 *             )
 *         )
 *         ...
 *         ...
 *     }
 * \endcode
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
/**
 * \brief Add a material to the registry, updating it as necessary.
 *
 * All materials must be added to the registry before they are subsequently involved in rendering.
 *
 * \param material A pointer to a material.
 *
 * \return Returns a pointer to the added item, else NULL if unsuccessful.
 *
 * \sa BrMaterialUpdate(), BrMaterialAddMany(), BrMaterialLoad(), BrMaterialFind(),
 *     BrMaterialRemove().
 */
br_material *BR_PUBLIC_ENTRY           BrMaterialAdd(br_material *material);
/**
 * \brief Remove a material from the registry.
 *
 * \param material A pointer to a material.
 *
 * \return Returns a pointer to the item removed.
 *
 * \sa BrMaterialAdd()
 */
br_material *BR_PUBLIC_ENTRY           BrMaterialRemove(br_material *material);
/**
 * \brief Find a material in the registry by name.
 *
 * A call-back function can be setup to be called if the search is unsuccessful. The search pattern
 * can include the standard wild cards ‘*’ and ‘?’.
 *
 * \param pattern Search pattern.
 *
 * \return Returns a pointer to the material if found, otherwise NULL. If a call-back exists and is
 *         called, the call-back’s return value is returned.
 *
 * \sa BrMaterialFindHook(), BrMaterialFindMany()
 */
br_material *BR_PUBLIC_ENTRY           BrMaterialFind(const char *pattern);
/**
 * \brief An application defined call-back function used when BrMaterialFind() or
 *        BrMaterialFindMany() fail.
 *
 * \param name The search pattern supplied to BrMaterialFind() or BrMaterialFindMany() that did not
 *             match any material.
 *
 * \pre BRender has completed initialisation. No material has an identifier that successfully
 *      matches the search pattern.
 *
 * \post Application defined.
 *
 * \return Either return an existing material that is deemed appropriate for the search pattern, or
 *         NULL if there isn't one. This value will be returned by BrMaterialFind() or
 *         BrMaterialFindMany().
 *
 * \remark This could either be used to supply a default material or to create a material. If
 *         materials were created on demand, then this function could search another list of
 *         available materials (but not yet created) and see if the pattern matched any of them, if
 *         it did, one of them could be registered and returned. Note that there is no way to supply
 *         more than one material.
 *
 * \sa BrMaterialFind(), BrMaterialFindMany(), BrMaterialFindHook(), BrMaterialFindFailedLoad().
 */
typedef br_material *BR_CALLBACK       br_material_find_cbfn(const char *name);
/**
 * \brief Functions to set up a call-back.
 *
 * \param hook A pointer to a call-back function.
 *
 * \post If BrMaterialFind() is unsuccessful and a call-back has been set up, the call-back is
 *       passed the search pattern as its only argument. The call-back should then return a pointer
 *       to a substitute or default item. For example, a call-back could be set up to return a
 *       default material if the desired material cannot be found in the registry. The function
 *       BrMaterialFindFailedLoad() is provided and will probably be sufficient in many cases.
 *
 * \return Returns a pointer to the old call-back function.
 *
 * \sa BrMaterialFindFailedLoad()
 *
 * \par Example
 * \code{.c}
 * br_material BR_CALLBACK * test_callback(const char* pattern)
 * { br_material* default_material;
 * ...
 *     return(default_material);
 * }
 * ...
 * { br_material* material;
 * ...
 *     BrMaterialFindHook(&test_callback);
 *     material = BrMaterialFind("non_existent_material");
 * }
 * \endcode
 */
br_material_find_cbfn *BR_PUBLIC_ENTRY BrMaterialFindHook(br_material_find_cbfn *hook);
/**
 * \brief Add a number of materials to the registry, updating them as necessary.
 *
 * \param items A pointer to an array of pointers to materials.
 * \param n     Number of materials to add to the registry.
 *
 * \return Returns the number of materials added successfully.
 *
 * \sa BrMaterialUpdate(), BrMaterialAdd(), BrMaterialRemove(), BrMaterialRemoveMany().
 */
br_uint_32 BR_PUBLIC_ENTRY             BrMaterialAddMany(br_material **items, int n);
/**
 * \brief Remove a number of materials from the registry.
 *
 * \param items A pointer to an array of pointers materials.
 * \param n     Number of materials to remove from the registry.
 *
 * \return Returns the number of items removed successfully.
 *
 * \sa BrMaterialAddMany()
 */
br_uint_32 BR_PUBLIC_ENTRY             BrMaterialRemoveMany(br_material **items, int n);
/**
 * \brief Find a number of materials in the registry by name.
 *
 * The search pattern can include the standard wild cards ‘*’ and ‘?’.
 *
 * \param pattern Search pattern.
 * \param items   A pointer to an array of pointers to materials.
 * \param max     Maximum number of materials to find.
 *
 * \return Returns the number of materials found. The pointer array is filled with pointers to the
 *         found materials.
 *
 * \sa BrMaterialFind(), BrMaterialFindHook()
 */
br_uint_32 BR_PUBLIC_ENTRY             BrMaterialFindMany(const char *pattern, br_material **items, int max);
/**
 * \brief Count the number of registered materials whose names match a given search pattern.
 *
 * The search pattern can include the standard wild cards ‘*’ and ‘?’.
 *
 * \param pattern Search pattern.
 *
 * \return Returns the number of items matching the search pattern.
 *
 * \sa BrMaterialEnum(), BrMaterialFind()
 */
br_uint_32 BR_PUBLIC_ENTRY             BrMaterialCount(const char *pattern);

/**
 * \brief An application defined call-back function accepting a material and an application supplied
 *        argument (as supplied to BrMaterialEnum()).
 *
 * \param item One of the resource classes selected by BrMaterialEnum().
 * \param arg  The argument supplied to BrMaterialEnum().
 *
 * \pre BRender has completed initialisation.
 *
 * \post Application defined. Avoid adding or removing materials within this function.
 *
 * \return Any non-zero value will terminate the enumeration and be returned by BrMaterialEnum().
 *         Return zero to continue the enumeration.
 *
 * \sa BrMaterialEnum(), BrMaterialFind().
 */
typedef br_uint_32 BR_CALLBACK br_material_enum_cbfn(br_material *item, void *arg);
/**
 * \brief Calls a call-back function for every material in the registry matching a given search
 *        pattern.
 *
 * The call-back is passed a pointer to each matching material, and its second argument is an
 * optional pointer supplied by the user. The search pattern can include the standard wild cards ‘*’
 * and ‘?’. The call-back itself returns a br_uint_32 value. The enumeration will halt at any stage
 * if the return value is non-zero.
 *
 * \param pattern  Search pattern.
 * \param callback A pointer to a call-back function.
 * \param arg      An optional argument to pass to the call-back function.
 *
 * \return Returns the first non-zero call-back return value, or zero if all matching items are
 *         enumerated.
 *
 * \par Example
 * \code{.c}
 * br_uint_32 BR_CALLBACK test_callback(br_material* material, void* arg)
 * { br_uint_32 count;
 * ...
 *     return(count);
 * }
 * ...
 * { br_uint_32 enum;
 * ...
 *     enum = BrMaterialEnum("material",&test_callback,NULL);
 * }
 * \endcode
 */
br_uint_32 BR_PUBLIC_ENTRY     BrMaterialEnum(const char *pattern, br_material_enum_cbfn *callback, void *arg);

/**
 * \brief Update a material that has changed in some respect since the previous update of this
 *        material (or BrMaterialAdd()).
 *
 * \param material A pointer to a material.
 * \param flags    Item update flags. In general, BR_MATU_ALL should be used. However, the following
 *                 table describes when to use more specific update flags. Note that flags can be
 *                 combined using the ‘Or’ operation (BR_MATU_ALL is such a combination of the other
 *                 flags, for your convenience). | Flag | To Be Used When | | --- | --- | |
 *                 BR_MATU_MAP_TRANSFORM | The map_transform has been changed | | BR_MATU_RENDERING
 *                 | The flags member has been changed. | | BR_MATU_LIGHTING | Lighting or Colour
 *                 parameters have been changed. | | BR_MATU_COLOURMAP | Elements of the texture at
 *                 colour_map have been changed | | BR_MATU_ALL | The change is unknown or
 *                 wholesale. This includes the case when an entirely different texture map is being
 *                 used, i.e. colour_map is set to a different pointer |
 *
 * \sa BrMaterialAdd().
 */
void BR_PUBLIC_ENTRY BrMaterialUpdate(br_material *material, br_uint_16 flags);

/**
 * \brief Allocate a new material.
 *
 * \param name String to initialise the identifier member to.
 *
 * \return Returns a pointer to the new material, or NULL if unsuccessful.
 */
br_material *BR_PUBLIC_ENTRY BrMaterialAllocate(const char *name);
/**
 * \brief Deallocate a material and any associated memory.
 *
 * \param m A pointer to a material.
 */
void BR_PUBLIC_ENTRY         BrMaterialFree(br_material *m);

/*
 * Model Handling
 */
/**
 * \brief Add a model to the registry, updating it as necessary.
 *
 * All models must be added to the registry before they are subsequently involved in rendering.
 *
 * \param model A pointer to a model.
 *
 * \return Returns a pointer to the added model, else NULL if unsuccessful.
 *
 * \sa BrModelUpdate(), BrModelAddMany(), BrModelLoad(), BrModelFind(), BrModelRemove()
 */
br_model *BR_PUBLIC_ENTRY           BrModelAdd(br_model *model);
/**
 * \brief Remove a model from the registry.
 *
 * \param model A pointer to a model.
 *
 * \return Returns a pointer to the model removed.
 *
 * \sa BrModelAdd()
 */
br_model *BR_PUBLIC_ENTRY           BrModelRemove(br_model *model);
/**
 * \brief Find a model in the registry by name.
 *
 * A call-back function can be setup to be called if the search is unsuccessful. The search pattern
 * can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 *
 * \return Returns a pointer to the model if found, otherwise NULL. If a call-back exists and is
 *         called, the call-back's return value is returned.
 *
 * \sa BrModelFindHook(), BrModelFindMany()
 */
br_model *BR_PUBLIC_ENTRY           BrModelFind(const char *pattern);
/**
 * \brief An application defined call-back function used when BrModelFind() or BrModelFindMany()
 *        fail.
 *
 * \param name The search pattern supplied to BrModelFind() or BrModelFindMany() that did not match
 *             any model.
 *
 * \pre BRender has completed initialisation. No model has an identifier that successfully matches
 *      the search pattern.
 *
 * \post Application defined.
 *
 * \return Either return an existing model that is deemed appropriate for the search pattern, or
 *         NULL if there isn't one. This value will be returned by BrModelFind() or
 *         BrModelFindMany().
 *
 * \remark This could either be used to supply a default model or to create a model. If models were
 *         created on demand, then this function could search another list of available models (but
 *         not yet created) and see if the pattern matched any of them, if it did, one of them could
 *         be registered and returned. Note that there is no way to supply more than one model.
 *
 * \sa BrModelFind(), BrModelFindMany(), BrModelFindHook(), BrModelFindFailedLoad().
 */
typedef br_model *BR_CALLBACK       br_model_find_cbfn(const char *name);
/**
 * \brief Functions to set up a call-back.
 *
 * \param hook A pointer to a call-back function.
 *
 * \post If BrModelFind() is unsuccessful and a call-back has been set up, the call-back is passed
 *       the search pattern as its only argument. The call-back should then return a pointer to a
 *       substitute or default model. For example, a call-back could be set up to return a default
 *       model if the desired model cannot be found in the registry. The function
 *       BrModelFindFailedLoad() is provided and will probably be sufficient in many cases.
 *
 * \return Returns a pointer to the old call-back function.
 *
 * \sa BrModelFindFailedLoad()
 *
 * \par Example
 * \code{.c}
 * br_model BR_CALLBACK * test_callback(const char* pattern)
 * { br_model* default_model;
 * ...
 *     return(default_model);
 * }
 * ...
 * { br_model* model;
 * ...
 *     BrModelFindHook(&test_callback);
 *     model = BrModelFind("non_existent_model");
 * }
 * \endcode
 */
br_model_find_cbfn *BR_PUBLIC_ENTRY BrModelFindHook(br_model_find_cbfn *hook);
/**
 * \brief Add a number of models to the registry, updating them as necessary.
 *
 * \param items A pointer to an array of pointers to models.
 * \param n     Number of models to add to the registry.
 *
 * \return Returns the number of models added successfully.
 *
 * \sa BrModelUpdate(), BrModelAdd(), BrModelRemove(), BrModelRemoveMany()
 */
br_uint_32 BR_PUBLIC_ENTRY          BrModelAddMany(br_model **items, int n);
/**
 * \brief Remove a number of models from the registry.
 *
 * \param items A pointer to an array of pointers to models.
 * \param n     Number of models to remove from the registry.
 *
 * \return Returns the number of models removed successfully.
 *
 * \sa BrModelAddMany()
 */
br_uint_32 BR_PUBLIC_ENTRY          BrModelRemoveMany(br_model **items, int n);
/**
 * \brief Find a number of models in the registry by name.
 *
 * The search pattern can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 * \param items   A pointer to an array of pointers to models.
 * \param max     Maximum number of models to find.
 *
 * \return Returns the number of models found. The pointer array is filled with pointers to the
 *         found models.
 *
 * \sa BrModelFind(), BrModelFindHook()
 */
br_uint_32 BR_PUBLIC_ENTRY          BrModelFindMany(const char *pattern, br_model **items, int max);
/**
 * \brief Count the number of registered models whose names match a given search pattern.
 *
 * The search pattern can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 *
 * \return Returns the number of models matching the search pattern.
 *
 * \sa BrModelEnum(), BrModelFind()
 */
br_uint_32 BR_PUBLIC_ENTRY          BrModelCount(const char *pattern);

/**
 * \brief An application defined call-back function accepting a model and an application supplied
 *        argument (as supplied to BrModelEnum()).
 *
 * \param item One of the models selected by BrModelEnum().
 * \param arg  The argument supplied to BrModelEnum().
 *
 * \pre BRender has completed initialisation.
 *
 * \post Application defined. Avoid adding or removing models within this function.
 *
 * \return Any non-zero value will terminate the enumeration and be returned by BrModelEnum().
 *         Return zero to continue the enumeration.
 *
 * \sa BrModelEnum(), BrModelFind().
 */
typedef br_uint_32 BR_CALLBACK br_model_enum_cbfn(br_model *item, void *arg);

/**
 * \brief Calls a call-back function for every model in the registry matching a given search
 *        pattern.
 *
 * The call-back is passed a pointer to each matching model, and its second argument is an optional
 * pointer supplied by the user. The search pattern can include the standard wild cards '*' and '?'.
 * The call-back itself returns a br_uint_32 value. The enumeration will halt at any stage if the
 * return value is non-zero.
 *
 * \param pattern  Search pattern.
 * \param callback A pointer to a call-back function.
 * \param arg      An optional argument to pass to the call-back function.
 *
 * \return Returns the first non-zero call-back return value, or zero if all matching models are
 *         enumerated.
 *
 * \par Example
 * \code{.c}
 * br_uint_32 BR_CALLBACK test_callback(br_model* model, void* arg)
 * { br_uint_32 count;
 * ...
 *     return(count);
 * }
 * ...
 * { br_uint_32 enum;
 * ...
 *     enum = BrModelEnum("model",&test_callback,NULL);
 * }
 * \endcode
 */
br_uint_32 BR_PUBLIC_ENTRY BrModelEnum(const char *pattern, br_model_enum_cbfn *callback, void *arg);

/**
 * \brief Update a model that has changed in some respect since the previous update of this model
 *        (or BrModelAdd()).
 *
 * \param model A pointer to a model.
 * \param flags Model update flags. In general, BR_MODU_ALL should be used. However, the following
 *              table describes when to use more specific update flags. Note that flags can be
 *              combined using the 'Or' operation (BR_MODU_ALL is such a combination of the other
 *              flags, for your convenience). | Update Flag Symbol | When to Use | | --- | --- | |
 *              BR_MODU_VERTICES | Any aspect of vertices changes, including: vertex co-ordinates,
 *              the number of vertices, pre-lighting values, texture co-ordinates | | BR_MODU_FACES
 *              | Any aspect of faces changes, including: the set of vertices used by all faces, a
 *              face's vertex order, a face's set of vertices, a face's smoothing group or flags,
 *              the number of faces | | BR_MODU_MATERIALS | A face's material pointer changes | |
 *              BR_MODU_ALL | An unknown or wholesale change occurs |
 *
 * \sa BrModelAdd()
 */
void BR_PUBLIC_ENTRY BrModelUpdate(br_model *model, br_uint_16 flags);

/**
 * \brief Generate texture co-ordinates (u,v) for a model's vertices, using a planar, spherical,
 *        cylindrical, disc or null mapping.
 *
 * The model's vertices can be pre-transformed by an optional matrix.
 *
 * \param model    A pointer to a model.
 * \param map_type Mapping type. This determines how a texture is wrapped around a model. Each type
 *                 is described in the following table. | Map Type Symbol | Mapping | Texture
 *                 Co-ordinates (u,v) | | --- | --- | --- | | BR_APPLYMAP_NONE | None | (0 0) | |
 *                 BR_APPLYMAP_PLANE | Planar | (½(x + 1) ½(y + 1)) | | BR_APPLYMAP_DISC | Disc |
 *                 (1/2π arctan -x/z √(x² + y²)) | | BR_APPLYMAP_CYLINDER | Cylindrical | (1/2π
 *                 arctan -x/z ½(y + 1)) | | BR_APPLYMAP_SPHERE | Spherical | (1/2π arctan -x/z 1 -
 *                 1/π arctan y/√(x² + z²)) | The disc mapping can be visualised by considering a
 *                 cylindrical mapping, but shrinking one end of the cylinder to a point and then
 *                 flattening it to form a disc. The cylindrical mapping, predictably, can be
 *                 visualised by imagining a texture wrapped around the outside of a cylinder. The
 *                 spherical mapping is similar to a cylindrical mapping, but the ends of the
 *                 cylinder are shrunk to single points.
 * \param xform    A pointer to an optional matrix. If NULL, the identity transformation is used.
 *
 * \sa BrModelFitMap()
 */
void BR_PUBLIC_ENTRY         BrModelApplyMap(br_model *model, int map_type, br_matrix34 *xform);
/**
 * \brief Generate a transformation which will map the bounds of a model onto a cube defined by the
 *        corner co-ordinates (-1,-1,-1) and (1,1,1).
 *
 * When passed to BrModelApplyMap(), texture co-ordinates will be generated which fit the model
 * exactly. The two axes along which the mapping is applied must be specified.
 *
 * \param model     A pointer to a model.
 * \param axis_0    Mapping axes, defined as follows: | Mapping Axis Symbol | Mapping is Applied
 *                  Along | | --- | --- | | BR_FITMAP_PLUS_X | Positive x axis | | BR_FITMAP_PLUS_Y
 *                  | Positive y axis | | BR_FITMAP_PLUS_Z | Positive z axis | | BR_FITMAP_MINUS_X |
 *                  Negative x axis | | BR_FITMAP_MINUS_Y | Negative y axis | | BR_FITMAP_MINUS_Z |
 *                  Negative z axis |
 * \param axis_1    Mapping axes, defined as follows: | Mapping Axis Symbol | Mapping is Applied
 *                  Along | | --- | --- | | BR_FITMAP_PLUS_X | Positive x axis | | BR_FITMAP_PLUS_Y
 *                  | Positive y axis | | BR_FITMAP_PLUS_Z | Positive z axis | | BR_FITMAP_MINUS_X |
 *                  Negative x axis | | BR_FITMAP_MINUS_Y | Negative y axis | | BR_FITMAP_MINUS_Z |
 *                  Negative z axis |
 * \param transform A pointer to the destination transformation matrix.
 *
 * \return Returns a pointer to the destination transformation matrix as supplied (for convenience).
 *
 * \sa BrModelApplyMap()
 */
br_matrix34 *BR_PUBLIC_ENTRY BrModelFitMap(br_model *model, int axis_0, int axis_1, br_matrix34 *transform);

/**
 * \brief Allocate a new model.
 *
 * \param name      String to initialise the identifier member to.
 * \param nvertices Size of vertex list to allocate. This should be set to zero if maintaining the
 *                  vertex list separately (in which case BR_MODF_KEEP_ORIGINAL should be
 *                  immediately set in the returned structure).
 * \param nfaces    Size of face list to allocate. This should be set to zero if maintaining the
 *                  face list separately (in which case BR_MODF_KEEP_ORIGINAL should be immediately
 *                  set in the returned structure).
 *
 * \return Returns a pointer to the new model, or NULL if unsuccessful.
 */
br_model *BR_PUBLIC_ENTRY BrModelAllocate(const char *name, int nvertices, int nfaces);
/**
 * \brief Deallocate a model and any associated memory.
 *
 * \param m A pointer to a model.
 */
void BR_PUBLIC_ENTRY      BrModelFree(br_model *m);

/*
 * New primitive handling
 */

br_primitive_list *BR_PUBLIC_ENTRY BrPrimitiveListAllocate(br_uint_32 prim_type, br_uint_16 num_prims);
br_uint_32 BR_PUBLIC_ENTRY         BrModelAddPrimitiveList(br_model *model, br_primitive_list *primitive_list);

/*
 * Texture handling
 */
/**
 * \brief Add a texture map to the registry, updating it as necessary.
 *
 * All texture maps must be added to the registry before they are subsequently involved in
 * rendering.
 *
 * \param pixelmap A pointer to a texture map.
 *
 * \return A pointer to the added texture map, or NULL if unsuccessful.
 *
 * \sa BrMapUpdate(), BrMapAddMany(), BrPixelmapLoad(), BrMapFind(), BrMapRemove()
 */
br_pixelmap *BR_PUBLIC_ENTRY      BrMapAdd(br_pixelmap *pixelmap);
/**
 * \brief Remove a texture map from the registry.
 *
 * \param pixelmap A pointer to a texture map.
 *
 * \return A pointer to the item removed.
 *
 * \sa BrMapAdd()
 */
br_pixelmap *BR_PUBLIC_ENTRY      BrMapRemove(br_pixelmap *pixelmap);
/**
 * \brief Find a texture map in the registry by name.
 *
 * A call-back function can be setup to be called if the search is unsuccessful. The search pattern
 * can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 *
 * \return A pointer to the texture map if found, otherwise NULL. If a call-back exists and is
 *         called, the call-back's return value is returned.
 *
 * \sa BrMapFindHook(), BrMapFindMany()
 */
br_pixelmap *BR_PUBLIC_ENTRY      BrMapFind(const char *pattern);
/**
 * \brief An application defined call-back function used when BrMapFind() or BrMapFindMany() fail.
 *
 * \param name The search pattern supplied to BrMapFind() or BrMapFindMany() that did not match any
 *             map.
 *
 * \pre BRender has completed initialisation. No map has an identifier that successfully matches the
 *      search pattern.
 *
 * \post Application defined.
 *
 * \return Either return an existing map that is deemed appropriate for the search pattern, or NULL
 *         if there isn't one. This value will be returned by BrMapFind() or BrMapFindMany().
 *
 * \remark This could either be used to supply a default map or to create a map. If maps were
 *         created on demand, then this function could search another list of available maps (but
 *         not yet created) and see if the pattern matched any of them, if it did, one of them could
 *         be registered and returned. Note that there is no way to supply more than one map.
 *
 * \sa BrMapFind(), BrMapFindMany(), BrMapFindHook(), BrMapFindFailedLoad().
 */
typedef br_pixelmap *BR_CALLBACK  br_map_find_cbfn(const char *name);
/**
 * \brief Set the call-back function used by BrMapFind().
 *
 * If BrMapFind() is unsuccessful and a call-back has been set up, the call-back is passed the
 * search pattern as its only argument. The call-back should then return a pointer to a substitute
 * or default texture map. For example, a call-back could be set up to return a default texture map
 * if the desired texture map cannot be found in the registry. The function BrMapFindFailedLoad()
 * is provided and will probably be sufficient in many cases.
 *
 * \param hook A pointer to a call-back function.
 *
 * \return A pointer to the old call-back function.
 *
 * \sa BrMapFindFailedLoad()
 */
br_map_find_cbfn *BR_PUBLIC_ENTRY BrMapFindHook(br_map_find_cbfn *hook);
/**
 * \brief Add a number of texture maps to the registry, updating them as necessary.
 *
 * \param items A pointer to an array of pointers to texture maps.
 * \param n     Number of texture maps to add to the registry.
 *
 * \return The number of texture maps added successfully.
 *
 * \sa BrMapUpdate(), BrMapAdd(), BrMapRemove(), BrMapRemoveMany()
 */
br_uint_32 BR_PUBLIC_ENTRY        BrMapAddMany(br_pixelmap **items, int n);
/**
 * \brief Remove a number of texture maps from the registry.
 *
 * \param items A pointer to an array of pointers to texture maps.
 * \param n     Number of texture maps to remove from the registry.
 *
 * \return The number of texture maps removed successfully.
 *
 * \sa BrMapAddMany()
 */
br_uint_32 BR_PUBLIC_ENTRY        BrMapRemoveMany(br_pixelmap **items, int n);
/**
 * \brief Find a number of texture maps in the registry by name.
 *
 * The search pattern can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 * \param items   A pointer to an array of pointers to texture maps.
 * \param max     Maximum number of texture maps to find.
 *
 * \return The number of texture maps found. The pointer array is filled with pointers to the found
 *         texture maps.
 *
 * \sa BrMapFind(), BrMapFindHook()
 */
br_uint_32 BR_PUBLIC_ENTRY        BrMapFindMany(const char *pattern, br_pixelmap **items, int max);
/**
 * \brief Count the number of registered texture maps whose names match a given search pattern.
 *
 * The search pattern can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 *
 * \return The number of texture maps matching the search pattern.
 *
 * \sa BrMapEnum(), BrMapFind()
 */
br_uint_32 BR_PUBLIC_ENTRY        BrMapCount(const char *pattern);

/**
 * \brief An application defined call-back function accepting a pixel map and an application
 *        supplied argument (as supplied to BrMapEnum()).
 *
 * \param item One of the maps selected by BrMapEnum().
 * \param arg  The argument supplied to BrMapEnum().
 *
 * \pre BRender has completed initialisation.
 *
 * \post Application defined. Avoid adding or removing maps within this function.
 *
 * \return Any non-zero value will terminate the enumeration and be returned by BrMapEnum(). Return
 *         zero to continue the enumeration.
 *
 * \sa BrMapEnum(), BrMapFind().
 */
typedef br_uint_32 BR_CALLBACK br_map_enum_cbfn(br_pixelmap *item, void *arg);
/**
 * \brief Call a call-back function for every texture map matching a given search pattern.
 *
 * The call-back is passed a pointer to each matching item, and its second argument is an optional
 * pointer supplied by the user. The search pattern can include the standard wild cards '*' and
 * '?'. The call-back itself returns a br_uint_32 value. The enumeration will halt at any stage if
 * the return value is non-zero.
 *
 * \param pattern  Search pattern.
 * \param callback A pointer to a call-back function.
 * \param arg      An optional argument to pass to the call-back function.
 *
 * \return The first non-zero call-back return value, or zero if all matching texture maps are
 *         enumerated.
 *
 * \par Example
 * \code{.c}
 * br_uint_32 BR_CALLBACK test_callback(br_pixelmap* map, void* arg)
 * { br_uint_32 count;
 * ...
 *     return(count);
 * }
 * ...
 * { br_uint_32 enum;
 * ...
 *     enum = BrMapEnum("map",&test_callback,NULL);
 * }
 * \endcode
 */
br_uint_32 BR_PUBLIC_ENTRY     BrMapEnum(const char *pattern, br_map_enum_cbfn *callback, void *arg);

/**
 * \brief Update a texture map.
 *
 * \param item  A pointer to a texture map.
 * \param flags Texture map update flags. In general, BR_MAPU_ALL should be used.
 *
 * \sa BrMapAdd()
 */
void BR_PUBLIC_ENTRY BrMapUpdate(br_pixelmap *item, br_uint_16 flags);

/*
 * Index lighting table handling
 */
/**
 * \brief Add a shade table to the registry, updating it as necessary.
 *
 * All shade tables must be added to the registry before they are used subsequently.
 *
 * \param pixelmap A pointer to a shade table.
 *
 * \return A pointer to the added shade table, or NULL if unsuccessful.
 *
 * \sa BrTableUpdate(), BrTableAddMany(), BrPixelmapLoad(), BrTableFind(), BrTableRemove()
 */
br_pixelmap *BR_PUBLIC_ENTRY        BrTableAdd(br_pixelmap *pixelmap);
/**
 * \brief Remove a shade table from the registry.
 *
 * \param pixelmap A pointer to a shade table.
 *
 * \return A pointer to the shade table removed.
 *
 * \sa BrTableAdd()
 */
br_pixelmap *BR_PUBLIC_ENTRY        BrTableRemove(br_pixelmap *pixelmap);
/**
 * \brief Find a shade table in the registry by name.
 *
 * A call-back function can be setup to be called if the search is unsuccessful. The search pattern
 * can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 *
 * \return A pointer to the shade table if found, otherwise NULL. If a call-back exists and is
 *         called, the call-back's return value is returned.
 *
 * \sa BrTableFindHook(), BrTableFindMany()
 */
br_pixelmap *BR_PUBLIC_ENTRY        BrTableFind(const char *pattern);
/**
 * \brief An application defined call-back function used when BrTableFind() or BrTableFindMany()
 *        fail.
 *
 * \param name The search pattern supplied to BrTableFind() or BrTableFindMany() that did not match
 *             any table.
 *
 * \pre BRender has completed initialisation. No table has an identifier that successfully matches
 *      the search pattern.
 *
 * \post Application defined.
 *
 * \return Either return an existing table that is deemed appropriate for the search pattern, or
 *         NULL if there isn't one. This value will be returned by BrTableFind() or
 *         BrTableFindMany().
 *
 * \remark This could either be used to supply a default table or to create a table. If tables were
 *         created on demand, then this function could search another list of available tables (but
 *         not yet created) and see if the pattern matched any of them, if it did, one of them could
 *         be registered and returned. Note that there is no way to supply more than one table.
 *
 * \sa BrTableFind(), BrTableFindMany(), BrTableFindHook(), BrTableFindFailedLoad().
 */
typedef br_pixelmap *BR_CALLBACK    br_table_find_cbfn(const char *name);
/**
 * \brief Set the call-back function used by BrTableFind().
 *
 * If BrTableFind() is unsuccessful and a call-back has been set up, the call-back it is passed the
 * search pattern as its only argument. The call-back should then return a pointer to a substitute
 * or default shade table. For example, a call-back could be set up to return a default shade table
 * if the desired shade table cannot be found in the registry. The function BrTableFindFailedLoad()
 * is provided and will probably be sufficient in many cases.
 *
 * \param hook A pointer to a call-back function.
 *
 * \return A pointer to the old call-back function.
 *
 * \sa BrTableFindFailedLoad()
 */
br_table_find_cbfn *BR_PUBLIC_ENTRY BrTableFindHook(br_table_find_cbfn *hook);
/**
 * \brief Add a number of shade tables to the registry, updating them as necessary.
 *
 * \param items A pointer to an array of pointers to shade tables.
 * \param n     Number of shade tables to add to the registry.
 *
 * \return The number of shade tables added successfully.
 *
 * \sa BrTableUpdate(), BrTableAdd(), BrTableRemove(), BrTableRemoveMany()
 */
br_uint_32 BR_PUBLIC_ENTRY          BrTableAddMany(br_pixelmap **items, int n);
/**
 * \brief Remove a number of shade tables from the registry.
 *
 * \param items A pointer to an array of pointers to shade tables.
 * \param n     Number of shade tables to remove from the registry.
 *
 * \return The number of shade tables removed successfully.
 *
 * \sa BrTableAddMany()
 */
br_uint_32 BR_PUBLIC_ENTRY          BrTableRemoveMany(br_pixelmap **items, int n);
/**
 * \brief Find a number of shade tables in the registry by name.
 *
 * The search pattern can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 * \param items   A pointer to an array of pointers to shade tables.
 * \param max     Maximum number of shade tables to find.
 *
 * \return The number of shade tables found. The pointer array is filled with pointers to the found
 *         shade tables.
 *
 * \sa BrTableFind(), BrTableFindHook()
 */
br_uint_32 BR_PUBLIC_ENTRY          BrTableFindMany(const char *pattern, br_pixelmap **items, int max);
/**
 * \brief Count the number of registered shade tables whose names match a given search pattern.
 *
 * The search pattern can include the standard wild cards '*' and '?'.
 *
 * \param pattern Search pattern.
 *
 * \return The number of shade tables matching the search pattern.
 *
 * \sa BrTableEnum(), BrTableFind()
 */
br_uint_32 BR_PUBLIC_ENTRY          BrTableCount(const char *pattern);

/**
 * \brief An application defined call-back function accepting a table and an application supplied
 *        argument (as supplied to BrTableEnum()).
 *
 * \param item One of the tables selected by BrTableEnum().
 * \param arg  The argument supplied to BrTableEnum().
 *
 * \pre BRender has completed initialisation.
 *
 * \post Application defined. Avoid adding or removing tables within this function.
 *
 * \return Any non-zero value will terminate the enumeration and be returned by BrTableEnum().
 *         Return zero to continue the enumeration.
 *
 * \sa BrTableEnum(), BrTableFind().
 */
typedef br_uint_32 BR_CALLBACK br_table_enum_cbfn(br_pixelmap *item, void *arg);

/**
 * \brief Call a call-back function for every shade table matching a given search pattern.
 *
 * The call-back is passed a pointer to each matching shade table, and its second argument is an
 * optional pointer supplied by the user. The search pattern can include the standard wild cards
 * '*' and '?'. The call-back itself returns a br_uint_32 value. The enumeration will halt at any
 * stage if the return value is non-zero.
 *
 * \param pattern  Search pattern.
 * \param callback A pointer to a call-back function.
 * \param arg      An optional argument to pass to the call-back function.
 *
 * \return The first non-zero call-back return value, or zero if all matching shade tables are
 *         enumerated.
 *
 * \par Example
 * \code{.c}
 * br_uint_32 BR_CALLBACK test_callback(br_pixelmap* table, void* arg)
 * { br_uint_32 count;
 * ...
 *     return(count);
 * }
 * ...
 * { br_uint_32 enum;
 * ...
 *     enum = BrTableEnum("Table",&test_callback,NULL);
 * }
 * \endcode
 */
br_uint_32 BR_PUBLIC_ENTRY BrTableEnum(const char *pattern, br_table_enum_cbfn *callback, void *arg);

/**
 * \brief Update a shade table.
 *
 * \param item  A pointer to a shade table.
 * \param flags Shade table update flags. In general, BR_TABU_ALL should be used.
 *
 * \sa BrTableAdd()
 */
void BR_PUBLIC_ENTRY BrTableUpdate(br_pixelmap *item, br_uint_16 flags);

/*
 * Actor Handling
 */
/**
 * \brief An application defined call-back function accepting an actor and an application supplied
 *        argument (as supplied to BrActorEnum()).
 *
 * \param mat One of the child actors enumerated by BrActorEnum().
 * \param arg The argument supplied to BrActorEnum().
 *
 * \pre BRender has completed initialisation.
 *
 * \post Application defined. Avoid adding or removing children to the parent actor within this
 *       function.
 *
 * \return Any non-zero value will terminate the enumeration and be returned by BrActorEnum().
 *         Return zero to continue the enumeration.
 *
 * \sa BrActorEnum(), BrActorSearch().
 */
typedef br_uint_32 BR_CALLBACK br_actor_enum_cbfn(br_actor *mat, void *arg);
/**
 * \brief Enumerates an actor's children, calling a user supplied call-back function for each child
 *        actor.
 *
 * \param parent   A pointer to the actor whose children are to be enumerated.
 * \param callback A pointer to the call-back function to be called for each child actor.
 * \param arg      The argument to pass to the call-back function (use NULL if not required).
 *
 * \pre Between BrBegin() & BrEnd().
 *
 * \post For each child of parent, callback is invoked supplied with a pointer to the child and the
 *       arg pointer. If callback returns a non-zero result, the BrActorEnum() function will
 *       immediately return with the same result, otherwise the enumeration continues until all
 *       children have been enumerated.
 *
 * \return The result is zero, or the first and only non-zero result returned by callback.
 *
 * \remark An entire actor hierarchy can be enumerated simply by having the call-back function
 *         invoke BrActorEnum() itself.
 *
 * \sa br_actor_enum_cbfn.
 *
 * \par Example
 * \code{.c}
 * br_uint_32 BR_CALLBACK CountDescendantsCB(br_actor* a,void* n)
 * { ++*(int*)n;
 *   return BrActorEnum(a,CountDescendantsCB,n);
 * }
 * int CountDescendants(br_actor* a)
 * { int N=0;
 *   BrActorEnum(a,CountDescendantsCB,&N);
 *   return N;
 * }
 * \endcode
 */
br_uint_32 BR_PUBLIC_ENTRY     BrActorEnum(br_actor *parent, br_actor_enum_cbfn *callback, void *arg);

/**
 * \brief Add an actor hierarchy as a child of a given parent.
 *
 * \param parent A non-NULL pointer to the parental actor.
 * \param a      A non-NULL pointer to its new child.
 *
 * \pre Between BrBegin() & BrEnd(). New child must be root of its hierarchy, i.e. must not still be
 *      child of another parent.
 *
 * \post Parent's children member may be modified. Added hierarchy will be linked into linked list
 *       of parent's children. Members next and prev of some of parent's children and of added
 *       hierarchy will be modified. The parent member of the added hierarchy will be modified. All
 *       depth members of added hierarchy will be updated.
 *
 * \return The new child pointer a is returned as supplied (for convenience).
 *
 * \remark Do not attempt to add an actor at more than one place in a hierarchy at a time — BRender
 *         currently only supports simple hierarchical actor structures, not directed acyclic (or
 *         cyclic) graph structures. Note that in spite of this restriction, models on the other
 *         hand, may be simultaneously referred to by any number of actors.
 *
 * \sa BrActorRemove(), BrActorRelink()
 */
br_actor *BR_PUBLIC_ENTRY  BrActorAdd(br_actor *parent, br_actor *a);
br_actor *BR_PUBLIC_ENTRY  BrActorAddNoRenumber(br_actor *parent, br_actor *a);
/**
 * \brief Remove an actor hierarchy from its parent.
 *
 * \param a A non-NULL pointer to the hierarchy to remove.
 *
 * \pre Between BrBegin() & BrEnd(). Actor to remove must be a child.
 *
 * \post Parent's children member may be modified. Removed hierarchy will be unlinked from linked
 *       list of parent's children. Members next and prev of some of parent's children and of
 *       removed hierarchy will be modified. The parent member of the removed hierarchy will be
 *       modified. All depth members of removed hierarchy will be updated.
 *
 * \return The pointer to the removed hierarchy is returned as supplied (for convenience).
 *
 * \remark Note that a Light or Clip plane actor must be disabled before it is removed.
 *
 * \sa BrActorAdd(), BrActorRelink()
 */
br_actor *BR_PUBLIC_ENTRY  BrActorRemove(br_actor *a);
br_actor *BR_PUBLIC_ENTRY  BrActorRemoveNoRenumber(br_actor *a);
/**
 * \brief Move an actor in a hierarchy, but preserve its apparent world transformation by
 *        manipulating its own transformation as necessary.
 *
 * \param parent A non-NULL pointer to the new parent.
 * \param actor  A non-NULL pointer to the actor to move.
 *
 * \pre Between BrBegin() & BrEnd(). Both supplied actors are in the same hierarchy.
 *
 * \post Equivalent to a call of BrActorRemove(a) followed by BrActorAdd(parent,a), except that the
 *       moved actor's transform is modified to reproduce the effects of the original transform.
 *
 * \remark Note that a light or clip plane actor must be disabled before it is relinked.
 *
 * \sa BrActorAdd(), BrActorRemove()
 */
void BR_PUBLIC_ENTRY       BrActorRelink(br_actor *parent, br_actor *actor);
/**
 * \brief Accumulate the transformations between one actor and another, representing the result as a
 *        matrix.
 *
 * \param m A non-NULL pointer to the destination matrix, into which will be placed the transform.
 * \param a A non-NULL pointer to the actor from whose co-ordinate space co-ordinates are to be
 *          transformed.
 * \param b A non-NULL pointer to the actor into whose co-ordinate space co-ordinates are to be
 *          transformed.
 *
 * \pre Between BrBegin() & BrEnd(). Both actors must be in the same hierarchy.
 *
 * \post Will calculate the transform required to transform co-ordinates in the co-ordinate space of
 *       a, into the co-ordinate space of b.
 *
 * \return The type of the accumulated transformation, e.g. BR_TRANSFORM_MATRIX34 (see
 *         br_transform).
 */
br_uint_16 BR_PUBLIC_ENTRY BrActorToActorMatrix34(br_matrix34 *m, br_actor *a, br_actor *b);
/**
 * \brief Accumulate the transformations between an actor and the screen, representing the result as
 *        a matrix.
 *
 * \param m      A non-NULL pointer to the destination matrix to receive the transform between
 *               homogenous co-ordinates in the actor's co-ordinate space into the homogenous screen
 *               space.
 * \param a      A non-NULL pointer to an actor.
 * \param camera A non-NULL pointer to a camera actor.
 *
 * \remark The function BrMatrix4ApplyP() is typically used with this function to convert
 *         co-ordinates in an actor's co-ordinate space into homogenous screen space (assuming a
 *         centred projection). Note though that the resultant 4-vector is a set of homogenous
 *         co-ordinates and thus the x, y and z values will need to be divided by the w component.
 *
 * \sa BrMatrix4Perspective().
 */
void BR_PUBLIC_ENTRY       BrActorToScreenMatrix4(br_matrix4 *m, br_actor *a, br_actor *camera);

/**
 * \brief Allocate a new actor.
 *
 * \param actor_type Defines the type member of the actor allocated (See the type member of br_actor
 *                   for a list of acceptable types). It is also used to determine suitable type
 *                   specific data if NULL is supplied for the type_data argument.
 * \param type_data  A pointer to optional, additional, type specific data used to determine the
 *                   type_data member of the actor allocated. (See the type_data member of br_actor
 *                   to determine the type of data structure that should be referenced). NULL may be
 *                   supplied in all cases.
 *
 * \pre Between BrBegin() & BrEnd(). Memory can be allocated.
 *
 * \post A br_actor data structure is allocated and initialised (See br_actor Initialisation). Its
 *       type member is initialised with the value of actor_type. Where the actor requires type
 *       specific data, and NULL has been supplied, the type_data member is initialised with a
 *       pointer to an appropriate, freshly allocated data structure containing default values.
 *
 * \return A pointer to the new br_actor data structure.
 *
 * \remark If this function allocates type specific data, it is allocated and attached to the actor
 *         using BrResAllocate(), so will be freed with the actor even if the actor's type_data
 *         member is subsequently changed. Until the actor is freed, the data may be freely
 *         accessed.
 *
 * \sa BrActorFree().
 *
 * \par Example
 * \code{.c}
 * br_actor * t_pActor=BrActorAllocate(BR_ACTOR_MODEL,NULL);
 * \endcode
 */
br_actor *BR_PUBLIC_ENTRY BrActorAllocate(br_uint_8 actor_type, void *type_data);
/**
 * \brief Free an actor and all its descendants (children) if any.
 *
 * \param a A pointer to an actor previously allocated using BrActorAllocate(). NULL is not
 *          acceptable. If a Light, or Clip Plane actor, it should have first been disabled. The
 *          actor should have first been detached from any hierarchy by using BrActorRemove().
 *
 * \pre Between BrBegin() & BrEnd().
 *
 * \post Effectively applies BrActorRemove() and BrActorFree() to each child of the actor. Calls
 *       BrResFree() to release storage for the actor and any attached data (e.g. type specific data
 *       created by default).
 *
 * \remark Not recommended to be applied to an actor during rendering of the hierarchy that it is
 *         part of. Ensure that any references to the actor or its children will not be used
 *         subsequent to this call. Remember that a light or clip plane should be disabled before it
 *         is removed and freed.
 *
 * \sa BrActorAllocate(), BrResFree().
 *
 * \par Example
 * \code{.c}
 * br_actor * t_pActor=BrActorAllocate(BR_ACTOR_MODEL,NULL);
 * BrActorFree(t_pActor);
 * \endcode
 */
void BR_PUBLIC_ENTRY      BrActorFree(br_actor *a);

/**
 * \brief Traverse an actor's descendants, searching for actors of a particular generation with a
 *        lineage and identifier matching a specified wild carded string.
 *
 * \param root    A pointer to an actor.
 * \param pattern Zero terminated character string containing the search pattern. This specifies the
 *                generation of actor required by separating descendants with slash characters
 *                ('/'), e.g. "/<Child>/<Grand-child>/<Great grand-child>/etc.". Identifiers of
 *                actors along the lineage may be matched exactly or in combination with the
 *                wild-card characters '*' (match any number of any characters) and '?' (match any
 *                single character). The pattern is fully permuted, e.g. "b*an?s" will match
 *                "bananas", "banks", and "bandstands".
 * \param actors  A pointer to a series of actor pointers. Storage must have been allocated (though
 *                not necessarily initialised) for at least max actor pointers.
 * \param max     The maximum number of matched actors to store at actors.
 *
 * \pre Between BrBegin() & BrEnd().
 *
 * \post Compares the actor's descendants' identifiers with the respective component of the search
 *       pattern. Terminates as soon as the number of complete lineage matches has reached max, or
 *       all possible matching lineages have been explored.
 *
 * \return The number of matched actors stored at actors. This will be between zero and max
 *         (inclusive).
 *
 * \remark The same as BrActorSearch() except that it allows matches of more than one actor. Note
 *         that the search pattern's first generation will be compared with the identifiers of the
 *         children of root – the identifier of root is not involved in the pattern matching
 *         process. To find actors irrespective of their lineage, use this function in combination
 *         with BrActorEnum().
 *
 * \sa BrActorSearch(), BrActorEnum().
 *
 * \par Example
 * \code{.c}
 * br_actor *t_aActorWheels[10];
 * int t_n;
 * t_n = BrActorSearchMany(pActorLorry, "Chassis/*Axle/Wheel?", t_aActorWheels, 10);
 * ASSERT(!(t_n%2));
 * \endcode
 */
br_uint_32 BR_PUBLIC_ENTRY BrActorSearchMany(br_actor *root, char *pattern, br_actor **actors, int max);
/**
 * \brief Traverse an actor's descendants, searching for an actor of a particular generation with a
 *        lineage and identifier matching a specified wild carded string.
 *
 * To find actors anywhere within a hierarchy will require the additional use of BrActorEnum().
 *
 * \param root    A pointer to an actor.
 * \param pattern Zero terminated character string containing the search pattern. This specifies the
 *                generation of actor required by separating descendants with slash characters
 *                (`/`), e.g. `/<Child>/<Grand-child>/<Great grand-child>/etc.`. Identifiers of
 *                actors along the lineage may be matched exactly or in combination with the
 *                wild-card characters '*' (match any number of any characters) and '?' (match any
 *                single character). The pattern is fully permuted, e.g. `b*an?s` will match
 *                "bananas", "banks", and "bandstands".
 *
 * \pre Between BrBegin() & BrEnd().
 *
 * \post Compares the actor's descendants' identifiers with the respective component of the search
 *       pattern. Terminates as soon as the complete lineage matches, or all possible matching
 *       lineages have been explored.
 *
 * \return A pointer to the first actor that was found whose lineage matched the search pattern, or
 *         NULL if there was no match.
 *
 * \remark Note that the search pattern's first generation will be compared with the identifiers of
 *         the children of root – the identifier of root is not involved in the pattern matching
 *         process. Where more than one match is possible, it is not recommended that any reliance
 *         be placed on which match is found first. No assumption should be made regarding how the
 *         actor hierarchy is traversed, e.g. whether branch by branch or generation by generation.
 *         To find an actor irrespective of its lineage, use this function in combination with
 *         BrActorEnum().
 *
 * \sa BrActorSearchMany(), BrActorEnum().
 *
 * \par Example
 * Given a root actor with identifier `Root`, children `Child1` and `Child2`, and child `Child1`
 * with grand-children `Grand-son1` and `Grand-daughter1`, and child `Child2` with grand-children
 * `Grand-son2` and `Grand-daughter2`:
 *
 * `Child?/Grand*1` would match either `Grand-son1` or `Grand-daughter1`.
 *
 * `Child1` would match `Child1`.
 *
 * `*`/ `*son?` would match either `Grand-son1` or `Grand-son2`.
 */
br_actor *BR_PUBLIC_ENTRY  BrActorSearch(br_actor *root, char *pattern);

/**
 * \brief Compute an actor hierarchy's bounding box, encompassing the bounding box of all models
 *        (including descendants).
 *
 * \param b  Non-NULL pointer to resulting bounding box.
 * \param ap Non-NULL pointer to actor whose bounding box is required.
 *
 * \pre Between BrBegin() & BrEnd().
 *
 * \post Obtains smallest bounding box (in this actor's co-ordinate space) that will contain any
 *       model or descendant model.
 *
 * \return The bounding box result pointer b is returned as supplied (for convenience).
 *
 * \remark The default model is used in cases where a model actor would inherit from an ancestor of
 *         the supplied actor.
 *
 * \sa BrBoundsToMatrix34(), BrScenePick3D(), br_bounds, br_model
 */
br_bounds *BR_PUBLIC_ENTRY BrActorToBounds(br_bounds *b, br_actor *ap);

/*
 * File operations
 */
/**
 * \brief Load a model.
 *
 * Note that it is not added to the registry.
 *
 * \param filename Name of the file containing the model to load.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined).
 *
 * \return Returns a pointer to the loaded model, or NULL if unsuccessful.
 *
 * \sa BrModelLoadMany(), BrModelSave(), BrModelAdd()
 */
br_model *BR_PUBLIC_ENTRY  BrModelLoad(const char *filename);
/**
 * \brief Save a model to a file.
 *
 * \param filename Name of the file to save the model to.
 * \param model    A pointer to a model.
 *
 * \post Writes the model to a file*.
 *
 * \return Returns NULL if the model could not be saved.
 *
 * \sa BrWriteModeSet()
 */
br_uint_32 BR_PUBLIC_ENTRY BrModelSave(const char *filename, br_model *model);
/**
 * \brief Load a number of models.
 *
 * Note that they are not added to the registry.
 *
 * \param filename Name of the file containing the models to load.
 * \param models   A non-NULL pointer to an array of pointers to models.
 * \param num      Maximum number of models to load.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined).
 *
 * \return Returns the number of models loaded successfully. The pointer array if supplied, is
 *         filled with pointers to the loaded models.
 *
 * \sa See BrModelFileCount() to determine the number of models in a file.
 */
br_uint_32 BR_PUBLIC_ENTRY BrModelLoadMany(const char *filename, br_model **models, br_uint_16 num);
/**
 * \brief Save a number of models to a file.
 *
 * \param filename Name of the file to save the models to.
 * \param models   A pointer to an array of pointers to models. If NULL, all registered models are
 *                 saved (irrespective of num).
 * \param num      Number of models to save.
 *
 * \post Writes the models to a file*.
 *
 * \return Returns the number of models saved successfully.
 *
 * \sa BrWriteModeSet()
 */
br_uint_32 BR_PUBLIC_ENTRY BrModelSaveMany(const char *filename, br_model **models, br_uint_16 num);
/**
 * \brief Locate a given file and count the number of models in it.
 *
 * \param filename Name of the file containing the models to count.
 * \param num      Pointer to the variable in which to store the number of models counted in the
 *                 file. If NULL, the file will still be located and appropriate success returned,
 *                 but no count will be made.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined). If a file is
 *       found, will count the number of models stored in it.
 *
 * \return Returns zero if the file was found (even if it is not a models file), non-zero otherwise.
 */
br_error BR_PUBLIC_ENTRY   BrModelFileCount(const char *filename, br_uint_16 *num);

/**
 * \brief Load a material.
 *
 * Note that it is not added to the registry.
 *
 * \param filename Name of the file containing the material to load.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined).
 *
 * \return Returns a pointer to the loaded material, or NULL if unsuccessful.
 *
 * \sa BrMaterialLoadMany(), BrMaterialSave(), BrMaterialAdd().
 */
br_material *BR_PUBLIC_ENTRY BrMaterialLoad(const char *filename);
/**
 * \brief Save a material to a file.
 *
 * \param filename Name of the file to save the material to.
 * \param material A pointer to a material.
 *
 * \post Writes the material to a file.
 *
 * \return Returns NULL if the material could not be saved.
 *
 * \note Any existing file of the same name is overwritten.
 *
 * \sa BrWriteModeSet()
 */
br_uint_32 BR_PUBLIC_ENTRY   BrMaterialSave(const char *filename, br_material *material);
/**
 * \brief Load a number of materials.
 *
 * Note that they are not added to the registry.
 *
 * \param filename  Name of the file containing the materials to load.
 * \param materials A non-NULL pointer to an array of pointers to materials.
 * \param num       Maximum number of materials to load.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined).
 *
 * \return Returns the number of materials loaded successfully. The pointer array supplied, is
 *         filled with pointers to the loaded materials.
 *
 * \sa To determine how many materials are stored in a file see BrMaterialFileCount().
 */
br_uint_32 BR_PUBLIC_ENTRY   BrMaterialLoadMany(const char *filename, br_material **materials, br_uint_16 num);
/**
 * \brief Save a number of materials to a file.
 *
 * \param filename  Name of the file to save the materials to.
 * \param materials A pointer to an array of pointers to materials. If NULL, all registered
 *                  materials are saved (irrespective of num).
 * \param num       Number of materials to save.
 *
 * \post Writes the materials to a file.
 *
 * \return Returns the number of materials saved successfully.
 *
 * \note Any existing file of the same name is overwritten.
 *
 * \sa BrWriteModeSet()
 */
br_uint_32 BR_PUBLIC_ENTRY   BrMaterialSaveMany(const char *filename, br_material **materials, br_uint_16 num);
/**
 * \brief Locate a given file and count the number of materials in it.
 *
 * \param filename Name of the file containing the materials to count.
 * \param num      Pointer to the variable in which to store the number of materials counted in the
 *                 file. If NULL, the file will still be located and appropriate success returned,
 *                 but no count will be made.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined). If a file is
 *       found, will count the number of materials stored in it.
 *
 * \return Returns zero if the file was found (even if it is not a materials file), non-zero
 *         otherwise.
 */
br_error BR_PUBLIC_ENTRY     BrMaterialFileCount(const char *filename, br_uint_16 *num);

/**
 * \brief Load a hierarchy of actors from a file.
 *
 * \param filename Name of the file previously created by BrActorSave() or BrActorSaveMany() (loads
 *                 the first hierarchy) (See Filing System Support).
 *
 * \pre Between BrBegin() & BrEnd(). Filing system available. Memory can be allocated.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined). Allocates a
 *       root actor, reads members (type, identifier, render_style, t) in the order they were saved
 *       in, attempts to set model with BrModelFind() using the model identifier read, attempts to
 *       set material with BrMaterialFind() using the material identifier read, allocates and sets
 *       the type specific actor data. Any child actors are read using this same process (thus
 *       recursively), and then added to this actor using BrActorAdd(). If a model or material was
 *       not found, the member is left at NULL.
 *
 * \return A pointer to the root actor of the loaded hierarchy, or NULL if unsuccessful.
 *
 * \remark Ensure that necessary models and materials are loaded and added into the registry before
 *         calling this function.
 *
 * \sa BrActorSave(), BrActorLoadMany().
 *
 * \par Example
 * \code{.c}
 * br_actor* t_pMyScene=BrActorLoad("MyScene");
 * \endcode
 */
br_actor *BR_PUBLIC_ENTRY  BrActorLoad(const char *filename);
/**
 * \brief Saves an actor and its descendants as a hierarchy.
 *
 * \param filename The name of the file under which the hierarchy should be saved (See Filing System
 *                 Support).
 * \param actor    A pointer to the effective root actor of the hierarchy to be saved.
 *
 * \pre Between BrBegin() & BrEnd(). Available filing system. Success depends on sufficient file
 *      space.
 *
 * \post If successful, the actor is written to a file* along with the identifier of its material
 *       (if non-NULL), the identifier of its model (if non-NULL), and any type specific data. This
 *       same process is then applied to each child in turn.
 *
 * \return The result is one if the hierarchy was saved successfully, and zero if not.
 *
 * \remark If the function fails (returns zero), the filing system can be considered returned to the
 *         state it was in just before the call, e.g. no file created. The hierarchical relationship
 *         between actors is recorded implicitly, no use is made of the actor's parent, next, prev,
 *         children, or identifier members. Note, that model and material data is not saved with
 *         actors, therefore this should be saved separately if required. It will need to be
 *         available to BrModelFind() and BrMaterialFind() (in the registry) before the actor
 *         hierarchy is loaded.
 *
 * \sa BrActorLoad(), BrActorSaveMany(), BrWriteModeSet().
 *
 * \par Example
 * \code{.c}
 * ...
 * return BrActorSave("MyScene",pActorScene);
 * \endcode
 *
 * \note Any existing file of the same name is overwritten.
 */
br_uint_32 BR_PUBLIC_ENTRY BrActorSave(const char *filename, br_actor *actor);
/**
 * \brief Load one or more hierarchies of actors from a file.
 *
 * \param filename Name of the file previously created by BrActorSaveMany() or BrActorSave(). See
 *                 Filing System Support for details of file naming.
 * \param actors   A pointer to a series of actor pointers. Storage must have been allocated (though
 *                 not necessarily initialised) for at least num actor pointers. See
 *                 BrActorFileCount() for a way of determining this value from a file.
 * \param num      The maximum number of matched actors to store at actors.
 *
 * \pre Between BrBegin() & BrEnd(). Filing system available. Memory can be allocated.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined). Stores a
 *       pointer to the root actor of each loaded hierarchy into the respective element of actors.
 *       See BrActorLoad() for details of how each hierarchy is loaded.
 *
 * \return The number of actor hierarchies loaded successfully. Will be between zero and num
 *         (inclusive).
 *
 * \remark Ensure that necessary models and materials are loaded and added into the registry before
 *         calling this function.
 *
 * \sa BrActorSaveMany(), BrActorLoad().
 *
 * \par Example
 * \code{.c}
 * br_actor* t_aScenes[5];
 * br_uint_32 t_u32;
 * t_u32=BrActorLoadMany("MyScenes",t_aScenes,5);
 * \endcode
 */
br_uint_32 BR_PUBLIC_ENTRY BrActorLoadMany(const char *filename, br_actor **actors, br_uint_16 num);
/**
 * \brief Saves a series of actors and their descendants as hierarchies.
 *
 * \param filename The name of the file under which the hierarchies should be saved (See Filing
 *                 System Support).
 * \param actors   A pointer to a series of num pointers to effective root actors of the hierarchies
 *                 to be saved. The actors do not necessarily need to be from unrelated hierarchies.
 * \param num      Number of hierarchies to save.
 *
 * \pre Between BrBegin() & BrEnd(). Available filing system. Success depends on sufficient file
 *      space.
 *
 * \post If successful for every one of the actor hierarchies, each is saved as described in
 *       BrActorSave(), except that all are written to the same file*.
 *
 * \return The function returns the number of complete actor hierarchies that were written to the
 *         file. This will be between zero and num (inclusive). Success is indicated by the return
 *         value being equal to num.
 *
 * \remark If the function fails (returns a value less than num), the filing system can be
 *         considered returned to the state it was in just before the call, e.g. no file created.
 *         Note, that model and material data is not saved with actors, therefore this should be
 *         saved separately if required. It will need to be available to BrModelFind() and
 *         BrMaterialFind() (in the registry) before any of the actor hierarchies are loaded.
 *
 * \sa BrActorLoadMany(), BrActorSave(), BrWriteModeSet().
 *
 * \par Example
 * \code{.c}
 * br_actor* apActorsInMyScene[N];
 * ...
 * if (BrActorSaveMany("MyScene",apActorsInMyScene,(br_uint_16)N)<N)
 * ...
 * \endcode
 *
 * \note Any existing file of the same name is overwritten.
 */
br_uint_32 BR_PUBLIC_ENTRY BrActorSaveMany(const char *filename, br_actor **actors, br_uint_16 num);
/**
 * \brief Locate a given file and count the number of actor hierarchies stored in it.
 *
 * \param filename Name of the file previously created by BrActorSave() or BrActorSaveMany() (See
 *                 Filing System Support).
 * \param num      Pointer to destination into which the number of actor hierarchies will be stored.
 *                 If NULL, the file will still be located and the success result returned, but no
 *                 count will be made.
 *
 * \pre Between BrBegin() & BrEnd(). Filing system available.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined). Counts the
 *       number of actor hierarchies stored in the file. If the file is not an actor file, the
 *       function is still successful, but would obviously produce a zero count of actors.
 *
 * \return Returns zero if successful (the file was found), non-zero otherwise.
 *
 * \sa BrActorLoad(), BrActorLoadMany().
 */
br_error BR_PUBLIC_ENTRY   BrActorFileCount(const char *filename, br_uint_16 *num);

/*
 * Lights
 */
/**
 * \brief Enable a light actor's effect as a light source within the scene.
 *
 * Light actors only affect the lighting of a scene if they are in the enabled state before
 * rendering.
 *
 * \param l A non-NULL pointer to a light actor.
 *
 * \remark By default, light actors are disabled. Light actors must be in the disabled state before
 *         they are removed from, relinked within, or added to a hierarchy. They must also be
 *         disabled before they are freed (or a parent is). For optimum performance, disable all
 *         lights known to have insignificant effect upon the scene. Some platforms are limited in
 *         the number simultaneously enabled lights that they can support.
 *
 * \sa BrLightDisable().
 */
void BR_PUBLIC_ENTRY BrLightEnable(br_actor *l);
/**
 * \brief Disable a light actor's effect as a light source within the scene.
 *
 * Light actors only affect the lighting of a scene if they are in the enabled state before
 * rendering.
 *
 * \param l A non-NULL pointer to a light actor.
 *
 * \remark By default, light actors are disabled. Light actors must be in the disabled state before
 *         they are removed from, relinked within, or added to a hierarchy. They must also be
 *         disabled before they are freed (or a parent is). For optimum performance, disable all
 *         lights known to have insignificant effect upon the scene. Some platforms are limited in
 *         the number of simultaneously enabled lights that they can support.
 *
 * \sa BrLightEnable().
 */
void BR_PUBLIC_ENTRY BrLightDisable(br_actor *l);

/*
 * Environment
 */
/**
 * \brief This function is relevant to model actors using environment mapping (set in their
 *        materials' flags member by using BR_MATF_ENVIRONMENT_I or BR_MATF_ENVIRONMENT_L).
 *
 * It set a new environment anchor for such model actors. By default, the reflective effect produced
 * by an environment map on the surface of a model appears to rotate with the model itself. If this
 * is unsatisfactory, the map can be anchored to an actor. If the actor is the root of a world
 * hierarchy, then reflection effects will appear more realistic.
 *
 * \param a A pointer to an actor to which all environment maps should be anchored. If NULL,
 *          environment maps are not anchored, but rotate with models.
 *
 * \return Returns a pointer to the old environment anchor.
 *
 * \remark Environment actors must be part of the rendered scene (a descendant of the effective
 *         root). If rendering a scene to produce a reflection environment map in another scene,
 *         then (unless the reflected scene also involves environment maps) the function should be
 *         called with NULL before the reflected scene is rendered, and then called with an
 *         appropriate anchor actor (that is part of the latter scene, in which the environment map
 *         is used).
 */
br_actor *BR_PUBLIC_ENTRY BrEnvironmentSet(br_actor *a);

/*
 * Clip planes
 */
/**
 * \brief Enable a clip plane.
 *
 * Such clip plane actors affect the contents of a scene (as long as they are enabled before
 * rendering).
 *
 * \param cp A non-NULL pointer to a clip plane actor.
 *
 * \remark By default, clip planes are disabled. Clip planes must be in the disabled state before
 *         they are removed from, relinked within, or added to a hierarchy. They must also be
 *         disabled before they are freed (or a parent is).
 */
void BR_PUBLIC_ENTRY BrClipPlaneEnable(br_actor *cp);
/**
 * \brief Disable a clip plane.
 *
 * \param cp A non-NULL pointer to a clip plane actor.
 *
 * \remark By default, clip planes are disabled. Clip planes must be in the disabled state before
 *         they are removed from, relinked within, or added to a hierarchy. They must also be
 *         disabled before they are freed (or a parent is).
 */
void BR_PUBLIC_ENTRY BrClipPlaneDisable(br_actor *cp);

/*
 * Horizon planes
 */
void BR_PUBLIC_ENTRY BrHorizonPlaneEnable(br_actor *h);
void BR_PUBLIC_ENTRY BrHorizonPlaneDisable(br_actor *h);

/*
 * Picking
 */
/**
 * \brief An application defined call-back function that is called by BrScenePick2D().
 *
 * It is called for each model actor whose bounds intersect the ray passing from a camera through a
 * particular screen pixel (see BrScenePick2D()).
 *
 * \param a        Pointer to model actor whose model bounds intersect the pick ray.
 * \param model    Pointer to the model attributed to the model actor whose bounds intersect the
 *                 pick ray (may be an inherited model).
 * \param material Pointer to the default material attributed to the model actor that may be used by
 *                 the model (may be an inherited material).
 * \param ray_pos  Pointer to a 3D vector giving a starting position of the pick ray in the model's
 *                 co-ordinate space. This position has no special significance, i.e. it is not
 *                 necessarily the position of the view-point.
 * \param ray_dir  Pointer to a 3D vector giving direction of ray from view-point through pixel in
 *                 the model's co-ordinate space. The magnitude has no special significance, i.e. it
 *                 is not necessarily the position of the model.
 * \param t_near   The co-efficient giving the position of the entry point of the pick ray into the
 *                 bounds of the intersecting model, in the model's co-ordinate space. To obtain
 *                 this position refer to the following formula. P_entry = P_ray + t_near d_ray
 * \param t_far    The co-efficient giving the position of the exit point of the pick ray out of the
 *                 bounds of the intersecting model, in the model's co-ordinate space. To obtain
 *                 this position refer to the following formula. P_exit = P_ray + t_far d_ray
 * \param arg      The corresponding value specified in the call of BrScenePick2D() that invoked
 *                 this call-back.
 *
 * \pre BRender has completed initialisation. A model's bounds intersect the pick ray. The order in
 *      which intersections are computed is undefined.
 *
 * \post Behaviour is up to the application.
 *
 * \return Return zero continue the search for intersecting model actors, non-zero to terminate. A
 *         non-zero value will be returned by BrScenePick2D().
 *
 * \remark For borderline cases, the pick ray is defined to be such that if a model's rendering
 *         would appear in the pixel then the model's bounds will intersect the ray. The precise
 *         sub-pixel position of the ray is consistent, but undefined.
 *
 * \sa br_model_custom_cbfn(), br_renderbounds_cbfn(), br_pick3d_cbfn()
 *
 * \par Example
 * Possible uses include:
 * \li Selection
 * \li Manipulation
 */
typedef int BR_CALLBACK br_pick2d_cbfn(br_actor *a, br_model *model, br_material *material, br_vector3 *ray_pos, br_vector3 *ray_dir,
                                       br_scalar t_near, br_scalar t_far, void *arg);

/**
 * \brief Traverse a world hierarchy, picking model actors from the scene by casting a ray through a
 *        given viewport pixel attached to a given camera.
 *
 * A call-back is invoked for each actor whose bounds intersect the ray. If the call-back returns a
 * non-zero value, traversal halts.
 *
 * \param world    A pointer to the root of a world hierarchy.
 * \param camera   A pointer to a camera actor.
 * \param viewport A pointer to the viewport through which the pick ray passes.
 * \param pick_x   Co-ordinates of viewport pixel through which the pick ray passes.
 * \param pick_y   Co-ordinates of viewport pixel through which the pick ray passes.
 * \param callback A pointer to a pick-2D call-back function.
 * \param arg      An optional argument to pass to the call-back function.
 *
 * \return If the call-back returns a non-zero value and traversal halts, that value is returned.
 *         Otherwise, zero is returned.
 *
 * \sa BrScenePick3D(), BrModelPick2D().
 */
int BR_PUBLIC_ENTRY BrScenePick2D(br_actor *world, br_actor *camera, br_pixelmap *viewport, int pick_x, int pick_y,
                                  br_pick2d_cbfn *callback, void *arg);

/**
 * \brief An application defined call-back function that is called by BrScenePick3D().
 *
 * It is called for each model actor whose bounds intersect a given bounds with respect to a
 * particular actor's co-ordinate space (see BrScenePick3D()).
 *
 * \param a         Pointer to model actor whose model bounds intersect the specified bounds.
 * \param model     Pointer to model whose bounds intersect the pick ray.
 * \param material  Pointer to the default material attributed to the model actor that may be used
 *                  by the model (may be an inherited material).
 * \param transform A pointer to a matrix transforming the intersecting actor's model co-ordinates
 *                  into the co-ordinate space of the reference actor (as supplied to
 *                  BrScenePick3D()).
 * \param bounds    A pointer to the original bounds (in the reference actor's co-ordinate space).
 * \param arg       The corresponding value specified in the call of BrScenePick3D() that invoked
 *                  this call-back.
 *
 * \pre BRender has completed initialisation. A model's bounds intersect the bounds in the reference
 *      actor's co-ordinate space.
 *
 * \post Behaviour is up to the application.
 *
 * \sa br_model_custom_cbfn(), br_renderbounds_cbfn(), br_pick2d_cbfn()
 *
 * \par Example
 * Possible uses include:
 * \li Collision detection (does not necessarily indicate the best method)
 * \li Volumetric selection
 */
typedef int BR_CALLBACK br_pick3d_cbfn(br_actor *a, br_model *model, br_material *material, br_matrix34 *transform, br_bounds *bounds, void *arg);

/**
 * \brief Traverse an actor hierarchy and invoke a call-back function for each model actor whose
 *        bounds intersect a given bounds in a given reference actor's space.
 *
 * If the call-back returns a non-zero value, traversal halts.
 *
 * \param world    A pointer to the root of a world hierarchy.
 * \param actor    A pointer to the reference actor.
 * \param bounds   A pointer to a bounds structure.
 * \param callback A pointer to a pick-3D call-back function.
 * \param arg      An optional argument to pass to the call-back function.
 *
 * \return If the call-back returns a non-zero value and traversal halts, that value is returned.
 *         Otherwise, zero is returned.
 *
 * \sa BrScenePick2D()
 */
int BR_PUBLIC_ENTRY BrScenePick3D(br_actor *world, br_actor *actor, br_bounds *bounds, br_pick3d_cbfn *callback, void *arg);

/**
 * \brief An application defined call-back function that is called by BrModelPick2D().
 *
 * It is called for each face that intersects a specific section of a particular ray in a model's
 * co-ordinate space.
 *
 * \param model    Pointer to the model to pick from.
 * \param material Pointer to the default material attributed to the model.
 * \param ray_pos  Pointer to a 3D vector giving a starting position of the pick ray in the model's
 *                 co-ordinate space.
 * \param ray_dir  Pointer to a 3D vector giving direction of ray from view-point through pixel in
 *                 the model's co-ordinate space.
 * \param t        The co-efficient giving the position of the intersection of the face with the
 *                 pick ray. The position is supplied in p.
 * \param face     The index of the face intersecting the ray. See faces of br_model.
 * \param edge     The index giving the edge nearest the intersection point. This is the edge from
 *                 vertices[edge] to vertices[(edge+1)%3]. See br_face.
 * \param vertex   The index giving the vertex nearest the intersection point. This is the vertex
 *                 vertices[vertex] in the br_face structure.
 * \param p        The position of the point at which the ray intersects the face, obtained using
 *                 the following formula. Pintersect = Pray + tintersect dray
 * \param map      The texture co-ordinates of the intersection point on the face. Apply the
 *                 (perspective correct) material's texture map transform to obtain the row and
 *                 column indices into the texture map.
 * \param arg      The corresponding value specified in the call of BrModelPick2D() that invoked
 *                 this call-back.
 *
 * \pre The function is called from within BrModelPick2D().
 *
 * \post Behaviour is up to the application.
 *
 * \return Return zero to continue the search for intersecting faces, non-zero to terminate. A
 *         non-zero value will be returned by BrModelPick2D().
 *
 * \sa br_model, br_pick2d_cbfn, br_pick3d_cbfn.
 *
 * \par Example
 * Possible uses include:
 * \li Face, Edge and Vertex Selection
 */
typedef int BR_CALLBACK br_modelpick2d_cbfn(br_model *model, br_material *material, br_vector3 *ray_pos, br_vector3 *ray_dir, br_scalar t,
                                            int face, int edge, int vertex, br_vector3 *p, br_vector2 *map, void *arg);

/**
 * \brief Casts a ray into a model and calls a call-back for all faces that intersect the ray.
 *
 * This can be used in conjunction with BrScenePick2D() to give face/edge/vertex picking.
 *
 * \param model    Non-NULL pointer to model.
 * \param material Non-NULL pointer to model's default material.
 * \param ray_pos  Non-NULL pointer to a 3D vector giving a starting position of a pick ray in the
 *                 model's co-ordinate space.
 * \param ray_dir  Non-NULL pointer to a 3D vector giving the direction of the pick ray.
 * \param t_near   Coefficients of ray_dir defining the section of the ray that should be
 *                 considered. Intersections outside this section are ignored. t_near should be less
 *                 than t_far.
 * \param t_far    Coefficients of ray_dir defining the section of the ray that should be
 *                 considered. Intersections outside this section are ignored. t_near should be less
 *                 than t_far.
 * \param callback Non-NULL pointer to call-back function to be called for each face intersecting
 *                 the specified ray section.
 * \param arg      An optional argument to pass to the call-back function.
 *
 * \pre Between BrBegin() and BrEnd(). BRender has completed initialisation. The specified model and
 *      material have been updated.
 *
 * \post The model's geometry is scanned to find faces that intersect the specified ray section. The
 *       supplied call-back is called for each intersecting face.
 *
 * \return If the call-back returns a non-zero value, traversal halts and that value is returned.
 *         Otherwise, zero is returned.
 *
 * \remark This function is typically used within BrScenePick2D() to refine selection further down
 *         to the face level. Perspective texture co-ordinates are also available to provide such
 *         things as 3D texture editing, or surface controls.
 *
 * \sa BrScenePick2D()
 *
 * \par Example
 * \code{.c}
 * int BR_CALLBACK MyPickNearestModelCallback(br_model* model, const br_material* material,
 *                                           const br_vector3* ray_pos, const br_vector3* ray_dir,
 *                                           br_scalar t, int f, int e, int v, const br_vector3* p,
 *                                           const br_vector2* map, my_pick_nearest* pn)
 * { if (t<pn->t)
 *   { pn->t=t;
 *     pn->actor=pn->temp_actor;
 *     pn->model=model;
 *     pn->material=material;
 *     pn->point=*p;
 *     pn->face=f;
 *     pn->edge=e;
 *     pn->vertex=v;
 *     pn->map=*map;
 *   }
 *   return 0;
 * }
 * int BR_CALLBACK MyPickNearestCallback(br_actor* actor, const br_model* model,
 *                                      const br_material* material, const br_vector3* ray_pos,
 *                                      const br_vector3* ray_dir, br_scalar t_near,
 *                                      br_scalar t_far, my_pick_nearest* pn)
 * { pn->temp_actor=actor;
 *   if (actor->model)
 *      BrModelPick2D(actor->model, material, ray_pos, ray_dir, t_near, t_far, MyPickNearestModelCallback, pn);
 *   return 0;
 * }
 * ...
 * BrScenePick2D(test_world, observer, back_buffer, PickCursor_X, PickCursor_Y, MyPickNearestCallback, &PickNearest);
 * if(PickNearest.model)
 * { PickNearest.model->faces[PickNearest.face].material=pick_material;
 *   BrModelUpdate(PickNearest.model,BR_MODU_ALL);
 * }
 * \endcode
 */
int BR_PUBLIC_ENTRY BrModelPick2D(br_model *model, br_material *material, br_vector3 *ray_pos, br_vector3 *ray_dir, br_scalar t_near,
                                  br_scalar t_far, br_modelpick2d_cbfn *callback, void *arg);

/*
 * Custom calback support
 */
/**
 * \brief Check a bounding box in the model space against the view volume and any clip-planes.
 *
 * \param bounds A pointer to a br_bounds structure giving the bounding box dimensions in the
 *               model's co-ordinate space.
 *
 * \return Returns one of the following: | Flag Symbol | Meaning | | --- | --- | | OSC_REJECT | The
 *         model is entirely outside the viewing volume | | OSC_PARTIAL | The model is partially
 *         within the viewing volume | | OSC_ACCEPT | The model is entirely within the viewing
 *         volume |
 */
br_uint_32 BR_PUBLIC_ENTRY BrOnScreenCheck(br_bounds3 *bounds);

/**
 * \brief Transform and project the origin in the model's co-ordinate space onto the screen.
 *
 * \param screen A pointer to the destination vector to receive the co-ordinates of the point in
 *               projected screen space (See Projected screen space). Note that the point is not
 *               necessarily on screen, i.e. inside the bounds of the output pixel map.
 *
 * \return Returns zero if the point is in front of the eye (the viewing pyramid).
 */
br_uint_8 BR_PUBLIC_ENTRY  BrOriginToScreenXY(br_vector2 *screen);
/**
 * \brief Transform and project the origin in the model's co-ordinate space onto the screen,
 *        generating x, y and z co-ordinates.
 *
 * If it is off-screen, it is not projected.
 *
 * \param screen A pointer to the destination vector to receive the co-ordinates of the point in
 *               projected screen space (See Projected screen space).
 *
 * \post The origin is checked against the viewing volume. If within, the equivalent screen and
 *       depth buffer co-ordinates are calculated, otherwise the function has no effect.
 *
 * \return If the co-ordinates have been placed in the destination vector the function returns zero,
 *         otherwise the origin is off-screen, in which case its out-code, made up from a
 *         combination of the flags in the following table is returned. | Out-Code Flag | Flag Value
 *         | Set When Origin Is | | --- | --- | --- | | OUTCODE_LEFT | 0x01 | Outside left plane | |
 *         OUTCODE_RIGHT | 0x02 | Outside right plane | | OUTCODE_TOP | 0x04 | Outside top plane | |
 *         OUTCODE_BOTTOM | 0x08 | Outside bottom plane | | OUTCODE_HITHER | 0x10 | Outside hither
 *         plane | | OUTCODE_YON | 0x20 | Outside yon plane |
 */
br_uint_32 BR_PUBLIC_ENTRY BrOriginToScreenXYZO(br_vector3 *screen);

/**
 * \brief Transform and project a single point in the model's co-ordinate space onto the screen.
 *
 * \param screen A pointer to the destination vector to receive the co-ordinates of the point in
 *               projected screen space (See Projected screen space). Note that the point is not
 *               necessarily on screen, i.e. inside the bounds of the output pixel map.
 * \param point  A pointer to the source vector containing the co-ordinates of the point to project.
 *
 * \return Returns zero if the point is in front of the eye (the viewing pyramid).
 */
br_uint_8 BR_PUBLIC_ENTRY  BrPointToScreenXY(br_vector2 *screen, br_vector3 *point);
/**
 * \brief Transform and project a point in the model's co-ordinate space onto the screen, generating
 *        x, y and z co-ordinates.
 *
 * If it is off-screen, it is not projected.
 *
 * \param screen A pointer to the destination vector to receive the co-ordinates of the point in
 *               projected screen space (See Projected screen space).
 * \param point  A pointer to the source vector.
 *
 * \post The point is checked against the viewing volume. If within, the equivalent screen and depth
 *       buffer co-ordinates are calculated, otherwise the function has no effect.
 *
 * \return If the co-ordinates have been placed in the destination vector the function returns zero,
 *         otherwise the point is off-screen, in which case its out-code, made up from a combination
 *         of the flags in the following table is returned. | Out-Code Flag | Flag Value | Set When
 *         Point Is | | --- | --- | --- | | OUTCODE_LEFT | 0x01 | Outside left plane | |
 *         OUTCODE_RIGHT | 0x02 | Outside right plane | | OUTCODE_TOP | 0x04 | Outside top plane | |
 *         OUTCODE_BOTTOM | 0x08 | Outside bottom plane | | OUTCODE_HITHER | 0x10 | Outside hither
 *         plane | | OUTCODE_YON | 0x20 | Outside yon plane |
 *
 * \sa BrOriginToScreenXYZO()
 */
br_uint_32 BR_PUBLIC_ENTRY BrPointToScreenXYZO(br_vector3 *screen, br_vector3 *point);

/**
 * \brief Transform and project a number of points in the model's co-ordinate system onto the
 *        screen.
 *
 * \param screens A pointer to an array of destination vectors to receive the co-ordinates of each
 *                point in projected screen space (See Projected screen space). Note that the points
 *                are not necessarily on screen, i.e. inside the bounds of the output pixel map.
 * \param points  A pointer to an array of source vectors containing co-ordinates of points in model
 *                space.
 * \param npoints Number of points to process.
 *
 * \post Equivalent to a call of BrPointToScreenXY() for each screen and point vector.
 */
void BR_PUBLIC_ENTRY BrPointToScreenXYMany(br_vector2 *screens, br_vector3 *points, br_uint_32 npoints);
/**
 * \brief Transform and project a number of points in the model's co-ordinate space onto the screen,
 *        generating a series x, y and z co-ordinates and out-codes.
 *
 * All those that are off-screen are not projected.
 *
 * \param screens  A pointer to a list of destination vectors, each of which will receive
 *                 co-ordinates of each point in projected screen space (See Projected screen
 *                 space).
 * \param outcodes A pointer to an array of out-codes for each point. If the co-ordinates have been
 *                 placed in the corresponding destination vector the out-code will be zero,
 *                 otherwise the point is off-screen, in which case only its out-code (see
 *                 BrPointToScreenXYZO()) is stored.
 * \param points   A pointer to an array of source vectors containing the points to be projected.
 * \param npoints  Number of points.
 *
 * \post Equivalent to a call of BrPointToScreenXYZO() for each point. Only co-ordinates of points
 *       in the viewing volume are written to corresponding elements of screens.
 *
 * \sa BrOriginToScreenXYZO()
 */
void BR_PUBLIC_ENTRY BrPointToScreenXYZOMany(br_vector3 *screens, br_uint_32 *outcodes, br_vector3 *points, br_uint_32 npoints);

/**
 * \brief Generate prelit lighting values for the vertices of a given model, using the current
 *        rendering's lighting set-up.
 *
 * \param model            Non-NULL pointer to model to calculate prelit values for.
 * \param default_material Non-NULL pointer to default material to use for model.
 * \param root             Pointer to root actor of scene, e.g. as supplied to
 *                         BrZbSceneRenderBegin(). If this function is called from within a custom
 *                         model call-back, NULL may be used to indicate that the root effective for
 *                         the current actor is to be used.
 * \param a                Pointer to actor defining the required co-ordinate space for the model to
 *                         be prelit. If this function is called from within a custom model
 *                         call-back, NULL may be used to indicate that the current actor should is
 *                         to be used.
 *
 * \pre Between BrBegin() and BrEnd(). Currently rendering, e.g. between BrZbSceneRenderBegin() and
 *      BrZbSceneRenderEnd().
 *
 * \post Works out lighting for the supplied model as though it were attached to the supplied actor.
 *       Modifies the appropriate prelit members of each vertex in accordance with the model and its
 *       material.
 *
 * \remark Generally useful for scenes having little change in lighting. Lights may be enabled for
 *         the first frame, this function called to pre-light various models and then most or all
 *         lighting disabled for performance in subsequent frames. Note that each model whose
 *         vertices are so set will need a BrModelUpdate() applied before the next rendering.
 *
 * \sa br_vertex
 */
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
/**
 * \brief This function is provided as a suitable function to supply to BrModelFindHook().
 *
 * \param name The name supplied to BrModelFind().
 *
 * \post Attempts to load the model from the filing system using name as the filename. Searches in
 *       current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if
 *       defined). If successful, sets this name as the identifier of the loaded model and adds the
 *       model to the registry.
 *
 * \return Returns a pointer to the model, if found, else NULL.
 *
 * \par Example
 * \code{.c}
 * BrModelFindHook(BrModelFindFailedLoad);
 * \endcode
 */
br_model *BR_CALLBACK    BrModelFindFailedLoad(const char *name);
/**
 * \brief This function is provided as a suitable function to supply to BrMaterialFindHook().
 *
 * \param name The name supplied to BrMaterialFind().
 *
 * \post Attempts to load the material from the filing system using name as the filename. Searches
 *       current directory, if not found tries, in order, the directories listed in BRENDER_PATH (if
 *       defined). If successful, sets this name as the identifier of the loaded material and adds
 *       the material to the registry.
 *
 * \return Returns a pointer to the material, if found, else NULL.
 *
 * \par Example
 * \code{.c}
 * BrMaterialFindHook(BrMaterialFindFailedLoad);
 * \endcode
 */
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
/**
 * \brief Render a model actor as part of the current Z-Buffer rendering.
 *
 * \param actor      The pointer to the model actor referencing the supplied model (must not be
 *                   NULL).
 * \param model      The pointer to the model to be rendered (must not be NULL).
 * \param material   A pointer to the material to use for rendering faces that don't specify a
 *                   material (must not be NULL).
 * \param style      The rendering style to use for rendering the model (any defined style may be
 *                   specified, even BR_RSTYLE_NONE).
 * \param on_screen  A flag specifying whether the model is either partially or completely on-screen
 *                   (either OSC_PARTIAL or OSC_ACCEPT). If OSC_ACCEPT is specified, even off-screen
 *                   faces will be rendered. See BrOnScreenCheck().
 * \param use_custom If non-zero, invoke the specified model's custom call-back function. This is
 *                   typically zero if the same model is specified.
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

/**
 * \brief Close down the Z-Sort renderer.
 *
 * \pre Between BrBegin() & BrEnd().
 *      BrZsBegin() has been called, and BrZsEnd() has not yet been called since.
 *      The registry is empty.
 *      No other rendering engine is currently enabled.
 *
 * \post Releases resources used by the Z-Buffer renderer.
 *
 * \sa BrZsBegin()
 */
void BR_PUBLIC_ENTRY BrZsEnd(void);

/**
 * \brief Set up a new scene to be rendered using the Z-Sort renderer, processing the camera,
 *        lights and environment.
 *
 * \param world         A non-NULL pointer to the root actor of a scene.
 * \param camera        A non-NULL pointer to a camera actor that is a descendant of the root actor.
 * \param colour_buffer A non-NULL pointer to the pixel map to render the scene info, whose type is
 *                      \p colour_type as supplied to BrZsBegin().
 *
 * \pre Between BrBegin() & BrEnd(). Between BrZsBegin() & BrZsEnd(). Not currently rendering.
 *
 * \post Enter rendering state, prepare for destination buffer, preprocess view, screen and
 *       environment transforms, preprocess enabled lights, handle environment actor, preprocess
 *       enabled clip planes.
 *
 * \remark The colour buffer should not be a texture map (or even a shade table), though it can
 *         of course be subsequently added as such once the rendering has completed.
 *
 * \sa BrZsSceneRenderAdd(), BrZsSceneRenderEnd(), BrZsRenderBoundsCallbackSet(),
 *     BrZsPrimitiveCallbackSet(), BrZsModelRender().
 */
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
/**
 * \brief Render a model actor as part of the current Z-Sort rendering.
 *
 * \param actor       The pointer to the model actor referencing the supplied model (must not be
 *                    NULL).
 * \param model       The pointer to the model to be rendered (must not be NULL).
 * \param material    A pointer to the material to use for rendering faces that don't specify a
 *                    material (must not be NULL).
 * \param order_table A pointer to the order table the model's primitives should be inserted into
 *                    (must not be NULL).
 * \param style       The rendering style to use for rendering the model (any defined style may be
 *                    specified, even BR_RSTYLE_NONE).
 * \param on_screen   A flag specifying whether the model is either partially or completely
 *                    on-screen (either OSC_PARTIAL or OSC_ACCEPT). If OSC_ACCEPT is specified, even
 *                    off-screen faces will be rendered. See BrOnScreenCheck().
 * \param use_custom  If non-zero, invoke the specified model's custom call-back function. This is
 *                    typically zero if the same model is specified.
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

/**
 * \brief Allocate a new order table.
 *
 * \param size  Number of buckets in order table (See br_uint_16 size).
 * \param flags Order table flags (See br_uint_32 flags).
 * \param type  Order table type (See br_uint_16 type).
 *
 * \pre Between BrZsBegin() & BrZsEnd(). Memory can be allocated.
 *
 * \post A br_order_table data structure is allocated (using BrResAllocate(), from the
 *       BR_MEMORY_RENDER_DATA memory class) and initialised (See br_order_table, Initialisation).
 *       The z range and sort z are initialised to zero.
 *
 * \return A pointer to the new br_order_table data structure.
 *
 * \remark Ensure min_z, max_z and sort_z are set appropriately if they will not be initialised
 *         automatically, i.e. if the BR_ORDER_TABLE_LEAVE_BOUNDS flag is set.
 *
 * \sa BrZsOrderTableFree().
 */
struct br_order_table *BR_PUBLIC_ENTRY BrZsOrderTableAllocate(br_uint_16 size, br_uint_32 flags, br_uint_16 type);
/**
 * \brief Free an order table.
 *
 * \param order_table A non-NULL pointer to an order table previously allocated using
 *                    BrZsOrderTableAllocate().
 *
 * \pre Between BrZsBegin() & BrZsEnd(). Not currently rendering.
 *
 * \post Uses BrResFree() to release storage.
 *
 * \remark Ensure that no actor is currently using the order table before using this function. Do
 *         not attempt to free an active order table, such as is passed as an argument to a
 *         rendering call-back function, e.g. CBFnPrimitive() 316.
 *
 * \sa BrZsOrderTableAllocate(), BrResFree().
 */
void BR_PUBLIC_ENTRY                   BrZsOrderTableFree(br_order_table *order_table);
/**
 * \brief Given a model actor, set the order table into which its primitives will be sorted.
 *
 * \param actor       A non-NULL pointer to a model actor.
 * \param order_table A pointer to an order table. NULL if the actor is to inherit an order table
 *                    (the default condition).
 *
 * \post Assigns the specified order table to the actor, replacing the current one if any. Be very
 *       careful, if assigning an order table to more than one model actor (whether explicitly or by
 *       inheritance). In such cases judicious use of the order table flags, such as
 *       BR_ORDER_TABLE_NEW_BOUNDS, may be required within a custom model call-back function.
 *
 * \return Returns order_table as supplied (for convenience).
 *
 * \remark Only useful to model actors involved during rendering by the Z-Sort renderer.
 *
 * \sa BrZsActorOrderTableGet().
 */
struct br_order_table *BR_PUBLIC_ENTRY BrZsActorOrderTableSet(struct br_actor *actor, struct br_order_table *order_table);
/**
 * \brief Given a model actor, obtain a pointer to the currently set order table (into which its
 *        primitives will be sorted).
 *
 * \param actor A non-NULL pointer to a model actor.
 *
 * \return Returns the order table currently assigned to the specified actor. NULL is returned if no
 *         order table is explicitly assigned, i.e. an order table is inherited.
 *
 * \remark Only useful to model actors involved during rendering by the Z-Sort renderer.
 *
 * \sa BrZsActorOrderTableSet().
 */
struct br_order_table *BR_PUBLIC_ENTRY BrZsActorOrderTableGet(struct br_actor *actor);
/**
 * \brief Clear an order table, re-initialising its table of pointers to primitives.
 *
 * \param order_table A non-NULL pointer to the order table to be cleared.
 *
 * \pre Between BrZsBegin() & BrZsEnd().
 *
 * \post The series of void pointers pointed to by table are reset to NULL. Some private flags may
 *       be modified.
 *
 * \remark Note that order tables are marked for clearing after rendering and allocation. Upon entry
 *         to a primitive call-back, the order table will have been cleared, and may already contain
 *         primitives. Given automatic clearing, the use of this function is not generally required.
 *
 * \sa BrZsActorOrderTableGet().
 */
struct br_order_table *BR_PUBLIC_ENTRY BrZsOrderTableClear(struct br_order_table *order_table);

/**
 * \brief Insert a rendering primitive into an order table.
 *
 * \param order_table Non-NULL pointer to order table in which to insert primitive (supplied as
 *                    argument to CBFnPrimitive() 316).
 * \param primitive   Non-NULL pointer to primitive to insert (supplied as argument to
 *                    CBFnPrimitive() 316).
 * \param bucket      Bucket of order table in which to insert primitive (between 0 and
 *                    order_table->size-1). See BrZsPrimitiveBucketSelect().
 *
 * \pre Between BrZsSceneRenderBegin() & BrZsSceneRenderEnd(). Within a primitive call-back
 *      function.
 *
 * \post The primitive is inserted into the order table in the specified bucket.
 *
 * \remark This function is only provided to customise the ordering of primitives in a particular
 *         order table. It is not intended to permit automatic generation of primitives. Note that
 *         order tables are marked for clearing after rendering and allocation. Upon entry to a
 *         primitive call-back, the order table will have been cleared, and may already contain
 *         primitives. The state of other order tables is undefined.
 *
 * \sa BrZsModelRender().
 */
void BR_PUBLIC_ENTRY BrZsOrderTablePrimitiveInsert(struct br_order_table *order_table, struct br_primitive *primitive, br_uint_16 bucket);

/**
 * \brief Determine into which bucket a primitive would be placed given a particular sorting method.
 *
 * \param z         A non-NULL pointer to the z values corresponding to each vertex. This should
 *                  point to as many values as are indicated by the primitive type pr_type.
 * \param type      The primitive type (see br_primitive).
 * \param min_z     Depth range of buckets (use BrZsScreenZToDepth() if wishing to work with screen
 *                  ordinates).
 * \param max_z     Depth range of buckets (use BrZsScreenZToDepth() if wishing to work with screen
 *                  ordinates).
 * \param size      Number of buckets (see br_order_table).
 * \param sort_type How the bucket should be determined (see br_order_table).
 *
 * \return The index of the bucket as would be passed to BrZsOrderTablePrimitiveInsert().
 *
 * \remark Although the calculation performed by this function is trivial, it is provided to allow
 *         applications to ensure that the same bucket selection occurs in primitive call-back
 *         functions as would occur otherwise.
 */
br_uint_16 BR_PUBLIC_ENTRY BrZsPrimitiveBucketSelect(br_scalar *z, br_uint_16 type, br_scalar min_z, br_scalar max_z, br_uint_16 size,
                                                     br_uint_16 sort_type);
/**
 * \brief Enable the use of a primary order table, between whose buckets all other order tables are
 *        then rendered.
 *
 * By default, no primary order table is enabled.
 *
 * \param order_table A pointer to the order table to be used as the primary order table. If NULL is
 *                    supplied, the default order table will be used.
 *
 * \pre Between BrZsBegin() & BrZsEnd().
 *
 * \post The Z sort renderer will use the specified order table as a primary order table, making
 *       other order tables secondary. All secondary order tables are rendered sequentially, as
 *       appropriate, between the rendering of the buckets of the primary order table. Secondary
 *       order tables whose sort_z lies further away than max_z are rendered first (in order of
 *       their sort_z). Then the furthest bucket of the primary order table is rendered, followed by
 *       the secondary order tables whose sort_z lies within that bucket (in order of their sort_z).
 *       This continues for each bucket of the primary order table. Finally, the secondary order
 *       tables whose sort_z is in front of the nearest bucket of the primary order table are
 *       rendered (in order of their sort_z).
 *
 * \remark Note that the primary order table facility only provides a relatively coarse way of
 *         merging the ordering of primitives (or buckets) in secondary order tables with those in
 *         the primary order table. Nevertheless, with a bit of care, it often gives satisfactory
 *         results for a wide range of scenes, with relatively straightforward allocation of order
 *         tables.
 *
 * \sa BrZsOrderTablePrimaryDisable().
 */
void BR_PUBLIC_ENTRY       BrZsOrderTablePrimaryEnable(struct br_order_table *order_table);
/**
 * \brief Disable the use of a primary order table.
 *
 * Note that the primary order table is disabled by default.
 *
 * \pre Between BrZsBegin() & BrZsEnd().
 *
 * \post Order tables are rendered sequentially in order of their sort_z.
 *
 * \remark The simpler Z sort algorithm that this function engages, generally requires a deal of
 *         care and effort to ensure correct rendering order, especially in complex scenes. The
 *         results generally produce a more reliable ordering than with a primary order table, as so
 *         much more care is needed.
 *
 * \sa BrZsOrderTablePrimaryEnable().
 */
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
