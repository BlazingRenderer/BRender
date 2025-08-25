/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: drv_ip.h 1.1 1997/12/10 16:53:59 jon Exp $
 * $Locker: $
 *
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
br_device *DeviceVESAAllocate(const char *identifier, const char *arguments);
\
br_boolean DeviceVESALinearAvailable(struct br_device *self, struct vesa_modeinfo *mi);

br_error DeviceVESALinearInitialise(struct br_device *self, struct vesa_modeinfo *mi);

/*
 * outtype.c
 */
br_error OutputFacilityVESAInitialise(int *count, br_device *dev);

/*
 * devpixmp.c
 */
br_device_pixelmap *DevicePixelmapVESAAllocateMode(br_device *dev, br_output_facility *type, br_uint_16 w, br_uint_16 h);

/*
 * devpmw.c
 */
extern struct br_device_pixelmap_dispatch devicePixelmapDispatch_w;

/*
 * devpmwb.c
 */
extern struct br_device_pixelmap_dispatch devicePixelmapDispatch_wb;

/*
 * devpml.c
 */
extern struct br_device_pixelmap_dispatch devicePixelmapDispatch_l;

/*
 * devclut.c
 */
br_device_clut *DeviceClutVESAAllocate(struct br_device *dev, const char *identifier);

/*
 * bios.c
 */
br_error   BIOSVideoSetMode(br_uint_16 mode);
br_uint_16 BIOSVideoGetMode(void);

#ifdef __cplusplus
};
#endif

#endif
#endif
