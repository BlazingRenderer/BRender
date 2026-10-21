# Agent Guidance for BRender

## Project Overview
BRender is a 3D rendering engine with multiple renderer backends:
- `core/` - Core engine, model preparation, math utilities
- `drivers/softrend/` - Software renderer
- `drivers/softprim/` - Software rasterizer, or the "primitive library"
- `drivers/glrend/` - OpenGL renderer
- `contrib/` - Contributed utilities (e.g., editorcam)

The tree descends from the BRender version shipped with Croc, with features from the
Argonaut 1.3.2 release backported; `MIGRATION.md` lists the differences from
Argonaut-official versions.

## Building
- Uses CMake.
- The software rasterizer (`drivers/softprim/`) is portable C/C++ and is built with the drivers on every architecture. It presents
  the driver entry point, device identifier (`SOFTPRMF`) and primitive library name (`Default-Primitives-Float`) that the x86 assembly
  rasteriser it replaced did.
- `drivers/softrend/` is the transform-and-lighting stage only and has no rasteriser of its own. It registers a renderer facility,
  whose `rendererNew()` calls `BrPrimitiveLibraryFind()` (`drivers/softrend/rendfcty.c`) to find softprim's primitive library.
- An OpenGL rasteriser is also available in `glrend1x`, giving softrend T&L over an OpenGL rasteriser. The demo programs select
  it with `--opengl-device-name=glrend1x` (`examples/brdemo/brdemo.c`); that flag is brdemo-level, not engine API - the engine receives
  the name as the `BRT_OPENGL_DEVICE_NAME_CSTR` token, which the SDL device uses to load the named driver (`drivers/sdl3dev/glrend.c`).
  glrend1x exposes a `BRT_PRIMITIVE_LIBRARY_O` that softrend's search then finds (`drivers/glrend1x/devpmglf.c`).

Local probes live under `scratch/`, which is git-excluded, and are wired into the build by an uncommitted block at the end of
`tools/CMakeLists.txt`. That block is never committed, and any history rewrite (`git reset --hard`, `git checkout -b`, a hand-rebuilt
series) discards it silently — re-add it afterwards instead of assuming it survived. It also makes `git worktree add` unusable from tools
that demand a clean tree.

## Benchmarking
- `tools/gltfbench` is the benchmark: a headless glTF scene renderer with no demo framework, no editor camera and no input, so a frame
  means the same thing to it as it does to `rendertest`. It renders through `BrZbSceneRender()` and takes `--device glrend|glrend1x|softrend`,
  `--width`/`--height` (default 1280x720), `--warmup N` (default 10), `--frames N` (default 1), the camera knobs
  `--dist`/`--angle`/`--elevation`, and `--checksum`, `--ppm PATH`, `--csv PATH` (one millisecond line per measured frame) and `--finish`.
- **`--finish` is what makes a CSV line a frame time.** It calls `glFinish()` after the swap, so a line runs from the frame's start until its
  GPU work has completed, rather than until its commands were submitted. Offscreen has no presentation, so without it the CPU runs ahead of
  the GPU and the samples barely move with resolution. It serialises the CPU and the GPU: absolute times are worse than a pipelined game
  achieves, and that is the price of the number being about one frame.
- **Run it under `SDL_VIDEODRIVER=offscreen`.** That gives a real GL context on the real GPU, and keeps the swap from pinning every frame to
  the refresh: `gltfbench` never sets the swap interval and SDL defaults it to 1, so a windowed run reports the refresh period whatever it
  rendered.
- **`examples/gltfview` is a viewer, not an instrument.** It builds on `brdemo`, which hardcodes vsync on (`SDL_GL_SetSwapInterval(1)`) and
  draws the frame time into the colour buffer; every demo built on `brdemo.c` inherits both. Pass `--no-stats` if you run one anyway, or the
  overlay makes runs differ and the checksum worthless.
- **A `brdemo` program never exits on its own**, `gltfview` included: `BrDemoRunArg()` loops until `SDL_EVENT_QUIT`, and offscreen has no
  quit to send it. `gltfbench` defaults to one measured frame and always returns. Bound anything that might spin with `timeout -s KILL` and
  never a plain `timeout`, because these programs install a SIGTERM handler and a plain `timeout` leaves the process running. The timeout is
  a backstop, not a mechanism: if it is the thing ending a run, the invocation is wrong.
- The checksum and the PPM come from the window-sized framebuffer, so they are only comparable between runs with identical
  `--width`/`--height`. There are no GPU timer queries; `--finish` is the only GPU-side number.
- Protocol: build each variant to its own binary, warm every binary, and run variants interleaved (ABBA). Report min and median and
  require them to agree; use n>=6 for anything under ~5%. Absolute times drift badly across sessions - only same-session interleaved
  ratios mean anything.

