#ifndef MSWITNESS_H_
#define MSWITNESS_H_

#include "rt_demo.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The multisample witness: one scene drawn into a single-sampled offscreen and into a
 *        multisampled one, then compared pixel by pixel.
 *
 * It passes when the multisampled frame has coverage levels the single-sampled frame cannot
 * have along the shape's edges, and when the two agree away from them.
 */
br_error rt_ms_witness_run(rt_demo *demo, const char *device);

#ifdef __cplusplus
};
#endif

#endif /* MSWITNESS_H_ */
