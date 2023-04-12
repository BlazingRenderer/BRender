#ifndef _BRSDL2DEV_P_H
#define _BRSDL2DEV_P_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

/*
 * Main entry point for device.
 */
struct br_device *BR_EXPORT BrDrv1SDL2Begin(const char *arguments);

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif /* _BRSDL2DEV_P_H */
