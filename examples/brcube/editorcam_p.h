#ifndef EDITORCAM_P_H_
#define EDITORCAM_P_H_

#ifndef _NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif
br_editor_camera *BrEditorCamAllocate(void *res);
void              BrEditorCamSetViewportSize(br_editor_camera *cam, br_uint_16 width, br_uint_16 height);
void              BrEditorCamUpdate(br_editor_camera *cam, float dt);
void              BrEditorCamReset(br_editor_camera *cam);
void              BrEditorCamFree(br_editor_camera *cam);

#ifdef __cplusplus
}
#endif

#endif

#endif /* EDITORCAM_P_H_ */
