/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: fmt_p.h 1.1 1997/12/10 16:41:00 jon Exp $
 * $Locker: $
 *
 * Function prototypes for foreign file format support
 */
#ifndef _FMT_P_H_
#define _FMT_P_H_

#ifndef _NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 3D Studio .3DS files
 */
br_fmt_results *BR_PUBLIC_ENTRY BrFmt3DSLoad(const char *name, br_fmt_options *fmt_options, br_3ds_options *options);

/**
 * \brief Import 3D Studio models (geometry only). The models are neither
 *        updated nor registered.
 *
 * Searches for \p name, if no path is specified with file, looks in the current
 * directory, if not found tries, in order, the directories listed in
 * BRENDER_PATH (if defined).
 *
 * \param name       Name of the file containing the models.
 * \param mtable     A pointer to an array of pointers to models, which will
 *                   be filled as they are imported. If NULL, the models are
 *                   still imported, but must be referenced subsequently by
 *                   name.
 * \param max_models Maximum number of models to import.
 *
 * \return Returns the number of models successfully imported.
 */
br_uint_32 BR_PUBLIC_ENTRY BrFmtASCLoad(const char *name, br_model **mtable, br_uint_16 max_models);

/**
 * \brief Import a model expressed in the Neutral File Format. The model is
 *        neither updated nor registered.
 *
 * Searches for \p name, if no path is specified with file, looks in the current
 * directory, if not found tries, in order, the directories listed in
 * BRENDER_PATH (if defined).
 *
 * \param name Name of the file containing the model.
 *
 * \return Returns a pointer to the imported model, or NULL if it could not
 *         be imported.
 */
br_model *BR_PUBLIC_ENTRY BrFmtNFFLoad(const char *name);

/*
 * Script files
 */
/**
 * \brief Load a number of materials from a material script.
 *
 * \param filename  Name of the file containing a number of concatenated material script entries.
 * \param materials A non-NULL pointer to an array of pointers to materials.
 * \param num       Maximum number of materials to load.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined).
 *
 * \return Return the number of materials loaded successfully. The pointer array if supplied, is
 *         filled with pointers to the loaded materials.
 */
br_uint_32 BR_PUBLIC_ENTRY   BrFmtScriptMaterialLoadMany(const char *filename, br_material **materials, br_uint_16 num);
/**
 * \brief Load a material from a material script.
 *
 * Note that all maps and tables in a script should be already loaded and registered. If not, this
 * function can be combined with BrMapFindHook() and BrTableFindHook() to facilitate rapid setup of
 * materials with textures.
 *
 * \param filename Name of the file containing the material script.
 *
 * \post Searches for filename, if no path specified with file looks in current directory, if not
 *       found tries, in order, the directories listed in BRENDER_PATH (if defined).
 *
 * \return Returns a pointer to the loaded material, or NULL if unsuccessful.
 *
 * \par Example
 * \code{.c}
 * Material scripts are formatted as follows:
 *   # Comment
 *   #
 *   # Fields may be given in any order or omitted
 *   # (a sensible default will be supplied)
 *   # Extra white spaces are ignored
 *   material=
 *   [ name = "foo";
 *     flags=
 *     [
 *     light,prelit,smooth,environment,environment_local,perspective,
 *        decal,always_visible,two_sided,force_z_0
 *     ];
 *     colour = [10,10,20];
 *     ambient = 0.10;
 *     diffuse = 0.70;
 *     specular = 0.40;
 *     power = 30;
 *     map_transform = [[1,0], [0,1], [0,0]];
 *     index_base = 0;
 *     index_range = 0;
 *     colour_map = "brick";
 *     index_shade = "shade.tab";
 *   ];
 * \endcode
 */
br_material *BR_PUBLIC_ENTRY BrFmtScriptMaterialLoad(const char *filename);

br_uint_32 BR_PUBLIC_ENTRY BrFmtScriptMaterialSaveMany(const char *filename, br_material **materials, br_uint_16 num);
br_uint_32 BR_PUBLIC_ENTRY BrFmtScriptMaterialSave(const char *filename, br_material *ptr);

