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
br_device *DeviceDOSAllocate(const char *identifier);

/*
 * outfcty.c
 */
br_output_facility *OutputFacilityDOSCreateGeneric(br_device *dev);

/*
 * devpixmp.c
 */

br_device_pixelmap *DevicePixelmapDOSAllocate(br_device *dev, br_output_facility *outfcty, br_int_32 width, br_int_32 height);
br_boolean          DevicePixelmapDOSIsOurs(const br_pixelmap *pm);
void               *DevicePixelmapDOSMemAddress(br_device_pixelmap *self, br_int_32 x, br_int_32 y);

#ifdef __cplusplus
};
#endif

#endif
#endif
