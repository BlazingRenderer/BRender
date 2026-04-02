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

## Contributing
- This is a fork of the BRender version shipped with Croc,
  with features from the official Argonaut 1.3.2 release backported.
  Major differences from prior Argonaut-official versions are outlined in `MIGRATION.md`.
- All PRs should be rebased against current master.
- Merge commits are not accepted except at maintainer discretion (e.g. octopus merges, or cases where preserving branch history has clear value). When in doubt, rebase.

## Coding Conventions
- C codebase with `br_*` prefix for public structs, `Br*` prefix for public functions.
- Run `clang-format` before committing (project has `.clang-format` configuration)
- Public API functions are decorated with `BR_PUBLIC_ENTRY`, cross-library (non-static internal) functions with `BR_RESIDENT_ENTRY`

## Commit Style
Format:
```
component: short description

More detailed description if necessary.
```
Each commit should be a single logical unit. Larger commits are acceptable if the change is nontrivial and can't be decoupled.

Examples:
```
core/v1db: fix RegenerateVertexNormals to respect smoothing groups
```
```
contrib/editorcam: fix camera offset after pan mode, add orbit mode
- Pan mode now applies translations to actor (world position) rather
  than camera (orientation), fixing offset after switching modes
- Add orbit mode (Alt+LMB drag) with azimuth/elevation rotation
```

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
