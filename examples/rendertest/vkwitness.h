#ifndef RT_VKWITNESS_H_
#define RT_VKWITNESS_H_

#include <brender.h>

#include "brdemo.h"

/*
 * Run the vkrend witness against the front screen the demo created. The screen
 * must be a vkrend front screen (brdemo's --vulkan path); the witness matches an
 * offscreen from it and drives the device-pixelmap and renderer scene cases.
 * Returns BRE_OK if every step passed, BRE_FAIL otherwise.
 */
br_error rt_vk_witness_run(br_demo *demo);

#endif /* RT_VKWITNESS_H_ */
