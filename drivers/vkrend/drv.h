/*
 * Private device driver structure
 */
#ifndef DRV_H_
#define DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "glad/vulkan.h"
#include "vk_mem_alloc.h"

#define BR_DEVICE_PRIVATE
#define BR_OUTPUT_FACILITY_PRIVATE
#define BR_RENDERER_FACILITY_PRIVATE

#include "brddi.h"
#include "brvkrend.h"
#include "formats.h"
#include "pm.h"

#include "uuid.h"
#include "template.h"
#include "device.h"
#include "outfcty.h"
#include "rendfcty.h"

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
#endif /* DRV_H_ */
