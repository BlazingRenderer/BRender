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
br_device              *DeviceNullAllocate(const char *identifier);
br_geometry_primitives *GeometryPrimitivesNullAllocate(br_renderer_facility *type, const char *id);
br_geometry_lighting   *GeometryLightingNullAllocate(br_renderer_facility *type, const char *id);

#ifdef __cplusplus
};
#endif

#endif
#endif /* _DRV_IP_H_ */
