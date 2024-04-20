#ifndef BR_INTRO_H_
#define BR_INTRO_H_

#include <brender.h>

struct br_intro;
typedef struct br_intro br_intro;

br_intro  *BrIntroCreate(void);
void       BrIntroReset(br_intro *state);
void       BrIntroDestroy(br_intro *state);
void       BrIntroSetAspect(br_intro *state, br_scalar aspect);
void       BrIntroUpdate(br_intro *state, float dt);
br_actor  *BrIntroGetWorld(br_intro *state);
br_actor  *BrIntroGetCamera(br_intro *state);
void       BrIntroForceFade(br_intro *state);
br_boolean BrIntroIsDone(br_intro *state);

#endif /* BR_INTRO_H_ */
