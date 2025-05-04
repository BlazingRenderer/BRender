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

br_boolean VIDEOI_CompileRectShader(HVIDEO hVideo);

br_boolean VIDEOI_CompileTextShader(HVIDEO hVideo);

br_boolean VIDEOI_CompileLineShader(HVIDEO hVideo);

br_boolean VIDEOI_CompileBRenderShader(HVIDEO hVideo, const char *vertPath, const char *fragPath);

const br_pixelmap_gl_fmt *DeviceGLGetFormatDetails(br_uint_8 type);

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

state_stack *RendererGLAllocState(br_renderer *self, const state_stack *tpl, br_uint_32 refs);
void         RendererGLUnrefState(br_renderer *self, state_stack *state);

br_int_32 RendererGLNextImmTri(br_renderer *self, struct v11group *group, br_vector3_u16 fp);

GLuint RendererGLGetSampler(br_renderer *self, const br_sampler_info_gl *info);

/*
 * devpixmp.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, match) (br_device_pixelmap *self, br_device_pixelmap **newpm,
                                                        br_token_value *tv);

/*
 * Gets the pixelmap viewport rect (for glViewport), converting to bottom-left coordinates.
 */
br_rectangle DevicePixelmapGLGetViewport(const br_device_pixelmap *pm);

/*
 * devpmsub.c
 */
br_error BR_CMETHOD_DECL(br_device_pixelmap_gl, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm,
                                                             br_rectangle *rect);

void DevicePixelmapGLIncRef(br_device_pixelmap *self);

void DevicePixelmapGLDecRef(br_device_pixelmap *self);

/*
 * devpmgl.c
 */
br_device_pixelmap *DevicePixelmapGLAllocateFront(br_device *dev, br_output_facility *outfcty, br_token_value *tv);

/*
 * devclut.c
 */
br_device_clut *DeviceClutGLAllocate(br_device_pixelmap *pm);

/*
 * sbuffer.c
 */
struct br_buffer_stored *BufferStoredGLAllocate(br_renderer *renderer, br_token use, struct br_device_pixelmap *pm,
                                                br_token_value *tv);

GLenum BufferStoredGLGetTexture(const br_buffer_stored *self);
GLuint BufferStoredGLGetCLUTTexture(const br_buffer_stored *self, br_device_pixelmap *target, GLuint fallback);

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
void StateGLCopy(state_stack *dst, const state_stack *src, uint32_t mask);

/*
 * v1model.c
 */
void DeviceGLExtractPrimitiveState(const state_stack *state, br_primitive_state_info_gl *info, GLuint tex_white);

void RendererGLRenderGroup(br_renderer *self, br_geometry_stored *stored, const gl_groupinfo *groupinfo);
void RendererGLRenderTri(br_renderer *self, br_uintptr_t offset, const gl_groupinfo *groupinfo);

/*
 * font.c
 */
br_error FontGLBuildArray(br_font_gl *gl_font, br_font *font);

/*
 * util.c
 */
br_error DevicePixelmapGLBindFramebuffer(GLenum target, br_device_pixelmap *pm);
GLuint   DeviceGLBuildWhiteTexture(void);
GLuint   DeviceGLBuildCheckerboardTexture(void);
GLuint   DeviceGLPixelmapToGLTexture(br_pixelmap *pm);
br_error DeviceGLPixelmapToExistingGLTexture(GLuint tex, br_pixelmap *pm);

br_uint_8 DeviceGLTypeOrBits(br_uint_8 pixel_type, br_int_32 pixel_bits);

void DeviceGLObjectLabel(GLenum identifier, GLuint name, const char *s);
void DeviceGLObjectLabelF(GLenum identifier, GLuint name, const char *fmt, ...);

br_boolean  DeviceGLCheckErrors(void);
const char *DeviceGLStrError(GLenum err);

br_vector3 DeviceGLTriangleCentroid(const br_vector3 *v1, const br_vector3 *v2, const br_vector3 *v3);

br_clip_result DevicePixelmapGLRectangleClip(br_rectangle *restrict out, const br_rectangle *restrict r, const br_pixelmap *pm);

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
