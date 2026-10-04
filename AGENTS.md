# Agent Guidance for BRender

## Project Overview
BRender is a 3D rendering engine with multiple renderer backends:
- `core/` - Core engine, model preparation, math utilities
- `drivers/softrend/` - Software renderer
- `drivers/pentprim/` - Software rasterizer, or the "primitive library"
- `drivers/glrend/` - OpenGL renderer
- `contrib/` - Contributed utilities (e.g., editorcam)

## Building
- Uses CMake.
- The software rasterizer (`drivers/pentprim/`) is disabled by default. It requires explicit enablement with `-DBRENDER_BUILD_SOFT=ON` and only works on 32-bit x86-compatible platforms due to its assembly-heavy implementation.
- Building `pentprim` also needs [`h2inc`](https://github.com/BlazingRenderer/h2inc), which turns `drivers/pentprim/drv.h` into the
  MASM include `drv.inc` (`cmake/h2inc.cmake`). It replaces the old Wine + `H2INC.EXE` + `contrib/mkdrv.pl` pipeline; with
  `BRENDER_BUILD_SOFT=ON`, `-DBRENDER_H2INC_EXECUTABLE=<path>` must point at a release binary or the configure step fails.
- `drivers/softrend/` is the transform-and-lighting stage only and has no rasteriser of its own. It registers a renderer facility,
  but that facility's `rendererNew()` calls `BrPrimitiveLibraryFind()` (`drivers/softrend/rendfcty.c`), which finds nothing when no
  rasteriser backend is built. `--force-software` then dies with `Failed to load renderer` (`core/v1db/dbsetup.c`).
- A rasteriser other than pentprim is available in `glrend1x`, giving softrend T&L over an OpenGL rasteriser. The demo programs select
  it with `--opengl-device-name=glrend1x` (`examples/brdemo/brdemo.c`); that flag is brdemo-level, not engine API - the engine receives
  the name as the `BRT_OPENGL_DEVICE_NAME_CSTR` token, which the SDL device uses to load the named driver (`drivers/sdl3dev/glrend.c`).
  glrend1x exposes a `BRT_PRIMITIVE_LIBRARY_O` that softrend's search then finds (`drivers/glrend1x/devpmglf.c`).

## Benchmarking
- `examples/gltfview/bench.c` is the benchmark harness, driven by `GLTFVIEW_BENCH_*` environment variables: `FRAMES` (frames to render;
  0 or unset disables it), `WARMUP` (frames to discard, default 10), `NOVSYNC` (`1` to disable vsync), `FINISH` (`1` to `glFinish()`
  each frame), `CSV` (path to write per-frame samples to), `PPM` (path to write the final frame to as a PPM - GL path only) and
  `SOFT_PNG` (path to write the software colour buffer as a PNG, via `BrFmtImageSave`, for softrend and the software drivers).
- **An unset `FRAMES` means the demo never exits.** It renders in an endless loop rather than returning, so a run without it either
  hangs or has to be killed - and inside `gdb` that looks like a dump that produced nothing. Always set it:
  `SDL_VIDEODRIVER=offscreen GLTFVIEW_BENCH_FRAMES=5 GLTFVIEW_BENCH_NOVSYNC=1`. Bound anything that could spin with
  `timeout -s KILL` and never a plain `timeout`, because these programs install a SIGTERM handler and a plain `timeout` leaves the
  process running. The timeout is a backstop, not a mechanism: if it is the thing ending a run, the invocation is wrong.
- Every demo built on `examples/brdemo/brdemo.c` is useless as an instrument: it hardcodes vsync on (`SDL_GL_SetSwapInterval(1)`)
  and draws the frame time into the colour buffer, so it reports the refresh period (~16.7ms on a 60Hz display) whatever it rendered.
- Always measure with `--no-stats` (`examples/brdemo/brdemo.c`, inherited by `gltfview`). Without it the overlay writes frame time
  into the colour buffer, so runs differ and the checksum is worthless.
- GPU timings come from GL timer queries and need a GL context; without one they are reported as 0 rather than crashing. Softrend has
  no GL context of its own - run it over `glrend1x` (see Building) to get GPU timings for the software path. The end-of-run checksum is
  taken from the window-sized framebuffer, so it is only comparable between runs with identical `-w`/`-h`.
- Protocol: build each variant to its own binary, warm every binary, and run variants interleaved (ABBA). Report min and median and
  require them to agree; use n>=6 for anything under ~5%. Absolute times drift badly across sessions - only same-session interleaved
  ratios mean anything.

## Contributing
- This is a fork of the BRender version shipped with Croc,
  with features from the official Argonaut 1.3.2 release backported.
  Major differences from prior Argonaut-official versions are outlined in `MIGRATION.md`.
- All PRs should be rebased against current master.
- Merge commits are not accepted except at maintainer discretion (e.g. octopus merges, or cases where preserving branch history has clear value). When in doubt, rebase.

## Coding Conventions
- C codebase with `br_*` prefix for public structs, `Br*` prefix for public functions.
- Run `clang-format` before committing (project has `.clang-format` configuration).
- `BR_PUBLIC_ENTRY` decorates functions that are part of the public API. `BR_RESIDENT_ENTRY` decorates
  functions that cross library boundaries internally but are not part of the public API (i.e., non-static
  internal linkage). Do not omit these on applicable functions.

## Commit Style
We use a similar commit style to open source projects like the Linux Kernel and FFmpeg.
**Do not use Conventional Commits** (e.g., `chore:`, `feat:`, `fix:`, `docs:`, `style:`, `refactor:`, `test:`, `ci:`).
See the [Commit messages](https://ffmpeg.org/developer.html#toc-Patches_002fCommitting) section of the FFmpeg developer guide.

Format:
```
component: short description

More detailed description if necessary.
```

Each commit should be a single logical unit, though larger commits are acceptable when the change is nontrivial and cannot be cleanly decoupled.

Examples:

```
gitignore: ignore /build-*/
```

```
core/v1db: fix RegenerateVertexNormals to respect smoothing groups
```

```
contrib/editorcam: fix camera offset after pan mode, add orbit mode

- Pan mode now applies translations to actor (world position) rather
  than camera (orientation), fixing offset after switching modes
- Add orbit mode (Alt+LMB drag) with azimuth/elevation rotation
```

## Testing
There is no unit-test suite, and none should be added. There is a render regression harness:
- `examples/rendertest` renders the fixtures in `examples/rendertest/dat/` under one device and diffs each frame against the checked-in
  reference, `examples/rendertest/rendertest.txt`. Run it (`--device <glrend|glrend1x|softrend> [--bpp n] [-w W -h H] [--no-depth]`) after
  any change that can move pixels; `result=PASS failures=0` is the pass condition.
- The reference key is `<device>/<driver>/<zb|zs>/<pixel-type>/<WxH>/<scene>`, so each device, driver and depth mode is held separately.
  `--bless` rewrites entries; only do so to accept a deliberate change. The checked-in entries are llvmpipe-only, so a real-GPU run reports
  `NO-REFERENCE` for every scene - force glrend onto llvmpipe with `LIBGL_ALWAYS_SOFTWARE=true` to compare against them.
- The run also asserts relations between fixtures (e.g. `scene-unlit == scene-unlit-plain`), so a path that silently stops firing fails
  rather than only moving a checksum. `--ppm-dir <dir>` writes each frame as `<dir>/<scene>.ppm` when a checksum cannot show how a render changed.
- softrend is the 32-bit software rasteriser path and needs `-DBRENDER_BUILD_SOFT=ON` (see Building); `--no-depth` renders through
  `BrZsSceneRender()` and covers the Z-sort path. The fixtures come from `mkres scenes` (`examples/mkres/scene.c`), which writes them to
  the current directory.

## Common Patterns

### Croc-Specific Code
Some code is conditionally compiled for the Croc game:
```c
#if BRENDER_BUILD_FOR_CROC
```
See `core/v1db/prepmesh.c` for the primary usage.

### glrend Shaders
`glrend` GLSL shaders have a preprocessing and compilation pipeline. See `drivers/glrend/CMakeLists.txt` for details.

### Driver Dispatch Tables
Driver dispatch tables are generated from `core/fw/dev_objs.hpp` (a specially-formatted C++ header)
by `core/fw/classgen.pl`. Do not edit the generated files directly.

## Documentation

See `resources/aidocs/` for detailed documentation on rendering architecture, lighting, and individual renderers/drivers.
