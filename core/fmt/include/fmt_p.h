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
br_uint_32 BR_PUBLIC_ENTRY   BrFmtScriptMaterialLoadMany(const char *filename, br_material **materials, br_uint_16 num);
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

/*
 * .IFF/.LBM files
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
