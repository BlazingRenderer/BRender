/*
 * softprim private driver interface
 */
#ifndef _SOFTPRIM_DRV_H_
#define _SOFTPRIM_DRV_H_

#define BR_OBJECT_PRIVATE
#define BR_DEVICE_PRIVATE
#define BR_PRIMITIVE_LIBRARY_PRIVATE
#define BR_PRIMITIVE_STATE_PRIVATE
#define BR_BUFFER_STORED_PRIVATE

#ifndef _BRDDI_H_
#include "brddi.h"
#endif

#ifndef _PRIMINFO_H_
#include "priminfo.h"
#endif

#include "softprim.h"
#include "timestmp.h"
#include "object.h"
#include "template.h"
#include "device.h"
#include "plib.h"
#include "pstate.h"
#include "sbuffer.h"

/*
 * Macros that expand to the first two arguments of a template entry
 * Builtin or device token
 */
#define BRT(t) BRT_##t, 0
#define DEV(t) 0, #t

#ifndef _SOFTPRIM_DRV_IP_H_
#include "drv_ip.h"
#endif

#endif
