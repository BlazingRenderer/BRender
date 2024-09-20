/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: drv.h 1.1 1997/12/10 16:45:16 jon Exp $
 * $Locker: $
 *
 * Private device driver structure
 */
#ifndef _DRV_H_
#define _DRV_H_

#define BR_OBJECT_PRIVATE
#define BR_DEVICE_PRIVATE
#define BR_OUTPUT_FACILITY_PRIVATE
#define BR_DEVICE_PIXELMAP_PRIVATE
#define BR_DEVICE_CLUT_PRIVATE

#ifndef _BRDDI_H_
#include "brddi.h"
#endif

#ifndef _TEMPLATE_H_
#include "template.h"
#endif

#ifndef _DEVICE_H_
#include "device.h"
#endif

#ifndef _OUTFCTY_H_
#include "outfcty.h"
#endif

#ifndef _DEVPIXMP_H_
#include "devpixmp.h"
#endif

#ifndef _DEVCLUT_H_
#include "devclut.h"
#endif

#ifndef _OBJECT_H_
#include "object.h"
#endif

/*
 * Macros that expand to the first two arguments of a template entry
 * Builtin or device token
 */
#define BRT(t) BRT_##t, 0
#define DEV(t) 0, #t

/*
 * Round up to the nearest multiple of four
 */
#define ALIGN4(n) (((n) + 3) & ~3)

/*
 * Critical section wrappers
 */
#if 0

#define CSBegin(p)                           \
    do {                                     \
        br_device *dev;                      \
        dev = ObjectDirectDrawDevice(p);     \
        ASSERT(dev != NULL);                 \
        ASSERT(dev->csection != NULL);       \
        EnterCriticalSection(dev->csection); \
    } while(0)

#define CSEnd(p)                             \
    do {                                     \
        br_device *dev;                      \
        dev = ObjectDirectDrawDevice(p);     \
        ASSERT(dev != NULL);                 \
        ASSERT(dev->csection != NULL);       \
        LeaveCriticalSection(dev->csection); \
    } while(0)

#else

#define CSBegin(p)
#define CSEnd(p)

#endif

/*
 * Pull in private prototypes
 */
#ifndef _NO_PROTOTYPES

#ifndef _DRV_IP_H_
#include "drv_ip.h"
#endif

#endif

#ifdef __cplusplus
}
;
#endif
#endif
