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
 * video.c
 */
HVIDEO VIDEO_Open(HVIDEO hVideo, const char *vertShader, const char *fragShader);

void VIDEO_Close(HVIDEO hVideo);

GLuint VIDEOI_CreateAndCompileShader(GLenum type, const char *shader, size_t size);

GLuint VIDEOI_LoadAndCompileShader(GLenum type, const char *path, const char *default_data, size_t default_size);

GLuint VIDEOI_CreateAndCompileProgram(GLuint vert, GLuint frag);

br_boolean VIDEOI_CompileDefaultShader(HVIDEO hVideo);

br_boolean VIDEOI_CompileTextShader(HVIDEO hVideo);

br_boolean VIDEOI_CompileBRenderShader(HVIDEO hVideo, const char *vertPath, const char *fragPath);

GLuint VIDEO_BrPixelmapToGLTexture(br_pixelmap *pm);

br_error VIDEOI_BrPixelmapToExistingTexture(GLuint tex, br_pixelmap *pm);

br_error VIDEOI_BrPixelmapGetTypeDetails(br_uint_8 pmType, GLint *internalFormat, GLenum *format, GLenum *type,
                                         GLsizeiptr *elemBytes, br_boolean *blended);

void VIDEOI_BrRectToGL(const br_pixelmap *pm, br_rectangle *r);

void VIDEOI_BrRectToUVs(const br_pixelmap *pm, const br_rectangle *r, float *x0, float *y0, float *x1, float *y1);

br_matrix4 *VIDEOI_D3DtoGLProjection(br_matrix4 *m);

/*
 * device.c
 */
br_device *DeviceGLAllocate(const char *identifier, const char *arguments);

/*
 * rendfcty.cpp
 */
br_renderer_facility *RendererFacilityGLInit(br_device *dev);

/*
 * outfcty.c
 */
br_output_facility *OutputFacilityGLInit(br_device *dev, br_renderer_facility *rendfcty);

/*
 * renderer.cpp
 */
br_renderer *RendererGLAllocate(br_device *device, br_renderer_facility *facility, br_device_pixelmap *dest);

/*
 * devpixmp.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, match) (br_device_pixelmap *self, br_device_pixelmap **newpm,
                                                        br_token_value *tv);
/*
 * devpmgl.c
 */
br_device_pixelmap *DevicePixelmapGLAllocateFront(br_device *dev, br_output_facility *outfcty, br_token_value *tv);

/*
 * sbuffer.c
 */
struct br_buffer_stored *BufferStoredGLAllocate(br_renderer *renderer, br_token use, struct br_device_pixelmap *pm,
                                                br_token_value *tv);

GLenum BufferStoredGLGetTexture(br_buffer_stored *self);

/*
 * gv1buckt.c
 */
br_geometry_v1_buckets *GeometryV1BucketsGLAllocate(br_renderer_facility *type, const char *id);

/*
 * gv1model.c
 */
br_geometry_v1_model *GeometryV1ModelGLAllocate(br_renderer_facility *type, const char *id);

/*
 * gstored.c
 */
br_geometry_stored *GeometryStoredGLAllocate(br_geometry_v1_model *gv1model, const char *id, br_renderer *r,
                                             struct v11model *model);

/*
 * onscreen.c
 */
br_token GLOnScreenCheck(const br_matrix4 *model_to_screen, const br_bounds3_f *bounds);

/*
 * sstate.c
 */
br_renderer_state_stored *RendererStateStoredGLAllocate(br_renderer *renderer, state_stack *base_state, br_uint_32 m,
                                                        br_token_value *tv);

/*
 * state.c and friends
 */
void StateGLInit(state_all *state, void *res);
void StateGLInitMatrix(state_all *state);
void StateGLInitCull(state_all *state);
void StateGLInitClip(state_all *state);
void StateGLInitSurface(state_all *state);
void StateGLInitPrimitive(state_all *state);
void StateGLInitOutput(state_all *state);
void StateGLInitHidden(state_all *state);
void StateGLInitLight(state_all *state);

