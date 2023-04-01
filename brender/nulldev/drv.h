/*
 * Private device driver structure
 */
#ifndef _DRV_H_
#define _DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define BR_OBJECT_PRIVATE
#define BR_DEVICE_PRIVATE
#define BR_GEOMETRY_PRIMITIVES_PRIVATE
#define BR_GEOMETRY_LIGHTING_PRIVATE

#include "brddi.h"
#include "template.h"
#include "device.h"
#include "gprim.h"
#include "glighting.h"

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
