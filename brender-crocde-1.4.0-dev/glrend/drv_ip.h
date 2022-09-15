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

void VIDEOI_BuildFontAtlas(HVIDEO hVideo, HVIDEO_FONT hFont, br_font *font, br_int_32 width, br_int_32 height);

GLuint VIDEO_BrPixelmapToGLTexture(br_pixelmap *pm);

void VIDEOI_BrPixelmapToExistingTexture(GLuint tex, br_pixelmap *pm);

br_error VIDEOI_BrPixelmapGetTypeDetails(br_uint_8 pmType, GLint *internalFormat, GLenum *format, GLenum *type,
                                         GLsizeiptr *elemBytes);

void VIDEOI_BrRectToGL(br_pixelmap *pm, br_rectangle *r);

void VIDEOI_BrRectToUVs(br_pixelmap *pm, br_rectangle *r, float *x0, float *y0, float *x1, float *y1);

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
br_output_facility *OutputFacilityGLCreateTemporary(br_device *dev, br_token_value *tv);

br_error OutputFacilityGLEnumerate(br_device *device);

/*
 * renderer.cpp
 */
br_renderer *RendererGLAllocate(br_device *device, br_renderer_facility *facility, br_device_pixelmap *dest);

/*
 * devpixmp.c
 */
br_device_pixelmap *DevicePixelmapGLAllocate(br_device *device, br_output_facility *facility, br_token_value *tv);

/*
 * sbuffer.c
 */
struct br_buffer_stored *BufferStoredGLAllocate(br_renderer *renderer,
                                                br_token use, struct br_device_pixelmap *pm, br_token_value *tv);

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
br_geometry_stored *GeometryStoredGLAllocate(br_geometry_v1_model *gv1model, const char *id, struct v11model_f *model);

/*
 * onscreen.c
 */
br_token GLOnScreenCheck(const br_matrix4 *model_to_screen, const br_bounds3_f *bounds);

/*
 * sstate.c
 */
br_renderer_state_stored *RendererStateStoredGLAllocate(br_renderer *renderer, HGLSTATE_STACK base_state,
                                                        br_uint_32 m, br_token_value *tv);

/*
 * renderer.c
 */
void StoredGLRenderGroup(br_geometry_stored *self, br_renderer *renderer, const gl_groupinfo *groupinfo);

#define RendererSceneBegin(self) ((br_error (*)(br_renderer *(self)))BR_CMETHOD_CALL(br_renderer,_reserved0,self))((br_renderer *)(self))
#define RendererSceneEnd(self) ((br_error (*)(br_renderer *(self)))BR_CMETHOD_CALL(br_renderer,_reserved1,self))((br_renderer *)(self))

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
