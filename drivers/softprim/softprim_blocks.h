/*
 * softprim kernel list
 *
 * Includes the generated kernel list and expands each entry with the
 * SOFTPRIM_BLOCK_* macro the includer defines, so raster.cpp's kernel
 * definitions come from the same description of the block set as the matcher's
 * table (match.c includes the generated matcher list, softprim_matchers.inc,
 * which carries the metadata as well).
 *
 * One entry per distinct axis tuple. Two .ifg blocks that share a tuple share a
 * kernel and are one entry here; the matcher sees them separately, because they
 * can carry different requirements.
 *
 * The macro is chosen by topology, because the kernel's arity is: softrend
 * calls a triangle with three vertices, a line with two and a point with one.
 *
 *   SOFTPRIM_BLOCK_TRI(name, colour7bit, ...tuple)
 *   SOFTPRIM_BLOCK_LINE(name, colour7bit, ...tuple)
 *   SOFTPRIM_BLOCK_POINT(name, colour7bit, ...tuple)
 *
 * The file is generated at build time by drivers/softprim/infogen.pl in its
 * softprim mode and merged by drivers/softprim/merge_blocks.pl, which spells the
 * topology into the macro name; see drivers/softprim/CMakeLists.txt for the axis
 * spec that decides which blocks are emitted.
 */
#ifndef SOFTPRIM_BLOCK_TRI
#error "softprim_blocks.h: define SOFTPRIM_BLOCK_TRI/_LINE/_POINT(name, colour7bit, ...) first"
#endif

#include "softprim_blocks_all.inc"

#undef SOFTPRIM_BLOCK_TRI
#undef SOFTPRIM_BLOCK_LINE
#undef SOFTPRIM_BLOCK_POINT
