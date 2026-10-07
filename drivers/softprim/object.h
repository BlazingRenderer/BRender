/*
 * softprim private object structure
 */
#ifndef _SOFTPRIM_OBJECT_H_
#define _SOFTPRIM_OBJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Private state of an object
 */
typedef struct br_object {
    struct br_object_dispatch *dispatch;
    char                      *identifier;
    struct br_device          *device;
} br_object;

#define ObjectSoftPrimIdentifier(d) (((br_object *)d)->identifier)
#define ObjectSoftPrimDevice(d)     (((br_object *)d)->device)

#ifdef __cplusplus
};
#endif
#endif
