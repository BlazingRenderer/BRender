#ifndef RT_B2DWITNESS_H_
#define RT_B2DWITNESS_H_

#include <brender.h>

#include "rt_demo.h"

/*
 * Run the 2D witness: drive the immediate geometry path through Br2D*() and
 * assert what it drew, against the software clipper and against itself. The
 * path is otherwise only reached by examples/imgui-demo, which CI does not run,
 * and a driver whose render() is a stub fails silently - the batch draws
 * nothing and there is no reference to notice it.
 *
 * Every check is a self-check: no reference table, nothing to bless.
 * Returns BRE_OK if every step passed, BRE_FAIL otherwise.
 */
br_error rt_2d_witness_run(rt_demo *demo);

#endif /* RT_B2DWITNESS_H_ */
