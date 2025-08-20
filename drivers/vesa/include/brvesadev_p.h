#ifndef _BRVESADEV_P_H
#define _BRVESADEV_P_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

/*
 * Main entry point for device.
 */
struct br_device *BR_EXPORT BrDrv1VESABegin(const char *arguments);

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif /* _BRVESADEV_P_H */