/**
 * \brief Load a pixel map in the BMP format.
 *
 * Searches for \p name, if no path specified with file, looks in current
 * directory, if not found tries, in order, the directories listed in
 * BRENDER_PATH (if defined).
 *
 * \param name  Name of the file containing the pixel map.
 * \param flags Either BR_PMT_RGBX_888 or BR_PMT_RGBA_8888, when the source
 *              pixel map uses 32 bits per pixel. Zero otherwise.
 *
 * \return Returns a pointer to the loaded pixel map.
 */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtBMPLoad(const char *name, br_uint_32 flags);

/**
 * \brief Load a pixel map in the TGA format.
 *
 * Searches for \p name, if no path specified with file, looks in current
 * directory, if not found tries, in order, the directories listed in
 * BRENDER_PATH (if defined).
 *
 * \param name  Name of the file containing the pixel map.
 * \param flags Either BR_PMT_RGBX_888 or BR_PMT_RGBA_8888, when the source
 *              pixel map uses 32 bits per pixel. Zero otherwise.
 *
 * \return Returns a pointer to the loaded pixel map.
 */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtTGALoad(const char *name, br_uint_32 flags);

/**
 * \brief Load a pixel map in the GIF format.
 *
 * Searches for filename, if no path specified with file, looks in current
 * directory, if not found tries, in order, the directories listed in
 * BRENDER_PATH (if defined).
 *
 * \param name  Name of the file containing the pixel map.
 * \param flags Either BR_PMT_RGBX_888 or BR_PMT_RGBA_8888 when the source
 *              pixel map uses 32 bits per pixel. Zero otherwise.
 *
 * \return Returns a pointer to the loaded pixel map, or NULL if
 *         unsuccessful.
 */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtGIFLoad(const char *name, br_uint_32 flags);

/**
 * \brief Load a pixel map in the IFF format.
 *
 * Searches for filename, if no path specified with file looks in current
 * directory, if not found tries, in order, the directories listed in
 * BRENDER_PATH (if defined).
 *
 * \param name  Name of the file containing the pixel map.
 * \param flags Either BR_PMT_RGBX_888 or BR_PMT_RGBA_8888, when the source
 *              pixel map uses 32 bits per pixel. Zero otherwise.
 *
 * \return Returns a pointer to the loaded pixel map.
 */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtIFFLoad(const char *name, br_uint_32 flags);

/*
 * .VUE files
 */
br_vue *BR_PUBLIC_ENTRY BrVueAllocate(int nframes, int ntransforms);
void BR_PUBLIC_ENTRY    BrVueFree(br_vue *vue);
void BR_PUBLIC_ENTRY    BrLoadVUE(const char *file_name, br_actor *root, br_vue *vue);
void BR_PUBLIC_ENTRY    BrApplyVue(br_vue *vue, br_actor *actors);

/*
 * .PNG files
 */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtPNGLoad(const char *name, br_uint_32 flags);

/*
 * .JPG files
 */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtJPGLoad(const char *name, br_uint_32 flags);

/*
 * Image files
 */
br_uint_32 BR_PUBLIC_ENTRY BrFmtImageSave(const char *name, br_pixelmap *pm, br_uint_8 type);

/*
 * .PCX files
 */
br_pixelmap *BR_PUBLIC_ENTRY BrFmtPCXLoad(const char *name, br_uint_32 flags);

/*
 * .GLTF files
 */
br_error BR_PUBLIC_ENTRY BrFmtGLTFActorSaveMany(const char *name, br_actor **actors, br_size_t num);
br_error BR_PUBLIC_ENTRY BrFmtGLTFActorSave(const char *name, br_actor *actor);

br_error BR_PUBLIC_ENTRY BrFmtGLTFModelSaveMany(const char *name, br_model **models, br_size_t num);
br_error BR_PUBLIC_ENTRY BrFmtGLTFModelSave(const char *name, br_model *model);

br_fmt_results *BR_PUBLIC_ENTRY BrFmtGLTFActorLoadMany(const char *name, const br_gltf_options *options);

#ifdef __cplusplus
};
#endif
#endif
#endif
