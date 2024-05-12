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
BR_API br_fmt_results *BR_PUBLIC_ENTRY BrFmt3DSLoad(const char *name, br_fmt_options *fmt_options, br_3ds_options *options);

/*
 * 3D Studio .ASC files
 */
BR_API br_uint_32 BR_PUBLIC_ENTRY BrFmtASCLoad(const char *name, br_model **mtable, br_uint_16 max_models);

/*
 * Eric Haines' NFF format
 */
BR_API br_model *BR_PUBLIC_ENTRY BrFmtNFFLoad(const char *name);

/*
 * Script files
 */
BR_API br_uint_32 BR_PUBLIC_ENTRY   BrFmtScriptMaterialLoadMany(const char *filename, br_material **materials, br_uint_16 num);
BR_API br_material *BR_PUBLIC_ENTRY BrFmtScriptMaterialLoad(const char *filename);

BR_API br_uint_32 BR_PUBLIC_ENTRY BrFmtScriptMaterialSaveMany(const char *filename, br_material **materials, br_uint_16 num);
BR_API br_uint_32 BR_PUBLIC_ENTRY BrFmtScriptMaterialSave(const char *filename, br_material *ptr);

/*
 * Windows .BMP files
 */
BR_API br_pixelmap *BR_PUBLIC_ENTRY BrFmtBMPLoad(const char *name, br_uint_32 flags);

/*
 * .TGA files
 */
BR_API br_pixelmap *BR_PUBLIC_ENTRY BrFmtTGALoad(const char *name, br_uint_32 flags);

/*
 * .GIF files
 */
BR_API br_pixelmap *BR_PUBLIC_ENTRY BrFmtGIFLoad(const char *name, br_uint_32 flags);

/*
 * .IFF/.LBM files
 */
BR_API br_pixelmap *BR_PUBLIC_ENTRY BrFmtIFFLoad(const char *name, br_uint_32 flags);

/*
 * .VUE files
 */
BR_API br_vue *BR_PUBLIC_ENTRY BrVueAllocate(int nframes, int ntransforms);
BR_API void BR_PUBLIC_ENTRY    BrVueFree(br_vue *vue);
BR_API void BR_PUBLIC_ENTRY    BrLoadVUE(const char *file_name, br_actor *root, br_vue *vue);
BR_API void BR_PUBLIC_ENTRY    BrApplyVue(br_vue *vue, br_actor *actors);

/*
 * .PNG files
 */
BR_API br_pixelmap *BR_PUBLIC_ENTRY BrFmtPNGLoad(const char *name, br_uint_32 flags);

/*
 * .JPG files
 */
BR_API br_pixelmap *BR_PUBLIC_ENTRY BrFmtJPGLoad(const char *name, br_uint_32 flags);

/*
 * Image files
 */
BR_API br_uint_32 BR_PUBLIC_ENTRY BrFmtImageSave(const char *name, br_pixelmap *pm, br_uint_8 type);

/*
 * .PCX files
 */
BR_API br_pixelmap *BR_PUBLIC_ENTRY BrFmtPCXLoad(const char *name, br_uint_32 flags);

/*
 * .GLTF files
 */
BR_API br_error BrFmtGLTFSave(const char *name, const br_model *model, void *res);

#ifdef __cplusplus
};
#endif
#endif
#endif