struct br_tv_template *StateGLGetStateTemplate(state_all *state, br_token part, br_int_32 index);
void                   StateGLTemplateActions(state_all *state, uint32_t mask);

void StateGLReset(state_cache *cache);

br_boolean StateGLPush(state_all *state, uint32_t mask);
br_boolean StateGLPop(state_all *state, uint32_t mask);
void       StateGLDefault(state_all *state, uint32_t mask);

void StateGLUpdateScene(state_cache *cache, state_stack *state);
void StateGLUpdateModel(state_cache *cache, state_matrix *matrix);
void StateGLCopy(state_stack *dst, state_stack *src, uint32_t mask);

/*
 * renderer.c
 */
void StoredGLRenderGroup(br_geometry_stored *self, br_renderer *renderer, const gl_groupinfo *groupinfo);

/*
 * font.c
 */
br_error FontGLBuildAtlas(br_font_gl *gl_font, br_font *font, br_int_32 width, br_int_32 height);

/*
 * quad.c
 */

void DeviceGLInitQuad(br_device_pixelmap_gl_quad *self, HVIDEO hVideo);
void DeviceGLFiniQuad(br_device_pixelmap_gl_quad *self);

/*
 * Patch the X/Y and U/V coordinates.
 * X/Y = rect of destination
 * U/V = rect of source
 * Rects are in OpenGL coordianges -- (0,0) at bottom-left
 */
void DeviceGLPatchQuad(br_device_pixelmap_gl_quad *self, const br_pixelmap *dst, const br_rectangle *dr,
                       const br_pixelmap *src, const br_rectangle *sr);
void DeviceGLPatchQuadFont(br_device_pixelmap_gl_quad *self, const br_pixelmap *dst, const br_rectangle *dr,
                           const br_font_gl *font, br_uint_8 glyph);
void DeviceGLDrawQuad(br_device_pixelmap_gl_quad *self);
void DeviceGLDrawQuadText(br_device_pixelmap_gl_quad *self);

/*
 * util.c
 */
br_error DevicePixelmapGLBindFramebuffer(GLenum target, br_device_pixelmap *pm);
GLuint   DeviceGLBuildWhiteTexture(void);
GLuint   DeviceGLBuildCheckerboardTexture(void);

br_uint_8 DeviceGLTypeOrBits(br_uint_8 pixel_type, br_int_32 pixel_bits);

/*
 * Wrappers for br_device_gl_procs.
 */
void *DevicePixelmapGLExtCreateContext(br_device_pixelmap *self);

void DevicePixelmapGLExtDeleteContext(br_device_pixelmap *self, void *ctx);

br_error DevicePixelmapGLExtMakeCurrent(br_device_pixelmap *self, void *ctx);

void DevicePixelmapGLExtSwapBuffers(br_device_pixelmap *self);

br_device_pixelmap_gl_getprocaddress_cbfn *DevicePixelmapGLExtGetGetProcAddress(br_device_pixelmap *self);

br_error DevicePixelmapGLExtResize(br_device_pixelmap *self, br_int_32 w, br_int_32 h);

void DevicePixelmapGLExtPreSwap(br_device_pixelmap *self, GLuint fbo);

void DevicePixelmapGLExtFree(br_device_pixelmap *self);

br_error DevicePixelmapGLExtHandleWindowEvent(br_device_pixelmap *self, void *arg);

/*
 * Hijack nulldev's no-op implementations.
 * They're designed for this.
 */
br_geometry_lighting *GeometryLightingNullAllocate(br_renderer_facility *type, const char *id);

br_geometry_primitives *GeometryPrimitivesNullAllocate(br_renderer_facility *type, const char *id);

#ifdef __cplusplus
};
#endif

#endif
#endif /* _DRV_IP_H_ */
