#ifndef GLTFVIEW_BENCH_H
#define GLTFVIEW_BENCH_H

#include <brender.h>

typedef struct gltfview_bench gltfview_bench;
typedef struct br_demo        br_demo;

/*
 * Benchmark harness for gltfview.
 *
 * Enabled by setting GLTFVIEW_BENCH_FRAMES. When disabled, GLTFViewBenchAllocate()
 * returns NULL and every other entry point is a no-op on NULL. An unset or empty
 * value disables it too, except under the "offscreen" video driver, where it
 * defaults to 5 frames so a headless run cannot render forever.
 *
 * Environment:
 *   GLTFVIEW_BENCH_FRAMES   Frames to render before quitting. 0 disables. Unset or
 *                           empty disables, except under "offscreen", where it
 *                           defaults to 5.
 *   GLTFVIEW_BENCH_WARMUP   Frames to discard before recording. Default 10.
 *   GLTFVIEW_BENCH_NOVSYNC  "1" to disable vsync. Default 0.
 *   GLTFVIEW_BENCH_FINISH   "1" to glFinish() each frame (serialises the pipeline
 *                           so frame wall-time is the true frame cost). Default 0.
 *   GLTFVIEW_BENCH_CSV      Path to write per-frame samples to.
 *
 * Reports CPU frame time and GPU time for the scene render separately, which is
 * what distinguishes a fragment-bound frame from a driver/CPU-bound one.
 */
gltfview_bench *GLTFViewBenchAllocate(br_demo *demo);

typedef enum bench_stage {
    BENCH_STAGE_CLEAR = 0,
    BENCH_STAGE_SCENE,

    BENCH_STAGE_COUNT
} bench_stage;

extern const char *const bench_stage_names[BENCH_STAGE_COUNT];

/* Bracket a stage; collects its GPU elapsed time. Stages must not overlap. */
void GLTFViewBenchBeginStage(gltfview_bench *bench, bench_stage stage);
void GLTFViewBenchEndStage(gltfview_bench *bench, bench_stage stage);

/* Close out the frame; call after the present. */
void GLTFViewBenchEndFrame(gltfview_bench *bench);

/* BR_TRUE once the requested frame count is reached. */
br_boolean GLTFViewBenchShouldQuit(gltfview_bench *bench);

/* Print the summary. Call once, before the GL context goes away. */
void GLTFViewBenchReport(gltfview_bench *bench);

#endif /* GLTFVIEW_BENCH_H */
