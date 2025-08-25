/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: device.h 1.1 1997/12/10 16:53:34 jon Exp $
 * $Locker: $
 *
 * Private device driver structure
 */
#ifndef _DEVICE_H_
#define _DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Mode dependant information stored at mode setup
 */
struct vesa_scanline {
    br_uint_32 offset;
    br_uint_32 page;
    br_uint_32 split;
    br_uint_32 _pad;
};

struct vesa_work {
    br_uint_32     stride;
    br_uint_8      bits_per_pixel;
    int            selector;
    __dpmi_meminfo mapping;
};

/*
 * Private state of device
 */
typedef struct br_device {
    /*
     * Dispatch table
     */
    struct br_device_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * List of objects associated with this device
     */
    void *object_list;

    /*
     * Anchor for all device's resources
     */
    void *res;

    /*
     * Driver-wide template store
     */
    struct device_templates templates;

    /*
     * VESA mode at startup
     */
    br_uint_16 original_mode;

    /*
     * Current mode
     */
    br_uint_16 current_mode;

    /*
     * Bank switching info
     */
    struct vesa_info *info;

    /*
     * Local copies of data in info, so templates can get at them.
     */
    br_uint_16  vbe_version;
    const char *vbe_oem_string;
    const char *vbe_oem_product_name;
    const char *vbe_oem_product_rev;
    const char *vbe_oem_vendor_name;

    struct vesa_work work;

    struct br_device_clut *clut;

    br_boolean screen_active;

} br_device;

/*
 * Some useful inline ops.
 */
#define DeviceVESAResource(d)          (((br_device *)d)->res)
#define DeviceVESAOriginalMode(d)      (((br_device *)d)->original_mode)
#define DeviceVESACurrentMode(d)       (((br_device *)d)->current_mode)
#define DeviceVESACurrentModeSet(d, m) ((((br_device *)d)->current_mode) = m)
#define DeviceVESAInfo(d)              ((((br_device *)d)->info))
#define DeviceVESAWork(d)              (&(((br_device *)d)->work))
#define DeviceVESAClut(d)              (((br_device *)d)->clut)

#ifdef __cplusplus
};
#endif
#endif