## Coding Conventions
- C codebase with `br_*` prefix for public structs, `Br*` prefix for public functions.
- Run `clang-format` before committing (project has `.clang-format` configuration).
- `BR_PUBLIC_ENTRY` decorates functions that are part of the public API. `BR_RESIDENT_ENTRY` decorates
  functions that cross library boundaries internally but are not part of the public API (i.e., non-static
  internal linkage). Do not omit these on applicable functions.

### Header names

A header's suffix says what it holds, and `_p` is **public**, not private:

- `name.h` — structures, and nothing another module has to link against. `br2d.h` carries
  `br_vertex_2d` and no prototypes.
- `name_p.h` — prototypes for the module's public entry points: the ones `brender.h` reaches,
  and the ones a driver may call through the DDI. `v1db_p.h`, `fw_p.h`, `pm_p.h`,
  `brddi_p.h`.
- `name_ip.h` — prototypes for internal linkage: callable inside the module, never exported.
  `v1db_ip.h`.

So a function's prototype goes in the `_p`/`_ip` header for its linkage, never beside a
structure in the plain `.h`, even when the two describe the same feature. A driver repeats the
split for its own private headers (`drv.h` for structures, `drv_ip.h` for prototypes).

## Commit Style
The commit conventions are the project's and are stated once, in
[CONTRIBUTING.md](CONTRIBUTING.md), so that this file and it cannot drift. Read it
before committing, and before sending a change.

What is specific to working in this tree with an agent, and is not a contributor
rule: commits are made unsigned and are never pushed — the maintainer rebases,
re-signs and pushes them, and every hash in this tree may change overnight.
`git commit --fixup` is how a follow-up is recorded; see the global conventions.

## Testing
There is no unit-test suite, and none should be added. `ctest` runs the render corpus with one test per configuration, and CI runs it for
both floating-point classes. `contrib/run-tests.sh [build-dir]` is the human-facing gate and adds the checks that do not belong in a
build - the build step itself and the `--history` commit walk. The corpus itself is:
- `tools/rendertest` renders the fixtures in `tools/rendertest/dat/` under one device and diffs each frame against the checked-in
  reference, `tools/rendertest/rendertest.txt`. Run it (`--device <glrend|glrend1x|softrend> [--bpp n] [--no-depth]`) after
  any change that can move pixels; `result=PASS failures=0` is the pass condition. The frame is fixed at 320x240.
- The reference key is `<device>/<driver>/<fp-class>/<zb|zs>/<pixel-type>/<WxH>/<scene>`. `<fp-class>` is the width `float` and `double`
  expressions are evaluated at - `x87` (80-bit intermediates, `FLT_EVAL_METHOD` 2) or `declared` (each operation rounded at its type's
  width, `FLT_EVAL_METHOD` 0) - and it reaches the frame through the renderer and through core's scene setup, so every entry is keyed by
  it and a build looks up its own class.
- The stored `glrend` entries are llvmpipe's, so a real-GPU run reports `NO-REFERENCE` for every scene - force it onto llvmpipe with
  `LIBGL_ALWAYS_SOFTWARE=true` to compare against them.
- The run also asserts relations between fixtures (e.g. `scene-unlit == scene-unlit-plain`), so a path that silently stops firing fails
  rather than only moving a checksum. `--ppm-dir <dir>` writes each frame as `<dir>/<scene>.ppm` when a checksum cannot show how a render changed.
- softrend is the software rasteriser path; `--no-depth` renders through
  `BrZsSceneRender()` and covers the Z-sort path. The fixtures come from `mkres scenes` (`tools/mkres/scene.c`), which writes them to
  the current directory.
The reference table is generated data, and blessing it is the one deliberate act:
- `tools/rendertest/dat/` and `tools/rendertest/rendertest.txt` are generated, so the commit that moves pixels or adds a fixture
  regenerates and re-blesses them; a change is not finished until it has.
- `--bless` rewrites every configuration the build registers for its floating-point class. Run it only to accept a deliberate change,
  then `git diff` the table and check the keys that moved are the ones you meant to move.
- The `x87` entries are **pentprim's**, not this build's, and are the bit-exactness oracle for softprim: bless them in the oracle
  worktree (`../<repo>-pent`), splice the entries in, and show 32-bit softprim reproduces them by running it *without* `--bless`.
  Blessing `x87` here records softprim's own output and the comparison stops meaning anything. `resources/aidocs/recipes.md` has the
  recipe. `declared` is this build's own bless and has nothing to reproduce.
- Bless once, when the tree that generates the fixtures is final: blessing against fixtures that are still moving costs a second bless
  and a second verification.

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
