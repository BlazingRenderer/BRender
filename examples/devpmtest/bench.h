#ifndef DEVPMTEST_BENCH_H
#define DEVPMTEST_BENCH_H

#include <brender.h>

typedef struct devpmtest_bench devpmtest_bench;

/*
 * The front screen the SDL3 device is asked for. SOFT passes neither accelerator
 * token, so it needs no GL or Vulkan driver and is the default.
 */
typedef enum devpmtest_front_screen {
    DEVPMTEST_FRONT_SOFT = 0,
    DEVPMTEST_FRONT_GL,
    DEVPMTEST_FRONT_VULKAN,
} devpmtest_front_screen;

typedef enum devpmtest_bench_parse {
    DEVPMTEST_BENCH_INTERACTIVE = 0, /* No run mode asked for; run the interactive loop. */
    DEVPMTEST_BENCH_CAPTURE,         /* --run-test/--run-all parsed into *bench. */
    DEVPMTEST_BENCH_HELP,            /* Usage printed; exit successfully. */
    DEVPMTEST_BENCH_ERROR,           /* Usage printed after a bad command line; exit non-zero. */
} devpmtest_bench_parse;

/*
 * Parse the command line. --run-test or --run-all selects the headless capture
 * mode and fills *bench with the run's options; without either the tool keeps its
 * interactive behaviour and *bench still carries --front-screen. --help and a bad
 * option print usage and return HELP/ERROR with *bench NULL.
 *
 * The default front screen is SOFT in both modes, so an argument-less launch
 * renders through the same front screen it did before this mode existed.
 */
devpmtest_bench_parse DevpmTestBenchParse(int argc, char **argv, devpmtest_bench **bench);

/* The run's front screen; valid in both modes. */
devpmtest_front_screen DevpmTestBenchFrontScreen(const devpmtest_bench *bench);

/* The capture options below are only meaningful for DEVPMTEST_BENCH_CAPTURE. */
br_boolean  DevpmTestBenchRunAll(const devpmtest_bench *bench);
br_int_32   DevpmTestBenchTestIndex(const devpmtest_bench *bench);
br_uint_32  DevpmTestBenchFrames(const devpmtest_bench *bench);
float       DevpmTestBenchDeltaMs(const devpmtest_bench *bench);
const char *DevpmTestBenchDumpPath(const devpmtest_bench *bench);
const char *DevpmTestBenchDumpDir(const devpmtest_bench *bench);
br_boolean  DevpmTestBenchChecksum(const devpmtest_bench *bench);

/*
 * Build the PNG path for one test of a --run-all run, DIR/<NN>-<slug>.png, into buf.
 * Returns buf, or NULL when no --dump-dir was given or the name did not fit.
 */
const char *DevpmTestBenchAllPath(const devpmtest_bench *bench, const char *name, br_int_32 index, char *buf, br_size_t buf_size);

/*
 * Copy colour_buffer into host memory (the same round-trip F5 uses), save it to
 * png_path as a PNG when png_path is non-NULL, and print its FNV-1a checksum and
 * non-clear-pixel coverage when checksum is true. The colour buffer is left alone.
 */
void DevpmTestBenchCapture(br_pixelmap *colour_buffer, const char *png_path, br_boolean checksum);

#endif /* DEVPMTEST_BENCH_H */
