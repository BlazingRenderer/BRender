/*
 * The material witness: every field the binary and script formats carry, written and read back
 * through the memory filesystem.
 */
#ifndef RT_MATERIALWITNESS_H_
#define RT_MATERIALWITNESS_H_

#include <brender.h>

#include "rt_demo.h"

br_error rt_material_witness_run(rt_demo *demo);

#endif /* RT_MATERIALWITNESS_H_ */
