/*
 * Prototypes for functions internal to driver
 */
#ifndef _DRV_IP_H_
#define _DRV_IP_H_

#ifndef NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

/*
 * device.c
 */
br_device *DeviceGL1xAllocate(const char *identifier, const char *arguments);

/*
 * driver.c
 */
// BrDrv1GL1xBegin declared in brglrend_p.h

/*
 * ext_procs.c
 */
br_error            DevicePixelmapGL1xExtCreateContext(br_device_pixelmap *self, br_device_gl_context_info *info);
void                DevicePixelmapGL1xExtDeleteContext(br_device_pixelmap *self, void *ctx);
br_error            DevicePixelmapGL1xExtMakeCurrent(br_device_pixelmap *self, void *ctx);
void                DevicePixelmapGL1xExtSwapBuffers(br_device_pixelmap *self);
GLADuserptrloadfunc DevicePixelmapGL1xExtGetGetProcAddress(br_device_pixelmap *self);
br_error            DevicePixelmapGL1xExtResize(br_device_pixelmap *self, br_int_32 w, br_int_32 h);
void                DevicePixelmapGL1xExtPreSwap(br_device_pixelmap *self, GLuint fbo);
void                DevicePixelmapGL1xExtFree(br_device_pixelmap *self);
br_error            DevicePixelmapGL1xExtHandleWindowEvent(br_device_pixelmap *self, void *arg);

/*
 * devpixmp.c
 */
br_rectangle         DevicePixelmapGL1xGetViewport(const br_device_pixelmap *pm);
const GladGLContext *DevicePixelmapGL1xGetGLContext(br_device_pixelmap *self);
void                 DevicePixelmapGL1xIncRef(br_device_pixelmap *self);
void                 DevicePixelmapGL1xDecRef(br_device_pixelmap *self);

/*
 * devpmsub.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_gl1x, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_rectangle *rect);

/*
 * devpmglf.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_gl1x, match)(br_device_pixelmap *self, br_device_pixelmap **newpm, br_token_value *tv);
br_device_pixelmap *DevicePixelmapGL1xAllocateFront(br_device *dev, br_output_facility *outfcty, br_token_value *tv);

/*
 * devclut.c
 */
br_device_clut *DeviceClutGL1xAllocate(br_device_pixelmap *self);

/*
 * outfcty.c
 */
br_output_facility *OutputFacilityGL1xInit(br_device *dev);

/*
 * sbuffer.c
 */
struct br_buffer_stored *BufferStoredGL1xAllocate(br_primitive_library *plib, br_token use, struct br_device_pixelmap *pm, br_token_value *tv);
GLuint BufferStoredGL1xGetTexture(const br_buffer_stored *self, br_boolean keyed);

/*
 * util.c
 */
GLuint   DeviceGL1xPixelmapToGLTexture(const GladGLContext *gl, br_pixelmap *pm);
br_error DeviceGL1xPixelmapToExistingGLTexture(const GladGLContext *gl, GLuint tex, br_pixelmap *pm);
br_error DeviceGL1xPixelmapToExistingGLTextureConvert(const GladGLContext *gl, GLuint tex, br_pixelmap *pm, const br_pixelmap_convert_options *opts);
br_uint_8   DeviceGL1xTypeOrBits(br_uint_8 pixel_type, br_int_32 pixel_bits);
void        DeviceGL1xObjectLabel(const GladGLContext *gl, GLenum identifier, GLuint name, const char *s);
void        DeviceGL1xObjectLabelF(const GladGLContext *gl, GLenum identifier, GLuint name, const char *fmt, ...);
br_boolean  DeviceGL1xCheckErrors(const GladGLContext *gl);
const char *DeviceGL1xStrError(GLenum err);

/*
 * formats.c
 */
const br_pixelmap_gl_fmt *DeviceGL1xGetFormatDetails(br_uint_8 type);

/*
 * plib.c
 */
extern const br_token PrimPartsGL1xTokens[];
br_primitive_library *PrimitiveLibraryGL1xAllocate(br_device_pixelmap *pm, const char *identifier, const GladGLContext *gl);

/*
 * pstate.c
 */
br_primitive_state *PrimitiveStateGL1xAllocate(br_primitive_library *plib);

/*
 * rendfunc.c
 */
void BR_ASM_CALL TriangleRenderGL1x(brp_block *block, union brp_vertex *v0, union brp_vertex *v1, union brp_vertex *v2);
void BR_ASM_CALL LineRenderGL1x(brp_block *block, union brp_vertex *v0, union brp_vertex *v1);
void BR_ASM_CALL PointRenderGL1x(brp_block *block, union brp_vertex *v0);
void             SetRendfuncGL1xContext(const GladGLContext *gl, const br_primitive_state *st);

#ifdef __cplusplus
};
#endif

#endif
#endif /* _DRV_IP_H_ */
