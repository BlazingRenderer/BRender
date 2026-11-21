/*
 * The parallax witness: the offset's sign, magnitude, depth linearity and obliquity, measured
 * through a gradient albedo on a flat-normal, constant-height bump map.
 */
#ifndef RT_PARALLAXWITNESS_H_
#define RT_PARALLAXWITNESS_H_

#include <brender.h>

#include "rt_demo.h"

br_error rt_parallax_witness_run(rt_demo *demo, const char *device);

#endif /* RT_PARALLAXWITNESS_H_ */
