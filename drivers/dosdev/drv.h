/*
 * Private device driver structure
 */
#ifndef _DRV_H_
#define _DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

// #define BR_OBJECT_PRIVATE
#define BR_DEVICE_PRIVATE
#define BR_OUTPUT_FACILITY_PRIVATE
#define BR_DEVICE_PIXELMAP_PRIVATE
#define BR_DEVICE_CLUT_PRIVATE

/*
 * Maximum number of possible entries on a CLUT.
 * BRender supports no more than 256 (for BR_PMT_INDEX_8).
 */
#define MAX_CLUT_ENTRIES 256

#include "brddi.h"
#include "brdosdev.h"
#include "template.h"
#include "device.h"
#include "outfcty.h"
#include "devpixmp.h"
#include "devclut.h"

/*
 * Macros that expand to the first two arguments of a template entry
 * Builtin or device token
 */
#define BRT(t) BRT_##t, 0
#define DEV(t) 0, #t

/*
 * Pull in private prototypes
 */
#ifndef _NO_PROTOTYPES

#include "drv_ip.h"

#endif

#ifdef __cplusplus
};
#endif
#endif /* _DRV_H_ */
