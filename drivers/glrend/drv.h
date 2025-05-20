/*
 * Private device driver structure
 */
#ifndef _DRV_H_
#define _DRV_H_

#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

#include "glad/glad.h"

#define BR_DEVICE_PRIVATE
#define BR_OUTPUT_FACILITY_PRIVATE
#define BR_DEVICE_PIXELMAP_PRIVATE
#define BR_DEVICE_CLUT_PRIVATE
#define BR_RENDERER_FACILITY_PRIVATE
#define BR_BUFFER_STORED_PRIVATE
#define BR_GEOMETRY_V1_MODEL_PRIVATE
#define BR_GEOMETRY_V1_BUCKETS_PRIVATE
#define BR_GEOMETRY_STORED_PRIVATE
#define BR_RENDERER_STATE_STORED_PRIVATE
#define BR_RENDERER_PRIVATE

#define BR_GLREND_DEBUG_INTERNAL_PREFIX "glrend:internal:"
#define BR_GLREND_DEBUG_USER_PREFIX     "glrend:user:"

/*
 * Maximum number of possible entries on a CLUT.
 * BRender supports no more than 256 (for BR_PMT_INDEX_8).
 */
#define BR_GLREND_MAX_CLUT_ENTRIES 256

/*
 * No. frames in the model-state ring buffer.
 */
#define BR_GLREND_MODEL_RB_FRAMES 3

/*
 * Max. no draws in-flight. This determines the size of the model-state buffer.
 */
#define BR_GLREND_MAX_DRAWS_IN_FLIGHT 4096

/*
 * Set to nonzero to glEnable(GL_DEBUG_OUTPUT)
 */
#define BR_GLREND_DEBUG 0

#include "brddi.h"
#include "brglrend.h"
#include "formats.h"
#include "pm.h"
#include "font.h"
#include "video.h"
#include "state.h"

#include "template.h"
#include "device.h"
#include "outfcty.h"
#include "devclut.h"
#include "devpixmp.h"
#include "rendfcty.h"
#include "renderer.h"
#include "sstate.h"
#include "sbuffer.h"
#include "gstored.h"
#include "gv1model.h"
#include "gv1buckt.h"

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
